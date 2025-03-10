/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   ctl_api.cpp
*
* Description:                                                              
*   CPP source file for CTL EUD Peripheral's public API's
*
***************************************************************************/

#define EUD_DEVICE_DECL 1
#define SWD_CMD_RETRY_VALE 0xfff

#include "eud.h"
#include "ctl_eud.h"
#include "device_manager.h"
#include "eud_api.h"
#include "ctl_api.h"
#include "jtag_api.h"
#include "swd_api.h"

#define ENABLE_LIME 0

#if ENABLE_LIME
extern bool limeTelematicsCheck(const uint32_t&, uint64_t&);
#endif

EXPORT EUD_ERR_t eud_get_version(uint32_t *major_rev_id, uint32_t *minor_rev_id, uint32_t *spin_rev_id)
{
    if ((major_rev_id == NULL) || (minor_rev_id == NULL) || (spin_rev_id == NULL))
    {
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }
    *major_rev_id = MAJOR_REV_ID;
    *minor_rev_id = MINOR_REV_ID;
    *spin_rev_id = SPIN_REV_ID;
    
    return EUD_SUCCESS;

}
EXPORT EUD_ERR_t eud_get_ctl_status_string(uint32_t deviceID, char* stringbuffer, uint32_t* stringsize_p){
    EUD_ERR_t err;

    if ((stringsize_p == NULL) || (stringbuffer == NULL)){
        return eud_set_last_error(EUD_ERR_NULL_POINTER);
    }
    //Initialize buffer on stack, to be copied to and cleaned up after function call.
    


    /////////////Get CTL status register////////////

    uint32_t* ctlstatus_p = new uint32_t;
    if ( (err = eud_get_ctl_status(deviceID, ctlstatus_p)) != 0){
        delete ctlstatus_p;
        return err;
    }
    
    
    uint32_t regstringlength = (uint32_t)strlen(CTL_REG_STRING_ARRAY[0]);
    uint32_t valuelength = (uint32_t)strlen(":0\n");
    uint32_t pointerindex = 0;
    
    uint32_t ctl_bit_value = 0;

    for (uint32_t i = 0; i < CTL_STATUS_REG_SIZE; i++){
        
        //Get 1 or 0 from ctl register
        ctl_bit_value = (1 << i)&(*ctlstatus_p)?1:0;

        sprintf_s(stringbuffer + pointerindex, regstringlength+1, CTL_REG_STRING_ARRAY[i]);
        pointerindex += regstringlength - 1; //overwrite the '\0'
        sprintf_s(stringbuffer + pointerindex, valuelength+1,    ":%d\n", ctl_bit_value);
        pointerindex += valuelength;
    }
    //Append terminating character;
    //sprintf_s(stringbuffer + pointerindex, valuelength, '\0');
    *(stringbuffer + pointerindex + 1) = 0;


    *stringsize_p = (pointerindex + 1);
    delete ctlstatus_p;
    return err;
}


//////////////////////////////////////////////////
//
//   Function: eud_get_ctl_status
//   Description:   Queries hardware for CTL status register
//                  and returns its numeric value 
//
//   Arguments:
//          uint32_t* ctl_status -  pointer to which function will copy
//                                  ctl status register numeric data.
//
//   Returns:   error - EUD_ERR_t type, errorcode
//             See eud_get_error_string for error definition.
//
//   Scope: External function (Externally callable)
//   
//
//////////////////////////////////////////////////
EXPORT EUD_ERR_t eud_get_ctl_status(uint32_t deviceID, uint32_t* ctl_status){
    EUD_ERR_t err;
    
    CtlEudDevice* ctl_handle_p = eud_initialize_device_ctl(deviceID, EUD_CTL_INIT_NO_OPTIONS_VALUE, &err);
    if (ctl_handle_p == NULL) return eud_get_last_error();

    uint8_t ctl_status_8bit[CTL_USB_READ_SIZE];
    memset(ctl_status_8bit, 0, CTL_USB_READ_SIZE);

    err = ctl_handle_p->WriteCommand(CTL_CMD_CTLIN_READ, ctl_status);

    eud_close_peripheral((PVOID*)ctl_handle_p);

    return err;
    
}

