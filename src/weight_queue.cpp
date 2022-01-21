#include "weight_queue.h"

weight_queue_s scale_weight_q;

void weight_q_reset(weight_queue_s *pWeightQueue_i)
{
  uint8_t i;
  pWeightQueue_i->weight_q_idx = 0;
  pWeightQueue_i->diff_q_idx = 0;
  pWeightQueue_i->total_diff = 0.0;
  for (i = 0; i < 5; i++)
  {
    pWeightQueue_i->weight_q[i] = 0.0;
    pWeightQueue_i->diff_q[i] = 0.0;
  }
  pWeightQueue_i->weight_q[5] = 0.0;
}

void weight_q_push(weight_queue_s *pWeightQueue_i, float weight_i)
{
  uint8_t i;
  float weight_diff;
  if (pWeightQueue_i->weight_q_idx == 0)
  {
    pWeightQueue_i->weight_q[0] = weight_i;
    pWeightQueue_i->weight_q_idx++;
    pWeightQueue_i->total_diff = 0.0;
  }
  else
  {
    pWeightQueue_i->total_diff = 0.0;
    if (pWeightQueue_i->weight_q_idx < 5)
    {
      pWeightQueue_i->weight_q[pWeightQueue_i->weight_q_idx] = weight_i;
      weight_diff = pWeightQueue_i->weight_q[pWeightQueue_i->weight_q_idx] - pWeightQueue_i->weight_q[pWeightQueue_i->weight_q_idx - 1];
      pWeightQueue_i->weight_q_idx++;
      if (weight_diff < 0)
      {
        weight_diff = 0;
      }
      pWeightQueue_i->diff_q[pWeightQueue_i->diff_q_idx] = weight_diff;
      for (i = 0; i < pWeightQueue_i->diff_q_idx; i++)
      {
        pWeightQueue_i->total_diff = pWeightQueue_i->total_diff + pWeightQueue_i->diff_q[i];
      }
      pWeightQueue_i->diff_q_idx++;
    }
    else
    {
      for (i = 5; i > 0; i--)
      {
        pWeightQueue_i->weight_q[i - 1] = pWeightQueue_i->weight_q[i];
      }
      pWeightQueue_i->weight_q[5] = weight_i;
      for (i = 4; i > 0; i--)
      {
        pWeightQueue_i->diff_q[i - 1] = pWeightQueue_i->diff_q[i];
        pWeightQueue_i->total_diff = pWeightQueue_i->total_diff + pWeightQueue_i->diff_q[i];
      }
      weight_diff = pWeightQueue_i->weight_q[5] - pWeightQueue_i->weight_q[4];
      if (weight_diff < 0)
      {
        weight_diff = 0;
      }
      pWeightQueue_i->diff_q[4] = weight_diff;
      pWeightQueue_i->total_diff = pWeightQueue_i->total_diff + weight_diff;
    }
  }
}
