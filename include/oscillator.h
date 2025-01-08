#ifndef OSCILLATOR_H_
#define OSCILLATOR_H_

#include <stddef.h>

typedef struct Oscillator {
  float freq;
  float time_step;
  float amp;
} Oscillator;

void change_frequency(Oscillator* osc, float new_freq);
void change_amp(Oscillator* osc, float new_amp);
void change_time_step(Oscillator* osc, float add_time_step);
void gen_signal_in_buf(Oscillator* osc, float* buf, size_t buf_length);

#endif // OSCILLATOR_H_
