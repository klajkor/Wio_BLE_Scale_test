#include "serial_print.h"

void serial_print_string_in_hex(std::string *p_string_i, uint32_t len_i)
{
    uint32_t i;
    uint32_t max_len;
    max_len = p_string_i->length();
    if (ENABLE_DEBUG_PRINT)
    {
        if (len_i < max_len)
        {
            max_len = len_i;
        }
        for (i = 0; i < max_len; i++)
        {
            if (p_string_i->at(i) < 16)
            {
                Serial.print("0x0");
            }
            else
            {
                Serial.print("0x");
            }
            Serial.print(p_string_i->at(i), HEX);
            Serial.print(",");
        }
    }
}

void serial_println_string_in_hex(std::string *p_string_i, uint32_t len_i)
{
    if (ENABLE_DEBUG_PRINT)
    {
        serial_print_string_in_hex(p_string_i, len_i);
        Serial.println("");
    }
}

void serial_print_chars_in_hex(uint8_t *p_string_i, uint32_t len_i)
{
    uint32_t i;
    if (ENABLE_DEBUG_PRINT)
    {
        for (i = 0; i < len_i; i++)
        {
            if (p_string_i[i] < 16)
            {
                Serial.print("0x0");
            }
            else
            {
                Serial.print("0x");
            }
            Serial.print(p_string_i[i], HEX);
            Serial.print(",");
        }
    }
}

void serial_println_chars_in_hex(uint8_t *p_string_i, uint32_t len_i)
{
    if (ENABLE_DEBUG_PRINT)
    {
        serial_print_chars_in_hex(p_string_i, len_i);
        Serial.println("");
    }
}

void serial_print(const char *p_string_i)
{
    if (ENABLE_DEBUG_PRINT)
    {
        Serial.print(p_string_i);
    }
}

void serial_println(const char *p_string_i)
{
    if (ENABLE_DEBUG_PRINT)
    {
        Serial.println(p_string_i);
    }
}
