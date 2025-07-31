/*************************************************************************
* 
* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
*
* File: 
*   usb.cpp
*
* Description:                                                              
*   CPP source file for EUD USB Driver
*
***************************************************************************/
#include <assert.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "libusb-1.0/libusb.h"
#if defined ( EUD_WIN_ENV )
	#include <ctime>
#endif

#if defined ( EUD_LNX_ENV )
	#include <stdio.h>
	#include <stdlib.h>
	#include <vector>
#endif

#include "usb.h"

#if defined ( EUD_WIN_ENV )
//===---------------------------------------------------------------------===//
//
// UsbDevice class functions.
//
//===---------------------------------------------------------------------===//
//Constructor
#if not defined(EUD_WIN_LIBUSB)
UsbDevice::UsbDevice() {

	USB_Handle = INVALID_HANDLE_VALUE;
	devmgr_devname = new char[EUD_DEVMGR_MAX_DEVMGR_STRING_LEN];
	usb_device_initialized = FALSE;
	debug = FALSE;
}

//Constructor
UsbDevice::UsbDevice(bool debug_en){
	
	USB_Handle = INVALID_HANDLE_VALUE;
	devmgr_devname = new char[EUD_DEVMGR_MAX_DEVMGR_STRING_LEN];
    usb_device_initialized = FALSE;
	debug = debug_en;
}

//Destructor
UsbDevice::~UsbDevice(){
	usb_close();
}


USB_ERR_t UsbDevice::usb_open(char* error_code)
{
    HANDLE dev_handle = INVALID_HANDLE_VALUE;
	//char dwError;
	DWORD * errcode = new DWORD[1] ;
	USB_ERR_t rvalue;
	
	//CHAR buf1[8] = "\x08";
	CHAR buf1[2] = EUD_CMD_NOP;
	//fixme, not clean
	uint32_t device;
    device = devicetype;
	
	
	if (devmgr_devname == "ERROR")
	{
		QCEUD_Print("Error! Unknown device requested: %d", device);
		//return dev_handle; //returns invalid handle value
	}

    QCEUD_Print("-->UsbDevice::usb_open TID 0x%x <%s>\n",
                GetCurrentThreadId(), devmgr_devname);
	ZeroMemory(&OverlappedForWrite, sizeof(OVERLAPPED));
	ZeroMemory(&OverlappedForRead, sizeof(OVERLAPPED));
    OverlappedForWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    OverlappedForRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

#ifdef ENABLE_USB
    if (NULL != devmgr_devname)
    {

        dev_handle = CreateFile
                     (
                        devmgr_devname,
                        GENERIC_WRITE | GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                        NULL
                     );
    }
	*error_code= (char)GetLastError();
	if (dev_handle == INVALID_HANDLE_VALUE)
	{
		QCEUD_Print("<--UsbDevice::usb_open handle ERROR! Error code: %d. Is device connected?\n", *error_code);
		return EUD_USB_ERROR;
	}
	else
	{
		QCEUD_Print("<--UsbDevice::usb_open handle 0x%x\n", dev_handle);
	}
#else
#endif
	//Assign valid handle to USB_Handle
	USB_Handle = dev_handle;
	rvalue = WriteToDevice(buf1, 1, errcode); //sizeof(buf1));
    
	usb_device_initialized = TRUE;
	
	return rvalue;
	
}
#endif /* EUD_WIN_LIBUSB */

#define IOCTL_QCDEV_REQUEST_DEVICEID CTL_CODE(FILE_DEVICE_UNKNOWN, \
    3352, \
    METHOD_BUFFERED, \
    FILE_ANY_ACCESS);

