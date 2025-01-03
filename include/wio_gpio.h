#pragma once

#include <stdint.h>

#include "Arduino.h"

extern uint32_t KEY_STOPWATCH;
extern uint32_t KEY_SCALE_TARE;

extern bool key_stopwatch_pressed;
extern bool key_scale_tare_pressed;

void wio_gpio_init(void);
void key_stopwatch_handler(void);
void key_scale_handler(void);
