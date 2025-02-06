/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   jtag_eud.h
*
* Description:                                                              
*   Header file for EUD JTAG peripheral class and various defines
*   Note: JTAG Peripheral not completed due to hardware limitations 
*
***************************************************************************/
#ifndef JTAG_EUD_H_
#define JTAG_EUD_H_

#include "eud.h"

#define QCEUD_DEBUG_PRINT 1

///////////////Various EUD Hardware defines and packet sizes//////////////
#define JTG_CMD_NOP              '\x0'
#define JTG_CMD_FLUSH            '\x1'
#define JTG_CMD_FREQ_WRITE       '\x2'
#define JTG_CMD_DELAY            '\x3'
#define JTG_CMD_BITBANG          '\x4'
#define JTG_CMD_TMS              '\x5'
#define JTG_CMD_NBIT_TOSS        '\x6'
#define JTG_CMD_NBIT_KEEP        '\x7'
#define JTG_CMD_NBIT_END_TOSS    '\x8'
#define JTG_CMD_NBIT_END_KEEP    '\x9'
#define JTG_CMD_32BIT_TOSS       '\xA'
#define JTG_CMD_32BIT_KEEP       '\xB'
#define JTG_CMD_32BIT_END_TOSS   '\xC'
#define JTG_CMD_32BIT_END_KEEP   '\xD'
#define JTG_CMD_PERIPHERAL_RESET '\xE'
#define JTG_CMD_FREQ_READ        '\xF'

#define CMD_JTAG_NOP             0x00
#define CMD_JTAG_FLUSH           0x01
#define CMD_JTAG_FREQ_WR         0x02
#define CMD_JTAG_DELAY           0x03
#define CMD_JTAG_BITBANG         0x04
#define CMD_JTAG_TMS             0x05
#define CMD_JTAG_NBIT_TOSS       0x06
#define CMD_JTAG_NBIT_KEEP       0x07
#define CMD_JTAG_NBIT_END_TOSS   0x08
#define CMD_JTAG_NBIT_END_KEEP   0x09
#define CMD_JTAG_32BIT_TOSS      0x0A
#define CMD_JTAG_32BIT_KEEP      0x0B
#define CMD_JTAG_32BIT_END_TOSS  0x0C
#define CMD_JTAG_32BIT_END_KEEP  0x0D
#define CMD_JTAG_PERIPH_RST      0x0E
#define CMD_JTAG_FREQ_RD         0x0F

#define  EUD_JTAG_FREQ_120_MHz   0x0
#define  EUD_JTAG_FREQ_80_MHz    0x1
#define  EUD_JTAG_FREQ_60_MHz    0x2
#define  EUD_JTAG_FREQ_40_MHz    0x3
#define  EUD_JTAG_FREQ_30_MHz    0x4
#define  EUD_JTAG_FREQ_15_MHz    0x5
#define  EUD_JTAG_FREQ_7_5_MHz   0x6
#define  EUD_JTAG_FREQ_3_75_MHz  0x7
#define  EUD_JTAG_FREQ_1_875_MHz 0x8
#define  EUD_JTAG_FREQ_0_938_MHz 0x9
#define  EUD_JTAG_FREQ_0_469_MHz 0xA
#define  EUD_JTAG_FREQ_0_234_MHz 0xB
#define  EUD_JTAG_FREQ_0_117_MHz 0xC
#define  JTAG_MAX_FREQUENCY      EUD_JTAG_FREQ_0_117_MHz
///////////////Packet size definitions//////////////////
#define JTG_PCKT_SZ_SEND_OPCODE  1
#define JTG_PCKT_SZ_SEND_GENERAL 5


#define JTG_PCKT_SZ_SEND_NOP        1
#define JTG_PCKT_SZ_SEND_FLUSH      1
#define JTG_PCKT_SZ_SEND_FREQ_WR    5
#define JTG_PCKT_SZ_SEND_DELAY      5
#define JTG_PCKT_SZ_SEND_BITBANG    5
#define JTG_PCKT_SZ_SEND_TMS        5
#define JTG_PCKT_SZ_SEND_NBIT_TOSS  5
#define JTG_PCKT_SZ_SEND_NBIT_KEEP  5
#define JTG_PCKT_SZ_SEND_NBIT_END_TOSS    5
#define JTG_PCKT_SZ_SEND_NBIT_END_KEEP    5
#define JTG_PCKT_SZ_SEND_32BIT_TOSS       5
#define JTG_PCKT_SZ_SEND_32BIT_KEEP       5
#define JTG_PCKT_SZ_SEND_32BIT_END_KEEP    5
#define JTG_PCKT_SZ_SEND_32BIT_END_TOSS    5
#define JTG_PCKT_SZ_SEND_RST        1
#define JTG_PCKT_SZ_SEND_FREQ_RD    1

