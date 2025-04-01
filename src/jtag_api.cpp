/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   jtag_api.cpp
*
* Description:                                                              
*   CPP source file for EUD Jtag Peripheral APIs
*
***************************************************************************/

#include "jtag_api.h"
#include "jtag_eud.h"
#include <math.h>

inline EUD_ERR_t eud_jtag_generic_opcode(JtagEudDevice* jtg_handle_p, uint8_t opcode, uint32_t payload, uint32_t* response){

    EUD_ERR_t err;

    if (jtg_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }
    if (opcode > JTG_NUM_OPCODES){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }

    if (jtg_handle_p->jtag_check_opcode_queuable_table_[opcode]){
        if ( (err = jtg_handle_p->QueueCommand(opcode, payload, response)) != 0 ) return err;
        if ( (err = jtg_handle_p->ProcessBuffers()) != 0 ) return err;
    }
    else{
        if ( (err = jtg_handle_p->WriteCommand((uint32_t)opcode, payload, response)) != 0) return err;
    }

    return err;

}
#ifdef old_jtgcmd

    //Should not have anything in the USB Write queue pending.
    //Don't expect this error to occur.
    if ((jtg_handle_p->USB_Write_Index > 0) && (jtg_handle_p->handle->Mode == IMMEDIATEWRITEMODE)){
        return eud_set_last_error(EUD_JTG_ERR_REQUIRE_FLUSH);
    }

    uint32_t payload_sz, response_sz;
    payload_sz = jtg_handle_p->Periph_PayloadSize_Table[opcode];
    response_sz = jtg_handle_p->Periph_ResponseSize_Table[opcode];


    jtg_handle_p->USB_Write_Buffer[jtg_handle_p->USB_Write_Index] = opcode;
    if (payload_sz > 0){

        jtg_handle_p->USB_Write_Buffer[jtg_handle_p->USB_Write_Index + 1] = payload     & (0xFF);
        jtg_handle_p->USB_Write_Buffer[jtg_handle_p->USB_Write_Index + 2] = payload>>8  & (0xFF);
        jtg_handle_p->USB_Write_Buffer[jtg_handle_p->USB_Write_Index + 3] = payload>>16 & (0xFF);
        jtg_handle_p->USB_Write_Buffer[jtg_handle_p->USB_Write_Index + 4] = payload>>24 & (0xFF);

    }
    jtg_handle_p->USB_Write_Index += payload_sz;

    if ((jtg_handle_p->Mode == IMMEDIATEWRITEMODE)&&(response_sz > 0)){
        jtg_handle_p->USB_Write_Index += JTG_PCKT_SZ_SEND_FLUSH;
        jtg_handle_p->USB_Write_Buffer[jtg_handle_p->USB_Write_Index-1] = JTG_CMD_FLUSH;
        jtg_handle_p->USB_Read_Expected_Bytes += response_sz;
    }

    
    //TODO - wrap this in a try-catch block to catch pointer violations
    //TODO - Implement buffermodes  other than IMMEDIATEWRITE, this logic won't work.
    if (err = jtg_handle_p->handle->UsbWriteRead(   jtg_handle_p->USB_Write_Index, 
                                                    jtg_handle_p->USB_Write_Buffer,
                                                    jtg_handle_p->USB_Read_Expected_Bytes,
                                                    jtg_handle_p->USB_Read_Buffer
                                                    )) return err;
    
    if (jtg_handle_p->handle->Mode == IMMEDIATEWRITEMODE){
        jtg_handle_p->USB_Write_Index = 0;
        jtg_handle_p->USB_Read_Expected_Bytes = 0;
    }

    if ((response_sz > 0) && (jtg_handle_p->handle->Mode == IMMEDIATEWRITEMODE)){
        *response = jtg_handle_p->USB_Read_Buffer[jtg_handle_p->USB_Read_Index + 1];
        *response += jtg_handle_p->USB_Read_Buffer[jtg_handle_p->USB_Write_Index + 2] << 8;
        *response += jtg_handle_p->USB_Read_Buffer[jtg_handle_p->USB_Write_Index + 3] << 16;
        *response += jtg_handle_p->USB_Read_Buffer[jtg_handle_p->USB_Write_Index + 4] << 24;
    }

    return EUD_SUCCESS;
}


#endif

//===---------------------------------------------------------------------===//
//
// API Access
//
//===---------------------------------------------------------------------===//

