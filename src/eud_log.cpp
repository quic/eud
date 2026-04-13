/*************************************************************************
*
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File:
*   eud_log.cpp
*
* Description:
*   Implementation of the EUD file-logging infrastructure.
*
*   Architecture
*   ────────────
*   A single static FILE* (eud_log_fp_) is the log-file handle for the
*   entire session.  A simple integer reference counter (eud_log_ref_)
*   tracks how many peripheral USB devices are currently open:
*
*   eud_log_open()  – if ref count is 0, creates the directory, opens
*                     the file, and writes a session-start banner.
*                     Always increments the ref count.
*   eud_log_close() – decrements the ref count.  Only when it reaches
*                     zero is the file flushed, closed, and the handle
*                     set to NULL.
*
*   This design means the log file stays open for the lifetime of the
*   longest-lived peripheral.  Example:
*
*     eud_log_open()   → ref=1, file opened    (CTL peripheral opens)
*     eud_log_open()   → ref=2                 (TRC peripheral opens)
*     eud_log_close()  → ref=1, file stays open (CTL peripheral closes)
*     eud_log_close()  → ref=0, file closed     (TRC peripheral closes)
*
*   No mutex or thread synchronisation is used – the logging path is
*   intentionally simple and single-threaded.
*
*   Lifecycle
*   ─────────
*   eud_log_open()   – called from UsbOpen() for every peripheral.
*   eud_log_close()  – called from UsbClose() for every peripheral.
*   eud_log_write()  – called by every QCEUD_Print invocation.
*
***************************************************************************/

#include "eud_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*──────────────────────────────────────────────────────────────────────────
 * Platform headers
 *──────────────────────────────────────────────────────────────────────────*/
#if defined(EUD_WIN_ENV)
#   include <windows.h>   /* GetLocalTime, _mkdir */
#   include <direct.h>    /* _mkdir()             */
#else
#   include <sys/stat.h>  /* mkdir()              */
#   include <sys/types.h>
#   include <time.h>      /* clock_gettime, localtime_r */
#endif

/*──────────────────────────────────────────────────────────────────────────
 * Internal state  (translation-unit scope only)
 *──────────────────────────────────────────────────────────────────────────*/

/** Log-file handle.  NULL means the file is not currently open. */
static FILE *eud_log_fp_ = NULL;

/** Actual path of the log file chosen for this session. */
static char eud_log_path_[256] = EUD_LOG_FILE;

/**
 * Reference counter tracking how many peripheral USB devices are currently
 * open.  The log file is opened when this transitions from 0 to 1 and
 * closed when it transitions from 1 to 0.
 */
static int eud_log_ref_ = 0;

/*──────────────────────────────────────────────────────────────────────────
 * Internal helpers
 *──────────────────────────────────────────────────────────────────────────*/

/**
 * make_log_dir()
 *
 * Creates EUD_LOG_DIR if it does not already exist.
 * Both _mkdir (Windows) and mkdir (Linux) return non-zero if the directory
 * already exists, which is not treated as an error.
 */
static void make_log_dir(void)
{
#if defined(EUD_WIN_ENV)
    _mkdir(EUD_LOG_DIR);
#else
    mkdir(EUD_LOG_DIR, 0755);
#endif
}

/**
 * resolve_log_path()
 *
 * If EUD_LOG_FILE already exists, finds the next free numbered variant
 * (eud_logs_01.txt, eud_logs_02.txt, …) and stores it in eud_log_path_.
 */
static void resolve_log_path(void)
{
    FILE *probe = fopen(EUD_LOG_FILE, "r");
    if (probe == NULL) {
        /* Base file does not exist yet – use it directly. */
        strncpy(eud_log_path_, EUD_LOG_FILE, sizeof(eud_log_path_) - 1);
        return;
    }
    fclose(probe);

    /* Base file exists – find the next free numbered slot. */
    for (int i = 1; i <= 99; i++) {
        snprintf(eud_log_path_, sizeof(eud_log_path_),
                 EUD_LOG_DIR "/eud_logs_%02d.txt", i);
        probe = fopen(eud_log_path_, "r");
        if (probe == NULL) {
            break; /* This numbered file does not exist yet – use it. */
        }
        fclose(probe);
    }
}

/**
 * write_timestamp()
 *
 * Writes a  [HH:MM:SS.mmm]  prefix to the already-open file fp.
 */
