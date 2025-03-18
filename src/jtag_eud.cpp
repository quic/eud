/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   jtag_eud.cpp
*
* Description:                                                              
*   CPP source file for EUD Jtag Peripheral APIs
*
***************************************************************************/

#include "eud.h"
#include "jtag_eud.h"

#include <algorithm>
#include <string>
#include <math.h>
#include <stdlib.h>

extern "C" {
#include <assert.h>
}
#include <iostream>
#include <ctime>
#include <sstream>
#include <fstream>



jtag_state_t
JtagStateMachine::CalcNextState(jtag_state_t state, uint32_t tms)
{
    jtag_state_t next_state;

    if (tms == 0)
    {
        switch (state) {
        case JTAG_STATE_RESET:
        case JTAG_STATE_IDLE:
        case JTAG_STATE_DRUPDATE:
        case JTAG_STATE_IRUPDATE:
            next_state = JTAG_STATE_IDLE;
            break;
        case JTAG_STATE_DRSELECT:
            next_state = JTAG_STATE_DRCAPTURE;
            break;
        case JTAG_STATE_DRCAPTURE:
        case JTAG_STATE_DRSHIFT:
        case JTAG_STATE_DREXIT2:
            next_state = JTAG_STATE_DRSHIFT;
            break;
        case JTAG_STATE_DREXIT1:
        case JTAG_STATE_DRPAUSE:
            next_state = JTAG_STATE_DRPAUSE;
            break;
        case JTAG_STATE_IRSELECT:
            next_state = JTAG_STATE_IRCAPTURE;
            break;
        case JTAG_STATE_IRCAPTURE:
        case JTAG_STATE_IRSHIFT:
        case JTAG_STATE_IREXIT2:
            next_state = JTAG_STATE_IRSHIFT;
            break;
        case JTAG_STATE_IREXIT1:
        case JTAG_STATE_IRPAUSE:
            next_state = JTAG_STATE_IRPAUSE;
            break;
        default:
            exit(1);
            break;
        }
    }
    else
    {
        switch (state) {
        case JTAG_STATE_RESET:
            next_state = state;
            break;
        case JTAG_STATE_IDLE:
        case JTAG_STATE_DRUPDATE:
        case JTAG_STATE_IRUPDATE:
            next_state = JTAG_STATE_DRSELECT;
            break;
        case JTAG_STATE_DRSELECT:
            next_state = JTAG_STATE_IRSELECT;
            break;
        case JTAG_STATE_DRCAPTURE:
        case JTAG_STATE_DRSHIFT:
            next_state = JTAG_STATE_DREXIT1;
            break;
        case JTAG_STATE_DREXIT1:
        case JTAG_STATE_DREXIT2:
            next_state = JTAG_STATE_DRUPDATE;
            break;
        case JTAG_STATE_DRPAUSE:
            next_state = JTAG_STATE_DREXIT2;
            break;
        case JTAG_STATE_IRSELECT:
            next_state = JTAG_STATE_RESET;
            break;
        case JTAG_STATE_IRCAPTURE:
        case JTAG_STATE_IRSHIFT:
            next_state = JTAG_STATE_IREXIT1;
            break;
        case JTAG_STATE_IREXIT1:
        case JTAG_STATE_IREXIT2:
            next_state = JTAG_STATE_IRUPDATE;
            break;
        case JTAG_STATE_IRPAUSE:
            next_state = JTAG_STATE_IREXIT2;
            break;
        default:
            exit(1);
            break;
        }
    }


    return next_state;
}


EUD_ERR_t JtagStateMachine::LogJtagError(EUD_ERR_t errorstate){
    jtag_error_state_ = errorstate;
    return errorstate;
}


EUD_ERR_t JtagStateMachine::GetJtagErrorState(void){
    return jtag_error_state_;
}


EUD_ERR_t JtagStateMachine::ResetJtagErrorState(){
    jtag_error_state_ = EUD_SUCCESS;
    return EUD_SUCCESS;
}

//===---------------------------------------------------------------------===//
//
// JtagEudDevice class methods
//
//===---------------------------------------------------------------------===//
int32_t JtagEudDevice::Init(){

    //read_time_elapsed = 0;
    //write_time_elapsed = 0;
    num_of_reads_ = 0;
    num_of_writes_ = 0;

    //usb_bytes_pending_in = 0;
    usb_num_bytes_pending_in_ = 0;
    usb_num_bytes_pending_out_ = 0;
    //usb_bytes_pending_out = 0;

    memset(usb_buffer_in_, 0x0, JTAG_IN_BUFFER_SIZE);
    memset(usb_buffer_out_, 0x0, JTAG_OUT_BUFFER_SIZE);
    usb_device_handle_ = NULL;

    //Start in IMMEDIATEWRITEMODE for JTAG. Other modes not yet  available.
    mode_ = IMMEDIATEWRITEMODE; 

    

    periph_payload_size_table_[CMD_JTAG_NOP] = JTG_PCKT_SZ_SEND_NOP;
    periph_payload_size_table_[CMD_JTAG_FLUSH] = JTG_PCKT_SZ_SEND_FLUSH;
    periph_payload_size_table_[CMD_JTAG_FREQ_WR] = JTG_PCKT_SZ_SEND_FREQ_WR;
    periph_payload_size_table_[CMD_JTAG_DELAY] = JTG_PCKT_SZ_SEND_DELAY;
    periph_payload_size_table_[CMD_JTAG_BITBANG] = JTG_PCKT_SZ_SEND_BITBANG;
    periph_payload_size_table_[CMD_JTAG_TMS] = JTG_PCKT_SZ_SEND_TMS;
    periph_payload_size_table_[CMD_JTAG_NBIT_TOSS] = JTG_PCKT_SZ_SEND_NBIT_TOSS;
    periph_payload_size_table_[CMD_JTAG_NBIT_KEEP] = JTG_PCKT_SZ_SEND_NBIT_KEEP;
    periph_payload_size_table_[CMD_JTAG_NBIT_END_TOSS] = JTG_PCKT_SZ_SEND_NBIT_END_TOSS;
    periph_payload_size_table_[CMD_JTAG_NBIT_END_KEEP] = JTG_PCKT_SZ_SEND_NBIT_END_KEEP;
    periph_payload_size_table_[CMD_JTAG_32BIT_TOSS] = JTG_PCKT_SZ_SEND_32BIT_TOSS;
    periph_payload_size_table_[CMD_JTAG_32BIT_KEEP] = JTG_PCKT_SZ_SEND_32BIT_KEEP;
    periph_payload_size_table_[CMD_JTAG_32BIT_END_TOSS] = JTG_PCKT_SZ_SEND_32BIT_END_KEEP;
    periph_payload_size_table_[CMD_JTAG_32BIT_END_KEEP] = JTG_PCKT_SZ_SEND_32BIT_END_TOSS;
    periph_payload_size_table_[CMD_JTAG_PERIPH_RST] = JTG_PCKT_SZ_SEND_RST;
    periph_payload_size_table_[CMD_JTAG_FREQ_RD] = JTG_PCKT_SZ_SEND_FREQ_RD;

    periph_response_size_table_[CMD_JTAG_NOP] = JTG_PCKT_SZ_RECV_NOP;
    periph_response_size_table_[CMD_JTAG_FLUSH] = JTG_PCKT_SZ_RECV_FLUSH;
    periph_response_size_table_[CMD_JTAG_FREQ_WR] = JTG_PCKT_SZ_RECV_FREQ_WR;
    periph_response_size_table_[CMD_JTAG_DELAY] = JTG_PCKT_SZ_RECV_DELAY;
    periph_response_size_table_[CMD_JTAG_BITBANG] = JTG_PCKT_SZ_RECV_BITBANG;
    periph_response_size_table_[CMD_JTAG_TMS] = JTG_PCKT_SZ_RECV_TMS;
    periph_response_size_table_[CMD_JTAG_NBIT_TOSS] = JTG_PCKT_SZ_RECV_NBIT_TOSS;
    periph_response_size_table_[CMD_JTAG_NBIT_KEEP] = JTG_PCKT_SZ_RECV_NBIT_KEEP;
    periph_response_size_table_[CMD_JTAG_NBIT_END_TOSS] = JTG_PCKT_SZ_RECV_NBIT_END_TOSS;
    periph_response_size_table_[CMD_JTAG_NBIT_END_KEEP] = JTG_PCKT_SZ_RECV_NBIT_END_KEEP;
    periph_response_size_table_[CMD_JTAG_32BIT_TOSS] = JTG_PCKT_SZ_RECV_32BIT_TOSS;
    periph_response_size_table_[CMD_JTAG_32BIT_KEEP] = JTG_PCKT_SZ_RECV_32BIT_KEEP;
    periph_response_size_table_[CMD_JTAG_32BIT_END_TOSS] = JTG_PCKT_SZ_RECV_32BIT_END_TOSS;
    periph_response_size_table_[CMD_JTAG_32BIT_END_KEEP] = JTG_PCKT_SZ_RECV_32BIT_END_KEEP;
    periph_response_size_table_[CMD_JTAG_PERIPH_RST] = JTG_PCKT_SZ_RECV_RST;
    periph_response_size_table_[CMD_JTAG_FREQ_RD] = JTG_PCKT_SZ_RECV_FREQ_RD;
    
    periph_max_opcode_value_ = JTG_NUM_OPCODES;



    jtag_check_opcode_queuable_table_[JTG_NUM_OPCODES] = JTG_QUEUABLEFLAG_NOP;
    jtag_check_opcode_queuable_table_[CMD_JTAG_FLUSH] = JTG_QUEUABLEFLAG_FLUSH;
    jtag_check_opcode_queuable_table_[CMD_JTAG_FREQ_WR] = JTG_QUEUABLEFLAG_FREQ_WR;
    jtag_check_opcode_queuable_table_[CMD_JTAG_DELAY] = JTG_QUEUABLEFLAG_DELAY;
    jtag_check_opcode_queuable_table_[CMD_JTAG_BITBANG] = JTG_QUEUABLEFLAG_BITBANG;
    jtag_check_opcode_queuable_table_[CMD_JTAG_TMS] = JTG_QUEUABLEFLAG_TMS;
    jtag_check_opcode_queuable_table_[CMD_JTAG_NBIT_TOSS] = JTG_QUEUABLEFLAG_NBIT_TOSS;
    jtag_check_opcode_queuable_table_[CMD_JTAG_NBIT_KEEP] = JTG_QUEUABLEFLAG_NBIT_KEEP;
    jtag_check_opcode_queuable_table_[CMD_JTAG_NBIT_END_TOSS] = JTG_QUEUABLEFLAG_NBIT_END_TOSS;
    jtag_check_opcode_queuable_table_[CMD_JTAG_NBIT_END_KEEP] = JTG_QUEUABLEFLAG_NBIT_END_KEEP;
    jtag_check_opcode_queuable_table_[CMD_JTAG_32BIT_TOSS] = JTG_QUEUABLEFLAG_32BIT_TOSS;
    jtag_check_opcode_queuable_table_[CMD_JTAG_32BIT_KEEP] = JTG_QUEUABLEFLAG_32BIT_KEEP;
    jtag_check_opcode_queuable_table_[CMD_JTAG_32BIT_END_TOSS] = JTG_QUEUABLEFLAG_32BIT_END_TOSS;
    jtag_check_opcode_queuable_table_[CMD_JTAG_32BIT_END_KEEP] = JTG_QUEUABLEFLAG_32BIT_END_KEEP;
    jtag_check_opcode_queuable_table_[CMD_JTAG_PERIPH_RST] = JTG_QUEUABLEFLAG_RST;
    jtag_check_opcode_queuable_table_[CMD_JTAG_FREQ_RD] = JTG_QUEUABLEFLAG_FREQ_RD;

    
    periph_endian_option_table_[CMD_JTAG_NOP] = CMD_JTAG_ENDIANOPT_NOP;
    periph_endian_option_table_[CMD_JTAG_FLUSH] = CMD_JTAG_ENDIANOPT_FLUSH;
    periph_endian_option_table_[CMD_JTAG_FREQ_WR] = CMD_JTAG_ENDIANOPT_FREQ_WR;
    periph_endian_option_table_[CMD_JTAG_DELAY] = CMD_JTAG_ENDIANOPT_DELAY;
    periph_endian_option_table_[CMD_JTAG_BITBANG] = CMD_JTAG_ENDIANOPT_BITBANG;
    periph_endian_option_table_[CMD_JTAG_TMS] = CMD_JTAG_ENDIANOPT_TMS;
    periph_endian_option_table_[CMD_JTAG_NBIT_TOSS] = CMD_JTAG_ENDIANOPT_NBIT_TOSS;
    periph_endian_option_table_[CMD_JTAG_NBIT_KEEP] = CMD_JTAG_ENDIANOPT_NBIT_KEEP;
    periph_endian_option_table_[CMD_JTAG_NBIT_END_TOSS] = CMD_JTAG_ENDIANOPT_NBIT_END_TOSS;
    periph_endian_option_table_[CMD_JTAG_NBIT_END_KEEP] = CMD_JTAG_ENDIANOPT_NBIT_END_KEEP;
    periph_endian_option_table_[CMD_JTAG_32BIT_TOSS] = CMD_JTAG_ENDIANOPT_32BIT_TOSS;
    periph_endian_option_table_[CMD_JTAG_32BIT_KEEP] = CMD_JTAG_ENDIANOPT_32BIT_KEEP;
    periph_endian_option_table_[CMD_JTAG_32BIT_END_TOSS] = CMD_JTAG_ENDIANOPT_32BIT_END_TOSS;
    periph_endian_option_table_[CMD_JTAG_32BIT_END_KEEP] = CMD_JTAG_ENDIANOPT_32BIT_END_KEEP;
    periph_endian_option_table_[CMD_JTAG_PERIPH_RST] = CMD_JTAG_ENDIANOPT_PERIPH_RST;
    periph_endian_option_table_[CMD_JTAG_FREQ_RD] = CMD_JTAG_ENDIANOPT_FREQ_RD;
    
    return 0;
}


