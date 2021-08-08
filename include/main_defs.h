#pragma once

#include <Arduino.h>
#include <string.h>

#define MAX_NOTIFY_READ_CYCLE 50000U
#define DECENT_SCALE_PACKET_LEN 7
#define MIN_WEIGHT_INC ((float)0.2)

typedef struct
{
    float weight_q[6];
    float diff_q[5];
    uint8_t weight_q_idx;
    uint8_t diff_q_idx;
    float total_diff;
} weight_queue_s;

extern weight_queue_s scale_weight_q;

void weight_q_reset(weight_queue_s *pWeightQueue_i);
void weight_q_push(weight_queue_s *pWeightQueue_i, float weight_i);
void tap_counter(void);
void wio_accelerometer_init(void);
void wio_get_acceleroXYZ_str(char *pAcceleroStr_o);
void wio_gpio_init(void);
int16_t get_weight_tenthgramm_from_packet(char *pString_i);
float get_weight_gramm_from_packet(char *pString_i);
void Serial_print_chars_in_hex(uint8_t *pString_i, uint32_t len_i);
void Serial_println_chars_in_hex(uint8_t *pString_i, uint32_t len_i);
