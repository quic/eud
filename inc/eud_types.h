/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   eud_types.h
*
* Description:                                                              
*   Header files with general declerations
*
***************************************************************************/
#ifndef EUD_TYPES_H_
#define EUD_TYPES_H_

// #include <cstdint>
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "eud_top_defines_internal.h"
//QCOM_SNIP_END

#define EUD_EXPORT 1

#if defined(_WIN32) || defined(_WIN64)
    // #if defined(_MSC_VER)
    //#define EUD_WIN_ENV (1)
    #define EUD_WIN_LIBUSB  (1)    
#endif

#if (defined(__GNUC__) && defined(__unix__))
    #define EUD_LNX_ENV 1
#endif


#if defined ( EUD_LNX_ENV )
#if defined __GNUG__
#define EXPORT extern "C" __attribute__((visibility("default")))
#else
#define EXPORT extern
#endif
#elif defined (EUD_WIN_ENV)
#ifdef EUD_EXPORT
#ifdef __cplusplus
#define EXPORT extern "C" __declspec(dllexport) ///<Declaration making function externally accessible API.
#else
#define EXPORT extern __declspec(dllexport)
#endif
#else
#ifdef __cplusplus
#define EXPORT extern "C" __declspec(dllimport) ///<Declaration to implicitly link to EUD API's.
#else
#define EXPORT extern __declspec(dllimport) ///<Declaration to implicitly link to EUD API's.
#endif
#endif
#endif

 // Types
#if defined EUD_WIN_ENV
    typedef signed char       int8_t;
    typedef signed short      int16_t;
    typedef signed int        int32_t;
    typedef unsigned char     uint8_t;
    typedef unsigned short    uint16_t;
    typedef unsigned int      uint32_t;
    typedef signed __int64    int64_t;
    typedef unsigned __int64  uint64_t;
#endif


#if defined EUD_LNX_ENV
    #ifndef FALSE
    #define FALSE (0)
    #define TRUE (!(FALSE))
    #endif

    static inline
    int LIBSYS_SNPRINTF(char * str, size_t size, const char * format, ...)
    {
        int retval;
        va_list ap;
        va_start(ap, format);
        retval = vsnprintf(str, size, format, ap);
        va_end(ap);
        return retval;
    }


    #define sprintf_s LIBSYS_SNPRINTF  

    #define Sleep(sleepTime)  //DW Todo 

    #ifndef _cdecl
    #define _cdecl __attribute__((cdecl))
    #endif

    typedef void VOID;
    typedef char CHAR;

    typedef const CHAR* PCHAR;
    typedef unsigned long DWORD;

    typedef void *PVOID;
    typedef int BOOL;

#endif   // EUD_LNX_ENV
#endif