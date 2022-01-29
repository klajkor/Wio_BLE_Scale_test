#pragma once

#include "Arduino.h"

typedef struct
{
    uint32_t start_time;
    uint32_t secs;
    uint32_t mins;
    bool     running;
} AppTimer_st;
