#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include <string>

#include "main_defs.h"

void serial_print_string_in_hex(std::string *p_string_i, uint32_t len_i);
void serial_println_string_in_hex(std::string *p_string_i, uint32_t len_i);
void serial_print_chars_in_hex(uint8_t *p_string_i, uint32_t len_i);
void serial_println_chars_in_hex(uint8_t *p_string_i, uint32_t len_i);
void serial_print(const char *p_string_i);
void serial_println(const char *p_string_i);
