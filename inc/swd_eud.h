/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   swd_eud.h
*
* Description:                                                              
*   Header file for EUD SWD peripheral Class and other defines 
*
***************************************************************************/
#ifndef SWD_EUD_H_
#define SWD_EUD_H_

#include <stdbool.h>
#include "eud.h"
#include "arm_dap.h"

//===---------------------------------------------------------------------===//
//
//  Top level defines, typedef's
//
//===---------------------------------------------------------------------===//

//////////////////Various EUD Hardware defines and packet sizes///////////////////
#define SWD_CMD_NOP                     0
#define SWD_CMD_FLUSH                   1
#define SWD_CMD_FREQ                    2
#define SWD_CMD_DELAY                   3
#define SWD_CMD_BITBANG                 4
#define SWD_CMD_DITMS                   5
#define SWD_CMD_TIMING                  6
#define SWD_CMD_STATUS                  7
#define SWD_CMD_PERIPH_RST              8
#define SWD_CMD_CMD_BASEVALUE           0x80

#define  EUD_SWD_FREQ_120_MHz           0x0
#define  EUD_SWD_FREQ_80_MHz            0x1
#define  EUD_SWD_FREQ_60_MHz            0x2
#define  EUD_SWD_FREQ_40_MHz            0x3
#define  EUD_SWD_FREQ_30_MHz            0x4
#define  EUD_SWD_FREQ_15_MHz            0x5
#define  EUD_SWD_FREQ_7_5_MHz           0x6
#define  EUD_SWD_FREQ_3_75_MHz          0x7
#define  EUD_SWD_FREQ_1_875_MHz         0x8
#define  EUD_SWD_FREQ_0_938_MHz         0x9
#define  EUD_SWD_FREQ_0_469_MHz         0xA
#define  EUD_SWD_FREQ_0_234_MHz         0xB
#define  EUD_SWD_FREQ_0_117_MHz         0xC

///////////////////////////////Packet size definitions/////////////////////////////

#define SWD_PCKT_SZ_SEND_NOP            1
#define SWD_PCKT_SZ_SEND_FLUSH          1
#define SWD_PCKT_SZ_SEND_FREQ           5
#define SWD_PCKT_SZ_SEND_DELAY          5
#define SWD_PCKT_SZ_SEND_DITMS          5
#define SWD_PCKT_SZ_SEND_TIMING         5
#define SWD_PCKT_SZ_SEND_BITBANG        5
#define SWD_PCKT_SZ_SEND_STATUS         1
#define SWD_PCKT_SZ_SEND_RST            1
#define SWD_PCKT_SZ_SEND_SWDCMD_READ    1
#define SWD_PCKT_SZ_SEND_SWDCMD_WRITE   5

#define SWD_PCKT_SZ_RECV_NOP            0
#define SWD_PCKT_SZ_RECV_FLUSH          0
#define SWD_PCKT_SZ_RECV_FREQ           0
#define SWD_PCKT_SZ_RECV_DELAY          0
#define SWD_PCKT_SZ_RECV_BITBANG        4
#define SWD_PCKT_SZ_RECV_DITMS          0
#define SWD_PCKT_SZ_RECV_TIMING         0
#define SWD_PCKT_SZ_RECV_STATUS         4
#define SWD_PCKT_SZ_RECV_RST            0
#define SWD_PCKT_SZ_RECV_SWDCMD_READ    4
#define SWD_PCKT_SZ_RECV_SWDCMD_WRITE   0

///////////////////////////////Endian Option///////////////////////////////////////
#define SWD_ENDIANOPT_NOP                   BIGENDIAN
#define SWD_ENDIANOPT_FLUSH                 BIGENDIAN
#define SWD_ENDIANOPT_FREQ                  BIGENDIAN
#define SWD_ENDIANOPT_DELAY                 BIGENDIAN
#define SWD_ENDIANOPT_BITBANG               BIGENDIAN
#define SWD_ENDIANOPT_DITMS                 BIGENDIAN
#define SWD_ENDIANOPT_TIMING                BIGENDIAN
#define SWD_ENDIANOPT_STATUS                BIGENDIAN
#define SWD_ENDIANOPT_PERIPH_RST            BIGENDIAN


