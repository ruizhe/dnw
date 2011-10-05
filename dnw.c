#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <libusb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

static struct libusb_device_handle *init_s3c2440_usb()
{
    int r;
    struct libusb_device_handle *devh = NULL;

    r = libusb_init(NULL);
    if (r < 0) {
        fprintf(stderr, "failed to initialise libusb\n");
        return NULL;
    }

    devh = libusb_open_device_with_vid_pid(NULL, 0x5345, 0x1234);
    if (devh == NULL) {
        fprintf(stderr, "Could not find/open device\n");
        return NULL;
    }

    r = libusb_claim_interface(devh, 0);
    if (r < 0) {
        fprintf(stderr, "usb_claim_interface error %d\n", r);
        libusb_close(devh);
        devh = NULL;
    }

    return devh;
}

static void close_s3c2440_usb(struct libusb_device_handle *devh)
{
    if (devh != NULL)
        libusb_close(devh);
    libusb_exit(NULL);
}

int main(int argc, char *argv[])
{
    struct libusb_device_handle *devh = NULL;
    int fd = -1;
    struct stat sb;
    int count = 0;
    char *buf;
    int len;
    int r;
    int wrote;

    if (argc < 2) {
        printf("Usage: %s <filename>\n"
               "    <filename> : file to download\n", argv[0]);
        exit(1);
    }
    devh = init_s3c2440_usb();
    if (devh == NULL) {
        exit (1);
    }
    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Can't open file %s\n", argv[1]);
        goto err;
    }
    if (0 > fstat(fd, &sb)) {
        fprintf(stderr, "Can't stat fd\n");
        goto err;
    }
    buf = (char *)malloc(sb.st_size + 8);
    if (buf == NULL) {
        fprintf(stderr, "alloc mem failed\n");
        goto err;
    }
    *((unsigned int *)buf) = 0x32000000;
    *((unsigned int *)buf + 1) = sb.st_size;
    if (sb.st_size != read(fd, buf+8, sb.st_size)) {
        fprintf(stderr, "read file error\n");
        goto err;
    }

    r = libusb_bulk_transfer(devh, 0x03, buf, sb.st_size, &wrote, 0);
    if (r < 0) {
        fprintf(stderr, "error: libusb_bulk_transfer returned %d\n", r);
        goto err;
    }
    if (wrote != sb.st_size) {
        fprintf(stderr, "error: size wrote to 2440 not equal to file size\n");
        goto err;
    }

    if (buf != NULL)
        free(buf);
    if (fd != -1)
        close(fd);
    close_s3c2440_usb(devh);
    return 0;

err:
    if (buf != NULL)
        free(buf);
    if (fd != -1)
        close(fd);
    close_s3c2440_usb(devh);
    return 1;
}
