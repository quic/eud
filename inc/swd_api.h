/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   swd_api.h
*
* Description:                                                              
*   Header file for EUD SWD peripheral public APIs declerations
*
***************************************************************************/
#ifndef SWD_API_H_
#define SWD_API_H_

// #include "eud_api.h"
#include "swd_eud.h"

/////========----------------------------------------------------------------======/////
//                                                                                    //
//                          Macros / Definitions                                      //
//                                                                                    //
/////========----------------------------------------------------------------======/////
///Option for SWD EUD initialization. 
///If set, eud_initialize_device_swd API will enable SWD peripheral but won't send additional initialization commands
#define SWD_LIMITED_INIT_OPTION 1

#define SWD_BITBANG_CLK_SHFT                    0
#define SWD_BITBANG_DI_SHFT                     1
#define SWD_BITBANG_RCTLR_SRST_N_SHFT           2
#define SWD_BITBANG_GPIO_DI_OE                  3
#define SWD_BITBANG_GPIO_SRST_N_SHFT            4
#define SWD_BITBANG_GPIO_TRST_N_SHFT            5
#define SWD_BITBANG_DAP_TRST_N_SHFT             6


#define SWD_BITBANG_CLK_BMSK_ASSERT             (1<<SWD_BITBANG_CLK_SHFT         )
#define SWD_BITBANG_CLK_BMSK_DEASSERT           (0<<SWD_BITBANG_CLK_SHFT         )
                                                
#define SWD_BITBANG_DI_BMSK_ASSERT              (1<<SWD_BITBANG_DI_SHFT          )
#define SWD_BITBANG_DI_BMSK_DEASSERT            (0<<SWD_BITBANG_DI_SHFT          )
                                                
#define SWD_BITBANG_RCTLR_SRST_BMSK_ASSERT      (0<<SWD_BITBANG_RCTLR_SRST_N_SHFT)
#define SWD_BITBANG_RCTLR_SRST_BMSK_DEASSERT    (1<<SWD_BITBANG_RCTLR_SRST_N_SHFT)
                                                
#define SWD_BITBANG_GPIO_DI_OE_ASSERT           (1<<SWD_BITBANG_GPIO_DI_OE       )
#define SWD_BITBANG_GPIO_DI_OE_DEASSERT         (0<<SWD_BITBANG_GPIO_DI_OE       )
                                                
#define SWD_BITBANG_GPIO_SRST_BMSK_ASSERT       (0<<SWD_BITBANG_GPIO_SRST_N_SHFT )
#define SWD_BITBANG_GPIO_SRST_BMSK_DEASSERT     (1<<SWD_BITBANG_GPIO_SRST_N_SHFT )
                                                
#define SWD_BITBANG_GPIO_TRST_BMSK_ASSERT       (0<<SWD_BITBANG_GPIO_TRST_N_SHFT )
#define SWD_BITBANG_GPIO_TRST_BMSK_DEASSERT     (1<<SWD_BITBANG_GPIO_TRST_N_SHFT )
                                                
#define SWD_BITBANG_DAP_TRST_BMSK_ASSERT        (0<<SWD_BITBANG_DAP_TRST_N_SHFT  )
#define SWD_BITBANG_DAP_TRST_BMSK_DEASSERT      (1<<SWD_BITBANG_DAP_TRST_N_SHFT  )

#define  EUD_SWD_FREQ_120_MHz                   0x0
#define  EUD_SWD_FREQ_80_MHz                    0x1
#define  EUD_SWD_FREQ_60_MHz                    0x2
#define  EUD_SWD_FREQ_40_MHz                    0x3
#define  EUD_SWD_FREQ_30_MHz                    0x4
#define  EUD_SWD_FREQ_15_MHz                    0x5
#define  EUD_SWD_FREQ_7_5_MHz                   0x6
#define  EUD_SWD_FREQ_3_75_MHz                  0x7
#define  EUD_SWD_FREQ_1_875_MHz                 0x8
#define  EUD_SWD_FREQ_0_938_MHz                 0x9
#define  EUD_SWD_FREQ_0_469_MHz                 0xA
#define  EUD_SWD_FREQ_0_234_MHz                 0xB
#define  EUD_SWD_FREQ_0_117_MHz                 0xC



