/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   eud.h
*
* Description:                                                              
*   Header files with top level declerations for EUD
*
***************************************************************************/
#ifndef EUD_H_
#define EUD_H_

#include "eud_top_defines_internal.h"
#include "debugger_defines.h"
#include "eud_error_defines.h"
#include "eud_types.h"
#include "usb.h"
#include "device_manager_defines.h"
#include <stdlib.h>

#ifdef __cplusplus
#include <string>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if defined ( EUD_WIN_ENV )
#include <wtypes.h>
#include <windows.h>
#include <conio.h>
#include <winioctl.h>

#ifdef __cplusplus
extern "C" {
#endif
__declspec(dllexport) HRESULT DllRegisterServer(void);
__declspec(dllexport) HRESULT DllUnregisterServer(void);
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);
#ifdef __cplusplus
}
#endif 
#endif


//===---------------------------------------------------------------------===//
//
//                      Top level defines, typedef's
//
//===---------------------------------------------------------------------===//
typedef int32_t USB_ERR_t;


#define EUD_DEVICE_DECL 1

///DeviceID enumerations
#define DEVICETYPE_EUD_NULL       0 ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_CTL        1  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_JTG        2  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_SWD        3  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_TRC        4  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_COM        5  ///<DeviceID defines (represented as uint32_t). DeviceID populated by EUD software, used to identify EUD device type. User should not modify this field.
#define DEVICETYPE_EUD_MAX        DEVICETYPE_EUD_COM

/////////////////////EUD Various defines///////////////////////
#define EUD_GENERIC_MAX_NUM_OPCODES     50  ///<Used for peripheral payload size tables for various EUD peripherals.
                                            ///<Up to that peripheral's init function to populate.

#define ENABLE                          1   ///<Used by get_set_and_clear_bits to determine on/off toggling of EUD peripherals.
#define DISABLE                         0   ///<Used by get_set_and_clear_bits to determine on/off toggling of EUD peripherals.

#define LITTLEENDIAN                    1
#define BIGENDIAN                       0

#define USB_OUT_BUFFER_SIZE             1024 

#define SWD_OUT_BUFFER_SIZE             32  ///<Buffer USB Write Bytes. Same size as EUD Hardware FIFO buffer.
#define SWD_IN_BUFFER_SIZE              32  ///<Default size for USB Read Buffer size. 
                                            ///<Hardware won't return value greater than 32 Bytes. 
                                            ///<Effective size may vary depending on Mode.

#define EUD_PACKET_QUEUE_SIZE           32

#define EUD_GENERIC_USB_OUT_BUFFER_SIZE 32
#define EUD_GENERIC_USB_IN_BUFFER_SIZE  32

#define DEFAULT_USB_READ_BUFFER_VECTOR_SIZE 1024

#define USB_READ_BUFFER_OVERFLOW_SIZE 256

#define MAX_DIGIT_ASCII_TO_INT 4

//Other init defines
#define EUD_NULL_DEVMGR_NAME            "NULL"

#define EUD_NULL_DEVID                  0

#define BUFMODEDECL                     1       ///<To prevent double declarations in eud_api.h

#define MAXUSBWAIT                      1

#define DEFAULT_USB_OPEN_WAIT           20      ///<Wait time passed to UsbOpen for default case

#define MAX_FILE_PATH_NAME              4096    ///<Maximum  file path. Used with CurrentWorkingDir

#define EUD_EXECUTABLE_NAME             "eud.dll"
/// Buffer mode defines. Consuming functions use uint32_t as type.
/// Indicates buffering mode for EUD Software. See SetBufMode for more details
#define MAX_BUFFER_MODES 3
#define MANAGEDBUFFERMODE 0
#define IMMEDIATEWRITEMODE 1
#define MANUALBUFFERWRITEMODE 2

#define EUD_TRANS_QUEUE_OPCODEPAYLOAD_SZ 5

