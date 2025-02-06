/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   device_manager.h
*
* Description:                                                              
*   Class for device manager singleton operation.
*
***************************************************************************/
#ifndef DEVICE_MANAGER_H_
#define DEVICE_MANAGER_H_

#include "eud.h"
#include "usb.h"
#include "swd_eud.h"
#include "jtag_eud.h"
#include "ctl_eud.h"
#include "com_eud.h"
#include "trc_eud.h"
#include <malloc.h>
#include "device_manager.h"

//QCOM_SNIP_BEGIN
#if defined ( EUD_WIN_ENV )
#include <Windows.h>
#endif
#include <vector>
#include <map>
//#include <pair>
//#include <less>
#define EUD_ERR_USB_INIT_FAILED_MUTEX_ABANDONED 3
#define EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED 3

#define DEVMGR_INITVALUE                0xC5ACCE55
#define WAIT_TIME_EUD_ID_EXE            1300
#define EUD_PERIPH_OPEN_DEFAULT_MAXWAIT 100
#define EUD_PERIPH_ENABLE_SLEEP_TIME    300
#define MAX_DEVICESTRING_SIZE           4096 ///<Max size for string returned by eud_id5.exe

#define MAX_EUD_PERIPH_ENABLE_RETRIES   1

#define EUD_ID_EXE                      "eud_id5.exe"
#define PRELOAD_READ_PIPE_VALUE         "EUD USB Header"

#define NULL_BUS_PATH                   "Bus Location <PCIROOT(0)#PCI(1400)#USBROOT(0)#USB(0)#USB(0)#USB(0)>"            

#define RETRIEVE_EUD_ID_OPTIONS_NULL        0
#define RETRIEVE_EUD_ID_OPTIONS_LONGWAIT    1

//HANDLE ghMutex; ///<Declare this in header file.

//typedef std::map <std::string, std::string> EUDPeripheralMap_t;


class DevMgrTree{
public:
    DevMgrTree();
    uint32_t device_id_;
    std::string ctl_peripheral_devmgrname_;
    std::string trc_peripheral_devmgrname_;
    std::string swd_peripheral_devmgrname_;
    std::string jtg_peripheral_devmgrname_;
    std::string com_peripheral_devmgrname_;
};

typedef class PeriphTree{
public:
    PeriphTree();
    ~PeriphTree();
    //EUDPeripheralMap_t tree;
    //DevMgrTree* tree;
    uint32_t device_id_;
    usb_dev_access_type* ctl_peripheral_devmgrname_p_;
    usb_dev_access_type* trc_peripheral_devmgrname_p_;
    usb_dev_access_type* swd_peripheral_devmgrname_p_;
    usb_dev_access_type* jtg_peripheral_devmgrname_p_;
    usb_dev_access_type* com_peripheral_devmgrname_p_;
    std::string* device_usb_path_p_;
} periphtree_t;


typedef std::map < uint32_t, periphtree_t*> EUDDevicesMap_t;

typedef std::map <uint32_t, std::string> EUDDevicetoDevmgrName_t;



class DevMgrTreeList{
private:
    std::vector<DevMgrTree> tree_list_;
    
public:
    uint32_t GetDeviceIdByPeripheralName(std::string periph_name);
    uint32_t GetNumDevices();

    ///Consumes a list of found active devices. 
    ///Matches those devices to existing tree
    ///Devices in tree that are not in founddevicelist are returned to be made stale
    

    std::vector<std::string> GetMatchedDevices(std::vector<std::string> string_list, uint32_t device_type);
    std::vector<std::string> GetUnmatchedDevices(std::vector<std::string> string_list, uint32_t device_type);

};

typedef std::vector<std::string> basename_v;
class DeviceBaseNameTree{
public:
    DeviceBaseNameTree();

    static DeviceBaseNameTree *DeviceBaseNameTreeInstance();
    basename_v* devicebasenames_p_;
private:
    static DeviceBaseNameTree *device_basename_tree_instance_;
};

class DeviceMgr{
    DeviceMgr();
    ~DeviceMgr();
private:
    /* Private variable to keep track of the scanning */
    static bool scan_devices_done_;

public:
    static DeviceMgr *DeviceMgrInstance();
  
