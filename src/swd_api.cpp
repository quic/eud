/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   swd_api.cpp
*
* Description:                                                              
*   CPP source file for EUD SWD Peripheral public APIs
*
***************************************************************************/

#include "swd_api.h"
#include "swd_eud.h"
#include <assert.h>

volatile uint32_t SWDStatusMaxCount = SWD_APPEND_SWDSTATUS_FLAG;
volatile uint32_t CurrentSWDStatusCounter = 0;
volatile bool g_fushNopAppend = true;
// extern volatile bool readFlush;
//=====================================================================================//
//                      SWD Commands
//
//                      These operations are used primarily for
//                      sending data to SWD controller. Note that
//                      there are 3 states that EUD host software
//                      supports for managing data from these commands,
//                      selectable by set_buf_mode(). See documentation
//						in header file for details.
//=====================================================================================//

EXPORT EUD_ERR_t swd_set_swd_status_max_count(SwdEudDevice *swd_handle_p, uint32_t swd_getstatus_maxvalue)
{
	if (swd_handle_p == NULL)
	{
		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	}

	if (swd_getstatus_maxvalue > SWD_GETSTATUS_MAX_STATUS_COUNTER)
	{
		return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
	}

	SWDStatusMaxCount = swd_getstatus_maxvalue;

	return EUD_SUCCESS;
}

inline void pack_uint32_to_uint8_array2(uint32_t data32, uint8_t *data8_p)
{
#ifdef SWD_LITTLEENDIAN
	data8_p[3] = (uint8_t)(*data32_p & 0xFF);
	data8_p[2] = (uint8_t)((*data32_p >> 8) & 0xFF);
	data8_p[1] = (uint8_t)((*data32_p >> 16) & 0xFF);
	data8_p[0] = (uint8_t)((*data32_p >> 24) & 0xFF);
#else
	data8_p[0] = (uint8_t)(data32 & 0xFF);
	data8_p[1] = (uint8_t)((data32 >> 8) & 0xFF);
	data8_p[2] = (uint8_t)((data32 >> 16) & 0xFF);
	data8_p[3] = (uint8_t)((data32 >> 24) & 0xFF);
#endif
}

/// For fast lookup of parity value. Only need first 16 values stored
uint32_t ParityLookupTable[256] = {
	// A2_3 RnW APnDP
	0, // 00	0	0
	1, // 00	0	1
	1, // 00	1	0
	0, // 00	1	1
	1, // 01	0	0
	0, // 01	0	1
	0, // 01	1	0
	1, // 01	1	1

	1, // 10	0	0
	0, // 10	0	1
	0, // 10	1	0
	1, // 10	1	1
	0, // 11	0	0
	1, // 11	0	1
	1, // 11	1	0
	0, // 11	1	1
};

inline EUD_ERR_t assemble_swd_packet(SwdCmd_t *swd_cmd_p, uint32_t RnW, uint32_t APnDP, uint32_t A2_3)
{
	swd_cmd_p->s.RnW = RnW & SWD_RnW_MASK;
	swd_cmd_p->s.APnDP = APnDP & SWD_APnDP_MASK;
	swd_cmd_p->s.A2_3 = A2_3 & SWD_A2_3_MASK;

	swd_cmd_p->s.start = 1;
	swd_cmd_p->s.stop = 0;
	swd_cmd_p->s.park = 1;

	uint8_t temp = swd_cmd_p->s.A2_3 << 2 | swd_cmd_p->s.RnW << 1 | swd_cmd_p->s.APnDP;

	// calculate parity, which is simply the number of bits set in swd command
	swd_cmd_p->s.parity = ParityLookupTable[temp];

#ifdef SWD_DEBUG_ON
	QCEUD_Print("Opcode : %x - APnDP: %d, RnW: %d, A2_3: %d\n", swd_cmd_p->cmd, swd_cmd_p->s.APnDP, swd_cmd_p->s.RnW, swd_cmd_p->s.A2_3);
	QCEUD_Print("%s operation to %s Register: %x \n", swd_cmd_p->s.RnW ? "Read" : "Write", swd_cmd_p->s.APnDP ? "Access Port" : "Debug Port", swd_cmd_p->s.A2_3 << 2);
#endif

	if (swd_cmd_p->cmd & SWD_STOP_BIT_MASK)
	{
		return eud_set_last_error(SWD_ERR_SWD_PACKET_ASSEMBLE_ERR);
	}
	return EUD_SUCCESS;
}

//
//  Return status of queue buffer depending on how full it is.
//  Logic can subsequently add more commands or not depending on response.
//
inline int buffer_bounds_check(uint32_t opcode, EudPacketQueue *eud_queue_p, uint32_t payloadsize)
{
	// We can append a payloadsize (opcode + data) + an additional flush to terminate the packet.
	uint32_t max_possible_command_size = eud_queue_p->raw_write_buffer_idx_ + payloadsize + SWD_PCKT_SZ_SEND_FLUSH;

	max_possible_command_size += SWD_PCKT_SZ_SEND_STATUS;

	uint32_t buffer_size = SWD_OUT_BUFFER_SIZE;
	// buffer_size= 7;

	if (max_possible_command_size > buffer_size)
		return QUEUE_FULL_COMMAND_NOT_QUEUED;

	return EUD_SUCCESS;
}