extern char* CurrentWorkingDir;

typedef struct _eud_opcode_transaction{
    uint8_t opcode_payload_[EUD_TRANS_QUEUE_OPCODEPAYLOAD_SZ];
    uint32_t payload_size_;
    uint32_t response_size_;
    //uint8_t opcode;
    //uint32_t payload;
    uint32_t* return_ptr_;

} eud_opcode_transaction;

#ifdef  __cplusplus
class EudPacketQueue{
public:
    EudPacketQueue();
    ~EudPacketQueue();
    //eud_opcode_transaction transaction_queue[SWD_OUT_BUFFER_SIZE];
    eud_opcode_transaction transaction_queue_[EUD_PACKET_QUEUE_SIZE];
    uint8_t* raw_write_buffer_;      ///<Store bytes to be written to USB
    uint8_t* raw_read_buffer_;       ///<Store bytes to be read back from USB
    uint32_t raw_write_buffer_idx_;  ///<Index for write buffer.
    uint32_t raw_read_buffer_idx_;   ///<Index for read buffer

    bool read_packet_flag_;          ///<Flag if a SWD command read has been done. 
                                    ///<Used to determine if flush should be executed.
    bool swd_command_flag_;          ///<Flag if a swd command is queued in general. 
                                    ///<Used to distinguish between swd eud opcodes and swd commands queued.

    bool status_appended_flag_;      ///<Used to mark if a status packet has been appended to the packet queue.

    bool flush_appended_flag_;       ///<Used to mark if a flush packet has been appended to the packet queue.

    bool status_stale_flag_;         ///<Used to mark if status value is new or old.

    //uint32_t write_buffer_count;  ///<Keeps  track of bytes out so that write buffer doesn't overflow
    //uint32_t read_buffer_count;
    uint32_t read_expected_bytes_;   ///<Keeps track of how many bytes to read in during full read.

    
    int buffer_status_;
    
    uint32_t queue_index_;

};
#else
typedef void EudPacketQueue;
#endif

//===---------------------------------------------------------------------===//
//
//                  Function Declarations and  Classes
//
//===---------------------------------------------------------------------===//

/****************************************************************************//**
*   @brief EUD device class - abstract class for other EUD Peripherals to inherit.
*   Should never be instantiated as is.
*   Contains linkages to USB driver class as well as other various methods and defines 
*   generic to EUD peripheral devices
*
*********************************************************************************/
#ifdef  __cplusplus
class EudDevice
{
public:
    
    
    
    EudDevice(){ ///<Constructor
        Init();
    }

    virtual ~EudDevice(){ ///<Destructor
        Quit();
    }

    virtual int32_t Init(); ///<Virtual init function, called from constructor

    virtual int32_t Quit(); ///<Virtual quit function, called from destructor
    
    /* API to assert a reset using a EUD peripheral */
    EUD_ERR_t AssertReset(void);

    /* API to de-assert reset using a EUD peripheral */
    EUD_ERR_t DeAssertReset(void);

    ////////////////////////////////////////////////////////////
    //              EUD specific methods/members.             //
    ////////////////////////////////////////////////////////////

    std::string DvcMgr_DeviceName;

    uint32_t device_id_;
    uint32_t device_type_;
    usb_dev_access_type device_name_;
    usb_dev_access_type ctl_peripheral_name_;

    uint32_t mode_;
    uint32_t GetBufMode(void);              ///<Returns value of current buffer mode
    EUD_ERR_t SetBufMode(uint32_t mode);    ///<Returns value of current buffer mode

    uint8_t GetNumOfOutBytes()  { return usb_num_bytes_pending_out_; }  
    uint8_t GetNumOfInBytes()   { return usb_num_bytes_pending_in_;  }
    int32_t IsFlushRequired()     { return usb_num_bytes_pending_out_ + 5 >= SWD_OUT_BUFFER_SIZE; }

