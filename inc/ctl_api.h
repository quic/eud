/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   ctl_api.h
*
* Description:                                                              
*   Header file for EUD CTL Peripheral public APIs declerations and 
*   various header information
*
***************************************************************************/
#ifndef CTL_API_H_
#define CTL_API_H_

#include "ctl_eud.h"

/**************************************//**
*
*   @brief Gives major and minor spin numbers. 
*
******************************************/
EXPORT EUD_ERR_t            ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
eud_get_version(
    uint32_t* major_rev_id, ///<Major version number
    uint32_t* minor_rev_id,  ///<Minor version number
    uint32_t* spin_rev_id  ///<Spin version number
);

/**************************************//**
* @brief Populates given pointer with word output 
* of EUD CTL status register.
*  
*  [00] - CTL_CTL_RCTLR_SRST_N 
*  [01] - CTL_EDL_REQ          
*  [02] - CTL_SWD_PERIPH_EN    
*  [03] - CTL_TRACE_PERIPH_EN  
*  [04] - CTL_JTAG_PERIPH_EN   
*  [05] - CTL_COM_PERIPH_EN    
*  [06] - CTL_VDDMIN_TCK_EN    
*  [07] - CTL_GPIO_JTAG_SEL    
*  [08] - CTL_GPIO_SWD_SEL     
*  [09] - CTL_MTAP_MUX_SEL     
*  [10] - CTL_DAP_MUX_SEL      
*  [11] - CTL_ACC_TAP_SEL      
*  [12] - CTL_VBUS_ATTACH      
*  [13] - CTL_VBUS_INT         
*  [14] - CTL_CHGR_EN          
*  [15] - CTL_CHGR_INT         
*  [16] - CTL_HOST_EUD_DIS     
*  [17] - CTL_ACC_TDO_SEL      
*  [18] - CTL_DAP_TDO_SEL      
*  [19] - CTL_MTAP_TDO_SEL     
*  [20] - CTL_DAP_SWDO_SEL     
*  CTL bits[31:21] - Reserved
*
*******************************************/   
EXPORT EUD_ERR_t            ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
eud_get_ctl_status(
    uint32_t device_id,      ///<32bit device identifier. A list of attached devices can be retrived from get_device_id_array()
    uint32_t* ctl_status    ///<Pointer to be populated with ctl status. See documentation or use eud_get_ctl_status_string for bit fields.
    );


/**************************************//**
* @brief Populates given buffer with string representation
* of EUD CTL status register.
*******************************************/   
EXPORT EUD_ERR_t            ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
eud_get_ctl_status_string(
    uint32_t device_id,      ///<32bit device identifier. A list of attached devices can be retrived from get_device_id_array()
    char* string_buffer,     ///<Buffer to be populated with ASCII characters representing control register status.
    uint32_t* string_size_p  ///<Pointer populated with size of string. Not larger than 1024bytes
    );

/**************************************//**
* @brief Populates given buffer EUD Device ID
*******************************************/   
/*
EXPORT EUD_ERR_t ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
EUDGetDeviceID(
    uint32_t* device_id///<Pointer populated with size of string. Not larger than 1024bytes
    );
    */
/**************************************//**
* @brief Initiates reset signal to msm Reset Ctlr (eud_rctlr_srst_n).
*        The Reset Ctlr does not reset EUD when EUD initiates a 
*        system reset (USB connection to EUD won't be lost across 
*        MSM reset).
*******************************************/   
EXPORT EUD_ERR_t            ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
eud_msm_reset(
    uint32_t device_id,      ///<32bit device identifier. A list of attached devices can be retrived from get_device_id_array()
    uint32_t reset_time_ms   ///<Reset delay betweem assert and deassert reset in milliseconds.
    );


/**************************************//**
* @brief Deasserts eud_rctlr_srst_n bit, to allow MSM to come 
*        out of reset, assuming eud_msm_assert_reset was called prior.
*        The Reset Ctlr does not reset EUD when EUD initiates 
*        a system reset (USB connection to EUD won't be lost 
*        across MSM reset).
*
*******************************************/   
EXPORT EUD_ERR_t        ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
eud_msm_deassert_reset(
    uint32_t device_id   ///<32bit device identifier. A list of attached devices can be retrived from get_device_id_array()
    );



/**************************************//**
* @brief Asserts eud_rctlr_srst_n bit, to hold MSM in reset until
*        eud_msm_deassert_reset is called.
*        The Reset Ctlr does not reset EUD when EUD initiates a 
*        system reset (USB connection to EUD won't be lost 
*        across MSM reset).
*******************************************/   
EXPORT EUD_ERR_t        ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
eud_msm_assert_reset(
    uint32_t device_id   ///<32bit device identifier. A list of attached devices can be retrived from get_device_id_array()
    );

/**************************************//**
* @brief Calls eud_disable_charger, which updates chgr_en and sends 
*        a chgr_int for chip software to respond to disable 
*        charge update.
*******************************************/   
EXPORT EUD_ERR_t        ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
eud_disable_charger(
    uint32_t device_id   ///<32bit device identifier. A list of attached devices can be retrived from get_device_id_array()
    );



/**************************************//**
* @brief Updates chgr_en and sends a chgr_int
*        for chip software to respond to enable 
*        charge update.
*******************************************/   
EXPORT EUD_ERR_t        ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
eud_enable_charger(
    uint32_t device_id   ///<32bit device identifier. A list of attached devices can be retrived from get_device_id_array()
    );


/**************************************//**
* @brief Updates vbus_attach and sends
*        a vbus_int for chip software to respond 
*        and connect USB to EUD/Host PC.
*******************************************/   
EXPORT EUD_ERR_t        ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
eud_spoof_attach(
    uint32_t device_id   ///<32bit device identifier. A list of attached devices can be retrived from get_device_id_array()
    );




/**************************************//**
* @brief Updates vbus_attach and sends
*        a vbus_int for chip software to respond 
*        and disconnect USB to EUD/Host PC.
*        EUD control peripheral shouldn't lose
*        connection to host PC.
*******************************************/   
EXPORT EUD_ERR_t        ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
eud_spoof_detach(
    uint32_t device_id   ///<32bit device identifier. A list of attached devices can be retrived from get_device_id_array()
    );



//These are the same as SpoofAttach/SpoofDetach

/**************************************//**
* @brief Calls eud_spoof_attach, which updates vbus_attach 
*        and sends a vbus_int for chip software to respond 
*        and connect USB to EUD/Host PC.
*******************************************/   
EXPORT EUD_ERR_t        ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
eud_connect_usb(
    uint32_t device_id   ///<32bit device identifier. A list of attached devices can be retrived from get_device_id_array()
    );



/**************************************//**
* @brief Calls eud_spoof_detach, which updates vbus_attach 
*        and sends a vbus_int for chip software to respond 
*        and disconnect USB from EUD/Host PC.
*******************************************/   
EXPORT EUD_ERR_t        ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
eud_disconnect_usb(
    uint32_t device_id   ///<32bit device identifier. A list of attached devices can be retrived from get_device_id_array()
    );




#endif

