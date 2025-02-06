/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   ctl_eud.h
*
* Description:                                                              
*   Header file for EUD CTL Peripheral APIs declerations and 
*   various header information
*
***************************************************************************/
#ifndef CTL_EUD_H_
#define CTL_EUD_H_

#include "eud.h"

//===---------------------------------------------------------------------===//
//
//        Top level defines, typedef's
//
//===---------------------------------------------------------------------===//
#ifndef MAJOR_REV_ID
#define MAJOR_REV_ID                1   ///<Can be retrieved from eud_get_version
#endif
#ifndef MINOR_REV_ID
#define MINOR_REV_ID                14  ///<Can be retrieved from eud_get_version
#endif
#ifndef SPIN_REV_ID
#define SPIN_REV_ID                 6   ///<Can be retrieved from eud_get_version
#endif
/////////////////////////////Defines from EUD Spec///////////////////////

#define CTL_CMD_NOP                 0x00 
#define CTL_CMD_EUD_VERSION_READ    0x01 
#define CTL_CMD_DEVICE_ID_READ      0x02 
#define CTL_CMD_EUD_STATUS_READ     0x03 
#define CTL_CMD_CTLIN_READ          0x04 
#define CTL_CMD_CTLOUT_READ         0x05 
#define CTL_CMD_CTLOUT_WRITE        0x06 
#define CTL_CMD_CTLOUT_SET          0x07 
#define CTL_CMD_CTLOUT_CLR          0x08 
#define CTL_CMD_PERIPH_RST          0x09 
#define CTL_CMD_FLAG_IN             0x0A 
#define CTL_CMD_FLAG_OUT_WRITE      0x0B 
#define CTL_CMD_FLAG_OUT_SET        0x0C 
#define CTL_CMD_FLAG_OUT_CLR        0x0D 

///////////////Packet size definitions//////////////////

#define CTL_CMD_CTLOUT_READ_PKT_OUT_SIZE    1
#define CTL_CMD_CTLOUT_READ_PKT_IN_SIZE     4
#define CTL_CMD_CTLOUTSET_PKT_OUT_SIZE      5
#define CTL_CMD_CTLOUTSET_PKT_IN_SIZE       0
#define CTL_CMD_CTLOUT_CLR_PKT_OUT_SIZE     5
#define CTL_CMD_CTLOUT_CLR_PKT_IN_SIZE      0

#define CTL_PCKT_SZ_SEND_NOP                1
#define CTL_PCKT_SZ_SEND_EUD_VERSION_READ   1
#define CTL_PCKT_SZ_SEND_DEVICE_ID_READ     1
#define CTL_PCKT_SZ_SEND_EUD_STATUS_READ    1
#define CTL_PCKT_SZ_SEND_CTLIN_READ         1
#define CTL_PCKT_SZ_SEND_CTLOUT_READ        1
#define CTL_PCKT_SZ_SEND_CTLOUT_WRITE       5
#define CTL_PCKT_SZ_SEND_CTLOUTSET          5
#define CTL_PCKT_SZ_SEND_CTLOUT_CLR         5
#define CTL_PCKT_SZ_SEND_PERIPH_RST         1
#define CTL_PCKT_SZ_SEND_FLAG_IN            1
#define CTL_PCKT_SZ_SEND_FLAG_OUT_WRITE     5
#define CTL_PCKT_SZ_SEND_FLAG_OUT_SET       5
#define CTL_PCKT_SZ_SEND_FLAG_OUT_CLR       5

#define CTL_PCKT_SZ_RECV_NOP                0
#define CTL_PCKT_SZ_RECV_EUD_VERSION_READ   4
#define CTL_PCKT_SZ_RECV_DEVICE_ID_READ     4
#define CTL_PCKT_SZ_RECV_EUD_STATUS_READ    4
#define CTL_PCKT_SZ_RECV_CTLIN_READ         4
#define CTL_PCKT_SZ_RECV_CTLOUT_READ        4
#define CTL_PCKT_SZ_RECV_CTLOUT_WRITE       0
#define CTL_PCKT_SZ_RECV_CTLOUTSET          0
#define CTL_PCKT_SZ_RECV_CTLOUT_CLR         0
#define CTL_PCKT_SZ_RECV_PERIPH_RST         0
#define CTL_PCKT_SZ_RECV_FLAG_IN            4
#define CTL_PCKT_SZ_RECV_FLAG_OUT_WRITE     0
#define CTL_PCKT_SZ_RECV_FLAG_OUT_SET       0
#define CTL_PCKT_SZ_RECV_FLAG_OUT_CLR       0