USB_ERR_t UsbDevice::UsbOpen(char* error_code, const char* devmgrname_local)
{
	HANDLE dev_handle = INVALID_HANDLE_VALUE;
	//char dwError;
	DWORD * errcode = new DWORD[1];
	USB_ERR_t rvalue;

	//CHAR buf1[8] = "\x08";
	CHAR buf1[2] = EUD_CMD_NOP;
	//fixme, not clean
	//uint32_t device;
	//device = deviceID;

	if (strcmp(devmgrname_local, "ERROR") != 0)
	{
		QCEUD_Print("Error! Unknown device requested: %s",devmgrname_local);
		//return dev_handle; //returns invalid handle value
	}

	QCEUD_Print("-->UsbDevice::usb_open TID 0x%x <%s>\n",
		GetCurrentThreadId(), devmgrname_local);
	ZeroMemory(&overlapped_for_write_, sizeof(OVERLAPPED));
	ZeroMemory(&overlapped_for_read_, sizeof(OVERLAPPED));
	overlapped_for_write_.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	overlapped_for_read_.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

#ifdef ENABLE_USB
	if (NULL != devmgrname_local)
	{

		dev_handle = CreateFile
			(
			devmgrname_local,
			GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL
			);
	}
	*error_code = (char)GetLastError();
	if (dev_handle == INVALID_HANDLE_VALUE)
	{
		QCEUD_Print("<--UsbDevice::usb_open handle ERROR! Error code: %d. Is device connected?\n", *error_code);
		return EUD_USB_ERROR;
	}
	else
	{
		QCEUD_Print("<--UsbDevice::usb_open handle 0x%x\n", dev_handle);
	}
#else
#endif
	//Assign valid handle to USB_Handle
	usb_handle_ = dev_handle;
	rvalue = WriteToDevice(buf1, 1, errcode); //sizeof(buf1));
	if (rvalue == EUD_SUCCESS) {
		usb_device_initialized_ = TRUE;
	}
	else {
		usb_device_initialized_ = FALSE;
	}

	return rvalue;

}
/*
void
UsbDevice::usb_close()
{
	if (usb_device_initialized == TRUE){
		QCEUD_Print("UsbDevice::usb_close TID 0x%x\n", GetCurrentThreadId());

		CloseHandle(OverlappedForWrite.hEvent);
		CloseHandle(OverlappedForRead.hEvent);
		CloseHandle(USB_Handle);
		usb_device_initialized = FALSE;
	}
}


USB_ERR_t
UsbDevice::WriteToDevice( PVOID Buffer, DWORD SendSize,DWORD *errcode)
{
#ifdef ENABLE_USB
	if (USB_Handle == NULL){
		return EUD_USB_ERR_HANDLE_UNITIALIZED;
	}
	if (SendSize == 0)
	{
		return USB_ERR_SENDSIZE_0_NODATASENT;
	}
#endif
   BOOL  ret, ioRes;
   USB_ERR_t rvalue = 0;
   DWORD nWritten = 0;
//   DWORD dwError, waitStatus;
   DWORD waitStatus;

   if (debug) {
	   QCEUD_Print("-->UsbDevice::WriteToDevice TID 0x%x %uB\n",
		   GetCurrentThreadId(), SendSize);
   }

#ifdef ENABLE_USB
   ret = WriteFile
         (
            USB_Handle,
            Buffer,
            SendSize,
            &nWritten,
            &OverlappedForWrite
         );

   *errcode = GetLastError();
   if (ERROR_IO_PENDING != *errcode)
   {
      QCEUD_Print("Error: send failure: 0x%x",*errcode);
	  //Check if cast ok
	  return eud_set_last_error(EUD_USB_ERROR_SEND_FAILED);
   }



   if (debug) {
	   QCEUD_Print("Waiting for send confirmation...\n");
   }

   waitStatus = WaitForSingleObject(OverlappedForWrite.hEvent, MAX_WFSO_WRITE_TIME_MS);
   if (waitStatus == WAIT_TIMEOUT) {
	   return eud_set_last_error(EUD_USB_ERROR_SEND_CONFIRMATION_TIMEOUT);
   }

   else if (waitStatus == WAIT_OBJECT_0)
   {
      ioRes = GetOverlappedResult
              (
                 USB_Handle,
                 &OverlappedForWrite,
                 &nWritten,
                 TRUE  // no return until operaqtion completes
              );
      if (!ioRes)
      {
		 *errcode = GetLastError();
		 QCEUD_Print("WriteToDevice: result error 0x%x\n", *errcode);
		 if (*errcode == 0x1f){
			 rvalue = eud_set_last_error(EUD_USB_ERROR_WRITE_DEVICE_ERROR);
		 }
		 else{
			 rvalue = eud_set_last_error(EUD_USB_ERROR_WRITE_FAILED);
		 }
      }
      else
      {
		  if (debug) {
			  USBUTL_PrintBytes(Buffer, nWritten, SendSize, "SendData");
		  }
      }
   }
   else
   {

      QCEUD_Print("Send failed\n");
	  rvalue = eud_set_last_error(EUD_USB_ERROR_SEND_FAILED);
   }
#else
   USBUTL_PrintBytes(Buffer, SendSize, SendSize, "SendData");
#endif
   //EUD_Signal_Parser eud_signal;
   if (debug) {
	   QCEUD_Print("<--UsbDevice::WriteToDevice %uB\n", nWritten);
   }

   //QCEUD_Print2("Command: %s . Size: %uB\n", eud_signal.print_SWD_signal(Buffer), nWritten);


   return rvalue;
}  // WriteToDevice

USB_ERR_t UsbDevice::ReadFromDevice(PVOID Buffer, DWORD ReadSize, DWORD *errcode)
{
#ifndef ENABLE_USB
#define FAKE_USB_RETURN_CHAR 0xfa
	for (uint32_t i = 0; i < ReadSize; i++){
		*((char*)Buffer + i) = (char)FAKE_USB_RETURN_CHAR;
	}
	USBUTL_PrintBytes(Buffer, ReadSize, ReadSize, "FakeReadData");
	//return ReadSize;
	return EUD_SUCCESS;
#else



   BOOL ret, ioRes;
   //DWORD bytesRead = 0; //Declared as public in object for external access.
   DWORD dwError;
   BOOL  bDeviceError = FALSE, waitStatus;

   if (USB_Handle == NULL){
	   return eud_set_last_error(EUD_USB_ERR_HANDLE_UNITIALIZED);
   }

   if (debug) {
	   QCEUD_Print("-->UsbDevice::ReadFromDevice TID 0x%x %uB\n",
		   GetCurrentThreadId(), ReadSize);
   }
   ret = ReadFile
         (
            USB_Handle,
            Buffer,
            ReadSize,
            &bytesRead,
            &OverlappedForRead
        );

   if (!ret)
   {
      switch (dwError = GetLastError())
      {
         case ERROR_IO_PENDING:
         {
            break; //TODO - add timeout
         }
         default:
         {
            bDeviceError = TRUE;
            Sleep(10); // 10ms
            break;
         }
      } // switch
    } // if

    if (bDeviceError == TRUE)
    {

       QCEUD_Print("ReadFromDevice: ReadFile failure 0x%x\n", dwError);
	   return eud_set_last_error(EUD_USB_ERROR_DEVICE_ERROR);
    }

    waitStatus = WaitForSingleObject(OverlappedForRead.hEvent, MAX_WFSO_READ_TIME_MS);
    if (waitStatus == WAIT_OBJECT_0)
    {
       ioRes = GetOverlappedResult
               (
                  USB_Handle,
                  &OverlappedForRead,
                  &bytesRead,
                  TRUE  // no return until operaqtion completes
               );
       if (!ioRes)
       {
          QCEUD_Print("ReadFromDevice: result error 0x%x\n", GetLastError());
       }
       else
       {
		   if (debug) {
			   USBUTL_PrintBytes(Buffer, bytesRead, ReadSize, "ReadData");
		   }
       }
    }
	else if (waitStatus == WAIT_TIMEOUT){
		bytesRead = 0;
		QCEUD_Print("Read failed - timeout on data.\n");
		return eud_set_last_error(EUD_USB_ERROR_READ_TIMEOUT);
	}
    else
    {
       bytesRead = 0;
       QCEUD_Print("Read failed\n");
	   return eud_set_last_error(EUD_USB_ERROR_READ_FAILED_GENERIC);
    }

	if (debug) {
		QCEUD_Print("<--UsbDevice::ReadFromDevice %uB\n", bytesRead);
	}

    return EUD_USB_SUCCESS;
#endif
}
*/
VOID UsbDevice::UsbUtilPrintBytes(PVOID Buf, ULONG len, ULONG PktLen, char *info)
{
   char  buf[1024], *p, cBuf[128], *cp;
   unsigned char *buffer;
   ULONG lastCnt = 0, spaceNeeded;
   ULONG i, s;

   #define SPLIT_CHAR '|'

   buffer = (unsigned char *)Buf;
   RtlZeroMemory(buf, 1024);
   p = buf;
   cp = cBuf;

   if (PktLen < len)
   {
      len = PktLen;
   }

   sprintf_s(p, sizeof(buf),"\r\n\t   --- <%s> DATA %u/%u BYTES ---\r\n", info, len, PktLen);
   p += strlen(p);
   
   for (i = 1; i <= len; i++)
   {
      if (i % 16 == 1)
      {
         sprintf_s(p, (sizeof(buf)-(p-buf)), "  %04u:  ", i - 1);
         p += 9;
      }

	  sprintf_s(p, (sizeof(buf) - (p - buf)), "%02X ", (UCHAR)buffer[i - 1]);
      if (isprint(buffer[i-1]) && (!isspace(buffer[i-1])))
      {
		sprintf_s(cp, (sizeof(cBuf) - (cp - cBuf)), "%c", buffer[i - 1]);
      }
      else
      {
		sprintf_s(cp, (sizeof(cBuf) - (cp - cBuf)), ".");
      }

      p += 3;
      cp += 1;

      if ((i % 16) == 8)
      {
		 sprintf_s(p, (sizeof(buf) - (p - buf)), "  ");
         p += 2;
      }
      if (i % 16 == 0)
      {
         if (i % 64 == 0)
         {
			sprintf_s(p, (sizeof(buf) - (p - buf)), " %c  %s\r\n\r\n", SPLIT_CHAR, cBuf);
         }
         else
         {
			sprintf_s(p, (sizeof(buf) - (p - buf)), " %c  %s\r\n", SPLIT_CHAR, cBuf);
         }

         QCEUD_Print("%s", buf);
         RtlZeroMemory(buf, 1024);
         p = buf;
         cp = cBuf;
      }
   }

   lastCnt = i % 16;

   if (lastCnt == 0)
   {
      lastCnt = 16;
   }

   if (lastCnt != 1)
   {
      // 10 + 3*8 + 2 + 3*8 = 60 (full line bytes)
      spaceNeeded = (16 - lastCnt + 1) * 3;

      if (lastCnt <= 8)
      {
         spaceNeeded += 2;
      }
      for (s = 0; s < spaceNeeded; s++)
      {
		  sprintf_s(p, (sizeof(buf) - (p - buf)), " ");
		  p++;
      }
	  sprintf_s(p, (sizeof(buf) - (p - buf)), " %c  %s\r\n\t   --- <%s> END OF DATA BYTES(%u/%uB) ---\n",
              SPLIT_CHAR, cBuf, info, len, PktLen);
      QCEUD_Print("%s", buf);
   }
   else
   {
	  sprintf_s(buf, sizeof(buf), "\r\n\t   --- <%s> END OF DATA BYTES(%u/%uB) ---\n", info, len, PktLen);
      QCEUD_Print("%s", buf);
   }
}  //USBUTL_PrintBytes