/////========----------------------------------------------------------------======/////
//                                                                                    //
//                          SWD EUD API's                                             //
//                                                                                    //
/////========----------------------------------------------------------------======/////

//=====================================================================================//
//                                  SWD Operations.
//                           -------------------------------
//           These operations are used primarily for sending data to SWD controller. 
//           There are 3 states that EUD host software supports for managing data 
//           from these commands, selectable by set_buf_mode():
//           
//           0. Managed buffered write (Default)
//               - swd_read/Write can be called without need for swd_flush, nor 
//                 concern for size of buffers etc. Calling a swd_read results in
//                 immediate write to and from EUD hardware, and the result is placed
//                 into the return word. Writes will be queued up into the outgoing 
//                 buffer until it's full or until a swd_read is issued
//           1. Immediate write
//               - This mode immediately writes the desired command (swd_read or swd_write
//                 out to hardware. This mode is the simplest but you may see
//                 performance degredation.
//           2. Buffered write
//               - This will buffer SWD commands in EUD's own internal
//                 software buffers. The caller must call swd_flush in
//                 order to flush these buffers to EUD hardware.
//                 The user may queue up commands as needed but if a flush is 
//                 required, swd_read/swd_write will return error values indicating 
//                 that a flush is required. Depending on the error value, 
//                 the  user would then need to  re-queue the last command.
//                 Flush error values are defined in eud_error_defines.h:
//                      EUD_SWD_ERR_ITEMQUEUED_BUT_FLUSH_REQUIRED 
//                          - Just need to flush. Command is queued and will be written.
//                      EUD_SWD_ERR_FLUSH_REQUIRED 
//                          - Need to flush and then requeue command.
//
//           
//=====================================================================================//

/*************************************************************************************//**
*   @brief Primary EUD Serial Wire Debug Write function (RnW == 0)
*
*   Depending on buffer mode, command is queued or  written directly to hardware.
*   If command is queued, swd_flush will cause it to be written to hardare.
*   Note that RnW will be 0, as is implied by Write functionality.
*
*****************************************************************************************/   
EXPORT EUD_ERR_t                ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
swd_write(
    SwdEudDevice* swd_handle,   ///< SWD EUD Instance. Initialized through eud_initialize_device_swd
    uint32_t APnDP,             ///< AP/DP port selection (AP==1, DP==0). Only 1st bit evaluated.
    uint32_t A2_3,              ///< A2_3 selection. Only bits [0-1] are evaluated.
    uint32_t data               ///< 32bit word value to be sent to DAP.
    );

/*************************************************************************************//**
*   @brief Primary EUD Serial Wire Debug Read function (RnW == 1)
*
*   Depending on buffer mode, swd_flush may be required to write
*   to hardware and then returndata will be populated.
*   Note that RnW will be 1, as is implied by Read functionality
*
*****************************************************************************************/
EXPORT EUD_ERR_t                ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
swd_read(
    SwdEudDevice* swd_handle, ///< SWD EUD Instance. Initialized through eud_initialize_device_swd
    uint32_t APnDP,             ///< AP/DP port selection (AP==1, DP==0). Only 1st bit evaluated.
    uint32_t A2_3,              ///< A2_3 selection. Only bits [0-1] are evaluated.
    uint32_t* data_p        ///< 32bit word pointer to be populated when flush to hardware occurs.
                                ///< Please see note above on behavior depending on Managed, Manual,
                                ///< or Immediate buffer modes.
    );



/*************************************************************************************//**
*   @brief Manual bit bang of SWD lines. See detail for bit values.
*
*   Manual bit bang of SWD lines. 
*   SWDBitValues are as follows:
*           bit[0] swd_clk 
*           bit[1] swd_di 
*           bit[2] swd_rctlr_srst_n 
*           bit[3] swd_gpio_di_oe
*           bit[4] swd_gpio_srst_n 
*           bit[5] swd_gpio_trst_n 
*           bits[31:6] reserved
*
*****************************************************************************************/
EXPORT EUD_ERR_t                    ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
swd_bitbang (
    SwdEudDevice* swd_handle_p,   ///< SWD EUD Instance. Initialized through eud_initialize_device_swd
    uint32_t swd_bit_values,          ///<Bitmask of Serial Wire fields. See method details for fields.
    uint32_t* data_p            ///<Pointer to be filled by return data
    );


