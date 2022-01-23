#include "wio_battery.h"

static const uint32_t BATTERY_CAPACITY = 650; // Set Wio Terminal Battery's Capacity
static bool           battery_present = false;

bool battery_init(void)
{
    battery_present = lipo.begin();
    if (battery_present)
    {
        serial_println("Connected to battery");
        lipo.setCapacity(BATTERY_CAPACITY);
    }
    else
    {
        serial_println("Error: Unable to communicate with battery");
        return false;
    }
    return battery_present;
}

int32_t get_battery_state(void)
{
    if (battery_present)
    {
        return lipo.soc();
    }
    else
    {
        return -1;
    }
}
