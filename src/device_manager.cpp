/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   device_manager.cpp
*
* Description:                                                              
*   Handles the Windows Device Manager ID's that EUD can come up as.
*
***************************************************************************/

#include "eud.h"
#include "com_eud.h"
#include "ctl_eud.h"
#include "jtag_eud.h"
#include "swd_eud.h"                       
#include "trc_eud.h"
#include "usb.h"
#include <unistd.h>

// #include "eud_api.h"
#include "device_manager_defines.h"

#if defined ( EUD_WIN_ENV )
#include <Windows.h>
#endif
#include "device_manager.h"
#include <regex>
#include <string>
#include <sstream>
#include <fstream>
#include "usb.h"

#if defined ( EUD_LNX_ENV )
//    #define WAIT_OBJECT_0 0
//    #define WAIT_ABANDONED 0x00000080L
#endif

extern void eudlibInit(void);
extern void eudlibDeinit(void);
extern EUD_ERR_t check_eud_usb_device (const char * devicemgrname_p, uint32_t device_id);

extern "C" EUD_ERR_t eud_close_peripheral(PVOID* handle_p);
extern EUD_ERR_t DeviceManagerPoller(uint32_t device_id, char * devicemgrname);
extern EUD_ERR_t DeviceManagerPoller(uint32_t device_id, uint32_t startvalue, char* devicemgrname_p);
extern uint32_t  ascii_to_int (std::string asciinum);
extern std::string uint32_to_ascii (uint32_t number);


//===---------------------------------------------------------------------===//
//
// Functional description
//
//===---------------------------------------------------------------------===//

uint32_t DeviceManagerMode = DEVICEMGRMODE_AUTO;
const char* EUD_Null_devmgr_name = EUD_NULL_DEVMGR_NAME; 
const char* CTL_devicemgrname = EUD_Null_devmgr_name;
const char* TRC_devicemgrname = EUD_Null_devmgr_name;
const char* JTG_devicemgrname = EUD_Null_devmgr_name;
const char* SWD_devicemgrname = EUD_Null_devmgr_name;
const char* COM_devicemgrname = EUD_Null_devmgr_name;

/**************************************//**
*   @brief Configures Device Manager Mode.
*   Currently accepts 0x1 (Manual Select) or 0x0 (Auto Select)
*
*******************************************/
EXPORT EUD_ERR_t SetDeviceManagerMode(uint32_t Mode){
    if (Mode > MAX_DEVICE_MGR_MODES){
        return eud_set_last_error(EUD_ERR_DEVICE_MANAGER_MODE_PARAMETER_ERR);
    }

    DeviceManagerMode = Mode;

    return EUD_SUCCESS;

}

/**************************************//**
*   @brief Sets EUD device ID for the passed eud_handler_p pointer.
*   ID must be 0000 <= 9999 (A four decimal number)
*
*******************************************/
//EXPORT EUD_ERR_t set_device_manager_id(handleWrap* eud_handler_p, uint32_t ID){
//  if (DeviceManagerMode == DEVICEMGRMODE_AUTO){
//      return  eud_set_last_error(EUD_ERR_CANT_CHANGE_DEVICE_MGR_MODE_WHEN_IN_AUTO);
//  }
//
//  return DeviceManagerIDSetter(eud_handler_p, ID, FALSE);
//
//}


/**************************************//**
*   @brief Populates internal device variable with ID given.
*   This variable is then checked at usb attach time.
*   If checkflag is TRUE, then try usb_open to verify. Return result.
*
*******************************************/
EUD_ERR_t set_device_manager_id(uint32_t device_id, uint32_t ID, char* devicemgrname_p, uint32_t checkflag){

    static  bool short_device_name_flag = false;

    if ((device_id == 0)||(device_id > DEVICETYPE_EUD_MAX)){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }
    if (ID > MAX_DEVMGR_DEVICEID){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }
    if (checkflag > 2){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }
    //std::string backup1;
    //todo - make these into objects
    //char devmgrchar_p[MAX_DEVMGR_NAME_CHARLEN] = { 0 };
    //char devmgrbasechar_p[MAX_DEVMGR_NAME_CHARLEN] = { 0 };
    //char devmgrbasesuffix_p[MAX_DEVMGR_NAME_CHARLEN] = { 0 };
    std::string devmgrchar_p;
    std::string devmgrbasechar_p;
    std::string devmgrbasesuffix_p;
    //const char* EUD_devicemgrname_ptr_to_set;
    switch (device_id){

    case DEVICETYPE_EUD_NULL:
        return  eud_set_last_error(EUD_ERR_EUD_HANDLER_DEVICEID_NOT_POPULATED);
    case DEVICETYPE_EUD_CTL:
        devmgrchar_p = std::string(EUD_CTL_DEVMGR_BASENAME);
        //EUD_devicemgrname_ptr_to_set = CTL_devicemgrname;
        devmgrbasesuffix_p = std::string(EUD_CTL_DEVMGR_BASESUFFIX);
        break;
    case DEVICETYPE_EUD_TRC:
        devmgrchar_p = std::string(EUD_TRC_DEVMGR_BASENAME);
        //EUD_devicemgrname_ptr_to_set = TRC_devicemgrname;
        devmgrbasesuffix_p = std::string(EUD_TRC_DEVMGR_BASESUFFIX);
        break;
    case DEVICETYPE_EUD_JTG:
        devmgrchar_p = std::string(EUD_JTG_DEVMGR_BASENAME);
        //EUD_devicemgrname_ptr_to_set = JTG_devicemgrname;
        devmgrbasesuffix_p = std::string(EUD_JTG_DEVMGR_BASESUFFIX);
        break;
    case DEVICETYPE_EUD_SWD:
        devmgrchar_p = std::string(EUD_SWD_DEVMGR_BASENAME);
        //EUD_devicemgrname_ptr_to_set = SWD_devicemgrname;
        devmgrbasesuffix_p = std::string(EUD_SWD_DEVMGR_BASESUFFIX);
        break;
    case DEVICETYPE_EUD_COM:
        devmgrchar_p = std::string(EUD_COM_DEVMGR_BASENAME);
        //EUD_devicemgrname_ptr_to_set = COM_devicemgrname;
        devmgrbasesuffix_p = std::string(EUD_COM_DEVMGR_BASESUFFIX);
        break;
    default:
        return  eud_set_last_error(EUD_ERR_EUD_HANDLER_DEVICEID_NOT_RECOGNIZED);
    }
    //Check ID
    //Check eud_handler_p for type, cast to that type. Error out if cast fails

    //Special case - check for legacy device manager device name if present, only for 0x0 case.
    //If not, continue with 0x0 case.
    if (ID == 0){
        //std::string devmgrchar2_p   = devmgrchar_p;
        memcpy(devicemgrname_p, devmgrchar_p.c_str(), devmgrchar_p.length() + 1);
        //devmgrchar2_p += devmgrbasesuffix_p;
        //EUD_devicemgrname_ptr_to_set = devmgrchar2_p.c_str();

        std::string devmgr_testname = USB_DEBUG_PREFIX + devmgrchar_p + devmgrbasesuffix_p;

        if (EUD_SUCCESS == check_eud_usb_device(devmgr_testname.c_str(), device_id)){
            short_device_name_flag = true;
            return EUD_SUCCESS;
        }
        else {
            short_device_name_flag = false;
        }
    }
    if (short_device_name_flag == true) {
        return EUD_SUCCESS;
    }
    //set eud_handler_p's device ID to what's given.
    devmgrchar_p += DEVMGR_PERIPH_NUM_PREFIX;
    devmgrchar_p += uint32_to_ascii(ID);
    devmgrchar_p += DEVMGR_PERIPH_NUM_SUFFIX;

    //devmgrchar_p += devmgrbasesuffix_p;

    //Set global devicemgrname to what we've evaluated here.
    //EUD_devicemgrname_ptr_to_set = devmgrchar_p.c_str();
    memcpy(devicemgrname_p, devmgrchar_p.c_str(), devmgrchar_p.length() + 1);


    //if checkflag  is false, return success
    //if checkflag is true, try USB open. return results.
    if (checkflag == TRUE){
        std::string devmgr_testval = USB_DEBUG_PREFIX + devmgrchar_p + devmgrbasesuffix_p;
        return check_eud_usb_device(devmgr_testval.c_str(), device_id);
    }
    else{
        return EUD_SUCCESS;
    }

    //Should never get here
    return EUD_SUCCESS;
}
EUD_ERR_t check_eud_usb_device(const char * devicemgrname_p, uint32_t device_id_local){

    EUD_ERR_t  err;

    EudDevice* eud_device_p = new EudDevice;

    err = (EUD_ERR_t)eud_device_p->UsbOpen(devicemgrname_p, device_id_local, DEVNUM_POLL_VALUE);
    delete eud_device_p;
    eud_device_p = NULL;

    return err;
}