/******************************************************************************************//**
*
*   @brief Sends commands used to transition SWD controller between dormant and active states.
*
*   Sends commands used to transition SWD controller between dormant and active states.
*   This command will immediately write given bits to SWD Peripheral 
*   It is not affected by SWD Mode state.
*   See ARM document ID IHI 0031B section 5.4.3 Connection and Line Reset sequence.
*       Bit stream is as follows:
*       bit[15:0] - SWD_DITMSValue
*       bits[31:16] - count N
*           If N>15, then last payload bit is repeated.
*           //TODO - implement N as actual. On hardware, it's N+1
*     
*
*   Parameters: PVOID*  swd_handle_p - 
*               uint32_t SWD_DITMSValue - 
*               uint32_t count - 
*
*
**************************************************************************************************/                        
EXPORT EUD_ERR_t                ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
swd_di_tms(
    SwdEudDevice* swd_handle_p,   ///< SWD EUD Instance. Initialized through eud_initialize_device_swd

    uint32_t swd_di_tms_value,        ///< Bit field containing data to be shifted to target,
                                    ///< LSB first (little endian).
                                    ///< Note that max for this is 0xFFFF, the count parameter
                                    ///< for which would be 32. If SWD_DITMSValue is above 0xFFFF,
                                    ///< the function will return error.

    uint32_t count_integer           ///< Value for number of clock ticks to shift in DI_TMS data.
                                    ///< Usually this value is <= 32. If it's greater than 32, then the
                                    ///< last bit value from SWD_DITMSValue is repeated.
                                    ///< The max for this is 0xFFFF, but note that this represents
                                    ///< 65,535 clock ticks, which will take a long time on EUD hardware 
                                    ///< to complete.
                                    ///< Values greater than 0xFFFF will return error.
    );
        



/*********************************************************************************************//**
*   @brief swd_flush flushes out the software buffers to hardware. Resets Buffer counters.
*          Only effective in MANUALBUFFERWRITEMODE. For MANAGED mode, flushing occurs 
*          automatically. For IMMEDIATE mode, flushing occurs implicitly
*    
**************************************************************************************************/
EXPORT EUD_ERR_t                    ///< Error code. 0 for success.Call eud_get_error_string for ASCII description.
swd_flush(
    SwdEudDevice* swd_handle_p    ///< SWD EUD Instance. Initialized through eud_initialize_device_swd
    );
        

/////========----------------------------------------------------------------======/////
//                                                                                    //
//                          SWD EUD Configuration API's                               //
//                  Note that these  SWD operation are immediately                    //
//               written out. There are not buffering options available.              //
//                                                                                    //
/////========----------------------------------------------------------------======/////
/*********************************************************************************************//**
*
*   @brief Inserts a NOP into SWD Peripheral stream. All commands following NOP to next flush are ignored.
*
*   swd_nop can be called prior to flush. All the commands following the NOP are ignored. 
*   This should only be used in manual buffer mode
*    
**************************************************************************************************/
EXPORT EUD_ERR_t                    ///<Error code. 0 for success. Call eud_get_error_string for ASCII description.
swd_nop(
    SwdEudDevice* swd_handle_p    ///< SWD EUD Instance. Initialized through eud_initialize_device_swd
);
        
        
/*********************************************************************************************//**
*   @brief Sets various retry parameters for SWD Peripheral interaction with DAP.
*
*
*   Parameters: PVOID*  swd_handle_p - Pointer to EUD Handle instance
*               uint32_t retrycount 
*                    - Sets number of retries the SWD peripheral performs due to a wait response from the DAP.
*                    - Value 0 means no retries are attempted
*                    - Default is 3.
*                
*                uint32_t turnarounddelay
*                    - Turn around delay is used to specifiy the wait period between SWD command components.
*                    - Default is 0.
*                    
*                bool wait_35_if_err
*                    - If wait_35_if_err is set, and an SWD read or write fails, there are no retries, 
*                      and the SWD peripheral waits for 35 cycles before issuing another read or write to the DAP.
*                
*                    //Bit fields:
*                    bits[15:0] Retry count. Default = 3. 
*                    bits[17:16] Turn-around time delay (TRN). Default = 0. 
*                    bit[18] wait_35_if_err 
*                    bits[31:19] reserved
*
**************************************************************************************************/
EXPORT EUD_ERR_t                ///<Error code. 0 for success. Call eud_get_error_string for ASCII description.
swd_timing(
    SwdEudDevice* swd_handle_p,   ///<Pointer to EUD SWD instance
    uint32_t retry_count,        ///<Retry count
    uint32_t turn_around_delay,   ///<Turn around delay
    uint32_t wait_35_if_err     ///<Boolean - wait 35 cycles before issuing another r/w to DAP.
    );
        

