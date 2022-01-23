#pragma once

#include "Arduino.h"
#include "counter.h"
#include "main_defs.h"
#include "serial_print.h"
#include "wio_battery.h"

#if defined(__SAMD51__)
// Display driver for Wio Terminal:
#include "TFT_eSPI.h" //TFT LCD library
#include <SPI.h>
#endif

#if defined(__SAMD51__)
extern TFT_eSPI tft; // initialize TFT LCD

// Display fonts structure
typedef struct
{
    const GFXfont *title_font;
    const GFXfont *status_font;
    const GFXfont *weigth_font;
    const GFXfont *timer_font;
} display_fonts_s;

extern display_fonts_s Display_Fonts;
#endif

// Display parameter definition structure
typedef struct
{
    int32_t width;
    int32_t height;
    int32_t title_height;
    int32_t ble_status_height;
    int32_t ble_status_start_y;
    int32_t weight_height;
    int32_t weight_start_y;
    int32_t weight_x_pos;
    int32_t timer_height;
    int32_t timer_start_y;
    int32_t battery_status_height;
    int32_t battery_status_start_y;
} display_params_s;

extern display_params_s Display_Params;

// Display rotation enum
typedef enum Display_Rotation_e
{
    Display_Rotation_Portrait = 0,
    Display_Rotation_Landscape = 1
} Display_Rotation_e;

extern Display_Rotation_e display_rotation;

void wio_display_init(Display_Rotation_e rotation_i);
void wio_set_background(void);
void wio_ble_status_update(const char *pStatusMessage);
void wio_weight_display_update(float weight_i);
void wio_brew_timer_update(int sec_counter_i, int min_counter_i);
void wio_battery_status_update(void);