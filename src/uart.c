#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include "uart.h"

#define UART_DEV "/dev/ttyS1"

static int uart_fd = -1;

int uart_init(void) {
    uart_fd = open(UART_DEV, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (uart_fd < 0) { perror("open UART"); return -1; }
    struct termios options;
    tcgetattr(uart_fd, &options);
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8 | CREAD | CLOCAL;
    tcsetattr(uart_fd, TCSANOW, &options);
    return 0;
}

void uart_write(const char *msg) {
    if (uart_fd >= 0) {
        dprintf(uart_fd, "%s", msg);
    }
}

static void *uart_listener(void *arg) {
    void (*callback)(int, int) = arg;
    char buf[128];
    while (1) {
        int n = read(uart_fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            int f, l;
            if (sscanf(buf, "%d,%d", &f, &l) == 2) {
                callback(f, l);
                char response[64];
                snprintf(response, sizeof(response), "Manual FAN=%d%% LED=%d%%\n", f, l);
                uart_write(response);
            } else {
                uart_write("Ignored or invalid input\n");
            }
        }
        usleep(100000);
    }
    return NULL;
}

pthread_t uart_start_listener(void (*callback)(int fan_pct, int led_pct)) {
    pthread_t th;
    pthread_create(&th, NULL, uart_listener, (void *)callback);
    return th;
}