#define CTL_NUM_OPCODES                     13


////////////////////CTL register shifts///////////////////
#define CTL_CTL_RCTLR_SRST_N_SHFT   0
#define CTL_EDL_REQ_SHFT            1
#define CTL_SWD_PERIPH_EN_SHFT      2
#define CTL_TRACE_PERIPH_EN_SHFT    3
#define CTL_JTAG_PERIPH_EN_SHFT     4
#define CTL_COM_PERIPH_EN_SHFT      5
#define CTL_VDDMIN_TCK_EN_SHFT      6
#define CTL_GPIO_JTAG_SEL_SHFT      7
#define CTL_GPIO_SWD_SEL_SHFT       8
#define CTL_MTAP_MUX_SEL_SHFT       9
#define CTL_DAP_MUX_SEL_SHFT        10
#define CTL_ACC_TAP_SEL_SHFT        11
#define CTL_VBUS_ATTACH_SHFT        12
#define CTL_VBUS_INT_SHFT           13
#define CTL_CHGR_EN_SHFT            14
#define CTL_CHGR_INT_SHFT           15
#define CTL_HOST_EUD_DIS_SHFT       16
#define CTL_ACC_TDO_SEL_SHFT        17
#define CTL_DAP_TDO_SEL_SHFT        18
#define CTL_MTAP_TDO_SEL_SHFT       19
#define CTL_DAP_SWDO_SEL_SHFT       20

#define CTL_STATUS_REG_SIZE 21

////////////////////Endian options to parse//////////////////
#define CTL_ENDIANOPT_NOP                   BIGENDIAN 
#define CTL_ENDIANOPT_EUD_VERSION_READ      BIGENDIAN 
#define CTL_ENDIANOPT_DEVICE_ID_READ        LITTLEENDIAN 
#define CTL_ENDIANOPT_EUD_STATUS_READ       BIGENDIAN 
#define CTL_ENDIANOPT_CTLIN_READ            BIGENDIAN 
#define CTL_ENDIANOPT_CTLOUT_READ           BIGENDIAN 
#define CTL_ENDIANOPT_CTLOUT_WRITE          BIGENDIAN 
#define CTL_ENDIANOPT_CTLOUT_SET            BIGENDIAN 
#define CTL_ENDIANOPT_CTLOUT_CLR            BIGENDIAN 
#define CTL_ENDIANOPT_PERIPH_RST            BIGENDIAN 
#define CTL_ENDIANOPT_FLAG_IN               BIGENDIAN 
#define CTL_ENDIANOPT_FLAG_OUT_WRITE        BIGENDIAN 
#define CTL_ENDIANOPT_FLAG_OUT_SET          BIGENDIAN 
#define CTL_ENDIANOPT_FLAG_OUT_CLR          BIGENDIAN 


const char* const CTL_REG_STRING_ARRAY[] = {
    "CTL_CTL_RCTLR_SRST_N_SHFT ",
    "CTL_EDL_REQ_SHFT          ",
    "CTL_SWD_PERIPH_EN_SHFT    ",
    "CTL_TRACE_PERIPH_EN_SHFT  ",
    "CTL_JTAG_PERIPH_EN_SHFT   ",
    "CTL_COM_PERIPH_EN_SHFT    ",
    "CTL_VDDMIN_TCK_EN_SHFT    ",
    "CTL_GPIO_JTAG_SEL_SHFT    ",
    "CTL_GPIO_SWD_SEL_SHFT     ",
    "CTL_MTAP_MUX_SEL_SHFT     ",
    "CTL_DAP_MUX_SEL_SHFT      ",
    "CTL_ACC_TAP_SEL_SHFT      ",
    "CTL_VBUS_ATTACH_SHFT      ",
    "CTL_VBUS_INT_SHFT         ",
    "CTL_CHGR_EN_SHFT          ",
    "CTL_CHGR_INT_SHFT         ",
    "CTL_HOST_EUD_DIS_SHFT     ",
    "CTL_ACC_TDO_SEL_SHFT      ",
    "CTL_DAP_TDO_SEL_SHFT      ",
    "CTL_MTAP_TDO_SEL_SHFT     ",
    "CTL_DAP_SWDO_SEL_SHFT     "
};


