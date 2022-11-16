#include "./dance.h"
#include "./testing.h/logger.h"
#include <unistd.h>

// Sample lsusb entry for the mat
// Bus 005 Device 006: ID 054c:0268 Sony Corp. Batoh Device / PlayStation 3 Controller

#define VENDOR_ID 0x054c
#define PRODUCT_ID 0x0268
#define POLL_TIME_MS 5

void *dance_controller_poll_thread(void *ctx)
{
    dance_controller_t *cont = (dance_controller_t *) ctx;

    int flag = 1;
    while (flag) {
        pthread_mutex_lock(&cont->lock);
        pthread_mutex_unlock(&cont->lock);

        usleep(POLL_TIME_MS);
    }

    pthread_exit(NULL);
}

int init_dance_controller(dance_controller_t *cont)
{
    lprintf(LOG_INFO, "Starting dance mat controller version %s\n", VERSION);

    // Claim the dance pad
    int r = libusb_init(&cont->ctx);
    if (r != 0) {
        lprintf(LOG_ERROR, "Cannot init lib usb\n");
        return 0;
    }

    cont->handle = libusb_open_device_with_vid_pid(cont->ctx, VENDOR_ID, PRODUCT_ID);
    libusb_detach_kernel_driver(cont->handle, 0);
    libusb_claim_interface(cont->handle, 0);

    // Start the thread
    pthread_mutex_t minit = PTHREAD_MUTEX_INITIALIZER;
    cont->lock = minit;
    r = pthread_create(&cont->thread, NULL, &dance_controller_poll_thread, (void *) cont);
    if (r != 0) {
        lprintf(LOG_ERROR, "Cannot start polling thread\n");
        return 0;
    }

    return 1;
}

int dance_controller_state(dance_controller_t *cont, int *cont_state)
{
    int ret = 0;

    pthread_mutex_lock(&cont->lock);
    ret = cont->state;
    pthread_mutex_unlock(&cont->lock);

    return ret;
}

void free_dance_controller(dance_controller_t *cont)
{
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

    // Release the USB device
    libusb_close(cont->handle);
    libusb_exit(cont->ctx);
}