EUD_ERR_t
JtagEudDevice::ProcessBuffers(){
    return eud_set_last_error(EUD_ERR_FUNCTION_NOT_AVAILABLE);
}
JtagEudDevice::JtagEudDevice() {
    Init();
    

    //Max 64 bits for tdo_buffer
    //tdo_buffer_ = new uint8_t[TDO_BUFFER_MAX];
    memset(tdo_buffer_, 0, TDO_DATA_IN_MAX);
    tdo_index_ = 0;
    tdo_data_available_ = 0;
    /* //done in eud.cpp
    read_time_elapsed = 0;
    write_time_elapsed = 0;
    num_of_writes = 0;
    num_of_reads = 0;
    */
    //usb_device *usb_device_handle = EMPTY_USB_HANDLE;
    usb_device_handle_init_flag_ = false;
    device_type_ = DEVICETYPE_EUD_JTG;
    DvcMgr_DeviceName = std::string(EUD_JTG_DEVMGR_NAME);

    //JtagStateMachine jtag_state_ = *(new JtagStateMachine);
    ResetCounters();

}

EUD_ERR_t
JtagEudDevice::UsbWriteRead(uint32_t out_length, uint8_t* out_buffer, uint32_t in_length, uint8_t* in_buffer){
    USB_ERR_t err;
    unsigned long* errcode = new unsigned long;

    //FIXME - use temporary buffer in case  passed buffer is not large enough
    //Figure out how to optimize
    uint8_t* temp_buffer = new uint8_t[1024];

    if ((err = usb_device_handle_->WriteToDevice(out_buffer, out_length, errcode))!=0){
        delete[] temp_buffer;
        return err;
    }

    if (in_length > 0){
        if ((err = usb_device_handle_->ReadFromDevice((PVOID)temp_buffer, in_length, errcode))!=0){
            delete[] temp_buffer;
            return err;
        }
    }
    memcpy(in_buffer, temp_buffer, usb_device_handle_->bytes_read_);

    delete[] temp_buffer;

    return EUD_SUCCESS;
}

EUD_ERR_t JtagEudDevice::ResetJtagStateMachine(void){
    ResetCounters();
    return jtag_state_.ResetJtagErrorState();

}
EUD_ERR_t JtagEudDevice::ClearJtagStateMachineErrorState(void){
    //ResetCounters();
    return jtag_state_.ResetJtagErrorState();
}

EUD_ERR_t JtagEudDevice::GetJtagStateMachineErrorState(void){
    return jtag_state_.GetJtagErrorState();
}


EUD_ERR_t JtagEudDevice::EudSetConfig(uint32_t devicenumber){

    JtagEudDevice::config_->deviceType = devicenumber;
    return EUD_SUCCESS;
}


//////////////////////////////////////////////////////////////
//
// JTag shift commands - buffered write
//
////////////////////////////////////////////////////////////

EUD_ERR_t
JtagEudDevice::EudShiftTms(const char* shiftvalue){

    //If jtag is in error state, return
    EUD_ERR_t err;
    if ((err = jtag_state_.GetJtagErrorState())!=0){ return err; }

    //move to an object
    EudJtagData* jtag_data = new EudJtagData;

    uint16_t* raw_bit_stream = new uint16_t[TMS_DATA_IN_MAX];
    memset(raw_bit_stream, 0, TMS_DATA_IN_MAX);
    //CryptStringToBinaryA
    uint32_t i;
    uint32_t iterator = 0;
    uint32_t length = (uint32_t)strlen(shiftvalue);
    //uint32_t data_index = 0;
    uint32_t packet_index = 0;
    uint32_t bitcount = 0;

    for (i = 0; i < length; i++){
        if (!((*(shiftvalue + i) == '1') || (*(shiftvalue + i) == '0'))) continue;

        if (*(shiftvalue + i) == '1')
            raw_bit_stream[packet_index] |= 1 << iterator;

        if (iterator > 15){
            packet_index++;
            iterator = 0;
        }
        iterator++;
        bitcount++;
        //what to do if we overflow TDI_DATA_IN_Buffer?
        if (packet_index == TMS_DATA_IN_MAX - 1) break;


    }



    uint32_t clockticks = bitcount;
    err = EudJtagTmsShift(raw_bit_stream, clockticks);
    /*
    //This takes care of 32bit packets
    i = 0;
    if (packet_index > 0){
        for (; i < packet_index; i++){
            //jtag_data->number = iterator;
            //err = ShiftRawEud(jtag_data);
            err = EudJtagTmsShift(raw_bit_stream[i], TMS_DATA_COUNT_SIZE);
        }
    }

    //This takes care of the remainder packet
    //Iterator should still be remaining from for loop above
    uint32_t clockticks = iterator;
    err = EudJtagTmsShift(raw_bit_stream[i], clockticks);
    */

    delete jtag_data;
    return err;
}

