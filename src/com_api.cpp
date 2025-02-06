/*************************************************************************
*
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   com_api.cpp
*
* Description:                                                              
*   CPP source file for COM EUD Peripheral's public API's
*
***************************************************************************/

#define EUD_DEVICE_DECL 1

#include "eud.h"
#include "com_eud.h"

#define EUD_COM_EXEC_ENV_BASE 0x80

#define EUD_COM_EXEC_ENV_AOP      + EUD_COM_EXEC_ENV_BASE + 0x0
#define EUD_COM_EXEC_ENV_APPS     + EUD_COM_EXEC_ENV_BASE + 0x1
#define EUD_COM_EXEC_ENV_CDSP     + EUD_COM_EXEC_ENV_BASE + 0x2
#define EUD_COM_EXEC_ENV_RESERVED + EUD_COM_EXEC_ENV_BASE + 0x3
#define EUD_COM_EXEC_ENV_MPSS     + EUD_COM_EXEC_ENV_BASE + 0x4
#define EUD_COM_EXEC_ENV_ADSP     + EUD_COM_EXEC_ENV_BASE + 0x5
#define EUD_COM_EXEC_ENV_SLPI     + EUD_COM_EXEC_ENV_BASE + 0x6
#define EUD_COM_EXEC_ENV_SP       + EUD_COM_EXEC_ENV_BASE + 0x8

extern "C" ComEudDevice* eud_initialize_device_com(uint32_t deviceID, uint32_t options, EUD_ERR_t * errcode);

/*
Send
assemble message:
|-   0  -|-  1   -|---2<15-----|
|-SendID-|-MsgLen-|-----MSG----|


get API call with data:
    data is:
        sendID
        msg length (can be up to.. 1024bytes?)
        msg data
    for (msgdata/14):
        send 14 bytes of data
    send remining  bytes (modulo 14)


receive

|-   0  -|-  1   -|---2<15-----|
|-RcvID -|-MsgLen-|-----MSG----|



Functionality:
    API gives  Exec Env ID. All things to be filtered based on that.

    set send/receive timeout (1sec?)
    set internal var: poll_frequency
    
    while(input!='ctl-C'):
        poll host for data-to-send
            if data available:
                for (msgdata/14):
                    send(Exec_Env_ID,0x0E,<14bytesdata>);
                send(Exec_Env_ID,mod(msgdata,14),remainingdata);

        poll device for data
        if you get data, 
            byte 0 is Exec Env ID.
            byte 1 is how many bytes data
            remaining bytes are data. Add as many bytes indicated by numbytes to buffer.
        wait(poll_frequency)
        

    ////
    ////    Simultaneous in/out functionality
    ////
    while(input!='ctl-C'):
        if data_out_ready==1:
            
            
        if data_in_ready==1:
            usb_result=read_usb(0x0E)
            if usb_result == USB_ERR_TMOUT:
                data_in_ready=0;
        
        if (!data_in_ready)&&(!data_out_ready):
            wait(poll_frequency)

        if (!data_in_ready):
            usb_result=read_usb(0x0E)
            if  usb_result == EUD_SUCCESS:
                data_in_ready=1
        if (!data_out_ready):
            if (check_if_data_out_ready()):
                data_out_ready=1



--------------------------------------
T32 test operation:
    Send:
        enable the interrupt enable mask? (int0_en_mask)
        poll on interrupt mask bit
        enable Exec_Env for (apps 0x1<<1 to com_tx_id)
        read that reg to ensure it's on.

        set timeouts from EUD host side.

        update com_tx_id with 0x1<<1
            PERIPH_SS_EUD_COM_TX_ID : 0x088E0000
        set 0x0E  to com_tx_len register
            PERIPH_SS_EUD_COM_TX_LEN : 0x088E0004
        write data, one byte at a time to com_tx_dat.
            PERIPH_SS_EUD_COM_TX_DAT : 0x088E0008

        read data from host  side.

    receive:
        set timeouts from EUD host side.
        poll  the eud_int_mx
            PERIPH_SS_EUD_INT0_EN_MASK : 0x088E0020
        read  com_rx_id (should be apps, or whatever eud host s/w set it  to)
            HWIO_PERIPH_SS_EUD_COM_RX_ID_ADDR: 0x088E000C
        read com_rx_len (should be 0x0E or whatever eud host s/w set length to)
            HWIO_PERIPH_SS_EUD_COM_RX_LEN_ADDR : 0x088E0010
        read com_rx_dat 'com_rx_len' number of times.
            PERIPH_SS_EUD_COM_RX_DATA : 0x088E0014



    


-----------------------------------
*/
//===---------------------------------------------------------------------===//
//
// API Access
//
//===---------------------------------------------------------------------===//


