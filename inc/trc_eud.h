/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   treace_eud.h
*
* Description:                                                              
*   Header file for EUD Trace peripheral class and other defines
*   Note: Trace Peripheral not completed due to hardware limitations 
*
***************************************************************************/
#ifndef TRC_EUD_H
#define TRC_EUD_H

#include "eud.h"

//===---------------------------------------------------------------------===//
//
//  Top level defines, typedef's
//
//===---------------------------------------------------------------------===//

///Initialization options mask. No options allowed at this time.
#define INIT_TRC_ALLOWED_OPTIONS_MSK    0 

///Trace peripheral opcodes
#define TRC_CMD_NOP                     0x0
#define TRC_CMD_FLUSH                   0x1
#define TRC_CMD_TRNS_LEN                0x2
#define TRC_CMD_TRNS_TMOUT              0x3
#define TRC_CMD_TOSS                    0x4
#define TRC_CMD_KEEP                    0x5
#define TRC_CMD_PERIPH_RST              0x6

#define TRC_PCKT_SZ_SEND_NOP            1
#define TRC_PCKT_SZ_SEND_FLUSH          1
#define TRC_PCKT_SZ_SEND_TRNS_LEN       5
#define TRC_PCKT_SZ_SEND_TRNS_TMOUT     5
#define TRC_PCKT_SZ_SEND_TOSS           1
#define TRC_PCKT_SZ_SEND_KEEP           1
#define TRC_PCKT_SZ_SEND_PERIPH_RST     1

#define TRC_PCKT_SZ_RECV_NOP            0
#define TRC_PCKT_SZ_RECV_FLUSH          0
#define TRC_PCKT_SZ_RECV_TRNS_LEN       0
#define TRC_PCKT_SZ_RECV_TRNS_TMOUT     0
#define TRC_PCKT_SZ_RECV_TOSS           0
#define TRC_PCKT_SZ_RECV_KEEP           0
#define TRC_PCKT_SZ_RECV_PERIPH_RST     0

////////////////////Endian  Option///////////////////
#define TRC_ENDIANOPT_NOP                   BIGENDIAN
#define TRC_ENDIANOPT_FLUSH                 BIGENDIAN
#define TRC_ENDIANOPT_TRNS_LEN              BIGENDIAN
#define TRC_ENDIANOPT_TRNS_TMOUT            BIGENDIAN
#define TRC_ENDIANOPT_TOSS                  BIGENDIAN
#define TRC_ENDIANOPT_KEEP                  BIGENDIAN
#define TRC_ENDIANOPT_PERIPH_RST            BIGENDIAN

#define TRC_NUM_OPCODES                 7
/////////////////Trace API masks and defaults/////////////
#define TRACE_TRNS_TMOUT_MSK                0x7FFFFF
#define TRACE_TRNS_TMOUT_MAX                TRACE_TRNS_TMOUT_MSK
#define TRACE_TRNS_TMOUT_MIN                0x1
#define TRACE_TRNS_LEN_MSK                  0xFFFF

#define TRACE_DEFAULT_TRNS_TMOUT            8*500           /// ==500*8*125us == ~1/2 second. Default timeout value.

#define TRACE_DEFAULT_TRANSACTION_LENGTH    100             /// Default transaction length to be passed to EUD TRC peripheral.

#define TRACE_MAX_TRANSLEN_SIZE             3*1024                      ///<Max  static trace buffer size. 
#define TRACE_MIN_TRANSLEN_SIZE             80                          ///<Minimum  static trace buffer size. 
#define TRACE_MAX_BUFFER_SIZE               TRACE_MAX_TRANSLEN_SIZE*2   ///<Size of maximum  needed buffer size for trace

#define EUD_TRC_DEFAULT_CHUNKSIZE           1024*1024*1     ///Default chunk size to write trace data to. Default is 1MB.

#define EUD_TRC_DEFAULT_MAXCHUNKS           10              ///Max number of chunks. 10 is default.

#define EUD_TRC_MAX_CHUNKS_PARAMETER        1000            ///Maxchunks largest value allowed is 1000.
#define EUD_TRC_MIN_CHUNKS_PARAMETER        1               ///Maxchunks lowest  value allowed is 1.

#define EUD_TRC_MAX_CHUNKSIZE_PARAMETER     1024*1024*100   ///Max allowed chunksize is 100MB.
#define EUD_TRC_MIN_CHUNKSIZE_PARAMETER     100             ///Min allowed chunksize is 100.