#define JTG_NUM_OPCODES 16

#define JTG_PCKT_SZ_RECV_GENERAL    4

#define JTG_PCKT_SZ_RECV_NOP        0
#define JTG_PCKT_SZ_RECV_FLUSH      0
#define JTG_PCKT_SZ_RECV_FREQ_WR    0
#define JTG_PCKT_SZ_RECV_DELAY      0
#define JTG_PCKT_SZ_RECV_BITBANG    4
#define JTG_PCKT_SZ_RECV_TMS        0
#define JTG_PCKT_SZ_RECV_NBIT_TOSS  0
#define JTG_PCKT_SZ_RECV_NBIT_KEEP  4
#define JTG_PCKT_SZ_RECV_NBIT_END_TOSS    0
#define JTG_PCKT_SZ_RECV_NBIT_END_KEEP    4
#define JTG_PCKT_SZ_RECV_32BIT_TOSS       0
#define JTG_PCKT_SZ_RECV_32BIT_KEEP       4
#define JTG_PCKT_SZ_RECV_32BIT_END_TOSS   0
#define JTG_PCKT_SZ_RECV_32BIT_END_KEEP   4
#define JTG_PCKT_SZ_RECV_RST        0
#define JTG_PCKT_SZ_RECV_FREQ_RD    4

#define JTG_VALID_BYTES_RECV_NBIT_KEEP  2


#define TDI_DATA_COUNT_SIZE      16
#define TMS_DATA_COUNT_SIZE      16
#define JTAG_NBIT_KEEP_DATA_SIZE 16

#define JTAG_OUT_BUFFER_SIZE     32///< OUT fifo size on JTAG peripheral
#define JTAG_IN_BUFFER_SIZE      32


//TODO - Review - why 6?
#define BITBANG_SEND_SIZE 6


#define JTG_QUEUABLEFLAG_NOP            0
#define JTG_QUEUABLEFLAG_FLUSH          0
#define JTG_QUEUABLEFLAG_FREQ_WR        0
#define JTG_QUEUABLEFLAG_DELAY          0
#define JTG_QUEUABLEFLAG_BITBANG        0
#define JTG_QUEUABLEFLAG_TMS            1
#define JTG_QUEUABLEFLAG_NBIT_TOSS      1
#define JTG_QUEUABLEFLAG_NBIT_KEEP      1
#define JTG_QUEUABLEFLAG_NBIT_END_TOSS  1
#define JTG_QUEUABLEFLAG_NBIT_END_KEEP  1
#define JTG_QUEUABLEFLAG_32BIT_TOSS     1
#define JTG_QUEUABLEFLAG_32BIT_KEEP     1
#define JTG_QUEUABLEFLAG_32BIT_END_TOSS 1
#define JTG_QUEUABLEFLAG_32BIT_END_KEEP 1
#define JTG_QUEUABLEFLAG_RST            0
#define JTG_QUEUABLEFLAG_FREQ_RD        0


///////////////////////Endian option/////////////////////////
#define CMD_JTAG_ENDIANOPT_NOP                  BIGENDIAN
#define CMD_JTAG_ENDIANOPT_FLUSH                BIGENDIAN
#define CMD_JTAG_ENDIANOPT_FREQ_WR              BIGENDIAN
#define CMD_JTAG_ENDIANOPT_DELAY                BIGENDIAN
#define CMD_JTAG_ENDIANOPT_BITBANG              BIGENDIAN
#define CMD_JTAG_ENDIANOPT_TMS                  BIGENDIAN
#define CMD_JTAG_ENDIANOPT_NBIT_TOSS            BIGENDIAN
#define CMD_JTAG_ENDIANOPT_NBIT_KEEP            BIGENDIAN
#define CMD_JTAG_ENDIANOPT_NBIT_END_TOSS        BIGENDIAN
#define CMD_JTAG_ENDIANOPT_NBIT_END_KEEP        BIGENDIAN
#define CMD_JTAG_ENDIANOPT_32BIT_TOSS           BIGENDIAN
#define CMD_JTAG_ENDIANOPT_32BIT_KEEP           BIGENDIAN
#define CMD_JTAG_ENDIANOPT_32BIT_END_TOSS       BIGENDIAN
#define CMD_JTAG_ENDIANOPT_32BIT_END_KEEP       BIGENDIAN
#define CMD_JTAG_ENDIANOPT_PERIPH_RST           BIGENDIAN
#define CMD_JTAG_ENDIANOPT_FREQ_RD              BIGENDIAN
////////////////////masks,shifts//////////////////////
#define EUD_JTAG_FREQ_MASK                      0x3

