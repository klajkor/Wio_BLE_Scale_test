#include "display.h"

#if defined(__SAMD51__)
TFT_eSPI tft;
TFT_eSprite status_display(&tft);
TFT_eSprite scale_display(&tft);
TFT_eSprite brew_timer(&tft);

display_fonts_s Display_Fonts;
#endif

display_params_s Display_Params;
Display_Rotation_e display_rotation = Display_Rotation_Portrait;

void wio_display_init(Display_Rotation_e rotation_i)
{
#if defined(__SAMD51__)
    tft.begin();
    tft.init();
    tft.setRotation(rotation_i);
    Display_Params.weight_height = 70;
    Display_Params.timer_height = 70;
    Display_Params.status_height = 30;
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
    Display_Params.status_start_y = Display_Params.height - Display_Params.status_height - 1;
    Display_Fonts.title_font = &FreeSansBold18pt7b;
    Display_Fonts.weigth_font = &FreeSansBold18pt7b;
    Display_Fonts.timer_font = &FreeSansBold18pt7b;
    Display_Fonts.status_font = &FreeSans9pt7b;
    tft.fillScreen(TFT_BLACK);
    status_display.createSprite(Display_Params.width, Display_Params.status_height);
    scale_display.createSprite(Display_Params.width, Display_Params.weight_height);
    brew_timer.createSprite(Display_Params.width, Display_Params.timer_height);
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

void wio_status_update(char *pStatusMessage)
{
#if defined(__SAMD51__)
    //status_display.createSprite(Display_Params.width, Display_Params.status_height);
    status_display.fillSprite(TFT_LIGHTGREY);
    status_display.drawFastHLine(0, 0, Display_Params.width, TFT_BLUE);
    status_display.setFreeFont(Display_Fonts.status_font);
    status_display.setTextColor(TFT_BLACK);
    status_display.setTextDatum(ML_DATUM); // Middle-left
    status_display.drawString((const char *)pStatusMessage, 3, (Display_Params.status_height / 2) - 1);
    status_display.pushSprite(0, Display_Params.status_start_y);
#endif
}

void wio_weight_display_update(float weight_i)
{
#if defined(__SAMD51__)
    char weight_str[8];
    //scale_display.createSprite(Display_Params.width, Display_Params.weight_height);
    scale_display.fillSprite(TFT_WHITE);
    scale_display.setFreeFont(Display_Fonts.weigth_font);
    scale_display.setTextColor(TFT_BLACK);
    scale_display.setTextSize(2);
    snprintf(weight_str, sizeof(weight_str), "%5.1f", weight_i);
    scale_display.setTextDatum(MR_DATUM); // Middle-right
    scale_display.drawString((const char *)weight_str, Display_Params.weight_x_pos, (Display_Params.weight_height / 2) - 1);
    scale_display.pushSprite(0, Display_Params.weight_start_y);
#endif
}

void wio_brew_timer_update(int sec_counter_i)
{
#if defined(__SAMD51__)
    //status_display.createSprite(Display_Params.width, Display_Params.status_height);
    char timer_str[32];
    brew_timer.fillSprite(TFT_WHITE);
    brew_timer.drawFastHLine(0, 0, Display_Params.width, TFT_BLUE);
    brew_timer.setFreeFont(Display_Fonts.timer_font);
    brew_timer.setTextColor(TFT_BLACK);
    brew_timer.setTextDatum(ML_DATUM); // Middle-left
    snprintf(timer_str, 30, "Sec: %02i", sec_counter_i);
    brew_timer.drawString((const char *)timer_str, 3, (int32_t)(Display_Params.timer_height / 2) - 1);
    brew_timer.pushSprite(0, Display_Params.timer_start_y);
#endif
}