// special quque apui for adding FLUSH and NOP Commands
inline EUD_ERR_t queue_swd_packet_special(SwdEudDevice *swd_handle_p, uint8_t opcode, uint32_t senddata, uint32_t *return_ptr)
{

	// The current transaction for ease of reading.
	eud_opcode_transaction *thistransaction_p = &swd_handle_p->eud_queue_p_->transaction_queue_[swd_handle_p->eud_queue_p_->queue_index_];

#ifdef SWD_DEBUG_ON

	QCEUD_Print("\nqueue_swd_packet_special,cmd: %d\n", opcode);

#endif

	// payload for FLUSH and NULL commands is 1 byte and response size is 0 bytes
	thistransaction_p->response_size_ = swd_handle_p->periph_response_size_table_[opcode];
	thistransaction_p->payload_size_ = swd_handle_p->periph_payload_size_table_[opcode];

#ifdef SWD_DEBUG_ON

	if (thistransaction_p->response_size_ > 0)
		QCEUD_Print("queue_swd_packet_special:	rsponse Size is:%d\n", thistransaction_p->response_size_);
#endif

	//
	// Copy send/recieve data to appropriate buffers
	//
	thistransaction_p->opcode_payload_[0] = opcode;

	// payload_size for flush and nop is 1
	if (thistransaction_p->payload_size_ > 1)
	{
		pack_uint32_to_uint8_array2(senddata, (thistransaction_p->opcode_payload_ + 1));
	}

	//
	// copy opcode and payload into raw write buffer (length 1 or 5)
	//
	uint8_t *payload_destination = &swd_handle_p->eud_queue_p_->raw_write_buffer_[swd_handle_p->eud_queue_p_->raw_write_buffer_idx_];
	memcpy((payload_destination), thistransaction_p->opcode_payload_, (thistransaction_p->payload_size_));

	//
	// Now increment the indices to keep track of where things written to.
	// Opcode size needs to be added here since we added that in above.
	swd_handle_p->eud_queue_p_->raw_write_buffer_idx_ += thistransaction_p->payload_size_;
	swd_handle_p->eud_queue_p_->read_expected_bytes_ += thistransaction_p->response_size_;
	thistransaction_p->return_ptr_ = return_ptr;

#ifdef SWD_DEBUG_ON
	QCEUD_Print("queue_swd_packet_special: Raw Write Index: %d\n", swd_handle_p->eud_queue_p_->raw_write_buffer_idx_);
#endif

	// Increment index
	swd_handle_p->eud_queue_p_->queue_index_++;

	return EUD_SUCCESS;
}

uint32_t read_buffer_capacity_reached;
uint32_t max_read_buffer_capacity_used_so_far;

//
//  Queue requested packet into buffers with appropriate send and response sizes.
//
inline EUD_ERR_t queue_swd_packet(SwdEudDevice *swd_handle_p, uint8_t opcode, uint32_t senddata, uint32_t *return_ptr)
{

	// The current transaction for ease of reading.
	eud_opcode_transaction *thistransaction_p = &swd_handle_p->eud_queue_p_->transaction_queue_[swd_handle_p->eud_queue_p_->queue_index_];
	uint32_t read_buffer_capacity;

#ifdef SWD_DEBUG_ON
	QCEUD_Print("Queue swd entry Read packet flag: %d, response_size: %d, FlushRequired: %d \n", swd_handle_p->eud_queue_p_->read_packet_flag_, thistransaction_p->response_size_, swd_handle_p->flush_required_);
#endif

	// Depending on if this is a SWD command or a EUD SWD opcode, populate the queue packet sizes accordingly.

	// If it's a SWD command, we know what the sizes are. Update based on read or write.
	if (opcode >= SWD_CMD_CMD_BASEVALUE)
	{
		swd_handle_p->eud_queue_p_->status_stale_flag_ = true;
		swd_handle_p->eud_queue_p_->swd_command_flag_ = true;
		if (opcode & SWD_CMD_READ_BIT_MASK)
		{ // Read command
			swd_handle_p->eud_queue_p_->read_packet_flag_ = true;
			thistransaction_p->response_size_ = SWD_PCKT_SZ_RECV_SWDCMD_READ;
			thistransaction_p->payload_size_ = SWD_PCKT_SZ_SEND_SWDCMD_READ;
		}
		else
		{ // Write command
			thistransaction_p->response_size_ = SWD_PCKT_SZ_RECV_SWDCMD_WRITE;
			thistransaction_p->payload_size_ = SWD_PCKT_SZ_SEND_SWDCMD_WRITE;
			return_ptr = NULL;
		}

		// If swdstatus enabled, increment counter.
		if (SWDStatusMaxCount)
			CurrentSWDStatusCounter++;
	}
	// SWD Peripheral Device Opcode case. Get send/rcv and update flush policy
	else
	{

		if ((thistransaction_p->response_size_ > 0) && (return_ptr == NULL))
		{
			return eud_set_last_error(EUD_ERR_BAD_PARAMETER_NULL_POINTER_SWDREAD);
		}

		thistransaction_p->response_size_ = swd_handle_p->periph_response_size_table_[opcode];
		thistransaction_p->payload_size_ = swd_handle_p->periph_payload_size_table_[opcode];
		// If a response is required, a flush is needed along with a flush from upper layers
		swd_handle_p->flush_required_ = true;
	}
	//
	// Check bounds of send buffer, but only if opcode is not flush/status,
	// since bounds check already took these into account
	//
	swd_handle_p->eud_queue_p_->buffer_status_ = buffer_bounds_check(opcode, swd_handle_p->eud_queue_p_, thistransaction_p->payload_size_);

	if (swd_handle_p->eud_queue_p_->buffer_status_ == QUEUE_FULL_COMMAND_NOT_QUEUED)
	{

#ifdef SWD_DEBUG_ON
		QCEUD_Print("Queue_swd_packet start: QUEUE_FULL_COMMAND_NOT_QUEUED, returning success\n");
#endif

		return EUD_SUCCESS;
	}

#ifdef SWD_DEBUG_ON
	QCEUD_Print("Buffering Code active\n");
#endif

	// if (swd_handle_p->eud_queue_p->read_packet_flag == TRUE)
	if (opcode & SWD_CMD_READ_BIT_MASK)
	{

#ifdef SWD_DEBUG_ON
		QCEUD_Print("Buffering check: It is a read\n");
#endif

		//		QCEUD_Print("Buffering Code: Its a read and Return_ptr is : %p\n",(void *)return_ptr);
		// current command to be added to Queue could be a Read, in which case,  readbuff size 4 bytes should be <=SWD_IN_BUFFER_SIZE
		read_buffer_capacity = swd_handle_p->eud_queue_p_->read_expected_bytes_ + 4;

		if (read_buffer_capacity == (SWD_IN_BUFFER_SIZE))
		{
			read_buffer_capacity_reached++;

#ifdef SWD_DEBUG_ON
			QCEUD_Print("Buffering check: READ buff capacity reached - 32. No. of times reached: %d \n", read_buffer_capacity_reached);
			// QCEUD_Print("Buffering check: READ buff capacity reached - 28. No. of times reached: %d \n",read_buffer_capacity_reached);
			QCEUD_Print("Buffering check: Max Working READ buff capacity used so far: %d \n", max_read_buffer_capacity_used_so_far);
#endif
		}

		if ((read_buffer_capacity) >= (SWD_MAX_RESPONSE_SIZE))
		{
#ifdef SWD_DEBUG_ON
			QCEUD_Print("Buffering check: READ buff capacity reached. read_buff_capacity: %d \n", read_buffer_capacity);
#endif

			swd_handle_p->eud_queue_p_->buffer_status_ = QUEUE_FULL_COMMAND_NOT_QUEUED;
			//			QCEUD_Print("Buffering check: swd_handle_p->eud_queue_p->bufferstatus: %d",swd_handle_p->eud_queue_p->bufferstatus);
			return EUD_SUCCESS;
		}

#ifdef SWD_DEBUG_ON
		QCEUD_Print("Buffering check: READ buff capacity NOT reached. read_buff_capacity: %d \n", read_buffer_capacity);
#endif
	}


	//
	// Copy send/recieve data to appropriate buffers
	//
	thistransaction_p->opcode_payload_[0] = opcode;

	if (thistransaction_p->payload_size_ > 1)
	{
		pack_uint32_to_uint8_array2(senddata, (thistransaction_p->opcode_payload_ + 1));
	}

	//
	// copy opcode and payload into raw write buffer (length 1 or 5)
	//
	uint8_t *payload_destination = &swd_handle_p->eud_queue_p_->raw_write_buffer_[swd_handle_p->eud_queue_p_->raw_write_buffer_idx_];
	memcpy((payload_destination), thistransaction_p->opcode_payload_, (thistransaction_p->payload_size_));

	// All read operations will need a flush or if a write command followed by a response
	uint8_t FlushByte = 0;


	if (swd_handle_p->flush_required_ == true)
	{
		FlushByte = 2;
		payload_destination[thistransaction_p->payload_size_] = SWD_CMD_FLUSH;
		// payload_destination[thistransaction_p->payload_size + 1] = SWD_CMD_NOP;
#ifdef SWD_DEBUG_ON
		QCEUD_Print("Flush and NOP cmds added\n");
#endif
	}

	//
	// Now increment the indices to keep track of where things written to.
	// Opcode size needs to be added here since we added that in above.
	swd_handle_p->eud_queue_p_->raw_write_buffer_idx_ += thistransaction_p->payload_size_ + FlushByte;
	swd_handle_p->eud_queue_p_->read_expected_bytes_ += thistransaction_p->response_size_;
	if (swd_handle_p->eud_queue_p_->read_expected_bytes_ > max_read_buffer_capacity_used_so_far)
		max_read_buffer_capacity_used_so_far = swd_handle_p->eud_queue_p_->read_expected_bytes_;
	thistransaction_p->return_ptr_ = return_ptr;

#ifdef SWD_DEBUG_ON
	QCEUD_Print("Raw Write Index: %d, Flush Status: %d \n", swd_handle_p->eud_queue_p_->raw_write_buffer_idx, FlushByte);
#endif

	// Increment index
	swd_handle_p->eud_queue_p_->queue_index_++;

	return EUD_SUCCESS;
}

