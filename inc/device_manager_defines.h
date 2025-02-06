/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   device_manager_defines.h
*
* Description:                                                              
*   Header file with device manager name for EUD USB devices. USB driver
*   uses these to attach to devices for windows OS only
*
***************************************************************************/
#ifndef DEVICE_MANAGER_DEFINES_H_
#define DEVICE_MANAGER_DEFINES_H_

#include "eud_types.h"
//QCOM_EXPORT=True

//=====================================================================================//
//                      Device Manager ID Operations.
//                      --------------------------
//                      (For Windows OS only.)
//
//           Various EUD devices can show up in Windows Device manager 
//           with the suffix '(0000)' or '(0012)' etc.
//           This set of functions allows the user to specify the  device
//           manager ID, as a set of four digits, to the EUD handle, so that 
//           EUD software can connect with the  desired device.
//
//           There are two modes: Auto Select, and Manual Select.
//
//              - Auto Select will find the first available EUD device connected by 
//                polling for available devices and immediately connecting.
//                A failure will be logged if EUD cannot find a device  within 1 minute.
//                (The user can query the error string with eud_get_last_error)
//   
//              - Manual Select mode expects the user to specify the numerical ID.
//                If Manual load is set, EUD will log an error. Once the user specifies
//                the device value using the set_device_manager_id() function, EUD software
//                will attempt to reconnect to the device and  resume  normal operation.
//
//           Note  that the system starts in Auto  Select mode, so the user must 
//           specify Manual Select if desired.
//
//=====================================================================================//


/**Definitions for windows device manager*/

#define MAX_DEVMGR_NAME_CHARLEN 45

#define MAX_USB_ATTACH_RETRY_COUNT 4 ///<How many times to cycle through all available device manager names.

#define MAX_DEVICEMGR_POLL_VALUE 50
#define MAX_DEVMGR_DEVICEID 9999
#define MAX_DEVMGR_POLLNUM   100
#define DEVNUM_POLL_VALUE    1   ///<How long to poll on each device number before polling to the next.

#define MAX_DEVICE_MGR_MODES 2
#define DEVMGR_NUMBER_STRING_LEN 4
#define DEVMGR_PERIPH_NUM_PREFIX " ("
#define DEVMGR_PERIPH_NUM_SUFFIX ")"
#define DEVMGR_PERIPH_COM_NUM_PREFIX ""
#define DEVMGR_PERIPH_COM_NUM_SUFFIX ""
#define ASCII_0 48
#define EUD_DEVMGR_MAX_DEVMGR_STRING_LEN 60

#define EUD_NULL_DEVMGR_NAME "NULL"

#define EUD_MANUAL_MODE_NEED_USER_INPUT "Manual Mode - need user input"
#define EUD_AUTO_MODE_MAX_DEVICES_REACHED "Auto mode - max devices reached"
///Control Peripheral Device Manager name (Windows OS)
///Should show up if EUD is enabled from target. 
///Get a handleWrap pointer via EUDInitializeDeviceCTL
#define USB_DEBUG_PREFIX "\\\\.\\"
#define USB_DEBUG_BASESUFFIX "\\DEBUG"
#define EUD_CTL_DEVMGR_NAME "\\\\.\\Qualcomm EUD Control Device 9501 (0003)\\DEBUG"
#define EUD_CTL_DEVMGR_BASENAME "Qualcomm EUD Control Device 9501"
#define EUD_CTL_DEVMGR_BASESUFFIX "\\DEBUG"

///Trace Peripheral Device Manager name (Windows OS)
///Enable this peripheral and get a handleWrap pointer via eud_initialize_device_com
#define EUD_TRC_DEVMGR_NAME "\\\\.\\Qualcomm EUD Trace Device 9502\\DEBUG"
#define EUD_TRC_DEVMGR_BASENAME "Qualcomm EUD Trace Device 9502"
#define EUD_TRC_DEVMGR_BASESUFFIX "\\DEBUG"

///JTAG Peripheral Device Manager name (Windows OS)
///Enable this peripheral and get a handleWrap pointer via eud_initialize_device_jtag
//#define EUD_JTG_DEVMGR_NAME "\\\\.\\Qualcomm EUD JTAG Device 9503\\DEBUG"
#define EUD_JTG_DEVMGR_NAME "\\\\.\\Qualcomm EUD JTAG Device 9503 (0005)\\DEBUG"
#define EUD_JTG_DEVMGR_BASENAME "Qualcomm EUD JTAG Device 9503"
#define EUD_JTG_DEVMGR_BASESUFFIX "\\DEBUG"

///Serial Wire Debug Peripheral Device Manager name (Windows OS)
///Enable this peripheral and get a handleWrap pointer via eud_initialize_device_swd
#define EUD_SWD_DEVMGR_NAME "\\\\.\\Qualcomm EUD SWD Device 9504 (0004)\\DEBUG"
#define EUD_SWD_DEVMGR_BASENAME "Qualcomm EUD SWD Device 9504"
#define EUD_SWD_DEVMGR_BASESUFFIX "\\DEBUG"

///COM Peripheral Device Manager name (Windows OS)
///Enable this peripheral and get a handleWrap pointer via eud_initialize_device_com
//#define EUD_COM_DEVMGR_NAME "\\\\.\\Qualcomm EUD COM Device 9505\\DEBUG"
//#define EUD_COM_DEVMGR_NAME "\\\\.\\%QcomDevice9505%\\DEBUG"
#define EUD_COM_DEVMGR_NAME "\\\\.\\COM10"  //<FIXME - this needs to be a passable variable
#define EUD_COM_DEVMGR_BASENAME "COM10"  //<FIXME - this needs to be a passable variable
#define EUD_COM_DEVMGR_BASESUFFIX ""  

#define DEVICEMGRMODE_AUTO 0
#define DEVICEMGRMODE_MANUAL 1
//ask for ID or all threads to not ask for an ID
#if defined ( EUD_WIN_ENV )
typedef unsigned __int32  uint32_t;
#endif
extern uint32_t DeviceManagerMode;
extern const char* EUD_Null_devmgr_name;
extern const char* CTL_devicemgrname; 
extern const char* TRC_devicemgrname; 
extern const char* JTG_devicemgrname; 
extern const char* SWD_devicemgrname; 
extern const char* COM_devicemgrname; 

/**************************************//**
*   @brief Configures Device Manager Mode. 
*   Currently accepts 0x1 (Manual Select) or 0x0 (Auto Select)
*
*******************************************/ 
//EXPORT EUD_ERR_t SetDeviceManagerMode(uint32_t Mode);

/**************************************//**
*   @brief Sets EUD device ID for the passed eud_handler_p pointer.
*   ID must be 0000 <= 9999 (A four decimal number)
*
*******************************************/ 
//EXPORT EUD_ERR_t set_device_manager_id(handleWrap* eud_handler_p, uint32_t ID);


#endif

