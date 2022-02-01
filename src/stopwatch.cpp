#include "stopwatch.h"

AppTimer_st stopwatch_timer = {.start_time = 0, .secs = 0, .mins = 0, .running = false};

static stopwatch_states_t stopwatch_state = STOPWATCH_NO_CHANGE;
static stopwatch_states_t stopwatch_prev_state = STOPWATCH_NO_CHANGE;

stopwatch_states_t fsm_stopwatch(stopwatch_events_t new_event)
{
    uint32_t elapsed_time;
    uint32_t prev_secs;
    stopwatch_prev_state = stopwatch_state;
    switch (new_event)
    {
    case STOPWATCH_EVT_RESET:
        stopwatch_state = STOPWATCH_RESET;
        stopwatch_timer.start_time = 0;
        stopwatch_timer.secs = 0;
        stopwatch_timer.mins = 0;
        stopwatch_timer.running = false;
        wio_brew_timer_update(stopwatch_timer.secs, stopwatch_timer.mins);
        decent_cmd_timer_reset();
        break;
    case STOPWATCH_EVT_BUTTON_PUSHED:
        switch (stopwatch_state)
        {
        case STOPWATCH_RESET:
            stopwatch_state = STOPWATCH_RUNNING;
            stopwatch_timer.running = true;
            stopwatch_timer.start_time = millis();
            decent_cmd_timer_start();
            break;
        case STOPWATCH_RUNNING:
            stopwatch_state = STOPWATCH_STOPPED;
            stopwatch_timer.running = false;
            wio_brew_timer_update(stopwatch_timer.secs, stopwatch_timer.mins);
            decent_cmd_timer_stop();
            break;
        case STOPWATCH_STOPPED:
            stopwatch_state = STOPWATCH_RESET;
            stopwatch_timer.start_time = 0;
            stopwatch_timer.secs = 0;
            stopwatch_timer.mins = 0;
            stopwatch_timer.running = false;
            wio_brew_timer_update(stopwatch_timer.secs, stopwatch_timer.mins);
            if (stopwatch_prev_state == STOPWATCH_RUNNING)
            {
                decent_cmd_timer_stop();
            }
            decent_cmd_timer_reset();
            break;
        }
        break;
    case STOPWATCH_EVT_NO_CHANGE:
        break;
    }
    switch (stopwatch_state)
    {
    case STOPWATCH_RESET:
        break;
    case STOPWATCH_RUNNING:
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
        break;
    case STOPWATCH_NO_CHANGE:
        break;
    }
    return stopwatch_state;
}