/// Overload  of queue_swd_packet for senddata a four 8byte array.
inline EUD_ERR_t queue_swd_packet(SwdEudDevice *swd_handle_p, uint8_t opcode, uint8_t *senddata, uint32_t *return_ptr)
{
	uint32_t senddata32;
	unpack_uint8_to_uint32_array(&senddata32, senddata);
	return queue_swd_packet(swd_handle_p, opcode, senddata32, return_ptr);
}

inline EUD_ERR_t process_swd_status(uint32_t &swd_status)
{
#ifdef SWD_DEBUG_ON
	QCEUD_Print("Recieved SWD_CMD_STATUS: %X", swd_status);
#endif
	if ((swd_status & SWD_ACK_MASK) == SWD_ACK_FAULT_MASK)
	{
#ifdef SWD_DEBUG_ON
		QCEUD_Print("Err Recieved SWD_CMD_STATUS: %X", SWD_ERR_SWD_ACK_FAULT_DETECTED);
#endif
		return SWD_ERR_SWD_ACK_FAULT_DETECTED;
	}
	else if ((swd_status & SWD_ACK_MASK) == SWD_ACK_WAIT_MASK)
	{
#ifdef SWD_DEBUG_ON
		QCEUD_Print("Err Recieved SWD_CMD_STATUS: %X", SWD_ERR_SWD_ACK_WAIT_DETECTED);
#endif
		return EUD_SUCCESS;
	}
	else
	{
#ifdef SWD_DEBUG_ON
		QCEUD_Print("All ok Recieved SWD_CMD_STATUS: %X", EUD_SUCCESS);
#endif
		return EUD_SUCCESS;
	}
}
/// Manage buffers and flushing them out if needed.
EXPORT EUD_ERR_t swd_flush_buffers(SwdEudDevice *swd_handle_p, uint32_t flushoption)
{

	EUD_ERR_t err;
	bool status_sent_tempflag, swd_command_tempflag;
	uint32_t swd_status = 0;
	// Queue status/flush packets if required.

#ifdef SWD_DEBUG_ON
	QCEUD_Print("Flush start \n");
	QCEUD_Print("Expected bytes to be Written: %d, Expected bytes to be Read: %d \n", swd_handle_p->eud_queue_p_->raw_write_buffer_idx, swd_handle_p->eud_queue_p_->read_expected_bytes);
#endif

	status_sent_tempflag = swd_handle_p->eud_queue_p_->status_appended_flag_;
	swd_command_tempflag = swd_handle_p->eud_queue_p_->swd_command_flag_;


	queue_swd_packet_special(swd_handle_p, SWD_CMD_STATUS, 0, &swd_status);
	// Write and read bytes if required.
	if (flushoption == 1)
	{
		// QCEUD_Print("Going to flush \n");
		if ((swd_handle_p->eud_queue_p_->read_expected_bytes_ > 0) && (swd_handle_p->eud_queue_p_->read_expected_bytes_ != SWD_IN_BUFFER_SIZE))
		{
#ifdef SWD_DEBUG_ON
			QCEUD_Print("Queue FLUSH cmd, since there are Reads pending \n");
#endif
			queue_swd_packet_special(swd_handle_p, SWD_CMD_FLUSH, 0, 0);
			// readFlush = true;
		}

#ifdef SWD_DEBUG_ON
		QCEUD_Print("Queue NULL cmd \n");
#endif
		// queue_swd_packet_special(swd_handle_p, SWD_CMD_NOP, 0, 0);
#ifdef SWD_DEBUG_ON
		QCEUD_Print(" Call UsbWriteRead()\n");
		QCEUD_Print("Expected bytes to be Written: %d, Expected bytes to be Read: %d \n", swd_handle_p->eud_queue_p->raw_write_buffer_idx, swd_handle_p->eud_queue_p->read_expected_bytes);
#endif

		err = swd_handle_p->UsbWriteRead(swd_handle_p->eud_queue_p_);
	}
	else
	{
		if ((swd_handle_p->eud_queue_p_->read_packet_flag_) || (swd_handle_p->eud_queue_p_->buffer_status_ != QUEUE_NOT_YET_FULL))
		{
			err =
				swd_handle_p->mode_ == IMMEDIATEWRITEMODE ? swd_handle_p->UsbWriteRead(swd_handle_p->eud_queue_p_) : swd_handle_p->mode_ == MANUALBUFFERWRITEMODE ? eud_set_last_error(EUD_SWD_ERR_ITEMQUEUED_BUT_FLUSH_REQUIRED)
																												 : swd_handle_p->mode_ == MANAGEDBUFFERMODE		 ? swd_handle_p->UsbWriteRead(swd_handle_p->eud_queue_p_)
																																								 : eud_set_last_error(EUD_SWD_ERR_UNKNOWN_MODE_SELECTED);
		}
		else
		{
			err =
				swd_handle_p->mode_ == IMMEDIATEWRITEMODE ? swd_handle_p->UsbWriteRead(swd_handle_p->eud_queue_p_) : swd_handle_p->mode_ == MANUALBUFFERWRITEMODE ? EUD_SUCCESS
																												 : swd_handle_p->mode_ == MANAGEDBUFFERMODE		 ? EUD_SUCCESS
																																								 : eud_set_last_error(EUD_SWD_ERR_UNKNOWN_MODE_SELECTED);

			if ((swd_handle_p->mode_ == MANAGEDBUFFERMODE) || (swd_handle_p->mode_ == MANUALBUFFERWRITEMODE))
			{
				return err;
			}
		}
	}

#ifdef SWD_DEBUG_ON
	QCEUD_Print("Flush End \n");
#endif

	if (err != EUD_SUCCESS)
	{
		QCEUD_Print("Flush failed. return value:%d \n", err);
		return err;
	}

	err = process_swd_status(swd_status);

	if (err != EUD_SUCCESS)
	{
		QCEUD_Print("SWD status failure. return value:%d \n", err);
		return err;
	}

	// Reset Queue Status
	swd_handle_p->eud_queue_p_->buffer_status_ = QUEUE_NOT_YET_FULL;

	// Only get here if we read/wrote bytes
	if (status_sent_tempflag)
	{
		swd_handle_p->eud_queue_p_->status_stale_flag_ = false;
		// swd_handle_p->SWDStatusSentFlag = true;
		CurrentSWDStatusCounter = 0;
	}
	else if (swd_command_tempflag)
	{
		swd_handle_p->eud_queue_p_->status_stale_flag_ = true;
		// swd_handle_p->SWDStatusSentFlag = false;
	}
#if 0
	err1= swd_get_status(swd_handle_p, 1, &swd_status);
	if (err1 != EUD_SUCCESS) 
		QCEUD_Print("swd_get_status failed!\n");
	else
		QCEUD_Print("swd_get_status: %x\n",swd_status);
#endif

	return err;
}

