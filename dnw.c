#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <libusb.h>

#define CTRL_IN (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN)
#define CTRL_OUT (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT)
#define USB_RQ 0x04

static int print_f0_data(struct libusb_device_handle *devh)
{
    unsigned char data[0x10];
    int r;
    unsigned int i;
    int wrote = 0;

    memset(data, 0, 0x10);
    *((unsigned int *)data) = 0x32000000;
    *((unsigned int *)data + 1) = 0x10;


    r = libusb_bulk_transfer(devh, 0x03, data, 0x10, &wrote, 0);
    if (r < 0) {
        fprintf(stderr, "F0 error %d\n", r);
        return r;
    }
    printf("%d byte(s) wrote\n", wrote);
    return r;
}

int main(void)
{
    struct sigaction sigact;
    struct libusb_device_handle *devh = NULL;
    int r = 1;

    r = libusb_init(NULL);
    if (r < 0) {
        fprintf(stderr, "failed to initialise libusb\n");
        exit(1);
    }

    devh = libusb_open_device_with_vid_pid(NULL, 0x5345, 0x1234);
    if (devh == NULL) {
        fprintf(stderr, "Could not find/open device\n");
        goto out;
    }

    r = libusb_claim_interface(devh, 0);
    if (r < 0) {
        fprintf(stderr, "usb_claim_interface error %d\n", r);
        goto out;
    }

    r = print_f0_data(devh);

out:
    libusb_close(devh);
    libusb_exit(NULL);
    return r >= 0 ? r : -r;
}
