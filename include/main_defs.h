#pragma once

#include <string.h>

#include "Arduino.h"

#define MAX_NOTIFY_READ_CYCLE 64000U
#define MIN_WEIGHT_INC ((float)0.2)

void tap_counter(void);
void wio_accelerometer_init(void);
void wio_get_acceleroXYZ_str(char *pAcceleroStr_o);
void wio_gpio_init(void);