std::string devmgr_devnamegenerator(EudDevice* eud_handle_p){

    std::string rvalue = std::string(EUD_NULL_DEVMGR_NAME);
    char * devicemgrname_local = new char[EUD_DEVMGR_MAX_DEVMGR_STRING_LEN];
    memcpy(devicemgrname_local, rvalue.c_str(), rvalue.length() + 1); ///<Set devicemgrname char array to null value
    EUD_ERR_t err;
    if (DeviceManagerMode == DEVICEMGRMODE_AUTO){
        if ((err = DeviceManagerPoller(eud_handle_p->device_type_, devicemgrname_local))) {
            eud_set_last_error(err);
            return EUD_AUTO_MODE_MAX_DEVICES_REACHED;
        }
        rvalue = std::string(devicemgrname_local);
    }
    else{
        const char* deviceid_ptr;
        switch (eud_handle_p->device_type_){
        case DEVICETYPE_EUD_CTL:
            deviceid_ptr = CTL_devicemgrname;
            break;
        case DEVICETYPE_EUD_TRC:
            deviceid_ptr = TRC_devicemgrname;
            break;
        case DEVICETYPE_EUD_JTG:
            deviceid_ptr = JTG_devicemgrname;
            break;
        case DEVICETYPE_EUD_SWD:
            deviceid_ptr = SWD_devicemgrname;
            break;
        case DEVICETYPE_EUD_COM:
            deviceid_ptr = COM_devicemgrname;
            break;
        default:
            return rvalue;

            //If devicemgrname is populated with a null value,
            //then user has not yet set EUD device name. Return error.
            if ((strcmp(deviceid_ptr, EUD_NULL_DEVMGR_NAME)) == 0){
                rvalue = std::string(EUD_MANUAL_MODE_NEED_USER_INPUT);
            }
            //If devicemgrname is populated with a legitimate value,
            //Return that value and set current pointer to null.
            //TODO - guard this with mutex?
            else{
                rvalue = std::string(deviceid_ptr);
                deviceid_ptr = EUD_Null_devmgr_name;
            }
        }
    }
    return rvalue;
}

std::string devmgr_devnamegenerator(uint32_t device_id, uint32_t startvalue){

    std::string rvalue = std::string(EUD_NULL_DEVMGR_NAME);
    char * devicemgrname_local = new char[EUD_DEVMGR_MAX_DEVMGR_STRING_LEN];
    memcpy(devicemgrname_local, rvalue.c_str(), rvalue.length() + 1); ///<Set devicemgrname char array to null value
    EUD_ERR_t err;
    if (DeviceManagerMode == DEVICEMGRMODE_AUTO){
        if ((err = DeviceManagerPoller(device_id, startvalue, devicemgrname_local))) {
            eud_set_last_error(err);
            return EUD_AUTO_MODE_MAX_DEVICES_REACHED;
        }
        rvalue = std::string(devicemgrname_local);
    }
    else{
        const char* deviceid_ptr;
        switch (device_id){
        case DEVICETYPE_EUD_CTL:
            deviceid_ptr = CTL_devicemgrname;
            break;
        case DEVICETYPE_EUD_TRC:
            deviceid_ptr = TRC_devicemgrname;
            break;
        case DEVICETYPE_EUD_JTG:
            deviceid_ptr = JTG_devicemgrname;
            break;
        case DEVICETYPE_EUD_SWD:
            deviceid_ptr = SWD_devicemgrname;
            break;
        case DEVICETYPE_EUD_COM:
            deviceid_ptr = COM_devicemgrname;
            break;
        default:
            return rvalue;

            //If devicemgrname is populated with a null value,
            //then user has not yet set EUD device name. Return error.
            if (strcmp(deviceid_ptr, EUD_NULL_DEVMGR_NAME) == 0){
                rvalue = std::string(EUD_MANUAL_MODE_NEED_USER_INPUT);
            }
            //If devicemgrname is populated with a legitimate value,
            //Return that value and set current pointer to null.
            //TODO - guard this with mutex?
            else{
                rvalue = std::string(deviceid_ptr);
                deviceid_ptr = EUD_Null_devmgr_name;
            }
        }
    }
    return rvalue;
}
/**************************************//**
*   @brief Polls for devices using usb_open() function.
*   Sets eud_handler_p's device manager ID to first found
*   device and returns successful. Returns failure if not.
*
*******************************************/
//EUD_ERR_t DeviceManagerPoller(handleWrap* eud_handler_p){
EUD_ERR_t DeviceManagerPoller(uint32_t devicetype, char* devicemgrname_p){
    uint32_t iterator, retry_iterator;
    uint32_t err = -1;

    //if (eud_handler_p == NULL){
    //  return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    //}
    if ((devicetype == 0) || (devicetype > DEVICETYPE_EUD_MAX)){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }
    for (retry_iterator = 0; retry_iterator < MAX_USB_ATTACH_RETRY_COUNT; retry_iterator++){
        for (iterator = 0; iterator < MAX_DEVICEMGR_POLL_VALUE; iterator++){

            err = set_device_manager_id(devicetype, iterator, devicemgrname_p, TRUE);


            if ((strcmp(devicemgrname_p, EUD_NULL_DEVMGR_NAME) != 0) && (err == EUD_SUCCESS)){
                return EUD_SUCCESS;
            }

        }
        Sleep(2000);
    }
    //If error, then likely we didn't find a device. Set last error to that.
    err = EUD_USB_DEVICE_NOT_DETECTED;

    return eud_set_last_error(err);
}
EUD_ERR_t DeviceManagerPoller(uint32_t devicetype, char* devicemgrname_p, bool waitflag) {
    uint32_t iterator, retry_iterator;
    uint32_t err = -1;

    //if (eud_handler_p == NULL){
    //  return eud_set_last_error(EUD_ERR_BAD_HANDLE_PARAMETER);
    //}
    if ((devicetype == 0) || (devicetype > DEVICETYPE_EUD_MAX)) {
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }
    for (retry_iterator = 0; retry_iterator < MAX_USB_ATTACH_RETRY_COUNT; retry_iterator++) {
        for (iterator = 0; iterator < MAX_DEVICEMGR_POLL_VALUE; iterator++) {

            err = set_device_manager_id(devicetype, iterator, devicemgrname_p, TRUE);


            if ((strcmp(devicemgrname_p, EUD_NULL_DEVMGR_NAME) != 0) && (err == EUD_SUCCESS)) {
                return EUD_SUCCESS;
            }

        }
        if (waitflag == true)
            Sleep(2000);
    }
    //If error, then likely we didn't find a device. Set last error to that.
    err = EUD_USB_DEVICE_NOT_DETECTED;

    return eud_set_last_error(err);
}
#define EUD_ERR_POLL_VALUE_TOO_HIGH 3

EUD_ERR_t DeviceManagerPoller(uint32_t devicetype, uint32_t startvalue, char* devicemgrname_p){

    uint32_t iterator, retry_iterator;
    uint32_t err = -1;

    if (startvalue > MAX_DEVICEMGR_POLL_VALUE){
        return eud_set_last_error(EUD_ERR_POLL_VALUE_TOO_HIGH);
    }

    if ((devicetype == 0) || (devicetype > DEVICETYPE_EUD_MAX)){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }
    for (retry_iterator = 0; retry_iterator < MAX_USB_ATTACH_RETRY_COUNT; retry_iterator++){
        for (iterator = startvalue; iterator < MAX_DEVICEMGR_POLL_VALUE; iterator++){

            err = set_device_manager_id(devicetype, iterator, devicemgrname_p, TRUE);
            if (((strcmp(devicemgrname_p, EUD_NULL_DEVMGR_NAME)) != 0) && (err == EUD_SUCCESS)){
                return EUD_SUCCESS;
            }

        }
        Sleep(2000);
    }
    //If error, then likely we didn't find a device. Set last error to that.
    err = EUD_USB_DEVICE_NOT_DETECTED;

    return eud_set_last_error(err);
}





/*
HANDLE DeviceMgr::ghMutex_ = NULL;
uint32_t DeviceMgr::is_initialized_ = 0;
uint32_t DeviceMgr::instance_counter_ = 0;
DevMgrTree* DeviceMgr::devtree_p = new DevMgrTree;
*/
DeviceMgr *DeviceMgr::devmgr_instance_ = NULL;
/* Initialize the state to indicate that scanning is not done */
bool DeviceMgr::scan_devices_done_ = FALSE;

DeviceMgr *DeviceMgr::DeviceMgrInstance(){

    if (devmgr_instance_ == NULL)
    {
        devmgr_instance_ = new DeviceMgr();
    }
    return devmgr_instance_;
}

DeviceMgr::DeviceMgr(){
    //set init flag
    //init mutex
    //increment instance count?
    if (is_initialized_ == DEVMGR_INITVALUE){

        instance_counter_++;
        QCEUD_Print("DeviceMgrConstructor called. Instances: %d", instance_counter_);
    }
    else{

        is_initialized_ = true;
        instance_counter_ = 1;
        eudlibInit();

        #if defined ( EUD_WIN_ENV )
        ghMutex_ = CreateMutex(
            NULL,
            FALSE,
            NULL);
        #endif
        QCEUD_Print("DeviceMgrConstructor called. First instance");

        //ScanDevices();

    }
    device_basename_tree_instance_ = new DeviceBaseNameTree;

}
//DeviceMgr* DevMgrInstance_p = new DeviceMgr;
DeviceMgr::~DeviceMgr(){
    //Decrement usage count
    //if usage count== 0, free mutex
    eudlibDeinit();
    if (is_initialized_ == DEVMGR_INITVALUE){
        instance_counter_--;
        QCEUD_Print("DeviceMgrDestructor called. Instances: %d", instance_counter_);
        if (instance_counter_ == 0){
            #if defined ( EUD_WIN_ENV )
            CloseHandle(ghMutex_);
            #endif
            QCEUD_Print("Closed DeviceMgr mutex");

        }

    }

}