    double read_time_elapsed_;
    double write_time_elapsed_;
    double num_of_writes_;
    double num_of_reads_;

    uint8_t periph_payload_size_table_[EUD_GENERIC_MAX_NUM_OPCODES];
    uint8_t periph_response_size_table_[EUD_GENERIC_MAX_NUM_OPCODES];
    uint32_t periph_endian_option_table_[EUD_GENERIC_MAX_NUM_OPCODES];
    uint32_t periph_max_opcode_value_ = EUD_GENERIC_MAX_NUM_OPCODES;
    
    

    ////////////////////////////////////////////////////////////
    //              USB related methods/members.              //
    //              These link to USB object                  //
    ////////////////////////////////////////////////////////////

    ///USB device class instance for this EUD object to use. 
    UsbDevice * usb_device_handle_;

    ///Flag to indicate if USB device (/ref usb_device_handle) has been initialized
    int32_t usb_device_handle_init_flag_;

    /// The number of bytes expected back from the device 
    uint32_t usb_num_bytes_pending_in_;
    uint32_t usb_num_bytes_pending_out_;

    /// USB in/out buffers. usb_write/usb_read functions use these.
    uint8_t usb_buffer_in_[EUD_GENERIC_USB_IN_BUFFER_SIZE];
    uint8_t usb_buffer_out_[USB_OUT_BUFFER_SIZE];


    EudPacketQueue* eud_queue_p_;
    ///Generic usb initialize function. Uses default device manager name,
    ///deviceID and wait times.
    virtual EUD_ERR_t UsbOpen();
    
    ///USB Init function with additional arguments:
    USB_ERR_t UsbOpen(
        const char * devmgr_name,    ///<Windows Device Manager name; used to attach to USB device.
        uint32_t device_id_local,    ///<Device type (CTL, COM, TRC etc.)
        uint32_t max_wait            ///<Max timeout to attach to USB device before returning failure.
        );

    USB_ERR_t UsbOpen(
        std::string devmgr_name, 
        uint32_t device_ID_local, 
        uint32_t max_wait
        );


    USB_ERR_t UsbOpen(
        usb_dev_access_type dev, 
        uint32_t device_ID_local, 
        uint32_t maxwait
        );


    ///Close USB function.
    USB_ERR_t UsbClose();

    ///Generic USB write function. Writes /ref out_length number of bytes from usb_buffer_out_.
    ///Thus user must first populate usb_buffer_out_, then call /ref usb_write with out_length.
    ///Error codes are returned via USB_ERR_t
    USB_ERR_t UsbWrite(
        uint32_t out_length     ///<Number of bytes to write to USB from USB_BUffer_OUT.
        );

    ///Alternalte USB write function. Writes /out_length number of bytes from out_buffer to USB device.
    ///Error codes are returned via USB_ERR_t
    USB_ERR_t UsbWrite(
        uint8_t* out_buffer,
        uint32_t out_length
        );

    ///Generic USB write and read function. Writes /ref out_length number of bytes from usb_buffer_out_.
    ///Then reads /ref in_length number of bytes from usb_buffer_IN. 
    ///Error codes are returned via USB_ERR_t
    virtual USB_ERR_t UsbWriteRead(
        uint32_t out_length,    ///<Number of bytes to write to USB from USB_BUffer_OUT.
        uint32_t in_length      ///<Number of bytes to read from USB to USB_Buffer_IN.
        );

    ///USB Read/Write function which writes and reads to given buffers instead of to usb_buffer_out_/IN.
    ///Error codes are returned via USB_ERR_t
    virtual USB_ERR_t UsbWriteRead(
        uint32_t out_length,    ///<Number of bytes to write to USB from /ref out_buffer
        uint8_t* out_buffer,    ///<Buffer to be written to USB
        uint32_t in_length,     ///<Number of bytes to read from USB to /ref in_buffer
        uint8_t* in_buffer      ///<Buffer to store bytes read from USB
        );

