#ifndef ENVELOP_H_
#define ENVELOP_H_

#include <stdbool.h>

typedef enum EnvelopState {
  DEFAULT=0,
  PRESSED,
  RELEASED,
} EnvelopState;

typedef struct Envelop {
  EnvelopState envelop_state;
  size_t sample_count;
  float current_value;

  float attack;
  float release;
} Envelop;

void envelop_trigger(Envelop *envelop, bool is_pressed);

void envelop_change_adsr(Envelop *envelop, float attack, float release);

void envelop_apply_in_buf(Envelop *envelop, float* buf, size_t buf_length);

#endif // ENVELOP_H