//}


    /*
    //If jtag is in error state, return
    EUD_ERR_t err;
    if ((err = jtag_state_.GetJtagErrorState())){ return err; }
    
    EudJtagData* jtag_data = new EudJtagData;
    jtag_data->number = 0;
    jtag_data->tdi = new uint16_t[TDI_DATA_IN_MAX];
    jtag_data->tms = new uint16_t[TMS_DATA_IN_MAX];
    jtag_data->tdo = new uint32_t[TDO_BUFFER_MAX];
    
    jtag_data->tms[0] = 0;
    jtag_data->tdi[0] = 0;

    uint32_t i;
    uint32_t iterator = 0;
    uint32_t length = strlen(shiftvalue);
    for (i = 0; i < length; i++){
        if (*(shiftvalue + i) == ' ') continue;
        //Look up ascii to bin  converter
        if (!((*(shiftvalue + i) == '1') || (*(shiftvalue + i) == '0'))) continue;

        if (*(shiftvalue + i) == '1')
            //jtag_data->tms[iterator] = 1;
            jtag_data->tms[0] |= 1<<iterator;
        //else if (*(shiftvalue + i) == '0')
        //    jtag_data->tms[iterator] = 0;
        //else continue; //reject any ascii's other than 1,0

        //jtag_data->tms[iterator] = (uint8_t)*(shiftvalue + i);
        if (iterator == TMS_DATA_IN_MAX - 1) break;
        iterator++;
    }
    
    jtag_data->number = iterator;
#ifdef USE_SHIFT_RAW_FXN
    err = ShiftRawEud(jtag_data);
#else
    uint32_t clockticks = iterator;
    err = EudJtagTmsShift((uint32_t)(*jtag_data->tms), clockticks);
#endif
    delete jtag_data;
    return err;
    */
//}
#define use_shift_raw_eud 1

EUD_ERR_t
JtagEudDevice::EudJtagTmsShift(uint16_t* rawbitstream, uint32_t clockticks){



#ifdef use_shift_raw_eud
    //uint32_t payload = 0;
    uint8_t * payload = new uint8_t[JTG_PCKT_SZ_SEND_NBIT_KEEP + 1];

    for (uint8_t* i = payload; i < (payload + JTG_PCKT_SZ_SEND_NBIT_KEEP); i++){
        *i = 0;
    }

    //get the number of bytes
    //uint32_t bytecount = (uint32_t)ceil(clockticks / 8);
    EUD_ERR_t err;
    EudJtagData* jtag_data = new EudJtagData;
    memcpy(jtag_data->tms_, rawbitstream, TMS_DATA_IN_MAX);
    //jtag_data->tms[0] = tmsval;
    jtag_data->number_ = clockticks;
    err = ShiftRawEud(jtag_data);
    delete jtag_data;

    /*
    EUD_ERR_t err;
    EudJtagData* jtag_data = new EudJtagData;
    uint32_t j = 0;
    for (uint32_t i = 0; i < bytecount; i++){
        if (!(i & 1)) jtag_data->tms[i] = (uint8_t)rawbitstream[i] && 0xff; //even
        else          jtag_data->tms[i] = (uint8_t)(rawbitstream[i - 1] >> 8) && 0xff; //odd

    }
    

    jtag_data->number = clockticks;

    
    err = ShiftRawEud(jtag_data);
    err = JtagScan(
        const_cast<uint8_t*>(jtag_data->tms),
        const_cast<uint8_t*>(jtag_data->tdi),
        jtag_data->tdo,
        jtag_data->number,
        );
    delete jtag_data;
    */

#else
    EUD_ERR_t err = EudJtagGenericShift(JTG_TMS, rawbitstream, clockticks);
#endif
    return err;

}



EUD_ERR_t
JtagEudDevice::EudJtagTmsShift(uint32_t tmsval, uint32_t clockticks){
    EUD_ERR_t err;
    //uint32_t payload = 0;
    uint8_t * payload = new uint8_t[JTG_PCKT_SZ_SEND_TMS];
    
    for (uint8_t* i = payload; i < (payload + JTG_PCKT_SZ_SEND_TMS); i++){ 
        *i = 0; 
    }
#ifdef use_shift_raw_eud
    
    EudJtagData* jtag_data = new EudJtagData;

    jtag_data->tms_[0] = tmsval;
    jtag_data->number_ = clockticks;
    err = ShiftRawEud(jtag_data);
    delete jtag_data;
#else
    payload[0] = CMD_JTAG_TMS;
    payload[1] = ( tmsval           & 0x000000FF);
    payload[2] = ((tmsval     >> 8) & 0x000000FF);
    payload[3] = ( clockticks       & 0x000000FF);
    payload[4] = ((clockticks >> 8) & 0x000000FF);
    
    err = UsbWrite(payload, JTG_PCKT_SZ_SEND_TMS);
#endif
    return err;

}


EUD_ERR_t 
JtagEudDevice::EudShiftReg(const char* shiftvalue){
    return EudShiftTdi(shiftvalue);
}

EUD_ERR_t
JtagEudDevice::EudShiftTdi(const char* shiftvalue){

    //If jtag is in error state, return
    EUD_ERR_t err;
    if ((err = jtag_state_.GetJtagErrorState())){ return err; }

    
    
    
    //uint16_t* raw_bit_stream = new uint16_t[TDI_DATA_IN_MAX];
    uint16_t raw_bit_stream[TDI_DATA_IN_MAX];
    memset(raw_bit_stream, 0, (TDI_DATA_IN_MAX*sizeof(uint16_t)));

    

//CryptStringToBinaryA
    int32_t i;
    uint32_t iterator = 0;
    uint32_t length = (uint32_t)strlen(shiftvalue);
    //uint32_t data_index = 0;
    uint32_t packet_index = 0;
    uint32_t bitcount = 0;
#define LSBFIRST 1
#ifdef LSBFIRST

    //FIXME - leaving out LSB on highest index of raw_bit_stream[]
    for (i = length; i >= 0;){
        
        if (!((*(shiftvalue + i) == '1') || (*(shiftvalue + i) == '0'))) {
            i--;
            continue; //Only accept '1' or '0'
        }

        if (*(shiftvalue + i) == '1') //Apply bit to raw_bit_stream
            raw_bit_stream[packet_index] |= 1 << iterator;

        if (iterator >= 15){    //Check what iterator and index should be.
            packet_index++;
            iterator = 0;
        }
        else{
            iterator++;        
        }
        bitcount++;

        i--;
        
        //what to do if we overflow TDI_DATA_IN_Buffer?
        if (packet_index == TDI_DATA_IN_MAX - 1) break;
        
    }
#else //LSBFIRST
    for (i = 0; i < length; i++){
        if (!((*(shiftvalue + i) == '1') || (*(shiftvalue + i) == '0'))) continue;

        if (*(shiftvalue + i) == '1')
            raw_bit_stream[packet_index] |= 1 << iterator;

        if (iterator > 15){
            packet_index++;
            iterator = 0;
        }
        iterator++;
        bitcount++;
        //what to do if we overflow TDI_DATA_IN_Buffer?
        if (packet_index == TDI_DATA_IN_MAX - 1) break;

    }
#endif //LSBFIRST
    
    uint16_t* tdo_bit_stream = new uint16_t[TDO_DATA_IN_MAX];
    memset(tdo_bit_stream, 0, TDO_DATA_IN_MAX);
    uint8_t* tdo_raw_out = new uint8_t[TDO_DATA_IN_MAX * 2];
    memset(tdo_raw_out, 0, TDO_DATA_IN_MAX * 2);
    
    
    uint32_t clockticks = bitcount;

    //Print out bitstream
    QCEUD_Print("\nraw_bit_stream: ");
    for (i = packet_index; i >=0;){
        QCEUD_Print("%x", raw_bit_stream[i], clockticks);
        i--;
    }
    QCEUD_Print("\nclockticks: %d\n\n", clockticks);
    
    
    err = EudJtagTdiShift(raw_bit_stream, clockticks);


    /*
    //This takes care of 32bit packets
    i = 0;
    if (packet_index > 0){
        for (; i < packet_index; i++){
                //jtag_data->number = iterator;
                err = EudJtagTdiShift(raw_bit_stream[i], TDI_DATA_COUNT_SIZE);

        }
    }
    
    //This takes care of the remainder packet
    //Iterator should still be remaining from for loop above
    uint32_t clockticks = iterator;
    err = EudJtagTdiShift(raw_bit_stream[i], clockticks);
    
    //Now read out expected TDO (same number as TDI clockticks)
    */
    
    return err;
}





EUD_ERR_t
JtagEudDevice::EudJtagTdiShift(uint16_t* rawbitstream, uint32_t clockticks){


    
    //get the number of bytes
    //uint32_t bytecount = ceil(clockticks / 8);

#ifdef use_shift_raw_eud
    EUD_ERR_t err;
    EudJtagData* jtag_data = new EudJtagData;
    memcpy(jtag_data->tdi_, rawbitstream, TMS_DATA_IN_MAX);
    //jtag_data->tms[0] = tmsval;
    jtag_data->number_ = clockticks;
    err = ShiftRawEud(jtag_data);
    delete jtag_data;
    return err;

    /*  int32_t err;
        EudJtagData* jtag_data = new EudJtagData;
        uint32_t j = 0;
        for (uint32_t i = 0; i < bytecount; i++){
        if (!(i & 1)) jtag_data->tdi[i]   = (uint8_t) rawbitstream[i]       && 0xff; //even
        else          jtag_data->tdi[i] =   (uint8_t)(rawbitstream[i-1]>>8) && 0xff; //odd

        }

        jtag_data->number = clockticks;
        //err = ShiftRawEud(jtag_data);
        err = JtagScan(
        const_cast<uint8_t*>(jtag_data->tms),
        const_cast<uint8_t*>(jtag_data->tdi),
        jtag_data->tdo,
        jtag_data->number,
        );
        delete jtag_data;
        */
#else


    /*
    payload[0] = CMD_JTAG_NBIT_KEEP;
    payload[1] = (tdival & 0x000000FF);
    payload[2] = ((tdival >> 8) & 0x000000FF);
    payload[3] = (clockticks & 0x000000FF);
    payload[4] = ((clockticks >> 8) & 0x000000FF);
    payload[5] = CMD_JTAG_FLUSH; //flush command
    */
    EUD_ERR_t err =  
        EudJtagGenericShift(JTG_TDI, rawbitstream, clockticks);
    return err;
#endif
}