//FIXME - cause this to be compiled out if DEBUGLEVEL1 not defined
VOID _cdecl QCEUD_Print(const char* Format, ...)
{
#ifndef PERIPHERAL_PRINT_ENABLE
	return;
#endif

#define DBG_MSG_MAX_SZ 1024
	va_list arguments;
	CHAR   msgBuffer[DBG_MSG_MAX_SZ];

	va_start(arguments, Format);
	_vsnprintf_s(msgBuffer, DBG_MSG_MAX_SZ, _TRUNCATE, Format, arguments);
	va_end(arguments);
#ifndef DEBUGLEVEL1
	return;
#endif
	OutputDebugString(msgBuffer);
}

//FIXME - cause this to be compiled out if DEBUGLEVEL2 not defined
VOID _cdecl QCEUD_Print2(const char* Format, ...)
{
	#ifndef PERIPHERAL_PRINT_ENABLE
	return;
	#endif
#define DBG_MSG_MAX_SZ 1024
	va_list arguments;
	CHAR   msgBuffer[DBG_MSG_MAX_SZ];

	va_start(arguments, Format);
	_vsnprintf_s(msgBuffer, DBG_MSG_MAX_SZ, _TRUNCATE, Format, arguments);
	va_end(arguments);
#ifndef DEBUGLEVEL2
	return;
#endif
	OutputDebugString(msgBuffer);
}
#endif

