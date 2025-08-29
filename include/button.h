#ifndef BUTTON_H
#define BUTTON_H

#include <pthread.h>

pthread_t button_start_listener(void (*callback)(int mode));

#endif // BUTTON_H
