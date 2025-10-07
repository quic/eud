/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   eud.cpp
*
* Description:                                                              
*   CPP source file for generic EUD device APIs
*
***************************************************************************/

#include "eud.h"

#include <iostream>
#include <ctime>
#include <sstream>
#include <fstream>
#include <string>
// #include <chrono>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#if defined ( EUD_WIN_ENV )
    #include <Psapi.h>
    #include <Windows.h>
#endif

    //===---------------------------------------------------------------------===//
    //
    // External API functions
    //
    //===---------------------------------------------------------------------===//
#if defined ( EUD_WIN_ENV )    
    __declspec(dllexport) HRESULT DllRegisterServer(void)
    {
       QCEUD_Print("DllRegisterServer\n");
       return S_OK;
    }

    __declspec(dllexport) HRESULT DllUnregisterServer(void)
    {
       QCEUD_Print("DllUnregisterServer\n");
       return S_OK;
    }
#elif defined ( EUD_LNX_ENV )
    void __attribute__((constructor)) eudlibInit();
    void __attribute__((destructor)) eudlibDeinit();
#endif

#define MAX_FILE_PATH_NAME 4096
#define EUD_ERR_COULD_NOT_GET_PATH  3
#define EUD_ERR_FILE_PATH_TOO_LONG 5

char* CurrentWorkingDir;

// This function is assigned to execute before main using __attribute__((constructor))
void eudlibInit()
{
    libusb_usb_init();
}

// This function is assigned to execute after main using __attribute__((destructor))
void eudlibDeinit()
{
    libusb_usb_deinit();
}

#if defined ( EUD_WIN_ENV )    
    BOOL WINAPI DllMain
    (
        HINSTANCE hinstDLL,  // DLL module handle
        DWORD fdwReason,     // calling reason
        LPVOID lpReserved
    )
    {
        // Perform actions based on the reason for calling.
        switch( fdwReason ) 
        { 
            case DLL_PROCESS_ATTACH:
                QCEUD_Print("DLLMain: DLL_PROCESS_ATTACH\n");
                DisableThreadLibraryCalls(hinstDLL);
                break;

            case DLL_THREAD_ATTACH:
                QCEUD_Print("DLLMain: DLL_THREAD_ATTACH\n");
                break;

            case DLL_THREAD_DETACH:
                QCEUD_Print("DLLMain: DLL_THREAD_DETACH\n");
                break;

            case DLL_PROCESS_DETACH:
                QCEUD_Print("DLLMain: DLL_PROCESS_DETACH\n");
                break;
        }
        //DWORD len;
        //HMODULE hModule = GetModuleHandleW(NULL);
        //TCHAR pbuf[MAX_FILE_PATH_NAME+1];
        CurrentWorkingDir = new char[MAX_FILE_PATH_NAME + 1];
        HMODULE hm = NULL;
        const char* localFunc = EUD_EXECUTABLE_NAME;

        if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            localFunc,
            &hm)){
            return eud_set_last_error(EUD_ERR_COULD_NOT_GET_PATH);
        }

        if (!GetModuleFileName(hm, CurrentWorkingDir, MAX_FILE_PATH_NAME)){
        
            return eud_set_last_error(EUD_ERR_COULD_NOT_GET_PATH);
        }

        return TRUE;
    }
#endif






EUD_ERR_t
EudDevice::Init(void)
{
    //read_time_elapsed = 0;
    //write_time_elapsed = 0;
    num_of_reads_ = 0;
    num_of_writes_ = 0;

    //usb_bytes_pending_in = 0;
    usb_num_bytes_pending_in_ = 0;
    usb_num_bytes_pending_out_ = 0;
    //usb_bytes_pending_out = 0;
    
    memset(usb_buffer_in_,   0x0,    EUD_GENERIC_USB_IN_BUFFER_SIZE  );
    memset(usb_buffer_out_,  0x0,    EUD_GENERIC_USB_OUT_BUFFER_SIZE );

    usb_device_handle_ = NULL;
    
    mode_ = MANAGEDBUFFERMODE;

    
    return 0; 
}

/* API to assert a reset using a EUD peripheral */
EUD_ERR_t EudDevice::AssertReset(void) {
    return EUD_SUCCESS;
}

/* API to de-assert reset using a EUD peripheral */
EUD_ERR_t EudDevice::DeAssertReset(void) {
    return EUD_SUCCESS;
}

//BufModeSelect_e EudDevice::GetBufMode(void){
uint32_t EudDevice::GetBufMode(void){
    return mode_;
}


EUD_ERR_t EudDevice::SetBufMode(uint32_t mode){
    if (mode > MAX_BUFFER_MODES) return EUD_ERR_BAD_PARAMETER;

    //TODO - if transitioning between modes, may need to flush
    mode_ = mode;

    return EUD_SUCCESS;

}


