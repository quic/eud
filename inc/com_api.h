/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   com_api.h
*
* Description:                                                              
*   Header file for EUD COM Port public APIs declerations 
*
***************************************************************************/
#ifndef COM_API_H_
#define COM_API_H_

// #include "eud_api.h"
#include "com_eud.h"

#ifndef __cplusplus
typedef void ComEudDevice;
#endif

/**************************************//**
*   @brief Test routine
*
*******************************************/                        

EXPORT EUD_ERR_t test_function(uint32_t device_id, uint8_t exec_env_id);

#endif //COM_API_H_