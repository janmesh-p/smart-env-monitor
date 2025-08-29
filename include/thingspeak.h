#ifndef THINGSPEAK_H
#define THINGSPEAK_H

void thingspeak_post(float temp, float hum, int fan_pct, int led_pct, int ldr_val, int mode);

#endif // THINGSPEAK_H