EXPORT int swdGetDeviceID(SwdEudDevice *swd_handle_p, uint32_t &deviceID)
{
	return queue_swd_packet_special(swd_handle_p, 0x9, 0, &deviceID);
}
/*************************************************************************************
 *   @brief API to perform an SWD Write.
 *
 *
 *****************************************************************************************/
EXPORT EUD_ERR_t swd_write(SwdEudDevice *swd_handle, uint32_t APnDP, uint32_t A2_3, uint32_t senddata)
{

	// QCEUD_Print("**** calling swd write **** ");

	SwdCmd_t swd_cmd;
	EUD_ERR_t err = EUD_SUCCESS;
	bool requeue_command_flag = false;

	if (swd_handle == NULL)
	{
		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	}

	/* Save the DP register value in the event it is a write to the select register */
	swd_handle->SetDpApSelect(APnDP, A2_3 << 2, SWD_RNW_WRITE_BIT, senddata);
	if ( (err = assemble_swd_packet(&swd_cmd, SWD_RNW_WRITE_BIT, APnDP, A2_3)) != 0 )
		return err;

#ifdef SWD_DEBUG_ON
	QCEUD_Print("Writing register: %s , Write Data : 0x%x \n", swd_handle->GetRegInfo(APnDP, A2_3 << 2, SWD_RNW_WRITE_BIT).c_str(), senddata);
#endif

	if ((err = queue_swd_packet(swd_handle, swd_cmd.cmd, senddata, NULL)) != 0 )
		return err;

	if ((swd_handle->eud_queue_p_->buffer_status_ == QUEUE_FULL_COMMAND_NOT_QUEUED))
	{

#ifdef SWD_DEBUG_ON
		QCEUD_Print("swd_write: Queue is full, calling flush \n");
#endif
		requeue_command_flag = true;
		if ((err = swd_flush_buffers(swd_handle, FLUSH_OPTION_TRUE))!=0)
			return err;
#ifdef SWD_DEBUG_ON
		QCEUD_Print("swd_write: Flush was successfull \n");
#endif
	}

	// Requeue command if the current one was not queued
	if (requeue_command_flag == true)
	{
#ifdef SWD_DEBUG_ON
		QCEUD_Print("swd_write: ReQueuing command: 0x%x\n", swd_cmd.cmd);
#endif
		if ((err = queue_swd_packet(swd_handle, swd_cmd.cmd, senddata, NULL)) != 0 )
			return err;
	}
	return err;
}