EUD_ERR_t 
JtagEudDevice::EudJtagGenericShift(shifttype_t shifttype, uint16_t* rawbitstream, uint32_t clockticks){
    
    uint8_t fullbitcommand;
    uint8_t nbitcommand;
    if (shifttype == JTG_TDI){
        fullbitcommand = CMD_JTAG_32BIT_KEEP;
        nbitcommand = CMD_JTAG_NBIT_KEEP;
    }
    else{
        fullbitcommand = CMD_JTAG_TMS;
        nbitcommand = CMD_JTAG_TMS;
    }
    //uint32_t payload = 0;
    uint8_t * payload = new uint8_t[JTG_PCKT_SZ_SEND_NBIT_KEEP + 1];

    memset(payload, 0, JTG_PCKT_SZ_SEND_NBIT_KEEP + 1);

    EUD_ERR_t err;
    uint32_t stream_index = 0;
    //This takes care of 32bit packets
    if (clockticks > 32){
        
        uint32_t numpackets = (uint32_t)ceil(clockticks / 16);
        
        for (; stream_index < numpackets;){
            payload[0] = fullbitcommand;
            payload[1] = (rawbitstream[stream_index] & 0x000000FF);
            payload[2] = ((rawbitstream[stream_index] >> 8) & 0x000000FF);
            payload[3] = (rawbitstream[stream_index + 1] & 0x000000FF);
            payload[4] = ((rawbitstream[stream_index + 1] >> 8) & 0x000000FF);
            //jtag_data->number = iterator;

            if ((err = UsbWrite(payload, JTG_PCKT_SZ_SEND_32BIT_KEEP))!=0) return jtag_state_.LogJtagError(err);
            
            stream_index += 2;
        }
    }
    
    if (clockticks > 16){
        //stream_index;
        payload[0] = nbitcommand;
        payload[1] = (rawbitstream[stream_index] & 0x000000FF);
        payload[2] = ((rawbitstream[stream_index] >> 8) & 0x000000FF);
        payload[3] = (16); //16 clock ticks 
        //payload[4] = ((clockticks >> 8) & 0x000000FF);
        payload[4] = 0;
        //jtag_data->number = iterator;

        if ((err = UsbWrite(payload, JTG_PCKT_SZ_SEND_NBIT_KEEP))!=0) return jtag_state_.LogJtagError(err);
        

        stream_index++;
    }
    
    uint32_t remainingbits = clockticks & 0xFFFF; //gives remainder after dividing by 16
    if (remainingbits > 0){
        //This takes care of the remainder packet
        //Iterator should still be remaining from for loop above
        //uint32_t clockticks = iterator;
        
        

        payload[0] = nbitcommand;
        payload[1] = (rawbitstream[stream_index] & 0x000000FF);
        payload[2] = ((rawbitstream[stream_index] >> 8) & 0x000000FF);
        payload[3] = (remainingbits);
        //payload[4] = ((clockticks >> 8) & 0x000000FF); //shouldn't be needed
        payload[4] = 0;
        //jtag_data->number = iterator;

        if ((err = UsbWrite(payload, JTG_PCKT_SZ_SEND_NBIT_KEEP))!=0) return jtag_state_.LogJtagError(err);

        
    }
    

    


    //flush
    uint8_t flushcmd[1] = { CMD_JTAG_FLUSH };
    err = UsbWrite(flushcmd, JTG_PCKT_SZ_SEND_FLUSH);
    
    if (shifttype == JTG_TDI){
        uint8_t numbytestoread;
        if (clockticks <= 16) numbytestoread = 1;
        else numbytestoread = (uint8_t)ceil(clockticks / 16 * 2);
        if ((err = UsbRead(numbytestoread, (tdo_buffer_ + tdo_index_)))!=0) return jtag_state_.LogJtagError(err);
        tdo_index_ += numbytestoread;
    }
    return err;

}


EUD_ERR_t
JtagEudDevice::EudJtagTdiShift(uint32_t tdival, uint32_t clockticks){

    //uint32_t payload = 0;
    uint8_t * payload = new uint8_t[JTG_PCKT_SZ_SEND_NBIT_KEEP+1];

    for (uint8_t* i = payload; i < (payload + JTG_PCKT_SZ_SEND_NBIT_KEEP); i++){
        *i = 0;
    }

#ifdef use_shift_raw_eud
    EUD_ERR_t err;
    EudJtagData* jtag_data = new EudJtagData;

    jtag_data->tdi_[0] = tdival;
    jtag_data->number_ = clockticks;
    err = ShiftRawEud(jtag_data);
    delete jtag_data;
#else

    payload[0] = CMD_JTAG_NBIT_KEEP;
    payload[1] = ( tdival           & 0x000000FF);
    payload[2] = ((tdival >> 8)     & 0x000000FF);
    payload[3] = ( clockticks       & 0x000000FF);
    payload[4] = ((clockticks >> 8) & 0x000000FF);
    payload[5] = CMD_JTAG_FLUSH; //flush command

    EUD_ERR_t err = UsbWrite(payload, JTG_PCKT_SZ_SEND_NBIT_KEEP+1);


    
    uint8_t numbytestoread;
    if (clockticks < 16) numbytestoread = 1;
    else numbytestoread = 2;
    UsbRead(numbytestoread, (tdo_buffer_ + tdo_index_));
    tdo_index_ += numbytestoread;
#endif
    return err ;

}
//////////////////////////////////////////////////////////
//
//    Function: EudReadTdo
//
//    Reads out data in the tdo_buffer_ to given tdo_data_ret value
//    FIXME -  User must allocate pointer to tdo_data_ret
///
//FIXME - needs to flush things out if buffered commands
EUD_ERR_t
JtagEudDevice::EudReadTdo(uint8_t* tdo_data_ret){

    //If jtag is in error state, return
    EUD_ERR_t err;
    if ((err = jtag_state_.GetJtagErrorState())){ return err; }

    if (tdo_index_ == 0){
        *tdo_data_ret = 0;
        return EUD_ERR_TDO_BUFFER_UNDERFLOW;
    }
    *tdo_data_ret = tdo_buffer_[tdo_index_];
    tdo_index_--;
    return EUD_SUCCESS;

}

EUD_ERR_t
JtagEudDevice::EudFillTdoBuffer(EudJtagData* jtag_data){
    if (tdo_index_ >= TDO_BUFFER_MAX) return EUD_ERR_TDO_BUFFER_OVERFLOW;
    
    uint8_t size = 0;
    uint8_t i;
    //if (jtag_data->number < 8) size = 1;
    //FIXME - which of these to expect?
    size = tdo_data_available_;
    size = num_tdo_bytes_expected_;
    //size = ceil((float)((jtag_data->number) / 8));
    
    
    for (i = 0; i < size; i++){
        tdo_index_++;
        tdo_buffer_[tdo_index_] = *jtag_data->tdo_;
        tdo_data_available_--; //Decrement available data counter
        if (tdo_index_ >= TDO_BUFFER_MAX) return EUD_ERR_TDO_BUFFER_OVERFLOW;
    }
    
    
    return EUD_SUCCESS;
}




uint32_t convert_uint8_to_hex(uint8_t* data){
    uint32_t returndata = 0;

    return returndata;
}
//Separate data into bytes and pass to JtagScan
EUD_ERR_t
JtagEudDevice::ShiftRawEud(EudJtagData * data) {

    if (!data->number_) return EUD_SUCCESS;

    uint8_t* newtms = new uint8_t[TMS_DATA_IN_MAX * 2];
    uint8_t* newtdi = new uint8_t[TDI_DATA_IN_MAX * 2];
    uint8_t* newtdo = new uint8_t[TDO_DATA_IN_MAX * 2];

    memset(newtms, 0, TMS_DATA_IN_MAX * 2);
    memset(newtdi, 0, TDI_DATA_IN_MAX * 2);
    memset(newtdo, 0, TDO_DATA_IN_MAX * 2);

    uint32_t numbytes = (uint32_t)ceil((data->number_) / 8) + 1;
    //uint32_t j = 0;
#ifdef TMS_IS_16bits
    for (uint32_t i = 0; i < numbytes;){
        if (!(i & 1)){
            newtms[i] = (uint8_t)(data->tms[j] & 0xff); //even
            newtdi[i] = (uint8_t)(data->tdi[j] & 0xff); //even
        }
        else{
            newtms[i] = (uint8_t)((data->tms[j] >> 8) & 0xff); //odd
            newtdi[i] = (uint8_t)((data->tdi[j] >> 8) & 0xff); //odd
            j++; 
        }
        i++;
    }
#else
    memcpy(newtms, data->tms_, numbytes);
    memcpy(newtdi, data->tdi_, numbytes);
        
#endif
        
    EUD_ERR_t err = JtagScan(
        const_cast<uint8_t*>(newtms),
        const_cast<uint8_t*>(newtdi),
        newtdo,
        data->number_
        );

    memcpy(data->tdo_, newtdo, 4);

    delete [] newtms;
    delete [] newtdi;
    if (err != 0) {
        QCEUD_Print("Error - ShiftRawEud - JtagScan. Err: %d\n", err);
        return err;
    }

    return EUD_SUCCESS;
}
/*
EUD_ERR_t 
JtagEudDevice::ShiftRawEud(EudJtagData * data) {
    
    if (!data->number) return EUD_SUCCESS;

    uint8_t* newtms = new uint8_t[2];
    uint8_t* newtdi = new uint8_t[2];
    uint8_t* newtdo = new uint8_t[4];
    newtms[0] = (uint8_t)( *(data->tms)      & 0x00FF);
    newtms[1] = (uint8_t)((*(data->tms)>>8) & 0x00FF);
    newtdi[0] = (uint8_t)( *(data->tdi) & 0x00FF);
    newtdi[1] = (uint8_t)((*(data->tdi) >> 8) & 0x00FF);

    EUD_ERR_t err = JtagScan(
                        const_cast<uint8_t*>(newtms), 
                        const_cast<uint8_t*>(newtdi), 
                        newtdo, 
                        data->number
                        );

    memcpy(data->tdo, newtdo, 4);

    delete newtms;
    delete newtdi;
    if (err != 0) {
        QCEUD_Print("Error - ShiftRawEud - JtagScan. Err: %d\n", err);
        return err;
    }

    return EUD_SUCCESS;
}
*/
EUD_ERR_t
JtagEudDevice::Quit(void)
{
    delete config_;
    UsbClose();
    return 0;
}

