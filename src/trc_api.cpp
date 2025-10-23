/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   trc_api.cpp
*
* Description:                                                              
*   CPP source file for EUD Trace Peripheral public APIs
*
***************************************************************************/

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <string>
#include <math.h>

#include "trc_eud.h"
#include "jtag_eud.h"
#include "swd_eud.h"
#include "com_eud.h"
#include "ctl_eud.h"
#include "trc_api.h"
#include "eud_api.h"
#include "libusb-1.0/libusb.h"


using namespace std;

#if defined ( EUD_WIN_ENV )
#include <windows.h>
#endif

extern std::string uint32_to_ascii (uint32_t number);
extern uint32_t ascii_to_int (std::string asciinum);
trace_config_data* trace_cfg = new trace_config_data;
TraceEudDevice* trc_device = 0x0;

/**
*   @breif: Function for initialising trace. 
*   @param: device_id, trns_len, trns_tmout, on_time
*   @return: Error status
*/
EXPORT EUD_ERR_t eud_trace_device_init(uint32_t device_id, uint32_t trns_len, uint32_t trns_tmout, uint32_t on_time)
{
    EUD_ERR_t err = EUD_SUCCESS;

    //Initialising Trace Device. 
    trc_device = eud_initialize_device_trace(device_id, 1, &err);
    if (trc_device == NULL)
        QCEUD_Print("trc_device is NULL\n");

    trace_cfg->trace_trns_len = trns_len;
    trace_cfg->trace_trns_tmout = trns_tmout;
    trace_cfg->trace_on_time = on_time;
    trace_cfg->output_file_name = "Trace";

    QCEUD_Print("\nTrace transaction length :%d", trace_cfg->trace_trns_len);
    QCEUD_Print("\nTrace trans timeout  :%d", trace_cfg->trace_trns_tmout);
    QCEUD_Print("\nTrace on time :%d", trace_cfg->trace_on_time);

    //send commands to start trace
    if((err = eud_trace_config(trc_device, trace_cfg)) != EUD_SUCCESS) {
        QCEUD_Print("\n EUD Trace config FAILED!! with Err: %08x\n", err);
    }
    else 
        QCEUD_Print("\nEUD Trace config Success\n");

    if (err != EUD_SUCCESS)
        QCEUD_Print("\nInitialise FAILED!! with Err: %08x\n", err);
    else 
        QCEUD_Print("\nTrace Initialization Successful\n");

    return err;
}

/**
*   @breif: Read traces from EUD Trace peripheral
*   @return: Error status
*/
EXPORT EUD_ERR_t eud_trace_device_read(void) 
{
    return eud_start_tracing(trc_device, trace_cfg);
}
/**********************************************************************
*   @breif: closes trace peripheral. 
*   @param: NULL
*   @return: Error status
***********************************************************************/
EXPORT EUD_ERR_t eud_trace_device_close(void)
{
    EUD_ERR_t err = EUD_SUCCESS;

    if((err = eud_close_trace(trc_device)) != EUD_SUCCESS) {
        QCEUD_Print("CloseTrace Err: %08x\n", err);
    }

    QCEUD_Print("\nEUD Trace device closed"); 

    return err;
}

