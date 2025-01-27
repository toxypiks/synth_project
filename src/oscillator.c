#include "oscillator.h"
#include <math.h>

void change_frequency(Oscillator* osc, float new_freq)
{
  osc->freq = new_freq;
}

void change_amp(Oscillator* osc, float new_amp)
{
  float old_amp = osc->amp[1023];
  float delta_step = old_amp == new_amp ? 0 : (new_amp - old_amp)/1024;
  for(size_t i = 0; i < 1024; ++i) {
    osc->amp[i] = old_amp + (i * delta_step);
  }
}

void gen_signal_in_buf(Oscillator* osc, float* buf, size_t buf_length, Envelop* adsr_envelop)
{
  float new_phase = 0.0f;
  for(size_t i = 0; i < buf_length; ++i) {
    float phase = fmod((2*M_PI*osc->freq*i/48000.0f + osc->phase), 2.0*M_PI);
    buf[i] = osc->amp[i] * sin(phase);
    new_phase = phase;
  }
  osc->phase = new_phase;
  envelop_apply_in_buf(adsr_envelop, buf, buf_length);
}