EUD_ERR_t
EudDevice::UsbWrite(uint8_t* cmd, uint32_t num_bytes)
{
    
    USB_ERR_t errvalue;
    DWORD * errcode = new DWORD[1];
    
    *errcode = 0;
    if (num_bytes > SWD_OUT_BUFFER_SIZE)
    {
        return EUD_ERR_SWD_NUM_BYTES_GREATER_THAN_BUFFSIZE;
    }

    uint32_t bytes_sent = 0;
    errvalue = usb_device_handle_->WriteToDevice((PVOID)cmd, num_bytes, errcode);
    if(errvalue != 0)
    {
        return errvalue;
    }

    bytes_sent = num_bytes;  // TODO - get bytes sent


    if (bytes_sent != num_bytes)
    {
        return EUD_USB_ERROR_NOT_ALL_BYTES_SENT;
    }

    return EUD_USB_SUCCESS;
}

USB_ERR_t EudDevice::UsbOpen()
{
    
    int32_t usb_attached;
    //uint32_t counter = 0;
    
    //get instance of USB device and set up its parameters
    usb_device_handle_ = new UsbDevice(FALSE);
    usb_device_handle_->device_type_ = device_type_;
    
    std::string devmgrname_local;
    usb_device_handle_init_flag_ = TRUE;
    
    
    usb_attached = EUD_USB_ERROR;
#ifndef DEVICEMGR_ITERATOR_METHOD

    usb_attached = UsbOpen(this->device_name_, this->device_type_, DEFAULT_USB_OPEN_WAIT);
#else
    while ((counter < MAXUSBWAIT) && (usb_attached != EUD_USB_SUCCESS))
    {
        devmgrname_local = devmgr_devnamegenerator(this);//DvcMgr_DeviceName.c_str();
        memcpy(usb_device_handle->devmgr_devname, devmgrname_local.c_str(), devmgrname_local.length() + 1);

        if ((devmgrname_local == EUD_NULL_DEVMGR_NAME) || (devicetype == EUD_NULL_DEVID)){
            delete usb_device_handle;
            usb_device_handle_init_flag = FALSE;
            return EUD_USB_ERROR;
        }
        if (devmgrname_local == EUD_MANUAL_MODE_NEED_USER_INPUT){
            delete usb_device_handle;
            usb_device_handle_init_flag = FALSE;
            return EUD_ERR_NEED_DEVMGR_INPUT_MANUAL_MODE;
        }
        if (devmgrname_local == EUD_AUTO_MODE_MAX_DEVICES_REACHED){
            delete usb_device_handle;
            usb_device_handle_init_flag = FALSE;
            return EUD_ERROR_MAX_DEVMGR_DEVICES_SEARCHED;
        }


        usb_attached = usb_device_handle->UsbOpen(&error_code);
        Sleep(1);
        counter++;

    }

#endif
//  eud_set_last_usb_error(error_code);
    if (usb_attached == EUD_USB_SUCCESS){
        return EUD_SUCCESS;
    }
    else{
        delete usb_device_handle_;
        usb_device_handle_init_flag_ = FALSE;
        return usb_attached;
    }

}
USB_ERR_t EudDevice::UsbOpen(std::string devmgrname, uint32_t deviceID_local, uint32_t maxwait)
{
    
    return UsbOpen(devmgrname.c_str(), deviceID_local, maxwait);
}


USB_ERR_t EudDevice::UsbOpen(const char * devmgrname, uint32_t devicetype_local, uint32_t maxwait)
{
#if defined ( EUD_WIN_ENV )
    char error_code;
    int32_t usb_attached;
    uint32_t counter = 0;
    
    DvcMgr_DeviceName = std::string(devmgrname);

    //get instance of USB device and set up its parameters
    usb_device_handle_ = new UsbDevice;
    usb_device_handle_->device_type_ = devicetype_local;
    
    usb_device_handle_init_flag_ = TRUE;
    usb_attached = EUD_USB_ERROR;
    
    while ((counter < maxwait) && (usb_attached != EUD_USB_SUCCESS))
    {
        //usb_device_handle->devmgr_devname = devmgr_devnamegenerator(this);//DvcMgr_DeviceName.c_str();

        if ((strcmp(devmgrname, EUD_NULL_DEVMGR_NAME) != 0) || (devicetype_local == EUD_NULL_DEVID)){
            return EUD_USB_ERROR;
        }
        
        usb_attached = usb_device_handle_->UsbOpen(&error_code, devmgrname);

        //If send timeout, then return error.
        if (usb_attached == EUD_USB_ERROR_SEND_CONFIRMATION_TIMEOUT) {
            break;
        }
        Sleep(10);
        counter++;

    }
    
    //  eud_set_last_usb_error(error_code);
    
    return usb_attached;
#else
    return EUD_USB_ERROR;
#endif
}