EUD_ERR_t
JtagEudDevice::BitBang(    uint32_t tdi,
                            uint32_t tms,
                            uint32_t tck,
                            uint32_t trst,
                            uint32_t srst,
                            uint32_t enable){
    uint16_t *bitbang_bits = (uint16_t *)&usb_buffer_out_[1];

    usb_buffer_out_[0] = CMD_JTAG_BITBANG;
    usb_buffer_out_[1] = 0x0;
    usb_buffer_out_[2] = 0x0;
    usb_buffer_out_[3] = 0x0;
    usb_buffer_out_[4] = 0x0;

    if (tdi > 0)
    {
        *bitbang_bits |= 1 << BITBANG_TDI_OFFSET;
    }

    if (tms > 0)
    {
        *bitbang_bits |= 1 << BITBANG_TMS_OFFSET;
    }

    if (tck > 0)
    {
        *bitbang_bits |= 1 << BITBANG_TCK_OFFSET;
    }

    /* trst active low (ntrst)*/
    *bitbang_bits |= 1 << BITBANG_NTRST_OFFSET;
    if (trst > 0)
    {
        *bitbang_bits &= 0 << BITBANG_NTRST_OFFSET;
    }

    /* srst active low */
    *bitbang_bits |= 1 << BITBANG_NSRST_OFFSET;
    if (srst > 0)
    {
        *bitbang_bits &= 0 << BITBANG_NSRST_OFFSET;
    }

    usb_buffer_out_[5] = CMD_JTAG_FLUSH;

    return UsbWriteRead(BITBANG_SEND_SIZE, JTG_PCKT_SZ_RECV_BITBANG);
}

EUD_ERR_t
JtagEudDevice::UsbWriteRead(uint32_t out_length, uint32_t in_length)
{

    EUD_ERR_t err;

    if ((err = UsbWrite(out_length))!=0) return err;

    if (in_length > 0){
        err = UsbRead(in_length, usb_buffer_in_);
    }
    return err;
}
void
JtagEudDevice::ResetCounters(void)
{
    tms_count_ = 0;
    tdi_count_ = 0;
    memset(usb_buffer_out_, 0, JTAG_OUT_BUFFER_SIZE);
    memset(usb_buffer_in_, 0, JTAG_IN_BUFFER_SIZE);
    memset(tms_buffer_, 0, JTAG_OUT_BUFFER_SIZE);
    memset(tdi_buffer_, 0, JTAG_OUT_BUFFER_SIZE);
    //usb_bytes_pending_out = 0;
    usb_num_bytes_pending_out_ = 0;
    //usb_bytes_pending_in = 0;
    usb_num_bytes_pending_in_ = 0;

    num_tdo_bytes_expected_ = 0;
    //MOved to SWD?
    //Current_IN_idx = 0;
    tdo_index_ = 0;
}


uint32_t
JtagEudDevice::BitsFreeTms()
{
    // A TMS shift allows a 16 bit payload and 16-bit count. Therefore, 
    // with 1 byte for the instruction, 5 bytes are needed 
    // for each (16-bit command plus 3 bytes) and the remaining partial bits
    uint32_t full_seq_bits = (tms_count_ / 16) * (32 + 8);
    uint32_t partial_seq_bits = (tms_count_ % 16) + 24;
    //uint32_t available_bits = ((JTAG_OUT_BUFFER_SIZE - usb_bytes_pending_out) * 8) - (full_seq_bits + partial_seq_bits);
    uint32_t available_bits = ((JTAG_OUT_BUFFER_SIZE - usb_num_bytes_pending_out_) * 8) - (full_seq_bits + partial_seq_bits);

    return available_bits;
}

uint32_t
JtagEudDevice::BitsFreeTdi()
{
    uint32_t full_out = (tdi_count_ / 32) * (32 + 8);
    uint32_t N_shift_bits = (tdi_count_ % 32);
    uint32_t N_out = (N_shift_bits / 16) * (32 + 8) + (N_shift_bits % 16) + (16 + 8);
    //uint32_t available_out = ((JTAG_OUT_BUFFER_SIZE - usb_bytes_pending_out) * 8) - (full_out + N_out);
    uint32_t available_out = ((JTAG_OUT_BUFFER_SIZE - usb_num_bytes_pending_in_) * 8) - (full_out + N_out);
    
    // Also need to check the IN buffer size. The JTAG peripheral will stop processing commands 
    // from the OUT buffer when the IN buffer is full.
    // There is no instruction on the return value, so each overhead has 1 less byte.
    uint32_t full_in = (num_tdo_bytes_expected_ / 32) * 32;
    uint32_t N_in_bits = num_tdo_bytes_expected_ % 32;
    uint32_t partial_in = (N_in_bits / 16) * 32 + (N_in_bits % 16) + 16;
    //uint32_t available_in = ((JTAG_IN_BUFFER_SIZE - usb_bytes_pending_in) * 8) - (full_in + partial_in);
    uint32_t available_in = ((JTAG_IN_BUFFER_SIZE - usb_num_bytes_pending_in_) * 8) - (full_in + partial_in);
    
    #if defined ( EUD_WIN_ENV )
    return min(available_out, available_in);
    #elif defined (EUD_LNX_ENV)  
        return std::min(available_out, available_in);
    #endif
}

void
JtagEudDevice::InsertTms(uint8_t tms)
{
    uint32_t index_var = tms_count_ / 8;
    uint32_t bit_index = tms_count_ % 8;
    uint8_t bit = 1 << bit_index;

    assert(index_var < JTAG_OUT_BUFFER_SIZE);


    /* we do not pad TMS later, so be sure to initialize all bits */
    if (0 == bit_index)
    {
        tms_buffer_[index_var] = 0;
    }

    if (tms)
    {
        tms_buffer_[index_var] |= bit;
    }
    else
    {
        tms_buffer_[index_var] &= ~bit;
    }

    tms_count_++;
}

