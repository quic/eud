/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   treace_api.h
*
* Description:                                                              
*   Header file for EUD Trace peripheral public APIs
*   Note: Trace Peripheral not completed and is under develepment
*
***************************************************************************/
#ifndef trc_api_H_
#define trc_api_H_

#include "eud_top_defines_internal.h"
#include "eud_error_defines.h"

//=====================================================================================//
//                      EUD Trace Peripheral Interface
//                      --------------------------
//           To initialize and use EUD Trace peripheral, the following must be done:

//              1. Call eud_initialize_device_trace(0x0,uint32_t* errcode) to get trace handle.
//              2. Call eud_open_trace(tracehandle)
//                 The following are optional. If they are not specified, eud_open_trace()
//                 will initialize trace with default values (Note that you can use 
//                 eud_trace_get_output_directory to retrieve trace files).
//                 a. Call eud_trace_set_timeout_ms(tracehandle,uint32_t timeout_ms)
//                    Sets EUD Trace peripheral's Trace timeout value in milliseconds.
//
//                 b. Call eud_trace_set_transfer_length(tracehandle,uint32_t tranferlength)
//                    Sets transaction length from EUD Trace to host machine. Cannot
//                    be a multiple of 128.
//
//                 // EUD_TraceSetCaptureMode not yet implemented
//                 //c. Call EUD_TraceSetCaptureMode(tracehandle,uint32_t capturemode)
//                 //   Changes capture mode (0 - circular buffer, 1 - sequential)
//
//                 d. Call eud_trace_set_chunk_sizes(tracehandle,uint32_t chunksize, uint32_t maxchunks)
//                    Specify size of trace chunks and how many chunks to write before
//                    restarting trace or stopping trace (depending on capture mode).
//
//                 e. Call eud_trace_set_output_dir(tracehandle,char* outputdirectory)
//                    Sets the output directory for trace to save binaries to.
//
//              3. Call eud_start_tracing(tracehandle)
//                 This will immediately begin gathing traces. Stop the sequence by sending  a  'Ctrl-C'.
//                 If a timeout occurs, a trace flush will be attempted and resume tracing.
//                 If a second timeout occurs, traces will be saved and function will exit.
//
//              4. Call eud_close_trace(tracehandle)
//                 This will shut down EUD Trace peripheral on target side.
//
//
//=====================================================================================//

#include "trc_eud.h"

typedef struct {
    uint32_t eud_dev_id;
    uint32_t trace_trns_len;
    uint32_t trace_trns_tmout;
    uint32_t trace_on_time;
    #ifdef __cplusplus
        std::string output_file_name;
    #endif //__cplusplus
    uint32_t wait;
} trace_config_data;


/**
*   @breif: Function for initialising trace. 
*   @param: NULL
*   @return: Error status
*/
EXPORT EUD_ERR_t eud_trace_device_init(uint32_t device_id, uint32_t trns_len, uint32_t trns_tmout, uint32_t on_time);

/**
*  EUD_OpenTrace(TraceEudDevice* trace_handle_p,trace_config_data* json_trace_data);
*
*   @brief: Sets up EUD Trace peripheral for receiving traces. Populates TraceUSBStructure with USB parameters
*           for caller to directly read trace data from USB. Call EUD_StartTrace() for trace peripheral to start
*           recording traces. Call eud_initialize_device_trace() prior to this to get pointer to EUDHandlerWrap.
*
*   @param: trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance.
*           json_trace_data - pointer to a structure that stores trace 
*                                      configuration data like trace trsn time on time etc. 
*   @return: EUD_ERR_t - Error code. 0 for success. Call EUDGetErrorString for ASCII description.
*
*/
EXPORT EUD_ERR_t eud_trace_config(
    TraceEudDevice* trace_handle_p,  
    trace_config_data* json_trace_data
);

/**
*   @breif: Function that collects traces and saves it in the file. 
*   @param: TraceEudDevice* trace_handle_p - pointer to EUD Handle instance.   
*           trace_config_data* json_trace_data - stores trace configuration.
*   @return: EUD_ERR_t - Error status
*/
EXPORT EUD_ERR_t eud_start_tracing(
    TraceEudDevice* trace_handle_p,  
    trace_config_data* json_trace_data
);

