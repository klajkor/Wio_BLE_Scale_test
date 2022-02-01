#pragma once

#include "Arduino.h"
#include "app_timer.h"
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

typedef enum
{
    STOPWATCH_EVT_RESET = 0,
    STOPWATCH_EVT_BUTTON_PUSHED,
    STOPWATCH_EVT_NO_CHANGE
} stopwatch_events_t;

extern AppTimer_st stopwatch_timer;

stopwatch_states_t fsm_stopwatch(stopwatch_events_t new_event);