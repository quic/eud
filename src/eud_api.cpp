/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   eud_api.cpp
*
* Description:                                                              
*   CPP source file for EUD Peripheral's public API's
*
***************************************************************************/

#include <math.h>

#define EUD_DEVICE_DECL 1

#include "eud_top_defines_internal.h"
#include "eud_types.h"
#include "eud.h"
#include "com_eud.h"
#include "ctl_eud.h"
#include "jtag_eud.h"
#include "swd_eud.h"
#include "trc_eud.h"
#include "eud_api.h"
#include "device_manager.h"
#include "usb.h"

std::string uint32_to_ascii(uint32_t number){
    if (number > 9999){
        return "0000";
    }
    uint32_t temp = 0;

    std::string charstring = std::string("");

    if (number > 1000){
        temp = ((number - temp) % 10000) / 1000;
        charstring += ((char)(temp + ASCII_0));
    }
    else if (number == 1000){
        charstring += "1";
    }
    else{
        charstring += "0";
    }

    if (number > 100){
        temp = ((number - temp) % 1000) / 100;
        charstring += ((char)(temp + ASCII_0));
    }
    else if (number == 100){
        charstring += "1";
    }
    else{
        charstring += "0";
    }

    if (number > 10){
        temp = ((number - temp) % 100) / 10;
        charstring += ((char)(temp + ASCII_0));
    }
    else if (number == 10){
        charstring += "1";
    }
    else{
        charstring += "0";
    }

    temp = number % 10;
    charstring += ((char)temp + ASCII_0);

    return charstring;

}


uint32_t ascii_to_int(std::string asciinum){
    uint32_t number, idx;
    number = 0;
    uint32_t tempnum;

    if (asciinum.length() > MAX_DIGIT_ASCII_TO_INT){
        return 0;//Error
    }

    for (idx = 0; idx < asciinum.length(); idx++)
    {
        std::string temp = asciinum;
        temp.erase((temp.length() - idx - 1), (temp.length() - idx));

        tempnum = (int)(*temp.c_str());

        //Check that we're within ascii bounds.
        if ((tempnum < (int)('0')) || (tempnum >(int)('9'))){
            return 0; //Error.
        }

        tempnum -= (int)('0');

        number += tempnum * (uint32_t)pow(10, idx);

    }

    return number;
}

/****************************************************************
*   @brief Execute the reset sequence to begin communication with
           the debug port.
****************************************************************/
EXPORT EUD_ERR_t assert_reset_sequence( EudDevice* eud_handle ) 
{
    EudDevice *dev_handle = (EudDevice *)eud_handle;

    if (dev_handle == NULL)
        return EUD_ERR_NULL_POINTER;

    /* Call the reset sequence which is overridden by the type of the peripheral */
    if (dev_handle->device_type_ == DEVICETYPE_EUD_SWD)
        return ((SwdEudDevice *)dev_handle)->AssertReset();
    else
        return EUD_GENERIC_FAILURE;

}


/************************************************************************
*   @brief Execute the trailing portion of the reset sequence to 
           begin communication with the debug port after reset is de-asserted
****************************************************************/
EXPORT EUD_ERR_t deassert_reset_sequence(EudDevice* eud_handle)
{
    EudDevice *dev_handle = (EudDevice *)eud_handle;

    if (dev_handle == NULL)
        return EUD_ERR_NULL_POINTER;

    /* Call the reset sequence which is overridden by the type of the peripheral */
    if (dev_handle->device_type_ == DEVICETYPE_EUD_SWD)
        return ((SwdEudDevice *)dev_handle)->DeAssertReset();
    else
        return EUD_GENERIC_FAILURE;

}