EXPORT EUD_ERR_t swd_read(SwdEudDevice *swd_handle, uint32_t APnDP, uint32_t A2_3, uint32_t *returndata)
{
	// QCEUD_Print("swd_read: called\n");

	SwdCmd_t swd_cmd;
	EUD_ERR_t err = EUD_SUCCESS;
	bool requeue_command_flag = false;

	if ((swd_handle == NULL) || (returndata == NULL))
	{

#ifdef SWD_DEBUG_ON
		if (swd_handle == NULL)
			QCEUD_Print("swd_read: swd_handle is NULL\n");
		else
			QCEUD_Print("swd_read: return pointer is NULL\n");
#endif

		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	}

	/* Assemble and queue the packet */
	if ( (err = assemble_swd_packet(&swd_cmd, SWD_RNW_READ_BIT, APnDP, A2_3)) != 0 )
		return err;
	if ( (err = queue_swd_packet(swd_handle, swd_cmd.cmd, (uint32_t)0, returndata)) != 0 )
		return err;

	/* If full, re-queue after a flush*/
	if ((swd_handle->eud_queue_p_->buffer_status_ == QUEUE_FULL_COMMAND_NOT_QUEUED))
	{

#ifdef SWD_DEBUG_ON
		QCEUD_Print("swd_read: Handling Buffer full state: swd_handle->eud_queue_p->bufferstatus: %d", swd_handle->eud_queue_p->buffer_status_);
#endif
		requeue_command_flag = true;


#ifdef SWD_DEBUG_ON
		QCEUD_Print("\n Buffering changes: SWD Read: Queue is full, hence calling Flush \n");
		QCEUD_Print("\n SWD Read:  calling Flush \n");
#endif
		if ((err = swd_flush_buffers(swd_handle, FLUSH_OPTION_TRUE))!=0)
			return err;
	}

	// Requeue command if the current one was not queued
	if (requeue_command_flag == true)
	{
		if ((err = queue_swd_packet(swd_handle, swd_cmd.cmd, (uint32_t)0, returndata))!=0)
			return err;
	}

#ifdef SWD_DEBUG_ON
	QCEUD_Print("\n Read Api done. Reads Buffered \n");
	QCEUD_Print("\n Reading register: %s , Read Data : 0x%x \n", swd_handle->GetRegInfo(APnDP, A2_3 << 2, SWD_RNW_READ_BIT).c_str(), *returndata);
#endif

	return err;
}

EXPORT EUD_ERR_t set_buf_mode(SwdEudDevice *swd_handle_p, uint32_t mode)
{
	EUD_ERR_t err; 
	if (mode > MAX_BUFFER_MODES)
		return EUD_ERR_BAD_PARAMETER;
	if ((err = swd_handle_p->SetBufMode(static_cast<uint32_t>(mode))) != 0)
		return err;

	return EUD_SUCCESS;
}

EXPORT EUD_ERR_t get_buf_mode(SwdEudDevice *swd_handle_p, uint32_t *bufmode_p)
{

	*bufmode_p = (uint32_t)swd_handle_p->GetBufMode();

	return EUD_SUCCESS;
}

EXPORT EUD_ERR_t swd_flush(SwdEudDevice *swd_handle_p)
{
	if (swd_handle_p == NULL)
	{
		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	}

	return swd_flush_buffers(swd_handle_p, FLUSH_OPTION_TRUE);
}

EXPORT EUD_ERR_t swd_nop(SwdEudDevice *swd_handle_p)
{

	return EUD_SWD_ERR_FUNC_NOT_IMPLEMENTED;
}

EXPORT EUD_ERR_t swd_bitbang(SwdEudDevice *swd_handle_p, uint32_t SWDBitValues, uint32_t *returndata)
{

#ifdef SWD_DEBUG_ON
	QCEUD_Print("swd BitBang, bitval: %ld \n", SWDBitValues);
#endif
	EUD_ERR_t err = 0;

	if (swd_handle_p == NULL)
	{
		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	}

	if (SWDBitValues & SWDBITBANG_VALIDVALUE_MSK)
		return eud_set_last_error(EUD_ERR_BAD_PARAMETER);

	// If we have pending data to be sent out to the SWD peripheral, send it out and clear out the queue first
	if ( (err = swd_flush_buffers(swd_handle_p, FLUSH_OPTION_TRUE)) != 0)
		return err;

		// workaround
		// return swd_handle_p->WriteCommand(SWD_CMD_BITBANG, SWDBitValues, returndata);
#if 0
	uint8_t *outbuf = new uint8_t[6];
	outbuf[0] = SWD_CMD_BITBANG;
	pack_uint32_to_uint8_array2(SWDBitValues, &outbuf[1]);
	outbuf[5] = SWD_CMD_FLUSH;
	swd_handle_p->usb_write(outbuf, 6);
	err = swd_handle_p->usb_read(4, (uint8_t *)returndata);
#endif

	queue_swd_packet(swd_handle_p, SWD_CMD_BITBANG, (uint32_t)SWDBitValues, returndata);
	err = swd_handle_p->UsbWriteRead(swd_handle_p->eud_queue_p_);

	// QCEUD_Print("BitBang operation: Cmd: %x Data write: %x Ack data: %x \n", outbuf[0], outbuf[1], *returndata);
	swd_handle_p->flush_required_ = false;
	return err;
}

