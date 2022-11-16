#include "./dance.h"
#include "./testing.h/logger.h"
#include <unistd.h>
#include <string.h>

// Sample lsusb entry for the mat
// Bus 005 Device 006: ID 054c:0268 Sony Corp. Batoh Device / PlayStation 3 Controller
#define READ_ENDPOINT 0x81
#define WRITE_ENDPOINT 0x02
#define VENDOR_ID 0x054c
#define PRODUCT_ID 0x0268
#define INTERFACE 0
#define TIMEOUT 50
#define PACKET_SIZE (8 * 8)

static void *dance_controller_poll_thread(void *ctx)
{
    dance_controller_t *cont = (dance_controller_t *) ctx;
    unsigned char data[PACKET_SIZE];
    unsigned char data2[PACKET_SIZE];
    struct timeval tv = {0, TIMEOUT};

    int flag = 1;
    while (flag) {
        int completed;
        int r = libusb_handle_events_timeout_completed(cont->ctx, &tv, &completed);
        if (r != 0) {
            lprintf(LOG_ERROR, "Error handling usb events (%s)\n", libusb_strerror(r));
            flag = 0;
        }

        int transfered = 0;
        r = libusb_interrupt_transfer(cont->handle, READ_ENDPOINT, data, sizeof(data), &transfered, TIMEOUT);
        if (r == 0) {
            if (memcmp(data, data2, sizeof(data)) != 0) {
                lprintf(LOG_INFO, "Found new data, that is different: ");
                for (int i = 0; i < transfered; i++) {
                    fprintf(stderr, "%02x", (int) data[i]);
                    if (i % 8 == 0) {
                        fprintf(stderr, ",");
                    }
                }
                fprintf(stderr, "\n");
                memcpy(data2, data, sizeof(data));
            }
        } else if (r != LIBUSB_ERROR_TIMEOUT) {
            lprintf(LOG_ERROR, "Cannot read transfer data from device (%s)\n", libusb_strerror(r));
            flag = 0;
        }
    }

    pthread_mutex_lock(&cont->lock);
    cont->running = 0;
    pthread_mutex_unlock(&cont->lock);

    pthread_exit(NULL);
    return NULL;
}

int init_dance_controller(dance_controller_t *cont)
{
    lprintf(LOG_INFO, "Starting dance mat controller version %s\n", VERSION);

    // Claim the dance pad
    int r = libusb_init(&cont->ctx);
    if (r != 0) {
        lprintf(LOG_ERROR, "Cannot init lib usb (%d)\n", r);
        return 0;
    }

    cont->handle = libusb_open_device_with_vid_pid(cont->ctx, VENDOR_ID, PRODUCT_ID);
    if (cont->handle == NULL) {
        lprintf(LOG_ERROR, "Cannot connect to controller\n");
        return 0;
    }
    lprintf(LOG_INFO, "Device found, trying to connect\n");

    r = libusb_set_auto_detach_kernel_driver(cont->handle, INTERFACE);
    if (r != 0) {
        lprintf(LOG_WARNING, "Cannot detach kernel driver (%s)\n", libusb_strerror(r));
    }

    r = libusb_claim_interface(cont->handle, INTERFACE);
    if (r != 0) {
        lprintf(LOG_ERROR, "Cannot claim interface (%s)\n", libusb_strerror(r));
        lprintf(LOG_INFO, "You can try 'systemctl stop xboxdrv' to free the device\n");
        return 0;
    }

    // Start the thread
    pthread_mutex_t minit = PTHREAD_MUTEX_INITIALIZER;
    cont->lock = minit;
    r = pthread_create(&cont->thread, NULL, &dance_controller_poll_thread, (void *) cont);
    if (r != 0) {
        lprintf(LOG_ERROR, "Cannot start polling thread\n");
        return 0;
    }

    cont->running = 1;
    lprintf(LOG_INFO, "Device connected, thread is polling\n");

    return 1;
}

int dance_controller_state(dance_controller_t *cont, int *cont_state)
{
    int ret = 1;

    pthread_mutex_lock(&cont->lock);
    ret = cont->state;
    if (cont->running == 0) {
        ret = 0;
    }
    pthread_mutex_unlock(&cont->lock);

    return ret;
}

void free_dance_controller(dance_controller_t *cont)
{
    // Release the USB device
    libusb_release_interface(cont->handle, INTERFACE);
    libusb_close(cont->handle);

    // Join the thread and, destroy it
    void *ret;
    int r = pthread_join(cont->thread, &ret);
    if (r != 0) {
        lprintf(LOG_ERROR, "Cannot join polling thread\n");
    }

    r = pthread_mutex_destroy(&cont->lock);
    if (r != 0) {
        lprintf(LOG_ERROR, "Cannot destroy mutex\n");
    }

    libusb_exit(cont->ctx);
}