USB_ERR_t EudDevice::UsbOpen(usb_dev_access_type dev, uint32_t devicetype_local, uint32_t maxwait)
{

    int error_code;
    int32_t usb_attached;
    uint32_t counter = 0;
    


    //get instance of USB device and set up its parameters
    usb_device_handle_ = new UsbDevice;
    usb_device_handle_->device_type_ = devicetype_local;
    usb_device_handle_->dev_ = dev;
    
    usb_device_handle_init_flag_ = TRUE;
    usb_attached = EUD_USB_ERROR;
    
    while ((counter < maxwait) && (usb_attached != EUD_USB_SUCCESS))
    {


        if ((!dev) || (devicetype_local == EUD_NULL_DEVID)){
            return EUD_USB_ERROR;
        }
        
        usb_attached = usb_device_handle_->UsbOpen(&error_code, dev);
        Sleep(10);
        counter++;

    }
    //  eud_set_last_usb_error(error_code);
    
    return usb_attached;

}


/* Read data from USB into in_buffer. */

//fixme - data param is misleading in calls to this fxn
USB_ERR_t EudDevice::UsbRead(uint32_t in_length, uint8_t *in_buffer)
{
    DWORD  errcode = 0;
    usb_device_handle_->ReadFromDevice((PVOID)in_buffer, in_length, &errcode);

    if(LIBUSB_SUCCESS != static_cast<int>(errcode))
    {
        if(LIBUSB_ERROR_NOT_FOUND == static_cast<int>(errcode))
        {
            return EUD_USB_DEVICE_NOT_DETECTED;
        }
        else
        {
            return EUD_USB_ERROR_READ_FAILED_GENERIC;
        }
    }
    // if (errcode != LIBUSB_SUCCESS) {
    //     if (errcode == LIBUSB_ERROR_NOT_FOUND) {
    //         return EUD_USB_DEVICE_NOT_DETECTED;
    //     }
    //     else {
    //         return EUD_USB_ERROR_READ_FAILED_GENERIC;
    //     }
    // }
    else {
        return EUD_USB_SUCCESS;
    }
}

EUD_ERR_t EudDevice::UsbWriteRead(uint32_t out_length, uint8_t* out_buffer, uint32_t in_length, uint8_t* in_buffer){
    USB_ERR_t err;
    unsigned long errcode = 0;
    
    if ( (err = usb_device_handle_->WriteToDevice(out_buffer, out_length, &errcode)) != 0 ){
        return errcode;
    }
    
    memset(in_buffer, 0, in_length);
    if ( (err = usb_device_handle_->ReadFromDevice((PVOID)in_buffer, in_length, &errcode)) != 0 ){
        return errcode;
    }

    return EUD_USB_SUCCESS;
}



EUD_ERR_t
EudDevice::UsbWriteRead(uint32_t out_length, uint32_t in_length)
{
    
    uint8_t *statusbuf;
    //std::vector<uint8_t> data;
    EUD_ERR_t result;
    
    statusbuf = new uint8_t[in_length];
    //fixme - clean up
    
    result = UsbWrite(out_length);
    if (result != EUD_USB_SUCCESS)
    {
        return result;
    }

    //result = UsbRead(in_length, usb_buffer_in_);
    result = UsbRead(in_length, statusbuf);
    //result = UsbRead(in_length);

    delete [] statusbuf;
    if (result != EUD_USB_SUCCESS)
    {
        return result;
    }
    return EUD_USB_SUCCESS;
}
/*
try{
    USB_Write_Buffer

}
catch (std::out_of_range){
    *FlushRequired_p = TRUE;
    USB_Read_Buffer_p.push_back(SWD_PCKT_SZ_RECV_SWDCMD);
}
//Check this after 
if (*USB_Read_Index_p >= DEFAULT_USB_READ_BUFFER_MAX_VEC_SIZE) return eud_set_last_error(EUD_SWD_ERR_MAX_USB_IN_BUFFER_REACHED);
*/
EUD_ERR_t EudDevice::UsbWrite(uint32_t out_length)
{
    USB_ERR_t err;
    DWORD errcode = 0;

    if (out_length > EUD_GENERIC_USB_OUT_BUFFER_SIZE)
    {
        //return -1;
        return EUD_ERR_JTAG_OUT_BUFFER_OVERFLOW;
    }

    if (out_length <= 0)
    {
        return EUD_USB_SUCCESS;
    }

    err = usb_device_handle_->WriteToDevice(usb_buffer_out_, out_length, &errcode);
    if(err != 0)
    {
        return err;
    }
    
    if(LIBUSB_SUCCESS != static_cast<int>(errcode))
    {
        if(LIBUSB_ERROR_NOT_FOUND == static_cast<int>(errcode))
        {
            return EUD_USB_DEVICE_NOT_DETECTED;
        }
        else
        {
            return EUD_USB_ERROR_WRITE_FAILED;
        }
    }
    // if (errcode != LIBUSB_SUCCESS) {
    //     if (errcode == LIBUSB_ERROR_NOT_FOUND) {
    //         return EUD_USB_DEVICE_NOT_DETECTED;
    //     }
    //     else {
    //         return EUD_USB_ERROR_WRITE_FAILED;
    //     }
    // }
    // else {
    //     return EUD_USB_SUCCESS;
    // }
    return EUD_SUCCESS;
}

