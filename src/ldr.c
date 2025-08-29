#include <stdio.h>
#include "ldr.h"

int ldr_read(void) {
    int val = 0;
    FILE *fp = fopen("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", "r");
    if (fp) {
        fscanf(fp, "%d", &val);
        fclose(fp);
    }
    return val;
}
