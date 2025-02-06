/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   general_api_processing.cpp
*
* Description:                                                              
*   CPP source file for EUD Jtag general APIs
*
***************************************************************************/

// #include "eud_api.h"
#include "com_eud.h"
#include "ctl_eud.h"
#include "jtag_eud.h"
#include "swd_eud.h"
#include "trc_eud.h"
#include "usb.h"

#include "device_manager.h"

extern "C" CtlEudDevice* eud_initialize_device_ctl (uint32_t deviceID, uint32_t options, EUD_ERR_t * errcode);

// EXPORT EUD_ERR_t eud_close_peripheral(PVOID* handle_p) {
extern "C" EUD_ERR_t eud_close_peripheral(PVOID* handle_p) {
    
    EUD_ERR_t err = EUD_SUCCESS;
    
    if (handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    uint32_t devicetype = ((EudDevice*)handle_p)->device_type_;
    uint32_t options = 0;
    if (devicetype == DEVICETYPE_EUD_CTL){
        delete ((EudDevice*)(handle_p));
        return EUD_SUCCESS;
    }
    
    CtlEudDevice* ctl_handle_p = eud_initialize_device_ctl(((EudDevice*)handle_p)->device_id_, options, &err);
    if ((ctl_handle_p == NULL) || (err != EUD_SUCCESS)) {
        QCEUD_Print("Error during ctl_eud device init. Err code: %d\n", err);
        return err;
    }

    if ((err = toggle_peripheral(ctl_handle_p, ((EudDevice*)handle_p)->device_id_, ((EudDevice*)handle_p)->device_type_, DISABLE))!=0) return err;

    delete ((EudDevice*)(handle_p));
    delete ctl_handle_p;

    return err;
}


//===---------------------------------------------------------------------===//
//
// External API functions
//
//===---------------------------------------------------------------------===//

EUD_ERR_t
JTAG_EUD_Bitbang(   JtagEudDevice* jtg_handle_p, 
                    uint32_t tdi, 
                    uint32_t tms, 
                    uint32_t tck, 
                    uint32_t trst, 
                    uint32_t srst, 
                    uint32_t enable){
    

    if (jtg_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    //return JtagEudDevice::Instance()->BitBang(tdi, tms, tck, trst, srst, enable);
    return jtg_handle_p->BitBang(tdi, tms, tck, trst, srst, enable);
}

EUD_ERR_t
JTAG_EUD_Scan(  JtagEudDevice* jtg_handle_p, 
                uint8_t *tms_raw, 
                uint8_t *tdi_raw, 
                uint8_t *tdo_raw, 
                uint32_t scan_length)
{
    

    if (jtg_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    //return JtagEudDevice::Instance()->JtagScan(tms_raw, tdi_raw, tdo_raw, scan_length);
    return jtg_handle_p->JtagScan(tms_raw, tdi_raw, tdo_raw, scan_length);
}


EUD_ERR_t JTAG_CM_Read_Register(    JtagEudDevice* jtg_handle_p, 
                                    uint32_t * reg_addr, 
                                    uint32_t * rd_data){
    

    if (jtg_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    return jtg_handle_p->JtagCmReadRegister(reg_addr, rd_data);

}

EUD_ERR_t JTAG_CM_Write_Register(   JtagEudDevice* jtg_handle_p, 
                                    unsigned * reg_addr, 
                                    uint32_t * reg_data){
    

    if (jtg_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    return jtg_handle_p->JtagCmWriteRegister(reg_addr, reg_data);
}