//Cleanup and delete usb object
EUD_ERR_t
EudDevice::UsbClose()
{
    if ((DvcMgr_DeviceName == EUD_NULL_DEVMGR_NAME) || (device_type_ == EUD_NULL_DEVID))
        return EUD_USB_ERROR;

    //usb_device should have been init'd
    
    delete usb_device_handle_;
    usb_device_handle_ = NULL;
    DvcMgr_DeviceName = EUD_NULL_DEVMGR_NAME;
    device_type_ = DEVICETYPE_EUD_NULL;
    device_id_ = 0;

    return EUD_SUCCESS;
}

EUD_ERR_t
EudDevice::Quit(void)
{
    if (usb_device_handle_init_flag_ == FALSE)
    {
        return EUD_USB_ERROR;
    }
    usb_device_handle_->UsbClose();
    //FIXME
    //delete usb_device_handle;
    return EUD_USB_SUCCESS;

}

/*
// SWD-related methods
EUD_ERR_t 
EudDevice::read_cmd(uint8_t* in_ptr)
{
    if ((Current_IN_idx + READ_CMD_SIZE) > SWD_IN_BUFFER_SIZE)
        return -1;

    if (in_ptr)
    {
        for (uint32_t i = 0; i < READ_CMD_SIZE; i++)
        {
            in_ptr[i] = usb_buffer_in_[Current_IN_idx + i];
        }
    }
    Current_IN_idx += READ_CMD_SIZE;

    return 0;
}
*/
/*
EUD_ERR_t 
EudDevice::read_all_cmds()
{
    uint32_t ret = 0;
    //assert(IN_Ptrs_Holder_Vctr.size() <= 4);
    CHAR_VECTOR_ITR it = IN_Ptrs_Holder_Vctr.begin();

    while (it != IN_Ptrs_Holder_Vctr.end())
    {
        uint8_t* in = *it;
        ret = read_cmd(in);
        if (ret != 0)
        {
            break;
        }
        ++it;
    }
    IN_Ptrs_Holder_Vctr.clear();
    //usb_bytes_pending_in = 0;
    usb_num_bytes_pending_in_ = 0;
    return ret;
}
*/

using namespace std;

class EudSignalParser
{

    
public:
    string ParseSwdSignal(PVOID buffer);
    string ParseJtagSignal(PVOID buffer);
    char * PrintSwdSignal(PVOID buffer);

private:
    //SWD contents
    string GetSwdCmd(PVOID buffer);
    string GetSwdPayload(PVOID buffer);
    uint32_t GetSwdCmdPayloadSize(PVOID buffer);

};


char * EudSignalParser::PrintSwdSignal(PVOID buffer)
{
    ostringstream rstring, string1, string2;
    string str;
    //char * rvalue;
    
    //unsigned char rstring[50];
    //std::vector<std::string> rvalue;
    //string1 << GetSwdCmD(Buffer);
    //string2 << get_SWD_Payload(Buffer);
    //std::ostringstream stringstream;
    //rvalue << string1 + "|" + string2;
    rstring << GetSwdCmd(buffer) << "|" << GetSwdPayload(buffer) << "\n";
    //rstring = (unsigned char *)(rvalue.str());
    str = rstring.str();
    //sprintf(rstring, "%s|%s", (unsigned char *)string1, (unsigned char *)string2);
    
    //rvalue = rstring;
    char * rvalue = new char[rstring.str().length() + 1];
    copy(str.c_str(), str.c_str() + str.length() + 1, rvalue);
    
    return rvalue;
}


string EudSignalParser::GetSwdPayload(PVOID buffer)
{

    unsigned char  cmdsize;
    //char * rstring;
    
    ostringstream rvalue;
    //char * rstring = new char[];
    char rstring[100];
    cmdsize = GetSwdCmdPayloadSize(buffer);

        if (cmdsize == 0)
        {
            return "----";
        }
        else if (cmdsize <= 4)
        {
            sprintf_s(rstring, sizeof(rstring), "%x---", (unsigned char)*((unsigned char *)buffer + 1));
            
        }
        else if (cmdsize <= 8)
        {
            sprintf_s(rstring, sizeof(rstring), "%x%x--", (unsigned char)*((unsigned char *)buffer + 1), (unsigned char)*((unsigned char *)buffer + 2));
            
                
        }
        else
        {
            sprintf_s(rstring,sizeof(rstring), "%x%x%x%x",
                (unsigned char)*((unsigned char *)buffer + 1),
                (unsigned char)*((unsigned char *)buffer + 2),
                (unsigned char)*((unsigned char *)buffer + 3),
                (unsigned char)*((unsigned char *)buffer + 4));
        }



    rvalue << rstring<<"\n";
    return rvalue.str();
}


