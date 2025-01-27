#include <stddef.h>
#include "envelop.h"

#define MAX(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define MIN(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })

void envelop_trigger(Envelop *envelop, bool is_pressed)
{
  if(is_pressed) {
    envelop->envelop_state = PRESSED;
  }
  else {
    if(envelop->envelop_state == PRESSED) {
      envelop->envelop_state = RELEASED;
    }
  }
}

void envelop_change_adsr(Envelop *envelop, float attack, float release)
{
  envelop->attack = attack;
  envelop->release = release;
}

void envelop_apply_in_buf(Envelop *envelop, float* buf, size_t buf_length)
{
  float attack_step_size = 1.0f/(48000.0f * envelop->attack);
  float release_step_size = 1.0f/(48000.0f * envelop->release);
  float current_value = 0.0;
  if(envelop->envelop_state == PRESSED) {
      for(size_t i = 0; i < buf_length; i++) {
        current_value = MIN((i * attack_step_size + envelop->current_value), 1.0f);
        buf[i] = current_value * buf[i];
      }
  } else if(envelop->envelop_state == RELEASED) {
      for(size_t i = 0; i < buf_length; i++) {
        current_value = MAX((envelop->current_value - i * release_step_size), 0.0f);
        buf[i] = current_value * buf[i];
      }
    }
  envelop->current_value = current_value;
}