//Iterates through ALL EUD device types. Populates devtree_p with results
/*
PeriphTree::PeriphTree(uint32_t device_id, std::string ctl_peripheral_devname){


    tree->device_id = device_id;
    tree->ctl_peripheral_devmgrname_ = ctl_peripheral_devname;
    tree->swd_peripheral_devmgrname = "";
    tree->jtg_peripheral_devmgrname = "";
    tree->com_peripheral_devmgrname = "";

    //tree["CTL"] = "";
    //tree["JTG"] = "";
    //tree["SWD"] = "";
    //tree["TRC"] = "";
    //tree["COM"] = "";

}
*/
DevMgrTree::DevMgrTree(){
    device_id_ = uint32_t(0);
    ctl_peripheral_devmgrname_ = std::string("                                                      ");
    trc_peripheral_devmgrname_ = std::string("                                                      ");
    swd_peripheral_devmgrname_ = std::string("                                                      ");
    jtg_peripheral_devmgrname_ = std::string("                                                      ");
    com_peripheral_devmgrname_ = std::string("                                                      ");
}
PeriphTree::~PeriphTree(){
    delete ctl_peripheral_devmgrname_p_;
    delete trc_peripheral_devmgrname_p_;
    delete swd_peripheral_devmgrname_p_;
    delete jtg_peripheral_devmgrname_p_;
    delete com_peripheral_devmgrname_p_;
    delete device_usb_path_p_;

}
PeriphTree::PeriphTree(){

    //DevMgrTree* tree = new DevMgrTree;
    //tree->device_id = 0;
    //tree->ctl_peripheral_devmgrname_ = "";
    //tree->swd_peripheral_devmgrname = "";
    //tree->jtg_peripheral_devmgrname = "";
    //tree->com_peripheral_devmgrname = "";
    ctl_peripheral_devmgrname_p_ = new usb_dev_access_type; //new std::string;
    trc_peripheral_devmgrname_p_ = new usb_dev_access_type; //std::string;
    swd_peripheral_devmgrname_p_ = new usb_dev_access_type; //std::string;
    jtg_peripheral_devmgrname_p_ = new usb_dev_access_type; //std::string;
    com_peripheral_devmgrname_p_ = new usb_dev_access_type; //std::string;
    device_usb_path_p_ = new std::string;
    device_id_ = 0;
    /*
    tree["CTL"] = "";
    tree["JTG"] = "";
    tree["SWD"] = "";
    tree["TRC"] = "";
    tree["COM"] = "";
    */
}


std::vector<uint32_t> get_active_device_ids(std::string eud_id_output){
    std::vector<uint32_t> devicelist (1,0);
    return devicelist;


}

periphtree_t* DeviceMgr::GetDeviceTreeByDeviceId(uint32_t device_id){

    if (ScanDevices()) return NULL;

    std::map<uint32_t, periphtree_t*>::const_iterator it = device_map_.find(device_id);
    if (it == device_map_.end()){
        return NULL;
    }
    else{
        return it->second;
    }

}

/*
#if defined (EUD_WIN_ENV)
EUD_ERR_t DeviceMgr::GetCtlPeripheralStringByDeviceId(uint32_t device_id, char* ctl_peripheral_devname_p, uint32_t* length_p){

    if ((ctl_peripheral_devname_p == NULL) || (length_p == NULL)){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }

    //ScanDevices();

    periphtree_t* tree_p = GetDeviceTreeByDeviceId(device_id);

    if (tree_p == NULL){
        return eud_set_last_error(EUD_ERR_DEVICEID_NOT_FOUND);
    }

    usb_dev_access_type ctl_periph = *tree_p->ctl_peripheral_devmgrname_p_;
    sprintf_s(ctl_peripheral_devname_p, EUD_DEVMGR_MAX_DEVMGR_STRING_LEN, ctl_periph.c_str());
    *length_p = (uint32_t)ctl_periph.length();

    return EUD_SUCCESS;

}

EUD_ERR_t DeviceMgr::GetCtlPeripheralStringByDeviceId(uint32_t device_id, std::string* ctl_peripheral_devname_p){

    if (ctl_peripheral_devname_p == NULL){
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }

    //ScanDevices();

    periphtree_t* tree_p = GetDeviceTreeByDeviceId(device_id);

    if (tree_p == NULL){
        return eud_set_last_error(EUD_ERR_DEVICEID_NOT_FOUND);
    }

    usb_dev_access_type ctl_periph = *tree_p->ctl_peripheral_devmgrname_p_;
    *ctl_peripheral_devname_p = ctl_periph;

    return EUD_SUCCESS;

}
#endif
*/

usb_dev_access_type getdevicemgraccessstring(usb_dev_access_type basename){
    return basename;
}

CtlEudDevice* DeviceMgr::GetCtlPeripheralByDeviceId(uint32_t device_id, EUD_ERR_t* err_p){

    if (err_p == NULL){
        eud_set_last_error(EUD_ERR_BAD_PARAMETER);
        return NULL;
    }

    //ScanDevices();

    periphtree_t* tree_p = GetDeviceTreeByDeviceId(device_id);

    if (tree_p == NULL){
        *err_p = eud_set_last_error(EUD_ERR_DEVICEID_NOT_FOUND);
        return NULL;
    }

    usb_dev_access_type ctl_peripheral_devname_p = getdevicemgraccessstring(*tree_p->ctl_peripheral_devmgrname_p_);
    CtlEudDevice* ctl_device_p = new CtlEudDevice;
    *err_p = (EUD_ERR_t)ctl_device_p->UsbOpen(ctl_peripheral_devname_p, DEVICETYPE_EUD_CTL, 20);


    if (*err_p != EUD_SUCCESS){
        eud_close_peripheral((PVOID*)ctl_device_p);
        //QCEUD_Print("eud_initialize_device_ctl error - usb_open failed, error code %x\n", *errcode);
    }


    return ctl_device_p;

}



periphtree_t* parse_eud_id_result_by_deviceid(uint32_t device_id, std::string eud_id_result){
    return NULL;
}
DeviceBaseNameTree *DeviceBaseNameTree::device_basename_tree_instance_ = NULL;

DeviceBaseNameTree *DeviceBaseNameTree::DeviceBaseNameTreeInstance(){

    if (device_basename_tree_instance_ == NULL)
    {
        device_basename_tree_instance_ = new DeviceBaseNameTree();
    }
    return device_basename_tree_instance_;
}

DeviceBaseNameTree::DeviceBaseNameTree(){

    std::string basename;
    devicebasenames_p_ = new basename_v;

    basename = EUD_CTL_DEVMGR_BASENAME;
    devicebasenames_p_->push_back(basename);

    basename = EUD_JTG_DEVMGR_BASENAME;
    devicebasenames_p_->push_back(basename);

    basename = EUD_SWD_DEVMGR_BASENAME;
    devicebasenames_p_->push_back(basename);

    basename = EUD_TRC_DEVMGR_BASENAME;
    devicebasenames_p_->push_back(basename);

    basename = EUD_COM_DEVMGR_BASENAME;
    devicebasenames_p_->push_back(basename);

}


std::pair<std::string, std::string> extract(std::string regex, std::string text){
    std::regex re(regex);
    std::smatch match_results;
    if (std::regex_search(text, match_results, re) && (match_results.size() == 3)){
        std::pair<std::string, std::string> nullpair = { match_results[1].str(), match_results[2].str() };
        return nullpair;
    }
    else{
        std::pair<std::string, std::string> nullpair = { "stuff", "stuff1" };
        return nullpair;
    }
}

std::pair<std::string, std::string> getpair2(std::string stringtosearch, size_t* offset){
    std::string buslocationstr = "Bus Location";
    std::string devnamestr = "Device Name";

    //Trim beginning of string.
    std::string newstringtosearch = stringtosearch.substr(stringtosearch.find(buslocationstr));
    size_t bus_length = newstringtosearch.find("Device Name") - newstringtosearch.find("Bus Location");
    std::string stringlessbuspath = newstringtosearch.substr(bus_length);
    size_t devnamelength = stringlessbuspath.find("Bus Location");
    if (devnamelength == std::string::npos) 
        devnamelength = newstringtosearch.length() - bus_length; //Case when we've reached the end of the stringtosearch.

    std::string newstr = newstringtosearch.substr(0, bus_length + devnamelength); //gives us stringtosearch without bus location

    std::pair <std::string, std::string> rvalue_pair;
    std::string devicepath = newstr.substr(0, bus_length);
    std::string device = newstr.substr(bus_length, devnamelength);
    rvalue_pair = { devicepath, device };

    *offset = stringtosearch.find("Device Name") + devnamelength;
    return rvalue_pair;

}
std::string regextrim(std::string basestring, std::string regexkey){
    std::regex matchkey(regexkey);
    std::smatch searchobj;
    std::regex_search(basestring, searchobj, matchkey);

    std::string rvalue;
    rvalue = searchobj.prefix();
    return rvalue;
}
std::pair<std::string, std::string> getpair(std::string stringtosearch, size_t* offset){
    std::string buslocationstr = "Bus Location";
    std::string devnamestr = "Device Name";

    //
    //Trim beginning of string.
    //
    size_t devicename_position = stringtosearch.find(devnamestr);
    if (devicename_position == std::string::npos){
        return{ "", "" }; ///If string not found, return none.
    }
    std::string newstringtosearch = stringtosearch.substr(devicename_position);

    //
    //Get length of device name by looking at 0-->'Bus Location'
    //
    size_t buspath_position = newstringtosearch.find(buslocationstr);
    if (buspath_position == std::string::npos){
        return{ "", "" };///If string not found, return none.
    }
    size_t devicename_length = buspath_position - newstringtosearch.find(devnamestr);

    //
    //Get length of bus path by removing device  name and searching for '>' which is at the end of that string
    //
    std::string stringlessdevnamelength = newstringtosearch.substr(devicename_length);
    size_t buspath_length = stringlessdevnamelength.find(">") + 1;

    //
    //Trim string to contain only our two  paths. Clean up values and return.
    //
    std::string newstr = newstringtosearch.substr(0, devicename_length + buspath_length);

    std::pair <std::string, std::string> rvalue_pair;
    std::string devicepath = newstr.substr(devicename_length, buspath_length);
    std::string device = newstr.substr(0,devicename_length);
    //Trim up device
    if (device.find("\\n") != std::string::npos) device = device.erase(device.find("\\n"), std::string("\\n").size());
    if (device.find("<")   != std::string::npos) device = device.substr(device.find("<") + 1);
    if (device.find(">")   != std::string::npos) device = device.substr(0, device.find(">"));

    //Trim ending path off of bus location ('#USB(*)'), since that is the location, not the path, of the device
    //and we need to match the locations.
    devicepath = regextrim(devicepath, "\\#USB\\([0-9]?[0-9]?[0-9]?[0-9]?[0-9]\\)\\>");
    rvalue_pair = { devicepath, device };

    *offset = stringtosearch.find(buslocationstr) + buspath_length;
    return rvalue_pair;

}


