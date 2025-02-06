/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   jtag_api.h
*
* Description:                                                              
*   Header file for EUD JTAG peripheral public APIs declerations
*   Note: JTAG Peripheral not completed due to hardware limitations 
*
***************************************************************************/
#ifndef JTAG_API_H_
#define JTAG_API_H_

// #include "eud_api.h"
#include "eud_types.h"
#include "eud_error_defines.h"
#include "jtag_eud.h"

#define JTG_LIMITED_INIT_OPTION 1

#define  EUD_JTAG_FREQ_120_MHz		0x0
#define  EUD_JTAG_FREQ_80_MHz		0x1
#define  EUD_JTAG_FREQ_60_MHz		0x2
#define  EUD_JTAG_FREQ_40_MHz		0x3
#define  EUD_JTAG_FREQ_30_MHz		0x4
#define  EUD_JTAG_FREQ_15_MHz		0x5
#define  EUD_JTAG_FREQ_7_5_MHz		0x6
#define  EUD_JTAG_FREQ_3_75_MHz		0x7
#define  EUD_JTAG_FREQ_1_875_MHz	0x8
#define  EUD_JTAG_FREQ_0_938_MHz	0x9
#define  EUD_JTAG_FREQ_0_469_MHz	0xA
#define  EUD_JTAG_FREQ_0_234_MHz	0xB
#define  EUD_JTAG_FREQ_0_117_MHz	0xC
#define  JTAG_MAX_FREQUENCY			EUD_JTAG_FREQ_0_117_MHz


/**************************************//**
*
*   @brief Reset JTAG peripheral. JTAG peripheral should keep enumerated on USB.
*
*******************************************/                        
EXPORT EUD_ERR_t				///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
jtag_peripheral_reset(
    JtagEudDevice* jtg_handle_p	///<Pointer to EUD Handle instance
    );

/**************************************//**
*
*   @brief Causes the JTAG Peripheral to flush its IN buffer to the PC after next IN token.
*	The JTAG Peripheral signals the completion of the flush command to the PC by sending a partial or zero length packet.
*
*******************************************/  
EXPORT EUD_ERR_t 
jtag_nop(
    JtagEudDevice* jtg_handle_p
    );

/**************************************//**
*
*   @brief Causes the JTAG Peripheral to flush its IN buffer to the PC after next IN token.
*	The JTAG Peripheral signals the completion of the flush command to the PC by sending a partial or zero length packet.
*
*******************************************/  
EXPORT EUD_ERR_t 
jtag_flush(
    JtagEudDevice* jtg_handle_p
    );
/**************************************//**
*
*   @brief Write payload values to the JTAG output pins, and read reply values from JTAG input pins.
*	jtag_rctlr_srst_n is AND�ed with swd_rctlr_srst_n and ctl_rctlr_srst_n to produce eud_rctlr_srst_n, as shown in Figure 3 1.
*	The freezio_latch bit is set if rpmh_freezio_a_n_mx is asserted.
*	The freezio_latch bit is cleared after each JTAG_BITBANG command.
*	
*******************************************/  
EXPORT EUD_ERR_t 
jtag_bitbang(
    JtagEudDevice* jtg_handle_p, 
    uint32_t payload
    );

/**************************************//**
*
*   @brief Used to transition through JTAG states
*	N+1 bits from the payload are shifted out onto TMS, starting with bit 0.
*	If N+1 > 16, then the last payload bit (ie bit[15]) is repeated.
*	Used in all �pause� states and Test Logic Reset
*	
*******************************************/  
EXPORT EUD_ERR_t 
jtag_tms(
    JtagEudDevice* jtg_handle_p, 
    uint32_t payload
    );
/**************************************//**
*
*   @brief Shift N+1 bits in during SHIFT_IR or SHIFT_DR states.  
*	Do not end JTAG command sequence.  Do not return JTAG output.
*	Payload bit 0 is shifted out first.
*	If N+1 >16, the last payload bit is repeated.
*	
*******************************************/  
EXPORT EUD_ERR_t 
jtag_nbit_toss(
    JtagEudDevice* jtg_handle_p, 
    uint32_t payload
    );