//////////////////////////////////////////////////
//
//   Function: EUDGetDeviceID
//   Description:   Gets DeviceID from first available EUD device
//
//   Arguments:
//          uint32_t* deviceID -    pointer to which function will copy
//                                  deviceID data.
//
//   Returns:   error - EUD_ERR_t type, errorcode
//              See eud_get_error_string for error definition.
//
//   Scope: External function (Externally callable)
//   
//
//////////////////////////////////////////////////

/*
EXPORT EUD_ERR_t EUDGetDeviceID(uint32_t* deviceID){

    EUD_ERR_t err;

    CtlEudDevice* ctl_handle_p = eud_initialize_device_ctl(EUD_CTL_INIT_NO_OPTIONS_VALUE, &err);
    if (ctl_handle_p == NULL) return eud_get_last_error();

    err = ctl_handle_p->WriteCommand(CTL_CMD_DEVICE_ID_READ, deviceID);

    eud_close_peripheral((PVOID*)ctl_handle_p);

    return err;

}
*/

//////////////////////////////////////////////////
//
//   Function: eud_msm_assert_reset
//   Description:   Initiates reset signal to msm Reset Ctlr (eud_rctlr_srst_n).
//                  The Reset Ctlr does not reset EUD when EUD initiates a system reset
//                  (USB connection to EUD won't be lost across MSM reset).
//
//   Arguments:     None
//
//   Returns:   error - EUD_ERR_t type, errorcode
//              See eud_get_error_string for error definition.
//
//   Scope: External function (Externally callable)
//   
//
//////////////////////////////////////////////////

EXPORT EUD_ERR_t eud_msm_assert_reset(uint32_t deviceID){

    EUD_ERR_t err;

    CtlEudDevice* ctl_handle_p = eud_initialize_device_ctl(deviceID,EUD_CTL_INIT_NO_OPTIONS_VALUE, &err);
    if (ctl_handle_p == NULL) return eud_get_last_error();

    //It's an nbit (NotBit), so clearing it will assert reset
    uint32_t payload = 1 << CTL_CTL_RCTLR_SRST_N_SHFT;
    
    err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_CLR, payload);
    
    eud_close_peripheral((PVOID*)ctl_handle_p);

    return err;

}

//////////////////////////////////////////////////
//
//   Function: eud_spoof_attach / eud_connect_usb
//
//   Description:   Set vbus_attach to 1, then assert vbus_int 
//                  For chip software to service updated vbus_attach,
//                  which should result in MSM USB being attached through
//                  EUD to host PC
//
//  Arguments: None
//
//  Returns:   error - EUD_ERR_t type, errorcode
//            See eud_get_error_string for error definition.
//
//   Scope: External function (Externally callable)
//
//   Additional Notes:
//          Note that this will work on full silicon but not on FPGA emulation//   
//
//////////////////////////////////////////////////
EXPORT EUD_ERR_t eud_connect_usb(uint32_t deviceID){
    return eud_spoof_attach(deviceID);
}
EXPORT EUD_ERR_t eud_spoof_attach(uint32_t deviceID){

    EUD_ERR_t err;

    CtlEudDevice* ctl_handle_p = eud_initialize_device_ctl(deviceID, EUD_CTL_INIT_NO_OPTIONS_VALUE, &err);
    if (ctl_handle_p == NULL) return eud_get_last_error();

    uint32_t payload;
    
    payload = 1 << CTL_VBUS_ATTACH_SHFT;
    
    if ( (err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_SET, payload)) != EUD_SUCCESS ) {
        eud_close_peripheral((PVOID*)ctl_handle_p);
        return err;
    }
    
    //Now call VBUS_INT to send interrupt to 
    payload = 1 << CTL_VBUS_INT_SHFT;

    if ( (err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_SET, payload)) != EUD_SUCCESS ) {
        eud_close_peripheral((PVOID*)ctl_handle_p);
        return err;
    }

#ifndef HANA
    Sleep(10);
    //Workaround for Napali - clear interrupt after setting it.
    if (err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_CLR, payload)) {
        eud_close_peripheral((PVOID*)ctl_handle_p);
        return err;
    }

    err = eud_close_peripheral((PVOID*)ctl_handle_p);

    return err;
    

}

    //For Hana - will do polling