uint8_t outbuf[32] = {0};
uint32_t bitbang_bytecnt = 0;

EXPORT EUD_ERR_t SWDBitBangPack(SwdEudDevice* swd_handle_p, uint32_t SWDBitValues, uint32_t* returndata, bool force_flush) {

	EUD_ERR_t err = 0;
	uint8_t* inbuf = (uint8_t*) returndata;

	outbuf[bitbang_bytecnt] = SWD_CMD_BITBANG;
	pack_uint32_to_uint8_array2(SWDBitValues, &outbuf[bitbang_bytecnt+1]);
	bitbang_bytecnt += 5;

	if ( (bitbang_bytecnt + 5 > 32) || force_flush )
	{
#if 1
		swd_handle_p->UsbWriteRead(bitbang_bytecnt, outbuf, 32, inbuf);
#else
		//outbuf[bitbang_bytecnt]   = SWD_CMD_FLUSH;
		//outbuf[bitbang_bytecnt+1] = SWD_CMD_NOP;
		//swd_handle_p->usb_write_timeout(bitbang_bytecnt+2, outbuf, 1000);
		swd_handle_p->usb_write_timeout(bitbang_bytecnt, outbuf, 1000);
		swd_handle_p->usb_read_timeout(32, inbuf, 100);
#endif
		memset(outbuf, 0, 32);
		bitbang_bytecnt = 0;
	}

	//QCEUD_Print("BitBang operation: Cmd: %x Data write: %x Ack data: %x \n", outbuf[0], outbuf[1], *returndata);

	return err;
}


EXPORT EUD_ERR_t send_data_via_bitbang(SwdEudDevice* swd_handle_p, uint64_t SWDBitValues, uint32_t numBits) {
	EUD_ERR_t err = 0;
	uint8_t swd_rctlr_srst_n = 1;
    uint8_t swd_gpio_di_oe = 1;
    uint8_t swd_gpio_srst_n = 1;
    uint8_t swd_gpio_trst_n = 1;
    uint8_t swd_dap_trst_n = 1;
	int32_t idx = 0;
	uint32_t *returndata = new uint32_t[8];
	uint8_t bit = 0;
	uint64_t value = 0;
	bool force_flush = 0;

	for (idx = numBits-1; idx >= 0 ; idx --) {
		bit = (SWDBitValues >> idx) & 0x1;
		if (idx == 0) {
			force_flush = 1;
		} else {
			force_flush = 0;
		}

		SWDBitBangPack (swd_handle_p, (1 << 0) | (bit << 1) | (swd_rctlr_srst_n << 2) | (swd_gpio_di_oe << 3) | (swd_gpio_srst_n << 4) | (swd_gpio_trst_n << 5) | (swd_dap_trst_n << 6), returndata, 0);
		SWDBitBangPack (swd_handle_p, (0 << 0) | (bit << 1) | (swd_rctlr_srst_n << 2) | (swd_gpio_di_oe << 3) | (swd_gpio_srst_n << 4) | (swd_gpio_trst_n << 5) | (swd_dap_trst_n << 6), returndata, force_flush);

		value += (bit << idx);
	}
	delete [] returndata;
	return err;
}

EXPORT EUD_ERR_t swd_di_tms(SwdEudDevice *swd_handle_p, uint32_t SWD_DITMSValue, uint32_t count)
{

	if (swd_handle_p == NULL)
	{
		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	}
	if (SWD_DITMSValue > SWD_CMD_DITMS_VALUE_MAX)
	{
		return eud_set_last_error(EUD_SWD_ERR_BAD_DI_TMS_PARAMETER);
	}
	if (count > SWD_CMD_DITMS_COUNT_MAX)
	{
		return eud_set_last_error(EUD_SWD_ERR_BAD_DI_TMS_PARAMETER);
	}

	uint8_t swd_di_tms_payload[SWD_PCKT_SZ_SEND_DITMS - 1] = {
		(uint8_t)(SWD_DITMSValue & 0xFF),
		(uint8_t)((SWD_DITMSValue >> 8) & 0xFF),
		(uint8_t)(count & 0xFF),
		(uint8_t)((count >> 8) & 0xFF)};

	return swd_handle_p->WriteCommand(SWD_CMD_DITMS, swd_di_tms_payload);
}
//=====================================================================================//
//                      SWD Configuration Commands/Queries
//
//                      These SWD operations are immediately written out.
//                      There is no buffering options available.
//=====================================================================================//

