/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   swd_eud.cpp
*
* Description:                                                              
*   CPP source file for EUD SWD Peripheral APIs
*
***************************************************************************/

#include "swd_eud.h"
#include "swd_api.h"

SwdEudDevice::SwdEudDevice() {  ///< \ref SwdEudDevice generator
    Init();                         ///< generic initializer
    //UsbDevice *usb_device_handle = EMPTY_USB_HANDLE;
    device_type_ = DEVICETYPE_EUD_SWD;
    device_id_ = 0;
    DvcMgr_DeviceName = std::string(EUD_SWD_DEVMGR_NAME); //new char[MAX_DEVMGR_NAME_CHARLEN];
    usb_device_handle_init_flag_ = false;

    swd_to_jtag_operation_done_ = FALSE;
    eud_queue_p_ = new EudPacketQueue;
    swd_status_sent_flag_ = false;

    periph_payload_size_table_[SWD_CMD_NOP] = SWD_PCKT_SZ_SEND_NOP;
    periph_payload_size_table_[SWD_CMD_FLUSH] = SWD_PCKT_SZ_SEND_FLUSH;
    periph_payload_size_table_[SWD_CMD_FREQ] = SWD_PCKT_SZ_SEND_FREQ;
    periph_payload_size_table_[SWD_CMD_DELAY] = SWD_PCKT_SZ_SEND_DELAY;
    periph_payload_size_table_[SWD_CMD_BITBANG] = SWD_PCKT_SZ_SEND_BITBANG;
    periph_payload_size_table_[SWD_CMD_DITMS] = SWD_PCKT_SZ_SEND_DITMS;
    periph_payload_size_table_[SWD_CMD_TIMING] = SWD_PCKT_SZ_SEND_TIMING;
    periph_payload_size_table_[SWD_CMD_STATUS] = SWD_PCKT_SZ_SEND_STATUS;
    periph_payload_size_table_[SWD_CMD_PERIPH_RST] = SWD_PCKT_SZ_SEND_RST;

    periph_response_size_table_[SWD_CMD_NOP] = SWD_PCKT_SZ_RECV_NOP;
    periph_response_size_table_[SWD_CMD_FLUSH] = SWD_PCKT_SZ_RECV_FLUSH;
    periph_response_size_table_[SWD_CMD_FREQ] = SWD_PCKT_SZ_RECV_FREQ;
    periph_response_size_table_[SWD_CMD_DELAY] = SWD_PCKT_SZ_RECV_DELAY;
    periph_response_size_table_[SWD_CMD_BITBANG] = SWD_PCKT_SZ_RECV_BITBANG;
    periph_response_size_table_[SWD_CMD_DITMS] = SWD_PCKT_SZ_RECV_DITMS;
    periph_response_size_table_[SWD_CMD_TIMING] = SWD_PCKT_SZ_RECV_TIMING;
    periph_response_size_table_[SWD_CMD_STATUS] = SWD_PCKT_SZ_RECV_STATUS;
    periph_response_size_table_[SWD_CMD_PERIPH_RST] = SWD_PCKT_SZ_RECV_RST;

    
    periph_endian_option_table_[SWD_CMD_NOP] = SWD_ENDIANOPT_NOP;
    periph_endian_option_table_[SWD_CMD_FLUSH] = SWD_ENDIANOPT_FLUSH;
    periph_endian_option_table_[SWD_CMD_FREQ] = SWD_ENDIANOPT_FREQ;
    periph_endian_option_table_[SWD_CMD_DELAY] = SWD_ENDIANOPT_DELAY;
    periph_endian_option_table_[SWD_CMD_BITBANG] = SWD_ENDIANOPT_BITBANG;
    periph_endian_option_table_[SWD_CMD_DITMS] = SWD_ENDIANOPT_DITMS;
    periph_endian_option_table_[SWD_CMD_TIMING] = SWD_ENDIANOPT_TIMING;
    periph_endian_option_table_[SWD_CMD_STATUS] = SWD_ENDIANOPT_STATUS;
    periph_endian_option_table_[SWD_CMD_PERIPH_RST] = SWD_ENDIANOPT_PERIPH_RST;
    
    periph_max_opcode_value_ = SWD_NUM_OPCODES;

    dp_select_reg_hist_ = { 0 };
}
SwdEudDevice::~SwdEudDevice(){

    Quit();

}

