#pragma once

#include <stdint.h>

#include "Arduino.h"
#include "SparkFunBQ27441.h"
#include "display.h"
#include "serial_print.h"

bool    battery_init(void);
int32_t get_battery_state(void);
