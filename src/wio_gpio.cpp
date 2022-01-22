#include "wio_gpio.h"

void wio_gpio_init(void)
{
#if defined(__SAMD51__)
    pinMode(WIO_KEY_A, INPUT_PULLUP);
    pinMode(WIO_KEY_B, INPUT_PULLUP);
    pinMode(WIO_KEY_C, INPUT_PULLUP);
    pinMode(WIO_5S_PRESS, INPUT_PULLUP);
    pinMode(WIO_MIC, INPUT);
    Serial.println("GPIO init completed.");
#endif
}