#define SWD_RNW_WRITE_BIT                       0
#define SWD_RNW_READ_BIT                        1


#define SWD_NUM_OPCODES                 9

#define SWD_CMD_PAYLOAD_SIZE            SWD_PCKT_SZ_SEND_SWDCMD_WRITE - 1

#define SWD_OUT_BUFFER_SIZE             32  ///<Buffer USB Write Bytes. Same size as EUD Hardware buffer.
#define SWD_IN_BUFFER_SIZE              32  ///<Default size for USB Read Buffer size. 
                                            ///<Hardware won't return value greater than 32 Bytes. 
                                            ///<Effective size may vary depending on Mode.
#define SWD_MAX_RESPONSE_SIZE           28  //swd reposnse size when swd_status is queued in payload

#define SWD_ACK_MASK                    0x7
#define SWD_ACK_WAIT_MASK               0x1<<1
#define SWD_ACK_FAULT_MASK              0x1<<2

//Only allow  status reads/writes for FPGA
#if FPGA
#define SWD_APPEND_SWDSTATUS_FLAG       1
#else
#define SWD_APPEND_SWDSTATUS_FLAG       0
#endif

////////////////////////Various boundaries for  API's////////////////////////////
#define SWD_CMD_FREQ_MAX                        0xD
#define SWD_CMD_DELAY_MAX                       0xFF
#define SWD_CMD_DITMS_VALUE_MAX                 0xFFFF
#define SWD_CMD_DITMS_COUNT_MAX                 0xFFFF

#define SWD_CMD_TIMING_RETRYCOUNT_MAX           0xFFFF
#define SWD_CMD_TIMING_TURNAROUNDDELAY_MAX      0x3
#define SWD_CMD_TIMING_TURNAROUNDDELAY_MASK     0x3
#define SWD_CMD_TIMING_WAIT_35_IF_ERR_SHIFT     2

#define SWD_EUD_DELAY_VALUE                     18

#define SWD_OUT_BUFFER_SIZE                     32

#define EUD_SWD_MAX_USB_OUT_BUFFER_SIZE         SWD_OUT_BUFFER_SIZE - SWD_PCKT_SZ_SEND_SWDCMD_WRITE*1 - SWD_PCKT_SZ_SEND_FLUSH

#define DEFAULT_USB_READ_BUFFER_MAX_VEC_SIZE    DEFAULT_USB_READ_BUFFER_VECTOR_SIZE - SWD_PCKT_SZ_RECV_SWDCMD_READ

//Buffer limits
#define SWD_MAX_COUNT                           100

#define FLUSH_OPTION_NULL                       0
#define FLUSH_OPTION_TRUE                       1


#define QUEUE_FULL_COMMAND_NOT_QUEUED           EUD_SWD_ERR_FLUSH_REQUIRED                  ///<For checking  buffer bounds. Item couldn't be queued. Flush required.
#define QUEUE_FULL_COMMAND_QUEUED               EUD_SWD_ERR_ITEMQUEUED_BUT_FLUSH_REQUIRED   ///<For checking  buffer bounds. Item is queued but flush is required.
#define QUEUE_NOT_YET_FULL                      EUD_SUCCESS                                 ///<For checking  buffer bounds. Buffer not yet full.


#define SWD_GETSTATUS_MAX_STATUS_COUNTER        0xFFFF ///<For SWDSetSWDStatusParameter. Flush buffers will append swdstatus value once counter has reached this value.
                                                       ///<If 0, swdstatus will not  be appended.
///Option for SWD EUD initialization. 
///If set, INIT_SWD_DEVICE API will enable SWD peripheral but won't send additional initialization commands
#define SWD_LIMITED_INIT_OPTION                 1
#define INIT_SWD_ALLOWED_OPTIONS_MSK            SWD_LIMITED_INIT_OPTION //Option for SWD EUD init. Only 1 option allowed at this time.