    #if defined ( EUD_WIN_ENV )
    /*static*/ HANDLE ghMutex_;
    #endif
    /*      */
    /*static*/ uint32_t is_initialized_;
    /*static*/ uint32_t instance_counter_;
    bool scan_done_;

    /*static*/ DevMgrTreeList* active_devices_;
    

    /* Function to perform a scan for attached EUD peripherals / devices to skip unless done already */
    EUD_ERR_t ScanDevices(void);
    /* API to perform a forced scan for attached EUD peripherals / devices */
    EUD_ERR_t ScanDevices(bool force);
    
    ////////Static member functions
    PVOID*    InitializeUsbDevice(uint32_t device_id, uint32_t device_type, EUD_ERR_t* error_code_p);
    EUD_ERR_t ForceOffUsbDevice(uint32_t device_id, uint32_t device_type);
    EUD_ERR_t GetDeviceIdByDevMgrName(char* devmgr_name, uint32_t* device_id_p);
    EUD_ERR_t GetCtlPeripheralByPeripheralPtr(PVOID* eud_device, char* ctl_peripheral_dev_name);
    EUD_ERR_t get_device_id_array(uint32_t* array_p, uint32_t* length_p);
    CtlEudDevice* GetCtlPeripheralByDeviceId(uint32_t device_id, EUD_ERR_t* err_p);
    EUD_ERR_t GetCtlPeripheralStringByDeviceId(uint32_t device_id, std::string* ctl_peripheral_dev_name_p);
    EUD_ERR_t GetCtlPeripheralStringByDeviceId(uint32_t device_id, char* ctl_peripheral_dev_name_p, uint32_t* length_p);
    EUD_ERR_t GetAttachedDevicesString(char* string_buffer, uint32_t* string_size_p);
    

   //#if defined (EUD_WIN_ENV) 
   std::vector<periphtree_t*> ParseEudIdIntoTreeList(std::string eud_id_output_p);
   //#elif defined (EUD_LNX_ENV)
   std::vector<periphtree_t*> ParseEudIdIntoTreeList(std::vector<eud_device_info> eud_tree_v);
   //#endif
    periphtree_t* GetDeviceTreeByDeviceId(uint32_t device_id);
    DeviceBaseNameTree* device_basename_tree_instance_;

private:
    EUDDevicesMap_t device_map_;


    static DeviceMgr *devmgr_instance_;
};

extern DeviceMgr* DevMgrInstance;

//QCOM_SNIP_END
/**************************************//**
*
*   @brief Populates an array of uint32's with available devices. 
*   Guaranteed max of 100 devices (i.e. please initialize an array 
*   of size 100 and of type uint32 to pass to this API).
*
******************************************/
EXPORT EUD_ERR_t        ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
get_device_id_array(
    uint32_t* array_p,  ///<Pointer to  array to populate with device ID's. Max of 100. 
    uint32_t* length_p  ///<Length value to be populated of array.
);
/**************************************//**
*
*   @brief Populates given string pointer with string of available devices.
*   Suitable for display on a command prompt.
*
******************************************/
EXPORT EUD_ERR_t            ///< Error code. 0 for success. Call eud_get_error_string for ASCII description.
get_attached_devices_string(
    char* string_buffer,     ///<Pointer to char array to be populated with device ID string. Max of 1000. 
    uint32_t* string_size_p  ///<Length of returned string.
);


//QCOM_SNIP_BEGIN

EUD_ERR_t toggle_peripheral(std::string ctl_devname, uint32_t device_id, uint32_t device_type, uint32_t action);
EUD_ERR_t toggle_peripheral(CtlEudDevice* ctl_handle_p, uint32_t device_id, uint32_t device_type, uint32_t action);
PVOID* initialize_usb_device(uint32_t device_id, uint32_t device_type, EUD_ERR_t* err_p);
EUD_ERR_t force_off_usb_device(uint32_t device_id, uint32_t device_type);
//std::string* retrieve_eud_id_string(EUD_ERR_t* err_p,std::string executablename, uint32_t options);
std::string retrieve_eud_id_string(EUD_ERR_t* err_p, std::string executable_name, uint32_t options);
CtlEudDevice* get_ctl_peripheral_by_device_id(uint32_t device_id, EUD_ERR_t* err_p);
//QCOM_SNIP_END
#endif