void
JtagEudDevice::InsertTdi(uint8_t tdi, uint8_t *tdo_p)
{
    uint32_t index = tdi_count_ / 8;

    uint32_t bit_index = tdi_count_ % 8;
    uint8_t bit = 1 << bit_index;

    if (0 == bit_index)//clears out byte
    {
        tdi_buffer_[index] = 0;
    }

    if (tdi)
    {
        tdi_buffer_[index] |= bit;
    }
    else
    {
        tdi_buffer_[index] &= ~bit;
    }

    tdi_count_++;

    if (tdo_p != NULL)
    {
        num_tdo_bytes_expected_++;
    }

}
#if 0
static uint8_t *bitwise_copy(uint8_t *dest,
    uint32_t dest_offset,
    uint8_t *src,
    uint32_t src_offset,
    uint32_t length)
{
    uint8_t *src_start = src + src_offset / 8;
    uint8_t *dest_start = dest + dest_offset / 8;
    uint32_t src_shift = src_offset % 8;
    uint32_t dest_shift = dest_offset % 8;

    for (uint32_t i = 0; i < length; i++)
    {
        if (((*src_start >> src_shift) & 1) == 1)
        {
            *dest_start |= 1 << dest_shift;
        }
        else
        {
            *dest_start &= ~(1 << dest_shift);
        }
        //realign to byte
        if (src_shift++ == 7)
        {
            src_shift = 0;
            src_start++;
        }
        if (dest_shift++ == 7)
        {
            dest_shift = 0;
            dest_start++;
        }
    }

    return dest;
}
#endif
void
JtagEudDevice::RunScan(uint8_t scan_length,
uint8_t *tdo,
uint8_t tdo_offset,
bool final_scan)
{
    EUD_ERR_t err;
    FlushTdiQueue(tdi_count_,final_scan);
    //uint8_t result = UsbWrite(usb_bytes_pending_out);
    //err = UsbWrite(usb_bytes_pending_out);
    err = UsbWrite(usb_num_bytes_pending_out_);
    
    //if (result == usb_bytes_pending_out)
    if (err == EUD_USB_SUCCESS)
    {
        if (num_tdo_bytes_expected_ > 0)
        {
            //err = UsbRead(usb_bytes_pending_in,&result);
            uint8_t * data = new uint8_t[JTAG_IN_BUFFER_SIZE];
            //err = UsbRead(usb_bytes_pending_in,&notdata);
            err = UsbRead(usb_num_bytes_pending_in_, data);
            //use internal buffers instead of data
            delete [] data;

            //if (result == usb_bytes_pending_in)
            if (err != EUD_USB_SUCCESS){
                QCEUD_Print("RunScan error - USB Read error. Err code: %d\n", err);
            }
            else{
                assert(tdo != NULL);

                //why twice?
                //uint32_t full_bytes = scan_length / 8;
                uint32_t full_bytes = scan_length<8 ? 1 : (scan_length / 8);

                //why do we always read a null first character?
                //memcpy(tdo, usb_buffer_in_, full_bytes);

#ifndef OLD_TDO_COPY_METHOD
                uint32_t buffer_in_index = 0;
                uint32_t tdo_index = 0;
                uint32_t buffer_out_index = 0;
                for (; buffer_out_index + JTG_PCKT_SZ_RECV_GENERAL < usb_num_bytes_pending_out_;){
                    //check usbwrite command.
                    //read correct amount of data depending on that command

                    //buffer_out_index  - used to determine opcode is, which tells us the size of the incoming packet
                    //buffer_in_index   - index of the usb_buffer_in_, to tell us what the next data to copy to TDO buffer is
                    //tdo_incrementer - keeps track of where tdo_index_ should be so that tdo_index_ can be updated after buffer filled.

                    //switch (usb_num_bytes_pending_out_[index]){
                    switch (usb_buffer_out_[buffer_out_index]){

                    case JTG_CMD_NBIT_TOSS:
                    case JTG_CMD_NBIT_END_TOSS:
                    case JTG_CMD_32BIT_TOSS:
                    case JTG_CMD_32BIT_END_TOSS:
                        //Don't read any data, go to next command
                        buffer_out_index += JTG_PCKT_SZ_SEND_GENERAL;
                        break;
                    // For 32bit commands, keep entire 4 bytes returned
                    case JTG_CMD_32BIT_KEEP:
                    case JTG_CMD_32BIT_END_KEEP:
                        //memcpy((tdo + buffer_in_index), (usb_buffer_in_ + buffer_in_index), JTG_PCKT_SZ_RECV_32BIT_KEEP);
                        memcpy((tdo_buffer_ + tdo_index_ + tdo_index), usb_buffer_in_ + buffer_in_index, JTG_PCKT_SZ_RECV_32BIT_KEEP);
                        buffer_in_index += JTG_PCKT_SZ_RECV_32BIT_KEEP;
                        buffer_out_index += JTG_PCKT_SZ_SEND_GENERAL;
                        tdo_index += JTG_PCKT_SZ_RECV_32BIT_KEEP;
                        break;
                    // For Nbit commands, keep only first 2 bytes returned
                    case JTG_CMD_NBIT_KEEP:
                    case JTG_CMD_NBIT_END_KEEP:
                        //memcpy((tdo + tdo_index), (usb_buffer_in_ + buffer_in_index), JTG_VALID_BYTES_RECV_NBIT_KEEP);
                        memcpy((tdo_buffer_ + tdo_index_ + tdo_index), usb_buffer_in_ + buffer_in_index, JTG_VALID_BYTES_RECV_NBIT_KEEP);
                        buffer_in_index += JTG_PCKT_SZ_RECV_NBIT_KEEP;
                        buffer_out_index += JTG_PCKT_SZ_SEND_GENERAL;
                        tdo_index += JTG_VALID_BYTES_RECV_NBIT_KEEP;
                        break;
                    default://switch  on remaining commands. Shouldn't really make it here, 
                            //but in case these commands are present, increment buffer_in_index properly
                        switch (usb_buffer_out_[buffer_out_index]){
                        case CMD_JTAG_NOP:
                            buffer_out_index += JTG_PCKT_SZ_RECV_NOP + JTG_PCKT_SZ_SEND_OPCODE;
                            break;
                        case CMD_JTAG_FLUSH:
                            buffer_out_index += JTG_PCKT_SZ_RECV_FLUSH + JTG_PCKT_SZ_SEND_OPCODE;
                            break;
                        case CMD_JTAG_FREQ_WR:
                            buffer_out_index += JTG_PCKT_SZ_RECV_FREQ_WR + JTG_PCKT_SZ_SEND_OPCODE;
                            break;
                        case CMD_JTAG_DELAY:
                            buffer_out_index += JTG_PCKT_SZ_RECV_DELAY + JTG_PCKT_SZ_SEND_OPCODE;
                            break;
                        case CMD_JTAG_BITBANG:
                            buffer_out_index += JTG_PCKT_SZ_RECV_BITBANG + JTG_PCKT_SZ_SEND_OPCODE;
                            break;
                        case CMD_JTAG_TMS:
                            buffer_out_index += JTG_PCKT_SZ_RECV_TMS + JTG_PCKT_SZ_SEND_OPCODE;
                            break;
                        case CMD_JTAG_PERIPH_RST:
                            buffer_out_index += JTG_PCKT_SZ_RECV_RST + JTG_PCKT_SZ_SEND_OPCODE;
                            break;
                        case CMD_JTAG_FREQ_RD:
                            buffer_out_index += JTG_PCKT_SZ_RECV_FREQ_RD + JTG_PCKT_SZ_SEND_OPCODE;
                            break;
                        default:
                            buffer_out_index++;

                        } //second switch
    
                    } //first switch

                } //forloop

                //Now copy full bytes into tdo
                memcpy(tdo, (tdo_buffer_ + tdo_index_), full_bytes);
                //Update tdo_index_
                tdo_index_ += tdo_index;

#else
                memcpy(tdo, (usb_buffer_in_+1), full_bytes);
                memcpy((tdo_buffer_ + tdo_index_), (usb_buffer_in_ + 1), full_bytes);
                tdo_index_ += full_bytes;
                /* Copy tdo to the buffer */
                bitwise_copy(tdo,
                    tdo_offset,
                    (usb_buffer_in_+1),
                    0,
                    scan_length);
#endif
            }
        }
    }

    ResetCounters();
}

const char* Get_TAP_State_String(jtag_state state_idx){
    switch (state_idx){
    case JTAG_STATE_INVALID:
        return "JTAG_INVALID";
        break;
    case JTAG_STATE_DREXIT2:
        return "JTAG_DREXIT2";
        break;
    case JTAG_STATE_DREXIT1:
        return "JTAG_DREXIT1";
        break;
    case JTAG_STATE_DRSHIFT:
        return "JTAG_DRSHIFT";
        break;
    case JTAG_STATE_DRPAUSE:
        return "JTAG_DRPAUSE";
        break;
    case JTAG_STATE_IRSELECT:
        return "JTAG_IRSELECT";
        break;
    case JTAG_STATE_DRUPDATE:
        return "JTAG_DRUPDATE";
        break;
    case JTAG_STATE_DRCAPTURE:
        return "JTAG_DRCAPTURE";
        break;
    case JTAG_STATE_DRSELECT:
        return "JTAG_DRSELECT";
        break;
    case JTAG_STATE_IREXIT2:
        return "JTAG_IREXIT2";
        break;
    case JTAG_STATE_IREXIT1:
        return "JTAG_IREXIT1";
        break;
    case JTAG_STATE_IRSHIFT:
        return "JTAG_IRSHIFT";
        break;
    case JTAG_STATE_IRPAUSE:
        return "JTAG_IRPAUSE";
        break;
    case JTAG_STATE_IDLE:
        return "JTAG_IDLE";
        break;
    case JTAG_STATE_IRUPDATE:
        return "JTAG_IRUPDATE";
        break;
    case JTAG_STATE_IRCAPTURE:
        return "JTAG_IRCAPTURE";
        break;
    case JTAG_STATE_RESET:
        return "JTAG_RESET";
        break;
    default:
        return "Unknown!";

    }
}

/* ********************************************************************** */
/* Process a scan sequence.  T32 passes the entire TMS sequence along     */
/* with an equal-length TDI and (optional) TDO chain. This function       */
/* separates TDI and TMS sequences before passing them along to the       */
/* corresponding EUD command.                                             */
/*                                                                        */
/* Inputs:                                                                */
/*   JTAG in/out bits:                                                    */
/*      uint8_t tms_raw                                                   */
/*      uint8_t tdi_raw                                                   */
/*      uint8_t tdo_raw                                                   */
/*      uint8_t tms_raw                                                   */
/*   JTAG scan size:                                                      */
/*      uint8_t scan_size                                                 */
/* ********************************************************************** */
#define PRINT_TAP_STATE 1
EUD_ERR_t
JtagEudDevice::JtagScan(
uint8_t *tms_raw,
uint8_t *tdi_raw,
uint8_t *tdo_raw,
uint32_t scan_length)
{
    jtag_state_t current_state = jtag_state_.GetCurrentState();
    uint32_t scan_start_offset = 0;
    uint32_t bits_available =
        ((current_state == JTAG_STATE_DRSHIFT)
        || (current_state == JTAG_STATE_IRSHIFT))
        ? BitsFreeTdi() : BitsFreeTms();

    for (uint32_t i = 0; i < scan_length; i++)
    {
        jtag_state_t prev_state = current_state;
        uint32_t tms = (tms_raw[i / 8] >> (i % 8)) & 0x1;

        current_state = jtag_state_.CalcNextState(prev_state, tms);
#ifdef PRINT_TAP_STATE
        QCEUD_Print("TAP State: %s\n",Get_TAP_State_String(current_state));
#endif


        if ((current_state != JTAG_STATE_DRSHIFT)
            && (current_state != JTAG_STATE_IRSHIFT))
        {
            if ((current_state == JTAG_STATE_DREXIT1)    ///If we're in exit, shfit in with *NBIT*END instruction (?)
                || (current_state == JTAG_STATE_IREXIT1))
            {   //exit state machine
                //assert((i - scan_start_offset) == tdi_count_);
                if ((i - scan_start_offset) != tdi_count_){
                    QCEUD_Print("JTAG_Scan error. State is %d, expected tdi_count_ (%d) to match i-scan_start_offset (%d) Error code: %d\n", tdi_count_, (i-scan_start_offset),EUD_ERR_JTAG_SCAN_BAD_STATE);
                    return jtag_state_.LogJtagError(EUD_ERR_JTAG_SCAN_BAD_STATE);

                }

                // Complete the scan and assert TMS 1
                InsertTdi(((tdi_raw[i / 8] >> (i % 8)) & 0x1), tdo_raw);
                //
                RunScan(tdi_count_, tdo_raw, scan_start_offset, true);
                tdo_data_available_ += tdi_count_;
                bits_available = BitsFreeTms();
            }
            else
            {
                InsertTms(tms);
                bits_available--;

                if (bits_available < 1)
                {
                    FlushTmsQueue();

                    //assert(num_tdo_bytes_expected == 0);
                    if (num_tdo_bytes_expected_ != 0){
                        QCEUD_Print("JTAG_Scan error. Expected num_tdo_bytes_expected (%d) to be 0. Error code: %d\n", num_tdo_bytes_expected_, EUD_ERR_JTAG_SCAN_BAD_STATE);
                        return jtag_state_.LogJtagError(EUD_ERR_JTAG_SCAN_BAD_STATE);
                    }


                    EUD_ERR_t err = UsbWrite(usb_num_bytes_pending_out_);
                    
                    if (err != EUD_USB_SUCCESS){
                        QCEUD_Print("JTAG_Scan USB write error. Error code: %d\n", err);
                        return jtag_state_.LogJtagError(err);
                    }
                    ResetCounters();

                    bits_available = BitsFreeTms();
                }

            }
        }
        else
        {
            // IRSHIFT or DRSHIFT
            uint32_t tdi;

            if ((prev_state != JTAG_STATE_DRSHIFT) ///if we're now entering DR/IR shift, shift in our TMS bits so that hardware is in sync. (Don't do this if we're sitting in DR/IR shift)
                && (prev_state != JTAG_STATE_IRSHIFT))
            {
                // A new transition into shift
                InsertTms(tms);

                FlushTmsQueue();

                //assert(num_tdo_bytes_expected == 0);
                if (num_tdo_bytes_expected_ != 0){
                    QCEUD_Print("JTAG_Scan error. Error code: %d\n", EUD_ERR_JTG_EXPECTED_NONZERO_TDO);
                    return jtag_state_.LogJtagError(EUD_ERR_JTG_EXPECTED_NONZERO_TDO);
                }


                ////////////////////Write out bytes if in IR SHIFT or DR SHIFT////////////////////////
                EUD_ERR_t err = UsbWrite(usb_num_bytes_pending_out_);

                //assert(result == usb_bytes_pending_out);
                if (err != EUD_USB_SUCCESS){
                    QCEUD_Print("JTAG_Scan USB write error. Error code: %d\n", err);
                    return jtag_state_.LogJtagError(err);
                }

                ResetCounters();

                // Track the bit offset where this scan starts
                scan_start_offset = i + 1;

                bits_available = BitsFreeTdi();

                //////////Insert the first TDI Bit/////////////////
                //tdi = (tdi_raw[i / 8] >> (i % 8)) & 0x1; //find if tdi bit per index is 1 or 0
                //InsertTdi(tdi, tdo_raw);
                //bits_available--;

            }
            else
            {   ///////////////we're idle in SHIFT IR / SHIFT DR state. Start queuing up TDI bits////////////////////////
                tdi = (tdi_raw[i / 8] >> (i % 8)) & 0x1; //find if tdi bit per index is 1 or 0
                InsertTdi(tdi, tdo_raw);
                bits_available--;

                if (bits_available < 1)
                {
                    RunScan(tdi_count_, tdo_raw, scan_start_offset, true);
                    bits_available = BitsFreeTdi();
                    tdo_data_available_ += tdi_count_;
                }
            }
            
        }

        jtag_state_.SetState(current_state);
        //i++;
    }
    ///////////////////////If we're in IR shift or DR shift, push in any pending TDI bits/////////////////////
    if (((current_state == JTAG_STATE_DRSHIFT)
        || (current_state == JTAG_STATE_IRSHIFT))
        && (tdi_count_ > 0))
    {
        uint32_t partial_scan_size = scan_length - scan_start_offset;
        //RunScan(partial_scan_size, tdo_raw, scan_start_offset, false);
        RunScan(partial_scan_size, tdo_raw, scan_start_offset, true);
        tdo_data_available_ += partial_scan_size;
    }

    return EUD_SUCCESS;
}
/* API to assert a reset using a EUD peripheral */
EUD_ERR_t JtagEudDevice::AssertReset(void) {
    return EUD_SUCCESS;
}