/*********************************************************************************************//**
*   @brief Selects mode that EUD software will use to buffer commands sent: 
*           Managed Buffered Write (0), Immediate write (1) or Manual Buffered Write (2)
*
*       SWD Operation Modes
*       --------------------
*       There are 3 modes that these operations can be in (set by
*       set_buf_mode):
*           0. Managed buffered write (Default)
*               - swd_read/Write can be called without need for swd_flush, nor
*                 concern for size of buffers etc. Calling a swd_read results in
*                 immediate write to and from EUD hardware, and the result is placed
*                 into the return word. Writes will be queued up into the outgoing
*                 buffer until it's full or until a swd_read is issued
*           1. Immediate write
*               - This mode immediately writes the desired command (swd_read or swd_write
*                 out to hardware. This mode is the simplest but you may see
*                 performance degredation.
*           2. Buffered write
*               - This will buffer SWD commands in EUD's own internal
*                 software buffers. The caller must call swd_flush in
*                 order to flush these buffers to EUD hardware.
*                 The user may queue up commands as needed but if a flush is
*                 required, swd_read/swd_write will return error values indicating
*                 that a flush is required. Depending on the error value,
*                 the  user would then need to  re-queue the last command.
*                 Flush error values are defined in eud_error_defines.h:
*                      EUD_SWD_ERR_ITEMQUEUED_BUT_FLUSH_REQUIRED
*                          - Just need to flush. Command is queued and will be written.
*                      EUD_SWD_ERR_FLUSH_REQUIRED
*                          - Need to flush and then requeue command.
*       
*   Parameters: PVOID*  swd_handle_p - Pointer to EUD Handle instance
*               uint32_t BufModeSelect has the following values:
*                   0  - Managed Buffered Write (Default)
*                   1  - Immediate Write
*                   2  - Manual Buffered Write
*                   3=< - Return invalid parameter
*
**************************************************************************************************/
EXPORT EUD_ERR_t                    ///<Error code. 0 for success. Call eud_get_error_string for ASCII description.
set_buf_mode(
    SwdEudDevice* swd_handle_p,   ///<Pointer to EUD SWD instance
    uint32_t mode                   ///<Mode selection (0-2)
    );



/*********************************************************************************************//**
*
*   @brief Retrieves current buffer mode and stores it to bufmode_p.
*
*   See set_buf_mode for more information on EUD Serial Wire Debug buffer modes.
*   The following values are possible:
*                   0.  - Managed Buffered Write (Default)
*                   1.  - Immediate Write
*                   2.  - Manual Buffered Write
*                   3. =< - Return invalid parameter
*
**************************************************************************************************/
EXPORT EUD_ERR_t
get_buf_mode(
    SwdEudDevice* swd_handle_p, ///< Pointer to EUD handle instance.
    uint32_t* bufmode_p  ///<Pointer for GetBufMode to store current buffer mode. 
    );
    
    