EUD_ERR_t SwdEudDevice::AssertReset(void) {
    
    uint32_t return_val = 0, DPIDR = 0;
    // Assert reset and tap reset
    uint32_t swd_bitbang_value = 
        SWD_BITBANG_CLK_BMSK_DEASSERT +
        SWD_BITBANG_DI_BMSK_DEASSERT +
        SWD_BITBANG_RCTLR_SRST_BMSK_ASSERT + //this is the reset value
        SWD_BITBANG_GPIO_DI_OE_DEASSERT +
        SWD_BITBANG_GPIO_SRST_BMSK_DEASSERT +
        SWD_BITBANG_GPIO_TRST_BMSK_DEASSERT +
        SWD_BITBANG_DAP_TRST_BMSK_ASSERT; //Also reset TAP


    EUD_ERR_t err = swd_bitbang(this, swd_bitbang_value, &return_val);
    
    // De-assert tap reset but keep SRST asserted.
    swd_bitbang_value = 
        SWD_BITBANG_CLK_BMSK_DEASSERT +
        SWD_BITBANG_DI_BMSK_DEASSERT +
        SWD_BITBANG_RCTLR_SRST_BMSK_ASSERT + //this is the reset value
        SWD_BITBANG_GPIO_DI_OE_DEASSERT +
        SWD_BITBANG_GPIO_SRST_BMSK_DEASSERT +
        SWD_BITBANG_GPIO_TRST_BMSK_DEASSERT +
        SWD_BITBANG_DAP_TRST_BMSK_DEASSERT; //Deassert TAP reset

    err = swd_bitbang(this, swd_bitbang_value, &return_val);

    // This below has to be checked , how to handle it for adiv6
    err = jtag_to_swd_adiv5(this);

    err = swd_get_jtag_id(this, &DPIDR);

    return err;
}

EUD_ERR_t SwdEudDevice::DeAssertReset(void) {

    uint32_t return_val = 0;
    //deassert reset
    uint32_t swd_bitbang_value = 
        SWD_BITBANG_CLK_BMSK_DEASSERT +
        SWD_BITBANG_DI_BMSK_DEASSERT +
        SWD_BITBANG_RCTLR_SRST_BMSK_DEASSERT + //this is thede reset value
        SWD_BITBANG_GPIO_DI_OE_DEASSERT +
        SWD_BITBANG_GPIO_SRST_BMSK_DEASSERT +
        SWD_BITBANG_GPIO_TRST_BMSK_DEASSERT +
        SWD_BITBANG_DAP_TRST_BMSK_DEASSERT;


    EUD_ERR_t err = swd_bitbang(this, swd_bitbang_value, &return_val);
    
    return err;
}


std::string SwdEudDevice::GetDpRegInfo(uint32_t regidx, uint32_t ReadNWrite) {
    std::string ret ("Undefined");

    switch (regidx)
    {
        case 0:
            ret = ReadNWrite ? "DPIDR register" : "ABORT register";
            break;
        case 0x4:
            switch (dp_select_reg_hist_.dp_select_reg.dpbanksel)
            {
                case 0:
                    ret = "CTRL/STAT register";
                    break;
                case 1:
                    ret = "DLCR register";
                    break;
                case 2:
                    ret = "TARGETID register ";
                    break;
                case 3:
                    ret = "DLPIDR register";
                    break;
                case 4:
                    ret = "EVENTSTAT register";
                    break;
                default:
                    break;
            }
            break;
        case 0x8:
            ret = ReadNWrite ? "RESEND register" : "SELECT register";
            break;

        case 0xC:
            ret = ReadNWrite ? "RDBUFF register" : "TARGETSEL register";
            break;

        default:
            ret = "Invalid register received";
            break;



    }

    return ret;

}

std::string SwdEudDevice::GetApRegInfo(uint32_t regidx, uint32_t ReadNWrite) {
    std::string ret = "Undefined";

    switch (regidx)
    {
        case 0x00:
            ret = "CSW Register";
            break;
        case 0x04:
            ret = "TAR Low Register";
            break;
        case 0x08:
            ret = "TAR High Register";
            break;
        case 0x0C:
            ret = "Data RW Register";
            break;
        case 0x10:
        case 0x14:
        case 0x18:
        case 0x1C:
            ret = "Banked Data Register ";
            break;
        case 0x20:
            ret = "Memory Barrier Register";
            break;
        case 0xF0:
            ret = "Debug Base Address High Register";
            break;
        case 0xF4:
            ret = "Configuration Register";
            break;
        case 0xF8:
            ret = "Debug Base Address Low Register";
            break;
        case 0xFC:
            ret = "Identification Register";
            break;

        default:
            break;
    }

    return ret;
}

std::string SwdEudDevice::GetRegInfo(uint32_t access_port, uint32_t regidx, uint32_t ReadNWrite) {
    return (access_port ? GetApRegInfo(regidx, ReadNWrite) : GetDpRegInfo(regidx, ReadNWrite));

}

void SwdEudDevice::SetDpApSelect(uint32_t access_port, uint32_t regidx, uint32_t ReadNWrite, uint32_t val) {

    /* There is a write happening to DP register 0x8 which controls selection of the DP / AP. Save that value 
       i.e. access_port = 0, readnwrite = 0, regidx = 8 */

    if ((!access_port) && (!ReadNWrite) && (regidx == 8)) {
        dp_select_reg_hist_ = { val };
    }

}