EXPORT EUD_ERR_t jtag_peripheral_reset(JtagEudDevice* jtg_handle_p){
    
    //return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_PERIPHERAL_RESET, 0, NULL);
    return  jtg_handle_p->WriteCommand(JTG_CMD_PERIPHERAL_RESET);

}

EXPORT EUD_ERR_t jtag_nop(JtagEudDevice* jtg_handle_p) {
    
    //return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_NOP, 0, NULL);
    return  jtg_handle_p->WriteCommand(JTG_CMD_NOP);

}

EXPORT EUD_ERR_t jtag_flush(JtagEudDevice* jtg_handle_p) {
    
    //return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_FLUSH, 0, NULL);
    return  jtg_handle_p->WriteCommand(JTG_CMD_FLUSH);
}


EXPORT EUD_ERR_t jtag_set_delay(JtagEudDevice* jtg_handle_p, uint32_t delaytime){

    //return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_DELAY, delaytime, NULL);
    return  jtg_handle_p->WriteCommand(JTG_CMD_DELAY, delaytime);
}


EXPORT EUD_ERR_t jtag_bitbang(JtagEudDevice* jtg_handle_p, uint32_t payload) {

    //return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_BITBANG, payload, NULL);
    return  jtg_handle_p->WriteCommand(JTG_CMD_BITBANG, payload);
}


EXPORT EUD_ERR_t jtag_tms(JtagEudDevice* jtg_handle_p, uint32_t payload) {

    return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_TMS, payload, NULL);

}


EXPORT EUD_ERR_t jtag_nbit_toss(JtagEudDevice* jtg_handle_p, uint32_t payload) {

    return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_NBIT_TOSS, payload, NULL);

}

EXPORT EUD_ERR_t jtag_nbit_keep(JtagEudDevice* jtg_handle_p, uint32_t payload, uint32_t* response) {

    return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_NBIT_KEEP, payload, response);
    
}

EXPORT EUD_ERR_t jtag_nbit_end_toss(JtagEudDevice* jtg_handle_p, uint32_t payload) {
    
    return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_NBIT_END_TOSS, payload, NULL);
    
}


EXPORT EUD_ERR_t jtag_nbit_end_keep(JtagEudDevice* jtg_handle_p, uint32_t payload, uint32_t* response) {
    
    return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_NBIT_END_KEEP, payload, response);

}


EXPORT EUD_ERR_t jtag_32bit_toss(JtagEudDevice* jtg_handle_p, uint32_t payload) {

    return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_32BIT_TOSS, payload, NULL);
    
}


EXPORT EUD_ERR_t jtag_32bit_keep(JtagEudDevice* jtg_handle_p, uint32_t payload,uint32_t* response) {

    return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_32BIT_KEEP, payload, response);

}


EXPORT EUD_ERR_t jtag_32bit_end_toss(JtagEudDevice* jtg_handle_p, uint32_t payload) {

    return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_32BIT_END_TOSS, payload, NULL);
    
}


EXPORT EUD_ERR_t jtag_32bit_end_keep(JtagEudDevice* jtg_handle_p, uint32_t payload, uint32_t* response) {

    //return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_32BIT_END_KEEP, payload, response);
    return  jtg_handle_p->WriteCommand(JTG_CMD_32BIT_END_KEEP, payload, response);

}


EXPORT EUD_ERR_t jtag_set_frequency(JtagEudDevice* jtg_handle_p, uint32_t Frequency){
    if (Frequency > JTAG_MAX_FREQUENCY){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }
    //return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_FREQ_WRITE, Frequency, NULL);
    return  jtg_handle_p->WriteCommand(JTG_CMD_FREQ_WRITE, Frequency);

}

EXPORT EUD_ERR_t jtag_read_frequency(JtagEudDevice* jtg_handle_p, uint32_t* response) {

    //return eud_jtag_generic_opcode(jtg_handle_p, JTG_CMD_FREQ_READ, 0, response);
    return  jtg_handle_p->WriteCommand(JTG_CMD_FREQ_READ, response);

}


/////////////////Other JTAG API's. Potentially to deprecate?//////////////////