/**
*
*   @brief Traces are collected in chunks of <chunksize>, up to maximum of <maxchunks>. 
*   These parameters are specified here.
*
*   @param: trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance.   
*           chunksize - This indicates the size of chunks to be saved.Default is 1 MegaByte.Max is 10 MegaBytes.
*           maxchunks - This indicates the maximum size (sum of chunks) to be collected. Default is 10 MegaBytes. Max is 1 Gigabyte.
*   @return: EUD_ERR_t -  Error status
*
*/
EXPORT EUD_ERR_t eud_trace_set_chunk_sizes(
    TraceEudDevice* trace_handle_p, 
    uint32_t chunksize,         
    uint32_t maxchunks          
);

/**
*
*   @brief Set output directory for trace binary files. If directory doesn't exist, will attempt to create it, else will fail.
*   @param:trace_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance.
*          outputdirectory - This is the directory to save trace files at. Default is the OS temp directory. The location will be 
*          printed at the start of gathering traces.
*   @return: EUD_ERR_t -  Error status
*
*/
EXPORT EUD_ERR_t eud_trace_set_output_dir(
    TraceEudDevice* trace_handle_p, 
    char* outputdirectory                                      
);

/** 
*
*   @brief: Retrieve current output directory. Useful if defaults are used since 
*           default trace directory is a  timestamped temporary folder.
*   @param: trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance.
*           outputdirectory - ASCII pointer to be populated with output directory path
*           stringsize_p - Character length of ASCII filepath 
*   @return: EUD_ERR_t -  Error status
*/
EXPORT EUD_ERR_t eud_trace_get_output_dir(
    TraceEudDevice* trace_handle_p, 
    char* outputdirectory,      
    uint32_t* stringsize_p      
);

/**
*   @brief Sets trace timeout parameter in milliseconds. 
*   Each time a transfer is completed, the transfer timer is re-started. If transfer does not
*   complete before  the time expires, the Trace Peripheral terminates the transfer by sending
*   either a zero or parital length packet and resetting the transfer packet counter and the 
*   transfer timer.
*   @param: trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance.
*           MS_Timeout_Val - timeout value in milliseconds. Max value of 0xFFFFFF  
*   @return:EUD_ERR_t -  Error status
*/
EXPORT EUD_ERR_t eud_trace_set_timeout_ms(
    TraceEudDevice* trace_handle_p,
    uint32_t MS_Timeout_Val 
);

/**
*
*   @brief: Transfer length specifies the number of bytes in a USB transfer to the host. Upon receipt
*   of this command, the transfer length counter is reset. When the number of bytes loaded 
*   into the IN buffer equals Transfer Length, the trace peripheral sends either a short packet
*   or a zero length packet to the host upon receipt of the next IN token.
*   Note that teh value of Transfer length cannot be equal to an integer number of packets plus one,
*   or an integer number of packets plus 2. Transfer length counter is reset on EUD_ReInitTrace().
*
*   Each time a transfer is completed, the transfer timer is re-started. If transfer does not
*   complete before the time expires, the Trace Peripheral terminates the transfer by sending
*   either a zero or parital length packet and resetting the transfer packet counter and the 
*   transfer timer.
*  @param:trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance.
*          TransactionLength Transfer length in bytes. Max of 0xFFFF.  
*  @return:EUD_ERR_t -  Error status
*
*/
EXPORT EUD_ERR_t eud_trace_set_transfer_length(
    TraceEudDevice* trace_handle_p, 
    uint32_t TransactionLength 
);
   
/**
*   @brief Shut down trace peripheral internals. Alternative to calling EUDClosePeripheral(EUDHandlerWrap)
*   @param:trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance. 
*   @return:EUD_ERR_t -  Error status
*
*/
EXPORT EUD_ERR_t eud_close_trace(
    TraceEudDevice* trace_handle_p ///<PVOID*  trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance.
);

