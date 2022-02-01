#pragma once

#include "Arduino.h"

// Function defs for main.cpp only
bool connectToServer();
void setup(void);
void loop(void);
void scale_read_timer_callback(TimerHandle_t xTimer);
void battery_status_update_callback(TimerHandle_t xTimer);
void print_endianness(void);