#else

    bool pollresult;
    pollresult = poll_on_ctol_bit(ctl_handle_p, (1 << CTL_VBUS_INT_SHFT), CTL_MAX_VBUS_INT_TMOUT, &err);

    eud_close_peripheral((PVOID*)ctl_handle_p);

    if (pollresult == true){
        return EUD_SUCCESS;
    }
    else if (err != EUD_SUCCESS){
        return err;
    }
    else{
        return eud_set_last_error(CTL_EUD_ERR_CHGR_INT_NOT_CLEARED);
    }

}
#endif



//////////////////////////////////////////////////
//
//   Function:      eud_spoof_detach / eud_disconnect_usb
//
//   Description:   Set vbus_attach to 0, then assert vbus_int 
//                  For chip software to service updated vbus_attach,
//                  which should result in MSM USB disconnection from
//                  EUD, and thereby disconnection from host.
//
//   Arguments: None
//
//   Returns:   error - EUD_ERR_t type, errorcode
//              See eud_get_error_string for error definition.
//
//   Scope: External function (Externally callable)
//   
//   Additional Notes:
//          Note that this will work on full silicon but not on FPGA emulation
//
//////////////////////////////////////////////////

EXPORT EUD_ERR_t eud_disconnect_usb(uint32_t deviceID){
    return eud_spoof_detach(deviceID);
}
EXPORT EUD_ERR_t eud_spoof_detach(uint32_t deviceID){

    EUD_ERR_t err;

    CtlEudDevice* ctl_handle_p = eud_initialize_device_ctl(deviceID, EUD_CTL_INIT_NO_OPTIONS_VALUE, &err);
    if (ctl_handle_p == NULL) return eud_get_last_error();

    uint32_t payload;

    payload = 1 << CTL_VBUS_ATTACH_SHFT;

    if ( (err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_CLR, payload)) != EUD_SUCCESS ) {
        eud_close_peripheral((PVOID*)ctl_handle_p);
        return err;
    }

    //Now call VBUS_INT to send interrupt to 
    payload = 1 << CTL_VBUS_INT_SHFT;

    if ( (err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_SET, payload)) != EUD_SUCCESS) {
        eud_close_peripheral((PVOID*)ctl_handle_p);
        return err;
    }

#ifndef HANA
    Sleep(10);

    //Workaround for Napali - clear interrupt after setting it.
    if (err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_CLR, payload)) {
        eud_close_peripheral((PVOID*)ctl_handle_p);
        return err;
    }

    return err;
}
//For Hana - will do polling
#else


    bool pollresult;
    pollresult = poll_on_ctol_bit(ctl_handle_p, (1 << CTL_VBUS_INT_SHFT), CTL_MAX_VBUS_INT_TMOUT, &err);

    eud_close_peripheral((PVOID*)ctl_handle_p);

    if (pollresult == true){
        return EUD_SUCCESS;
    }
    else if (err != EUD_SUCCESS){
        return err;
    }
    else{
        return eud_set_last_error(CTL_EUD_ERR_CHGR_INT_NOT_CLEARED);
    }
}
#endif


//////////////////////////////////////////////////
//
//   Function: eud_enable_charger
//   Description:   Set vbus_attach to 1, then assert vbus_int 
//                  For chip software to service updated vbus_attach,
//                  which should result in enabling charging
//
//   Arguments: None
//
//   Returns:   error - EUD_ERR_t type, errorcode
//              See eud_get_error_string for error definition.
//
//   Scope: External function (Externally callable)
//
//   Additional Notes:
//          Note that this will work on full silicon but not on FPGA emulation
//
//////////////////////////////////////////////////
EXPORT EUD_ERR_t eud_enable_charger(uint32_t deviceID){

    EUD_ERR_t err;

    CtlEudDevice* ctl_handle_p = eud_initialize_device_ctl(deviceID, EUD_CTL_INIT_NO_OPTIONS_VALUE, &err);
    if (ctl_handle_p == NULL) return eud_get_last_error();


    //It's an n-bit, so clearing it will assert reset
    uint32_t payload;

    payload = 1 << CTL_CHGR_EN_SHFT;

    if ( (err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_SET, payload)) != EUD_SUCCESS ) {
        eud_close_peripheral((PVOID*)ctl_handle_p);
        return err;
    }


    //Now call VBUS_INT to send interrupt to 
    payload = 1 << CTL_CHGR_INT_SHFT;

    if ( (err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_SET, payload)) != EUD_SUCCESS ) {
        eud_close_peripheral((PVOID*)ctl_handle_p);
        return err;
    }

