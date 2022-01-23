#include "stopwatch.h"

StopwatchTimer_st stopwatch_timer = {.start_time = 0, .secs = 0, .mins = 0, .running = false};

static stopwatch_states_t stopwatch_state = STOPWATCH_NO_CHANGE;
static stopwatch_states_t stopwatch_prev_state = STOPWATCH_NO_CHANGE;

stopwatch_states_t fsm_stopwatch(stopwatch_states_t new_state)
{
    uint32_t elapsed_time;
    uint32_t prev_secs;
    switch (new_state)
    {
    case STOPWATCH_RESET:
        if (stopwatch_state != new_state)
        {
            stopwatch_state = new_state;
            stopwatch_prev_state = stopwatch_state;
            stopwatch_timer.start_time = 0;
            stopwatch_timer.secs = 0;
            stopwatch_timer.mins = 0;
            stopwatch_timer.running = false;
            wio_brew_timer_update(stopwatch_timer.secs, stopwatch_timer.mins);
        }
        //decent_cmd_timer_reset();
        break;
    case STOPWATCH_RUNNING:
        if (stopwatch_state != new_state)
        {
            stopwatch_state = new_state;
            stopwatch_prev_state = stopwatch_state;
            stopwatch_timer.running = true;
            stopwatch_timer.start_time = millis();
            //decent_cmd_timer_start();
        }
        prev_secs = stopwatch_timer.secs;
        elapsed_time = millis() - stopwatch_timer.start_time;
        stopwatch_timer.secs = uint32_t((elapsed_time / 1000) % 60);
        stopwatch_timer.mins = uint32_t((elapsed_time / 1000) / 60);
        if (prev_secs != stopwatch_timer.secs)
        {
            wio_brew_timer_update(stopwatch_timer.secs, stopwatch_timer.mins);
        }
        break;
    case STOPWATCH_STOPPED:
        if (stopwatch_state != new_state)
        {
            stopwatch_state = new_state;
            stopwatch_prev_state = stopwatch_state;
            stopwatch_timer.running = false;
            wio_brew_timer_update(stopwatch_timer.secs, stopwatch_timer.mins);
        }
        //decent_cmd_timer_stop();
        break;
    case STOPWATCH_NO_CHANGE:
        break;
    }
    return stopwatch_state;
}
