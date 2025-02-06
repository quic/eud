/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   eud_error_defines.h
*
* Description:                                                              
*   Header file with error codes for EUD Operations
*   Most API's return an error code (0 for success, non-0 for some failure)
*
***************************************************************************/
#ifndef EUD_ERROR_DEFINES_H_
#define EUD_ERROR_DEFINES_H_

//QCOM_EXPORT=True
#include "eud_types.h"


typedef int32_t EUD_ERR_t; ///<Error code typedef (int32_t)

////////////////////////////EUD ERROR Defines////////////////////////////////
//                                                                         //
//  If you add an error definition, please add its description in          //
//  eud_error_defines.cpp as a string to be returned by eud_get_error_string()  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//
//Different error classes. Added to subclass errors to be masked out if desired
#define EUD_ERR_CLASS_GENERAL   0x80000000
#define EUD_ERR_CLASS_HANDLE    0x40000000
#define EUD_ERR_CLASS_USB       0x20000000
#define EUD_ERR_CLASS_PERIPH    0x10000000

//Generic err codes
#define EUD_SUCCESS                    0
#define EUD_GENERIC_FAILURE            1
#define EUD_ERR_FUNCTION_NOT_AVAILABLE 2
//Handle and parameter errors
#define EUD_ERR_BAD_HANDLE_PARAMETER                            EUD_ERR_CLASS_HANDLE+1
#define EUD_ERR_NULL_POINTER                                    EUD_ERR_CLASS_HANDLE+2
#define EUD_ERR_BAD_CAST                                        EUD_ERR_CLASS_HANDLE+3
#define EUD_BAD_PARAMETER                                       EUD_ERR_CLASS_HANDLE+4
#define EUD_USB_ERR_HANDLE_UNITIALIZED                          EUD_ERR_CLASS_HANDLE+5
#define EUD_TRC_ERR_HANDLE_ALREADY_INITD                        EUD_ERR_CLASS_HANDLE+6
#define EUD_ERR_BAD_PARAMETER_HANDLE_NOT_POPULATED              EUD_ERR_CLASS_HANDLE+7

//EUD Generic err codes

#define EUD_ERR_NOT_YET_SET                                     EUD_ERR_CLASS_GENERAL+1
#define EUD_ERR_BAD_PARAMETER                                   EUD_ERR_CLASS_GENERAL+2
#define EUD_ERR_INCORRECT_ARGUMENTS_WRITE_COMMAND               EUD_ERR_CLASS_GENERAL+3
//EUD Generic err codes - device manager errors
#define EUD_ERR_CANT_CHANGE_DEVICE_MGR_MODE_WHEN_IN_AUTO        EUD_ERR_CLASS_GENERAL+4
#define EUD_ERR_EUD_HANDLER_DEVICEID_NOT_POPULATED              EUD_ERR_CLASS_GENERAL+5
#define EUD_ERR_EUD_HANDLER_DEVICEID_NOT_RECOGNIZED             EUD_ERR_CLASS_GENERAL+6
#define EUD_ERR_DEVICE_MANAGER_MODE_PARAMETER_ERR               EUD_ERR_CLASS_GENERAL+7
#define EUD_ERR_UNKNOWN_OPCODE_SELECTED                         EUD_ERR_CLASS_GENERAL+8
#define EUD_ERR_NULL_POINTER_GIVEN_FOR_NONZERO_PACKET_RESPONSE  EUD_ERR_CLASS_GENERAL+9
#define EUD_ERR_NEED_DEVMGR_INPUT_MANUAL_MODE                   EUD_ERR_CLASS_GENERAL+10
#define EUD_ERROR_MAX_DEVMGR_DEVICES_SEARCHED                   EUD_ERR_CLASS_GENERAL+11
#define EUD_ERR_WIN_PIPE_CREATE                                 EUD_ERR_CLASS_GENERAL+12
#define EUD_ERR_DEVICEID_NOT_FOUND                              EUD_ERR_CLASS_GENERAL+13
#define EUD_ERR_INITIALIZED_DEVICE_NOT_FOUND                    EUD_ERR_CLASS_GENERAL+14

#define EUD_ERR_DURING_EUD_ID_CHILD_CALL                        EUD_ERR_CLASS_GENERAL+15
#define EUD_ERR_DURING_CREATE_CHILD_PROCESS                     EUD_ERR_CLASS_GENERAL+16
#define EUD_ERR_DURING_READ_FROM_PIPE                           EUD_ERR_CLASS_GENERAL+17
#define EUD_ERR_EUD_ID_EXE_NOT_FOUND                            EUD_ERR_CLASS_GENERAL+18
#define EUD_ERR_CTL_ENUMERATION_FAILED                          EUD_ERR_CLASS_GENERAL+19
#define EUD_ERR_PIPE_PRELOAD                                    EUD_ERR_CLASS_GENERAL+20
//USB error codes

