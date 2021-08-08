#pragma once

#include <Arduino.h>
#include "main_defs.h"
#include "display.h"

typedef enum
{
    COUNTER_STATE_RESET = 0,
    COUNTER_STATE_DISABLED,
    COUNTER_STATE_START,
    COUNTER_STATE_COUNTING,
    COUNTER_STATE_STOP
} counter_states_e;

//SM Counter variables
extern int state_counter1;      //The actual state of the state machine
extern int prev_state_counter1; //Remembers the previous state (useful to know when the state has changed)
extern int iSecCounter1;
extern int prev_iSecCounter1;
extern int iMinCounter1;
extern int prev_iMinCounter1;
extern unsigned long start_counter1;
extern unsigned long elapsed_counter1;

void StateMachine_counter1(float weight_diff_i, float min_diff_treshold);