/* API to de-assert reset using a EUD peripheral */
EUD_ERR_t JtagEudDevice::DeAssertReset(void) {
return EUD_SUCCESS;
}

/* ********************************************************************** */
/* Send a scan sequence to the EUD.                                       */
/*                                                                        */
/* Inputs:                                                                */
/*      EUD_ERR_t final_scan: Whether it is the final packet                    */
/*      of this sequence (for generating NBIT_END instruction variant)  */
/* ********************************************************************** */
EUD_ERR_t
JtagEudDevice::FlushTdiQueue(uint32_t tdi_count, bool final_scan)
{
    uint32_t byte_length, in_size = 0;
    uint32_t num_commands, num_full_tdi;
    uint8_t full_shift_opcode, full_shift_end_opcode, n_shift_opcode, n_shift_end_opcode;
    uint32_t usb_pending_out_index;
    if (0 == tdi_count){
        return 0;
    }

    
    // # 32-bit shifts instructions + # n-bit shifts instructions
    //byte_length = (tdi_count / 32) + DIV_ROUND_UP(tdi_count % 32, 16);
    //#define DIV_ROUND_UP(m, n)    (((m) + (n) - 1) / (n))
    byte_length = (tdi_count / 32) + ( ((tdi_count % 32) + 16 - 1) / 16);

    //Checks that we're not overflowing buffers
    assert((byte_length * JTG_PCKT_SZ_SEND_GENERAL) <= JTAG_OUT_BUFFER_SIZE);
    assert((byte_length * JTG_PCKT_SZ_RECV_GENERAL) <= JTAG_IN_BUFFER_SIZE);

    ///////////////
    //Determine opcode to use
    ///////////
    if (num_tdo_bytes_expected_ > 0) //Keep return bits => use *_KEEP opcode
    {
        full_shift_opcode = CMD_JTAG_32BIT_KEEP;
        full_shift_end_opcode = final_scan ? CMD_JTAG_32BIT_END_KEEP : CMD_JTAG_32BIT_KEEP;
        n_shift_opcode = CMD_JTAG_NBIT_KEEP;
        n_shift_end_opcode = final_scan ? CMD_JTAG_NBIT_END_KEEP : CMD_JTAG_NBIT_KEEP;
    }
    else //Don't keep return bits => Don't use *_KEEP opcode
    {
        full_shift_opcode = CMD_JTAG_32BIT_TOSS;
        full_shift_end_opcode = final_scan ? CMD_JTAG_32BIT_END_TOSS : CMD_JTAG_32BIT_TOSS;
        n_shift_opcode = CMD_JTAG_NBIT_TOSS;
        n_shift_end_opcode = final_scan ? CMD_JTAG_NBIT_END_TOSS : CMD_JTAG_NBIT_TOSS;
    }

    ////////////////////////////////////////////////
    //number of full TDI words to pass in. If less than division/round up, then go to partial command
    //num_full_tdi = DIV_ROUND_UP(tdi_count, 32) - 1;
    //#define DIV_ROUND_UP(m, n)    (((m) + (n) - 1) / (n))
    // final one
    // Do 1 less packet than necessary and use the special *_END variant for the
    //Fill up the USB OUT buffer with opcodes and their commands.
    num_full_tdi = (((tdi_count)+ 32 - 1 ) / 32) - 1;
    QCEUD_Print("top of FlushTdiQueue, num_full_tdi: %d\n", num_full_tdi);
    for (
        num_commands = 0;
        num_commands < num_full_tdi;
        num_commands++){

                usb_pending_out_index = usb_num_bytes_pending_out_ + 5 * num_commands;

                PackCmd32(
                    full_shift_opcode,
                    &usb_buffer_out_[usb_pending_out_index],
                    &tdi_buffer_[4 * num_commands]);

                in_size += 4;
    }



    

    ///John
    ///////////this is for cases where there's 32bits or less left to shift in
    //first block shifts things the upper 16 bits
    //later block shifts in lower 16 bits
    //notes:
        //block above handles when things are 32 bits (using the 32bit opcodes)
        //note that EUD hw will add +1 bit to end of some of these, so account for that by subtracting a packet size
        //e.g. this line: remaining_bit_pos[num_tdi_left / 8] &= (1 << upper_bits) - 1;
    
    //Problem to fix is to make sure that shifting occurs correctly. 
    //Case is missed when tdi_count_ == 16


    //Then handle the remaining bits (i.e. if packet was not a multiple of 32)
    //Since it's the last packet, use the special *_END variant
    uint8_t num_tdi_left = (tdi_count % 32);
    if (num_tdi_left > 0)
    {
        //bad line?
        //uint8_t num_tdi_left = (tdi_count% 32) % 16;
        
        ////////////////////////////////
        //bits 16-31
        ////////////////////////////////
        uint8_t *remaining_bit_pos = &tdi_buffer_[4 * num_commands];
        uint16_t word_value = 0;
        // 16-31 bits remain. Do 1 N-packet
        if ((tdi_count % 32) > 16)
        {
            usb_pending_out_index = usb_num_bytes_pending_out_ + 5 * num_commands;

            PackCmdN(n_shift_opcode,
                &usb_buffer_out_[usb_pending_out_index],
                *((uint16_t *)remaining_bit_pos),
                0xf);

            in_size += 4;
            remaining_bit_pos += 2;
            //if (tdi_count!=16){
                num_tdi_left -= 16;
            //}
            
            num_commands++;
        }
        

        ////////////////////////////////
        //bits 0-15
        ////////////////////////////////
        
        //if (num_tdi_left > 0){}
        //uint8_t* data_bits = new uint8_t[2];
        //upper data bits
        if (num_tdi_left > 8){
            word_value += (*(remaining_bit_pos+1) & 0xFF)<<8;
        }
        //data_bits[1] = (num_tdi_left & 0xFF00) >> 8;
        //lower data bits
        word_value += (*remaining_bit_pos & 0xFF);
        

        QCEUD_Print("\t\tword_value=%x.\n", word_value);

        unsigned upper_bits = num_tdi_left % 8;
        uint16_t copysize = (((num_tdi_left)+8 - 1) / 8);
        memcpy(&word_value,
            remaining_bit_pos,
            copysize);
        

        QCEUD_Print("FlushTdiQueue - bits 0-15.\n");
        QCEUD_Print("\t\tnum_tdi_left=%d.\n", num_tdi_left);
        QCEUD_Print("\t\t*remaining_bit_pos=%x.\n", *remaining_bit_pos);
        QCEUD_Print("\t\tword_value=%x.\n", word_value);

        //whta's this doing? -> Looks like  this is filling up bits 9->16. 
        if (upper_bits > 0){
            remaining_bit_pos[num_tdi_left / 8] &= (1 << upper_bits) - 1;
        }

        in_size += 4;
        //This is where tdi_count==16 has issue, as num_tdi_left is 0 and we get -1 passed to PackCmdN
        uint16_t countvar = num_tdi_left > 0 ? num_tdi_left - 1 : 0;
        //take off one if it's a final_scan
        //uint16_t tdi_shiftbits = final_scan ? num_tdi_left - 1 : num_tdi_left;
        PackCmdN(n_shift_end_opcode,
            //&usb_buffer_out_[usb_bytes_pending_out + 5 * num_commands],
            &usb_buffer_out_[usb_num_bytes_pending_out_ + 5 * num_commands],
            word_value,
            //tdi_shiftbits); //error?
            countvar);
        num_commands++;
        


    }
    else
    {
        PackCmd32(full_shift_end_opcode,
            
            &usb_buffer_out_[usb_num_bytes_pending_out_ + 5 * num_commands],
            &tdi_buffer_[4 * num_commands]);
        in_size += 4;
        num_commands++;
    }

    byte_length = 5 * num_commands;

    if (num_tdo_bytes_expected_ > 0) //add a flush command at the end
    {
        //usb_buffer_out_[usb_bytes_pending_out + 5 * num_commands] = CMD_JTAG_FLUSH;
        usb_buffer_out_[usb_num_bytes_pending_out_ + 5 * num_commands] = CMD_JTAG_FLUSH;
        byte_length++;

        //usb_bytes_pending_in += in_size;
        usb_num_bytes_pending_in_ += in_size;
    }

    //usb_bytes_pending_out += byte_length;
    usb_num_bytes_pending_out_ += byte_length;

    tdi_count = 0;

    /////////////////////////////////
    // If it's final scan, we add a '1' to TMS
    // at end of our instruction (automatically done
    // via the *_END command.
    // Update jtag_state_, which shadows hardware, accordingly
    if (final_scan == true){
        jtag_state_.SetState(jtag_state_.CalcNextState(jtag_state_.GetCurrentState(), 1));
        QCEUD_Print("TAP State : %s (Just appended TMS 1 since *_END opcode)\n", Get_TAP_State_String(jtag_state_.GetCurrentState()));
    }


    return EUD_SUCCESS;
}