std::vector<std::pair<std::string, std::string>>
getpath_name_pair_v(std::string searchkey, std::string stringtosearch){
    std::vector<std::pair<std::string, std::string>> rvalue_pair_v;
    rvalue_pair_v.clear();
    std::pair<std::string, std::string> bus_device_pair;
    bus_device_pair = { "", "" };
    //bool exitflag = false;
    size_t offset = 0;
    do{
        if (offset > stringtosearch.length()){
            break;
        }
        stringtosearch = stringtosearch.substr(offset);
        bus_device_pair = getpair(stringtosearch, &offset);

        if (bus_device_pair.first == ""){ ///"" means that strings not found.
            break;
        }
        else if (std::string::npos == bus_device_pair.second.find(searchkey)){
            continue;
        }
        else{ //Else add the pair

            rvalue_pair_v.push_back(bus_device_pair);
        }
    } while (true);

    return rvalue_pair_v;
}
//find value in string. if it's there, push path and string pair into vector
//if there's a match with trimmed string, pass trimmed string to same function to recurse.

/*
#if defined (EUD_WIN_ENV)
std::vector<periphtree_t*> DeviceMgr::ParseEudIdIntoTreeList(std::string eud_id_output){
    std::vector<periphtree_t*> tree_v;

    //first, get and store all control devices
    basename_v::iterator devicebasename_iterator;
    devicebasename_iterator = DeviceBaseNameTreeInstance->devicebasenames_p->begin();
    //Used to remove executable name  from file. Not needed at this time.
    //std::string eud_idstring = eud_id_output_p->substr(eud_id_output_p->find(EUD_ID_EXE) + std::string(EUD_ID_EXE).length());
    std::vector<std::pair<std::string, std::string>> path_devname_pair_v = \
        getpath_name_pair_v(*devicebasename_iterator, eud_id_output);

    //Iterate through found ctl devices and create tree objects for them.
    //Use this temporary string to store existing ctl peripherals and compare to new ones to be stored
    //so that we don't store duplicate names.
    std::string ctl_devicenamelist;
    std::vector<std::pair<std::string, std::string>>::iterator path_devname_pair_v_iterator;
    for (path_devname_pair_v_iterator = path_devname_pair_v.begin(); \
        path_devname_pair_v_iterator != path_devname_pair_v.end(); \
        path_devname_pair_v_iterator++)
    {
        std::pair < std::string, std::string> pairthing = *path_devname_pair_v_iterator;
        std::string deviceusbpath = pairthing.first;
        std::string devicename = pairthing.second;
        if (ctl_devicenamelist.find(devicename) != std::string::npos){
            continue; ///<If we've already stored this devicename, skip this entry.
        }
        ctl_devicenamelist = ctl_devicenamelist + devicename;

        PeriphTree* tree_temp = new PeriphTree;
        tree_temp->ctl_peripheral_devmgrname_p_->clear();
        tree_temp->ctl_peripheral_devmgrname_p_->assign(devicename);
        *tree_temp->deviceusbpath_p = deviceusbpath;

        //Now check that current ctl devicename doesn't match anything already stored.
        std::vector<periphtree_t*>::iterator tree_v_iterator;

        tree_v.push_back(tree_temp);
    }

    for (devicebasename_iterator = (DeviceBaseNameTree::DeviceBaseNameTreeInstance()->devicebasenames_p->begin() + 1);
        devicebasename_iterator != DeviceBaseNameTree::DeviceBaseNameTreeInstance()->devicebasenames_p->end();
        devicebasename_iterator++)
    {
        std::vector<std::pair<std::string, std::string>> path_devname_pair_v = \
            getpath_name_pair_v(*devicebasename_iterator, eud_id_output);


        // for pathnames in tree_v
        //  for path names in path_devname_pair_v
        //      if match, append device.
        std::vector<std::pair<std::string, std::string>>::iterator path_devname_pair_v_iterator;
        std::vector<periphtree_t*>::iterator tree_v_iterator;
        for (path_devname_pair_v_iterator = path_devname_pair_v.begin();
            path_devname_pair_v_iterator != path_devname_pair_v.end();
            path_devname_pair_v_iterator++)
        {
            for (tree_v_iterator = tree_v.begin();
                tree_v_iterator != tree_v.end();
                tree_v_iterator++)
            {
                std::string devicepath = *(*tree_v_iterator)->deviceusbpath_p;
                std::string founddevpairpath = (*path_devname_pair_v_iterator).first;
                if (devicepath.find(founddevpairpath) != std::string::npos){

                    std::string devicename = (*path_devname_pair_v_iterator).second;

                    if (devicename.find("SWD", 0) != std::string::npos)
                        *(*tree_v_iterator)->swd_peripheral_devmgrname_p_ = devicename;
                    else if (devicename.find("JTAG", 0) != std::string::npos)
                        *(*tree_v_iterator)->jtg_peripheral_devmgrname_p_ = devicename;
                    else if (devicename.find("Trace", 0) != std::string::npos)
                        *(*tree_v_iterator)->trc_peripheral_devmgrname_p_ = devicename;
                    else
                        *(*tree_v_iterator)->com_peripheral_devmgrname_p_ = devicename;


                }
            }
        }


    }

    return tree_v;
}
#endif
*/