//Note that this returns payload size in bits NOT Bytes
uint32_t EudSignalParser::GetSwdCmdPayloadSize(PVOID buffer)
{
    unsigned char cmd;

    cmd = (unsigned char)*((unsigned char *)buffer);

    if (cmd <= 0x0A)
    {
        switch (cmd)
        {
            case 0x0: return 0;
            case 0x1: return 0;
            case 0x2: return 4;
            case 0x3: return 8;
            case 0x4: return 5;
            case 0x5: return 16;
            case 0x6: return 19;
            case 0x7: return 0;
            case 0x8: return 0;
            case 0x9: return 0;
            case 0xA: return 0;
        }
    }
    else if ((0x0B <= cmd) && (cmd <= 0x7F))
    {
        return 0;
    }
    else if ((0x0B <= cmd) && (cmd <= 0x7F))
    {
        return 0;
    }
    //This is SWD CMD case. conditional.
    else
    {
        //documentation: 
            //If write command (ie cmd_bit[2] = 0), then four byte payload.
            //If read command(ie cmd_bit[2] = 1), then no payload.
        if ((cmd)&(0x02))
        {
            return 0;
        }
        else
        {
            return 4;
        }
    }

    //Should never get here
    return 0;

}


string EudSignalParser::GetSwdCmd(PVOID buffer)
{
    unsigned char cmd;
    

    cmd = (unsigned char)*((unsigned char *)buffer);

#ifdef OLD_EUD_SPEC
    if (cmd <= 0x0A)
    {
        switch (cmd)
        {

            case 0x0: return "SWD_NOP_0x0";    
            case 0x1: return "SWD_FLUSH_0x1";
            case 0x2: return "SWD_FREQ_0x2";
            case 0x3: return "SWD_DELAY_0x03";
            case 0x4: return "SWD_BITBANG_0x04";
            case 0x5: return "SWD_DI_TMS_0x05";
            case 0x6: return "SWD_TIMING_0x06";
            case 0x7: return "SWD_STATUS_0x07";
            case 0x8: return "SWD_PORT0_SEL_0x08";
            case 0x9: return "SWD_PORT1_SEL_0x09";
            case 0xA: 
                return "SWD_PERIPH_RST_0x0A";

        }
    }
    else if ((0x0B <= cmd) && (cmd<=0x7F))
    {
        return "SWD_RESERVED";
    }
    else if ((0x0B <= cmd) && (cmd <= 0x7F))
    {
        return "SWD_ERROR";
    }
    else
    {
        return "SWD_CMD";
    }
#else
    if (cmd <= 0x08)
    {
        switch (cmd)
        {

        case 0x0: return "SWD_NOP_0x0";
        case 0x1: return "SWD_FLUSH_0x1";
        case 0x2: return "SWD_FREQ_0x2";
        case 0x3: return "SWD_DELAY_0x03";
        case 0x4: return "SWD_BITBANG_0x04";
        case 0x5: return "SWD_DI_TMS_0x05";
        case 0x6: return "SWD_TIMING_0x06";
        case 0x7: return "SWD_STATUS_0x07";
        case 0x8: return "SWD_PERIPH_RST_0x08";
        
        }
    }
    else if ((0x08 <= cmd) && (cmd <= 0x7F))
    {
        return "SWD_RESERVED";
    }
    else
    {
        return "SWD_CMD";
    }

#endif
    return "";
}

inline void pack_uint32_to_uint8_array(uint32_t* data32_p, uint8_t* data8_p){
#ifdef SWD_LITTLEENDIAN
    data8_p[3] = (uint8_t)(*data32_p & 0xFF);
    data8_p[2] = (uint8_t)((*data32_p >> 8) & 0xFF);
    data8_p[1] = (uint8_t)((*data32_p >> 16) & 0xFF);
    data8_p[0] = (uint8_t)((*data32_p >> 24) & 0xFF);
#else
    data8_p[0] = (uint8_t)(*data32_p & 0xFF);
    data8_p[1] = (uint8_t)((*data32_p >> 8) & 0xFF);
    data8_p[2] = (uint8_t)((*data32_p >> 16) & 0xFF);
    data8_p[3] = (uint8_t)((*data32_p >> 24) & 0xFF);
#endif
}