#define SWD_CLK_SHFT                            0
#define SWD_DI_SHFT                             1
#define SWD_RCTLR_SRST_N_SHFT                   2
#define SWD_GPIO_DI_OE_SHFT                     3
#define SWD_GPIO_SRST_N_SHFT                    4
#define SWD_GPIO_TRST_N_SHFT                    5
#define SWD_DAP_TRST_N_SHFT                     6

#define SWD_CLK_VAL                             (1<<SWD_CLK_SHFT)
#define SWD_DI_VAL                              (1<<SWD_DI_SHFT)
#define SWD_RCTLR_SRST_N_VAL                    (1<<SWD_RCTLR_SRST_N_SHFT)
#define SWD_GPIO_DI_OE_VAL                      (1<<SWD_GPIO_DI_OE_SHFT)
#define SWD_GPIO_SRST_N_VAL                     (1<<SWD_GPIO_SRST_N_SHFT)
#define SWD_GPIO_TRST_N_VAL                     (1<<SWD_GPIO_TRST_N_SHFT)
#define SWD_DAP_TRST_N_VAL                      (1<<SWD_DAP_TRST_N_SHFT)

#define swd_bitbang_VALIDVALUE_MSK               ~(  SWD_CLK_VAL         +\
                                                    SWD_DI_VAL          +\
                                                    SWD_RCTLR_SRST_N_VAL+\
                                                    SWD_GPIO_DI_OE_VAL  +\
                                                    SWD_GPIO_SRST_N_VAL +\
                                                    SWD_GPIO_TRST_N_VAL +\
                                                    SWD_DAP_TRST_N_VAL  )

///////////////////////////////SWD Command Masks///////////////////////////////////
#define SWD_APnDP_MASK                          1
#define SWD_RnW_MASK                            1
#define SWD_A2_3_MASK                           3
#define SWD_STOP_BIT_MASK                       0x40
#define SWD_CMD_READ_BIT_MASK                   0x1<<2

///////////////////////////////Reset Defines///////////////////////////////////////
//#define RESET_ENABLED 1

#define SWD_SRST_N_DEASSERT_RESET               1   ///<To deassert SRST (system reset) use this value.
#define SWD_SRST_N_ASSERT_RESET                 0   ///<To assert SRST (system reset) use this value.

#define SWD_GPIO_SRST_N_DEASSERT_RESET          1   ///<To deasssert SRST (system reset) via GPIO, use this value.
#define SWD_GPIO_SRST_N_ASSERT_RESET            0   ///<To deasssert SRST (system reset) via GPIO, use this value.

#define SWD_GPIO_TAP_RESET_N_DEASSERT_RESET     1   ///<To deasssert TRST (TAP reset), use this value.
#define SWD_GPIO_TAP_RESET_N_ASSERT_RESET       0   ///<To deasssert TRST (TAP reset), use this value.

//////////////For OLD EUD SPEC/Kept for reference////////////////
#define SWD_PORT1_SEL                           0x09

#define SWD_PAYLOAD_LINE_RESET                  "\xFF\xFF\x32\x00"
#define SWD_PAYLOAD_jtag_to_swd                 "\x9E\xE7\x0F\x00"

//For OLD EUD SPEC **end**  //



//===---------------------------------------------------------------------===//
//
//                  Structures, Function Declarations and Classes
//
//===---------------------------------------------------------------------===//

extern volatile uint32_t SWDStatusMaxCount;             // XXXX
extern volatile uint32_t CurrentSWDStatusCounter;       // XXXX
///Bitbang structures
///Not currently used
typedef struct _bitBangCmd
{

    uint8_t swdclk : 1;
    uint8_t swddi : 1;
    uint8_t swd0_gcc_srst : 1;
    uint8_t swd1_di_oe : 1;
    uint8_t swd1_gpio_srst : 1;
    uint8_t _res : 3;

}BitBangCmd;