EXPORT EUD_ERR_t swd_timing(SwdEudDevice *swd_handle_p, uint32_t retrycount, uint32_t turnarounddelay, uint32_t wait_35_if_err)
{

	if (swd_handle_p == NULL)
	{
		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	}

	if (((uint32_t)retrycount > SWD_CMD_TIMING_RETRYCOUNT_MAX) != 0)
	{
		return EUD_ERR_BAD_PARAMETER;
	}

	if (((uint32_t)turnarounddelay > SWD_CMD_TIMING_TURNAROUNDDELAY_MAX) != 0)
	{
		return EUD_ERR_BAD_PARAMETER;
	}
	wait_35_if_err &= 1;

	// Bit fields:
	//	bits[15:0] Retry count.Default = 3.
	//	bits[17:16] Turn - around time TIMING(TRN).Default = 0.
	//	bit[18] wait_35_if_err
	//	bits[31:19] reserved

	uint8_t byte3 = 0;
	byte3 += wait_35_if_err ? (1 << SWD_CMD_TIMING_WAIT_35_IF_ERR_SHIFT) : 0;
	byte3 += turnarounddelay & SWD_CMD_TIMING_TURNAROUNDDELAY_MASK;

	uint8_t payload[SWD_PCKT_SZ_SEND_TIMING - 1] = {
		(uint8_t)(retrycount & 0xFF),
		(uint8_t)((retrycount >> 8) & 0xFF),
		byte3,
		0x0};

	return swd_handle_p->WriteCommand(SWD_CMD_TIMING, payload);
}

EXPORT EUD_ERR_t swd_set_frequency(SwdEudDevice *swd_handle_p, uint32_t frequency)
{

	if (swd_handle_p == NULL)
	{
		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	}

	if ((frequency > SWD_CMD_FREQ_MAX) != 0)
	{
		return EUD_ERR_BAD_PARAMETER;
	}

	return swd_handle_p->WriteCommand(SWD_CMD_FREQ, frequency);
}

EXPORT EUD_ERR_t swd_set_delay(SwdEudDevice *swd_handle_p, uint32_t delaytime)
{

	if (swd_handle_p == NULL)
	{
		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	}

	if ((delaytime > SWD_CMD_DELAY_MAX) != 0)
	{
		return EUD_ERR_BAD_PARAMETER;
	}

	return swd_handle_p->WriteCommand(SWD_CMD_DELAY, delaytime);
}

EXPORT EUD_ERR_t swd_peripheral_reset(SwdEudDevice *swd_handle_p)
{

	EUD_ERR_t err = EUD_SUCCESS;
#ifdef SWD_DEBUG_ON
	QCEUD_Print("Attempting SWD Peripheral Reset entry.");
#endif
	if (swd_handle_p == NULL)
	{
		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	}

// INFO << "Attempting SWD Peripheral Reset write command.";
#ifdef SWD_DEBUG_ON
	QCEUD_Print("Attempting SWD Peripheral Reset write command.");
#endif
	if ((err = swd_handle_p->WriteCommand(SWD_CMD_PERIPH_RST))!= 0)
		return err;

#ifdef SWD_DEBUG_ON
	QCEUD_Print("Attempting SWD Peripheral Reset cleanup packet queue.");
#endif
	swd_handle_p->CleanupPacketQueue(swd_handle_p->eud_queue_p_);

#ifdef SWD_DEBUG_ON
	QCEUD_Print("done SWD Peripheral Reset cleanup packet queue.");
#endif
	swd_handle_p->swd_to_jtag_operation_done_ = FALSE;

	return EUD_SUCCESS;
}

// #define APPEND_STATUS_IN_FLUSH 1
EXPORT EUD_ERR_t swd_get_status(SwdEudDevice *swd_handle_p, uint32_t forcestatusflush_flag, uint32_t *status_p)
{

	if (!SWDStatusMaxCount)
	{
		return eud_set_last_error(EUD_SWD_ERR_STATUS_COUNT_0);
	}

	// if (!SWD_APPEND_SWDSTATUS_FLAG)
	//     return 0;

	// FIXME - shouldn't have to ignore getstatus if SWD_APPEND_SWDSTATUS_FLAG is 0
	// if (!SWD_APPEND_SWDSTATUS_FLAG)
	//     return 0;

	EUD_ERR_t err = 0;

	if (swd_handle_p == NULL)
	{
		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	}

	if (swd_handle_p->eud_queue_p_->status_stale_flag_ == false)
	{
		*status_p = swd_handle_p->swd_last_status_;
		return EUD_SUCCESS;
	}
	// This case should happen often: status is only updated after a flush. If status is requested after every transaction,
	//   and transactions are only sent to bus after flushes, this return value would be sent if there are pending items in
	//   swd queue and so swd_last_status_ is no longer relevant.
	else if (!forcestatusflush_flag)
	{
		*status_p = swd_handle_p->swd_last_status_;
		return SWD_WARN_SWDSTATUS_NOT_UPDATED;
	}
	else
	{

		if ( (err = swd_flush_buffers(swd_handle_p, 1)) !=0 )
			return err;
		// Write status opcode and flush
		// TODO - make  sure  all fields are reset.
		queue_swd_packet(swd_handle_p, SWD_CMD_STATUS, (uint32_t)0, &swd_handle_p->swd_last_status_);
		queue_swd_packet(swd_handle_p, SWD_CMD_FLUSH, (uint32_t)0, NULL);

		if ((err = swd_flush_buffers(swd_handle_p, 1))!=0)
			return err;

		CurrentSWDStatusCounter = 0;

		*status_p = swd_handle_p->swd_last_status_;

		if ((swd_handle_p->swd_last_status_ & SWD_ACK_MASK) == SWD_ACK_FAULT_MASK)
		{
			return eud_set_last_error(SWD_ERR_SWD_ACK_FAULT_DETECTED);
		}
		else if ((swd_handle_p->swd_last_status_ & SWD_ACK_MASK) == SWD_ACK_WAIT_MASK)
		{
			return eud_set_last_error(SWD_ERR_SWD_ACK_WAIT_DETECTED);
		}

		return err;
	}
}

EXPORT EUD_ERR_t swd_get_status_string(SwdEudDevice *swd_handle_p, char *stringbuffer_p, uint32_t *sizeofstringbuffer_p)
{
	return EUD_SWD_ERR_FUNC_NOT_IMPLEMENTED;
}

EXPORT EUD_ERR_t swd_flush_required(SwdEudDevice *swd_handle_p, uint8_t *flushreq_p)
{

	return EUD_SWD_ERR_FUNC_NOT_IMPLEMENTED;
}

