/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   usb.h
*
* Description:                                                              
*   Header file for EUD USB driver 
*
***************************************************************************/
#ifndef USB_H_
#define USB_H_

#include "device_manager_defines.h"
#include "eud_error_defines.h"
#include "debugger_defines.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if defined ( EUD_WIN_ENV )
#include <wtypes.h>
#include <windows.h>
#include <conio.h>
#include <winioctl.h>
#include <libusb-1.0/libusb.h>
#endif

#ifdef  __cplusplus
#include <vector>
#endif

#if defined ( EUD_LNX_ENV )  ||  defined ( EUD_CYG_ENV )
#ifdef  __cplusplus
#include <string>
#endif
#include <libusb-1.0/libusb.h>
#endif


#if defined ( EUD_WIN_ENV )
    #if (_MSC_VER < 1300)
    typedef signed char       int8_t;
    typedef signed short      int16_t;
    typedef signed int        int32_t;
    typedef unsigned char     uint8_t;
    typedef unsigned short    uint16_t;
    typedef unsigned int      uint32_t;
    #else
    typedef signed __int8     int8_t;
    typedef signed __int16    int16_t;
    typedef signed __int32    int32_t;
    typedef unsigned __int8   uint8_t;
    typedef unsigned __int16  uint16_t;
    typedef unsigned __int32  uint32_t;
    #endif
    typedef signed __int64       int64_t;
    typedef unsigned __int64     uint64_t;
#endif


#define EUD_WRITE_ENDPOINT        0x02
#define EUD_READ_ENDPOINT         0x81
#define USB_TIMEOUT               1000

///DeviceID enumerations
#define DEVICETYPE_EUD_NULL       0 ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_CTL        1  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_JTG        2  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_SWD        3  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_TRC        4  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_COM        5  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_MAX        DEVICETYPE_EUD_COM

#define EUD_QCOM_VID 0x05c6

#define EUD_CTL_PID 0x9501
#define EUD_TRC_PID 0x9502
#define EUD_JTG_PID 0x9503
#define EUD_SWD_PID 0x9504
#define EUD_COM_PID 0x9505


//===---------------------------------------------------------------------===//
//
//        Top level defines, typedef's
//
//===---------------------------------------------------------------------===//

#define EMPTY_USB_HANDLE NULL                   ///<Initialization value for USB_Handle

#define EUD_CMD_NOP "\x00"                      ///<All EUD devices take "\x00" as a NOP command.
                                                /// This is therefore used as a test message after
                                                /// opening a USB device
// #define EUD_DEVMGR_MAX_DEVMGR_STRING_LEN (60) ///<Maximum length for Windows Device manager char buffer.

#define DEBUGLEVEL1 1                           ///< For verbose levels. Used by QCEUD_Print. 
                                                /// Disable to restrict QCEUD_Print from printing USB data.
#define DEBUGLEVEL2 1                           ///< Used by QCEUD_Print2 for verbose levels
                                                /// /// Disable to restrict QCEUD_Print2 from printing USB data.

typedef int USB_ERR_t;                          ///<Most USB functions return an EUD error code. These
                                                /// codes map to error messages within eud_error_defines.h

typedef libusb_device * usb_dev_access_type;   //libusb_device
//typedef std::string usb_dev_access_type;  //libusb_device

#ifdef __cplusplus 
typedef struct eud_device_info{
    usb_dev_access_type deviceHandle;   // libusb device
    uint32_t            deviceType;     // DEVICETYPE_EUD_CTL , DEVICETYPE_EUD_JTG  ...
    std::string         devicePath;     // Device bus path
} eud_device_info;
#endif

//===---------------------------------------------------------------------===//
//
//                      Function Definitions
//
//===---------------------------------------------------------------------===//


#ifdef  __cplusplus

#if defined ( EUD_WIN_ENV )
//extern "C" {
/***********************************************************//**
*
* @brief Generic debug print function for USB packet debug
*
***************************************************************/  
VOID _cdecl QCEUD_Print(const char* Format, ...);
/***********************************************************//**
*
* @brief Additional debug print function for USB packet debug
*
***************************************************************/  
VOID _cdecl QCEUD_Print2(const char* Format, ...);
//}
#endif  // #if defined ( EUD_WIN_ENV )

