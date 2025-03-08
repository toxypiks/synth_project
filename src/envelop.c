#include <stddef.h>
#include "envelop.h"
#include "helpers.h"
#include <stdio.h>

#define EPS 0.0001f

void envelop_trigger(Envelop *envelop, bool is_pressed)
{
  if(is_pressed && (envelop->envelop_state == RELEASED || envelop->envelop_state == DEFAULT)) {
    envelop->envelop_state = PRESSED_ATTACK;
    envelop->sample_count = 0;

    return;
  }
  if(!is_pressed && envelop->envelop_state != DEFAULT) {
      envelop->envelop_state = RELEASED;
  }
}

void envelop_change_adsr(Envelop *envelop, float attack, float decay, float sustain, float release)
{
  float sum = attack + decay + 0.5f + release;
  float al  = attack / sum;
  float dl  = decay / sum;
  float sl  = 0.5f / sum;
  float rl  = release / sum;

  envelop->attack = al;
  envelop->decay = dl;
  envelop->sustain = sustain;
  envelop->release = rl;
  envelop->sustain_length = sl;
}

void envelop_apply_in_buf(Envelop *envelop, float* buf, size_t buf_length)
{
  float attack_step_size = 1.0f/(48000.0f * envelop->attack);
  float decay_step_size = -(1.0f-envelop->sustain)/(48000.0f * envelop->decay);
  float sustain_step_size = 0.0f;
  float release_step_size = -1.0f*envelop->sustain/(48000.0f * envelop->release);

  float step_size[4] = {attack_step_size, decay_step_size, sustain_step_size, release_step_size};
  // TODO! overthink current_value and envelop->current_value
  // TODO! why do we need a step?
  float current_value = 0.0f;
  if(envelop->envelop_state == PRESSED_ATTACK
     || envelop->envelop_state == PRESSED_DECAY
     || envelop->envelop_state == PRESSED_SUSTAIN) {
    size_t step = 0;
    for(size_t i = 0; i < buf_length; i++) {
        current_value = step * step_size[envelop->envelop_state - 1] + envelop->current_value;
        //TODO edge cases!
        if(envelop->envelop_state == PRESSED_ATTACK && current_value >= 1.0f) {
            current_value = 1.0f;
            envelop->current_value = current_value;
            step = 0;
            envelop->envelop_state = PRESSED_DECAY;
        }
        if(envelop->envelop_state == PRESSED_DECAY && current_value <= envelop->sustain) {
            current_value = envelop->sustain;
            envelop->current_value = current_value;
            step = 0;
            envelop->envelop_state = PRESSED_SUSTAIN;
        }
        buf[i] = current_value * buf[i];
        step++;
      }
      envelop->sample_count += buf_length;
  } else if(envelop->envelop_state == RELEASED) {
      for(size_t i = 0; i < buf_length; i++) {
          current_value = MAX((i * release_step_size + envelop->current_value), 0.0f);
          buf[i] = current_value * buf[i];
          if(current_value <= 0.00000001f) {
              envelop->envelop_state = DEFAULT;
              envelop->sample_count = 0;
              envelop->sample_count_release = 0;
        } else {
            envelop->sample_count += 1;
            envelop->sample_count_release += 1;
        }
      }
  } else {
    for(size_t i = 0; i < buf_length; i++) {
      buf[i] = 0.0f;
    }

  }
  envelop->current_value = current_value;
}
