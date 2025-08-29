#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fan.h"

#define FAN_PWM_CHIP "/sys/class/pwm/pwmchip5"
#define FAN_PWM0     FAN_PWM_CHIP "/pwm0"
#define FAN_EXPORT   FAN_PWM_CHIP "/export"
#define FAN_PERIOD   FAN_PWM0 "/period"
#define FAN_DUTY     FAN_PWM0 "/duty_cycle"
#define FAN_ENABLE   FAN_PWM0 "/enable"

int fan_init(void) {
    FILE *fp = fopen(FAN_EXPORT, "w");
    if (!fp) { perror("fan export"); return -1; }
    fprintf(fp, "0");
    fclose(fp);
    usleep(200000);
    fp = fopen(FAN_PERIOD, "w");
    if (!fp) { perror("fan period"); return -1; }
    fprintf(fp, "10000000");
    fclose(fp);
    fp = fopen(FAN_ENABLE, "w");
    if (!fp) { perror("fan enable"); return -1; }
    fprintf(fp, "1");
    fclose(fp);
    return 0;
}

void fan_set_percent(int percent) {
    if (percent < 11) percent = 0;
    if (percent > 100) percent = 100;
    int duty = (10000000 * percent) / 100;
    FILE *fp = fopen(FAN_DUTY, "w");
    if (fp) {
        fprintf(fp, "%d", duty);
        fclose(fp);
    }
}

void fan_set_auto(float temperature) {
    int percent = (temperature > 24.0f) ? (int)(((temperature - 24.0f) / 6.0f) * 100.0f) : 0;
    if (percent > 100) percent = 100;
    fan_set_percent(percent);
}

void fan_cleanup(void) {
    FILE *fp;
    fp = fopen(FAN_DUTY, "w");
    if (fp) { fprintf(fp, "0"); fclose(fp); }
    fp = fopen(FAN_ENABLE, "w");
    if (fp) { fprintf(fp, "0"); fclose(fp); }
}

