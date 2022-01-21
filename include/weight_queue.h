#pragma once

#include <Arduino.h>

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