inline void pack_uint32_to_uint8_array(uint32_t data32, uint8_t* data8_p){
#ifdef SWD_LITTLEENDIAN
    data8_p[3] = (uint8_t)(*data32_p & 0xFF);
    data8_p[2] = (uint8_t)((*data32_p >> 8) & 0xFF);
    data8_p[1] = (uint8_t)((*data32_p >> 16) & 0xFF);
    data8_p[0] = (uint8_t)((*data32_p >> 24) & 0xFF);
#else
    data8_p[0] = (uint8_t)(data32 & 0xFF);
    data8_p[1] = (uint8_t)((data32 >> 8) & 0xFF);
    data8_p[2] = (uint8_t)((data32 >> 16) & 0xFF);
    data8_p[3] = (uint8_t)((data32 >> 24) & 0xFF);
#endif
}


inline void unpack_uint8_to_uint32_array(uint32_t* data32_p, uint8_t* data8_p, uint32_t endianoption){
    *data32_p = 0;
    if (endianoption == EUD_LITTLEENDIAN){
        *data32_p = (uint32_t)(*(data8_p + 3));
        *data32_p += (uint32_t)(*(data8_p + 2) << 8);
        *data32_p += (uint32_t)(*(data8_p + 1) << 16);
        *data32_p += (uint32_t)(*(data8_p) << 24);
    }
    else{
        *data32_p = (uint32_t)* data8_p;
        *data32_p += (uint32_t)(*(data8_p + 1) << 8);
        *data32_p += (uint32_t)(*(data8_p + 2) << 16);
        *data32_p += (uint32_t)(*(data8_p + 3) << 24);
    }
}


///Write to device with opcode and payload. N bytes returned
EUD_ERR_t 
EudDevice::WriteCommand(uint32_t opcode, uint32_t data, uint32_t* rvalue)
{
    EUD_ERR_t err;

    uint32_t payload_sz, response_sz, endianoption;
    uint8_t* data_out_p;
    uint8_t* data_in_p;

    //Payload data
    payload_sz = periph_payload_size_table_[opcode];
    data_out_p = new uint8_t[payload_sz];
    *data_out_p = opcode & 0xFF;
    if (payload_sz > 1){
        pack_uint32_to_uint8_array(data, (data_out_p + 1));
    }

    //response data
    response_sz = periph_response_size_table_[opcode];
    data_in_p = new uint8_t[response_sz];
    
    
    err = UsbWriteRead(payload_sz, data_out_p, response_sz, data_in_p);
    
    endianoption = periph_endian_option_table_[opcode];
    unpack_uint8_to_uint32_array(rvalue, data_in_p, endianoption);

    delete[] data_in_p;
    delete[] data_out_p;

    return err;
}


///Write to device with opcode, payload. Expect N bytes returned. 8Bit parameters.
EUD_ERR_t
EudDevice::WriteCommand(uint32_t opcode, uint8_t* data, uint8_t* rvalue)
{
    EUD_ERR_t err;

    uint32_t payload_sz, response_sz;
    uint8_t* data_out_p;
    

    //Payload data
    payload_sz = periph_payload_size_table_[opcode];
    data_out_p = new uint8_t[payload_sz];
    *data_out_p = opcode & 0xFF;
    memcpy(data_out_p, data, (payload_sz - 1));

    //response data
    response_sz = periph_response_size_table_[opcode];

    err = UsbWriteRead(payload_sz, data_out_p, response_sz, rvalue);

    delete[] data_out_p;

    return err;
}


inline void pack_uint32_to_uint8_array2(uint32_t data32, uint8_t* data8_p) {
#ifdef SWD_LITTLEENDIAN
    data8_p[3] = (uint8_t)(*data32_p & 0xFF);
    data8_p[2] = (uint8_t)((*data32_p >> 8) & 0xFF);
    data8_p[1] = (uint8_t)((*data32_p >> 16) & 0xFF);
    data8_p[0] = (uint8_t)((*data32_p >> 24) & 0xFF);
#else
    data8_p[0] = (uint8_t)(data32 & 0xFF);
    data8_p[1] = (uint8_t)((data32 >> 8) & 0xFF);
    data8_p[2] = (uint8_t)((data32 >> 16) & 0xFF);
    data8_p[3] = (uint8_t)((data32 >> 24) & 0xFF);
#endif

}
EUD_ERR_t EudDevice::QueueCommand(uint32_t opcode, uint32_t payload, uint32_t* response_p){

    eud_queue_p_->transaction_queue_[eud_queue_p_->queue_index_].opcode_payload_[0] = opcode;
    if (opcode > periph_max_opcode_value_){
        return eud_set_last_error(EUD_ERR_UNKNOWN_OPCODE_SELECTED);
    }

    eud_queue_p_->transaction_queue_[eud_queue_p_->queue_index_].payload_size_ = \
        periph_payload_size_table_[opcode];

    if (periph_payload_size_table_[opcode] > 1){
        pack_uint32_to_uint8_array2(payload, (eud_queue_p_->transaction_queue_[eud_queue_p_->queue_index_].opcode_payload_ + 1));
    }

    eud_queue_p_->transaction_queue_[eud_queue_p_->queue_index_].response_size_ = \
        periph_response_size_table_[opcode];

    eud_queue_p_->transaction_queue_[eud_queue_p_->queue_index_].return_ptr_ = response_p;

    return EUD_SUCCESS;
}


