
#include <stdio.h>
#include "dht22.h"
#include "bbb_dht_read.h"
#include "common_dht_read.h"

#define DHT22_GPIO_CHIP 1
#define DHT22_GPIO_LINE 13

int dht22_read(float *humidity, float *temperature) {
    return bbb_dht_read(22, DHT22_GPIO_CHIP, DHT22_GPIO_LINE, humidity, temperature);
}

