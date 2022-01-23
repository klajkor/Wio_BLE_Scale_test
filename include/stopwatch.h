#pragma once

#include "Arduino.h"
#include "decent_scale.h"
#include "display.h"
#include "main_defs.h"

typedef enum
{
    STOPWATCH_RESET = 0,
    STOPWATCH_RUNNING,
    STOPWATCH_STOPPED,
    STOPWATCH_NO_CHANGE
} stopwatch_states_t;

typedef struct
{
    uint32_t start_time;
    uint32_t secs;
    uint32_t mins;
    bool     running;
} StopwatchTimer_st;

extern StopwatchTimer_st stopwatch_timer;

stopwatch_states_t fsm_stopwatch(stopwatch_states_t new_state);