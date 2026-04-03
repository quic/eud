#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "eud_error_defines.h"
#include "trc_api.h"
#include "device_manager.h"

static void write_trace(const char *file_name, const uint8_t *trace_buff, size_t bytes_read, size_t bytes_requested)
{
    FILE *fp = fopen(file_name, "wb");
    if (!fp) {
        printf("File Error: Could not open '%s' for writing\n", file_name);
        return;
    }

    size_t written = fwrite(trace_buff, 1, bytes_read, fp);
    if (written != bytes_read) {
        printf("File Warning: Only %zu of %zu bytes written\n", written, bytes_read);
    }
    fclose(fp);

    printf("Trace Info: Requested %zu bytes, wrote %zu bytes to '%s'\n",
           bytes_requested, bytes_read, file_name);
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: %s <sleep_delay_ms> <bytes_to_read>\n", argv[0]);
        printf("Example: %s 5000 2048\n", argv[0]);
        return -1;
    }

    uint32_t trns_len = 126;
    uint32_t trns_tmout = 0x00EE12EA;
    uint32_t device_id = 0U;
    uint32_t arr[100] = {0};
    uint32_t len = 0U;
    EUD_ERR_t err = EUD_SUCCESS;
    size_t bytes_read = 0;
    char file_name[100];
    size_t sleep_delay = atoi(argv[1]);

    struct timespec ts;
    ts.tv_sec = sleep_delay / 1000;
    ts.tv_nsec = (sleep_delay % 1000) * 1000000;

    size_t bytes_to_read = atoi(argv[2]);
    if (bytes_to_read == 0 || bytes_to_read > 10485760) {  // Max 10MB
        printf("Invalid bytes_to_read value. Must be between 1 and 10485760\n");
        return -1;
    }

    uint8_t *trace_buff = (uint8_t *)malloc(bytes_to_read);
    if (!trace_buff) {
        printf("Failed to allocate memory for trace buffer\n");
        return -1;
    }

    snprintf(file_name, sizeof(file_name), "Trace_%s.bin", argv[2]);

    // Get device ID
    err = get_device_id_array(arr, &len);
    device_id = arr[0];
    if (device_id == 0) {
        printf("Get Device ID failed\n");
        free(trace_buff);
        return err;
    }
    printf("DEVICE ID = 0x%08x\n", device_id);

    err = eud_trace_device_init(device_id, trns_len, trns_tmout);
    if (EUD_SUCCESS != err) {
        printf("Initialise EUD Trace failed with err : %x\n", err);
        eud_trace_device_close();   // ensure cleanup
        free(trace_buff);
        return err;
    }
    printf("Initialised EUD trace.\n");

    err = eud_trace_device_ready();
    if (EUD_SUCCESS != err) {
        printf("Device ready failed with err : %x\n", err);
        eud_trace_device_close();   // ensure cleanup
        free(trace_buff);
        return err;
    }
    printf("Device ready to collect traces.\n");

    nanosleep(&ts, NULL);

    err = eud_trace_device_read(trace_buff, bytes_to_read, &bytes_read);
    if (err == EUD_SUCCESS) {
        write_trace(file_name, trace_buff, bytes_read, bytes_to_read);
        printf("Trace API Status: %d. Requested %zu bytes, received %zu\n", err, bytes_to_read, bytes_read);
    } 
    else {
        printf("Trace API Error: %d.\n", err);
    }

    err = eud_trace_device_close();
    if (EUD_SUCCESS != err) {
        printf("Could not close device, failed with err : %x\n", err);
        free(trace_buff);
        return err;
    }
    printf("Closed trace device.\n");

    free(trace_buff);
    return err;
}