bool get_dev_info(libusb_device *dev, eud_device_info * eud_device);

FILE *log_file;
static libusb_context *ctx = NULL; //a libusb session	

void libusb_usb_init()
{
	int r; //for return values
	
	//initialize a library session
	r = libusb_init(&ctx); 
	if (r < 0) {
        QCEUD_Print("Init Error \n");
	}
	
	QCEUD_Print("libusb_init %d \n" , ctx);

	//set verbosity level to 3, as suggested in the documentation
	libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, 3);
	
}

void libusb_usb_deinit()
{
	//needs to be called to end the libusb library session
    libusb_exit(ctx);
}


//===---------------------------------------------------------------------===//
//
// UsbDevice class functions.
//
//===---------------------------------------------------------------------===//
//Constructor
UsbDevice::UsbDevice()
{
    usb_device_initialized_ = FALSE;
	debug_ = FALSE;
}


UsbDevice::UsbDevice(bool debug_en)
{
	usb_device_initialized_ = FALSE;
	debug_ = debug_en;
}


//Destructor
UsbDevice::~UsbDevice()
{
	UsbClose();
}

USB_ERR_t UsbDevice::UsbOpen(int* error_code, libusb_device *dev )
{
	//fixme, not clean
	uint32_t libusb_ret; 
    libusb_device_handle *cur_dev_handle; //a device handle
	
	libusb_ret = libusb_open(dev, &(this->dev_handle_));
	cur_dev_handle = this->dev_handle_;
	

	if (libusb_ret) {
		QCEUD_Print("Cannot open device: %s DeviceType: %d \n", libusb_error_name(libusb_ret), this->device_type_);
		*error_code = libusb_ret;
		return EUD_USB_ERROR;
	}
	else
		{
        QCEUD_Print("Device Opened %x; %d \n", this->dev_, this->device_type_ );
		
		if (libusb_kernel_driver_active(cur_dev_handle, 0) == 1) { //find out if kernel driver is attached
			QCEUD_Print("Kernel Driver Active");
			if (libusb_detach_kernel_driver(cur_dev_handle, 0) == 0) //detach it
				QCEUD_Print("Kernel Driver Detached!");
		}
	
		libusb_ret = libusb_claim_interface(cur_dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
		if (libusb_ret < 0) {
			QCEUD_Print("Cannot Claim Interface: %s, DeviceType: %d", libusb_error_name(libusb_ret), this->device_type_);
			*error_code = libusb_ret;
			return EUD_USB_ERROR;
		}
        QCEUD_Print("Claimed Interface \n");

        usb_device_initialized_ = TRUE;
		} 

    return EUD_SUCCESS;
}

void UsbDevice::UsbClose()
{
	int libusb_ret = LIBUSB_SUCCESS;

	if (usb_device_initialized_ == TRUE){
        QCEUD_Print(" Closing USB device. \n");

        libusb_ret = libusb_release_interface(this->dev_handle_, 0); //release the claimed interface
        if(libusb_ret !=0) {
			QCEUD_Print("Cannot Release Interface : %s, DeviceType: %d \n", (PCHAR)libusb_error_name(libusb_ret), this->device_type_);
        }
		else {
			QCEUD_Print("Released Interface. DeviceType: %d \n", this->device_type_);
			libusb_close(this->dev_handle_); //close the device we opened
			this->dev_handle_ = NULL;
			usb_device_initialized_ = FALSE;
		}

	}
}


USB_ERR_t UsbDevice::WriteToDevice( PVOID Buffer, DWORD SendSize, DWORD *errcode)
{
	int libusb_ret = LIBUSB_SUCCESS, BytesWritten = 0;
	USB_ERR_t rvalue = 0;
#ifdef ENABLE_USB
	if (dev_handle_ == NULL){
		// Generate a new handle
		if (EUD_SUCCESS != UsbOpen(&libusb_ret, dev_)) {
			*errcode = libusb_ret;
			return EUD_USB_ERR_HANDLE_UNITIALIZED;
		}
	}
	if (SendSize == 0)
	{
		return USB_ERR_SENDSIZE_0_NODATASENT;
	}
#endif

	// Added 100 msec timeout required complete bulk transfer
    libusb_ret = libusb_bulk_transfer(dev_handle_, EUD_WRITE_ENDPOINT, (unsigned char*) Buffer, SendSize, &BytesWritten, 100);
	// Print the data we are attempting to write regardless of the result
	// QCEUD_Print("WriteToDevice %x; Buffer: 0x", this->dev_);
	// for (DWORD i = 0; i<SendSize; i++) {
	// 	QCEUD_Print("%x", ((uint8_t*)Buffer)[i]);
	// }
	// QCEUD_Print("; Bytes: %d \n", SendSize);

	if (libusb_ret == 0 && BytesWritten == static_cast<int>(SendSize))
	{
		QCEUD_Print("Write successful. \n");
	}
	else
       {
		QCEUD_Print("Write Error: %s Bytes Written: %d, Bytes Expected: %d \n", (PCHAR)libusb_error_name(libusb_ret), BytesWritten, SendSize);
		// Save the error 
		*errcode = rvalue = libusb_ret;
		// Close the handle and open a new one next time
		UsbClose();
		}

	return rvalue;
}  // WriteToDevice


/*
API to read from an EUD USB device
Note:
	The device handle can be null due to a previous error. Initialize handle if necessary
*/
USB_ERR_t UsbDevice::ReadFromDevice(PVOID Buffer, DWORD ReadSize, DWORD *errcode)
{
   int libusb_ret = 0, bytesRead = 0;
   EUD_ERR_t err = EUD_GENERIC_FAILURE;


   if (dev_handle_ == NULL) {
	  // Generate a new handle
	  if (EUD_SUCCESS != UsbOpen(&libusb_ret, dev_)) {
		  *errcode = libusb_ret;
		  err = EUD_USB_ERR_HANDLE_UNITIALIZED;
	  }
   }

   else if (ReadSize > 1)
   {
   		// Added timeout 100 msec for libbulk transfer to complete
	   libusb_ret = libusb_bulk_transfer(dev_handle_, EUD_READ_ENDPOINT, (unsigned char*)Buffer, ReadSize, &bytesRead, 100);
	   if (libusb_ret == 0 && bytesRead == static_cast<int>(ReadSize)) {
		//    QCEUD_Print("ReadFromDevice %x; Bytes: %d, Buffer: 0x", this->dev_, bytesRead);
		//    for (int i = 0; i < bytesRead; i++) {
		// 	   QCEUD_Print("%x", ((uint8_t*)Buffer)[i]);
		//    }
		//    QCEUD_Print("\n");
		   err = EUD_USB_SUCCESS;
	   }

	   else {
		   QCEUD_Print("Read Error: %s \n", (PCHAR)libusb_error_name(libusb_ret));
		   // Return the error code
		   *errcode = libusb_ret;
		   // Close the interface
		   UsbClose();
		   err = EUD_USB_ERROR_READ_FAILED_GENERIC;
	   }
   }
   else { err = EUD_USB_SUCCESS; }

   return err;
}

/*
API to retrieve a list of EUD devices attached to the host
Return Value: A vector of eud_devices matching valid EUD devices
*/
std::vector<eud_device_info> *usb_scan_devices(void)
{
	libusb_device **devs;
	ssize_t cnt, i;
    std::vector<eud_device_info> *eud_device_v = new std::vector<eud_device_info>;
    eud_device_info eud_device;

	if (!eud_device_v) { 
		return NULL; 
	}

	cnt = libusb_get_device_list(ctx, &devs);
	if (cnt < 0) {
        QCEUD_Print("USB Scandevices Error. \n");
	}

	QCEUD_Print("\n Scanning for EUD devices: %d \n", (uint32_t)cnt);
	for (i = 0; i < cnt; i++) {
        if (get_dev_info( devs[i], &eud_device ) == true )
			{
				eud_device_v->push_back(eud_device);
			}
	}

	// Free the device list after getting the required USB devce
	// TODO: Unable to launch openocd in Windows host machine with the below change
	// Need to understand the issue in detail, As of now limiting to Linux.
	#ifdef EUD_LNX_ENV
		libusb_free_device_list(devs, 1);
	#endif

	return eud_device_v;
}

/*
API to retrieve the device descriptor to compare against a valid EUD device
@Arguments
	arg0 : libusb_device *, pointer to the device
	arg1 : eud_device_info *, handle to eud_device
*/
bool get_dev_info(libusb_device *dev, eud_device_info * eud_device)
{
    #define USB_DEPTH     7    //As per the USB 3.0 specs, the current maximum limit for the depth is 7.
    #define USB_PATH_SIZE 100	
	
    libusb_device_descriptor desc;
	bool bValid = false; 

	int j = 0;
	int elements_filled = 0; 
    uint8_t path[USB_DEPTH]; 
	char stringpath[USB_PATH_SIZE] = { 0 };
	int libusb_ret;
	 
    libusb_ret = libusb_get_device_descriptor(dev, &desc);
    if (libusb_ret != LIBUSB_SUCCESS) {
        QCEUD_Print("Failed to get device descriptor : %s \n", (PCHAR)libusb_error_name(libusb_ret));
        return bValid;
    }
	
	// Compare against Qualcomm Vendor ID and save the device type 
	if( desc.idVendor == EUD_QCOM_VID){

		QCEUD_Print("\n Device Class: %d VendorId: %x ProductId: %x",(int)desc.bDeviceClass, desc.idVendor, desc.idProduct);

		eud_device->deviceHandle = dev;
	
	    //Set the device type based on PID
		switch (desc.idProduct){
		case EUD_CTL_PID:
			eud_device->deviceType = DEVICETYPE_EUD_CTL;
			break;
		case EUD_JTG_PID:
			eud_device->deviceType = DEVICETYPE_EUD_JTG;
			break;
		case EUD_SWD_PID:
			eud_device->deviceType = DEVICETYPE_EUD_SWD;
			break;
		case EUD_COM_PID:
			eud_device->deviceType = DEVICETYPE_EUD_COM;
			break;
		case EUD_TRC_PID:
			eud_device->deviceType = DEVICETYPE_EUD_TRC;
			break;
		default:
			return bValid;

		}

		QCEUD_Print( "Libusb Detect : (bus %d, device %d)", libusb_get_bus_number(dev), libusb_get_device_address(dev));
        elements_filled = libusb_get_port_numbers(dev, path, sizeof(path));
        if (elements_filled > 0) {
		    sprintf_s(stringpath, USB_DEPTH ,"%d", path[0]);
            for (j = 1; j < (elements_filled-1); j++)
			    sprintf_s(stringpath + strlen(stringpath), (USB_PATH_SIZE - USB_DEPTH), ".%d ; ",path[j]);
				   
        }
	
		eud_device->devicePath = stringpath;
		bValid = true;

        QCEUD_Print( "Libusb Valid Device: Type %d ; Path %s ", eud_device->deviceType , stringpath);
			
    }
	
	return bValid;
}

#if defined ( EUD_LNX_ENV )
//FIXME - cause this to be compiled out if DEBUGLEVEL1 not defined
VOID _cdecl QCEUD_Print(PCHAR Format, ...)
{
	#ifndef PERIPHERAL_PRINT_ENABLE
	return;
	#endif
#define DBG_MSG_MAX_SZ 1024
	va_list arguments;
	CHAR   msgBuffer[DBG_MSG_MAX_SZ];

	va_start(arguments, Format);
	vsnprintf(msgBuffer, DBG_MSG_MAX_SZ, Format, arguments);

	va_end(arguments);
	log_file=fopen("libeud_output.txt", "a");
	fprintf(log_file, "%s", msgBuffer);
	fclose(log_file);
#ifndef DEBUGLEVEL1
	return;
#endif
	//OutputDebugString(msgBuffer);
	//std::clog<<msgBuffer<<std::endl;

}



VOID _cdecl QCEUD_Print2(PCHAR Format, ...)
{
	#ifndef PERIPHERAL_PRINT_ENABLE
	return;
	#endif
#define DBG_MSG_MAX_SZ 1024
	va_list arguments;
	CHAR   msgBuffer[DBG_MSG_MAX_SZ];

	va_start(arguments, Format);
	 vsnprintf(msgBuffer, DBG_MSG_MAX_SZ, Format, arguments);
	//_vsnprintf_s(msgBuffer, DBG_MSG_MAX_SZ, _TRUNCATE, Format, arguments);
	va_end(arguments);
    printf ("%s", msgBuffer); //DW: To print the message to cout
    fflush(stdout);
#ifndef DEBUGLEVEL2
	return;
#endif
	//OutputDebugString(msgBuffer);
	//std::clog<<msgBuffer<<std::endl;

}

#endif