std::vector<periphtree_t*> DeviceMgr::ParseEudIdIntoTreeList(std::vector<eud_device_info> eud_tree_v){
    std::vector<periphtree_t*> tree_v;

    //first, get and store all control devices
    basename_v::iterator devicebasename_iterator;
    devicebasename_iterator = device_basename_tree_instance_->devicebasenames_p_->begin();
    //Used to remove executable name  from file. Not needed at this time.
    //std::string eud_idstring = eud_id_output_p->substr(eud_id_output_p->find(EUD_ID_EXE) + std::string(EUD_ID_EXE).length());

    //Iterate through found ctl devices and create tree objects for them.
    //Use this temporary string to store existing ctl peripherals and compare to new ones to be stored
    //so that we don't store duplicate names.
    std::string ctl_devicenamelist;
    std::vector<eud_device_info>::iterator path_devname_pair_v_iterator;

    for (path_devname_pair_v_iterator = eud_tree_v.begin(); \
        path_devname_pair_v_iterator != eud_tree_v.end(); \
        path_devname_pair_v_iterator++)
    {
        if((path_devname_pair_v_iterator->deviceType)==DEVICETYPE_EUD_CTL){
            std::string deviceusbpath = path_devname_pair_v_iterator->devicePath;
            usb_dev_access_type devicehandle = (path_devname_pair_v_iterator)->deviceHandle;
            //if (ctl_devicenamelist.find(devicename) != std::string::npos){
            //    continue; ///<If we've already stored this devicename, skip this entry.
            //}
            //ctl_devicenamelist = ctl_devicenamelist + devicename;

            PeriphTree* tree_temp = new PeriphTree;


            //  DW: Just for WIN tree_temp->ctl_peripheral_devmgrname_p_->clear();
            *tree_temp->ctl_peripheral_devmgrname_p_ = devicehandle;
            *tree_temp->trc_peripheral_devmgrname_p_ = NULL;
            *tree_temp->swd_peripheral_devmgrname_p_ = NULL;
            *tree_temp->jtg_peripheral_devmgrname_p_ = NULL;
            *tree_temp->com_peripheral_devmgrname_p_ = NULL;
            *tree_temp->device_usb_path_p_ = deviceusbpath;

            //Now check that current ctl devicename doesn't match anything already stored.
            std::vector<periphtree_t*>::iterator tree_v_iterator;

            tree_v.push_back(tree_temp);
        }
    }


    std::vector<periphtree_t*>::iterator ctl_dev_iterator;
    for (ctl_dev_iterator = tree_v.begin(); \
        ctl_dev_iterator != tree_v.end(); \
        ctl_dev_iterator++)
    {
        std::string ctl_path = *(*ctl_dev_iterator)->device_usb_path_p_;

        std::vector<eud_device_info>::iterator eud_dev_iterator;
        for (eud_dev_iterator = eud_tree_v.begin(); \
            eud_dev_iterator != eud_tree_v.end(); \
            eud_dev_iterator++)
        {

            if (eud_dev_iterator->deviceType != DEVICETYPE_EUD_CTL) {
                if (eud_dev_iterator->devicePath == ctl_path)
                {
                    // do something
                    if (eud_dev_iterator->deviceType == DEVICETYPE_EUD_SWD)
                        *(*ctl_dev_iterator)->swd_peripheral_devmgrname_p_ = eud_dev_iterator->deviceHandle;
                    else if (eud_dev_iterator->deviceType == DEVICETYPE_EUD_JTG)
                        *(*ctl_dev_iterator)->jtg_peripheral_devmgrname_p_ = eud_dev_iterator->deviceHandle;
                    else if (eud_dev_iterator->deviceType == DEVICETYPE_EUD_TRC)
                        *(*ctl_dev_iterator)->trc_peripheral_devmgrname_p_ = eud_dev_iterator->deviceHandle;
                    else
                        *(*ctl_dev_iterator)->com_peripheral_devmgrname_p_ = eud_dev_iterator->deviceHandle;

                }
            }
        }


    }


    /* Print the Tree */
    QCEUD_Print( "\n ### Devices Tree Start ### \n ");

    for (ctl_dev_iterator = tree_v.begin(); \
        ctl_dev_iterator != tree_v.end(); \
        ctl_dev_iterator++)
    {

        QCEUD_Print( "*** Device ***  \n ");
        QCEUD_Print( " Devices_ID  %x \n ", (*ctl_dev_iterator)->device_id_);
        QCEUD_Print( " CTL Device  %x \n ", *(*ctl_dev_iterator)->ctl_peripheral_devmgrname_p_);
        QCEUD_Print( " TRC Device  %x \n ", *(*ctl_dev_iterator)->trc_peripheral_devmgrname_p_);
        QCEUD_Print( " SWD Device  %x \n ", *(*ctl_dev_iterator)->swd_peripheral_devmgrname_p_);
        QCEUD_Print( " JTG Device  %x \n ", *(*ctl_dev_iterator)->jtg_peripheral_devmgrname_p_);
        QCEUD_Print( " COM Device  %x \n ", *(*ctl_dev_iterator)->com_peripheral_devmgrname_p_);

      //  std::string   bus_path = *(*ctl_dev_iterator)->deviceusbpath_p;
      //  QCEUD_Print( " BusPath  %s \n ", *(bus_path.c_str())) ;
    }

    QCEUD_Print( "### Devices Tree End ### \n ");
    return tree_v;
}

std::string manual_devmgr_device_scan(EUD_ERR_t* err_p) {
    EUD_ERR_t err;
    uint32_t i;
    char* devicemgrname_p = new char[MAX_DEVICESTRING_SIZE];
    memset(devicemgrname_p, '\0', MAX_DEVICESTRING_SIZE);

    std::string devicestr = PRELOAD_READ_PIPE_VALUE;

    for (i = DEVICETYPE_EUD_CTL; i < DEVICETYPE_EUD_MAX; i++) {

        err = DeviceManagerPoller(i, devicemgrname_p, false);
        if (err == EUD_USB_DEVICE_NOT_DETECTED) {
            if (i == DEVICETYPE_EUD_CTL)
                return devicestr;   ///<If there are no ctl devices, exit.
            else
                continue;
        }

        devicestr += "Device Name <" + std::string(devicemgrname_p) + ">\n" + NULL_BUS_PATH;

    }
    return devicestr;

}

/* Function to perform a scan for attached EUD peripherals / devices to skip unless done already */
EUD_ERR_t DeviceMgr::ScanDevices(void)
{
    EUD_ERR_t err = 0;

   //Get all active Device ID's. create a new dictionary entry for each.
    std::vector<eud_device_info> *devices_v = NULL;
    devices_v = usb_scan_devices();

    //Get all active Device ID's. create a new dictionary entry for each.
    std::vector<periphtree_t*> tree_list_v = ParseEudIdIntoTreeList(*devices_v);

    device_map_.clear();

    if (tree_list_v.size() == 0){
        return EUD_SUCCESS;  ///<Simply clear device_map_ and return if no devices are present.
    }

    CtlEudDevice ctl_device = CtlEudDevice();
    std::vector<periphtree_t*>::iterator tree_v_iterator;

    for (tree_v_iterator = tree_list_v.begin();
        tree_v_iterator != tree_list_v.end();
        tree_v_iterator++)
    {
        err = (EUD_ERR_t)ctl_device.UsbOpen(*(*tree_v_iterator)->ctl_peripheral_devmgrname_p_, DEVICETYPE_EUD_CTL, 20);

        //If device is functioning properly, add it to device_map_.
        //Else, mark error and don't add.
        if (err == EUD_SUCCESS) {

            err = ctl_device.WriteCommand(CTL_CMD_DEVICE_ID_READ, &(*tree_v_iterator)->device_id_);
            if (err == EUD_SUCCESS) {
                device_map_[(*tree_v_iterator)->device_id_] = *tree_v_iterator;
            }
            else {
                QCEUD_Print("Error detected when attempting to retrieve device_id for device. Error: %x. Device will not be added to available devices.\n",  err);
                eud_set_last_error(EUD_ERR_CTL_ENUMERATION_FAILED);
            }

        }
        else{

            QCEUD_Print("Error detected when attempting to connect with Error: %x. Device will not be added to available devices.\n", err);

            eud_set_last_error(EUD_ERR_CTL_ENUMERATION_FAILED);
        }
    }

/*
    EUD_ERR_t err = 0;
    std::string eud_id_result = "";// = new std::string;
    size_t  bad_device_id_iterator = 0;

    //
    //  Retrieve EUD ID String
    //
    std::string currdir = std::string(CurrentWorkingDir);
    currdir = currdir.substr(0, currdir.length() - std::string(EUD_EXECUTABLE_NAME).length());

    std::string eud_id_path1 = std::string(currdir + std::string("\\") + std::string(EUD_ID_EXE)).c_str();
    std::string eud_id_path2 = std::string(currdir + std::string("..\\..") + std::string("\\") + std::string(EUD_ID_EXE)).c_str();
    std::string eud_id_path3 = std::string(currdir + std::string("..\\..\\..") + std::string("\\") + std::string(EUD_ID_EXE)).c_str();

    std::ifstream f1(eud_id_path1);
    std::ifstream f2(eud_id_path2);
    std::ifstream f3(eud_id_path3);

    std::string good_eud_id_path = f1.good() ? eud_id_path1 : \
        f2.good() ? eud_id_path2 : \
        f3.good() ? eud_id_path3 : \
        std::string("");

    if (good_eud_id_path == std::string("")) {
        return eud_set_last_error(EUD_ERR_EUD_ID_EXE_NOT_FOUND);
    }

    eud_id_result += retrieve_eud_id_string(&err, good_eud_id_path, RETRIEVE_EUD_ID_OPTIONS_NULL);

    if (eud_id_result.length() < (std::string(PRELOAD_READ_PIPE_VALUE).length() + 5)) {
        eud_id_result += retrieve_eud_id_string(&err, good_eud_id_path, RETRIEVE_EUD_ID_OPTIONS_LONGWAIT);
    }

    //If we didn't see any devices, manually scan through potential Device Manager names to see if we can get a match
    if (eud_id_result.length() < (std::string(PRELOAD_READ_PIPE_VALUE).length()*2 + 5)) {
        eud_id_result += manual_devmgr_device_scan(&err);
    }




    //uint32_t length = eud_id_result->length();
    //uint32_t reflength = std::string(PRELOAD_READ_PIPE_VALUE).length() + 10;
    //
    //  Parse EUD ID String into device_map_
    //
    //Get all active Device ID's. create a new dictionary entry for each.
    std::vector<periphtree_t*> tree_list_v = ParseEudIdIntoTreeList(eud_id_result);

    device_map_.clear();

    if (tree_list_v.size() == 0){
    //if (length < reflength) {
        return EUD_SUCCESS;  ///<Simply clear device_map_ and return if no devices are present.
    }




    //delete  eud_id_result;

    CtlEudDevice ctl_device = CtlEudDevice();

    std::vector<periphtree_t*>::iterator tree_v_iterator;

    for (tree_v_iterator = tree_list_v.begin();
        tree_v_iterator != tree_list_v.end();
        tree_v_iterator++)
    {

        std::string ctl_periphaccessname = getdevicemgraccessstring(*(*tree_v_iterator)->ctl_peripheral_devmgrname_p_);
        err = (EUD_ERR_t)ctl_device.usb_open(ctl_periphaccessname.c_str(), DEVICETYPE_EUD_CTL, 20);

        //If device is functioning properly, add it to device_map_.
        //Else, mark error and don't add.
        if (err == EUD_SUCCESS) {

            err = ctl_device.WriteCommand(CTL_CMD_DEVICE_ID_READ, &(*tree_v_iterator)->device_id);
            if (err == EUD_SUCCESS) {
                device_map_[(*tree_v_iterator)->device_id] = *tree_v_iterator;
            }
            else {
                QCEUD_Print("Error detected when attempting to retrieve device_id for device: %s. Error: %x. Device will not be added to available devices.\n", ctl_periphaccessname.c_str(), err);
                eud_set_last_error(EUD_ERR_CTL_ENUMERATION_FAILED);
            }

        }
        else{

            QCEUD_Print("Error detected when attempting to connect with %s. Error: %x. Device will not be added to available devices.\n", ctl_periphaccessname.c_str(), err);

            eud_set_last_error(EUD_ERR_CTL_ENUMERATION_FAILED);

        }

    } //End for loop

*/
    return EUD_SUCCESS;

}


