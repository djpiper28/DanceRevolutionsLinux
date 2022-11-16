#include <stdlib.h>
#include <stdio.h>
#include "./dance.h"
#include "./testing.h/logger.h"

int main(int argc, char **argv)
{
    lprintf(LOG_INFO, "Starting controller testing software...\n");

    dance_controller_t cont;
    int s = init_dance_controller(&cont);
    if (!s) {
        lprintf(LOG_ERROR, "Cannot init controller\n");
        return 1;
    }

    for (int last_state = 0;;) {
        int cont_state;
        if (!dance_controller_state(&cont, &cont_state)) {
            lprintf(LOG_ERROR, "Controller disconnect\n");
            break;
        }

        if (cont_state != last_state) {
            lprintf(LOG_INFO, "State: %08x\n", cont_state);
        }

        last_state = cont_state;
    }

    free_dance_controller(&cont);
    return 0;
}