EXPORT EUD_ERR_t eud_trace_config(TraceEudDevice* trace_handle_p, trace_config_data* trace_cfg ) {

    EUD_ERR_t err = EUD_SUCCESS;
    DWORD * errcode = new DWORD;

    if (trace_handle_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    if ((err = eud_trace_set_timeout_ms(trace_handle_p, trace_cfg->trace_trns_tmout)) != EUD_SUCCESS) { 
        QCEUD_Print ("\n TraceSetTimeout Error");
        return err;
    }
    else 
        QCEUD_Print ("\nTrace transfer time out sent as 0x%x", trace_cfg->trace_trns_tmout);

    //read existing buffer 
    QCEUD_Print ("\nReading existing buffer..");
    trace_handle_p->current_trace_transaction_length_ = trace_cfg->trace_trns_len;
    uint8_t* tracedata = new uint8_t[trace_handle_p->current_trace_transaction_length_ * 2];

    // Flush the ATB buffer
    for (int i = 0; i < 2; i++) {
        QCEUD_Print ("Initial buffer read\n");
        err = trace_handle_p->TraceUsbRead (trace_handle_p->current_trace_transaction_length_ * 2, (tracedata), errcode);

        if(err != EUD_SUCCESS)
        {
            QCEUD_Print ("Could not read anything from buffer..");
            QCEUD_Print ("Err: %08x\n", err);
        }
    }

    //Send Byte transfer length. 
    if ((err = eud_trace_set_transfer_length (trace_handle_p, trace_cfg->trace_trns_len)) != EUD_SUCCESS) {
        QCEUD_Print ("\nError in Sending Transferlength.");
        return err;
    }
    else {
        QCEUD_Print ("\nSetting Trace Transfer Length to %d bytes", trace_cfg->trace_trns_len );
    }
    delete errcode;
    return EUD_SUCCESS;
}
/*
EXPORT EUD_ERR_t EUD_TraceSetCaptureMode(TraceEudDevice* trace_handle_p, uint32_t capturemode){
    EUD_ERR_t err = EUD_SUCCESS;

    if (EUDHandlerWrap == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    if (capturemode > EUD_TRC_MAX_ALLOWED_CAPTUREMODES) {
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }

    TraceEudDevice* myhandle = dynamic_cast<TraceEudDevice*>(EUDHandlerWrap->handle);
    if (myhandle == NULL) return EUD_ERR_BAD_CAST;

    myhandle->CaptureMode = capturemode;

    return EUD_SUCCESS;
    
}
*/

std::string getincrementedfilename(std::string dirname, std::string filepath,bool findmaxflag){

    std::ifstream f(filepath.c_str());

    //if ((f.good() ) || (overwriteflag == true)){
    if (f.good()) {
        //Increment Ascii number within file
        std::string basefilename, suffix, prefix, asciinum;
        basefilename = filepath;
        basefilename.erase(0,dirname.length());
        
        prefix.assign(DEFAULT_TRACE_FILE_PREFIX);
        suffix.assign(DEFAULT_TRACE_FILE_SUFFIX);
        asciinum = basefilename.erase(0, (prefix.length()+1));
        asciinum = asciinum.erase(asciinum.length()-suffix.length(), suffix.length());
        
        //uint32_t number = (int)(*asciinum.c_str()) - (int)('0');
        uint32_t number = ascii_to_int(asciinum);

        //Increment
        number++;

        std::string newasciinum;
        newasciinum = uint32_to_ascii(number);
        newasciinum.erase(0, newasciinum.length() - 1);

        std::string newfilepath;
        newfilepath.assign(dirname + '\\' + prefix + newasciinum + suffix);

        //Finds max possible file. Default to not overwrite files
        if (findmaxflag == true) {
            filepath = getincrementedfilename(dirname, newfilepath, findmaxflag);
        }
        else{
            filepath = newfilepath;
        }
    }

    return filepath;
}
std::string getchunkfile(std::string dirname, bool overwriteflag){

    std::string fullfilepath;

    fullfilepath.assign(dirname + "\\" + DEFAULT_TRACE_FILE_PREFIX + "0" + DEFAULT_TRACE_FILE_SUFFIX);
    
    std::ifstream f(fullfilepath.c_str());

    if (overwriteflag == TRUE) {
        
        if (f.good()) {

            remove(fullfilepath.c_str());

        }

    }
    else{
        bool findmaxflag = true;
        fullfilepath = getincrementedfilename(dirname,fullfilepath, findmaxflag);

    }

    return fullfilepath;
}


volatile bool CloseEventReceived = false;

BOOL CtrlHandler(DWORD fdwCtrlType)
{
#if defined ( EUD_WIN_ENV )
    switch (fdwCtrlType)
    {
        // Handle the CTRL-C signal. 
    case CTRL_C_EVENT:
        CloseEventReceived = true;
        return TRUE;

        // CTRL-CLOSE: confirm that the user wants to exit. 
    case CTRL_CLOSE_EVENT:
        CloseEventReceived = true;
        return FALSE;

        // Pass other signals to the next handler. 
    case CTRL_BREAK_EVENT:
        return FALSE;

    case CTRL_LOGOFF_EVENT:
        return FALSE;

    case CTRL_SHUTDOWN_EVENT:
        return FALSE;

    default:
        return FALSE;
    }
#else 
    return FALSE;
#endif
}



EXPORT EUD_ERR_t eud_start_tracing (TraceEudDevice* trace_handle_p, trace_config_data* trace_cfg) {

    EUD_ERR_t err = EUD_SUCCESS;
    DWORD *errcode = new DWORD;
    
    if (trace_handle_p == NULL) {
        return eud_set_last_error (EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    err = trace_keep(trace_handle_p);
    if(err != EUD_SUCCESS) {
        return err;
    }

    QCEUD_Print("\nKEEP command sent.");

    //Sleep for 5 seconds so that traces can be triggered with trace generation/triggerring tool 
    Sleep(5000);

    uint32_t chunk_idx = 1;
    uint32_t data_captured;
    uint8_t* tracedata = new uint8_t[trace_handle_p->current_trace_transaction_length_ * 2];
    
    #if defined ( EUD_WIN_ENV )
    if (!SetConsoleCtrlHandler ((PHANDLER_ROUTINE)CtrlHandler, TRUE)) {
        return eud_set_last_error (EUD_TRACE_CTRL_HANDLER_REGISTRATION_ERR);
    }
    #endif    

    ofstream file;
    std::string bin_file = trace_cfg->output_file_name;
    bin_file = bin_file + ".bin";
    
    file.open(bin_file,  std::ios::binary);
    if (!file.is_open()) {
        QCEUD_Print("Could not open file to store the binary data.");
        return EUD_GENERIC_FAILURE;
    }

    //Main trace collection loop
    QCEUD_Print("\nTrace collection loop started.");

    time_t start_time = time (NULL), curr_time = time (NULL); 

    struct tm* ptr;
    ptr = gmtime (&start_time);
    QCEUD_Print("\n%s", asctime(ptr));

    memset (tracedata, 0, (trace_handle_p->current_trace_transaction_length_) * 2);
    
    while((difftime(curr_time, start_time)) < trace_cfg->trace_on_time) {
        //Start new trace file.
        data_captured = 0;
        err = trace_handle_p->TraceUsbRead(trace_handle_p->current_trace_transaction_length_, (tracedata), errcode);     
        if(*errcode != LIBUSB_SUCCESS)
        {
            QCEUD_Print("Read Error: %s \n", (PCHAR)libusb_error_name(*errcode));
        }
        if (err != EUD_SUCCESS) {
            QCEUD_Print("\n USB read error 0x%x ", err);
            break;
        }
        
        if(tracedata[0] == 0xFF) {
            //Data ATID is 0xFF, Discarding
            memset (tracedata, 0, trace_handle_p->current_trace_transaction_length_);
        }
        else  {
            for (uint8_t i = 2; i < trace_handle_p->current_trace_transaction_length_; i++) {
                file<<((uint8_t*)tracedata)[i];
            }

            data_captured += trace_handle_p->current_trace_transaction_length_;
        }
        chunk_idx++;
        curr_time = time (NULL);
    }

    ptr = gmtime(&curr_time);
    QCEUD_Print("\n%s", asctime(ptr));
    file.close();
    QCEUD_Print("\nTrace collection finished and trace logs are saved.");
    delete [] tracedata;
    delete errcode;
    return err;
}


EXPORT EUD_ERR_t eud_flush_trace (TraceEudDevice* trace_handle_p) {

    if (trace_handle_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    
    return trace_handle_p->WriteCommand (TRC_CMD_FLUSH);
}

EXPORT EUD_ERR_t eud_stop_trace (TraceEudDevice* trace_handle_p) {
    
    EUD_ERR_t err = EUD_SUCCESS;

    if (trace_handle_p == NULL) {
        return eud_set_last_error (EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    if ((err = trace_handle_p->WriteCommand (TRC_CMD_TOSS))!=EUD_SUCCESS) {
        return err;
    }
    if ((err = trace_handle_p->WriteCommand (TRC_CMD_FLUSH))!=EUD_SUCCESS) {
        return err;
    }
    
    //myhandle->TraceSignal->writedata(TRC_MESSAGE_CLOSE_TRACE);
    #if defined ( EUD_WIN_ENV )
    trace_handle_p->trace_stop_signal_->SetSignal ();
    #endif
    return EUD_SUCCESS;
}


EXPORT EUD_ERR_t eud_close_trace (TraceEudDevice* trace_handle_p)
{

    //ctl->close trace
    if (trace_handle_p == NULL) {
        return eud_set_last_error (EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    
    //myhandle->TraceSignal->writedata(TRC_MESSAGE_CLOSE_TRACE);
    #if defined ( EUD_WIN_ENV )
    EUD_ERR_t err = EUD_SUCCESS;
    err = trace_handle_p->trace_stop_signal_->SetSignal();
    if (err != EUD_SUCCESS) {
        return err;
    }
    #endif

    return eud_close_peripheral((PVOID*)trace_handle_p);
}


EXPORT EUD_ERR_t eud_reinit_trace(TraceEudDevice* trace_handle_p) {

    EUD_ERR_t err = EUD_SUCCESS;
    
    if (trace_handle_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    if ((err = trace_handle_p->WriteCommand(TRC_CMD_TOSS))!=0) return err;
    if ((err = trace_handle_p->WriteCommand(TRC_CMD_FLUSH))!=0) return err;

    //TODO wait for packet indicating flush finished
    if ((err = trace_handle_p->WriteCommand(TRC_CMD_KEEP))!=0) return err;

    return EUD_SUCCESS;
}


EXPORT EUD_ERR_t eud_trace_set_chunk_sizes(TraceEudDevice* trace_handle_p, uint32_t chunksize, uint32_t maxchunks) {
    if (trace_handle_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    if ((chunksize > EUD_TRC_MAX_CHUNKSIZE_PARAMETER) || (chunksize < EUD_TRC_MIN_CHUNKSIZE_PARAMETER)) {
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }

    if ((maxchunks > EUD_TRC_MAX_CHUNKS_PARAMETER) || (maxchunks < EUD_TRC_MIN_CHUNKS_PARAMETER)) {
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }

    if (chunksize < trace_handle_p->current_trace_transaction_length_) {
        return eud_set_last_error(EUD_ERR_TRACE_CHUNKSIZE_LESS_THAN_TRANSLEN);
    }
    trace_handle_p->chunk_size_ = chunksize;
    trace_handle_p->max_chunks_ = maxchunks;

    return EUD_SUCCESS;
}

EXPORT EUD_ERR_t eud_trace_set_output_dir(TraceEudDevice* trace_handle_p, char* outputdirectory) {
    
    if (trace_handle_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    if (outputdirectory == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }
    
#if defined ( EUD_WIN_ENV )
    DWORD attribs = GetFileAttributesA(outputdirectory);
    if (attribs != FILE_ATTRIBUTE_DIRECTORY) {
        if (attribs == FILE_ATTRIBUTE_READONLY) {
            return eud_set_last_error(EUD_ERR_READONLY_DIRECTORY_GIVEN);
        }

        try{
            CreateDirectory(outputdirectory, NULL);
        }
        catch (int) {
            return eud_set_last_error(EUD_ERR_INVALID_DIRECTORY_GIVEN);
        }
        DWORD attribs = GetFileAttributesA(outputdirectory);
        if (attribs != FILE_ATTRIBUTE_DIRECTORY) {
            return eud_set_last_error(EUD_ERR_INVALID_DIRECTORY_GIVEN);
        }
    }

    //Try making a directory  there to check that it's writeable
    std::string testdirectory;
    testdirectory.assign(outputdirectory);
    testdirectory.append(TRACETESTDIRECTORY);
    try{
        CreateDirectory(testdirectory.c_str(), NULL);
    }
    catch (int) {
        return  eud_set_last_error(EUD_ERR_INVALID_DIRECTORY_GIVEN);
    }
    attribs = GetFileAttributesA(outputdirectory);
    if (attribs != FILE_ATTRIBUTE_DIRECTORY) {
        return eud_set_last_error(EUD_ERR_INVALID_DIRECTORY_GIVEN);
    }
    RemoveDirectory(testdirectory.c_str());


    trace_handle_p->output_dir_.assign(outputdirectory);
#endif
    return EUD_SUCCESS;
}

EXPORT EUD_ERR_t eud_trace_get_output_dir(TraceEudDevice* trace_handle_p, char* outputdirectory, uint32_t* stringsize_p) {
    
    if (trace_handle_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    if (outputdirectory == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }
    if (stringsize_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }

    *stringsize_p = (uint32_t)trace_handle_p->output_dir_.length();

    memcpy(outputdirectory,trace_handle_p->output_dir_.c_str(), *stringsize_p);

    return EUD_SUCCESS;
}


EXPORT EUD_ERR_t eud_trace_set_timeout_ms (TraceEudDevice* trace_handle_p, uint32_t MS_Timeout_Val) {

    EUD_ERR_t err = EUD_SUCCESS;

    if (trace_handle_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    if ((MS_Timeout_Val > TRACE_TRNS_TMOUT_MAX) || (MS_Timeout_Val < TRACE_TRNS_TMOUT_MIN)) {
        QCEUD_Print ("\nPlease enter valid Trace Tansaction timeout.");
        return eud_set_last_error (EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    if ((err = trace_trns_tmout (trace_handle_p, MS_Timeout_Val))!=EUD_SUCCESS) {
        return err;
    }
    trace_handle_p->current_trace_timeout_value_ = MS_Timeout_Val;

    return EUD_SUCCESS;
}


EXPORT EUD_ERR_t eud_trace_set_transfer_length (TraceEudDevice* trace_handle_p, uint32_t TransactionLength) {

    EUD_ERR_t err = EUD_SUCCESS;

    if (trace_handle_p == NULL) {
        return eud_set_last_error (EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    if (TransactionLength > TRACE_MAX_TRANSLEN_SIZE || TransactionLength < TRACE_MIN_TRANSLEN_SIZE) {
        return eud_set_last_error (EUD_BAD_PARAMETER);
    }

    if (TransactionLength > trace_handle_p->chunk_size_) {
        return eud_set_last_error (EUD_TRC_TRANSLEN_GREATER_THAN_CHUNKSIZE);
    }

    if (TransactionLength % 128 == 0x0) {
        return eud_set_last_error (EUD_TRC_TRANSLEN_CANNOT_BE_MULTIPLE_128);
    }
    
    TransactionLength &= TRACE_TRNS_LEN_MSK;
    if((err = trace_trns_len (trace_handle_p, TransactionLength))!=EUD_SUCCESS) {
        return err;
    }
    trace_handle_p->current_trace_transaction_length_ = TransactionLength;

    return EUD_SUCCESS;
}

EXPORT EUD_ERR_t eud_trace_reset(TraceEudDevice* trace_handle_p) {

    EUD_ERR_t err = EUD_SUCCESS;

    if (trace_handle_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    trace_handle_p->WriteCommand(TRC_CMD_TOSS);
    trace_handle_p->WriteCommand(TRC_CMD_FLUSH);

    //wait for flush response or timeout
    trace_handle_p->WriteCommand(TRC_CMD_PERIPH_RST);
    
    if ((err = trace_handle_p->WriteCommand(TRC_CMD_TRNS_LEN, trace_handle_p->current_trace_transaction_length_))!=0) return err;
    if ((err = trace_handle_p->WriteCommand(TRC_CMD_TRNS_TMOUT, trace_handle_p->current_trace_timeout_value_))!=0) return err;
    
    return EUD_SUCCESS;
}


//===---------------------------------------------------------------------===//
//
// Trace direct API's
//
//===---------------------------------------------------------------------===//

EXPORT EUD_ERR_t trace_nop(TraceEudDevice* trc_handler_p) {
    /**No operation.  All commands following the first NOP command in an OUT packet are ignored.*/
    
    if (trc_handler_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    return trc_handler_p->WriteCommand(TRC_CMD_NOP);
    
}

EXPORT EUD_ERR_t trace_flush(TraceEudDevice* trc_handler_p) {
    /**Upon receipt of this command, Trace Peripheral asserts afvalid to the ATB master, forcing a flush.Once ATB master has flushed, it asserts afready.Trace Peripheral then sends either short packet or zero length packet after next IN token, and clears the transfer length counter.
    All commands following the first FLUSH command in an OUT packet are ignored.
    Upon completion of a FLUSH command(ie afready asserted and zero or partial length packet sent to PC), the transfer packet counter is reset.*/

    if (trc_handler_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    return trc_handler_p->WriteCommand(TRC_CMD_FLUSH);

}

EXPORT EUD_ERR_t trace_toss(TraceEudDevice* trc_handler_p) {
    /**Upon receipt of this command, the Trace Peripheral keeps the atready signal on the ATB interface asserted, and ignores any data from the ATB.  The IN buffer is not cleared and the transfer counter is not reset.
    This command can be used before the trace_flush command if the flush data does not need to be transferred to the host PC.
    After reset, the Trace Peripheral is in trace_toss mode.*/

    if (trc_handler_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    return trc_handler_p->WriteCommand(TRC_CMD_TOSS);
}

EXPORT EUD_ERR_t trace_keep (TraceEudDevice* trc_handler_p) {
    /**Upon receipt of this command, the Trace Peripheral sends all data from the ATB to the host PC. After reset, the Trace Peripheral is in trace_toss mode.*/

    if (trc_handler_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    return trc_handler_p->WriteCommand (TRC_CMD_KEEP);
}

EXPORT EUD_ERR_t trace_peripheral_reset(TraceEudDevice* trc_handler_p) {
    /**This command resets the Trace Peripheral while still keeping it enumerated with the host.  The OUT and IN buffers are cleared, and programmable parameters are restored to their default values.
    The operator is responsible for coordinating this reset with other processes on the chip that might be interfacing to this peripheral.
    The Trace Peripheral never NAKs OUT tokens, so the TRACE_PERIPH_RST command is always executed immediately.*/

    if (trc_handler_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    return trc_handler_p->WriteCommand(TRC_CMD_PERIPH_RST);
}

EXPORT EUD_ERR_t trace_trns_len (TraceEudDevice* trc_handler_p, uint32_t  TransactionLength) {

    EUD_ERR_t err; 
    if (trc_handler_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    err = trc_handler_p->WriteCommand(TRC_CMD_TRNS_LEN, TransactionLength);
    return err;
}

EXPORT EUD_ERR_t trace_trns_tmout (TraceEudDevice* trc_handler_p, uint32_t MS_Timeout_Val) {

    EUD_ERR_t err; 
    if (trc_handler_p == NULL) {
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    err = trc_handler_p->WriteCommand(TRC_CMD_TRNS_TMOUT, MS_Timeout_Val);
    return err;
}
