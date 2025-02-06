/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   eud_error_defines.cpp
*
* Description:                                                              
*   CPP source file for EUD Error APIs
*
***************************************************************************/

#include "eud_api.h"
#include "eud_error_defines.h"
// #include "ctl_api.h"
#include "usb.h"

// Agnelo - remove these two defines later
#define MAX_EUDMSMRESET_TIME_MS 10000 //Define as 10seconds for now.
#define CTL_MAX_VBUS_INT_TMOUT 5000 //5 seconds

///External define for last known error.
EUD_ERR_t LastError = EUD_ERR_NOT_YET_SET;
EUD_ERR_t LastUSBError = EUD_ERR_NOT_YET_SET;


/////FIXME - static variable not thread safe//////
//////////////////////////////////////////////////
//
//   Function: eud_set_last_error
//   Description:   Sets static variable LastError. 
//                  Usually called from internal EUD
//                  function and returns error code 
//                  to caller.
//                  eud_get_last_error should be called
//                  to properly retrieve error value.
//   Arguments: givenerror - error code
//   Returns:   Returns LastError, which is now set to givenerror.
//   Scope: Local function (Not externally callable)
//   
//
//////////////////////////////////////////////////
EUD_ERR_t eud_set_last_error(EUD_ERR_t givenerror){
    LastError = givenerror;

    return LastError;
}

EUD_ERR_t eud_set_last_usb_error(USB_ERR_t givenerror){
    LastUSBError = givenerror;
    
    return LastUSBError;

}

/////FIXME - static variable not thread safe//////
//////////////////////////////////////////////////
//
//   Function: eud_get_last_error
//   Description:   Returns static variable LastError 
//                  to caller.
//                  Usually called from external process
//                  to find last error called. 
//                  See also eud_get_error_string which
//                  returns string explanation of error code.
//
//   Arguments: None
//   Returns:   Returns LastError. If not set, returns code
//              claiming that no error has been logged yet.
//
//   Scope: External function (Externally callable)
//   
//
//////////////////////////////////////////////////
EXPORT EUD_ERR_t eud_get_last_error(void){
    if (LastError == -1){
        QCEUD_Print("EUD LastError not yet set!\n");
        return EUD_ERR_NOT_YET_SET;
    }

    return LastError;
}
//////////////////////////////////////////////////
//
//   Function: EUDGetLastUSBError
//   Description:   Returns static variable LastUSBError 
//                  to caller.
//                  Usually called from external process
//                  to find last error called. 
//                  See also eud_get_error_string which
//                  returns string explanation of error code.
//
//   Arguments: None
//   Returns:   Returns LastUSBError. If not set, returns code
//              claiming that no error has been logged yet.
//
//   Scope: External function (Externally callable)
//   
//
//////////////////////////////////////////////////

EXPORT EUD_ERR_t EUDGetLastUSBError(void){
    if (LastUSBError == -1){
        QCEUD_Print("EUD LastUSBError not yet set!\n");
        return EUD_ERR_NOT_YET_SET;
    }

    return LastUSBError;
}