    /*USB_ERR_t  UsbWriteRead(
        uint8_t opcode,                 ///<opcode to be sent. This is the first Byte of the packet sent.
        uint32_t out_payload_length,    ///<Length of payload. Doesn't include size  of opcode (i.e. which is size of packet - 1).
        uint32_t out_payload,           ///<payload to be sent. Ignored if out_payload_length is 0.
        uint32_t in_length,             ///<received packet size. If 0, a usb read is not attempted, else USB read will be  attempted.
        uint32_t* in_payload            ///<Pointer to received payload.
        );
        */

    EUD_ERR_t UsbWriteRead(
        EudPacketQueue* eud_queue_p
        );

    EUD_ERR_t CleanupPacketQueue(
        EudPacketQueue* eud_queue_p
        );
    
    virtual EUD_ERR_t QueueCommand(
        uint32_t opcode, 
        uint32_t payload, 
        uint32_t* response_p
        );

    ///Generic USB read function. Reads in /ref in_length from USB device to /ref in_buffer
    virtual USB_ERR_t UsbRead(
        uint32_t in_length,     ///<Number of bytes to be read from USB device to /ref in_buffer
        uint8_t *in_buffer      ///<Buffer to store bytes read from USB
        );

    EUD_ERR_t WriteCommand(uint32_t opcode, uint32_t data, uint32_t* rvalue);
    EUD_ERR_t WriteCommand(uint32_t opcode, uint32_t* rvalue);
    EUD_ERR_t WriteCommand(uint32_t opcode, uint32_t data);

    EUD_ERR_t WriteCommand(uint32_t opcode, uint8_t* data, uint8_t* rvalue);
    //EUD_ERR_t WriteCommand(uint32_t opcode, uint8_t* rvalue);
    EUD_ERR_t WriteCommand(uint32_t opcode, uint8_t* data);

    EUD_ERR_t WriteCommand(uint32_t opcode);

};
#else
typedef  void  EudDevice;
#endif

/****************************************************************************//**
*   @brief Queries device manager name state and retrieves appropriate
*   device name for device manager. Several return values:
*   EUD_NULL_DEVMGR_NAME - unknown error
*   EUD_MANUAL_MODE_NEED_USER_INPUT - Manual device manager mode selected and user
*                                     has not yet given input.
*   EUD_AUTO_MODE_MAX_DEVICES_REACHED - For auto mode case where we are cycling through 
*                                       potential matches, this returned if maximum reached.
*
*********************************************************************************/
#ifdef  __cplusplus
std::string devmgr_dev_name_generator(
    EudDevice* eud_handle_p
    );

/****************************************************************************//**
*   @brief copies 4-wide array of bytes into a 32bit word
*
*********************************************************************************/
inline void unpack_uint8_to_uint32_array(uint32_t* data32_p, uint8_t* data8_p )
{
    *data32_p = 0;
#ifdef SWD_LITTLEENDIAN
    *data32_p = (uint32_t)(*(data8_p + 3));
    *data32_p += (uint32_t)(*(data8_p + 2) << 8);
    *data32_p += (uint32_t)(*(data8_p + 1) << 16);
    *data32_p += (uint32_t)(*(data8_p) << 24);
#else
    *data32_p = (uint32_t)* data8_p;
    *data32_p += (uint32_t)(*(data8_p + 1) << 8);
    *data32_p += (uint32_t)(*(data8_p + 2) << 16);
    *data32_p += (uint32_t)(*(data8_p + 3) << 24);
#endif
}

inline void pack_uint32_to_uint8_array(
    uint32_t* data32_p,
    uint8_t* data8_p
    );

inline void pack_uint32_to_uint8_array(
    uint32_t data32,
    uint8_t* data8_p
    );

/*inline void pack_uint32_to_uint8_array(
    uint32_t data32,
    unsigned char* data8_p
);
*/
#endif

#endif //EUD_H_