#define EUD_USB_SUCCESS                                         0
#define EUD_USB_ERROR                                           EUD_ERR_CLASS_USB + 1
#define EUD_USB_ERROR_SEND_FAILED                               EUD_ERR_CLASS_USB + 2
#define EUD_USB_ERROR_WRITE_FAILED                              EUD_ERR_CLASS_USB + 3
#define EUD_USB_ERROR_WRITE_DEVICE_ERROR                        EUD_ERR_CLASS_USB + 4
#define EUD_USB_ERROR_NOT_ALL_BYTES_SENT                        EUD_ERR_CLASS_USB + 5
#define EUD_USB_ERROR_NOT_ALL_BYTES_READ                        EUD_ERR_CLASS_USB + 6
#define EUD_USB_ERROR_DEVICE_ERROR                              EUD_ERR_CLASS_USB + 7
#define EUD_USB_ERROR_READ_TIMEOUT                              EUD_ERR_CLASS_USB + 8
#define EUD_USB_ERROR_READ_FAILED_GENERIC                       EUD_ERR_CLASS_USB + 9
#define EUD_USB_DEVICE_NOT_DETECTED                             EUD_ERR_CLASS_USB + 10
#define USB_ERR_SENDSIZE_0_NODATASENT                           EUD_ERR_CLASS_USB + 11
#define EUD_USB_ERROR_SEND_CONFIRMATION_TIMEOUT                 EUD_ERR_CLASS_USB + 12
//Peripheral state errors

#define EUD_ERR_CLASS_CTL                                       0x0800000+EUD_ERR_CLASS_PERIPH
#define EUD_ERR_CLASS_JTG                                       0x0400000+EUD_ERR_CLASS_PERIPH
#define EUD_ERR_CLASS_SWD                                       0x0200000+EUD_ERR_CLASS_PERIPH
#define EUD_ERR_CLASS_TRC                                       0x0100000+EUD_ERR_CLASS_PERIPH
#define EUD_ERR_CLASS_COM                                       0x0080000+EUD_ERR_CLASS_PERIPH