typedef union _bitBangCmd_t
{
    BitBangCmd s;
    uint8_t cmd;
}BitBangCmd_t;

///SWDCmd  structure
///Used by SWDCMD and SWDCMD2 to assemble a swd command
typedef struct _swdCmd
{
    uint8_t start : 1;
    uint8_t APnDP : 1;
    uint8_t RnW : 1;
    uint8_t A2_3 : 2;
    uint8_t parity : 1;
    uint8_t stop : 1;
    uint8_t park : 1;

}SwdCmd;

///C-type union used for packing SWD commands to USB.
typedef union _SwdCmd_t
{
    SwdCmd s;
    uint8_t cmd;
}SwdCmd_t;


//class EudDevice;

//////////////////////////////////////////////
//
//  SWD EUD class
//
//      SWD peripheral. Inherits from EUD device
//
//////////////////////////////////////////////
#ifdef  __cplusplus
class SwdEudDevice : public EudDevice
{
public:
    SwdEudDevice();                   ///<Constructor
    ~SwdEudDevice();                  ///<Constructor

    /* API to assert a reset using a EUD peripheral */
    EUD_ERR_t AssertReset(void);

    /* API to de-assert reset using a EUD peripheral */
    EUD_ERR_t DeAssertReset(void);

    ////////////////////////////////////////////////////////////
    //                  SWD USB methods/members.              //
    ////////////////////////////////////////////////////////////

    // Buffer for USB Write Bytes. Size is SWD_OUT_BUFFER_SIZE (32)
    uint8_t* usb_write_buffer_p_;

    // Index for usb_write_buffer_p_ 
    uint32_t* usb_write_index_p_; 

    // Default size for USB Read Buffer size. 
    // Hardware won't return value greater than 32 Bytes. 
    // Effective size may vary depending on Mode.
    uint8_t* usb_read_buffer_p_;

    // Index for usb_read_buffer_p_
    uint32_t* usb_read_index_p_; 

    // Flag for flush required (call swd_flush).
    bool flush_required_;

    // Logs expected bytes to be read into USB_Read_Buffer_p.
    uint32_t* expected_bytes_p_;
    

    // In order to enable SWD mode on the DAP TAP, a special sequence
    bool swd_to_jtag_operation_done_;
    

    // Last saved status since swd_get_status was called.
    // If swd_status_sent_flag_ is true, swd_get_status returns this value
    // If swd_status_sent_flag_ is false, swd_get_status will query hardware
    // for SWD Status and populate this value.
    uint32_t swd_last_status_;

    // Flag indicating if SWD status has ben sent. This is used mainly by
    // swd_get_status to determine if EUD should be queried by host software
    // or if existing status should be returned.
    bool swd_status_sent_flag_;
    
    // DP registers for this peripheral 
    dp_reg_t dp_regs_[DP_MAX_REGISTERS_v2];

    // AP registers for this peripheral (could be shared across multiple APs 
    ap_reg_t ap_regs_[AP_MAX_REGISTERS_v2];

    // History of register related operations 
    // Keeps track of the last bank selected for the DP or AP 
    // 0 is DP, 1 is AP
    dp_reg_t dp_select_reg_hist_;

    /* API to retrieve the registers being accessed by a given transaction */
    std::string GetDpRegInfo(uint32_t reg_idx, uint32_t read_n_write);
    std::string GetApRegInfo(uint32_t reg_idx, uint32_t read_n_write);
    std::string GetRegInfo(uint32_t acess_port, uint32_t reg_idx,
                           uint32_t read_n_write);

    /* Setter method for the bank select operations */
    void SetDpApSelect(uint32_t access_port, uint32_t reg_idx, 
                       uint32_t read_n_write, uint32_t val);

};
#else  // __cplusplus
typedef void SwdEudDevice;
#endif // __cplusplus

#endif //SWD_EUD_H_


