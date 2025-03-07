#include "synth_model.h"
#include "oscillator.h"
#include "envelop.h"
#include <stdlib.h>
#include <string.h>

SynthModel* create_synth_model() {
  SynthModel* synth_model = (SynthModel*)malloc(sizeof(SynthModel));
  memset(synth_model->osc.amp, 1.0f, 1024);
  synth_model->osc.freq = 440.0;
  synth_model->osc.phase = 0.0;

  //TODO: init osc and envelop
  return synth_model;
}
void synth_model_update(SynthModel* synth_model,
                        float* signal_buffer,
                        float new_vol,
                        float new_freq,
                        float* adsr_current_value,
                        float* adsr_length
  ) {
  float sum_ads = 0.0f;
  float sum_adsr = 0.0f;

  change_frequency(&synth_model->osc, new_freq);
  change_amp(&synth_model->osc, new_vol);
  gen_signal_in_buf(&synth_model->osc,  signal_buffer, 1024, &synth_model->adsr_envelop);
  *adsr_current_value = synth_model->adsr_envelop.current_value;

  // calculate adsr x,y0,y1 values
  sum_ads = 48000.0f *(synth_model->adsr_envelop.attack + synth_model->adsr_envelop.decay + 0.5);
  sum_adsr = 48000.0f *(synth_model->adsr_envelop.attack + synth_model->adsr_envelop.decay + 0.5 + synth_model->adsr_envelop.release);

  if((synth_model->adsr_envelop.sample_count < sum_ads) && (synth_model->adsr_envelop.envelop_state ==  PRESSED_ATTACK
                                                            || synth_model->adsr_envelop.envelop_state == PRESSED_DECAY
                                                            || synth_model->adsr_envelop.envelop_state == PRESSED_SUSTAIN))
  {
    *adsr_length = synth_model->adsr_envelop.sample_count/ (sum_adsr);
  } else if ((synth_model->adsr_envelop.sample_count > sum_adsr) && synth_model->adsr_envelop.envelop_state ==  PRESSED_ATTACK
             || synth_model->adsr_envelop.envelop_state == PRESSED_DECAY
             || synth_model->adsr_envelop.envelop_state == PRESSED_SUSTAIN)
  {
    *adsr_length = (sum_ads/(sum_adsr));
  } else if (synth_model->adsr_envelop.envelop_state == RELEASED) {
    *adsr_length =  synth_model->adsr_envelop.attack + synth_model->adsr_envelop.decay + 0.5 + (synth_model->adsr_envelop.sample_count_release / (48000.0f));
  }
}

void synth_model_envelope_update(SynthModel* synth_model,
                                 float attack,
                                 float decay,
                                 float sustain,
                                 float release,
                                 bool is_triggered)
{
    // TODO: signal state via ringbuffer
    envelop_change_adsr(&synth_model->adsr_envelop,attack, decay, sustain, release);
    envelop_trigger(&synth_model->adsr_envelop,is_triggered);
}


void synth_model_clear(SynthModel* synth_model) {
  free(synth_model);
}
