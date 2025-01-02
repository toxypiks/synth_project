#include "oscillator.h"
#include <math.h>

void change_frequency(Oscillator* osc, float new_freq) {
  osc->freq = new_freq;
}
void change_time_step(Oscillator* osc, float add_time_step){
   osc->time_step += add_time_step;
}
void gen_signal_in_buf(Oscillator* osc, float* buf, size_t buf_length) {
  for(size_t i = 0; i < buf_length; ++i) {
    size_t t = osc->time_step + i;
    buf[i] = sinf(2*M_PI*osc->freq*(t/48000.0));
  }
}