/////////////////////////////////////////////////////////////////////////////////
///
///  Qualcomm Internal Trace functions
///
//////////////////////////////////////////////////////////////////////////////////

/**
*   @brief Issue trace flush operation
*   @param:trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance. 
*   @return:EUD_ERR_t -  Error status
*/
EXPORT EUD_ERR_t eud_flush_trace(
    TraceEudDevice* trace_handle_p
);

/**************************************//**
*   @brief Toggle the EUD Trace peripheral to stop trace flow. 
*
*   EUD_StartTrace toggles EUD Trace peripheral to stop passing traces to USB controller. 
*   No internals are shut off, so EUD_StartTrace and EUD_StopTrace can be called without 
*   affecting the state of the trace controller.
*   @param:trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance. 
*   @return:EUD_ERR_t -  Error status
*******************************************/
EXPORT EUD_ERR_t eud_stop_trace(
    TraceEudDevice* trace_handle_p  ///<Parameters: PVOID*  trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance.
);
   
/**
*   @brief Flushes trace buffer and restarts trace flows. Does not 
*   change internal settings.
*   @param:trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance. 
*   @return:EUD_ERR_t -  Error status
*/
EXPORT EUD_ERR_t eud_reinit_trace(
    TraceEudDevice* trace_handle_p ///<PVOID*  trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance.
);

/**
*   EXPORT EUD_ERR_t EUD_TraceReset(PVOID*  trc_handle_p);
*
*   @brief Resets Trace peripheral while still keeping it enumerated with the host.
*
*   Resets Trace peripheral while still keeping it enumerated with the host.
*   Buffers are cleared and default parameters are restored.
*   The caller is responsible for coordinating this reset with other processes on the chip
*   that may interface with the trace peripheral.
*   @param:trc_handle_p - pointer to EUD Handle instance. 
*          Call eud_initialize_device_trace to get EUD Handle Instance.
*   @return: EUD_ERR_t - Error code. 0 for success. Call EUDGetErrorString for ASCII description.
*/
EXPORT EUD_ERR_t eud_trace_reset(
    TraceEudDevice* trace_handle_p
);
 
/**
*   @brief: Sends TRACE_PERIPH_RST command.
*   @param:trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance. 
*   @return:EUD_ERR_t -  Error status
*/
EXPORT EUD_ERR_t trace_periph_rst(
    TraceEudDevice* trc_handler_p
);

/**
*   @brief: Sends TRACE_KEEP command.
*   @param:trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance. 
*   @return:EUD_ERR_t -  Error status
*/
EXPORT EUD_ERR_t trace_keep(
    TraceEudDevice* trc_handler_p
);


/**
*   @brief: sends TRACE_TRNS_TMOUT comamnd.
*   @param:trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance. 
*   @return:EUD_ERR_t -  Error status
*/
EXPORT EUD_ERR_t  trace_trns_tmout(
    TraceEudDevice* trc_handler_p, uint32_t MS_Timeout_Val
);


/**
*   @brief:  sends TRACE_TRNS_LEN comamnd.
*   @param:  trc_handle_p - pointer to EUD Handle instance. Call eud_initialize_device_trace to get EUD Handle Instance. 
*   @return: EUD_ERR_t -  Error status
*/
EXPORT EUD_ERR_t trace_trns_len(
    TraceEudDevice* trc_handler_p, uint32_t  TransactionLength
);

/**
*   @breif: Recieves trace configuration data from OPenOCD and sets the paramters and starts reading trace
*   @param: uint32_t trns_len - transaction length for trace 
*   @param: uint32_t trns_tmout - time out for trace 
*   @param: uint32_t on_time - time for which trace needs to be collected. 
*   @return: Error status
*/
EXPORT EUD_ERR_t eud_trace_device_read(void);

/**
*   @breif: closes trace peripheral. 
*   @param: NULL
*   @return: Error status
**/
EXPORT EUD_ERR_t eud_trace_device_close(void);

#endif //TRACEAPI_H