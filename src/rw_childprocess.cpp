/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   rw_childprocess.cpp
*
* Description:                                                              
*   CPP source file for creating and managing child process
*
***************************************************************************/
#if (defined(__WIN64))
#include <windows.h> 
#include <tchar.h>
#include <strsafe.h>
#endif

#include <stdio.h> 
#include <string>
// #include "eud_api.h"
#include <fstream>
#include "device_manager.h"

#include "eud.h"
#include "com_eud.h"
#include "ctl_eud.h"
#include "jtag_eud.h"
#include "swd_eud.h"
#include "trc_eud.h"

#define BUFSIZE 4096 
#define WRITE_TO_PIPE_USED 0


#if WRITE_TO_PIPE_USED
HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
#endif

#if 0
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

HANDLE g_hInputFile = NULL;

EUD_ERR_t CreateChildProcess(std::string executablename);
EUD_ERR_t PreloadReadPipe(HANDLE* handle_p);
std::string ReadFromPipe(EUD_ERR_t* err_p);
#endif

#if WRITE_TO_PIPE_USED
EUD_ERR_t WriteToPipe(void);
#endif
//#define TEST_DEVICEMGR_SCAN 1 ///<Disables this function so that manual device mgr scan is used instead.

#if 0
std::string retrieve_eud_id_string(EUD_ERR_t* err_p, std::string executablename,uint32_t options)
{
#ifdef TEST_DEVICEMGR_SCAN
    //return &std::string(PRELOAD_READ_PIPE_VALUE);
    return std::string(PRELOAD_READ_PIPE_VALUE);
#endif
    SECURITY_ATTRIBUTES saAttr;

    //std::string* rvalue_p = new std::string("");
    std::string rvalue_p;

    // Set the bInheritHandle flag so pipe handles are inherited
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT. 
    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)){
        //ErrorExit(TEXT("StdoutRd CreatePipe"));
        *err_p = eud_set_last_error(EUD_ERR_DURING_EUD_ID_CHILD_CALL);
       return rvalue_p;
    }

    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)){
        //ErrorExit(TEXT("Stdout SetHandleInformation"));
        *err_p = eud_set_last_error(EUD_ERR_DURING_EUD_ID_CHILD_CALL);
       return rvalue_p;
    }
#if WRITE_TO_PIPE_USED
    // Create a pipe for the child process's STDIN. 
    if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)){
        //ErrorExit(TEXT("Stdin CreatePipe"));
        *err_p = eud_set_last_error(EUD_ERR_DURING_EUD_ID_CHILD_CALL);
       return rvalue_p;
    }

    // Ensure the write handle to the pipe for STDIN is not inherited. 
    if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)){
        //ErrorExit(TEXT("Stdin SetHandleInformation"));
        *err_p = eud_set_last_error(EUD_ERR_DURING_EUD_ID_CHILD_CALL);
        return rvalue_p;
    }
#endif
    *err_p = PreloadReadPipe(&g_hChildStd_OUT_Wr);
    //Sleep(100);
    // Create the child process. 
    *err_p = CreateChildProcess(executablename);

#if WRITE_TO_PIPE_USED ///< This block in case we need to read from a file and write it to child process
    LPCSTR thing = TEXT("c:\\temp\\stuff.txt");
    g_hInputFile = CreateFile(
        thing,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_READONLY,
        NULL);

    if (g_hInputFile == INVALID_HANDLE_VALUE)
        //ErrorExit(TEXT("CreateFile"));
        *err_p = eud_set_last_error(EUD_ERR_DURING_EUD_ID_CHILD_CALL);

    // Write to the pipe that is the standard input for a child process. 
    // Data is written to the pipe's buffers, so it is not necessary to wait
    // until the child process is running before writing data.

    *err_p = WriteToPipe();
    
    //printf("\n->Contents of %s written to child STDIN pipe.\n", thing);
#endif
    
    if (options== RETRIEVE_EUD_ID_OPTIONS_LONGWAIT)
        Sleep(4000);
    else
        Sleep(2000);

    rvalue_p = ReadFromPipe(err_p);

    return rvalue_p;
}

// Create a child process that uses the previously created pipes for STDIN and STDOUT.
EUD_ERR_t CreateChildProcess(std::string executablename)
{

    std::ifstream f(executablename.c_str());
    if (!f.good()) {
        return eud_set_last_error(EUD_ERR_EUD_ID_EXE_NOT_FOUND);
    }
    TCHAR* szCmdline_p = new TCHAR[MAX_FILE_PATH_NAME];
    //TCHAR szCmdline[] = TEXT("c:\\dropbox\\eud\\eud_project\\eud_id5.exe");
    sprintf_s(szCmdline_p, MAX_FILE_PATH_NAME, executablename.c_str());

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;

    // Set up members of the PROCESS_INFORMATION structure. 

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure. 
    // This structure specifies the STDIN and STDOUT handles for redirection.

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    //siStartInfo.hStdInput = g_hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process. 

    bSuccess = CreateProcess(NULL,
        szCmdline_p,     // command line 
        NULL,          // process security attributes 
        NULL,          // primary thread security attributes 
        TRUE,          // handles are inherited 
        CREATE_NO_WINDOW,// creation flags 
        NULL,          // use parent's environment 
        NULL,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION 

    // If an error occurs, exit the application. 
    if (!bSuccess)
        //ErrorExit(TEXT("CreateProcess"));
        return eud_set_last_error(EUD_ERR_DURING_CREATE_CHILD_PROCESS);
    else
    {
        // Close handles to the child process and its primary thread.
        // Some applications might keep these handles to monitor the status
        // of the child process, for example. 

        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
    }
    return EUD_SUCCESS;
}
#endif

#if 0
EUD_ERR_t PreloadReadPipe(HANDLE* handle_p)
//Write data to specified handle buffer. Used to preload buffer with a desired message.
//In reality, a hack to get around Window's issue of never closing a PIPE  if no data is read.
{
    DWORD dwRead = 15;
    DWORD dwWritten;
    std::string strbuf = std::string(PRELOAD_READ_PIPE_VALUE);
    BOOL bSuccess = FALSE;

    if (!WriteFile(*handle_p, strbuf.c_str(), (DWORD)strbuf.length(), &dwWritten, NULL))
        return eud_set_last_error(EUD_ERR_PIPE_PRELOAD);

    return EUD_SUCCESS;
}
#endif

#if WRITE_TO_PIPE_USED
EUD_ERR_t WriteToPipe(void)

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data. 
{
    DWORD dwRead, dwWritten;
    CHAR chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;

    for (;;)
    {
        bSuccess = ReadFile(g_hInputFile, chBuf, BUFSIZE, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) break;

        bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
        if (!bSuccess) break;
    }

    // Close the pipe handle so the child process stops reading. 

    if (!CloseHandle(g_hChildStd_IN_Wr))
        return eud_set_last_error(EUD_ERR_DURING_READ_FROM_PIPE);
        //ErrorExit(TEXT("StdInWr CloseHandle"));
}
#endif

#if 0
std::string ReadFromPipe(EUD_ERR_t* err_p)

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
{
    DWORD dwRead;
    CHAR chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;
    HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    
    bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);

    if (!bSuccess || dwRead == 0){
        *err_p = eud_set_last_error(EUD_ERR_DURING_READ_FROM_PIPE);
        return std::string("");
    }

    return std::string(chBuf);
}
#endif