//////////////////////////////////////////////////
//
//   Function: eud_get_error_string
//   Description:  Returns string description of given 
//                  error code.
//                  Usually called from external process
//                  to get a description of a reported error code
//
//   Arguments: 
//              errorcode -     EUD_ERR_t type - error code in question
//              stringbuffer -  char* type - buffer to which 
//                              function will copy error code to.
//                              Note that EUD API guarantees that 
//                              copied string will be no more than 100 bytes long
//              stringsize      uint32_t* type - Function will write size of string 
//                              to this location for caller to asses.
//
//   Returns:   Returns errorcode. 0 is success
//
//   Scope: External function (Externally callable)
//   
//
//////////////////////////////////////////////////
EXPORT EUD_ERR_t eud_get_error_string(EUD_ERR_t errorcode, char* stringbuffer, uint32_t* stringsize_p){

    if ((stringsize_p == NULL) || (stringbuffer == NULL)){
        LastError = EUD_ERR_NULL_POINTER;
        return EUD_ERR_NULL_POINTER;
    }
    //Initialize buffer on stack, to be copied to and cleaned up after function call.
    char tempstring[200];

    switch (errorcode){
    case EUD_SUCCESS:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Generic Success - Action returned success.");
        break;
    case EUD_GENERIC_FAILURE:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Generic Error. No detail given.");
        break;
    case EUD_ERR_FUNCTION_NOT_AVAILABLE:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Generic Error. Function not implemented, usually due to hw limitation.");
        break;
    //Handle and parameter errors
    case EUD_ERR_BAD_HANDLE_PARAMETER:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - EUD function received an invalid handle");
        break;
    case EUD_ERR_NULL_POINTER:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - EUD function received null pointer as an argument.");
        break;
    case EUD_ERR_BAD_CAST:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Unable to cast handler's handle object. Is handle initialized?");
        break;
    case EUD_BAD_PARAMETER:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Parameter Error - Bad parameter passed to EUD API.");
        break;
    case EUD_ERR_INCORRECT_ARGUMENTS_WRITE_COMMAND:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Internal Error with WriteCommand usage. Please contact technical support.");
        break;
    case EUD_USB_ERR_HANDLE_UNITIALIZED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD handle error - USB handle passed was not properly initialized.");
        break;
    case EUD_TRC_ERR_HANDLE_ALREADY_INITD:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD handle error - Trace handle already initialized.");
        break;
    case EUD_ERR_INVALID_DIRECTORY_GIVEN:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Trace error - Directory passed inaccessible and attempt to create directory was unsuccessful.");
        break;
    case EUD_ERR_BAD_PARAMETER_HANDLE_NOT_POPULATED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD handle error - handeWrap->handle not populated with a  valid EUD handle pointer. Was handle closed and reused?");
        break;
    //EUD Generic err codes
    case EUD_ERR_NOT_YET_SET:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error not yet set: Call ocurred to eud_get_last_error before error logged.");
        break;
    case EUD_ERR_BAD_PARAMETER:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error not yet set: Bad parameter passed to EUD API.");
        break;
    //EUD Generic err codes - device manager errors
    case EUD_ERR_CANT_CHANGE_DEVICE_MGR_MODE_WHEN_IN_AUTO:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Device Manager Mode must  be in Manual Select in order to set device ID (See SetDeviceManagerMode()).");
        break;
    case EUD_ERR_EUD_HANDLER_DEVICEID_NOT_POPULATED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Device ID not populated. EUD Internal error, please call technical support.");
        break;
    case EUD_ERR_EUD_HANDLER_DEVICEID_NOT_RECOGNIZED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Device ID not recognized. EUD Internal error, please call technical support.");
        break;
    case EUD_ERR_DEVICE_MANAGER_MODE_PARAMETER_ERR:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Invalid parameter given for SetDeviceManagerMode(Mode) argument.");
        break;

    case EUD_ERR_UNKNOWN_OPCODE_SELECTED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Invalid opcode selected to send to target. Internal error; please call technical support.");
        break;
    case EUD_ERR_NULL_POINTER_GIVEN_FOR_NONZERO_PACKET_RESPONSE:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Opcode given has nonzero return size, but NULL pointer given for response destination.");
        break;
    case EUD_ERR_NEED_DEVMGR_INPUT_MANUAL_MODE:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - SetDeviceManagerMode has been set to Manual Mode (1), but a device ID number has not yet been set by user. Please set and try again.");
        break;
    case EUD_ERROR_MAX_DEVMGR_DEVICES_SEARCHED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Searched all potential Device Manger names and found none (SetDeviceManagerMode==Auto). Does device show up in Device Manager?");
        break;
    case EUD_ERR_WIN_PIPE_CREATE:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Error ocurred when creating windows pipe to find attached EUD devices. Please contact technical support.");
        break;
    case EUD_ERR_DEVICEID_NOT_FOUND:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Selected device ID not found. Try unplugging and plugging back in, or try reverifying device ID for the device.");
        break;
    case EUD_ERR_INITIALIZED_DEVICE_NOT_FOUND:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Could not find newly initialized device after running initialization sequence. Is EUD Control device properly functioning?");
        break;

    case EUD_ERR_DURING_EUD_ID_CHILD_CALL:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Windows API error ocurred during child process call. Please call technical support.");
        break;
    case EUD_ERR_DURING_CREATE_CHILD_PROCESS:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Windows API error ocurred during child process creation. Please call technical support.");
        break;
    case EUD_ERR_DURING_READ_FROM_PIPE:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Windows API error ocurred during child pipe read. Please call technical support.");
        break;
    case EUD_ERR_EUD_ID_EXE_NOT_FOUND:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Could not locate eud_id5.exe in directory above eud.dll. Please check that this file is present for Windows platforms.");
        break;
    case EUD_ERR_CTL_ENUMERATION_FAILED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Error ocurred when attempting to enumerate control peripheral. Is USB device functioning properly?");
        break;
    case EUD_ERR_PIPE_PRELOAD:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - Error ocurred during PIPE writing in Windows configuration. Please call technical support.");
        break;
    //USB err codes
    case EUD_USB_ERROR:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD USB generic error - some USB transaction error occurred.");
        break;
    case EUD_USB_ERROR_SEND_FAILED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD USB error - send data failed. This may be due to a USB cable failure or disconnect.");
        break;
    case EUD_USB_ERROR_SEND_CONFIRMATION_TIMEOUT:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD USB error - Wait for send confirmation timed out. This usually indicates a lock up of the EUD hardware. Please restart hardware and try again.");
        break;
    case EUD_USB_ERROR_WRITE_FAILED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD USB error - write data failed. This may be due to a USB cable failure or disconnect.");
        break;
    case EUD_USB_ERROR_WRITE_DEVICE_ERROR:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD USB error - device (0x1f) failure ocurred during write. Could be device or connection issue.");
        break;
    case EUD_USB_ERROR_NOT_ALL_BYTES_SENT:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD USB error - not all USB bytes sent.");
        break;
    case EUD_USB_ERROR_NOT_ALL_BYTES_READ:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD USB Read error - not all bytes read/underflow.");
        break;
    case EUD_USB_ERROR_DEVICE_ERROR:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD USB device error - a device or connection error could have ocurred, or an EUD internal software error.");
        break;
    case EUD_USB_ERROR_READ_TIMEOUT:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD USB Read Warning - no data received from device.");
        break;
    case EUD_USB_ERROR_READ_FAILED_GENERIC:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD USB Read generic error.");
        break;
    case EUD_USB_DEVICE_NOT_DETECTED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error - No EUD USB device found. EUD incremented through likely device manager names and attach failed for each.");
        break;
    case USB_ERR_SENDSIZE_0_NODATASENT:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD USB Warning - Data send size was 0, so no data sent.");
        break;
    ///////////////////////////////
    //  Peripheral error codes   //
    ///////////////////////////////

    //CTL error state defines
    case EUD_ERR_CTL_GIVEN_RSTDELAY_TOO_HIGH:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD CTL error - Given reset delay parameter was too high. Max value: %d", MAX_EUDMSMRESET_TIME_MS);
        break;
    case CTL_EUD_ERR_VBUS_INT_NOT_CLEARED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD CTL error - VBUS_INT not cleared by MSM software within max timeout (%dms)", CTL_MAX_VBUS_INT_TMOUT);
        break;
    case CTL_EUD_ERR_CHGR_INT_NOT_CLEARED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD CTL error - CHGR_INT not cleared by MSM software within max timeout (%dms)", CTL_MAX_VBUS_INT_TMOUT);
        break;
    //JTG error state defines
    case EUD_ERR_TDO_BUFFER_OVERFLOW:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD JTG error - TDO Index exceeded max TDO Buffer.");
        break;
    case EUD_ERR_TDO_BUFFER_UNDERFLOW:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD JTG error - TDO buffer underflow - expected to read more bytes. Device issue?");
        break;
    case EUD_ERR_JTAG_OUT_BUFFER_OVERFLOW:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD JTG error - Exceeded allowed number of bytes out for JTAG packet.");
        break;
    case EUD_ERR_JTG_EXPECTED_NONZERO_TDO:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD JTG error - JTAG state machine error - expected nonzero TDO value.");
        break;
    case EUD_ERR_JTAG_SCAN_BAD_STATE:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD JTG error - JTAG state machine error.");
        break;
    case EUD_ERR_JTG_NOT_IMPLEMENTED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD JTG error - Function not implemented.");
        break;
    case EUD_ERR_JTG_DEVICE_NOT_RESPONSIVE:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD JTG error - CHIP ID couldn't be accessed from DAP. Incorrect config or unresponsive device.");
        break;
    case EUD_JTG_ERR_REQUIRE_FLUSH:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD JTG error - Internal EUD buffers already populated when calling a write-through API. Please flush other commands through then try API again.");
        break;
    //SWD error state defines
    case EUD_SWD_ERR_FUNC_NOT_IMPLEMENTED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - function not yet implemented.");
        break;
    case EUD_SWD_ERR_SWDCMD_MAXCOUNT_EXCEEDED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - Parameter for SWDCMD() 'count' too high.");
        break;
    case EUD_SWD_ERR_NULL_DATA_POINTER_ON_WRITE_CMD:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - Parameter for SWDCMD() 'data' was Null pointer on a write command.");
        break;
    case EUD_SWD_ERR_BAD_DI_TMS_PARAMETER:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - Parameter for swd_di_tms() was too large. Please see documentation.");
        break;
    case EUD_SWD_ERR_SWD_TO_JTAG_ALREADY_DONE:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - JTAG to SWD operation has already been done. To perform operation, call SWDResetPeripheral, then jtag_to_swd.");
        break;
    case EUD_SWD_ERR_JTAGID_REQUESTED_BEFORE_STOJDONE:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - JTAGID requested via SWD but JTAG->SWD not yet done. Call  operation has already been done. To perform operation, call jtag_to_swd first.");
        break;
    case EUD_SWD_ERR_JTAGID_NOT_RECEIVED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - JTAGID not received from target via SWD. Some connection or configuration error possible.");
        break;
    case EUD_SWD_ERR_UNKNOWN_MODE_SELECTED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - Unknown buffer mode selected. Internal error. Please contact Qualcomm Technologies Inc tool support.");
        break;
    case EUD_SWD_ERR_MAX_USB_IN_BUFFER_REACHED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - USB Read buffer max reached. Please read out data in SWDHandler->USB_Read_Buffer and call swd_flush to clear this buffer");
        break;
    case EUD_SWD_ERR_EXPECTED_BYTES_MISCALCULATION:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - Expected Bytes miscalculation error. Internal error. Please contact Qualcomm Technologies Inc tool support.");
        break;
    case EUD_ERR_SWD_MANAGED_BUFFER_USB_IN_BUFFER_OVERFLOW:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - USB IN buffer has overflowed. Last write request has been lost. Please read out current buffer, call swd_flush, and resend data.");
        break;
    case EUD_ERR_SWD_BAD_DATA_STRUCT:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - Bad \ref swd_cmd_st passed - valid data mask test failed. Please check passed data structure.");
        break;
    case EUD_SWD_ERR_CANNOT_CHANGE_READBUFFER_IN_MANAGEDBUFMODE:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - Cannot change USB_Read_Buffer when in MANAGEDBUFFERMODE. Please change mode and try again.");
        break;
    case EUD_SWD_ERR_NO_JTAGID:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - Initialized SWD peripheral but couldn't retrieve JTAG ID From DAP via SWD.");
        break;
    case EUD_ERR_SWD_NUM_BYTES_GREATER_THAN_BUFFSIZE:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - Data buffer size given greater than SWD hardware buffer size.");
        break;
    case SWD_ERR_SWD_ACK_WAIT_DETECTED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - Detected  wait response from bus (SWD ACK 010 toggled).");
        break;
    case SWD_ERR_SWD_ACK_FAULT_DETECTED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - Detected fault response from bus (SWD ACK 100 toggled).");
        break;
    case SWD_ERR_SWD_PACKET_ASSEMBLE_ERR:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - Error detected in SWD assembled packet. Please call technical support. ");
        break;
    case EUD_ERR_BAD_PARAMETER_NULL_POINTER_SWDREAD:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - NULL pointer given to swd_read function as a return value.");
        break;

    case SWD_WARN_SWDSTATUS_NOT_UPDATED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD Warning - SWDStatus parameter has not been updated since last sent value.");
        break;
    case EUD_SWD_ERR_STATUS_COUNT_0:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD SWD error - swd_set_swd_status_max_count is set to 0, so status packets will not be appended to outgoing packets. No status to report.");
        break;

    //Trace Error state defines
    case EUD_TRC_ERR_FUNCTION_NOT_IMPLEMENTED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD TRC error - Function not yet implemented. Potentially due to hardware limitation.");
        break;
    case EUD_TRC_ERR_NO_DATA:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD TRC error - No data received after multiple reads and flush command.");
        break;

    case EUD_TRACE_CTRL_HANDLER_REGISTRATION_ERR:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD TRC error - Attempt to register a Windows exit handler failed. Internal error. Contact support.");
        break;
    case EUD_ERR_TRACE_CHUNKSIZE_LESS_THAN_TRANSLEN:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD TRC error - User attempted to set chunksize to less than transaction length. Chunksize must be greater than transaction length.");
        break;
    case EUD_TRC_TRANSLEN_GREATER_THAN_CHUNKSIZE:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD TRC error - User attempted to set transaction length to greater than chunk size. Chunksize must be greater than transaction length.");
        break;
    case EUD_ERR_READONLY_DIRECTORY_GIVEN:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD TRC error - Read only directory given for trace destination.");
        break;
    case EUD_TRC_TRANSLEN_CANNOT_BE_MULTIPLE_128:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD TRC error - Transaction Length cannot be a multiple of 128.");
        break;
    case EUD_ERROR_TRACE_DATA_STOPPED:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD TRC error - Trace data stream halted due to USB timeout. Recommend adjusting to shorter trace length or longer trace timeout parameters");
        break;
    //COM error state defines
    //---

    default:
        sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Error: Unknown error code given.");
        break;
        
    }
    //TODO - replace with switch statement wiht err values
    //char teststring_p[30] = "TEST";
    *stringsize_p = (uint32_t)strlen(tempstring) + 1; //Add 1 for terminating character

    memcpy(stringbuffer, tempstring, *stringsize_p);

    return EUD_SUCCESS;

}
