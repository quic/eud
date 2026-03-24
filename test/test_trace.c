#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "eud_error_defines.h"
#include "trc_api.h"
#include "device_manager.h"

int main (void)
{
    uint32_t trns_len = 126;
    uint32_t trns_tmout = 0x00EE12EA;
    uint32_t device_id=0U;
    uint32_t arr[100] = {0};
    uint32_t len = 0U; 
    EUD_ERR_t err = EUD_SUCCESS;
    uint8_t trace_buff[2048]; 
    size_t bytes_read = 0;
    size_t bytes_to_read = 2048;
    char file_name[100] = "Trace.bin";

    //Getting device ID 
    err = get_device_id_array(arr, &len);
    device_id = arr[0];
    if(device_id == 0) {
        printf("\nGet Device ID failed\n");
        return err;
    }
    printf("\nDEVICE ID = 0x%08x\n", device_id);

    err = eud_trace_device_init(device_id, trns_len, trns_tmout);
    if(EUD_SUCCESS != err) {
        printf("\nInitialise EUD Trace failed with err : %x\n", err );
    }
    printf("\nDone. Initialising EUD trace.\n");

    err = eud_trace_device_ready();
    if(EUD_SUCCESS != err) {
        printf("\nInit : %x\n", err );
    }
    printf("\nDone. Device is ready to collect traces.\n");
    Sleep(5000);
    err = eud_trace_device_read(trace_buff, bytes_to_read, &bytes_read);
    if ((err == EUD_SUCCESS || err == LIBUSB_ERROR_TIMEOUT )&& trace_buff != NULL && bytes_read > 0) {
        FILE* fp = fopen(file_name, "wb");
        if (fp != NULL) {
            size_t written = fwrite(trace_buff, 1, bytes_read, fp);
            if (written != bytes_read) {
                printf("Warning: Only %zu of %zu bytes written to file\n", written, bytes_read);
            }
            fclose(fp);
        } else {
            printf("Error:  %x Could not open file for writing\n");
    }
    } else {
        printf("\nError:%d Failed with to start tracing or no data returned\n", err );
    }


    err = eud_trace_device_close();
    if(EUD_SUCCESS != err) {
        printf("\nCould not Close device failed with err : %x\n", err );
        return err;
    }
    printf("\nDone. Closing the trace device.\n");

    return err; 
}