/**************************************//**
*
*   @brief Shift N+1 bits into TAP controller during SHIFT_IF or SHIFT_DR states.  
*	Do not end JTAG command sequence.  Return JTAG output.
*	Payload bit[0] is shifted out first.  Reply bit[15] is loaded first.
*	Unused bits in the reply are zero.
*	If N+1 >16, the last payload bit is repeated, and reply bits after bit[15] are discarded.
*
*******************************************/ 
EXPORT EUD_ERR_t				///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
jtag_nbit_keep(	
    JtagEudDevice* jtg_handle_p,	///<Jtag device handle pointer.  Must be initialized from eud_initialize_device_jtag, else fatal error will occur.
                                ///<Note that handle checks are kept to a minimum in order to increase throughput.
    uint32_t payload,			///<Payload  to be sent to EUD device.
    uint32_t* response			///<Pointer to data to be returned from EUD device.
    );


/**************************************//**
*
*   @brief Shift N+1 bits into TAP controller during SHIFT_IF or SHIFT_DR states.  
*	End JTAG command sequence by asserting TMS during last shifted bit.  
*	Do not return JTAG output.
*
*******************************************/ 
EXPORT EUD_ERR_t 
jtag_nbit_end_toss(
    JtagEudDevice* jtg_handle_p, 
    uint32_t payload
    );


/**************************************//**
*
*   @brief Shift N+1 bits into TAP controller during SHIFT_IF or SHIFT_DR states.  
*	End JTAG command sequence by asserting TMS during last shifted bit.  
*	Return JTAG output.
*
*******************************************/ 
EXPORT EUD_ERR_t 
jtag_nbit_end_keep(
    JtagEudDevice* jtg_handle_p, 
    uint32_t payload, 
    uint32_t* response
    );

/**************************************//**
*
*   @brief Shift 32 bits into TAP controller during SHIFT_IR and SHIFT_DR states.  
*	Do not end JTAG command sequence.  Do not return JTAG output.
*	Payload bit[0] is shifted out first.  Reply bit[31] is loaded first.
*
*******************************************/ 
EXPORT EUD_ERR_t 
jtag_32bit_toss(
    JtagEudDevice* jtg_handle_p, 
    uint32_t payload
    );

/**************************************//**
*
*   @brief Shift 32 bits into TAP controller during SHIFT_IR and SHIFT_DR states.  
*	Do not end JTAG command sequence.  Return JTAG output.
*
*******************************************/ 
EXPORT EUD_ERR_t 
jtag_32bit_keep(
    JtagEudDevice* jtg_handle_p, 
    uint32_t payload,
    uint32_t* response
    );

/**************************************//**
*
*   @brief Shift 32 bits into TAP controller during SHIFT_IR and SHIFT_DR states.  
*	End JTAG command sequence by asserting TMS during last shifted bit.  
*	Do not return JTAG output.
*
*******************************************/ 
EXPORT EUD_ERR_t 
jtag_32bit_end_toss(
    JtagEudDevice* jtg_handle_p, 
    uint32_t payload
    );
/**************************************//**
*
*   @brief Shift 32 bits into TAP controller during SHIFT_IR and SHIFT_DR states. 
*	End JTAG command sequence by asserting TMS during last shifted bit.  
*	Return JTAG output.
*
*******************************************/ 
EXPORT EUD_ERR_t 
jtag_32bit_end_keep(
    JtagEudDevice* jtg_handle_p, 
    uint32_t payload, 
    uint32_t* response
    );


/**************************************//**
*
*   @brief Command resets the JTAG Peripheral while still keeping it enumerated with the host.  
*	The OUT and IN buffers are cleared, and programmable parameters are restored to 
*	their default values.
*	The operator is responsible for coordinating this reset with other processes on the chip that 
*	might be interfacing to this peripheral.
*	The JTAG Peripheral will NAK any OUT tokens if the IN buffer is full, 
*	and the OUT buffer is not empty.  
*	This could cause execution of the reset command to be delayed or ignored.  
*	To prevent this, the IN buffer must be drained when issuing a JTAG_Peripheral_Reset command.
*
*******************************************/ 
EXPORT EUD_ERR_t 
jtag_peripheral_reset(
    JtagEudDevice* jtg_handle_p
    );

