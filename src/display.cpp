#include "display.h"

#if defined(__SAMD51__)
TFT_eSPI           tft;
static TFT_eSprite ble_status_display(&tft);
static TFT_eSprite scale_display(&tft);
static TFT_eSprite brew_timer(&tft);
static TFT_eSprite battery_status_display(&tft);

display_fonts_s Display_Fonts;
#endif

display_params_s   Display_Params;
Display_Rotation_e display_rotation = Display_Rotation_Portrait;

void wio_display_init(Display_Rotation_e rotation_i)
{
#if defined(__SAMD51__)
    tft.begin();
    tft.init();
    tft.setRotation(rotation_i);
    Display_Params.weight_height = 70;
    Display_Params.timer_height = 70;
    Display_Params.ble_status_height = 30;
    Display_Params.battery_status_height = 30;
    switch (rotation_i)
    {
    case Display_Rotation_Landscape:
        Display_Params.width = 320;
        Display_Params.height = 240;
        Display_Params.title_height = 50;
        Display_Params.weight_start_y = Display_Params.title_height;
        Display_Params.weight_x_pos = Display_Params.width - 40;
        Display_Params.timer_start_y = Display_Params.weight_start_y + Display_Params.weight_height;
        break;
    case Display_Rotation_Portrait:
        Display_Params.width = 240;
        Display_Params.height = 320;
        Display_Params.title_height = 60;
        Display_Params.weight_start_y = Display_Params.title_height + 20;
        Display_Params.weight_x_pos = Display_Params.width - 10;
        Display_Params.timer_start_y = Display_Params.weight_start_y + Display_Params.weight_height;
        break;
    }
    Display_Params.ble_status_start_y =
        Display_Params.height - Display_Params.ble_status_height - Display_Params.battery_status_height - 1;
    Display_Params.battery_status_start_y = Display_Params.height - Display_Params.battery_status_height - 1;
    Display_Fonts.title_font = &FreeSansBold18pt7b;
    Display_Fonts.weigth_font = &FreeSansBold18pt7b;
    Display_Fonts.timer_font = &FreeSansBold18pt7b;
    Display_Fonts.status_font = &FreeSans9pt7b;
    tft.fillScreen(TFT_BLACK);
    ble_status_display.createSprite(Display_Params.width, Display_Params.ble_status_height);
    scale_display.createSprite(Display_Params.width, Display_Params.weight_height);
    brew_timer.createSprite(Display_Params.width, Display_Params.timer_height);
    battery_status_display.createSprite(Display_Params.width, Display_Params.battery_status_height);
    Serial.println("Display init completed.");
#endif
}

void wio_set_background(void)
{
#if defined(__SAMD51__)
    tft.fillScreen(TFT_WHITE);
    tft.fillRect(0, 0, Display_Params.width, Display_Params.title_height - 1, TFT_DARKGREEN);
    tft.setTextColor(TFT_WHITE);
    tft.setFreeFont(Display_Fonts.title_font);
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM); // Middle-center
    tft.drawString("Decent Scale", (Display_Params.width / 2) - 1, (Display_Params.title_height / 2) - 2);
    Serial.println("Display set background completed.");
#endif
}

void wio_ble_status_update(const char *pStatusMessage)
{
#if defined(__SAMD51__)
    ble_status_display.fillSprite(TFT_LIGHTGREY);
    ble_status_display.drawFastHLine(0, 0, Display_Params.width, TFT_BLUE);
    ble_status_display.setFreeFont(Display_Fonts.status_font);
    ble_status_display.setTextColor(TFT_BLACK);
    ble_status_display.setTextDatum(ML_DATUM); // Middle-left
    ble_status_display.drawString(pStatusMessage, 3, (Display_Params.ble_status_height / 2) - 1);
    ble_status_display.pushSprite(0, Display_Params.ble_status_start_y);
#endif
}

void wio_weight_display_update(float weight_i)
{
#if defined(__SAMD51__)
    char weight_str[8];
    scale_display.fillSprite(TFT_WHITE);
    scale_display.setFreeFont(Display_Fonts.weigth_font);
    scale_display.setTextColor(TFT_BLACK);
    scale_display.setTextSize(2);
    snprintf(weight_str, sizeof(weight_str), "%5.1f", weight_i);
    scale_display.setTextDatum(MR_DATUM); // Middle-right
    scale_display.drawString((const char *)weight_str, Display_Params.weight_x_pos,
                             (Display_Params.weight_height / 2) - 1);
    scale_display.pushSprite(0, Display_Params.weight_start_y);
#endif
}

void wio_brew_timer_update(int sec_counter_i, int min_counter_i)
{
    char timer_str[10];
    brew_timer.fillSprite(TFT_WHITE);
    brew_timer.drawFastHLine(0, 0, Display_Params.width, TFT_BLUE);
    brew_timer.setFreeFont(Display_Fonts.timer_font);
    brew_timer.setTextColor(TFT_BLACK);
    brew_timer.setTextDatum(MC_DATUM); // Middle-center
    snprintf(timer_str, 9, "%02i:%02i", min_counter_i, sec_counter_i);
    brew_timer.drawString((const char *)timer_str, (int32_t)((Display_Params.width / 2) - 1),
                          (int32_t)(Display_Params.timer_height / 2) - 1);
    brew_timer.pushSprite(0, Display_Params.timer_start_y);
}

void wio_battery_status_update(void)
{
    char    wio_battery_status_msg[15];
    int32_t battery_state;
    battery_state = get_battery_state();
    if (battery_state >= 0)
    {
        snprintf(wio_battery_status_msg, 14, "Batt: %3d %%", (int)battery_state);
    }
    else
    {
        snprintf(wio_battery_status_msg, 14, "Batt: N/A");
    }
    battery_status_display.fillSprite(TFT_LIGHTGREY);
    battery_status_display.drawFastHLine(0, 0, Display_Params.width, TFT_BLUE);
    battery_status_display.setFreeFont(Display_Fonts.status_font);
    battery_status_display.setTextColor(TFT_BLACK);
    battery_status_display.setTextDatum(MC_DATUM); // Middle-center
    battery_status_display.drawString(wio_battery_status_msg, (Display_Params.width / 2) - 1,
                                      (Display_Params.battery_status_height / 2) - 1);
    battery_status_display.pushSprite(0, Display_Params.battery_status_start_y);
}