#ifndef HANA
    Sleep(10);
    //Workaround for Napali - clear interrupt after setting it.

    err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_CLR, payload);

    eud_close_peripheral((PVOID*)ctl_handle_p);

    return err;

}
//For Hana - will do polling
#else
    //Now poll on vbus_int. 
    
    bool pollresult;
    pollresult = poll_on_ctol_bit(ctl_handle_p, 1 << CTL_CHGR_INT_SHFT, CTL_MAX_VBUS_INT_TMOUT, &err);

    eud_close_peripheral((PVOID*)ctl_handle_p);

    if (pollresult == true){
        return EUD_SUCCESS;
    }
    else if (err != EUD_SUCCESS){
        return err;
    }
    else{
        return eud_set_last_error(CTL_EUD_ERR_CHGR_INT_NOT_CLEARED);
    }

}
#endif //HANA



//////////////////////////////////////////////////
//
//   Function: eud_disable_charger
//   Description:   Set vbus_attach to 0, then assert vbus_int 
//                  For chip software to service updated vbus_attach,
//                  which should result in enabling charging
//
//   Arguments: None
//
//   Returns:   error - EUD_ERR_t type, errorcode
//              See eud_get_error_string for error definition.
//
//   Scope: External function (Externally callable)
//   
//   Additional Notes:
//          Note that this will work on full silicon but not on FPGA emulation
//
//////////////////////////////////////////////////
EXPORT EUD_ERR_t eud_disable_charger(uint32_t deviceID){

    EUD_ERR_t err;

    CtlEudDevice* ctl_handle_p = eud_initialize_device_ctl(deviceID, EUD_CTL_INIT_NO_OPTIONS_VALUE, &err);
    if (ctl_handle_p == NULL) return eud_get_last_error();


    //It's an n-bit, so clearing it will assert reset
    uint32_t payload;
    payload = 1 << CTL_CHGR_EN_SHFT;

    if ( (err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_CLR, payload)) != EUD_SUCCESS ) {
        eud_close_peripheral((PVOID*)ctl_handle_p);
        return err;
    }

    //Now call VBUS_INT to send interrupt to 
    payload = 1 << CTL_CHGR_INT_SHFT;

    if ( (err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_SET, payload)) != EUD_SUCCESS ) {
        eud_close_peripheral((PVOID*)ctl_handle_p);
        return err;
    }

#ifndef HANA
    Sleep(10);
    //Workaround for Napali - clear interrupt after setting it.
    if (err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_CLR, payload)) {
        eud_close_peripheral((PVOID*)ctl_handle_p);
        return err;
    }

    eud_close_peripheral((PVOID*)ctl_handle_p);
    return err;


}
//For Hana - will do polling
#else

    //Now poll on vbus_int. 
    
    bool pollresult;
    pollresult = poll_on_ctol_bit(ctl_handle_p, 1 << CTL_CHGR_INT_SHFT, CTL_MAX_VBUS_INT_TMOUT, &err);

    eud_close_peripheral((PVOID*)ctl_handle_p);

    if (pollresult == true){
        return EUD_SUCCESS;
    }
    else if (err != EUD_SUCCESS){
        return err;
    }
    else{
        return eud_set_last_error(CTL_EUD_ERR_CHGR_INT_NOT_CLEARED);
    }

}
#endif //HANA

//////////////////////////////////////////////////
//
//   Function: eud_msm_deassert_reset
//   Description:   Initiates reset signal to msm Reset Ctlr (eud_rctlr_srst_n).
//                  The Reset Ctlr does not reset EUD when EUD initiates a system reset
//                  (USB connection to EUD won't be lost across MSM reset).
//   Arguments: 
//                  resettime_ms - int32_t type - time in milliseconds.
//                  Cannot be greater than MAX_EUDMSMRESET_TIMEMS.
//
//   Returns:   error - EUD_ERR_t type, errorcode
//              See eud_get_error_string for error definition.
//
//   Scope: Local function (Not externally callable)
//   
//
//////////////////////////////////////////////////