EUD_ERR_t get_set_and_clear_bits(uint32_t devicetype, uint32_t* ctl_setbits_p, uint32_t* ctl_clearbits_p, uint32_t action){


    switch (devicetype){
    case DEVICETYPE_EUD_CTL:
        ctl_setbits_p = NULL;
        ctl_clearbits_p = NULL;
    case DEVICETYPE_EUD_JTG:
        //devname = "JTG";
        if (action == ENABLE) {

            *ctl_setbits_p = CTL_PAYLOAD_JTAGON;
            *ctl_clearbits_p = CTL_BITSCLEAR_JTAGON;
        }
        else if (action == DISABLE) {
            *ctl_setbits_p = CTL_PAYLOAD_JTAGOFF;
            *ctl_clearbits_p = CTL_BITSCLEAR_JTAGOFF;
        }
        break;
    case DEVICETYPE_EUD_SWD:
        //devname = "SWD";
        if (action == ENABLE) {
            *ctl_setbits_p = CTL_PAYLOAD_SWDON;
            *ctl_clearbits_p = CTL_BITSCLEAR_SWDON;
        }
        else if (action == DISABLE) {
            *ctl_setbits_p = CTL_PAYLOAD_SWDOFF;
            *ctl_clearbits_p = CTL_BITSCLEAR_SWDOFF;
        }
        break;
    case DEVICETYPE_EUD_COM:
        //devname = "COM";
        if (action == ENABLE) {
            *ctl_setbits_p = CTL_PAYLOAD_COMON;
            *ctl_clearbits_p = CTL_BITSCLEAR_COMON;
        }
        else if (action == DISABLE) {
            *ctl_setbits_p = CTL_PAYLOAD_COMOFF;
            *ctl_clearbits_p = CTL_BITSCLEAR_COMOFF;
        }
        break;
    case DEVICETYPE_EUD_TRC:
        //devname = "TRC";
        if (action == ENABLE) {
            *ctl_setbits_p = CTL_PAYLOAD_TRCON;
            *ctl_clearbits_p = CTL_BITSCLEAR_TRCON;
        }
        else if (action == DISABLE) {
            *ctl_setbits_p = CTL_PAYLOAD_JTAGOFF;
            *ctl_clearbits_p = CTL_BITSCLEAR_JTAGOFF;
        }
        break;
    default:
        return eud_set_last_error(EUD_BAD_PARAMETER);

    }

    return EUD_SUCCESS;

}

EUD_ERR_t toggle_peripheral(std::string ctl_devname, uint32_t device_id, uint32_t devicetype, uint32_t action){
    EUD_ERR_t err;

    CtlEudDevice* ctl_handle_p = new CtlEudDevice;
    err = (EUD_ERR_t)ctl_handle_p->UsbOpen(ctl_devname.c_str(), device_id, EUD_PERIPH_OPEN_DEFAULT_MAXWAIT);

    ///////////////////////////
    //Get set and clear bits depending on device_id.
    uint32_t ctl_setbits = 0;
    uint32_t ctl_clearbits = 0;


    if ((err = get_set_and_clear_bits(device_id, &ctl_setbits, &ctl_clearbits, action))!=0) 
    {
        return err;
    }
    ///////////////////////////////////////
    /// Write to control periph to enable desired peripheral
    if (devicetype != DEVICETYPE_EUD_CTL)
    {
        err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_CLR, ctl_clearbits);
        err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_SET, ctl_setbits);
    }
    ctl_handle_p->UsbClose();
    delete ctl_handle_p;

    return err;

}

EUD_ERR_t toggle_peripheral(CtlEudDevice* ctl_handle_p, uint32_t device_id, uint32_t devicetype, uint32_t action){

    EUD_ERR_t err;

    ///////////////////////////
    //Get set and clear bits depending on device_id.
    uint32_t ctl_setbits = 0;
    uint32_t ctl_clearbits = 0;


    if ((err = get_set_and_clear_bits(devicetype, &ctl_setbits, &ctl_clearbits, action))!=0) return err;
    ///////////////////////////////////////
    /// Write to control periph to enable desired peripheral
    if (devicetype != DEVICETYPE_EUD_CTL){
        err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_CLR, ctl_clearbits);
        err = ctl_handle_p->WriteCommand(CTL_CMD_CTLOUT_SET, ctl_setbits);
    }

    return err;

}
EUD_ERR_t DeviceMgr::ForceOffUsbDevice(uint32_t device_id, uint32_t devicetype) {
    EUD_ERR_t err;
    CtlEudDevice* ctl_device_p = GetCtlPeripheralByDeviceId(device_id, &err);
    if (ctl_device_p == NULL) {
        return err;
    }
    //Can't force a ctl device off.
    if (devicetype == DEVICETYPE_EUD_CTL) {
        return eud_set_last_error(EUD_ERR_BAD_PARAMETER);
    }

    if ((err = toggle_peripheral(ctl_device_p, device_id, devicetype, DISABLE))!=0) return err;

    eud_close_peripheral((PVOID*)ctl_device_p);

    return err;


}
PVOID* DeviceMgr::InitializeUsbDevice(uint32_t device_id, uint32_t devicetype, EUD_ERR_t* errorcode_p){

    //from devicetype, determine which ctl device to access

    CtlEudDevice* ctl_device_p = GetCtlPeripheralByDeviceId(device_id, errorcode_p);
    if (ctl_device_p == NULL){
        return NULL;
    }
    //If it's ctl device wanted, just return that.
    if (devicetype == DEVICETYPE_EUD_CTL){
        return (PVOID*)ctl_device_p;
    }
    // from device type, determine which device to turn on. issue those commands.
    if ((*errorcode_p = toggle_peripheral(ctl_device_p, device_id, devicetype, ENABLE))!=0) return NULL;

    eud_close_peripheral((PVOID*)ctl_device_p);

    //This delay is required before attempting USB connection for EUD SWD Node
    usleep(150 * 1000);

    periphtree_t* devtree_p;
    uint32_t counter;
    for (counter = 0; counter < MAX_EUD_PERIPH_ENABLE_RETRIES; counter++) {

        usleep(EUD_PERIPH_ENABLE_SLEEP_TIME * 1000);
        devtree_p = GetDeviceTreeByDeviceId(device_id);

        if ((devtree_p != NULL) || (*devtree_p->swd_peripheral_devmgrname_p_ != NULL))
            break;
    }
    if (devtree_p == NULL){
        *errorcode_p = eud_set_last_error(EUD_ERR_DEVICEID_NOT_FOUND);
        return NULL;
    }

    JtagEudDevice* jtg_device_handle;
    SwdEudDevice* swd_device_handle;
    TraceEudDevice* trc_device_handle;
    ComEudDevice* com_device_handle;

    //we want ctl device to remain until we explicitly delete it
    switch (devicetype){
    case DEVICETYPE_EUD_JTG:

        jtg_device_handle = new JtagEudDevice();

        /*if (!(*devtree_p->jtg_peripheral_devmgrname_p_)) {
            *errorcode_p =  eud_set_last_error(EUD_ERR_INITIALIZED_DEVICE_NOT_FOUND);
            return NULL;
        }*/

        jtg_device_handle->device_name_ = getdevicemgraccessstring(*devtree_p->jtg_peripheral_devmgrname_p_);
        jtg_device_handle->ctl_peripheral_name_ = getdevicemgraccessstring(*devtree_p->ctl_peripheral_devmgrname_p_);
        jtg_device_handle->device_id_ = device_id;

        *errorcode_p = jtg_device_handle->UsbOpen();

        if (*errorcode_p != EUD_SUCCESS){
            
            jtg_device_handle = NULL;
            QCEUD_Print("INIT_%s_EUD_DEVICE error - usb_open failed, error code %x\n", "JTG", *errorcode_p);
            return NULL;
        }
        else{
            return (PVOID*)jtg_device_handle;
        }

        break;

    case DEVICETYPE_EUD_SWD:

        swd_device_handle = new SwdEudDevice();

        /*if (!(*devtree_p->swd_peripheral_devmgrname_p_ )) {
            *errorcode_p = eud_set_last_error(EUD_ERR_INITIALIZED_DEVICE_NOT_FOUND);
            return NULL;
        }*/

        swd_device_handle->device_name_ = getdevicemgraccessstring(*devtree_p->swd_peripheral_devmgrname_p_);
        swd_device_handle->ctl_peripheral_name_ = getdevicemgraccessstring(*devtree_p->ctl_peripheral_devmgrname_p_);
        swd_device_handle->device_id_ = device_id;
        //Sleep(2000);
        *errorcode_p = swd_device_handle->UsbOpen();

        if (*errorcode_p != EUD_SUCCESS){
            //Try again
            Sleep(2000);
            *errorcode_p = swd_device_handle->UsbOpen();
            if (*errorcode_p != EUD_SUCCESS) {
                //delete swd_device_handle; //<Don't double delete
                swd_device_handle = NULL;
                QCEUD_Print("INIT_%s_EUD_DEVICE error - usb_open failed, error code %x\n", "SWD", *errorcode_p);
                *errorcode_p = *errorcode_p;
                return NULL;
            }
        }
        else{
            return (PVOID*)swd_device_handle;
        }

        break;

    case DEVICETYPE_EUD_COM:

        com_device_handle = new ComEudDevice;
        com_device_handle->ctl_peripheral_name_ = getdevicemgraccessstring(*devtree_p->ctl_peripheral_devmgrname_p_);
        com_device_handle->device_name_ = getdevicemgraccessstring(*devtree_p->com_peripheral_devmgrname_p_);
        //*errorcode_p = (EUD_ERR_t)com_device_handle->UsbOpen();
        if ((*errorcode_p = (EUD_ERR_t)com_device_handle->UsbOpen(EUD_COM_DEVMGR_NAME, com_device_handle->device_type_, 100))!=0){

            delete com_device_handle;
            com_device_handle = NULL;
            QCEUD_Print("INIT_%s_EUD_DEVICE error - usb_open failed, error code %x\n", "COM", *errorcode_p);
            *errorcode_p = *errorcode_p;
            return NULL;
        }
        else{
            return (PVOID*)com_device_handle;
        }

        break;

    case DEVICETYPE_EUD_TRC:

        trc_device_handle = new TraceEudDevice();

        trc_device_handle->device_name_ = getdevicemgraccessstring(*devtree_p->trc_peripheral_devmgrname_p_);
        trc_device_handle->ctl_peripheral_name_ = getdevicemgraccessstring(*devtree_p->ctl_peripheral_devmgrname_p_);
        trc_device_handle->device_id_ = device_id;

        *errorcode_p = trc_device_handle->UsbOpen();

        if (*errorcode_p != EUD_SUCCESS){
            delete trc_device_handle;
            trc_device_handle = NULL;
            QCEUD_Print("INIT_%s_EUD_DEVICE error - usb_open failed, error code %x\n", "TRC", *errorcode_p);
            *errorcode_p = *errorcode_p;
            return NULL;
        }
        else{
            return (PVOID*)trc_device_handle;
        }

        break;

        //Should never get here
    default:
        return NULL;

    }

    //Should never get here
    return NULL;

}

    // attach to that device. add it to the appropriate device tree
    // initialize the device, return its pointer.


