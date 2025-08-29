#ifndef UART_H
#define UART_H

#include <pthread.h>

int uart_init(void);
void uart_write(const char *msg);
pthread_t uart_start_listener(void (*callback)(int fan_pct, int led_pct));

#endif // UART_H
