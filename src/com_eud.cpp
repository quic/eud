/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   com_eud.cpp
*
* Description:                                                              
*   CPP source file for COM EUD peripheral
*
***************************************************************************/

#include "eud.h"
#include "com_eud.h"
#include <string>
extern "C" {
#include <assert.h>
}
#include <iostream>
#include <ctime>
#include <sstream>
#include <fstream>

EUD_ERR_t
ComEudDevice::Init(void)
{
    //assert(SWD_IN_BUFFER_SIZE == JTAG_IN_BUFFER_SIZE);
    //usb_handle_CTL = INVALID_HANDLE_VALUE;
    return 0; 
}

//Constructor
ComEudDevice::ComEudDevice()
{
    Init();

    //usb_device *usb_device_handle = EMPTY_USB_HANDLE;
    usb_device_handle_init_flag_ = false;
    device_type_ = DEVICETYPE_EUD_COM;
    device_id_ = 0;
    //DvcMgr_DeviceName = EUD_COM_DEVMGR_NAME;
    DvcMgr_DeviceName = std::string(EUD_COM_DEVMGR_NAME); //new char[MAX_DEVMGR_NAME_CHARLEN];


    periph_payload_size_table_[COM_CMD_NOP] = COM_PCKT_SZ_SEND_NOP;
    periph_payload_size_table_[COM_CMD_TX_TMOUT] = COM_PCKT_SZ_SEND_TX_TMOUT;
    periph_payload_size_table_[COM_CMD_RX_TMOUT] = COM_PCKT_SZ_SEND_RX_TMOUT;
    periph_payload_size_table_[COM_CMD_PORT_RESET] = COM_PCKT_SZ_SEND_PORT_RST;

    periph_response_size_table_[COM_CMD_NOP] = COM_PCKT_SZ_RECV_NOP;
    periph_response_size_table_[COM_CMD_TX_TMOUT] = COM_PCKT_SZ_RECV_TX_TMOUT;
    periph_response_size_table_[COM_CMD_RX_TMOUT] = COM_PCKT_SZ_RECV_RX_TMOUT;
    periph_response_size_table_[COM_CMD_PORT_RESET] = COM_PCKT_SZ_RECV_PORT_RST;
    
    periph_endian_option_table_[COM_CMD_NOP] = COM_ENDIANOPT_NOP;
    periph_endian_option_table_[COM_CMD_TX_TMOUT] = COM_ENDIANOP_TX_TMOUT;
    periph_endian_option_table_[COM_CMD_RX_TMOUT] = COM_ENDIANOP_RX_TMOUT;
    periph_endian_option_table_[COM_CMD_PORT_RESET] = COM_ENDIANOP_PORT_RST;
    
    periph_max_opcode_value_ = COM_NUM_OPCODES;

}