static void write_timestamp(FILE *fp)
{
#if defined(EUD_WIN_ENV)
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(fp, "[%02d:%02d:%02d.%03d] ",
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
    struct timespec ts;
    struct tm       tm_info;
    clock_gettime(CLOCK_REALTIME, &ts);
    localtime_r(&ts.tv_sec, &tm_info);
    fprintf(fp, "[%02d:%02d:%02d.%03ld] ",
            tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec,
            (long)(ts.tv_nsec / 1000000L));
#endif
}

/**
 * eud_log_open()
 *
 * Increments the internal reference count.  On the first call (ref count
 * transitions from 0 to 1) the log directory is created if needed, the
 * log file is opened in append mode, and a session-start banner is written.
 *
 * Subsequent calls (from additional peripherals opening their USB device)
 * only increment the counter – the already-open file handle is reused.
 *
 * A warning is printed to stderr if the file cannot be opened, but the
 * call never aborts the process.
 *
 * Called from UsbOpen() for every peripheral USB device that is opened.
 */
void eud_log_open(void)
{
#if defined(EUD_LOG_SINK_DEBUGVIEW)
    /* DebugView sink: no file to open. */
    return;
#else
    /* Always increment the reference count first. */
    eud_log_ref_++;

    /* Only open the file on the very first call. */
    if (eud_log_ref_ > 1) {
        return;
    }

    make_log_dir();
    resolve_log_path();

    eud_log_fp_ = fopen(eud_log_path_, "a");
    if (eud_log_fp_ == NULL) {
        fprintf(stderr,
                "[EUD] WARNING: cannot open log file '%s' – "
                "logging to stderr instead.\n",
                eud_log_path_);
        return;
    }

    /* Session-start banner so successive runs are easy to separate. */
    write_timestamp(eud_log_fp_);
    fprintf(eud_log_fp_,
            "===== EUD log session started (file: %s) =====\n",
            eud_log_path_);
    fflush(eud_log_fp_);
#endif /* EUD_LOG_SINK_DEBUGVIEW */
}

/**
 * eud_log_close()
 *
 * Decrements the internal reference count.  The log file is physically
 * flushed and closed only when the count reaches zero, i.e. when the last
 * active peripheral calls UsbClose().
 *
 * Intermediate calls (e.g. closing the control peripheral while a trace
 * session is still running) simply decrement the count and return, leaving
 * the file open so that subsequent log messages are not lost.
 *
 * Called from UsbClose() for every peripheral USB device that is closed.
 */
void eud_log_close(void)
{
#if defined(EUD_LOG_SINK_DEBUGVIEW)
    /* DebugView sink: no file to close. */
    return;
#else
    /* Guard against unmatched close calls. */
    if (eud_log_ref_ <= 0) {
        return;
    }

    eud_log_ref_--;

    /* Only close the file when the last peripheral has been closed. */
    if (eud_log_ref_ > 0) {
        return;
    }

    if (eud_log_fp_ != NULL) {
        write_timestamp(eud_log_fp_);
        fprintf(eud_log_fp_,
                "===== EUD log session ended =====\n\n");
        fflush(eud_log_fp_);
        fclose(eud_log_fp_);
        eud_log_fp_ = NULL;
    }
#endif /* EUD_LOG_SINK_DEBUGVIEW */
}

/**
 * eud_log_write()
 *
 * Writes  [HH:MM:SS.mmm] <msg>  to the log file.
 *
 * Fallback behaviour when the file is not open:
 *   – Writes to stderr so the message is never silently lost.
 *
 * Performance:
 *   – fflush() is called after every write so that the file is always
 *     up-to-date even if the process crashes.  The cost is one kernel
 *     call per log line, which is acceptable given that logging is only
 *     active when PERIPHERAL_PRINT_ENABLE is defined.
 */
void eud_log_write(const char *msg)
{
#if defined(EUD_LOG_SINK_DEBUGVIEW)
    /* DebugView sink: QCEUD_Print posts directly via OutputDebugString() /
     * stderr; nothing to do here. */
    (void)msg;
    return;
#else
    if (msg == NULL) {
        return;
    }

    if (eud_log_fp_ == NULL) {
        /* File not open – fall back to stderr so the message is not lost. */
        fprintf(stderr, "[EUD-nolog] %s", msg);
        return;
    }

    write_timestamp(eud_log_fp_);
    fprintf(eud_log_fp_, "%s", msg);
    fflush(eud_log_fp_);
#endif /* EUD_LOG_SINK_DEBUGVIEW */
}