/**************************************//**
*
*   @brief Set JTAG TCK frequency. 
*	Any values 0xD  or greater will result in bad parameter error.
*	Values are as follows:
*		- 0x0:  120 MHz
*		- 0x1:  80 MHz
*		- 0x2:  60 MHz
*		- 0x3:  40 MHz
*		- 0x4:  30 MHz
*		- 0x5:  15 MHz
*		- 0x6:  7.5 MHz
*		- 0x7:  3.75 MHz
*		- 0x8:  1.875 MHz
*		- 0x9:  0.938 MHz
*		- 0xA:  0.469 MHz
*		- 0xB:  0.234 MHz
*		- 0xC:  0.117 MHz
*		- 0xD to 0xF:  reserved
*
*******************************************/ 
EXPORT EUD_ERR_t 
jtag_set_frequency(
    JtagEudDevice* jtg_handle_p, 
    uint32_t frequency
    );

/**************************************//**
*
*   @brief Read current JTAG TCK frequency from EUD hardware
*	Values returned are as follows:
*		- 0x0:  120 MHz
*		- 0x1:  80 MHz
*		- 0x2:  60 MHz
*		- 0x3:  40 MHz
*		- 0x4:  30 MHz
*		- 0x5:  15 MHz
*		- 0x6:  7.5 MHz
*		- 0x7:  3.75 MHz
*		- 0x8:  1.875 MHz
*		- 0x9:  0.938 MHz
*		- 0xA:  0.469 MHz
*		- 0xB:  0.234 MHz
*		- 0xC:  0.117 MHz
*		- 0xD to 0xF:  reserved
*
*******************************************/ 
EXPORT EUD_ERR_t 
jtag_read_frequency(
    JtagEudDevice* jtg_handle_p, 
    uint32_t* response
    );




//QCOM_SNIP_BEGIN
inline EUD_ERR_t eud_jtag_generic_opcode(JtagEudDevice* jtg_handle_p, uint8_t opcode, uint32_t payload, uint32_t* response);


EXPORT EUD_ERR_t jtag_eud_speed(JtagEudDevice* jtg_handle_p, uint32_t speed);
EXPORT EUD_ERR_t jtag_eud_bitbang(	JtagEudDevice* jtg_handle_p, 
                                    uint8_t tdi, 
                                    uint8_t tms, 
                                    uint8_t tck, 
                                    uint8_t trst, 
                                    uint8_t srst, 
                                    uint8_t enable);
EXPORT EUD_ERR_t jtag_eud_scan(	JtagEudDevice* jtg_handle_p, 
                                uint8_t *tms_raw, 
                                uint8_t *tdi_raw, 
                                uint8_t *tdo_raw, 
                                uint32_t scan_length);

EXPORT EUD_ERR_t jtag_get_jtag_id(JtagEudDevice* jtg_handle_p, uint32_t* jtag_id);

//EXPORT EUD_ERR_t JTGSetFrequency(JtagEudDevice* jtg_handle_p, uint32_t freq);
//EXPORT EUD_ERR_t JTGSetFrequency(JtagEudDevice* jtg_handle_p, uint32_t Frequency);
EXPORT EUD_ERR_t jtag_set_delay(JtagEudDevice* jtg_handle_p, uint32_t payload);


//EXPORT EUD_ERR_t EudReadTdo(JtagEudDevice* jtg_handle_p, uint8_t* tdo_data_ret);
EXPORT EUD_ERR_t eud_read_tdo(uint8_t* tdo_data_ret, uint32_t* tdo_num_bytes_p);
EXPORT EUD_ERR_t eud_shift_tdi(JtagEudDevice* jtg_handle_p, char* shift_value);
EXPORT EUD_ERR_t eud_shift_tms(JtagEudDevice* jtg_handle_p, char* shift_value);
EXPORT EUD_ERR_t eud_shift_reg(JtagEudDevice* jtg_handle_p, char* shift_value);


EXPORT EUD_ERR_t eud_jtag_trst(JtagEudDevice* jtg_handle_p);


//error handling for jtag device
EXPORT EUD_ERR_t eud_jtag_clear_jtag_error_state(JtagEudDevice* jtg_handle_p);
//QCOM_SNIP_END
      
#endif //JTAG_API_H_