#define EXEC_ENV_ID_ALLOWED_MASK 0x80
EXPORT EUD_ERR_t test_function(uint32_t deviceID, uint8_t ExecEnvID){
    if (!(ExecEnvID & EXEC_ENV_ID_ALLOWED_MASK)){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }

    //API gives  Exec Env ID.All things to be filtered based on that.
    EUD_ERR_t errcode = 0;
    ComEudDevice* com_handle_p = eud_initialize_device_com(deviceID, 0, &errcode);
    if (com_handle_p == NULL){
        return errcode;
    }


    //set send / receive timeout(1sec ? )
    uint8_t* tx_timeout = new uint8_t[4];
    *tx_timeout = 0xFF;
    *(tx_timeout+1) = 0xFF;
    *(tx_timeout + 2) = 0x0;
    *(tx_timeout + 3) = 0x0;
    EUD_ERR_t err = 0;
    if ((err = com_handle_p->WriteCommand(COM_CMD_TX_TMOUT, tx_timeout)) != 0) 
        return eud_set_last_error(err);

    uint8_t* rx_timeout = new uint8_t[4];
    rx_timeout[0] = 0xFF;
    rx_timeout[1] = 0xFF;
    rx_timeout[2] = 0x0;
    rx_timeout[3] = 0x0;
    if ((err = com_handle_p->WriteCommand(COM_CMD_RX_TMOUT, rx_timeout)) != 0)
         return eud_set_last_error(err);
    

    //set internal var: poll_frequency


    uint8_t* sendbuf = new uint8_t[3];
    *sendbuf = ExecEnvID;
    *(sendbuf+1) = 1;
    *(sendbuf+2) = 13; //Carriage return
    uint32_t poll_value = 100; //100 milliseconds

    uint32_t counter;
    //uint8_t* in_data = new uint8_t[16];
    BOOL data_ready = FALSE;
    //uint32_t len = 0;
    for (counter = 0; counter < 1000; counter++){

        /* FIXME
        len = 10;
        //buffer = assemble_send_message(EE_ID, msglen, defaultmsg);
        com_device_handle->usb_buffer_out_[0] = ExecEnvID; //env ID
        com_device_handle->usb_buffer_out_[1] = len-2; //length of message (1)
        com_device_handle->usb_buffer_out_[2] = 13; //carriage return
        com_device_handle->usb_buffer_out_[3] = 14; //carriage return
        com_device_handle->usb_buffer_out_[4] = 15; //carriage return
        com_device_handle->usb_buffer_out_[5] = 16; //carriage return
        com_device_handle->usb_buffer_out_[6] = 17; //carriage return
        com_device_handle->usb_buffer_out_[7] = 18; //carriage return
        com_device_handle->usb_buffer_out_[8] = 19; //carriage return
        com_device_handle->usb_buffer_out_[9] = 20; //carriage return
        
        //memcpy(com_device_handle->usb_buffer_out_,
        //      sendbuf,
        //      3
        // );

        if (result = com_device_handle->usb_write(10)) return eud_set_last_error(result);

        result=com_device_handle->usb_read(16, in_data);
        if (result == EUD_USB_ERROR_READ_TIMEOUT){
            data_ready = FALSE;
        }
        else if (result == EUD_SUCCESS){
            data_ready = TRUE;
        }
        //Error condition
        else{
            return result;
        }

        */
        //Sleep for poll_value if no data is ready.
        if (data_ready == FALSE) Sleep(poll_value);
        
    }

    return EUD_SUCCESS;
}