EXPORT EUD_ERR_t eud_msm_deassert_reset(uint32_t deviceID){
    
    EUD_ERR_t err;

    CtlEudDevice* ctl_handle_p = eud_initialize_device_ctl(deviceID, EUD_CTL_INIT_NO_OPTIONS_VALUE, &err);
    if (ctl_handle_p == NULL) return eud_get_last_error();

    err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_SET, (1 << CTL_CTL_RCTLR_SRST_N_SHFT));

    eud_close_peripheral((PVOID*)ctl_handle_p);
    
    return err;

}
//////////////////////////////////////////////////
//
//   Function: eud_msm_reset
//   Description:   Initiates reset signal to msm Reset Ctlr (eud_rctlr_srst_n).
//                  The Reset Ctlr does not reset EUD when EUD initiates a system reset
//                  (USB connection to EUD won't be lost across MSM reset).
//   Arguments: 
//                  resettime_ms - int32_t type - time in milliseconds.
//                  Cannot be greater than MAX_EUDMSMRESET_TIMEMS.
//
//   Returns:   error - EUD_ERR_t type, errorcode
//              See eud_get_error_string for error definition.
//
//   Scope: Local function (Not externally callable)
//   
//
//////////////////////////////////////////////////

EXPORT EUD_ERR_t eud_msm_reset(uint32_t deviceID, uint32_t resettime_ms){

    EUD_ERR_t err;

    CtlEudDevice* ctl_handle_p = eud_initialize_device_ctl(deviceID, EUD_CTL_INIT_NO_OPTIONS_VALUE, &err);
    if (ctl_handle_p == NULL) return eud_get_last_error();


    if (resettime_ms > MAX_EUDMSMRESET_TIME_MS) return eud_set_last_error(EUD_ERR_CTL_GIVEN_RSTDELAY_TOO_HIGH);
    
    //It's an n-bit, so clearing it will assert reset
    if ( (err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_CLR, (1 << CTL_CTL_RCTLR_SRST_N_SHFT))) !=EUD_SUCCESS ) {
        eud_close_peripheral((PVOID*)ctl_handle_p);
        return err;
    }
    
    Sleep(resettime_ms);

    if ( (err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_SET, (1 << CTL_CTL_RCTLR_SRST_N_SHFT))) !=EUD_SUCCESS ) {
        eud_close_peripheral((PVOID*)ctl_handle_p);
        return err;
    }

    eud_close_peripheral((PVOID*)ctl_handle_p);
    return err;

}



//===---------------------------------------------------------------------===//
//
//Reinitialize Ctl peripheral
//
//===---------------------------------------------------------------------===//

EXPORT EUD_ERR_t reinitialize_ctl(uint32_t deviceID, CtlEudDevice* ctl_handle_p) {
    
    if (ctl_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    
    return ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_WRITE, CTL_PAYLOAD_REINIT_CTL);
    
}



//===---------------------------------------------------------------------===//
//
//Peripheral Toggles
//
//===---------------------------------------------------------------------===//
//EXPORT EUD_ERR_t CTL_INIT_BUF(handleWrap* rvalue_ctl_handler_p) {
EXPORT CtlEudDevice* eud_initialize_device_ctl(uint32_t deviceID, uint32_t options, EUD_ERR_t* errorcode_p) {
    
    //
    //  Check Parameters
    //
    if (errorcode_p == NULL){
        eud_set_last_error(EUD_ERR_BAD_PARAMETER);
        return  NULL;
    }

    if ((options & (~INIT_CTL_ALLOWED_OPTIONS_MSK)) != 0){
        *errorcode_p = eud_set_last_error(EUD_ERR_BAD_PARAMETER);
        return 0;
    }

    //
    //  Init and access Control Peripheral.
    //  We want ctl device to remain until we explicitly delete it.
    //    
    CtlEudDevice* ctl_handle_p = get_ctl_peripheral_by_device_id(deviceID, errorcode_p);
    if (ctl_handle_p == NULL){
        delete ctl_handle_p;
        return NULL;
    }

    
    return ctl_handle_p;
}