EUD_ERR_t EudDevice::CleanupPacketQueue(EudPacketQueue* eud_queue_p){
    eud_queue_p->queue_index_ = 0;
    
    eud_queue_p->raw_write_buffer_idx_ = 0;

    //eud_queue_p->read_buffer_count = 0;
    //eud_queue_p->write_buffer_count = 0;
    eud_queue_p->read_packet_flag_ = false;
    eud_queue_p->swd_command_flag_ = false;
    eud_queue_p->status_appended_flag_ = false;
    eud_queue_p->flush_appended_flag_ = false;
    eud_queue_p->status_stale_flag_ = false;
    eud_queue_p->buffer_status_ = 0;

    //eud_queue_p->write_buffer_count = 0; 
    
    eud_queue_p->read_expected_bytes_ = 0; 

    eud_queue_p->raw_read_buffer_idx_ = 0;

    return EUD_SUCCESS;
}


EUD_ERR_t EudDevice::UsbWriteRead(EudPacketQueue* eud_queue_p){
    
    uint32_t idx;
    EUD_ERR_t err;
    DWORD errcode;
    //Write bytes out.
    if (eud_queue_p->raw_write_buffer_idx_ > 0){
        err = usb_device_handle_->WriteToDevice(eud_queue_p->raw_write_buffer_, eud_queue_p->raw_write_buffer_idx_, &errcode);
        if (err != EUD_SUCCESS){
            eud_set_last_usb_error((EUD_ERR_t)errcode);
            return err;
        }
    }
    //Read bytes in and copy them to destination pointers
    if (eud_queue_p->read_expected_bytes_ > 0){
        //QCEUD_Print("UsbWriteRead: Read. expectedbytes: %d\n", eud_queue_p->read_expected_bytes_);
        err = usb_device_handle_->ReadFromDevice(eud_queue_p->raw_read_buffer_, eud_queue_p->read_expected_bytes_, &errcode);
        if (err != EUD_SUCCESS){
            eud_set_last_usb_error((EUD_ERR_t)errcode);
            return err;
        }
        uint32_t read_buffer_current_idx = 0;
        for (idx = 0; idx < eud_queue_p->queue_index_;idx++){
            if (eud_queue_p->transaction_queue_[idx].response_size_ == 0)
                continue;

            unpack_uint8_to_uint32_array(eud_queue_p->transaction_queue_[idx].return_ptr_, &eud_queue_p->raw_read_buffer_[read_buffer_current_idx]);
            read_buffer_current_idx += eud_queue_p->transaction_queue_[idx].response_size_;
        }
    }
    
    CleanupPacketQueue(eud_queue_p);
    
    return EUD_SUCCESS;
}

EudPacketQueue::EudPacketQueue(){
    uint32_t idx;
    for (idx = 0; idx < EUD_PACKET_QUEUE_SIZE; idx++){
        memset(transaction_queue_[idx].opcode_payload_, 0, EUD_TRANS_QUEUE_OPCODEPAYLOAD_SZ);
        transaction_queue_[idx].response_size_ = 0;
        transaction_queue_[idx].payload_size_ = 0;
        transaction_queue_[idx].return_ptr_ = NULL;
    }

    read_packet_flag_ = false;
    swd_command_flag_ = false;
    status_appended_flag_ = false;
    flush_appended_flag_ = false;
    status_stale_flag_ = true;
    buffer_status_ = 0;
    //write_buffer_count = 0;
    raw_write_buffer_idx_ = 0;
    // 
    read_expected_bytes_ = 0;
    raw_read_buffer_idx_ = 0;
    queue_index_ = 0;
    
    raw_write_buffer_ = new uint8_t[SWD_OUT_BUFFER_SIZE];
    raw_read_buffer_ = new uint8_t[SWD_OUT_BUFFER_SIZE];
}


EudPacketQueue::~EudPacketQueue() {

    delete raw_write_buffer_;
    delete raw_read_buffer_;

}