#define BITBANG_TDI_OFFSET                      0
#define BITBANG_TMS_OFFSET                      1
#define BITBANG_TCK_OFFSET                      2
#define BITBANG_NTRST_OFFSET                    3
#define BITBANG_NSRST_OFFSET                    4



///////////////////API and software defines///////////////////////
//#define JTAG_BIGENDIAN                1
#define JTG_LIMITED_INIT_OPTION         1
#define INIT_JTG_ALLOWED_OPTIONS_MSK    JTG_LIMITED_INIT_OPTION //Only 1 option allowed at this time.
#define JTG_CMD_DELAY_MAX               0xFF
#define JTG_CMD_FREQWR_MAX              0xD
#define EUD_JTG_INTERNAL_ERROR          130
#define EUD_JTG_ERR_TDO_EMPTY           111
//make same as SWD_EUD_DELAY_VALUE
#define JTG_EUD_DELAY_VALUE         18


///////////software buffer values///////////
#define TDO_BUFFER_MAX              8
#define TDI_DATA_IN_MAX             64
#define TMS_DATA_IN_MAX             64
#define TDO_DATA_IN_MAX             64

#define JTAG_DATA_BUF_UNIT_SIZE     8

//Max JTAG packets is 64. This value to ensure that (given_packets + opcodes) !> JTAG max packet size
//3 TDI bytes must be represented in two NBIT packets, resulting in 10 bytes. So exactly 48 bytes of data is ok, 
//But more than 46 bytes of data is not ok due to NBIT packets. 
//Thus subtract calculated max by 2
#define TDI_EXACT_MAX_BITS                      TDI_DATA_IN_MAX - (TDI_DATA_IN_MAX / (JTG_PCKT_SZ_SEND_GENERAL - 1)) 
#define TDI_MAX_NET_PACKETS                     TDI_EXACT_MAX_BITS-2
#define EUD_ERR_MAX_TDI_PACKETS_EXCEEDED        120;
//TMS packets are only 5 bytes in length (1 opcode, 4 data bytes), so simple calculation is enough here
#define TMS_MAX_NET_PACKETS                     TMS_DATA_IN_MAX-(TMS_DATA_IN_MAX/(JTG_PCKT_SZ_SEND_GENERAL-1)) //Max JTAG packets is 64. This value to ensure that (given_packets + opcodes) !> JTAG max packet size
#define EUD_ERR_MAX_TMS_PACKETS_EXCEEDED        113

#define DIV_ROUND_UP(m, n)                      (((m) + (n) - 1) / (n))



//===---------------------------------------------------------------------===//
//
//                  Structures, enums, data variables
//
//===---------------------------------------------------------------------===//



typedef struct {
    uint32_t deviceType;
} deviceconfig_t;

typedef enum shifttype {
    JTG_TMS = 1,
    JTG_TDI = 2
    
} shifttype_t;

typedef enum jtag_state {
    JTAG_STATE_INVALID = -1,

    JTAG_STATE_DREXIT2   = 0x0,
    JTAG_STATE_DREXIT1   = 0x1,
    JTAG_STATE_DRSHIFT   = 0x2,
    JTAG_STATE_DRPAUSE   = 0x3,
    JTAG_STATE_IRSELECT  = 0x4,
    JTAG_STATE_DRUPDATE  = 0x5,
    JTAG_STATE_DRCAPTURE = 0x6,
    JTAG_STATE_DRSELECT  = 0x7,
    JTAG_STATE_IREXIT2   = 0x8,
    JTAG_STATE_IREXIT1   = 0x9,
    JTAG_STATE_IRSHIFT   = 0xa,
    JTAG_STATE_IRPAUSE   = 0xb,
    JTAG_STATE_IDLE      = 0xc,
    JTAG_STATE_IRUPDATE  = 0xd,
    JTAG_STATE_IRCAPTURE = 0xe,
    JTAG_STATE_RESET     = 0x0f,

} jtag_state_t;


