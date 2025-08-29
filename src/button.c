#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gpiod.h>
#include <time.h>
#include "button.h"

#define GPIO_CHIP "gpiochip1"
#define GPIO_LINE 12

static void *button_listener(void *arg) {
    void (*callback)(int) = arg;
    struct gpiod_chip *chip = gpiod_chip_open_by_name(GPIO_CHIP);
    if (!chip) { perror("gpiod_chip_open_by_name"); return NULL; }
    struct gpiod_line *line = gpiod_chip_get_line(chip, GPIO_LINE);
    if (!line) { perror("gpiod_chip_get_line"); gpiod_chip_close(chip); return NULL; }
    if (gpiod_line_request_input(line, "mode_button") < 0) {
        perror("gpiod_line_request_input");
        gpiod_chip_close(chip);
        return NULL;
    }

    int last = 1;
    struct timespec last_press = {0, 0};

    while (1) {
        int val = gpiod_line_get_value(line);
        if (val == 0 && last == 1) { // falling edge
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            long diff_ms = (now.tv_sec - last_press.tv_sec) * 1000 +
                           (now.tv_nsec - last_press.tv_nsec) / 1000000;
            if (diff_ms > 200) { // debounce
                callback(1); // Toggle mode
                last_press = now;
            }
        }
        last = val;
        usleep(10000);
    }
    gpiod_chip_close(chip);
    return NULL;
}

pthread_t button_start_listener(void (*callback)(int mode)) {
    pthread_t th;
    pthread_create(&th, NULL, button_listener, (void *)callback);
    return th;
}
