#ifndef LED_H
#define LED_H

int led_init(void);
void led_set_percent(int percent);
void led_set_auto(int ldr_value);
void led_cleanup(void);

#endif // LED_H