//if i have, say, a swd peripheral devmgr name, and i want the device ID that maps to it.
EUD_ERR_t DeviceMgr::GetDeviceIdByDevMgrName(char* devmgrname, uint32_t* device_id_p){
    return eud_set_last_error(EUD_ERR_FUNCTION_NOT_AVAILABLE);



 //   map<string, Car>::const_iterator it = cars.find(name);
 //   return it != cars.end();
}
EUD_ERR_t DeviceMgr::get_device_id_array(uint32_t* array_p, uint32_t* length_p){
    EUD_ERR_t err;

    if ((err = ScanDevices())!=0) return err;

    EUDDevicesMap_t::iterator devmap_iterator;

    uint32_t idx = 0;
    for (devmap_iterator = device_map_.begin(); \
        devmap_iterator != device_map_.end(); \
        devmap_iterator++)
    {
        *(array_p + idx) = (*devmap_iterator).first;
        idx++;
    }

    *length_p = (uint32_t)device_map_.size();

    return EUD_SUCCESS;

}
///lookup ctl peripheral device mgr name by eud peripheral device ptr.
EUD_ERR_t DeviceMgr::GetCtlPeripheralByPeripheralPtr(PVOID* EudDevice, char*ctl_peripheral_devname){
    return eud_set_last_error(EUD_ERR_FUNCTION_NOT_AVAILABLE);
}
/*
///e.g. i want to acess a swd device by serial number
EUD_ERR_t DeviceMgr::getperipheralbydeviceid(uint32_t device_id,char* devname){
    return eud_set_last_error(EUD_ERR_FUNCTION_NOT_AVAILABLE);
}
*/

EUD_ERR_t DeviceMgr::GetAttachedDevicesString(char* stringbuffer, uint32_t* stringsize_p){

    EUD_ERR_t err;

    if ((stringsize_p == NULL) || (stringbuffer == NULL)){
        return eud_set_last_error(EUD_ERR_NULL_POINTER);
    }

    //Initialize buffer on stack, to be copied to and cleaned up after function call.
    //TCHAR tempstring[200];
    //TCHAR* tempstring_p = "c:\\temp\\stuff.txt";
    if ((err = ScanDevices())!=0) return err;

    std::string resultstring;
    std::stringstream result_ss;
    resultstring.clear();
    EUDDevicesMap_t::iterator devmap_iterator;
    for (devmap_iterator = device_map_.begin();\
        devmap_iterator != device_map_.end();\
        devmap_iterator++)
    {
        result_ss << "Device ID: " << std::hex << (*devmap_iterator).first << "\n";
        result_ss << "Devices:\n";
        /*
        if (*(*devmap_iterator).second->ctl_peripheral_devmgrname_p_ != "")
            result_ss << "\t" << *(*devmap_iterator).second->ctl_peripheral_devmgrname_p_ << "\n";
        if (*(*devmap_iterator).second->jtg_peripheral_devmgrname_p_ != "")
            result_ss << "\t" << *(*devmap_iterator).second->jtg_peripheral_devmgrname_p_ << "\n";
        if (*(*devmap_iterator).second->swd_peripheral_devmgrname_p_ != "")
            result_ss << "\t" << *(*devmap_iterator).second->swd_peripheral_devmgrname_p_ << "\n";
        if (*(*devmap_iterator).second->trc_peripheral_devmgrname_p_ != "")
            result_ss << "\t" << *(*devmap_iterator).second->trc_peripheral_devmgrname_p_ << "\n";
        if (*(*devmap_iterator).second->com_peripheral_devmgrname_p_ != "")
            result_ss << "\t" << *(*devmap_iterator).second->com_peripheral_devmgrname_p_ << "\n";
        */
        if (*(*devmap_iterator).second->ctl_peripheral_devmgrname_p_ != NULL)
            result_ss << EUD_CTL_DEVMGR_BASENAME  << "\t(" << *(*devmap_iterator).second->ctl_peripheral_devmgrname_p_ << ")\n";
        if (*(*devmap_iterator).second->jtg_peripheral_devmgrname_p_ != NULL)
            result_ss << EUD_JTG_DEVMGR_BASENAME  << "\t("<< *(*devmap_iterator).second->jtg_peripheral_devmgrname_p_ << "\n";
        if (*(*devmap_iterator).second->swd_peripheral_devmgrname_p_ != NULL)
            result_ss << EUD_SWD_DEVMGR_BASENAME  << "\t(" << *(*devmap_iterator).second->swd_peripheral_devmgrname_p_ << "\n";
        if (*(*devmap_iterator).second->trc_peripheral_devmgrname_p_ != NULL)
            result_ss << EUD_TRC_DEVMGR_BASENAME  << "\t(" << *(*devmap_iterator).second->trc_peripheral_devmgrname_p_ << "\n";
        if (*(*devmap_iterator).second->com_peripheral_devmgrname_p_ != NULL)
            result_ss << EUD_COM_DEVMGR_BASENAME  << "\t(" << *(*devmap_iterator).second->com_peripheral_devmgrname_p_ << "\n";

    }

    resultstring = result_ss.str();
    *stringsize_p = (uint32_t)resultstring.size();
    sprintf_s(stringbuffer,MAX_DEVICESTRING_SIZE,resultstring.c_str());
#if 0
#if 0
    TCHAR szCmdline[] = TEXT("c:\\temp\\stuff32.txt");


    //sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Device ID: 0xabcd1234: \n\tEUD Control Device 9501 (0023)\n\tEUD SWD Device 0594 (0055)\nEUD Device ID: 0xabcd1238: \n\tEUD Control Device 9501 (0000)\n\tEUD Trace Device 9503 (0023)\n");
    //int myfunc(int argc, TCHAR *argv[]);
    std::string result = myfunc(200, &szCmdline[0]);
    char tempstring[10000];
    sprintf_s(tempstring, 10000, result.c_str());

    //*stringsize_p = (uint32_t)strlen(tempstring) + 1; //Add 1 for terminating character
    //*stringsize_p = result.length();

    //*stringsize_p = (uint32_t)strlen(tempstring) + 1; //Add 1 for terminating character
    *stringsize_p = (uint32_t)result.length() + 1;
#else
    char tempstring[10000];
    sprintf_s(tempstring, MAX_ERR_STRING_SIZE, "EUD Device ID: 0xabcd1234: \n\tEUD Control Device 9501 (0023)\n\tEUD SWD Device 0594 (0055)\nEUD Device ID: 0xabcd1238: \n\tEUD Control Device 9501 (0000)\n\tEUD Trace Device 9503 (0023)\n");
    *stringsize_p = (uint32_t)strlen(tempstring) + 1;
#endif

    memcpy(stringbuffer, tempstring, *stringsize_p);

    //memcpy(stringbuffer, tempstring, *stringsize_p);
#endif
    return EUD_SUCCESS;

}
EUD_ERR_t force_off_usb_device(uint32_t device_id, uint32_t devicetype) {

    EUD_ERR_t rvalue = EUD_SUCCESS;

    #if defined ( EUD_WIN_ENV )
    DWORD dwCount = 0, dwWaitResult;

    dwWaitResult = WaitForSingleObject(
        DeviceMgr::DeviceMgrInstance()->ghMutex_,
        INFINITE);

    switch (dwWaitResult) {
    case WAIT_OBJECT_0:
        try {
            dwCount++;
            rvalue = DeviceMgr::DeviceMgrInstance()->ForceOffUsbDevice(device_id, devicetype);
        }
        catch (...){
            if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)) {
                rvalue = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
            }
        }
        if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)) {
            rvalue = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
        }
        break;
    case WAIT_ABANDONED:
        rvalue = eud_set_last_error(EUD_ERR_USB_INIT_FAILED_MUTEX_ABANDONED);
        break;
    }

    #elif defined (EUD_LNX_ENV)
        rvalue = DeviceMgr::DeviceMgrInstance()->ForceOffUsbDevice(device_id, devicetype);
    #endif

    return rvalue;

}
PVOID* initialize_usb_device(uint32_t device_id, uint32_t devicetype, EUD_ERR_t* err_p){

    PVOID* rvalue_p = NULL;

    #if defined ( EUD_WIN_ENV )
    DWORD dwCount = 0, dwWaitResult;

    dwWaitResult = WaitForSingleObject(
        DeviceMgr::DeviceMgrInstance()->ghMutex_,
        INFINITE);

    switch (dwWaitResult){
    case WAIT_OBJECT_0:
        try{
            dwCount++;
            rvalue_p = DeviceMgr::DeviceMgrInstance()->InitializeUsbDevice(device_id, devicetype, err_p);
        }
        catch (...){
            if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)){
                *err_p = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
                rvalue_p = NULL;
            }
        }
        if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)){
            *err_p = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
            rvalue_p = NULL;
        }
        break;
    case WAIT_ABANDONED:
        *err_p = eud_set_last_error(EUD_ERR_USB_INIT_FAILED_MUTEX_ABANDONED);
        rvalue_p = NULL;
        break;
    }

    #elif defined (EUD_LNX_ENV)
        rvalue_p = DeviceMgr::DeviceMgrInstance()->InitializeUsbDevice(device_id, devicetype, err_p);
    #endif
    return rvalue_p;
}