EXPORT JtagEudDevice* eud_initialize_device_jtag(uint32_t deviceID, uint32_t options, EUD_ERR_t * errcode) {
    
    //
    //  Check parameters
    //
    if ((options & (~INIT_JTG_ALLOWED_OPTIONS_MSK)) != 0){
        *errcode = eud_set_last_error(EUD_ERR_BAD_PARAMETER);
        return 0;
    }

    //
    //  Initialize JTG device
    //
    JtagEudDevice* jtg_handle = (JtagEudDevice*)initialize_usb_device(deviceID, DEVICETYPE_EUD_JTG, errcode);

    
    if ((jtg_handle == NULL) || (*errcode != EUD_SUCCESS)){
        QCEUD_Print("Error during jtag eud device init. Err code: %d\n", *errcode);
        return NULL;
    }
    //If limited initialization specified, simply return SWD handler.
    if (options & (JTG_LIMITED_INIT_OPTION)){
        return jtg_handle;
    }


    //
    //  Go through JTG initialization sequence.
    //  If we're here, we're attached to JTG USB device
    //

    if ( (*errcode = jtag_set_frequency(jtg_handle, EUD_JTAG_FREQ_0_117_MHz)) != EUD_SUCCESS ){
        eud_close_peripheral((PVOID*)jtg_handle);
        return NULL;
    }

    if ( (*errcode = jtag_set_delay(jtg_handle, JTG_EUD_DELAY_VALUE)) != EUD_SUCCESS ){
        eud_close_peripheral((PVOID*)jtg_handle);
        return NULL;
    }
    
    //Get JTAG ID to verify device connected.
    uint32_t* jtag_ID = new uint32_t;
    *jtag_ID = 0;
    if ( (*errcode = jtag_get_jtag_id(jtg_handle, jtag_ID)) != EUD_SUCCESS ){
        eud_close_peripheral((PVOID*)jtg_handle);
        return NULL;
    }

    if (*jtag_ID == 0x0){
        *errcode = EUD_ERR_JTG_DEVICE_NOT_RESPONSIVE;
        eud_close_peripheral((PVOID*)jtg_handle);
        return NULL;
    }

    //Return the initialized peripheral handle
    return jtg_handle;
}

#define MAX_ERR_STRING_SIZE 200



EXPORT SwdEudDevice *eud_initialize_device_swd(uint32_t deviceID, uint32_t options, EUD_ERR_t *errcode)
{

    //
    //  Check parameters
    //

    // if ((options & (~INIT_SWD_ALLOWED_OPTIONS_MSK)) != 0){
    //     *errcode = eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    //     return NULL;
    // }
    //
    //  Initialize SWD device
    //
    SwdEudDevice* swd_handle = (SwdEudDevice*)initialize_usb_device(deviceID, DEVICETYPE_EUD_SWD, errcode);
    
    if (swd_handle == NULL){
        QCEUD_Print("Error during SWD Init. Going to force turn off EUD SWD device");
        //Error case. Try and shut down swd peripheral and reinitialize.
        *errcode = force_off_usb_device(deviceID, DEVICETYPE_EUD_SWD);
        QCEUD_Print("Attempting to Initialize SWD device again.");
        swd_handle = (SwdEudDevice*)initialize_usb_device(deviceID, DEVICETYPE_EUD_SWD, errcode);



        if (swd_handle == NULL) {
            QCEUD_Print("SWD Init failed. Handler NULL.");
            //Error case - handle not initialized
            return NULL;
        }
    }
    // If limited initialization specified, simply return SWD handler. This is the typical use case.
    // if (options & (SWD_LIMITED_INIT_OPTION))
    // {
    //     return swd_handle;
    // }

    //
    //  Go through SWD initialization sequence.
    //  If we're here, we're attached to SWD USB device
    //
    uint32_t swd_freq = EUD_SWD_FREQ_7_5_MHz;

    QCEUD_Print("Attempting SWD Peripheral Reset.");
    if ((*errcode = swd_peripheral_reset(swd_handle)) != EUD_SUCCESS ) {
        QCEUD_Print("Encountered error during SWD Peripheral Reset");
        return NULL;
    }

    QCEUD_Print("Attempting SWD Frequency Set.");
    if ((*errcode = swd_set_frequency(swd_handle, swd_freq)) != EUD_SUCCESS ){
        QCEUD_Print("Encountered error during SWD Frequency set.");
        eud_close_peripheral((PVOID*)swd_handle);
        return NULL;
    }
    

    if ((*errcode = swd_set_delay(swd_handle, SWD_EUD_DELAY_VALUE)) != EUD_SUCCESS ){
        QCEUD_Print("Encountered error during SWD Delay set.");
        eud_close_peripheral((PVOID*)swd_handle);
        return NULL;
    }

    swd_handle->WriteCommand(SWD_CMD_TIMING, SWD_CMD_RETRY_VALE);
    
    if ((*errcode = jtag_to_swd(swd_handle)) != EUD_SUCCESS ){
        eud_close_peripheral((PVOID*)swd_handle);
        return NULL;
    }

    uint32_t* jtagID = new uint32_t;


    if ((*errcode = swd_get_jtag_id(swd_handle, jtagID)) != EUD_SUCCESS ) {
        eud_close_peripheral((PVOID*)swd_handle);
        delete jtagID;
        return NULL;
    }
    
    //
    //  Check if we got any value from target.
    //
    if (*jtagID == 0){
        QCEUD_Print("IDCODE read failed\n");
        eud_close_peripheral((PVOID*)swd_handle);
        eud_set_last_error(EUD_SWD_ERR_NO_JTAGID);
        delete jtagID;
        return NULL;
    }

    //Cleanup and return
    delete jtagID;

#if ENABLE_LIME
    uint64_t limeErrcode = 0;
    if (false == limeTelematicsCheck(options,limeErrcode))
    {
        QCEUD_Print( "Failed to initialize, license invalid! %x ", limeErrcode );
        return NULL;
    }
#endif

    return swd_handle;
}

