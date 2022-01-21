#include "serial_print.h"

void serial_print_string_in_hex(std::string *pString_i, uint32_t len_i)
{
    uint32_t i;
    uint32_t max_len;
    max_len = pString_i->length();
    if (len_i < max_len)
    {
        max_len = len_i;
    }
    for (i = 0; i < max_len; i++)
    {
        if (pString_i->at(i) < 16)
        {
            Serial.print("0x0");
        }
        else
        {
            Serial.print("0x");
        }
        Serial.print(pString_i->at(i), HEX);
        Serial.print(",");
    }
}

void serial_println_string_in_hex(std::string *pString_i, uint32_t len_i)
{
    serial_print_string_in_hex(pString_i, len_i);
    Serial.println("");
}

void serial_print_chars_in_hex(uint8_t *pString_i, uint32_t len_i)
{
    uint32_t i;
    for (i = 0; i < len_i; i++)
    {
        if (pString_i[i] < 16)
        {
            Serial.print("0x0");
        }
        else
        {
            Serial.print("0x");
        }
        Serial.print(pString_i[i], HEX);
        Serial.print(",");
    }
}

void serial_println_chars_in_hex(uint8_t *pString_i, uint32_t len_i)
{
    serial_print_chars_in_hex(pString_i, len_i);
    Serial.println("");
}