/*
//reset should set mode to default, as well as frequency and buffers etc.
*/

EXPORT EUD_ERR_t jtag_to_swd_adiv5(SwdEudDevice *swd_handle_p)
{

	EUD_ERR_t err = EUD_SUCCESS;

	if (swd_handle_p == NULL)
	{
		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	}

	// if (swd_handle_p->SWD_to_JTAG_operation_done == TRUE){
	//	return eud_set_last_error(EUD_SWD_ERR_SWD_TO_JTAG_ALREADY_DONE);
	// }

	if ((err = swd_di_tms(swd_handle_p, 0xFFFF, 0x32)) != EUD_SUCCESS)
		return err;
	// uint8_t swd_di_tms2[] = { 0x9E, 0xE7, 0x0F, 0x00 }; //Note that these are reverse endian
	if ((err = swd_di_tms(swd_handle_p, 0xE79E, 0x0F)) != EUD_SUCCESS)
		return err;
	// uint8_t swd_di_tms3[] = { 0xFF, 0xFF, 0x32, 0x00 }; //Note that these are reverse endian
	if ((err = swd_di_tms(swd_handle_p, 0xFFFF, 0x32)) != EUD_SUCCESS)
		return err;

	swd_handle_p->swd_to_jtag_operation_done_ = TRUE;

	return EUD_SUCCESS;
}

EXPORT EUD_ERR_t jtag_to_swd_adiv6(SwdEudDevice *swd_handle_p)
{
	EUD_ERR_t err = EUD_SUCCESS;

	if (swd_handle_p == NULL)
	{
		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	}

	//cycles TMS high
	err = send_data_via_bitbang(swd_handle_p, 0x1F, 0x05);
	if (err != EUD_SUCCESS) return err;

	//Activation sequence
	err = send_data_via_bitbang(swd_handle_p, 0x2EEEEEE6, 31);
	if (err != EUD_SUCCESS) return err;

	//Leave dormant state
    // -----------------------------
    // 8 cycles TMS high
	err = send_data_via_bitbang(swd_handle_p, 0xFF, 8);
	if (err != EUD_SUCCESS) return err;

    //send_data_via_bitbang(swd_handle_p, 0x49CF9046A9B4A16197F5BBC745703D98, 128);      // MSB first
	send_data_via_bitbang(swd_handle_p, 0x49CF9046A9B4A161, 64);      // MSB first

	send_data_via_bitbang(swd_handle_p, 0x97F5BBC745703D98, 64);      // MSB first

    // 4 cycles TMS low
    send_data_via_bitbang(swd_handle_p, 0x0, 4);

    // CoreSight SW-DP Activation Code
    send_data_via_bitbang(swd_handle_p, 0x58, 8);      // MSB first

    // Reset
    send_data_via_bitbang(swd_handle_p, 0xFFFFFFFFFFFFC, 52);

	swd_handle_p->swd_to_jtag_operation_done_ = TRUE;

	return EUD_SUCCESS;
}


// EXPORT EUD_ERR_t swd_get_jtag_id(SwdEudDevice* swd_handle_p, uint32_t* jtagid_p){
EXPORT EUD_ERR_t swd_get_jtag_id(SwdEudDevice *swd_handle_p, uint32_t *jtagid_p)
{
	// check if jtag_to_swd operation done already.

	EUD_ERR_t err = EUD_SUCCESS;

	if (swd_handle_p == NULL)
	{
		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	}

	if (jtagid_p == NULL)
		return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
	// Check that user has done SWD_to_JTAG first
	// Could just do it ourselves...
	if (swd_handle_p->swd_to_jtag_operation_done_ == FALSE)
	{
		return eud_set_last_error(EUD_SWD_ERR_JTAGID_REQUESTED_BEFORE_STOJDONE);
	}

	// 0xa501 gets jtag id
	// EXPORT EUD_ERR_t SWDCMD(SwdEudDevice* swd_handle_p, uint32_t SWD_CMD, uint8_t* RnW,
	// uint8_t* APnDP, uint8_t* A2_3, uint32_t* data, uint32_t count)

	// need to change mode to immediate r/w, then restore it to original
	//uint32_t RnW = 1;
	uint32_t APnDP = 0;
	uint32_t A2_3 = 0;
	//uint32_t send_data = 0;
	//uint32_t count = 1;

	// uint8_t* jtag_8bit_id = new uint8_t[4];
	// memset(jtag_8bit_id, 0x0, 4);
	uint32_t currentmode = swd_handle_p->mode_;

	swd_handle_p->mode_ = IMMEDIATEWRITEMODE;
	// if (err = set_buf_mode(swd_handle_p, IMMEDIATEWRITEMODE)) return err; //We want this to be immediately written out.

	if ((err = swd_read(swd_handle_p, APnDP, A2_3, jtagid_p))!=0)
		return err;

	// if (err = set_buf_mode(swd_handle_p, currentstate)) return err; //Restore original
	swd_handle_p->mode_ = currentmode;

#ifdef old_swdcmd
	*jtagid_p = 0;
	*jtagid_p = EUDHandler_p->USB_Read_Buffer[0];
	*jtagid_p += EUDHandler_p->USB_Read_Buffer[1] << 8;
	*jtagid_p += EUDHandler_p->USB_Read_Buffer[2] << 16;
	*jtagid_p += EUDHandler_p->USB_Read_Buffer[3] << 24;
#endif
	// delete jtag_8bit_id;
	if (*jtagid_p == 0x0)
		return eud_set_last_error(EUD_SWD_ERR_JTAGID_NOT_RECEIVED);
	// if (*jtagid_p == 0x0) return eud_set_last_error(3);

	return EUD_SUCCESS;
}

EXPORT EUD_ERR_t swd_line_reset(SwdEudDevice *swd_handle_p)
{
	return EUD_SWD_ERR_FUNC_NOT_IMPLEMENTED;
}
