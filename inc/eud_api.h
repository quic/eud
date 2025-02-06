/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   eud_api.h
*
* Description:                                                              
*   EUD parent top level defines for Embedded USB Debugger (EUD) host 
*   software
*
***************************************************************************/
#ifndef EUD_API_H_
#define EUD_API_H_

// #include <string>
#include "eud_error_defines.h"
// #include "device_manager.h"
#include "com_api.h"
#include "swd_api.h"
#include "ctl_api.h"
#include "jtag_api.h"
#include "trc_api.h"

////////////////////////////Top level defines/////////////////////////////

typedef void * PVOID;

#define MANAGEDBUFFERMODE       0 ///<Managed Buffer mode define. Indicates buffering mode for EUD Software. See SetBufMode for more details
#define IMMEDIATEWRITEMODE      1 ///<Immediate Buffer Write mode define. Indicates buffering mode for EUD Software. See SetBufMode for more details
#define MANUALBUFFERWRITEMODE   2 ///<Manual Buffer Write mode define. Indicates buffering mode for EUD Software. See SetBufMode for more details

#define DEVICETYPE_EUD_NULL     0 ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_CTL      1  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_JTG      2  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_SWD      3  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_TRC      4  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_COM      5  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_MAX      DEVICETYPE_EUD_COM

#ifndef EUD_DEVICE_DECL

#ifndef __cpluplus
typedef void EudDevice;
#endif  // __cplusplus

#endif

////////////////////////Peripheral Toggles ////////////////////////
//                                                               //
// These handle turning peripherals on. These return a handle    //
// pointer, 0 if error.                                          //
//                                                               //
///////////////////////////////////////////////////////////////////


/**************************************//**
* @brief Initializes EUD Control peripheral and returns pointer to an initialized handleWrap containing CTL peripheral handle.
*
* Initializing and holding control peripheral is only necessary for changing control
* values, such as charger enable / disable, or spoof attach/detach. For initializing
* a peripheral, control periphal init is not necessary.
*******************************************/
EXPORT CtlEudDevice* ///< Pointer to initialized EUD handler instance. NULL (0) if failure ocurred.
eud_initialize_device_ctl(
    uint32_t device_id,
    uint32_t options,
    EUD_ERR_t * errcode ///< Error code to be populated. 0 for success. Call eud_get_error_string for ASCII description.
    );


/**************************************//**
* @brief Initializes EUD JTAG peripheral and returns pointer to an initialized handleWrap containing JTAG peripheral handle.
*
* This method will initialize a control peripheral, turn on JTAG peripheral, set defaults, close control peripheral
* and return a handle.
*******************************************/
EXPORT JtagEudDevice* ///<Pointer to initialized JTAG EUD handler instance. NULL (0) if failure ocurred.
eud_initialize_device_jtag(
    uint32_t device_id,
    uint32_t options,
    EUD_ERR_t * errcode ///< Error code to be populated. 0 for success. Call eud_get_error_string for ASCII description.
    );


/**************************************//**
* @brief Initializes EUD SWD peripheral and returns pointer to an initialized handleWrap containing SWD peripheral handle.
*
* This method will initialize a control peripheral, turn on SWD peripheral, set defaults, close control peripheral
* and return a handle. Defaults that are set are:
*       Frequency set: swd_set_frequency(handleWrap,EUD_SWD_FREQ_7_5_MHz)
*       Delay value set: swd_set_delay(handleWrap,18)
*       Operations performed:
*           jtag_to_swd(handleWrap) 
*           swd_get_jtag_id(handleWrap) 
*******************************************/
EXPORT SwdEudDevice* ///<Pointer to initialized SWD EUD handler instance. NULL (0) if failure ocurred.
eud_initialize_device_swd(
    uint32_t device_id,
    uint32_t options,
    EUD_ERR_t * errcode///< Error code to be populated. 0 for success. Call eud_get_error_string for ASCII description.
    );


/**************************************//**
* @brief Initializes EUD COM peripheral and returns pointer to an initialized handleWrap containing COM peripheral handle.
*
* This method will initialize a control peripheral, turn on COM peripheral, set defaults, close control peripheral
* and return a handle.
*******************************************/
EXPORT ComEudDevice* ///<Pointer to initialized COM EUD handler instance. NULL (0) if failure ocurred.
eud_initialize_device_com(
    uint32_t device_id,
    uint32_t options,
    EUD_ERR_t * errcode///< Error code to be populated. 0 for success. Call eud_get_error_string for ASCII description.
    );


