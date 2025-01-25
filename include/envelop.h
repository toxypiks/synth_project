#ifndef ENVELOP_H_
#define ENVELOP_H_

typedef enum EnvelopState {
  DEFAULT=0;
  PRESSED;
  RELEASED;
} EnvelopState;

typedef struct Envelop {
  EnvelopState envelop_state;
  size_t sample_count;
  float current_value;

  float attack;
  float release;
} Envelop;

void envelop_trigger(Envelop *envelop, bool is_pressed)
{
  if(is_pressed) {
    envelop->envelop_state = EnvelopState.PRESSED;
  }
  else {
    if(envelop->envenlop_state == EnvelopState.PRESSED) {
      envelop->envelop_state = EnvelopState.RELEASED;
    }
  }
}

void envelop_change_adsr(Envelop *envelop, float attack, float release)
{
  envelop->attack = attack;
  envelop->release = release;
}

void envelop_gen_envelop_in_buf(Envelop *envelop, float* buf, size_t buf_length)
{
  if(envelop->envelop_state == EnvelopState.PRESSED) {
      for(size_t i = 0; i < buf_length; i++) {
        buf[i] = i/buf_length;
      }
  }
  envelop->current_value = buf[buf_length-1];
}

#endif // ENVELOP_H
