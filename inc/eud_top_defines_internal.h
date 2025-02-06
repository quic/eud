/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   eud_top_defines_internal.h
*
* Description:                                                              
*   OS Environment configuration
*
***************************************************************************/
#ifndef EUD_TOP_DEFINES_INTERNAL_
#define EUD_TOP_DEFINES_INTERNAL_

#include "eud_revision.h"

//#if defined(__WIN32) || defined(__WIN64)
//#if defined(_MSC_VER)
//#define EUD_WIN_ENV 1
//#endif

#if defined(_WIN32) || defined(_WIN64)
#define EUD_WIN_ENV 1
#endif

#if (defined(__GNUC__) && defined(__unix__))
#define EUD_LNX_ENV 1
#endif

#if (defined(__GNUC__) && defined(__WIN64))
#define EUD_CYG_ENV 1
#endif

#endif // EUD_TOP_DEFINES_INTERNAL_