EXPORT EUD_ERR_t eud_jtag_trst(JtagEudDevice* jtg_handle_p){

    EUD_ERR_t err = EUD_SUCCESS;

    if (jtg_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    //handler_validity_checker should have checked for successful 

    //send two packets:
    // tdi tms tck trst_n rctlr_srst_n gpio_srst_n gpio_trst_n
    //   0   0   0      0            1           1           1
    //   0   0   0      1            1           1           1

    //err = myhandle->BitBang(tdi, tms, tck, trst, srst, enable);
    err = jtg_handle_p->BitBang(0, 0, 0, 1, 0, 0);//Note that bits are active low, but that's handled within BitBang()
    err = jtg_handle_p->BitBang(0, 0, 0, 0, 0, 0);
    return err;
}


EXPORT EUD_ERR_t eud_jtag_clear_jtag_error_state(JtagEudDevice* jtg_handle_p){

    EUD_ERR_t err = EUD_SUCCESS;

    if (jtg_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    err = jtg_handle_p->ClearJtagStateMachineErrorState();

    return err;
}

static uint8_t TDO_Result[TDO_DATA_IN_MAX];
static uint32_t TDO_Number = 0;
static bool TDO_WrittenFlag = FALSE;

EXPORT EUD_ERR_t eud_read_tdo(uint8_t* tdo_data_ret, uint32_t* tdo_num_bytes_p){

    if (TDO_WrittenFlag == FALSE) return EUD_JTG_ERR_TDO_EMPTY;

    if (TDO_Number >= TDO_BUFFER_MAX) return EUD_JTG_INTERNAL_ERROR; //Should never do this since TDO_Number is managed internally, but just in case...

    //Copy data and return
    *tdo_num_bytes_p = TDO_Number;
    memcpy(tdo_data_ret, TDO_Result, TDO_Number);
    return EUD_SUCCESS;
}


EXPORT EUD_ERR_t eud_shift_reg(JtagEudDevice* jtg_handle_p, char* shiftvalue){
    return eud_shift_tdi(jtg_handle_p, shiftvalue);
}

/**Read TDO from available buffer*/
EXPORT EUD_ERR_t eud_shift_tdi(JtagEudDevice* jtg_handle_p, char* shiftvalue){

    EUD_ERR_t err = EUD_SUCCESS;
    //Check and setup handles
    if (jtg_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    EudJtagData* jtag_data = new EudJtagData; //jtag data structure/object

    
    //Clear out our return buffer
    //This is read by EudReadTdo, 
    //but is cleared each time EudShiftTdi is called.
    memset(TDO_Result, 0, TDO_BUFFER_MAX);
    //static uint32_t TDO_Number = 0;
    //static bool TDO_WrittenFlag = FALSE;

    //Iterators and such
    uint32_t i; //Index for scanning given string
    uint32_t iterator = 0; //Index for shifting within array index (0->7)
    uint32_t length = (uint32_t)strlen(shiftvalue);
    uint32_t packet_index = 0; //Index for raw_bit_stream[packet_index]

    uint8_t* raw_bit_stream;
    raw_bit_stream = jtag_data->tdi_;//Assign bit stream to tdi pointer
#ifdef LSBFIRST
    for (i = length; i >= 0;){
        if (!((*(shiftvalue + i) == '1') || (*(shiftvalue + i) == '0'))) {
            i--;
            continue; //Only accept '1' or '0', jokers wild
        }

        if (*(shiftvalue + i) == '1') //update  bit
            raw_bit_stream[packet_index] |= 1 << iterator;

        if (iterator >= JTAG_DATA_BUF_UNIT_SIZE - 1){
            packet_index++;
            iterator = 0;
        }
        else{
            iterator++;
        }

        jtag_data->number++;
        i--;

        //Avoid running into space. Proper check is below
        if (packet_index + 1 >= TDI_EXACT_MAX_BITS) return EUD_ERR_MAX_TDI_PACKETS_EXCEEDED;
    }
#else
    for (i = 0; i <= length;){
        if (!((*(shiftvalue + i) == '1') || (*(shiftvalue + i) == '0'))) {
            i++;
            continue; //Only accept '1' or '0', jokers wild
        }

        if (*(shiftvalue + i) == '1') //update  bit
            raw_bit_stream[packet_index] |= 1 << iterator;

        if (iterator >= JTAG_DATA_BUF_UNIT_SIZE - 1){
            packet_index++;
            iterator = 0;
        }
        else{
            iterator++;
        }

        jtag_data->number_++;
        i++;

        //Avoid running into space. Proper check is below
        if (packet_index + 1 >= TDI_EXACT_MAX_BITS) return EUD_ERR_MAX_TDI_PACKETS_EXCEEDED;
    }

#endif //LSBFIRST
    uint32_t bytecount = (uint32_t)ceil(jtag_data->number_ / 8);

    //We can either be exactly the right clock ticks, or two bytes less due to opcode sizes
    if (bytecount > TDI_MAX_NET_PACKETS) {
        if (jtag_data->number_ == TDI_EXACT_MAX_BITS){
            return EUD_ERR_MAX_TDI_PACKETS_EXCEEDED;
        }
    }

#ifdef QCEUD_DEBUG_PRINT
    //Print out bitstream
    QCEUD_Print("\nTDI raw_bit_stream: ");
    for (i = bytecount; i > 0;){
        QCEUD_Print("%x", raw_bit_stream[i], jtag_data->number_);
        i--;
    }
    QCEUD_Print("\nclockticks: %d\n\n", jtag_data->number_);
#endif

    //Pass it to the jtag
    err = jtg_handle_p->JtagScan(jtag_data->tms_, jtag_data->tdi_, jtag_data->tdo_, jtag_data->number_);

    //Copy resulting TDO data to static var to be picked up by EudReadTdo.
    //This is cleared each time EudShiftTdi is called again
    memcpy(TDO_Result, jtag_data->tdo_, bytecount);
    TDO_Number = bytecount;
    TDO_WrittenFlag = TRUE;

#ifdef QCEUD_DEBUG_PRINT
    QCEUD_Print("\nTDO Received:");
    i = bytecount;
    if (i > 0){
        for (; i > 0; i--){
            QCEUD_Print("%x", TDO_Result[i]);
        }
    }
#endif

    delete jtag_data;
    return err;
}


/**Shift in TMS bits, char list*/
EXPORT EUD_ERR_t eud_shift_tms(JtagEudDevice* jtg_handle_p, char* shiftvalue){

    EUD_ERR_t err = EUD_SUCCESS;

    if (jtg_handle_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    }

    EudJtagData* jtag_data = new EudJtagData; //jtag data structure/object
    

    //Iterators and such
    uint32_t i; //Index for scanning given string
    uint32_t iterator = 0; //Index for shifting within array index (0->7)
    uint32_t length = (uint32_t)strlen(shiftvalue);
    uint32_t packet_index = 0; //Index for raw_bit_stream[packet_index]

    uint8_t* raw_bit_stream;
    raw_bit_stream = jtag_data->tms_;//Assign bit stream to tms pointer
#ifdef LSBFIRST
    for (i = length; i >= 0;){
        if (!((*(shiftvalue + i) == '1') || (*(shiftvalue + i) == '0'))) {
            i--;
            continue; //Only accept '1' or '0', jokers wild
        }

        if (*(shiftvalue + i) == '1') //update  bit
            raw_bit_stream[packet_index] |= 1 << iterator;

        if (iterator >= JTAG_DATA_BUF_UNIT_SIZE - 1){
            packet_index++;
            iterator = 0;
        }
        iterator++;
        jtag_data->number_++;
        i--;

        //Avoid running into space. Proper check is below
        if (packet_index + 1 > TMS_MAX_NET_PACKETS) return EUD_ERR_MAX_TMS_PACKETS_EXCEEDED;
    }
#else
    for (i = 0; i <= length;){
        if (!((*(shiftvalue + i) == '1') || (*(shiftvalue + i) == '0'))) {
            i++;
            continue; //Only accept '1' or '0', jokers wild
        }

        if (*(shiftvalue + i) == '1') //update  bit
            raw_bit_stream[packet_index] |= 1 << iterator;

        if (iterator >= JTAG_DATA_BUF_UNIT_SIZE - 1){
            packet_index++;
            iterator = 0;
        }
        iterator++;
        jtag_data->number_++;
        i++;

        //Avoid running into space. Proper check is below
        if (packet_index + 1 > TMS_MAX_NET_PACKETS) return EUD_ERR_MAX_TMS_PACKETS_EXCEEDED;
    }
#endif //LSBFIRST
    uint32_t bytecount = (uint32_t)ceil(jtag_data->number_ / 8);

    //We can either be exactly 64 clock ticks
    if (bytecount > TMS_MAX_NET_PACKETS) return EUD_ERR_MAX_TMS_PACKETS_EXCEEDED;

    //Print out bitstream
    QCEUD_Print("\nTMS raw_bit_stream: ");
    for (i = packet_index; i > 0;){
        QCEUD_Print("%x", raw_bit_stream[i], jtag_data->number_);
        i--;
    }
    QCEUD_Print("\nclockticks: %d\n\n", jtag_data->number_);

    //Pass it to the jtag
    err = jtg_handle_p->JtagScan(jtag_data->tms_, jtag_data->tdi_, jtag_data->tdo_, jtag_data->number_);

    delete jtag_data;
    return err;
}