#if defined (EUD_LNX_ENV)
/***********************************************************//**
*
* @brief Generic debug print function for USB packet debug
*
***************************************************************/  
VOID _cdecl QCEUD_Print(PCHAR Format, ...);
/***********************************************************//**
*
* @brief Additional debug print function for USB packet debug
*
***************************************************************/  
VOID _cdecl QCEUD_Print2(PCHAR Format, ...);

#endif  // #if defined (EUD_LNX_ENV)


// API to make use of libusb init 
void libusb_usb_init(void);
// API to perform libusb de-init
void libusb_usb_deinit(void);
// Detection of attached EUD USB devices
std::vector<eud_device_info> *usb_scan_devices(void);



//===---------------------------------------------------------------------===//
//
//                               Classes
//
//===---------------------------------------------------------------------===//


/*******************************************************************//**
*
* @brief USB Device Class. Each EUD Device contains a UsbDevice, 
* by which all USB traffic is routed through. This object is either
* interfaced  with libusb (Linux) or with Qualcomm proprietary Windows 
* USB driver.
*
************************************************************************/  
class UsbDevice
{
    //
    //  Public members
    //
public:

    //////////////////////Member functions/////////////////////////

    UsbDevice();           ///<Constructor
    UsbDevice(bool debug_en); 
    ~UsbDevice();          ///<Destructor

    DWORD bytes_read_ = 0;    ///<Stores number of bytes read during USB operation

    /// General subroutine to initialize and open a USB device. 
    /// Expects that device manager name should be populated in devmgrname
    USB_ERR_t UsbOpen(char* error_code); 

    // API to use if using  the libusb layer underneath
    USB_ERR_t UsbOpen(int* error_code, libusb_device *dev); 

    /// Alternate subroutine to initialize  and open a USB device.
    /// Takes Windows device manager name as argument (devmgrname_local)

    USB_ERR_t UsbOpen(char* error_code, const char* devmgrname_local);

    void UsbClose();       ///< Close USB device. Called within destructor, but can be called explicitly.

    /// Write buffer to USB device.
    USB_ERR_t WriteToDevice(
        PVOID buffer,       ///<Buffer to write to USB device.
        DWORD send_size,     ///<Size to write in bytes.
        DWORD *errcode      ///<Windows rrror code pointer to be populated.
                            /// Note that this is different from the EUD error 
                            /// code that would be returned as USB_ERR_t.
        );
    USB_ERR_t ReadFromDevice(
        PVOID buffer,       ///<Buffer to populate with read bytes.
        DWORD read_size,     ///<Size to read in bytes.
        DWORD *errcode      ///<Windows rrror code pointer to be populated.
                            /// Note that this is different from the EUD error
                            /// code that would be returned as USB_ERR_t.
        );

    /////////////////////Member variables/////////////////////////
#if defined ( EUD_WIN_ENV )    
    HANDLE usb_handle_;         ///<Stores Windows USB Handle
#endif
    
    uint8_t device_type_;         ///<Numerical identifier for what type of device (CTL, JTG, SWD, COM, TRC)

    char* devmgr_dev_name_;       ///<Windows Device Manager name. Required for Qualcomm Windows USB Driver.


    libusb_context *ctx_ = NULL; //a libusb session
    libusb_device * dev_; 
    libusb_device_handle *dev_handle_; //a device handle


    uint32_t usb_device_initialized_;


    ///////////////////////////////////////////////////////////////
    // Private members
    ///////////////////////////////////////////////////////////////
private:
#if defined ( EUD_WIN_ENV )    
    /// Private member variables for Qualcomm Windows USB driver
    OVERLAPPED overlapped_for_write_;      ///< Variable for Qualcomm Windows USB driver 
    OVERLAPPED overlapped_for_read_;       ///< Variable for Qualcomm Windows USB driver 
    
    /// @brief Debug print routine used by Qualcomm Windows USB driver
    static VOID UsbUtilPrintBytes(
        PVOID Buf,      ///<String buffer to print.
        ULONG len,      ///<Length of string buffer (number of char's).
        ULONG PktLen,   ///<Length of packet that printed to USB device successfully.
        char *info      ///<USB device communication info
        );
#endif
    bool debug_;
};
#else    // __cplusplus
typedef void  UsbDevice;
#endif   // __cplusplus

#endif //USB_H_
