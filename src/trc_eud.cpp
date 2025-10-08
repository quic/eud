/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   trace_eud.cpp
*
* Description:                                                              
*   CPP source file for EUD Trace Peripheral APIs
*
***************************************************************************/
#include "time.h"
#include "eud.h"
#include "trc_eud.h"
#if defined ( EUD_WIN_ENV )
    #include <string>
    #include <iostream>
    #include <ctime>
    #include <fstream>
#endif

#include <sstream>


TraceEudDevice::TraceEudDevice(){

    //usb_device *usb_device_handle = EMPTY_USB_HANDLE;
    usb_device_handle_init_flag_ = false;
    device_type_ = DEVICETYPE_EUD_TRC;
    device_id_ = 0;
    DvcMgr_DeviceName = EUD_TRC_DEVMGR_NAME;

    current_trace_timeout_value_ = TRACE_DEFAULT_TRNS_TMOUT;
    current_trace_transaction_length_ = TRACE_DEFAULT_TRANSACTION_LENGTH;

    
    //Create timestamped directory
    std::ostringstream oss;

    struct tm * now = new struct tm;

    #if defined ( EUD_WIN_ENV ) 
    time_t t = time(0);   // get time now
    localtime_s(now,&t);
    #endif
    
    oss << EUD_TRC_DEFAULT_OUTPUTDIR    << '-'
        << (now->tm_year + 1900)        << '-'
        << (now->tm_mon + 1)            << '-'
        << now->tm_mday                 << '-'
        << now->tm_hour                 << '-'
        << now->tm_min                  << '-'
        << now->tm_sec;

    output_dir_ = oss.str();
    delete now;
    now = NULL;
    //TraceSignaller* TraceSignal = new TraceSignaller;
    #if defined ( EUD_WIN_ENV ) 
    trace_stop_signal_ = new TraceStopSignal;
    #endif                


    periph_payload_size_table_[TRC_CMD_NOP] = TRC_PCKT_SZ_SEND_NOP;
    periph_payload_size_table_[TRC_CMD_FLUSH] = TRC_PCKT_SZ_SEND_FLUSH;
    periph_payload_size_table_[TRC_CMD_TRNS_LEN] = TRC_PCKT_SZ_SEND_TRNS_LEN;
    periph_payload_size_table_[TRC_CMD_TRNS_TMOUT] = TRC_PCKT_SZ_SEND_TRNS_TMOUT;
    periph_payload_size_table_[TRC_CMD_TOSS] = TRC_PCKT_SZ_SEND_TOSS;
    periph_payload_size_table_[TRC_CMD_KEEP] = TRC_PCKT_SZ_SEND_KEEP;
    periph_payload_size_table_[TRC_CMD_PERIPH_RST] = TRC_PCKT_SZ_SEND_PERIPH_RST;
    
    periph_response_size_table_[TRC_CMD_NOP] = TRC_PCKT_SZ_RECV_NOP;
    periph_response_size_table_[TRC_CMD_FLUSH] = TRC_PCKT_SZ_RECV_FLUSH;
    periph_response_size_table_[TRC_CMD_TRNS_LEN] = TRC_PCKT_SZ_RECV_TRNS_LEN;
    periph_response_size_table_[TRC_CMD_TRNS_TMOUT] = TRC_PCKT_SZ_RECV_TRNS_TMOUT;
    periph_response_size_table_[TRC_CMD_TOSS] = TRC_PCKT_SZ_RECV_TOSS;
    periph_response_size_table_[TRC_CMD_KEEP] = TRC_PCKT_SZ_RECV_KEEP;
    periph_response_size_table_[TRC_CMD_PERIPH_RST] = TRC_PCKT_SZ_RECV_PERIPH_RST;
    
    periph_endian_option_table_[TRC_CMD_NOP] = TRC_ENDIANOPT_NOP;
    periph_endian_option_table_[TRC_CMD_FLUSH] = TRC_ENDIANOPT_FLUSH;
    periph_endian_option_table_[TRC_CMD_TRNS_LEN] = TRC_ENDIANOPT_TRNS_LEN;
    periph_endian_option_table_[TRC_CMD_TRNS_TMOUT] = TRC_ENDIANOPT_TRNS_TMOUT;
    periph_endian_option_table_[TRC_CMD_TOSS] = TRC_ENDIANOPT_TOSS;
    periph_endian_option_table_[TRC_CMD_KEEP] = TRC_ENDIANOPT_KEEP;
    periph_endian_option_table_[TRC_CMD_PERIPH_RST] = TRC_ENDIANOPT_PERIPH_RST;
    
    periph_max_opcode_value_ = TRC_NUM_OPCODES;
}


TraceEudDevice::~TraceEudDevice(){
    //Write data for any threads to catch to close trace.
    //TraceSignal->writedata(TRC_MESSAGE_CLOSE_TRACE);
    #if defined ( EUD_WIN_ENV ) 
    trace_stop_signal_ = new TraceStopSignal;
    #endif
    Sleep(1000);
    
    //delete TraceSignal;
    //TraceSignal = NULL;

}