////////////////////////CTL Peripheral Enablement defines///////////////////////////
///                                                                              ///
///   These defines specify what control register bits are to be set or  cleared ///
///   upon initialization of various peripherals. These may change depending on  ///
///   target or hardware revision.                                               ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////


#ifdef FPGA
#define CTL_PAYLOAD_JTAGON          (1<<CTL_JTAG_PERIPH_EN_SHFT     )+\
                                    (1<<CTL_GPIO_JTAG_SEL_SHFT      )

#define CTL_PAYLOAD_JTAGOFF         (1<<CTL_JTAG_PERIPH_EN_SHFT     )+\
                                    (1 << CTL_MTAP_MUX_SEL_SHFT     )+\
                                    (1 << CTL_GPIO_JTAG_SEL_SHFT    )+\
                                    (1 << CTL_DAP_MUX_SEL_SHFT      )+\
                                    (1 << CTL_DAP_TDO_SEL_SHFT      )
                                    
#else
#define CTL_PAYLOAD_JTAGON          (1<<CTL_JTAG_PERIPH_EN_SHFT     )+\
                                    (1<<CTL_MTAP_MUX_SEL_SHFT       )+\
                                    (1<<CTL_DAP_MUX_SEL_SHFT        )+\
                                    (1<<CTL_DAP_TDO_SEL_SHFT        )

#define CTL_PAYLOAD_JTAGOFF         (1<<CTL_JTAG_PERIPH_EN_SHFT     )+\
                                    (1<<CTL_MTAP_MUX_SEL_SHFT       )+\
                                    (1<<CTL_DAP_MUX_SEL_SHFT        )+\
                                    (1<<CTL_DAP_TDO_SEL_SHFT        )

#endif

#define CTL_BITSCLEAR_JTAGON        (1<<CTL_SWD_PERIPH_EN_SHFT      )+\
                                    (1<<CTL_GPIO_SWD_SEL_SHFT       )+\
                                    (1<<CTL_ACC_TDO_SEL_SHFT        )+\
                                    (1<<CTL_DAP_TDO_SEL_SHFT        )+\
                                    (1<<CTL_MTAP_TDO_SEL_SHFT       )+\
                                    (1<<CTL_DAP_SWDO_SEL_SHFT       )

#define CTL_BITSCLEAR_JTAGOFF       0


//#ifdef FPGA ///<Turns out that CTL_PAYLOAD_SWDON_DAP is the right config for FPGA setup
#if 0
#define CTL_PAYLOAD_SWDON           (1<<CTL_CTL_RCTLR_SRST_N_SHFT   )+\
                                    (1<<CTL_SWD_PERIPH_EN_SHFT      )+\
                                    (1<<CTL_GPIO_SWD_SEL_SHFT       )+\
                                    (1<<CTL_VDDMIN_TCK_EN_SHFT      )+\
                                    (0<<CTL_DAP_MUX_SEL_SHFT        )

#define CTL_PAYLOAD_SWDOFF          (1<<CTL_SWD_PERIPH_EN_SHFT      )+\
                                    (1<<CTL_DAP_SWDO_SEL_SHFT       )+\
                                    (1<<CTL_VDDMIN_TCK_EN_SHFT      )+\
                                    (1<<CTL_DAP_MUX_SEL_SHFT        )
#else

#define CTL_PAYLOAD_SWDON_DAP       (1<<CTL_CTL_RCTLR_SRST_N_SHFT   )+\
                                    (1<<CTL_SWD_PERIPH_EN_SHFT      )+\
                                    (1<<CTL_DAP_SWDO_SEL_SHFT       )+\
                                    (1<<CTL_VDDMIN_TCK_EN_SHFT      )+\
                                    (1<<CTL_DAP_MUX_SEL_SHFT        )

