#include "doomgeneric.h"

#include <unistd.h>   /* for usleep(3p) */
#include <sys/time.h> /* for gettimeofday(3p) */

void DG_SleepMs(uint32_t ms) {
    usleep(ms * 1000);
}

uint32_t DG_GetTicksMs() {
    struct timeval  tp;
    struct timezone tzp;

    gettimeofday(&tp, &tzp);

    return (tp.tv_sec * 1000) + (tp.tv_usec / 1000); /* return milliseconds */
}

