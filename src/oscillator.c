#include "oscillator.h"
#include <math.h>

void change_frequency(Oscillator* osc, float new_freq) {
  osc->freq = new_freq;
}

void change_amp(Oscillator* osc, float new_amp) {
  osc->amp = new_amp;
}

void gen_signal_in_buf(Oscillator* osc, float* buf, size_t buf_length) {
  float new_phase = 0.0f;
  for(size_t i = 0; i < buf_length; ++i) {
    float phase = fmod((2*M_PI*osc->freq*i/48000.0f + osc->phase), 2.0*M_PI);
    buf[i] = osc->amp * sin(phase);
    new_phase = phase;
  }
  osc->phase = new_phase;
}
