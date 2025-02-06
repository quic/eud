/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   arm_dap.h
*
* Description:                                                              
*   Header file for ARM DAP register structures
*
***************************************************************************/
#ifndef ARM_DAP_H_
#define ARM_DAP_H_

/* Total number of DP registers as in DPv2 protocol */
#define DP_MAX_REGISTERS_v2     12
#define AP_MAX_REGISTERS_v2     13

/* Debug Port Registers */
/* Abort Register*/
typedef struct {
    uint32_t apabort : 1;
    uint32_t stkcmpclr : 1;
    uint32_t stkerrclr : 1;
    uint32_t wderrclr : 1;
    uint32_t orunerrclr : 1;
    uint32_t res0 : 27;
}_dp_abort_reg_t;

/* Control / Stat register */
typedef struct {
    uint32_t csyspwrupack : 1;
    uint32_t csyspwrupreq : 1;
    uint32_t cdbgpwrupack : 1;
    uint32_t cdbgpwrupreq : 1;
    uint32_t cdbgrstack   : 1;
    uint32_t cdbgrstreq   : 1;
    uint32_t res0 : 2;
    uint32_t trncnt : 12;
    uint32_t masklane : 4;
    uint32_t wdataerr : 1;
    uint32_t readok : 1;
    uint32_t stickyerr : 1;
    uint32_t stickcmp : 1;
    uint32_t trnmode : 2;
    uint32_t stickorun : 1;
    uint32_t orundetect : 1;
}_dp_ctrlstat_reg_t;

/* Data Link Control Register */
typedef struct {
    uint32_t res0 : 6;
    uint32_t res1 : 2;
    uint32_t turnaround : 2;
    uint32_t res2 : 22;
}_dp_dlcr_reg_t;

/* Data Link Protocol Identification Register */
typedef struct {
    uint32_t protvsn : 4;
    uint32_t res0 : 24;
    uint32_t tinstance : 4;
}_dp_dlipr_reg_t;

/* Debug Port Identification Register */
typedef struct {
    uint32_t ra0 : 1;
    uint32_t designer : 11;
    uint32_t version : 4;
    uint32_t min : 1;
    uint32_t res0 : 3;
    uint32_t partno : 8;
    uint32_t revision : 4;
}_dp_dpidr_reg_t;

/* Event Status register */
typedef struct {
    uint32_t ea : 1;
    uint32_t res0 : 31;
}_dp_eventstat_reg_t;

/* Read Buffer Register */
typedef struct {
    uint32_t data : 32;
}_dp_rdbuff_reg_t;

/* Read Resend Register */
typedef struct {
    uint32_t data : 32;
}_dp_resend_reg_t;

/* AP Select register */
typedef struct {
    uint32_t dpbanksel : 4;
    uint32_t apbanksel : 4;
    uint32_t res0 : 16;
    uint32_t apsel : 8;
}_dp_select_reg_t;

/* Target Identification Register */
typedef struct {
    uint32_t ra0 : 1;
    uint32_t tdesigner : 11;
    uint32_t tpartno : 16;
    uint32_t trevision : 4;
}_dp_targetid_reg_t;

/* Target Select Register */
typedef struct {
    uint32_t ra0 : 1;
    uint32_t tdesigner : 11;
    uint32_t tpartno : 16;
    uint32_t tinstance : 4;
}_dp_targetsel_reg_t;

typedef union
{
    uint32_t val;
    _dp_abort_reg_t dp_abort_reg;
    _dp_ctrlstat_reg_t dp_ctrl_stat_reg;
    _dp_dlcr_reg_t dp_dlcr_reg;
    _dp_dlipr_reg_t dp_dlipr_reg;
    _dp_dpidr_reg_t dp_dpidr_reg;
    _dp_eventstat_reg_t dp_eventstat_reg;
    _dp_rdbuff_reg_t dp_rdbuff_reg;
    _dp_resend_reg_t dp_resend_reg;
    _dp_select_reg_t dp_select_reg;
    _dp_targetid_reg_t dp_targetid_reg;
    _dp_targetsel_reg_t dp_targetsel_reg;
}dp_reg_t;

/* Access Port Registers */

/* Control / Status Word Register */
typedef struct {
    uint32_t size : 3; 
    uint32_t res0 : 1;
    uint32_t addrinc : 2;
    uint32_t deviceen : 1;
    uint32_t trinprog : 1;
    uint32_t mode : 4;
    uint32_t type : 4;
    uint32_t res1 : 7;
    uint32_t spiden : 1;
    uint32_t prot : 7;
    uint32_t dbgswenable : 1;
}_ap_csw_reg_t;

/* Transfer Address Registers (Low and High 32) */
typedef struct {
    uint32_t addr;
}_ap_tarlo_reg_t;

typedef struct {
    uint32_t addr;
}_ap_tarhi_reg_t;

/* Data Read / Write Register */
typedef struct {
    uint32_t data;
}_ap_drw_reg_t;

/* Banked Data registers */
typedef struct {
    uint32_t data;
}_ap_bd_reg_t;

/* Memory Barrier Transfer Register */
typedef struct {
    uint32_t data;
}_ap_mbt_reg_t;

/* Debug Base address register (High 32)*/
typedef struct {
    uint32_t base;
}_ap_basehi_reg_t;

/* Configuration Register */
typedef struct {
    uint32_t be : 1;
    uint32_t la : 1;
    uint32_t ld : 1;
    uint32_t res0 : 29;
}_ap_cfg_reg_t;

/* Debug Base address register (Low 32) */
typedef struct {
    uint32_t base;
}_ap_baselo_reg_t;

/* ID register */
typedef struct {
    uint32_t type : 4;
    uint32_t variant : 4;
    uint32_t res0 : 5;
    uint32_t apclass : 4;
    uint32_t designer : 7;
    uint32_t id : 4;
    uint32_t revision : 4;
}_ap_idr_reg_t;

typedef union {
    uint32_t val;
    _ap_csw_reg_t ap_csw_reg;
    _ap_tarlo_reg_t ap_tarlo_reg;
    _ap_tarhi_reg_t ap_tarhi_reg;
    _ap_drw_reg_t ap_drw_reg;
    _ap_bd_reg_t ap_bd_reg[4];
    _ap_mbt_reg_t ap_mbt_reg;
    _ap_basehi_reg_t ap_basehi_reg;
    _ap_cfg_reg_t ap_cfg_reg;
    _ap_baselo_reg_t ap_baselo_reg;
    _ap_idr_reg_t ap_idr_reg;
}ap_reg_t;

#endif /* ARM_DAP_H_*/