//===---------------------------------------------------------------------===//
//
//      Classes and standalone functions
//
//===---------------------------------------------------------------------===//
const char* get_tap_state_string(jtag_state_t state_idx);

uint32_t convert_uint8_to_hex(uint8_t* data);


#ifdef __cplusplus
class EudJtagData{
public:
    EudJtagData(){
        number_ = 0;
        //tdi = new uint16_t[TDI_DATA_IN_MAX];
        //tms = new uint16_t[TMS_DATA_IN_MAX];
        //tdo = new uint32_t[TDO_BUFFER_MAX];
        tdi_ = new uint8_t[TDI_DATA_IN_MAX];
        tms_ = new uint8_t[TMS_DATA_IN_MAX];
        tdo_ = new uint8_t[TDO_DATA_IN_MAX];

        memset(tms_, 0, TMS_DATA_IN_MAX);
        memset(tdi_, 0, TDI_DATA_IN_MAX);
        memset(tdo_, 0, TDO_DATA_IN_MAX);

    }
    ~EudJtagData(){
        delete tdi_;
        delete tms_;
        delete tdo_;
    }

    uint8_t *tdi_;
    uint8_t *tms_;
    uint8_t *tdo_;
    uint32_t number_;

    
};

class JtagStateMachine
{
public:
    JtagStateMachine() : current_state_(JTAG_STATE_RESET) {
        jtag_error_state_ = EUD_SUCCESS;
    }; //syntax to initialize field current_state with state JTAG_RESET
    
    

    jtag_state_t GetCurrentState()
    {
        return current_state_;
    };

    void SetState(jtag_state_t new_state)
    {
        current_state_ = new_state;
    }

    jtag_state_t CalcNextState(jtag_state_t state, uint32_t tms);
    EUD_ERR_t LogJtagError(EUD_ERR_t error_state);
    EUD_ERR_t GetJtagErrorState(void);
    EUD_ERR_t ResetJtagErrorState(void);

    

private:
    jtag_state_t current_state_;
    EUD_ERR_t jtag_error_state_;
};


//////////////////////////////////////////////
//
//  JTG EUD class
//
//      JTAG peripheral. Inherits from EUD device
//
//////////////////////////////////////////////
class JtagEudDevice: public EudDevice
{
public:
    //static JtagEudDevice *Instance();
    JtagEudDevice();
    
    ~JtagEudDevice() {
                //delete &jtag_state;
    }

    virtual int32_t Init();
    EUD_ERR_t Quit();
    void ResetCounters();
    
    uint8_t tdo_buffer_[TDO_DATA_IN_MAX];
    uint8_t tdo_index_;



    uint8_t* usb_write_buffer_p_;        ///<Buffer for USB Write Bytes. Size is SWD_OUT_BUFFER_SIZE (32)
    uint8_t* usb_read_buffer_p_;         ///<Default size for USB Read Buffer size. 
                                        ///<Hardware won't return value greater than 32 Bytes. 
                                        ///<Effective size may vary depending on Mode.

    uint32_t* usb_write_index_p_;        ///<Index for USB_Write_Buffer_p. 
    uint32_t* usb_read_index_p_;         ///<Index for USB_Read_Buffer_p.

    bool* flush_required_p_;              ///<Flag for flush required (call swd_flush).
    uint32_t* expected_bytes_p_;          ///<Logs expected bytes to be read into USB_Read_Buffer_p.

    uint32_t jtag_check_opcode_queuable_table_[JTG_NUM_OPCODES]; ///<Array to determine if certain opcodes are queued. Used by WriteCommand.

private:
    uint32_t tdo_data_available_; /** Data is available to be placed into TDO_Buffer */
public:

    ///////////////////////////////////////////////
    //Higher level / API-like functions
    //Perhaps to be  deprecated / revised
    //error handling
    EUD_ERR_t ResetJtagStateMachine(void);
    EUD_ERR_t ClearJtagStateMachineErrorState(void);
    EUD_ERR_t GetJtagStateMachineErrorState(void);
    
    
    //VI team
    EUD_ERR_t EudSetConfig(uint32_t device_number);
    deviceconfig_t* config_ = new deviceconfig_t;
    EUD_ERR_t JtagCmWriteRegister(uint32_t * reg_addr_p, uint32_t * reg_data_p);
    EUD_ERR_t JtagCmReadRegister(uint32_t * reg_addr_p, uint32_t *  rd_data_p);