#define CTL_PAYLOAD_SWDON_GPIO      (1<<CTL_CTL_RCTLR_SRST_N_SHFT   )+\
                                    (1<<CTL_SWD_PERIPH_EN_SHFT      )+\
                                    (1<<CTL_GPIO_SWD_SEL_SHFT       )+\
                                    (1<<CTL_VDDMIN_TCK_EN_SHFT      )+\
                                    (1<<CTL_DAP_MUX_SEL_SHFT        )

#define CTL_PAYLOAD_SWDON           CTL_PAYLOAD_SWDON_DAP


#define CTL_PAYLOAD_SWDOFF          0


#endif


#define CTL_BITSCLEAR_SWDON         (1<<CTL_JTAG_PERIPH_EN_SHFT     )+\
                                    (1<<CTL_GPIO_JTAG_SEL_SHFT      )+\
                                    (1<<CTL_MTAP_TDO_SEL_SHFT       )+\
                                    (1<<CTL_ACC_TDO_SEL_SHFT        )+\
                                    (1<<CTL_DAP_TDO_SEL_SHFT        )

#define CTL_BITSCLEAR_SWDOFF        (1<<CTL_SWD_PERIPH_EN_SHFT      )+\
                                    (1<<CTL_DAP_SWDO_SEL_SHFT       )+\
                                    (1<<CTL_VDDMIN_TCK_EN_SHFT      )+\
                                    (1<<CTL_DAP_MUX_SEL_SHFT        )

#define CTL_PAYLOAD_TRCON           (1<<CTL_TRACE_PERIPH_EN_SHFT    ) //0x0000008 

#define CTL_BITSCLEAR_TRCON         0x0
#define CTL_BITSCLEAR_TRCOFF        CTL_PAYLOAD_TRCON

#define CTL_PAYLOAD_COMON           (1<<CTL_COM_PERIPH_EN_SHFT      ) 
#define CTL_PAYLOAD_COMOFF          0

#define CTL_BITSCLEAR_COMON         0x0 
#define CTL_BITSCLEAR_COMOFF        CTL_PAYLOAD_COMON 

//#define CTL_PAYLOAD_REINIT_CTL    0x01000E00
#define CTL_PAYLOAD_REINIT_CTL      (1<<CTL_MTAP_MUX_SEL_SHFT       )+\
                                    (1<<CTL_DAP_MUX_SEL_SHFT        )+\
                                    (1<<CTL_ACC_TAP_SEL_SHFT        )+\
                                    (1<<CTL_CTL_RCTLR_SRST_N_SHFT   )



////////////////////////CTL API defines, various limits//////////////////////////

#define EUD_CTL_INIT_NO_OPTIONS_VALUE 0

#define CTL_USB_READ_SIZE 4

#define INIT_CTL_ALLOWED_OPTIONS_MSK 0 ///<Initialization options mask. No options allowed at this time.
#define MAX_EUDMSMRESET_TIME_MS 10000 //Define as 10seconds for now.
#define CTL_MAX_VBUS_INT_TMOUT 5000 //5 seconds
#define CTL_MAX_CHGR_INT_TMOUT 5000 //5 seconds


//===---------------------------------------------------------------------===//
//                  
//  Function Declarations and  Classes
//
//===---------------------------------------------------------------------===//

/****************************************************************************//**
*   @brief CtlEudDevice Class - class for EUD CTL peripheral. 
*   Inherits from EudDevice. Basically no changes from pure EudDevice class. 
*
*********************************************************************************/
#ifdef __cplusplus
class CtlEudDevice: public EudDevice
{
public:
    CtlEudDevice();   ///<Constructor
    ~CtlEudDevice(){  ///<Destructor
        Quit();
    }
    
private:
};
#else
typedef void CtlEudDevice;
#endif

///Helper function to poll on a particular bit from ctl status register.
///Returns true if bit has been set. False if time ran out or if an error ocurred.
bool 
poll_on_ctol_bit(  CtlEudDevice* ctl_handle_p, \
                uint32_t mask_value, \
                uint32_t timeout,\
                EUD_ERR_t* err_p\
    );
#endif //CTL_EUD_H_