/**************************************//**
* @brief Initializes EUD TRACE peripheral and returns pointer to an initialized handleWrap containing TRACE peripheral handle.
*
* This method will initialize a control peripheral, turn on TRACE peripheral, set defaults, close control peripheral
* and return a handle.
*******************************************/
EXPORT TraceEudDevice* ///<Pointer to initialized Trace EUD handler instance. NULL (0) if failure ocurred.
eud_initialize_device_trace(
    uint32_t device_id, ///<32bit device identifier. A list of attached devices can be retrived from get_device_id_array()
    uint32_t options,
    EUD_ERR_t * errcode///< Error code to be populated. 0 for success. Call eud_get_error_string for ASCII description.
    );


/// \cond
//don't document. likely to be deprecated

/**************************************//**
*
*
*
*******************************************/
EXPORT EUD_ERR_t ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
reinitialize_ctl(
    uint32_t device_id, ///<32bit device identifier. A list of attached devices can be retrived from get_device_id_array()
    CtlEudDevice* ctl_handle_p
    );
/// \endcond


/**************************************//**
* @Shuts down peripheral contained in passed handleWrap pointer. Clears and empties buffers. 
* This should be called on your peripheral before closing your application
*
*******************************************/
EXPORT EUD_ERR_t ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
eud_close_peripheral(
    PVOID* eud_handle///<Pointer to EUD peripheral instance.
    );


/**************************************//**
*   @brief Sets EUD device ID for the passed eud_handler_p pointer.
*   ID must be 0000 <= 9999 (A four decimal number)
*
*******************************************/ 
EUD_ERR_t 
set_device_manager_id(
    uint32_t device_id,
    uint32_t ID,
    char* device_mgr_name_p,
    uint32_t checkflag
    );

/****************************************************************
*   @brief Execute the reset sequence to begin communication with
           the debug port.
****************************************************************/
EXPORT EUD_ERR_t assert_reset_sequence(EudDevice* eud_handle );

/************************************************************************
*   @brief Execute the trailing portion of the reset sequence to 
           begin communication with the debug port after reset is de-asserted
****************************************************************/
EXPORT EUD_ERR_t deassert_reset_sequence(EudDevice* eud_handle);

//QCOM_SNIP_BEGIN

#if 0
/**************************************//**
*   @brief Sets device manager ASCII string to the given
*   eud_handler_p. If checkflag is set, will attempt to connect
*   to device and return error code if successful/failed.
*
*******************************************/ 
//EUD_ERR_t 
//DeviceManagerIDSetter(
//  handleWrap* eud_handler_p, 
//  uint32_t ID, 
//  uint32_t checkflag
//  );
/**************************************//**
*   @brief Pass a four digit number and receive the 
*   ASCII version of it in a string.
*
*******************************************/ 
std::string
uint32_to_ascii(
    uint32_t number
    );
/**************************************//**
*   @brief Pass a four digit number in ascii form (string)
*   and receive uint32_t version of it.
*
*******************************************/ 
uint32_t 
ascii_to_int(
    std::string asciinum
    );
#endif

/**************************************//**
*   @brief Quick check to see if EUD USB device is present, using given
*   device manager name and device ID.
*
*******************************************/ 
EUD_ERR_t 
check_eud_usb_device(
    const char * device_mgr_name_p, 
    uint32_t device_id
    );

// Agnelo - uncomment if needed to be exported
// EUD_ERR_t DeviceManagerPoller(uint32_t device_id, char * devicemgrname);
// EUD_ERR_t DeviceManagerPoller(uint32_t device_id, uint32_t startvalue, char* devicemgrname_p);

PVOID* init_generic_eud_device(uint32_t device_id, CtlEudDevice* ctl_handle_p, EUD_ERR_t * errcode);


EXPORT EUD_ERR_t get_device_id_array(uint32_t* array_p, uint32_t* length_p);
EXPORT EUD_ERR_t get_attached_devices_string(char* string_buffer, uint32_t* string_size_p);



//QCOM_SNIP_END
#endif //EUD_API_H_