//Generic peripheral errors
#define EUD_ERR_UNKNOWN_FREQ_REQUEST                            EUD_ERR_CLASS_PERIPH+1
//CTL error state defines
#define EUD_ERR_CTL_GIVEN_RSTDELAY_TOO_HIGH                     EUD_ERR_CLASS_CTL+1
#define CTL_EUD_ERR_VBUS_INT_NOT_CLEARED                        EUD_ERR_CLASS_CTL+2
#define CTL_EUD_ERR_CHGR_INT_NOT_CLEARED                        EUD_ERR_CLASS_CTL+3
//Jtag error state defines
#define EUD_ERR_TDO_BUFFER_OVERFLOW                             EUD_ERR_CLASS_JTG+1
#define EUD_ERR_TDO_BUFFER_UNDERFLOW                            EUD_ERR_CLASS_JTG+2
#define EUD_ERR_JTAG_OUT_BUFFER_OVERFLOW                        EUD_ERR_CLASS_JTG+3
#define EUD_ERR_JTG_EXPECTED_NONZERO_TDO                        EUD_ERR_CLASS_JTG+4
#define EUD_ERR_JTAG_SCAN_BAD_STATE                             EUD_ERR_CLASS_JTG+5
#define EUD_ERR_JTG_NOT_IMPLEMENTED                             EUD_ERR_CLASS_JTG+6
#define EUD_ERR_JTG_DEVICE_NOT_RESPONSIVE                       EUD_ERR_CLASS_JTG+7
#define EUD_JTG_ERR_REQUIRE_FLUSH                               EUD_ERR_CLASS_JTG+8
//SWD error state defines
#define EUD_SWD_ERR_FUNC_NOT_IMPLEMENTED                        EUD_ERR_CLASS_SWD+1
#define EUD_SWD_ERR_SWDCMD_MAXCOUNT_EXCEEDED                    EUD_ERR_CLASS_SWD+2
#define EUD_SWD_ERR_NULL_DATA_POINTER_ON_WRITE_CMD              EUD_ERR_CLASS_SWD+3
#define EUD_SWD_ERR_BAD_DI_TMS_PARAMETER                        EUD_ERR_CLASS_SWD+4
#define EUD_SWD_ERR_SWD_TO_JTAG_ALREADY_DONE                    EUD_ERR_CLASS_SWD+5
#define EUD_SWD_ERR_JTAGID_REQUESTED_BEFORE_STOJDONE            EUD_ERR_CLASS_SWD+6
#define EUD_SWD_ERR_JTAGID_NOT_RECEIVED                         EUD_ERR_CLASS_SWD+7
#define EUD_SWD_ERR_ITEMQUEUED_BUT_FLUSH_REQUIRED               EUD_ERR_CLASS_SWD+8
#define EUD_SWD_ERR_FLUSH_REQUIRED                              EUD_ERR_CLASS_SWD+9
#define EUD_SWD_ERR_UNKNOWN_MODE_SELECTED                       EUD_ERR_CLASS_SWD+10
#define EUD_SWD_ERR_MAX_USB_IN_BUFFER_REACHED                   EUD_ERR_CLASS_SWD+11
#define EUD_SWD_ERR_EXPECTED_BYTES_MISCALCULATION               EUD_ERR_CLASS_SWD+12
#define EUD_ERR_SWD_MANAGED_BUFFER_USB_IN_BUFFER_OVERFLOW       EUD_ERR_CLASS_SWD+13
#define EUD_ERR_SWD_BAD_DATA_STRUCT                             EUD_ERR_CLASS_SWD+14
#define EUD_SWD_ERR_CANNOT_CHANGE_READBUFFER_IN_MANAGEDBUFMODE  EUD_ERR_CLASS_SWD+15
#define EUD_SWD_ERR_NO_JTAGID                                   EUD_ERR_CLASS_SWD+16
#define EUD_ERR_SWD_NUM_BYTES_GREATER_THAN_BUFFSIZE             EUD_ERR_CLASS_SWD+17
#define SWD_ERR_SWD_ACK_WAIT_DETECTED                           EUD_ERR_CLASS_SWD+18
#define SWD_ERR_SWD_ACK_FAULT_DETECTED                          EUD_ERR_CLASS_SWD+19
#define SWD_ERR_SWD_PACKET_ASSEMBLE_ERR                         EUD_ERR_CLASS_SWD+20
#define EUD_ERR_BAD_PARAMETER_NULL_POINTER_SWDREAD              EUD_ERR_CLASS_SWD+21
#define SWD_WARN_SWDSTATUS_NOT_UPDATED                          EUD_ERR_CLASS_SWD+22
#define EUD_SWD_ERR_STATUS_COUNT_0                              EUD_ERR_CLASS_SWD+23
//Trace Error state defines
#define EUD_TRC_ERR_FUNCTION_NOT_IMPLEMENTED                    EUD_ERR_CLASS_TRC+1
#define EUD_TRC_ERR_NO_DATA                                     EUD_ERR_CLASS_TRC+2
#define EUD_ERR_INVALID_DIRECTORY_GIVEN                         EUD_ERR_CLASS_TRC+3
#define EUD_ERR_READONLY_DIRECTORY_GIVEN                        EUD_ERR_CLASS_TRC+4
#define EUD_TRACE_CTRL_HANDLER_REGISTRATION_ERR                 EUD_ERR_CLASS_TRC+5
#define EUD_ERR_TRACE_CHUNKSIZE_LESS_THAN_TRANSLEN              EUD_ERR_CLASS_TRC+6
#define EUD_TRC_TRANSLEN_GREATER_THAN_CHUNKSIZE                 EUD_ERR_CLASS_TRC+7
#define EUD_TRC_TRANSLEN_CANNOT_BE_MULTIPLE_128                 EUD_ERR_CLASS_TRC+8
#define EUD_ERROR_TRACE_DATA_STOPPED                            EUD_ERR_CLASS_TRC+9
//COM error state defines
//---


/////////////////////Static global variables/////////////////////////

extern EUD_ERR_t LastError; ///<Last Error, returned by eud_get_last_error(). EUD Does not support storing more than one error at this time.
extern EUD_ERR_t LastUSBError; ///<Last USB Error, returned by EUDGetLastUSBError().

#define MAX_ERR_STRING_SIZE 200 ///<Guarantee that eud_get_error_string will not return a string longer than \ref MAX_ERR_STRING_SIZE

/// \cond
/////////////////////Internal error functions////////////////////////

EUD_ERR_t eud_set_last_error(EUD_ERR_t givenerror);
EUD_ERR_t eud_set_last_usb_error(EUD_ERR_t givenerror);

/// \endcond

/////////////////////External error functions////////////////////////
#if defined (EUD_WIN_ENV)
typedef unsigned __int32  uint32_t;
#endif

EXPORT EUD_ERR_t
/**************************************//**
*   @brief Retrieve ASCII description of given error string. 
*   
*   Print to given \ref stringbuffer. Size of string is output to \ref stringsize_p
*
*******************************************/
eud_get_error_string(
    EUD_ERR_t errorcode, ///<Error code in question. Type EUD_ERR_t (int32_t)
    char* stringbuffer,  ///<ASCII error description printed to this pointer. Length is /stringsize_p. Guaranteed no longer than MAX_ERR_STRING_SIZE
    uint32_t* stringsize_p ///<Length of ASCII error string in bytes output to this parameter.
    );
    
    
EXPORT EUD_ERR_t 
/**************************************//**
*   @brief Get last stored error from EUD Software
*
*   Usually error is returned from any method immediately. 
*   However in case of initializer functions which return a pointer 
*   and return 0 if failure, eud_get_last_error can be called to retrieve
*   the error code indicating failure. eud_get_error_string can be used
*   to get an ASCII description of the error.
*
*******************************************/
eud_get_last_error(void); 




#endif