#define EUD_TRC_ERR_OCURRED_SIGNAL 12
TraceSignaller::TraceSignaller(){
#if defined ( EUD_WIN_ENV ) 
    sec_attrib_ = new SECURITY_ATTRIBUTES;
    sec_attrib_->nLength = sizeof(SECURITY_ATTRIBUTES);
    sec_attrib_->bInheritHandle = TRUE;
    sec_attrib_->lpSecurityDescriptor = NULL;
    this->read_handle_ = new HANDLE;
    this->write_handle_ = new HANDLE;
    //bool result;
    CreatePipe(this->read_handle_, this->write_handle_, sec_attrib_, 2);
    //ASSERT(result);
#endif    
}

TraceSignaller::~TraceSignaller(){
#if defined ( EUD_WIN_ENV )
    delete sec_attrib_;
    sec_attrib_ = NULL;
    CloseHandle(*this->read_handle_);
    CloseHandle(*this->write_handle_);
#endif
}
//FIXME - cause this to be compiled out if DEBUGLEVEL1 not defined
/*
VOID _cdecl QCEUD_Print(PCHAR Format, ...)
{
#define DBG_MSG_MAX_SZ 1024
    va_list arguments;
    CHAR   msgBuffer[DBG_MSG_MAX_SZ];

    va_start(arguments, Format);
    _vsnprintf_s(msgBuffer, DBG_MSG_MAX_SZ, _TRUNCATE, Format, arguments);
    va_end(arguments);
#ifndef DEBUGLEVEL1
    return;
#endif
    OutputDebugString(msgBuffer);
}
*/
#define TRC_SIGNAL_BUFFER_SIZE 4
EUD_ERR_t TraceSignaller::WriteData(uint32_t data){
#if defined ( EUD_WIN_ENV )
    //DWORD dwRead, dwWritten;
    uint32_t* buffer_p = new uint32_t[TRC_SIGNAL_BUFFER_SIZE];
    //DWORD* dwRead = new DWORD;
    DWORD* dwWritten_p = new DWORD;
    *dwWritten_p = 0;
    HANDLE* readhandle_p = new HANDLE;
    HANDLE* writehandle_p = new HANDLE;
    LPOVERLAPPED* thing_p = new LPOVERLAPPED;
    ZeroMemory(thing_p, sizeof(OVERLAPPED));
    //SAttrib = new SECURITY_ATTRIBUTES;
    SECURITY_ATTRIBUTES* SAttrib = new SECURITY_ATTRIBUTES;
    SAttrib->nLength = sizeof(SECURITY_ATTRIBUTES);
    SAttrib->bInheritHandle = TRUE;
    SAttrib->lpSecurityDescriptor = NULL;
    //bool result = CreatePipe(readhandle_p, writehandle_p, SAttrib, 2);
    CreatePipe(readhandle_p, writehandle_p, SAttrib, 2);
    //bool bSuccess = WriteFile(*writehandle_p, buffer_p, TRC_SIGNAL_BUFFER_SIZE, dwWritten_p, *thing_p);
    WriteFile(*writehandle_p, buffer_p, TRC_SIGNAL_BUFFER_SIZE, dwWritten_p, *thing_p);
    //bool bSuccess = WriteFile(*this->WriteHandle, buffer_p, TRC_SIGNAL_BUFFER_SIZE, dwWritten, NULL);
    //if (!bSuccess){
    //    return eud_set_last_error(EUD_TRC_ERR_OCURRED_SIGNAL);
    //}
    //TODO - handle exceptions
    return EUD_SUCCESS;
#else 
    return EUD_GENERIC_FAILURE;
#endif    
}


EUD_ERR_t TraceSignaller::ReadData(uint32_t* data){
    //DWORD dwRead, dwWritten;

    DWORD* dwRead = new DWORD;

    #if defined ( EUD_WIN_ENV )
    uint32_t buffer[TRC_SIGNAL_BUFFER_SIZE];
    
    ReadFile(*this->read_handle_, buffer, TRC_SIGNAL_BUFFER_SIZE, dwRead, NULL);
    #endif
    //TODO - handle exceptions
    delete dwRead;
    return EUD_SUCCESS;
}

TraceStopSignal::TraceStopSignal(){
    signal_set_ = false;
}
EUD_ERR_t TraceStopSignal::SetSignal(){
    //semaphore
    this->signal_set_ = true;
    return EUD_SUCCESS;
}
EUD_ERR_t TraceStopSignal::CheckSignal(uint32_t* signalset_p){
    if (this->signal_set_){
        *signalset_p = 1;
        this->signal_set_ = false;
    }
    else{
        *signalset_p = 0;
    }
    return EUD_SUCCESS;
}


USB_ERR_t TraceEudDevice::TraceUsbRead(uint32_t expected_size, uint8_t *data, DWORD *errcode){

    //uint32_t device = 0;

    //TODO wko : retries needed?
    //device = devicetype;
    //if (err = usb_device_handle->ReadFromDevice((PVOID)Trace_Buffer_IN, expected_size, errcode)) return err;
    #if defined ( EUD_WIN_ENV )
    EUD_ERR_t err;
    if ((err = usb_device_handle_->ReadFromDevice((PVOID)data, expected_size, errcode))!=0) return err;
    #endif
    //std::copy(Trace_Buffer_IN, Trace_Buffer_IN + usb_device_handle->bytesRead, data);

    return EUD_USB_SUCCESS;

}
