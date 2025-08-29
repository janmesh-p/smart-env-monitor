#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "led.h"

#define LED_PWM_CHIP "/sys/class/pwm/pwmchip5"
#define LED_PWM1     LED_PWM_CHIP "/pwm1"
#define LED_EXPORT   LED_PWM_CHIP "/export"
#define LED_PERIOD   LED_PWM1 "/period"
#define LED_DUTY     LED_PWM1 "/duty_cycle"
#define LED_ENABLE   LED_PWM1 "/enable"

int led_init(void) {
    FILE *fp = fopen(LED_EXPORT, "w");
    if (!fp) { perror("led export"); return -1; }
    fprintf(fp, "1");
    fclose(fp);
    usleep(200000);
    fp = fopen(LED_PERIOD, "w");
    if (!fp) { perror("led period"); return -1; }
    fprintf(fp, "10000000");
    fclose(fp);
    fp = fopen(LED_ENABLE, "w");
    if (!fp) { perror("led enable"); return -1; }
    fprintf(fp, "1");
    fclose(fp);
    return 0;
}

void led_set_percent(int percent) {
    if (percent < 15) percent = 0;
    if (percent > 85) percent = 100;
    int duty = (10000000 * percent) / 100;
    FILE *fp = fopen(LED_DUTY, "w");
    if (fp) {
        fprintf(fp, "%d", duty);
        fclose(fp);
    }
}

void led_set_auto(int ldr_value) {
    int percent = (int)((4095 - ldr_value) * 100 / 4095);
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    led_set_percent(percent);
}

void led_cleanup(void) {
    FILE *fp;
    fp = fopen(LED_DUTY, "w");
    if (fp) { fprintf(fp, "0"); fclose(fp); }
    fp = fopen(LED_ENABLE, "w");
    if (fp) { fprintf(fp, "0"); fclose(fp); }
}