EXPORT TraceEudDevice* eud_initialize_device_trace(uint32_t deviceID, uint32_t options, EUD_ERR_t * errcode) {
    
    EUD_ERR_t err;

    //  Check parameters
    if ((options & (~INIT_TRC_ALLOWED_OPTIONS_MSK)) != 0){
        *errcode = eud_set_last_error(EUD_ERR_BAD_PARAMETER);
        return 0;
    }

    TraceEudDevice* trc_handle = (TraceEudDevice*)initialize_usb_device(deviceID, DEVICETYPE_EUD_TRC,&err);

    //Initialize the structure and return it
    return trc_handle;
}

EXPORT ComEudDevice* eud_initialize_device_com(uint32_t deviceID, uint32_t options, EUD_ERR_t * errcode) {
    
    EUD_ERR_t err;

    //  Check parameters
    if ((options & (~INIT_COM_ALLOWED_OPTIONS_MSK)) != 0){
        *errcode = eud_set_last_error(EUD_ERR_BAD_PARAMETER);
        return 0;
    }
    
    ComEudDevice* com_handle = (ComEudDevice*)initialize_usb_device(deviceID, DEVICETYPE_EUD_COM,&err);

    
    //Initialize the structure and return it
    return com_handle;
}






//Test cases:
// pass two 0's -> expect failure
// pass an uninit'd ctl_handler_p -> expect bad cast failure
// pass normal args, no USB attached -> expect a usb failure
// pass normal args, but jtag is taken by another instance -> expect usb concurrency failure
// pass normal args, expect success

