/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   debugger_defines.h
*
* Description:                                                              
*   Header file with macros used for additional debug options 
*
***************************************************************************/

#ifndef DEBUGGER_DEFINES_H_
#define DEBUGGER_DEFINES_H_ 1


/*********************//**
* @brief Enables or disables communication to USB.
* If disabled, return Bytes are populated with 0xFA
*
*************************/
#define ENABLE_USB 1        ///<Debug check option. If not defined, fake usb data  will be returned  and no hardware is  needed.

/*********************//**
* @brief Useful for debugging on XILINX FPGA instead of 
* silicon or RUMI. Notably, MSMAssertReset, MSMDeassertReset
* SpoofAttach and SpoofDetach are disabled.
*************************/
//#define FPGA 0 ///<Conditionally defined on compile command line for FPGA platforms.



#define HANA            ///<Check for spoof attach/detach, charger disable/enable logic



#endif

