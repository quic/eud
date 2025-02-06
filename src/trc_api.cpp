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

#if defined ( EUD_WIN_ENV )
#include <windows.h>
#endif

extern std::string uint32_to_ascii (uint32_t number);
extern uint32_t ascii_to_int (std::string asciinum);


//===---------------------------------------------------------------------===//
//
// Trace Usecase API's
//
//===---------------------------------------------------------------------===//
//#define DEFAULT_FILENAME_TRACESIMPLE "eud_trace_output.txt"
//#define EUD_ERR_TRC_FILE_OPEN_FAILED 3
//EXPORT EUD_ERR_t eud_start_tracingSimple(char* filedir){
//
//  std::string filename = std::string(filedir) + std::string(DEFAULT_FILENAME_TRACESIMPLE);
//  std::ofstream input_stream(filename);
//  if (!input_stream){
//      return EUD_ERR_TRC_FILE_OPEN_FAILED;
//  }
//
//
//
//
//
//}

//EXPORT EUD_ERR_t eud_open_trace(uint32_t* tracebuffer, uint32_t index, uint32_t datareadyflag){
EXPORT EUD_ERR_t eud_open_trace(TraceEudDevice* trace_handle_p)
{
    EUD_ERR_t err;

    if (trace_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    
    //Trace_periph_rst
    if ((err = trace_handle_p->WriteCommand(TRC_CMD_PERIPH_RST))!=0) return err;
    //trace_trans_length
    //if (err = eud_trace_set_transfer_length(EUDHandlerWrap, TRACE_DEFAULT_TRANSACTION_LENGTH)) return err;
    if ((err = trace_handle_p->WriteCommand(TRC_CMD_TRNS_LEN, trace_handle_p->current_trace_transaction_length_))!=0) return err;
    //Trace_trans_timeout
    //if (err = eud_trace_set_timeout_ms(EUDHandlerWrap,TRACE_DEFAULT_TRNS_TMOUT)) return err;
    if ((err = trace_handle_p->WriteCommand(TRC_CMD_TRNS_TMOUT, trace_handle_p->current_trace_timeout_value_))!=0) return err;

    //Set chunk size parameters
    if ((err = eud_trace_set_chunk_sizes(trace_handle_p, EUD_TRC_DEFAULT_CHUNKSIZE, EUD_TRC_DEFAULT_MAXCHUNKS))!=0) return err;

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
    if (f.good()){
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
        if (findmaxflag == true){
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

    if (overwriteflag == TRUE){
        
        if (f.good()){

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
#endif    
}



EXPORT EUD_ERR_t eud_start_tracing(TraceEudDevice* trace_handle_p){

    EUD_ERR_t err = EUD_SUCCESS;
    
    if (trace_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    
    std::string first_outputbinary = getchunkfile(trace_handle_p->output_dir_, FALSE);
    
    trace_handle_p->WriteCommand(TRC_CMD_KEEP);
    Sleep(500);
    //Main trace collection loop
    uint32_t chunk_idx;
    uint32_t data_captured;
    uint8_t* tracedata = new uint8_t[trace_handle_p->current_trace_transaction_length_ * 2];
    
    #if defined ( EUD_WIN_ENV )
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE)){
        return eud_set_last_error(EUD_TRACE_CTRL_HANDLER_REGISTRATION_ERR);
    }
    #endif    

    while (1){
            //We'll only loop back to this again if in circular buffer mode.
            std::string outputbinary = first_outputbinary;

            for (chunk_idx = 0; chunk_idx < trace_handle_p->max_chunks_; chunk_idx++){

                //Start new trace file.
                std::ofstream ofile(outputbinary, std::ios::binary);
                
                data_captured = 0;
                while ((CloseEventReceived == false) && (err == EUD_SUCCESS) && (data_captured < trace_handle_p->chunk_size_)){
                    if (trace_handle_p->current_trace_transaction_length_ < 200){
                        Sleep(250);
                    }
                    err = trace_handle_p->UsbRead(trace_handle_p->current_trace_transaction_length_ * 2, (tracedata)); //FIXME - undo hardcode
                    //trace_handle_p->WriteCommand(TRC_CMD_FLUSH);
                    //err = trace_handle_p->UsbRead(trace_handle_p->current_trace_transaction_length_ * 2, (tracedata)); //FIXME - undo hardcode
                    //trace_handle_p->WriteCommand(TRC_CMD_TOSS);
                    //trace_handle_p->WriteCommand(TRC_CMD_FLUSH);
                    //trace_handle_p->WriteCommand(TRC_CMD_KEEP);
                    //err = trace_handle_p->UsbRead(trace_handle_p->current_trace_transaction_length_ * 2, (tracedata)); //FIXME - undo hardcode
                    if (err == EUD_USB_ERROR_READ_TIMEOUT){
                        Sleep(500);
                        err = trace_handle_p->UsbRead(trace_handle_p->current_trace_transaction_length_ * 2, (tracedata)); //FIXME - undo hardcode
                        //if (err == EUD_USB_ERROR_READ_TIMEOUT){
                        //    if (err = trace_handle_p->WriteCommand(TRC_CMD_FLUSH)){
                        //      ofile.close();
                        //      delete tracedata;
                        //      return eud_set_last_error(err);
                        //  }
                        //    err = trace_handle_p->UsbRead(trace_handle_p->current_trace_transaction_length_ * 2, (tracedata)); //FIXME - undo hardcode
                        //}
                    }

                    if (err != EUD_SUCCESS) {
                        ofile.close();
                        delete[] tracedata;
                        //Use special error message for trace timeout.
                        if (err == EUD_USB_ERROR_READ_TIMEOUT) {
                            err = eud_set_last_error(EUD_ERROR_TRACE_DATA_STOPPED);
                        }
                        return err;
                    }
                    ofile.write((const char*)tracedata, trace_handle_p->current_trace_transaction_length_);
                    data_captured += trace_handle_p->current_trace_transaction_length_;
                    
                    //If we got a signal to stop tracing.
                    uint32_t closeflag = 0;
                    //err = myhandle->TraceSignal->readdata(&closeflag);
                    #if defined ( EUD_WIN_ENV )
                    err = trace_handle_p->trace_stop_signal_->CheckSignal(&closeflag);
                    #endif
                    if (closeflag != 0){
                        ofile.close();
                        delete[] tracedata;
                        return err;
                    }

                }

                ofile.close();

                //Exit if requested.
                if (CloseEventReceived == true){
                    return EUD_SUCCESS;
                }

                //Increment output file.
                outputbinary = getincrementedfilename(trace_handle_p->output_dir_, outputbinary, false);
            }

            if (trace_handle_p->capture_mode_ == EUD_TRC_SEQUENTIAL_MODE){
                break;
            }

    }
    
    return EUD_SUCCESS;
}


EXPORT EUD_ERR_t eud_flush_trace(TraceEudDevice* trace_handle_p){

    if (trace_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    
    return trace_handle_p->WriteCommand(TRC_CMD_FLUSH);
}

EXPORT EUD_ERR_t eud_stop_trace(TraceEudDevice* trace_handle_p){
    
    EUD_ERR_t err = EUD_SUCCESS;

    if (trace_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    if ((err = trace_handle_p->WriteCommand(TRC_CMD_TOSS))!=0) return err;
    if ((err = trace_handle_p->WriteCommand(TRC_CMD_FLUSH))!=0) return err;
    
    //myhandle->TraceSignal->writedata(TRC_MESSAGE_CLOSE_TRACE);
    #if defined ( EUD_WIN_ENV )
    trace_handle_p->trace_stop_signal_->SetSignal();
    #endif
    return EUD_SUCCESS;
}


EXPORT EUD_ERR_t eud_close_trace(TraceEudDevice* trace_handle_p){
    //ctl->close trace
    if (trace_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    
    //myhandle->TraceSignal->writedata(TRC_MESSAGE_CLOSE_TRACE);
    #if defined ( EUD_WIN_ENV )
    trace_handle_p->trace_stop_signal_->SetSignal();
    #endif
    eud_close_peripheral((PVOID*)trace_handle_p);
    
    return EUD_SUCCESS;
}


EXPORT EUD_ERR_t eud_reinit_trace(TraceEudDevice* trace_handle_p){

    EUD_ERR_t err = EUD_SUCCESS;
    
    if (trace_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    if ((err = trace_handle_p->WriteCommand(TRC_CMD_TOSS))!=0) return err;
    if ((err = trace_handle_p->WriteCommand(TRC_CMD_FLUSH))!=0) return err;

    //TODO wait for packet indicating flush finished
    if ((err = trace_handle_p->WriteCommand(TRC_CMD_KEEP))!=0) return err;

    return EUD_SUCCESS;
}


EXPORT EUD_ERR_t eud_trace_set_chunk_sizes(TraceEudDevice* trace_handle_p, uint32_t chunksize, uint32_t maxchunks){
    if (trace_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    if ((chunksize > EUD_TRC_MAX_CHUNKSIZE_PARAMETER) || (chunksize < EUD_TRC_MIN_CHUNKSIZE_PARAMETER)){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }

    if ((maxchunks > EUD_TRC_MAX_CHUNKS_PARAMETER) || (maxchunks < EUD_TRC_MIN_CHUNKS_PARAMETER)){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }

    if (chunksize < trace_handle_p->current_trace_transaction_length_){
        return eud_set_last_error(EUD_ERR_TRACE_CHUNKSIZE_LESS_THAN_TRANSLEN);
    }
    trace_handle_p->chunk_size_ = chunksize;
    trace_handle_p->max_chunks_ = maxchunks;

    return EUD_SUCCESS;
}

EXPORT EUD_ERR_t eud_trace_set_output_dir(TraceEudDevice* trace_handle_p, char* outputdirectory){
    
    if (trace_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    if (outputdirectory == NULL){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }
    
#if defined ( EUD_WIN_ENV )
    DWORD attribs = GetFileAttributesA(outputdirectory);
    if (attribs != FILE_ATTRIBUTE_DIRECTORY){
        if (attribs == FILE_ATTRIBUTE_READONLY){
            return eud_set_last_error(EUD_ERR_READONLY_DIRECTORY_GIVEN);
        }

        try{
            CreateDirectory(outputdirectory, NULL);
        }
        catch (int){
            return eud_set_last_error(EUD_ERR_INVALID_DIRECTORY_GIVEN);
        }
        DWORD attribs = GetFileAttributesA(outputdirectory);
        if (attribs != FILE_ATTRIBUTE_DIRECTORY){
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
    catch (int){
        return  eud_set_last_error(EUD_ERR_INVALID_DIRECTORY_GIVEN);
    }
    attribs = GetFileAttributesA(outputdirectory);
    if (attribs != FILE_ATTRIBUTE_DIRECTORY){
        return eud_set_last_error(EUD_ERR_INVALID_DIRECTORY_GIVEN);
    }
    RemoveDirectory(testdirectory.c_str());


    trace_handle_p->output_dir_.assign(outputdirectory);
#endif
    return EUD_SUCCESS;
}

EXPORT EUD_ERR_t eud_trace_get_output_dir(TraceEudDevice* trace_handle_p, char* outputdirectory, uint32_t* stringsize_p){
    
    if (trace_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    if (outputdirectory == NULL){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }
    if (stringsize_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }

    *stringsize_p = (uint32_t)trace_handle_p->output_dir_.length();

    memcpy(outputdirectory,trace_handle_p->output_dir_.c_str(), *stringsize_p);

    return EUD_SUCCESS;
}


EXPORT EUD_ERR_t eud_trace_set_timeout_ms(TraceEudDevice* trace_handle_p, uint32_t MS_Timeout_Val){

    EUD_ERR_t err = EUD_SUCCESS;

    if (trace_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    if ((MS_Timeout_Val > TRACE_TRNS_TMOUT_MAX) || (MS_Timeout_Val < TRACE_TRNS_TMOUT_MIN)){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }

    MS_Timeout_Val &= TRACE_TRNS_TMOUT_MSK;
    if ((err = trace_handle_p->WriteCommand(TRC_CMD_TRNS_TMOUT, MS_Timeout_Val))!=0) return err;

    // trace_handle_p->current_trace_timeout_value_ = MS_Timeout_Val;

    return EUD_SUCCESS;
}


EXPORT EUD_ERR_t eud_trace_set_transfer_length(TraceEudDevice* trace_handle_p, uint32_t TransactionLength){

    EUD_ERR_t err = EUD_SUCCESS;

    if (trace_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    if (TransactionLength > TRACE_MAX_TRANSLEN_SIZE || TransactionLength < TRACE_MIN_TRANSLEN_SIZE){
        return eud_set_last_error(EUD_BAD_PARAMETER);
    }

    if (TransactionLength > trace_handle_p->chunk_size_){
        return eud_set_last_error(EUD_TRC_TRANSLEN_GREATER_THAN_CHUNKSIZE);
    }

    if (TransactionLength % 128 == 0x0){
        return eud_set_last_error(EUD_TRC_TRANSLEN_CANNOT_BE_MULTIPLE_128);
    }
    
    TransactionLength &= TRACE_TRNS_LEN_MSK;

    if ((err = trace_handle_p->WriteCommand(TRC_CMD_TRNS_LEN, TransactionLength))!=0) return err;

    trace_handle_p->current_trace_transaction_length_ = TransactionLength;

    return EUD_SUCCESS;
}

EXPORT EUD_ERR_t eud_trace_reset(TraceEudDevice* trace_handle_p){

    EUD_ERR_t err = EUD_SUCCESS;

    if (trace_handle_p == NULL){
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
    
    if (trc_handler_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    return trc_handler_p->WriteCommand(TRC_CMD_NOP);
    
}

EXPORT EUD_ERR_t trace_flush(TraceEudDevice* trc_handler_p) {
    /**Upon receipt of this command, Trace Peripheral asserts afvalid to the ATB master, forcing a flush.Once ATB master has flushed, it asserts afready.Trace Peripheral then sends either short packet or zero length packet after next IN token, and clears the transfer length counter.
    All commands following the first FLUSH command in an OUT packet are ignored.
    Upon completion of a FLUSH command(ie afready asserted and zero or partial length packet sent to PC), the transfer packet counter is reset.*/

    if (trc_handler_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    return trc_handler_p->WriteCommand(TRC_CMD_FLUSH);

}

EXPORT EUD_ERR_t trace_toss(TraceEudDevice* trc_handler_p) {
    /**Upon receipt of this command, the Trace Peripheral keeps the atready signal on the ATB interface asserted, and ignores any data from the ATB.  The IN buffer is not cleared and the transfer counter is not reset.
    This command can be used before the trace_flush command if the flush data does not need to be transferred to the host PC.
    After reset, the Trace Peripheral is in trace_toss mode.*/

    if (trc_handler_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    return trc_handler_p->WriteCommand(TRC_CMD_TOSS);
}

EXPORT EUD_ERR_t trace_keep(TraceEudDevice* trc_handler_p) {
    /**Upon receipt of this command, the Trace Peripheral sends all data from the ATB to the host PC. After reset, the Trace Peripheral is in trace_toss mode.*/

    if (trc_handler_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    return trc_handler_p->WriteCommand(TRC_CMD_KEEP);
}

EXPORT EUD_ERR_t trace_peripheral_reset(TraceEudDevice* trc_handler_p) {
    /**This command resets the Trace Peripheral while still keeping it enumerated with the host.  The OUT and IN buffers are cleared, and programmable parameters are restored to their default values.
    The operator is responsible for coordinating this reset with other processes on the chip that might be interfacing to this peripheral.
    The Trace Peripheral never NAKs OUT tokens, so the TRACE_PERIPH_RST command is always executed immediately.*/

    if (trc_handler_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    return trc_handler_p->WriteCommand(TRC_CMD_PERIPH_RST);
}