    //Direct JTAG shift API's
    EUD_ERR_t EudShiftTms(const char* shift_value);
    EUD_ERR_t EudShiftTdi(const char* shift_value);
    EUD_ERR_t EudShiftReg(const char* shift_value);
    EUD_ERR_t EudReadTdo(uint8_t* tdo_data_ret);
    EUD_ERR_t EudFillTdoBuffer(EudJtagData* jtag_data);

    EUD_ERR_t ShiftRawEud(EudJtagData * data);

    EUD_ERR_t EudJtagTmsShift(uint32_t tms_val, uint32_t clock_ticks);
    EUD_ERR_t EudJtagTdiShift(uint32_t tdi_val, uint32_t clock_ticks);
    EUD_ERR_t EudJtagTmsShift(uint16_t* raw_bitstream, uint32_t clock_ticks);
    EUD_ERR_t EudJtagTdiShift(uint16_t* raw_bitstream, uint32_t clock_ticks);

    EUD_ERR_t EudJtagGenericShift(shifttype_t shift_type, uint16_t* raw_bitstream, uint32_t clock_ticks);
    //////////////////////////////////////////////////////////////
    //Lower level functions - perhaps to be made private
    
    
    //EUD_ERR_t get_speed(uint32_t khz, uint32_t *speed);
    EUD_ERR_t BitBang(uint32_t tdi,
                uint32_t tms, 
                uint32_t tck, 
                uint32_t trst, 
                uint32_t srst,
                uint32_t enable = 0);

    EUD_ERR_t JtagScan(uint8_t *tms_raw, 
                    uint8_t *tdi_raw, 
                    uint8_t *tdo_raw, 
                    uint32_t scan_length);


    // Calculate the number of bits available in the buffer for tms or tdi bits
    uint32_t BitsFreeTms();
    uint32_t BitsFreeTdi();

    void RunScan(uint8_t scan_length, 
                    uint8_t *tdo, 
                    uint8_t tdo_offset, 
                    bool final_scan);

    // Push the contents of the tdi queue into the USB out buffer
    EUD_ERR_t FlushTdiQueue(uint32_t tdi_count,bool final_scan);

    // Push the contents of the tms queue into the USB out buffer
    EUD_ERR_t FlushTmsQueue(void);
            
    // Shift 1 tdi or tms bit into respective buffers
    void InsertTdi(uint8_t tdi, uint8_t *tdo);
    void InsertTms(uint8_t tms);
    

    // utility functions for creating commands
    uint8_t *PackCmd32(uint8_t cmd,
                            uint8_t *dest,
                            const uint8_t *src);
    uint8_t *PackCmdN(uint8_t cmd,
                        uint8_t *dest,
                        const uint16_t data,
                        const uint16_t count);

    uint8_t usb_buffer_in_[JTAG_IN_BUFFER_SIZE];
    
    EUD_ERR_t UsbWriteRead(uint32_t out_length, uint8_t* out_buffer, uint32_t in_length, uint8_t* in_buffer);

    USB_ERR_t UsbWriteRead(
        uint32_t out_length,    ///<Number of bytes to write to USB from usb_buffer_out_.
        uint32_t in_length      ///<Number of bytes to read from USB to usb_buffer_in_.
        );


    EUD_ERR_t ProcessBuffers();
    /* API to assert a reset using a EUD peripheral */
    EUD_ERR_t AssertReset(void);

    /* API to de-assert reset using a EUD peripheral */
    EUD_ERR_t DeAssertReset(void);

private:
    
    //Instance of jtag statemachine class
    JtagStateMachine jtag_state_;

    // Current length of JTAG data (in bits)
    uint32_t tms_count_;
    uint32_t tdi_count_;

    uint8_t tms_buffer_[JTAG_OUT_BUFFER_SIZE];
    uint8_t tdi_buffer_[JTAG_OUT_BUFFER_SIZE];

    //uint8_t TDO_Buffer[TDO_DATA_IN_MAX];
    uint32_t tdo_buffer_idx_;

    // The number of bytes expected back from the device (tdo count)
    uint32_t num_tdo_bytes_expected_;
  
};

#else  // __cplusplus
typedef void JtagEudDevice;
#endif // __cplusplus

#endif