/*********************************************************************************************//**
*   EUD_ERR_t swd_set_frequency(SwdEudDevice* swd_handle_p,Freq_e Frequency)
*
*   @brief Sets EUD SWD Peripheral clock frequency. See table for values
*
*   Freq_e Frequency enumeration values:
*          Freq Enum           | Value
*         -------------------- | ---------
*         EUD_FREQ_120_MHz     |   0x0
*         -------------------- | ---------
*         EUD_FREQ_80_MHz      |   0x1
*         -------------------- | ---------
*         EUD_FREQ_60_MHz      |   0x2
*         -------------------- | ---------
*         EUD_FREQ_40_MHz      |   0x3
*         -------------------- | ---------
*         EUD_FREQ_30_MHz      |   0x4
*         -------------------- | ---------
*         EUD_FREQ_15_MHz      |   0x5
*         -------------------- | ---------
*         EUD_FREQ_7_5_MHz     |   0x6
*         -------------------- | ---------
*         EUD_FREQ_3_75_MHz    |   0x7
*         -------------------- | ---------
*         EUD_FREQ_1_875_MHz   |   0x8
*         -------------------- | ---------
*         EUD_FREQ_0_938_MHz   |   0x9
*         -------------------- | ---------
*         EUD_FREQ_0_469_MHz   |   0xA
*         -------------------- | ---------
*         EUD_FREQ_0_234_MHz   |   0xB
*         -------------------- | ---------
*         EUD_FREQ_0_117_MHz   |   0xC
*         -------------------- | ---------
*         Reserved, return err | 0xD-0xFF
*         -------------------- | ---------
*
**************************************************************************************************/
EXPORT EUD_ERR_t ///<Error code. 0 for success. Call eud_get_error_string for ASCII description.
swd_set_frequency(
    SwdEudDevice* swd_handle_p, ///<Pointer to EUD SWD instance
    uint32_t frequency  ///<Freq_e Frequency - uint32_t enumeration indicating desired frequency. See table in detailed description.
    );


/*********************************************************************************************//**
*   @brief Forces a delay (swd_clk_freq/(delaytime+1)) between SWD commands. Default is (swd_clk_freq/1)
*
*   Resets EUD Serial Wire Debug (SWD) peripheral, which includes:
*       Keeps peripheral enumerated on USB. 
*       OUT/IN buffers are cleared.
*       Programmable parameters are restored to their default values.
*
**************************************************************************************************/
EXPORT EUD_ERR_t ///<Error code. 0 for success. Call eud_get_error_string for ASCII description.
swd_set_delay( 
    SwdEudDevice* swd_handle_p, ///<Pointer to EUD SWD instance
    uint32_t delay_time ///<delaytime - Forces delay between commands. Delay = swd_clk_freq / (delaytime+1). Default delaytime is 0 (==swd_clk_freq/1)
    );


/*********************************************************************************************//**
*   @brief Resets EUD Serial Wire Debug (SWD) peripheral. 
*
*   Resets EUD Serial Wire Debug (SWD) peripheral, which includes:
*       Keeps peripheral enumerated on USB. 
*       OUT/IN buffers are cleared.
*       Programmable parameters are restored to their default values.
*
**************************************************************************************************/
EXPORT EUD_ERR_t ///<Error code. 0 for success. Call eud_get_error_string for ASCII description.
swd_peripheral_reset(
    SwdEudDevice* swd_handle_p ///<Pointer to EUD SWD instance
    );
//TODO - drain IN buffer before issuing SWD_PERIPH_RST command
//TODO - reset all programmable values (delay etc.)
            