///Write to device with opcode, no payload. Expect N bytes in return
EUD_ERR_t
EudDevice::WriteCommand(uint32_t opcode, uint32_t* rvalue)
{
    EUD_ERR_t err;

    uint32_t payload_sz, response_sz, endianoption;
    uint8_t* data_out_p;
    uint8_t* data_in_p;
    //libusb_device * dev;

    
    //Payload data
    payload_sz = periph_payload_size_table_[opcode];
    
    if (payload_sz > 1){
        return eud_set_last_error(EUD_ERR_INCORRECT_ARGUMENTS_WRITE_COMMAND);
    }
    data_out_p = new uint8_t[payload_sz];
    *data_out_p = opcode & 0xFF;
    
    //Response data
    response_sz = periph_response_size_table_[opcode];
    data_in_p = new uint8_t[response_sz];

    //Write to USB
    
    //this->deviceID = rvalue;
    //dev = this->devicename;

    err = UsbWriteRead(payload_sz, data_out_p, response_sz, data_in_p);

    endianoption = periph_endian_option_table_[opcode];
    unpack_uint8_to_uint32_array(rvalue, data_in_p, endianoption);
    
    delete[] data_in_p;
    delete[] data_out_p;

    return err;

}
/*
EUD_ERR_t
EudDevice::WriteCommand(uint32_t opcode, uint8_t* rvalue)
{
    EUD_ERR_t err;

    uint32_t payload_sz, response_sz;
    uint8_t* data_out_p;
    uint8_t* data_in_p;

    //Payload data
    payload_sz = periph_payload_size_table_[opcode];
    if (payload_sz > 1){
        return eud_set_last_error(EUD_ERR_INCORRECT_ARGUMENTS_WRITE_COMMAND);
    }
    data_out_p = new uint8_t[payload_sz];
    *data_out_p = opcode & 0xFF;

    //Response data
    response_sz = periph_response_size_table_[opcode];

    //Write to USB.
    err = UsbWriteRead(payload_sz, data_out_p, response_sz, rvalue);

    delete data_in_p, data_out_p;

    return err;

}
*/
///Write to device with opcode and payload. No data to return.
EUD_ERR_t
EudDevice::WriteCommand(uint32_t opcode, uint32_t data)
{
    EUD_ERR_t err;

    uint32_t payload_sz, response_sz;
    uint8_t* data_out_p;
    uint8_t* data_in_p;

    //Payload data
    payload_sz = periph_payload_size_table_[opcode];
    data_out_p = new uint8_t[payload_sz];
    *data_out_p = opcode & 0xFF;
    if (payload_sz > 1){

        pack_uint32_to_uint8_array(data, (data_out_p + 1));
    }

    //Response data
    response_sz = periph_response_size_table_[opcode];
    //Check if there's a response size. If there is, fail out.
    if (response_sz > 0){
        return eud_set_last_error(EUD_ERR_INCORRECT_ARGUMENTS_WRITE_COMMAND);
    }
    data_in_p = new uint8_t[response_sz];

    //Write data to device
    err = UsbWriteRead(payload_sz, data_out_p, response_sz, data_in_p);

    delete[] data_in_p;
    delete[] data_out_p;

    return err;

}
///Write data to device, opcode and payload, no return data. Byte size parameters.
EUD_ERR_t
EudDevice::WriteCommand(uint32_t opcode, uint8_t* data)
{
    EUD_ERR_t err;

    uint32_t payload_sz, response_sz;
    uint8_t* data_out_p;
    uint8_t* data_in_p;

    //Payload data
    payload_sz = periph_payload_size_table_[opcode];
    data_out_p = new uint8_t[payload_sz];
    *data_out_p = opcode & 0xFF;
    memcpy((data_out_p+1), data, (payload_sz - 1));

    //Response data
    response_sz = periph_response_size_table_[opcode];
    //Check if there's a response size. If there is, fail out.
    if (response_sz > 0){
        return eud_set_last_error(EUD_ERR_INCORRECT_ARGUMENTS_WRITE_COMMAND);
    }
    data_in_p = new uint8_t[response_sz];
    
    //Write to device
    err = UsbWriteRead(payload_sz, data_out_p, response_sz, data_in_p);

    delete[] data_in_p;
    delete[] data_out_p;

    return err;

}
///Write to device only opcode. No payload nor return value.
EUD_ERR_t
EudDevice::WriteCommand(uint32_t opcode)
{
    uint32_t payload_sz, response_sz;
    uint8_t* data_out_p;
    uint8_t* data_in_p;

    EUD_ERR_t err;
    payload_sz = periph_payload_size_table_[opcode];
    data_out_p = new uint8_t[payload_sz];
    *data_out_p = opcode & 0xFF;
    
    //For this function overload, payload_sz should be 1 and response should be 0.
    if (payload_sz > 1){
        //pack_uint32_to_uint8_array(data, (data_out_p + 1));
        return eud_set_last_error(EUD_ERR_INCORRECT_ARGUMENTS_WRITE_COMMAND);
    }
    response_sz = periph_response_size_table_[opcode];
    if (response_sz > 0){
        return eud_set_last_error(EUD_ERR_INCORRECT_ARGUMENTS_WRITE_COMMAND);
    }

    data_in_p = new uint8_t[response_sz];

    err = UsbWriteRead(payload_sz, data_out_p, response_sz, data_in_p);

    delete[] data_in_p;
    delete[] data_out_p;

    return err;

}


