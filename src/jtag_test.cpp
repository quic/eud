/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   jtag_test.cpp
*
* Description:                                                              
*   CPP source file for EUD JTAG ID read from device
*
***************************************************************************/
#include "jtag_eud.h"
#include "jtag_api.h"

EXPORT EUD_ERR_t jtag_get_jtag_id(JtagEudDevice* jtg_handle_p, uint32_t* jtag_id){

    if (jtag_id == NULL) return eud_set_last_error(EUD_ERR_BAD_PARAMETER);

    //EUD_ERR_t err = handler_validity_checker(DEVICETYPE_EUD_JTG, jtg_handle_p);
    //if (err != EUD_SUCCESS){
    //  return err;
    //}
    //see if we can get jtag ID
    //send 5 1's
    //get to shift DR
    //DAP: IR is 4, DR is 32. instruction 1110 is IDCODE. DR scan width is then 35
    //  ID is in DR
    //msm tap DR is 1, ACC tap DR is 1

    //so.. shift 11111, get to shift DR, shift in 34 bits and read output.
    /*
    char* payload = new char[5];
    memset(payload, 0, 4);
    payload[0] = CMD_JTAG_TMS;
    payload[4] = 0x1f;
    err = JTAG_TMS(jtg_handle_p, payload);


    delete payload;
    */
    //uint8_t* tdo_data = new uint8_t[4];
    //uint32_t* tdo_size = new uint32_t[1];
    /*
    EudShiftTms(jtg_handle_p, "1 1 1 1 1 1 0");
    EudShiftTms(jtg_handle_p, "1 0 0");
    EudShiftTdi(jtg_handle_p, "1111111111111111111111111111111111"); //0B FF FF FF FF 09 03 00   01 00 01
    // 0B FF FF FF FF 09 00 03   00 01 01, got FF FF FF FF FF FF 00 00
    EudReadTdo(tdo_data,tdo_size);
    */
    /*
    EudShiftTms(jtg_handle_p, "1 1 1 1 1 1 0"); //reset and get to run test idle
    EudShiftTms(jtg_handle_p, "1 1 0 0"); //get it Shift IR
    //EudShiftTdi(jtg_handle_p, "11111111111111111110"); //DAPIRPOST is 16. 1110 is JTAGID from dap. it will exit IR after shifting in bits
    EudShiftTdi(jtg_handle_p, "01111111111111111111"); //DAPIRPOST is 16. 1110 is JTAGID from dap. it will exit IR after shifting in bits
    EudShiftTms(jtg_handle_p, "1 1 0 0"); //get it Shift DR
    EudShiftTdi(jtg_handle_p, "1111111111111111111111111111111111"); //DAPDRPOST is 2, device ID scan width is 32 => 34//0B FF FF FF FF 09 03 00   01 00 01
    EudShiftTms(jtg_handle_p, "1 1 1 1 1 1 0"); //reset and get to run test idle

    EudReadTdo(tdo_data, tdo_size);
    */
    //EudShiftTms(jtg_handle_p, "1 1 1 1 1 1 0"); //reset and get to run test idle
    //EudShiftTms(jtg_handle_p, "1 1 0 0"); //get it Shift IR
    /*
    char* pay = { "\xBF\x01\x0A\x00" };
    JTAG_TMS(jtg_handle_p, pay); //Note: can't use this as it's deprecated



    */
#define MSBFIRST 1 
#ifdef MSBFIRST
#define PYLD_IDX    0
#define INSTR_IDX_0 1
#define INSTR_IDX_1 2
#define CLK_IDX_0   3
#define CLK_IDX_1   4
#else
#define PYLD_IDX    0
#define INSTR_IDX_0 4
#define INSTR_IDX_1 3
#define CLK_IDX_0   2
#define CLK_IDX_1   1
#endif

    //uint8_t* scratchpad = new uint8_t[5];

    //Reset TMS and get it to IR Shift
    //Use  TMS reset stuff?

    //scratchpad[PYLD_IDX] = CMD_JTAG_BITBANG;
    //scratchpad[INSTR_IDX_0] = (1<<3) + (1 << 4) + (1 << 5) + /*(1 << 6) +*/ (1 << 2);
    //scratchpad[INSTR_IDX_1] = 0;
    //scratchpad[CLK_IDX_0] = 0x00;
    //scratchpad[CLK_IDX_1] = 0x00;
    //jtg_handle_p->handle->usb_write(scratchpad, 5);
    //
    ////Reset TMS and get it to IR Shift
    //scratchpad[PYLD_IDX] = CMD_JTAG_BITBANG;
    //scratchpad[INSTR_IDX_0] = (1 << 4) + (1 << 5) + (1 << 6) + (1 << 2);
    //scratchpad[INSTR_IDX_1] = 0;
    //scratchpad[CLK_IDX_0] = 0x00;
    //scratchpad[CLK_IDX_1] = 0x00;
    //jtg_handle_p->handle->usb_write(scratchpad, 5);

#ifdef FPGA
    ////////////////////////////////////////////////////
    /// This sequence for 8996 on FPGA XILINX board.  //
    ////////////////////////////////////////////////////

    /*
    EudShiftTms(jtg_handle_p, "1 1 1 1 1 1 0"); //reset and get to run test idle
    EudShiftTms(jtg_handle_p, "1 1 0 0"); //get it Shift IR
    //EudShiftTdi(jtg_handle_p, "11111111111111111110"); //DAPIRPOST is 16. 1110 is JTAGID from dap. it will exit IR after shifting in bits
    EudShiftTdi(jtg_handle_p, "01111111111111111111"); //DAPIRPOST is 16. 1110 is JTAGID from dap. it will exit IR after shifting in bits
    EudShiftTms(jtg_handle_p, "1 1 0 0"); //get it Shift DR
    EudShiftTdi(jtg_handle_p, "1111111111111111111111111111111111"); //DAPDRPOST is 2, device ID scan width is 32 => 34//0B FF FF FF FF 09 03 00   01 00 01
    EudShiftTms(jtg_handle_p, "1 1 1 1 1 1 0"); //reset and get to run test idle
    EudReadTdo(tdo_data, tdo_size);
    */


    //jtag.shifttms 0 1 1 0 0 1 1 1 1 1 1
    //reverse it, since LSB first here. Jtag.shifttms is MSB first.
    //jtag.shifttms 
    scratchpad[PYLD_IDX] = CMD_JTAG_TMS;
    scratchpad[INSTR_IDX_0] = 0x3F;
    scratchpad[INSTR_IDX_1] = 0x03;
    //scratchpad[INSTR_IDX_1] = 0xC0;
    scratchpad[CLK_IDX_0] = 0x0B-1;
    scratchpad[CLK_IDX_1] = 0x00;
    jtg_handle_p->usb_write(scratchpad, 5);


    //MSM TAP IR is 16 bits, DAP TAP IR is 4. shift in 1110 to DAP tap.
    //jtag.shiftreg 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 
    //shift in 1111 1111 1111 1111 1110   ///<to iR
    scratchpad[PYLD_IDX] = JTG_CMD_NBIT_TOSS;
    scratchpad[INSTR_IDX_0] = 0xFE;
    scratchpad[INSTR_IDX_1] = 0xFF;
    scratchpad[CLK_IDX_0] = 0x14-1;
    scratchpad[CLK_IDX_1] = 0x00;
    jtg_handle_p->usb_write(scratchpad, 5);
    
    //Get to DR shift
    //jtag.shiftreg 1 0 1 0 0 
    //so data is 0 0 1 1 0  1 1
    scratchpad[PYLD_IDX] = CMD_JTAG_TMS;
    scratchpad[INSTR_IDX_0] = 0x0B;
    scratchpad[INSTR_IDX_1] = 0x00;
    scratchpad[CLK_IDX_0] = 0x06-1;
    scratchpad[CLK_IDX_1] = 0x00;
    jtg_handle_p->usb_write(scratchpad, 5);

    //MSM TAP DR is in bypass (1 wide), DAP TAP DR is 32 bits. shift out 33 bits.
    //shift in 1111 1111 1111 110   ///<to iR
    scratchpad[PYLD_IDX] = JTG_CMD_NBIT_KEEP;
    scratchpad[INSTR_IDX_0] = 0xFF;
    scratchpad[INSTR_IDX_1] = 0xFF;
    scratchpad[CLK_IDX_0] = 0x22-1;
    scratchpad[CLK_IDX_1] = 0x00;
    jtg_handle_p->usb_write(scratchpad, 5);
#else

    //jtag.shifttms 0 1 1 0 0 1 1 1 1 1 1
    //reverse it, since LSB first here. Jtag.shifttms is MSB first.
    //jtag.shifttms 

    //scratchpad[PYLD_IDX] = CMD_JTAG_TMS;
    //scratchpad[INSTR_IDX_0] = 0x3F;
    //scratchpad[INSTR_IDX_1] = 0x03;
    ////scratchpad[INSTR_IDX_1] = 0xC0;
    //scratchpad[CLK_IDX_0] = 0x0B-1;
    //scratchpad[CLK_IDX_1] = 0x00;
    //jtg_handle_p->handle->usb_write(scratchpad, 5);
    
    uint32_t payload;

    payload = (      0x3f +
                ( 0x03 << 8) +
                ((0x0B - 1) << 16) +
                ( 0x00 << 24)
        );

#if defined ( EUD_WIN_ENV )
    jtag_tms(jtg_handle_p, payload);
#endif
    //MSM TAP IR is 11 bits, DAP TAP IR is 4. shift in 1110 to DAP tap.
    //jtag.shiftreg 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1
    //shift in 1111 1111 111 1110   ///<to iR

    //scratchpad[PYLD_IDX] = JTG_CMD_NBIT_TOSS;
    //scratchpad[INSTR_IDX_0] = 0xFE;
    //scratchpad[INSTR_IDX_1] = 0x7F;
    //scratchpad[CLK_IDX_0] = 0x0F-1;
    //scratchpad[CLK_IDX_1] = 0x00;
    //jtg_handle_p->handle->usb_write(scratchpad, 5);
    

    payload = (  0xFE + 
                (0x7F << 8) + 
                ((0x0F - 1) << 16) + 
                (0x00 << 24)
                );
#if defined ( EUD_WIN_ENV )
    jtag_nbit_toss(jtg_handle_p, payload);
#endif

    //Get to DR shift
    //jtag.shiftreg 1 0 1 0 0 
    //so data is 0 0 1 0 1 
    //scratchpad[PYLD_IDX] = CMD_JTAG_TMS;
    //scratchpad[INSTR_IDX_0] = 0x05;
    //scratchpad[INSTR_IDX_1] = 0x00;
    //scratchpad[CLK_IDX_0] = 0x05-1;
    //scratchpad[CLK_IDX_1] = 0x00;
    //jtg_handle_p->handle->usb_write(scratchpad, 5);
    payload = ( 0x05 +
                (0x00 << 8) +
                ((0x05 - 1) << 16) +
                (0x00 << 24)
        );

#if defined ( EUD_WIN_ENV )
    jtag_tms(jtg_handle_p, payload);
#endif

    //MSM TAP DR is in bypass (1 wide), DAP TAP DR is 32 bits. shift out 33 bits.
    //shift in 1111 1111 1111 110   ///<to iR
    //scratchpad[PYLD_IDX] = JTG_CMD_NBIT_KEEP;
    //scratchpad[INSTR_IDX_0] = 0xFF;
    //scratchpad[INSTR_IDX_1] = 0xFF;
    //scratchpad[CLK_IDX_0] = 0x21-1;
    //scratchpad[CLK_IDX_1] = 0x00;
    //jtg_handle_p->handle->usb_write(scratchpad, 5);
    
    uint32_t* response = new uint32_t;
    payload = (0xFE +
        (0x7F << 8) +
        ((0x0F - 1) << 16) +
        (0x00 << 24)
        );
        
#if defined ( EUD_WIN_ENV )
    jtag_nbit_keep(jtg_handle_p, payload,response);
#endif

#endif
    //uint8_t* data = new uint8_t[8];
    //memset(data, 0, 4);
    //scratchpad[PYLD_IDX] = CMD_JTAG_FLUSH;
    //jtg_handle_p->handle->usb_write(scratchpad, 1);
    //jtg_handle_p->handle->usb_read(4, data);
    //
    //
    //
    return EUD_SUCCESS;

}