PVOID* init_generic_eud_device(uint32_t deviceID, CtlEudDevice* ctl_handle_p, EUD_ERR_t * errcode){
    
    //char* devname = new char[4];
    uint32_t ctl_setbits;
    uint32_t ctl_clearbits;

    //Check parameters
    if (errcode == NULL) {
        eud_set_last_error(EUD_ERR_BAD_PARAMETER);
        return NULL;
    }
    if ((ctl_handle_p == NULL) || (deviceID == DEVICETYPE_EUD_NULL) || (deviceID > DEVICETYPE_EUD_MAX)){
        *errcode = eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }

    ///////////////////////////
    //Get set and clear bits depending on deviceID.
    switch (deviceID){
    case DEVICETYPE_EUD_JTG:
        //devname = "JTG";
        ctl_setbits = CTL_PAYLOAD_JTAGON;
        ctl_clearbits = CTL_BITSCLEAR_JTAGON;
        break;
    case DEVICETYPE_EUD_SWD:
        //devname = "SWD";
        ctl_setbits = CTL_PAYLOAD_SWDON;
        ctl_clearbits = CTL_BITSCLEAR_SWDON;
        break;
    case DEVICETYPE_EUD_COM:
        //devname = "COM";
        ctl_setbits = CTL_PAYLOAD_COMON;
        ctl_clearbits = CTL_BITSCLEAR_COMON;
        break;
    case DEVICETYPE_EUD_TRC:
        //devname = "TRC";
        ctl_setbits = CTL_PAYLOAD_TRCON;
        ctl_clearbits = CTL_BITSCLEAR_TRCON;
        break;
    default:
        return NULL;

    }

    ///////////////////////////////////////
    /// Write to control periph to enable desired peripheral
    *errcode = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_CLR, ctl_clearbits);
    *errcode = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_SET, ctl_setbits);
    
    //Sleep(USB_DEVICE_INIT_WAIT_TIME_MS);
    //*errcode = CTL_OUT_WRITE(ctl_handle_p, ctl_setbits);
    if (*errcode != EUD_SUCCESS){
        return NULL;
    }
    JtagEudDevice* jtg_device_handle;
    SwdEudDevice* swd_device_handle;
    TraceEudDevice* trc_device_handle;
    ComEudDevice* com_device_handle;


    //we want ctl device to remain until we explicitly delete it
    switch (deviceID){
    case DEVICETYPE_EUD_JTG:

        jtg_device_handle = new JtagEudDevice;
        
        *errcode = (EUD_ERR_t)jtg_device_handle->UsbOpen();

        if (*errcode != EUD_SUCCESS){
            //delete jtg_device_handle; //<Don't double delete.
            jtg_device_handle = NULL;
            QCEUD_Print("INIT_%s_EUD_DEVICE error - UsbOpen failed, error code %x\n", "JTG", *errcode);
            return NULL;
        }
        else{
            return (PVOID*)jtg_device_handle;
        }
            
        break;

    case DEVICETYPE_EUD_SWD:

        swd_device_handle = new SwdEudDevice;
        
        if ((*errcode = (EUD_ERR_t)swd_device_handle->UsbOpen()) != EUD_SUCCESS ){

            //delete swd_device_handle; //<Don't double delete
            swd_device_handle = NULL;
            QCEUD_Print("INIT_%s_EUD_DEVICE error - UsbOpen failed, error code %x\n", "SWD", *errcode);
            return NULL;
        }
        else{
            return (PVOID*)swd_device_handle;
        }

        break;

    case DEVICETYPE_EUD_COM:

        com_device_handle = new ComEudDevice;
        
        //*errcode = (EUD_ERR_t)com_device_handle->UsbOpen();    
        if ((*errcode = (EUD_ERR_t)com_device_handle->UsbOpen(EUD_COM_DEVMGR_NAME, com_device_handle->device_type_, 100)) != EUD_SUCCESS ){

            delete com_device_handle;
            com_device_handle = NULL;
            QCEUD_Print("INIT_%s_EUD_DEVICE error - UsbOpen failed, error code %x\n", "COM", *errcode);
            return NULL;
        }
        else{
            return (PVOID*)com_device_handle;
        }
        
        break;

    case DEVICETYPE_EUD_TRC:
        
        trc_device_handle = new TraceEudDevice;

        if ((*errcode = (EUD_ERR_t)trc_device_handle->UsbOpen()) != EUD_SUCCESS ){

            delete trc_device_handle;
            trc_device_handle = NULL;
            QCEUD_Print("INIT_%s_EUD_DEVICE error - UsbOpen failed, error code %x\n", "TRC", *errcode);
            return NULL;
        }
        else{
            return (PVOID*)trc_device_handle;
        }

        break;

    //Should never get here
    default:
        return NULL;

    }
        
    //Should never get here    
    return NULL;
    
}

bool poll_on_ctol_bit(CtlEudDevice* ctl_handle_p, uint32_t maskvalue, uint32_t timeout, EUD_ERR_t* err_p){
    uint32_t timeval = 0;
    uint32_t ctl_reg_value = 0;
    do{

        //Chip software should service the interrupt and clear that bit
        if ((*err_p = ctl_handle_p->WriteCommand(CTL_CMD_CTLIN_READ, 0, &ctl_reg_value)) != EUD_SUCCESS ) {
            eud_close_peripheral((PVOID*)ctl_handle_p);
            return false;
        }

        //Check if bit has gone to 0
        if ((ctl_reg_value & maskvalue) == 0){
            return true;
        }

        Sleep(10);
        timeval += 10;

    } while (timeval < timeout);

    if ((ctl_reg_value & maskvalue) == 0){
        return false;
    }

    return true;

}
