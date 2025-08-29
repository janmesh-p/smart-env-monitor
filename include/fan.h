#ifndef FAN_H
#define FAN_H

int fan_init(void);
void fan_set_percent(int percent);
void fan_set_auto(float temperature);
void fan_cleanup(void);

#endif // FAN_H
