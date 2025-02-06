/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   ctl_eud.cpp
*
* Description:                                                              
*   CPP source file for CTL EUD Peripheral
*
***************************************************************************/

#include "eud.h"
#include "ctl_eud.h"
#include <string>
extern "C" {
#include <assert.h>
}
#include <iostream>
#include <ctime>
#include <sstream>
#include <fstream>

CtlEudDevice::CtlEudDevice() {
    Init();

    read_time_elapsed_ = 0;
    write_time_elapsed_ = 0;
    num_of_writes_ = 0;
    num_of_reads_ = 0;

    //UsbDevice *usb_device_handle = EMPTY_USB_HANDLE;
    usb_device_handle_init_flag_ = false;

    device_type_ = DEVICETYPE_EUD_CTL;
    device_id_ = 0;
    //DvcMgr_DeviceName = EUD_CTL_DEVMGR_NAME;
    DvcMgr_DeviceName = std::string(EUD_CTL_DEVMGR_NAME);

    periph_payload_size_table_[CTL_CMD_NOP] = CTL_PCKT_SZ_SEND_NOP;
    periph_payload_size_table_[CTL_CMD_EUD_VERSION_READ] = CTL_PCKT_SZ_SEND_EUD_VERSION_READ;
    periph_payload_size_table_[CTL_CMD_DEVICE_ID_READ] = CTL_PCKT_SZ_SEND_DEVICE_ID_READ;
    periph_payload_size_table_[CTL_CMD_EUD_STATUS_READ] = CTL_PCKT_SZ_SEND_EUD_STATUS_READ;
    periph_payload_size_table_[CTL_CMD_CTLIN_READ] = CTL_PCKT_SZ_SEND_CTLIN_READ;
    periph_payload_size_table_[CTL_CMD_CTLOUT_READ] = CTL_PCKT_SZ_SEND_CTLOUT_READ;
    periph_payload_size_table_[CTL_CMD_CTLOUT_WRITE] = CTL_PCKT_SZ_SEND_CTLOUT_WRITE;
    periph_payload_size_table_[CTL_CMD_CTLOUT_SET] = CTL_PCKT_SZ_SEND_CTLOUTSET;
    periph_payload_size_table_[CTL_CMD_CTLOUT_CLR] = CTL_PCKT_SZ_SEND_CTLOUT_CLR;
    periph_payload_size_table_[CTL_CMD_PERIPH_RST] = CTL_PCKT_SZ_SEND_PERIPH_RST;
    periph_payload_size_table_[CTL_CMD_FLAG_IN] = CTL_PCKT_SZ_SEND_FLAG_IN;
    periph_payload_size_table_[CTL_CMD_FLAG_OUT_WRITE] = CTL_PCKT_SZ_SEND_FLAG_OUT_WRITE;
    periph_payload_size_table_[CTL_CMD_FLAG_OUT_SET] = CTL_PCKT_SZ_SEND_FLAG_OUT_SET;
    periph_payload_size_table_[CTL_CMD_FLAG_OUT_CLR] = CTL_PCKT_SZ_SEND_FLAG_OUT_CLR;

    periph_response_size_table_[CTL_CMD_NOP] = CTL_PCKT_SZ_RECV_NOP;
    periph_response_size_table_[CTL_CMD_EUD_VERSION_READ] = CTL_PCKT_SZ_RECV_EUD_VERSION_READ;
    periph_response_size_table_[CTL_CMD_DEVICE_ID_READ] = CTL_PCKT_SZ_RECV_DEVICE_ID_READ;
    periph_response_size_table_[CTL_CMD_EUD_STATUS_READ] = CTL_PCKT_SZ_RECV_EUD_STATUS_READ;
    periph_response_size_table_[CTL_CMD_CTLIN_READ] = CTL_PCKT_SZ_RECV_CTLIN_READ;
    periph_response_size_table_[CTL_CMD_CTLOUT_READ] = CTL_PCKT_SZ_RECV_CTLOUT_READ;
    periph_response_size_table_[CTL_CMD_CTLOUT_WRITE] = CTL_PCKT_SZ_RECV_CTLOUT_WRITE;
    periph_response_size_table_[CTL_CMD_CTLOUT_SET] = CTL_PCKT_SZ_RECV_CTLOUTSET;
    periph_response_size_table_[CTL_CMD_CTLOUT_CLR] = CTL_PCKT_SZ_RECV_CTLOUT_CLR;
    periph_response_size_table_[CTL_CMD_PERIPH_RST] = CTL_PCKT_SZ_RECV_PERIPH_RST;
    periph_response_size_table_[CTL_CMD_FLAG_IN] = CTL_PCKT_SZ_RECV_FLAG_IN;
    periph_response_size_table_[CTL_CMD_FLAG_OUT_WRITE] = CTL_PCKT_SZ_RECV_FLAG_OUT_WRITE;
    periph_response_size_table_[CTL_CMD_FLAG_OUT_SET] = CTL_PCKT_SZ_RECV_FLAG_OUT_SET;
    periph_response_size_table_[CTL_CMD_FLAG_OUT_CLR] = CTL_PCKT_SZ_RECV_FLAG_OUT_CLR;
    
    periph_endian_option_table_[CTL_CMD_NOP] = CTL_ENDIANOPT_NOP;
    periph_endian_option_table_[CTL_CMD_EUD_VERSION_READ] = CTL_ENDIANOPT_EUD_VERSION_READ;
    periph_endian_option_table_[CTL_CMD_DEVICE_ID_READ] = CTL_ENDIANOPT_DEVICE_ID_READ;
    periph_endian_option_table_[CTL_CMD_EUD_STATUS_READ] = CTL_ENDIANOPT_EUD_STATUS_READ;
    periph_endian_option_table_[CTL_CMD_CTLIN_READ] = CTL_ENDIANOPT_CTLIN_READ;
    periph_endian_option_table_[CTL_CMD_CTLOUT_READ] = CTL_ENDIANOPT_CTLOUT_READ;
    periph_endian_option_table_[CTL_CMD_CTLOUT_WRITE] = CTL_ENDIANOPT_CTLOUT_WRITE;
    periph_endian_option_table_[CTL_CMD_CTLOUT_SET] = CTL_ENDIANOPT_CTLOUT_SET;
    periph_endian_option_table_[CTL_CMD_CTLOUT_CLR] = CTL_ENDIANOPT_CTLOUT_CLR;
    periph_endian_option_table_[CTL_CMD_PERIPH_RST] = CTL_ENDIANOPT_PERIPH_RST;
    periph_endian_option_table_[CTL_CMD_FLAG_IN] = CTL_ENDIANOPT_FLAG_IN;
    periph_endian_option_table_[CTL_CMD_FLAG_OUT_WRITE] = CTL_ENDIANOPT_FLAG_OUT_WRITE;
    periph_endian_option_table_[CTL_CMD_FLAG_OUT_SET] = CTL_ENDIANOPT_FLAG_OUT_SET;
    periph_endian_option_table_[CTL_CMD_FLAG_OUT_CLR] = CTL_ENDIANOPT_FLAG_OUT_CLR;

    periph_max_opcode_value_ = CTL_NUM_OPCODES;
}
