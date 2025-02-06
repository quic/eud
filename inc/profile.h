/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   profile.h
*
* Description:                                                              
*   Header file with APIs for time profiling 
*
***************************************************************************/
#ifndef PROFILE_H_
#define PROFILE_H_

#include <windows.h>

uint64_t timedelta(uint64_t end, uint64_t start)
{
    uint64_t freq;
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    if (!freq) {
        return 0;
    }
    else {
        return (uint64_t)((end - start));
    }
}

uint64_t GetPerformanceCounter() {
    LARGE_INTEGER current_time;
    if (QueryPerformanceCounter((LARGE_INTEGER *)&current_time) != 0)
        return (uint64_t)(current_time.QuadPart);
    else
        return 0;
}

#endif /* PROFILE_H_ */