/*********************************************************************************************//**
*
*   @brief Populates given integer with error data returned from EUD SWD peripheral.  
*
*   This function is designed to be called after every swd_read/swd_write. 
*   Since appending a swd_flush to every packet can result in performance degredation,
*   the user may use swd_set_swd_status_max_count to set a packet count at which a status packet is sent. 
*   swd_get_status will return the last status known status until the counter expires, at which time it will append
*   a status packet and the status_p will be updated with the new value.
*   Note that forcestatusflush_flag is an option which will force a status to be sent out immediately.
*
*   Pseudocode would be as follows:
*       swd_set_swd_status_max_count(swd_device_p,3)
*       err = swd_read(..)
*       err = swd_get_status(swd_device_p,0,status_p);
*           //err ==  SWD_WARN_SWDSTATUS_NOT_UPDATED. status_p updated to last known status value
*       err = swd_read(..)
*       err = swd_get_status(swd_device_p,0,status_p);
*           //err ==  SWD_WARN_SWDSTATUS_NOT_UPDATED. status_p updated to last known status value
*       err = swd_read(..)
*       err = swd_get_status(swd_device_p,0,status_p);
*           //err ==  EUD_SUCCESS. status_p updated to new value. 
*           If there's a SWD fault, use bits[31:16] of status_p to determine which packet caused SWD fault.
*       err = swd_read(..)
*       err = swd_get_status(swd_device_p,0,status_p);
*           //err ==  SWD_WARN_SWDSTATUS_NOT_UPDATED. status_p updated to last known status value
*       err = swd_read(..)
*       err = swd_get_status(swd_device_p,1,status_p);
*           //err ==  EUD_SUCCESS. status_p updated to new value because force flag was set to 1.

*   See detailed table for more information.
*
*   Parameters: PVOID*  swd_handle_p - pointer to EUD Handle instance
*               forcestatusflush_flag - 
*               status_p - swd_get_status will populate status_p with reply from EUD SWD device of its status.
*                   The following table indicates status string status:
*
*     Bit Fields   |    Title      | Description
*    ------------- | ------------- | -----------------------------------------------------------------------------------------
*    bits[31:16]   | cmd_cntr      |  Each time SWD Peripheral processes a SWD r/w command, the cmd_cntr is incremented.
*                  |               |  If an error occurs, cmd_cntr is stopped and an error flag is set.
*                  |               |  The SWD Peripheral does not stop operation but continues to process commands as normal.
*                  |               |  If an error does not occur between SWD_STATUS commands, then cmd_cntr == commands that 
*                  |               |  have been processed.
*                  |               |  When Status is requested, cmd_cntr and error status bits are set to zero.
*    ------------- | ------------- | -----------------------------------------------------------------------------------------
*    bits[15:5]    | Reserved      |  Reserved             
*    ------------- | ------------- | -----------------------------------------------------------------------------------------
*    bit[5]        | freezio_latch |  If rpmh_freezio_nhv is asserted, freezio_latch is set and cmd_cntr is stopped
*    ------------- | ------------- | -----------------------------------------------------------------------------------------
*    bit[4]        | parity_err    |  If the parity of the response from the DAP does not match the parity bit from the DAP,
*                  |               |  then parity_err bit is set and cmd_cntr is stopped.
*    ------------- | ------------- | -----------------------------------------------------------------------------------------
*    bit[3]        | wait_timeout  |  When an SWD transaction occurs, the DAP may respond with a WAIT. The SWD Peripheral
*                  |               |  may retry multiple times depending on SWD_TIMING retry count. If all retries fail, 
*                  |               |  the wait_timeout bit is set and cmd_cntr is stopped.
*    ------------- | ------------- | -----------------------------------------------------------------------------------------
*    bits[2:0]     | ack[2:0]      |  OK==?, WAIT==?, OTHER==?
*                  |               |  If the DAP responds with an ack[2:0] value other than OK or WAIT, the DAP ack[2:0]
*                  |               |  bits are stored and the cmd_cntr is stopped.
*    ------------- | ------------- | -----------------------------------------------------------------------------------------
*
*
**************************************************************************************************/
EXPORT EUD_ERR_t        ///<Error code. 0 for success.Call eud_get_error_string for ASCII description.
swd_get_status(
    SwdEudDevice* swd_handle_p, ///<Pointer to EUD SWD instance
    uint32_t force_status_flush_flag, ///<Flag wheter to force status and flush packet ot be sent to hardware. Default to 0. If nonzero, status and flush occurs immediately.
    uint32_t* status_p  ///<32bit pointer to be filled with status word. See detailed documentation.
    );

/*********************************************************************************************//**
*   @brief Sets parameter for how often to append SWDStatus to SWD packets. See swd_get_status for details on status value.
*
*   Parameters: PVOID*    swd_handle_p - Pointer to EUD Handle instance
*               uint32_t* swd_getstatus_maxvalue - A counter is incremented internally to determine when to append SWDStatus to outgoing packets.
*                                                  0 - Status is never appended.
*                                                  1 - Status is appended whenever a flush occurs (note that timing of flush dependent on buffer mode).
*                                                  0xFFFF - Maximum value available. Higher than this results in error.
*                                                  A high performance impact is seen if status is appended on every flush. 
*                                                  This API allows the user to space out status packets and use the packet counter which is retrieved from EUDGetStatus
*                                                  to determine which packet was erroring. See swd_get_status for more detail.
*
**************************************************************************************************/
EXPORT  EUD_ERR_t   ///<Error code. 0 for success.Call eud_get_error_string for ASCII description.
swd_set_swd_status_max_count(
    SwdEudDevice* swd_handle_p, ///<Pointer to EUD SWD instance
    uint32_t swd_get_status_max_value ///<Max value that SWDStatus will be appended. If 0, status is never appended. If 1, status appended on every swd_read.
);

