#pragma once

#include "Arduino.h"
#include "app_timer.h"
#include "decent_scale.h"
#include "display.h"
#include "main_defs.h"

typedef enum
{
    STOPWATCH_RESET = 0,
    STOPWATCH_STARTING,
    STOPWATCH_RUNNING,
    STOPWATCH_STOPPING,
    STOPWATCH_STOPPED,
    STOPWATCH_NO_CHANGE
} stopwatch_states_t;

extern AppTimer_st stopwatch_timer;

stopwatch_states_t fsm_stopwatch(stopwatch_states_t new_state);