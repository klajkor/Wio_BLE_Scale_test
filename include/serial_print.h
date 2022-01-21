#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include <string>

void serial_print_string_in_hex(std::string *pString_i, uint32_t len_i);
void serial_println_string_in_hex(std::string *pString_i, uint32_t len_i);
void serial_print_chars_in_hex(uint8_t *pString_i, uint32_t len_i);
void serial_println_chars_in_hex(uint8_t *pString_i, uint32_t len_i);