EUD_ERR_t
JtagEudDevice::FlushTmsQueue(void)
{
    uint32_t byte_length;
    unsigned i;

    if (0 == tms_count_)
    {
        return 0;
    }

    byte_length = DIV_ROUND_UP(tms_count_, 16);
    assert((byte_length * 5) <= JTAG_OUT_BUFFER_SIZE);

    for (i = 0; i < (tms_count_ / 16); i++)
    {
        uint8_t *remaining_bit_ptr = &tms_buffer_[2 * i];

        PackCmdN(CMD_JTAG_TMS,
            //&usb_buffer_out_[usb_bytes_pending_out + 5 * i],
            &usb_buffer_out_[usb_num_bytes_pending_out_ + 5 * i],
            *((uint16_t *)remaining_bit_ptr),
            0xf);
    }

    uint8_t partial_bitcount = tms_count_ % 16;
    if (partial_bitcount > 0)
    {
        uint16_t rem_bits = 0x0;
        uint8_t *remaining_bit_ptr = &tms_buffer_[2 * i];
        unsigned upper_bits = partial_bitcount % 8;

        memcpy(&rem_bits,
            &tms_buffer_[2 * i],
            DIV_ROUND_UP(partial_bitcount, 8));
        if (upper_bits > 0)
        {
            remaining_bit_ptr[partial_bitcount / 8] &= (1 << upper_bits) - 1;
        }

        PackCmdN(CMD_JTAG_TMS,
            //&usb_buffer_out_[usb_bytes_pending_out + 5 * i],
            &usb_buffer_out_[usb_num_bytes_pending_out_+ 5 * i],
            rem_bits,
            (partial_bitcount - 1)); //why - 1? in somecases, see this should be 5 but this gives it 3...
            //Because hardware (per spec) shifts out N+1 bits
            //(partial_bitcount+1)); //why - 1? in somecases, see this should be 5 but this gives it 3...
        i++;
    }

    byte_length = 5 * i;

    //usb_bytes_pending_out += byte_length;
    usb_num_bytes_pending_out_ += byte_length;

    tms_count_ = 0;

    return 0;
}


//#define LOWLEVEL_LITTLEENDIAN 1
///*****************************************************************************/
///* EUD command packing utilities */
inline uint8_t *
JtagEudDevice::PackCmd32(uint8_t opcode,
uint8_t *dest,
const uint8_t *src)
{
    if ((NULL == dest)
        || (NULL == src))
    {
        return NULL;
    }

    *dest = opcode;
#ifdef LOWLEVEL_LITTLEENDIAN
    //memcpy((dest + 1), src, 4);
    *(dest + 1) = *(src + 0);
    *(dest + 2) = *(src + 1);
    *(dest + 3) = *(src + 2);
    *(dest + 4) = *(src + 3);
#else
    *(dest + 1) = *(src + 3);
    *(dest + 2) = *(src + 2);
    *(dest + 3) = *(src + 1);
    *(dest + 4) = *(src + 0);
#endif //LOWLEVEL_LITTLEENDIAN
    return dest;
}

// Pack a 5 byte command with an N-bit payload and count
// bits [15:0] data
// bits [31:16] count
inline uint8_t *
JtagEudDevice::PackCmdN(    uint8_t opcode,
                            uint8_t *dest,
                            const uint16_t data,
                            const uint16_t count)
{
    if (NULL == dest)
    {
        return NULL;
    }

    *dest = opcode;

    //if ((data == 0xffff)||(count==0xffff))
    if ((count == 0xffff))
    {
        return NULL;
    }

    
#ifdef LOWLEVEL_LITTLEENDIAN
    //uint16_t*  word_ptr = (uint16_t *)(dest + 1);
    //word_ptr[0] = data;
    //word_ptr[1] = count;

    *(dest + 2) = (uint8_t)(data & 0xFF);
    *(dest + 1) = (uint8_t)((data >> 8) & 0xFF);
    *(dest + 4) = (uint8_t)(count & 0xFF);
    *(dest + 3) = (uint8_t)((data >> 8) & 0xFF);

#else
    //uint16_t*  word_ptr = (uint16_t *)(dest + 1);

    *(dest + 1) = (uint8_t) (data&0xFF);
    *(dest + 2) = (uint8_t)((data>>8) & 0xFF);
    *(dest + 3) = (uint8_t)(count & 0xFF);
    *(dest + 4) = (uint8_t)((data >> 8) & 0xFF);
    
#endif //LOWLEVEL_LITTLEENDIAN
    return dest;
}



//Experimental - to be implemented
EUD_ERR_t
JtagEudDevice::JtagCmWriteRegister(uint32_t * reg_addr_p, uint32_t * reg_data_p){
    //word char_count;
    uint32_t reg_addr, address_offset;;
    //const char * the_string = "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0";
    EUD_ERR_t err = 0;
    //    uint32_t rd_data;
    uint8_t * jtag_tdo_data = new uint8_t[TDO_BUFFER_MAX];
    //word char_count;
    address_offset = 0;
    reg_addr = 0;
    reg_addr += address_offset;

    //   printf("wr 0x%04x:0x%08x\n", reg_addr, reg_data);

    
    EudShiftTms("1 0 0");//JTAG is in Shift DR
    //the_string = take_hex_number_and_turn_into_jtag_shift_command_w((uint32_t)reg_addr, (uint32_t)reg_data);

    
    EudShiftTms(" 1 0 1 0 0");//JTAG is in Shift DR
    EudShiftReg("0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");
    
    if ((err = EudReadTdo(jtag_tdo_data))!=0) return err;
    
    //T32_Cmd("JTAG.SHIFTTMS 1 0"); //JTAG is in Idle
    EudShiftTms("1 0");
    
    return EUD_SUCCESS;

}

EUD_ERR_t
JtagEudDevice::JtagCmReadRegister(uint32_t * reg_addr_p, uint32_t *  rd_data_p){

    //word char_count;
    uint32_t reg_addr, address_offset;
    reg_addr = 0;
    //Fixme - put this variable somewhere
    JtagEudDevice::config_->deviceType = 1;

    

    //char * the_string = "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0";
    //65 0's

    uint32_t rd_data;
    EUD_ERR_t err = 0;
    uint8_t * jtag_tdo_data = new uint8_t[TDO_BUFFER_MAX];
    
    //where does this come from?
    address_offset = 0;
    reg_addr += address_offset;

    
    if ((err = EudShiftTms("1 0 0"))!=0) return err;

    //implement
    //the_string = take_hex_number_and_turn_into_jtag_shift_command_r(reg_addr);

    
    if ((err = EudShiftTms("1 0 1 0 0"))!=0) return err;

    if (
        (config_->deviceType == 1) // Istari, Tesla etc
        ||
        (config_->deviceType == 3) // Lykan
        ||
        (config_->deviceType == 4) // Feero
        ||
        (config_->deviceType == 5) // Tesla,McLaren
        )
    {
        
        EudShiftReg("0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");
    }
    else if (config_->deviceType == 2)
    { // Nazgul
        
        if ((err = EudShiftTdi("0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0"))!=0) return err;
        
        if ((err = EudShiftReg("0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0"))!=0) return err;
    }

    if ((err = EudReadTdo(jtag_tdo_data))!=0) return err;
    rd_data = convert_uint8_to_hex(jtag_tdo_data);

    if ((err = EudShiftTms("1 0"))!=0) return err;

    *rd_data_p = rd_data;
    //return(rd_data);

    return EUD_SUCCESS;

}