#define EUD_TRC_DEFAULT_OUTPUTDIR           "C:\\temp\\tracedata"
#define  TRACETESTDIRECTORY                 "\\testdir"

#define EUD_TRC_CIRCULAR_BUFFER_MODE        0
#define EUD_TRC_SEQUENTIAL_MODE             1
#define EUD_TRC_DEFAULT_CAPTURE_MODE        EUD_TRC_CIRCULAR_BUFFER_MODE
#define EUD_TRC_MAX_ALLOWED_CAPTUREMODES    1

#define DEFAULT_TRACE_FILE_PREFIX           "eud_trace_data_"
#define DEFAULT_TRACE_FILE_SUFFIX           ".bin"

///Trace pipe data to be passed to close  trace from separate threads
#define TRC_MESSAGE_CLOSE_TRACE             1


//To Deprecate
#define TRACE_DEFAULT_MAX_READ_COUNT        10

#ifdef  __cplusplus
class TraceSignaller{
public:
    TraceSignaller();
    ~TraceSignaller();
    EUD_ERR_t WriteData(uint32_t data);
    EUD_ERR_t ReadData(uint32_t* data);

private:
    #if defined (EUD_WIN_ENV)
        PHANDLE read_handle_ = NULL;
        PHANDLE write_handle_ = NULL;
        LPSECURITY_ATTRIBUTES sec_attrib_;
    #endif
};




class TraceStopSignal{
public:
    TraceStopSignal();
    //~tracestopsignal();
    EUD_ERR_t SetSignal();
    EUD_ERR_t CheckSignal(uint32_t* signalsent);
private:
    bool signal_set_;
};


//===---------------------------------------------------------------------===//
//
//  Structures, enums, data variables
//
//===---------------------------------------------------------------------===//

///Trace EUD Device class. Inherits from EudDevice class.
///Main differences are larger buffer (Trace_USB_Buffer) and a few
///eud trace specific parameters.
class TraceEudDevice: public EudDevice
{
public:
    TraceEudDevice();   ///<Constructor
    ~TraceEudDevice();  ///<Destructor

    #if defined (EUD_WIN_ENV)
        TraceStopSignal* trace_stop_signal_;
    #endif
    ///Trace peripheral version of usb_read. Passes in the larger Trace_Buffer_IN
    ///instead of usb_buffer_in_ in order to handle larger data sizes.
    virtual USB_ERR_t UsbRead( uint32_t expected_size, ///<Number of bytes to be read
                                uint8_t *data           ///<Data buffer to populate
                            );

    ///Shadow copy of Trace timeout value in EUD hardware. 
    ///Each time a transfer is completed, the transfer timer is re - started.
    ///If the transfer does not complete before the time expires, the Trace Peripheral 
    ///terminates the transfer by sending either a zero or partial length packet and 
    ///resetting the transfer packet counter and the transfer timer.
    uint32_t current_trace_timeout_value_;

    ///Shadow copy of Trace Transfer Length in EUD hardware.
    ///Transaction Length indicates the number of bytes in a USB transfer from Trace
    ///peripheral to host. Note that Transaction_Length cannot be a multiple of 128,
    ///nor can it be equal to an integer number of packets plus on, nor an integer number
    ///of packets plus two.
    uint32_t current_trace_transaction_length_;

    ///Trace buffer. Basically a larger version of usb_buffer_in_ found in EudDevice class.
    ///Needed for larger trace transfers.
    uint8_t trace_buffer_in_[TRACE_MAX_BUFFER_SIZE];

    ///Two modes are available during trace capture - circular buffer and sequential.
    ///Circular buffer will write traces in ChunkSize up to MaxChunks, and then start 
    ///overwriting trace files.
    ///Sequential Capture mode will write traces in ChunkSize up to MaxChunks, and then stop.
    uint32_t capture_mode_ = EUD_TRC_DEFAULT_CAPTURE_MODE;

    ///Trace software will capture trace and save it in files of approximately ChunkSize.
    uint32_t chunk_size_ = EUD_TRC_DEFAULT_CHUNKSIZE;

    ///Trace software will capture trace and save it in files of approximately ChunkSize,
    ///with MaxChunks being the number of 'chunks' saved. Depending on CaptureMode, trace
    ///will either start rewriting chunks from the beginning, or stop capture.
    uint32_t max_chunks_ = EUD_TRC_DEFAULT_MAXCHUNKS;

    
    std::string output_dir_;
};
#else   // __cplusplus
typedef void TraceEudDevice;
#endif  // __cplusplus

#endif //TRC_EUD_H


