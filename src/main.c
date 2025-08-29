#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <curl/curl.h>
#include "dht22.h"
#include "ssd1306.h"
#include "fan.h"
#include "led.h"
#include "ldr.h"
#include "uart.h"
#include "button.h"
#include "thingspeak.h"

static volatile int manual_mode = 0;
static volatile int fan_manual = 0;
static volatile int led_manual = 0;
static volatile int show_mode_screen = 0;

static void uart_callback(int fan_pct, int led_pct) {
    if (manual_mode) {
        fan_manual = fan_pct;
        led_manual = led_pct;
        fan_set_percent(fan_manual);
        led_set_percent(led_manual);
    }
}

static void button_callback(int toggle) {
    (void)toggle;
    manual_mode = !manual_mode;
    show_mode_screen = 1;
    const char *mode_str = manual_mode ? "MANUAL" : "AUTO";
    printf("Mode switched: %s\n", mode_str);
    char uart_msg[32];
    snprintf(uart_msg, sizeof(uart_msg), "Mode switched: %s\n", mode_str);
    uart_write(uart_msg);
}

static void handle_sigint(int sig) {
    (void)sig;
    fan_cleanup();
    led_cleanup();
    curl_global_cleanup();
    printf("\nClean exit, PWM disabled.\n");
    exit(0);
}

int main(void) {
    signal(SIGINT, handle_sigint);

    if (ssd1306_init() < 0) {
        fprintf(stderr, "SSD1306 init failed\n");
    } else {
        ssd1306_draw_string_5x7("System Init...", 3, 20);
    }

    if (fan_init() < 0 || led_init() < 0 || uart_init() < 0) {
        fprintf(stderr, "Initialization failed\n");
        return 1;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    uart_start_listener(uart_callback);
    button_start_listener(button_callback);

    while (1) {
        float hum = 0.0f, temp = 0.0f;
        int ldr_val = ldr_read();
        int fan_pct, led_pct;

        if (dht22_read(&hum, &temp) != 0) {
            temp = 0.0f;
            hum = 0.0f;
        }

        if (!manual_mode) {
            fan_set_auto(temp);
            led_set_auto(ldr_val);
            fan_pct = (temp > 24.0f) ? (int)(((temp - 24.0f) / 6.0f) * 100.0f) : 0;
            if (fan_pct > 100) fan_pct = 100;
            led_pct = (int)((4095 - ldr_val) * 100 / 4095);
            if (led_pct < 0) led_pct = 0;
            if (led_pct > 100) led_pct = 100;
        } else {
            fan_pct = fan_manual;
            led_pct = led_manual;
            fan_set_percent(fan_pct);
            led_set_percent(led_pct);
        }

        if (show_mode_screen) {
            ssd1306_clear();
            ssd1306_draw_big_centered(manual_mode ? "MANUAL" : "AUTO");
            sleep(2);
            show_mode_screen = 0;
        }

        ssd1306_clear();
        char line1[24], line2[24];
        snprintf(line1, sizeof(line1), "Temp: %.1f C", temp);
        snprintf(line2, sizeof(line2), "Hum : %.1f %%", hum);
        ssd1306_draw_string_5x7(line1, 2, 8);
        ssd1306_draw_string_5x7(line2, 4, 8);

        printf("%s  T=%.1fC  H=%.1f%%  Fan=%d%%  LED=%d%%  LDR=%d\n",
               manual_mode ? "MANUAL" : "AUTO", temp, hum, fan_pct, led_pct, ldr_val);
        char uart_msg[128];
        snprintf(uart_msg, sizeof(uart_msg), "%s T=%.1f H=%.1f Fan=%d%% LED=%d%% LDR=%d\n",
                 manual_mode ? "MANUAL" : "AUTO", temp, hum, fan_pct, led_pct, ldr_val);
        uart_write(uart_msg);

        thingspeak_post(temp, hum, fan_pct, led_pct, ldr_val, manual_mode);

        sleep(15);
    }

    return 0;
}

