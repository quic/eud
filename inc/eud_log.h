/*************************************************************************
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File:
*   eud_log.h
*
* Description:
*   Public header for the EUD file-logging infrastructure.
***************************************************************************/
#ifndef EUD_LOG_H_
#define EUD_LOG_H_

#include "eud_top_defines_internal.h"   
#include <stddef.h>                     

#if defined(EUD_LOG_SINK_FILE) && defined(EUD_LOG_SINK_DEBUGVIEW)
#   error "EUD logging: define only one of EUD_LOG_SINK_FILE or EUD_LOG_SINK_DEBUGVIEW, not both."
#endif

#if !defined(EUD_LOG_SINK_FILE) && !defined(EUD_LOG_SINK_DEBUGVIEW)
    /* Default sink when the caller has not specified one explicitly. */
#   define EUD_LOG_SINK_FILE
#endif

/*--------------------------------------------------------------------------
 * Platform-specific file log paths  
 *--------------------------------------------------------------------------*/
#if defined(EUD_WIN_ENV)
#   define EUD_LOG_DIR   "C:/Temp/EUD"
#   define EUD_LOG_FILE  "C:/Temp/EUD/eud_logs.txt"
#else
#   define EUD_LOG_DIR   "/tmp/EUD"
#   define EUD_LOG_FILE  "/tmp/EUD/eud_logs.txt"
#endif

/*--------------------------------------------------------------------------
 * Maximum size of a single formatted log message (bytes, including NUL).
 * Matches DBG_MSG_MAX_SZ used inside QCEUD_Print.
 *--------------------------------------------------------------------------*/
#define EUD_LOG_MSG_MAX_SZ  1024U

/*--------------------------------------------------------------------------
 * Cross-platform vsnprintf wrapper
 *--------------------------------------------------------------------------*/
#if defined(EUD_WIN_ENV)
#   define EUD_VSNPRINTF(buf, bufsz, fmt, ap) \
        _vsnprintf_s((buf), (bufsz), _TRUNCATE, (fmt), (ap))
#else
#   include <stdio.h>
#   define EUD_VSNPRINTF(buf, bufsz, fmt, ap) \
        vsnprintf((buf), (bufsz), (fmt), (ap))
#endif

/*--------------------------------------------------------------------------
 * Public API  (C linkage so usable from both C and C++ translation units)
 *--------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * eud_log_open()
 * Called from UsbOpen() each time a peripheral USB device is opened.
 */
void eud_log_open(void);

/**
 * eud_log_close()
 * Called from UsbClose() each time a peripheral USB device is closed.
 */
void eud_log_close(void);

/**
 * eud_log_write()
 * Intended to be called only from QCEUD_Print / QCEUD_Print2.
 * @param msg  NUL-terminated string to write.  Must not be NULL.
 */
void eud_log_write(const char *msg);

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* EUD_LOG_H_ */
