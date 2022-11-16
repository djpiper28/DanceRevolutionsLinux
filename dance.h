#pragma once
#include <libusb-1.0/libusb.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Flags for the state of each of the dance pads pressure pads
typedef enum dance_state_flag_t {
    START_DOWN = 1 << 0,
    SELECT_DOWN = 1 << 1,
    X_DOWN = 1 << 2,
    O_DOWN = 1 << 3,
    TOP_DOWN = 1 << 4,
    RIGHT_DOWN = 1 << 5,
    LEFT_DOWN = 1 << 6,
    BOTTOM_DOWN = 1 << 7
} dance_state_flag_t;

typedef struct dance_controller_t {
    /// USB context
    libusb_context *ctx;
    /// Controller's device handle
    libusb_device_handle *handle;
    /// Used to see if the USB has is active
    int running;
    /// Made up of dance_state_flag_t
    int state;
    /// Lock for safety
    pthread_mutex_t lock;
    /// The controller is polled on its very own thread
    pthread_t thread;
} dance_controller_t;

int init_dance_controller(dance_controller_t *cont);
int dance_controller_state(dance_controller_t *cont, int *cont_state);
void free_dance_controller(dance_controller_t *cont);

#ifdef __cplusplus
}
#endif
