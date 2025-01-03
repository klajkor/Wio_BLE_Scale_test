#include "wio_gpio.h"

#if defined(__SAMD51__)
uint32_t KEY_STOPWATCH = WIO_KEY_A;
uint32_t KEY_SCALE_TARE = WIO_KEY_C;
#endif

bool key_stopwatch_pressed = false;
bool key_scale_tare_pressed = false;

void wio_gpio_init(void)
{
#if defined(__SAMD51__)
    pinMode(KEY_STOPWATCH, INPUT_PULLUP);
    pinMode(KEY_SCALE_TARE, INPUT_PULLUP);
    pinMode(WIO_5S_PRESS, INPUT_PULLUP);
    Serial.println("GPIO init completed.");
#endif
}

// Interrupt Service Routine for Stopwatch Button
void key_stopwatch_handler(void)
{
    key_stopwatch_pressed = true;
}

// Interrupt Service Routine for Scale Tare Button
void key_scale_handler(void)
{
    key_scale_tare_pressed = true;
}