EUD_ERR_t GetDeviceIdByDevMgrName(char* devmgrname, uint32_t* device_id_p){

    EUD_ERR_t err = EUD_SUCCESS;

    #if defined (EUD_WIN_ENV)
    DWORD dwWaitResult;

    dwWaitResult = WaitForSingleObject(
        DeviceMgr::DeviceMgrInstance()->ghMutex_,
        INFINITE);

    switch (dwWaitResult){
    case WAIT_OBJECT_0:
        try{

            err = DeviceMgr::DeviceMgrInstance()->GetDeviceIdByDevMgrName(devmgrname, device_id_p);
        }
        catch(...){
            if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)){
                err = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
            }
        }
//        finally {
            if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)){
                 err = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
            }
//      }
        break;
    case WAIT_ABANDONED:
        err = eud_set_last_error(EUD_ERR_USB_INIT_FAILED_MUTEX_ABANDONED);
        break;
    }

        #elif defined (EUD_LNX_ENV)
            err = DeviceMgr::DeviceMgrInstance()->GetDeviceIdByDevMgrName(devmgrname, device_id_p);
        #endif
    return err;

}





EUD_ERR_t GetCtlPeripheralByPeripheralPtr(PVOID* EudDevice, char* ctl_peripheral_devname){

    EUD_ERR_t err = EUD_SUCCESS;

    #if defined (EUD_WIN_ENV)
    DWORD dwWaitResult;

    dwWaitResult = WaitForSingleObject(
        DeviceMgr::DeviceMgrInstance()->ghMutex_,
        INFINITE);

    switch (dwWaitResult){
    case WAIT_OBJECT_0:
        try{

            err = DeviceMgr::DeviceMgrInstance()->GetCtlPeripheralByPeripheralPtr(EudDevice, ctl_peripheral_devname);
        }
        catch(...){
            if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)){
                err = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
            }
        }
//        finally {
            if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)){
                err = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
            }
//        }

        break;
    case WAIT_ABANDONED:
        err = eud_set_last_error(EUD_ERR_USB_INIT_FAILED_MUTEX_ABANDONED);
        break;
    }

        #elif defined (EUD_LNX_ENV)
             err = DeviceMgr::DeviceMgrInstance()->GetCtlPeripheralByPeripheralPtr(EudDevice, ctl_peripheral_devname);
        #endif
    return err;

}

EXPORT EUD_ERR_t get_device_id_array(uint32_t* array_p, uint32_t* length_p){

    EUD_ERR_t err = EUD_SUCCESS;

    #if defined (EUD_WIN_ENV)
    DWORD dwWaitResult;

    dwWaitResult = WaitForSingleObject(
        DeviceMgr::DeviceMgrInstance()->ghMutex_,
        INFINITE);

    switch (dwWaitResult){
    case WAIT_OBJECT_0:
        try{
            err = DeviceMgr::DeviceMgrInstance()->get_device_id_array(array_p, length_p);
        }
        catch(...){
            if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)){
                err = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
            }
        }
//        finally {
            if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)){
                err = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
            }
//        }

        break;
    case WAIT_ABANDONED:
        err = eud_set_last_error(EUD_ERR_USB_INIT_FAILED_MUTEX_ABANDONED);
        break;
    }

    #elif defined (EUD_LNX_ENV)
         err = DeviceMgr::DeviceMgrInstance()->get_device_id_array(array_p, length_p);
    #endif
    return err;

}

EXPORT EUD_ERR_t GetAttachedDevicesString(char* stringbuffer, uint32_t* stringsize_p){
    EUD_ERR_t err = EUD_SUCCESS;

    #if defined (EUD_WIN_ENV)
    bool exitflag = false;
    DWORD dwCount = 0, dwWaitResult;

        while ((dwCount < 20) && (exitflag == false)){

            dwWaitResult = WaitForSingleObject(
                DeviceMgr::DeviceMgrInstance()->ghMutex_,
                INFINITE);

            switch (dwWaitResult){
            case WAIT_OBJECT_0:
                try{
                    dwCount++;
                    err = DeviceMgr::DeviceMgrInstance()->GetAttachedDevicesString(stringbuffer, stringsize_p);
                    exitflag = true;
                }
                catch(...){
                    if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)){
                        err = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
                        exitflag = true;
                    }
                }
//              finally {
                    if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)){
                        err = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
                        exitflag = true;
                    }
//              }

                break;

            case WAIT_ABANDONED:
                err = eud_set_last_error(EUD_ERR_USB_INIT_FAILED_MUTEX_ABANDONED);
                break;
            }
        }
    #elif defined (EUD_LNX_ENV)
        err = DeviceMgr::DeviceMgrInstance()->GetAttachedDevicesString(stringbuffer, stringsize_p);
    #endif

    return err;

}


EUD_ERR_t GetCtlPeripheralStringByDeviceId(uint32_t device_id, char* ctl_peripheral_devname_p, uint32_t* length_p){

    EUD_ERR_t err = EUD_SUCCESS;
    /*DWORD dwWaitResult;
    #if defined (EUD_WIN_ENV)
    dwWaitResult = WaitForSingleObject(
        DeviceMgr::DeviceMgrInstance()->ghMutex_,
        INFINITE);

    switch (dwWaitResult){
    case WAIT_OBJECT_0:
        __try{
            err = DeviceMgr::DeviceMgrInstance()->GetCtlPeripheralStringByDeviceId(device_id, ctl_peripheral_devname_p, length_p);
        }
        __finally{
            if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)){
                err = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
            }
        }
        break;
    case WAIT_ABANDONED:
        err = eud_set_last_error(EUD_ERR_USB_INIT_FAILED_MUTEX_ABANDONED);
        break;
    }

    #elif defined (EUD_LNX_ENV)

    #endif*/
    return err;

}

CtlEudDevice* get_ctl_peripheral_by_device_id(uint32_t device_id, EUD_ERR_t* err_p){

   // EUD_ERR_t err = EUD_SUCCESS;
    CtlEudDevice* rvalue_p = NULL;

    #if defined (EUD_WIN_ENV)
    DWORD dwWaitResult;

    dwWaitResult = WaitForSingleObject(
        DeviceMgr::DeviceMgrInstance()->ghMutex_,
        INFINITE);

    switch (dwWaitResult){
    case WAIT_OBJECT_0:
        try{
            rvalue_p = DeviceMgr::DeviceMgrInstance()->GetCtlPeripheralByDeviceId(device_id, err_p);
        }
        catch(...){
            if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)){
                *err_p = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
            }
        }
//        finally {
            if (!ReleaseMutex(DeviceMgr::DeviceMgrInstance()->ghMutex_)){
                *err_p = eud_set_last_error(EUD_ERR_DEVICEMGR_MUTEX_NOT_FREED);
            }
//        }
        break;
    case WAIT_ABANDONED:
        *err_p = eud_set_last_error(EUD_ERR_USB_INIT_FAILED_MUTEX_ABANDONED);
        break;
    }

    #elif defined (EUD_LNX_ENV)
           rvalue_p = DeviceMgr::DeviceMgrInstance()->GetCtlPeripheralByDeviceId(device_id, err_p);
    #endif

    return rvalue_p;
}

/*
EUD_ERR_t InitializeDeviceManager(){


    //Put this within init area. Only done once.
    ghMutex_ = CreateMutex(
        NULL,
        FALSE,
        NULL);

    return 0;
}

*/

