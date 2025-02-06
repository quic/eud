/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   com_eud.h
*
* Description:                                                              
*   Header file for EUD COM Port APIs declerations and various header 
*   information
*
***************************************************************************/
#ifndef COM_EUD_H_
#define COM_EUD_H_

#include "eud.h"

typedef int EUD_ERR_t;

#define COM_CMD_NOP         0
#define COM_CMD_TX_TMOUT    1
#define COM_CMD_RX_TMOUT    2
#define COM_CMD_PORT_RESET  3


///////////////Packet size definitions//////////////////

#define COM_PCKT_SZ_SEND_NOP        1
#define COM_PCKT_SZ_SEND_TX_TMOUT   5
#define COM_PCKT_SZ_SEND_RX_TMOUT   5
#define COM_PCKT_SZ_SEND_PORT_RST   1


#define COM_PCKT_SZ_RECV_NOP        0
#define COM_PCKT_SZ_RECV_TX_TMOUT   0
#define COM_PCKT_SZ_RECV_RX_TMOUT   0
#define COM_PCKT_SZ_RECV_PORT_RST   0


#define COM_ENDIANOPT_NOP           BIGENDIAN
#define COM_ENDIANOP_TX_TMOUT       BIGENDIAN
#define COM_ENDIANOP_RX_TMOUT       BIGENDIAN
#define COM_ENDIANOP_PORT_RST       BIGENDIAN


#define COM_NUM_OPCODES             4

#define INIT_COM_ALLOWED_OPTIONS_MSK 0 ///<Initialization options mask. No options allowed at this time.

#ifdef __cplusplus
class ComEudDevice: public EudDevice
{
public:

    ComEudDevice(); //constructor

    virtual EUD_ERR_t Init();

    void ResetCounters();

private:

};
#else  // __cplusplus
typedef void ComEudDevice; 
#endif // __cplusplus

#endif //COM_EUD_H_

