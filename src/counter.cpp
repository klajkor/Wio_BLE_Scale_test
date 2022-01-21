#include "counter.h"

// SM Counter variables
int state_counter1 = COUNTER_STATE_RESET; // The actual state of the state machine
int prev_state_counter1 =
    COUNTER_STATE_RESET; // Remembers the previous state (useful to know when the state has changed)
int           iSecCounter1 = -1;
int           prev_iSecCounter1 = 0;
int           iMinCounter1 = -1;
int           prev_iMinCounter1 = 0;
unsigned long start_counter1 = 0;
unsigned long elapsed_counter1 = 0;

/**
 * @brief Counter 1 state machine - counts the seconds
 */
void StateMachine_counter1(float weight_diff_i, float min_diff_treshold)
{
    prev_state_counter1 = state_counter1;

    // State Machine Section
    switch (state_counter1)
    {
    case COUNTER_STATE_RESET:
        iSecCounter1 = 0;
        iMinCounter1 = 0;
        elapsed_counter1 = 0;
        state_counter1 = COUNTER_STATE_DISABLED;
        break;
    case COUNTER_STATE_DISABLED:
        if (weight_diff_i >= min_diff_treshold)
        {
            iSecCounter1 = 0;
            iMinCounter1 = 0;
            elapsed_counter1 = 0;
            start_counter1 = millis();
            state_counter1 = COUNTER_STATE_START;
        }
        break;
    case COUNTER_STATE_START:
        state_counter1 = COUNTER_STATE_COUNTING;
        //decent_cmd_timer_start();
        wio_brew_timer_update(iSecCounter1);
        break;
    case COUNTER_STATE_COUNTING:
        prev_iSecCounter1 = iSecCounter1;
        elapsed_counter1 = millis() - start_counter1;
        iSecCounter1 = int((elapsed_counter1 / 1000) % 60);
        iMinCounter1 = int((elapsed_counter1 / 1000) / 60);
        if (iSecCounter1 != prev_iSecCounter1)
        {
            wio_brew_timer_update(iSecCounter1);
        }
        if (weight_diff_i < min_diff_treshold)
        {
            state_counter1 = COUNTER_STATE_STOP;
        }
        break;
    case COUNTER_STATE_STOP:
        state_counter1 = COUNTER_STATE_DISABLED;
        //decent_cmd_timer_stop();
        wio_brew_timer_update(iSecCounter1);
        break;
    }
}