/*********************************************************************************************//**
*   @brief Populates given string buffer with SWD status and its description. See table under swd_get_status for further detail.
*
*   Parameters: PVOID*  swd_handle_p - Pointer to EUD Handle instance
*               char* stringbuffer - swd_get_status_string will populate this buffer with 5 lines separated by '\n'. 
*                                    This string represents ack, wait_timeout, parity_err, freezio_latch and cmd_cntr.   
*               uint32_t* sizeofstringbuffer - swd_get_status_string will populate this value with the size of the returned string.
*                                              swd_get_status_string guarantees that no string will be greater than 1000 characters.
*
**************************************************************************************************/
EXPORT EUD_ERR_t ///<Error code. 0 for success. Call eud_get_error_string for ASCII description.
swd_get_status_string(
    SwdEudDevice* swd_handle_p, ///<Pointer to EUD SWD instance
    char* string_buffer_p, ///<swd_get_status_string will populate this buffer with 5 lines separated by '\n'. 
                          ///<This string represents ack, wait_timeout, parity_err, freezio_latch and cmd_cntr.   
    uint32_t* size_of_string_buffer_p ///<swd_get_status_string will populate this value with the size of the returned string.
                                   ///<swd_get_status_string guarantees that no string will be greater than 1000 characters.
    );

        
/*********************************************************************************************//**
*   @brief Returns 1 if flush is required, 0 if not. For manual buffered mode. Call swd_flush() to flush buffers.
*
**************************************************************************************************/
EXPORT EUD_ERR_t                    ///<Error code. 0 for success. Call eud_get_error_string for ASCII description.
swd_flush_required(
    SwdEudDevice* swd_handle_p,   ///<Pointer to EUD SWD instance
    uint8_t* flush_req_p             ///<Pointer to flush value. swd_flush_required will populate this with 1 (flush) or 0 (no flush)
    );



/*********************************************************************************************//**
*   @brief Issues SWD_DI_TMS commands to switch chip's JTAG to SWD mode
*
*   Parameters: (PVOID)* EUDHandlerWrap - 
* 
**************************************************************************************************/
EXPORT EUD_ERR_t                    ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
jtag_to_swd (
    SwdEudDevice* swd_handle_p    ///<Pointer to EUD SWD instance
    );
        
       
/*********************************************************************************************//**
*   @brief Retrieves DAP JTAGID from chip using SWD commands. 
*   Assumes that JTAG is in SWD mode via jtag_to_swd.
*
*       swd_get_jtag_id will issue the necessary SWD commands to retrieve DAP JTAG ID from 
*       the hardware's DAP and populate jtagid_p uint32_t* jtagid_p - 
*       swd_get_jtag_id will populate given pointer jtagid_p with hardware's DAP JTAG ID.
*
**************************************************************************************************/
EXPORT EUD_ERR_t                    ///<Error code. 0 for success. Call eud_get_error_string for ASCII description. 
swd_get_jtag_id(
    SwdEudDevice* swd_handle_p,   ///<Pointer to EUD SWD instance
    uint32_t* jtag_id_p              ///<Pointer to jtagID value to populate
    );

//EXPORT EUD_ERR_t swd_get_jtag_id(SwdEudDevice* swd_handle_p, uint8_t* jtagid_p);



/*********************************************************************************************//**
*   @brief Resets chip by toggling SWD Peripheral's swd_gpio_trst_n. Doesn't reset EUD SWD peripheral
*
**************************************************************************************************/
EXPORT EUD_ERR_t ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
swd_line_reset(
    SwdEudDevice* swd_handle_p///<Pointer to EUD SWD instance
    );
        

                 
#endif //SWD_API_H_