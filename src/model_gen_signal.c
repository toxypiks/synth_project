#include <stdio.h>
#include <jack/ringbuffer.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "synth_model.h"
#include "thread_stuff.h"
#include "adsr.h"
#include "msg_handler.h"
#include "midi_msg.h"

void set_adsr_values(void* adsr_new_raw, void* adsr_values_raw){
    ADSR* adsr_values = (ADSR*)adsr_values_raw;
    ADSR* adsr_new = (ADSR*)adsr_new_raw;
    adsr_values->attack  = adsr_new->attack;
    adsr_values->decay   = adsr_new->decay;
    adsr_values->sustain = adsr_new->sustain;
    adsr_values->release = adsr_new->release;
    //free(adsr_new);
};

void set_midi_msg(void* midi_msg_new_raw, void* midi_msg_raw){
    MidiMsg* midi_msg = (MidiMsg*)midi_msg_raw;
    MidiMsg* midi_msg_new = (MidiMsg*)midi_msg_new_raw;
    memcpy(midi_msg, midi_msg_new, sizeof(MidiMsg));
}

void* model_gen_signal_thread_fct(void* thread_stuff_raw)
{
    ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;
    SynthModel* synth_model = create_synth_model();

    ADSR adsr_values = {0};
    float vol = 0.0;
    float freq = 0.0;
    bool is_play_pressed = false;

    MidiMsg midi_msg = {0};

    MsgHdl msg_hdl = {0};
    float adsr_height = 0.0;
    float adsr_length = 0.0;

    msg_hdl_add_key2fct(&msg_hdl, "adsr", set_adsr_values, (void*)&adsr_values);
    msg_hdl_add_key2fct(&msg_hdl, "vol", set_float_value, (void*)&vol);
    msg_hdl_add_key2fct(&msg_hdl, "freq", set_float_value, (void*)&freq);
    msg_hdl_add_key2fct(&msg_hdl, "is_play_pressed", set_bool_value, (void*)&is_play_pressed);
    msg_hdl_add_key2fct(&msg_hdl, "midi_msg", set_midi_msg, (void*)&midi_msg);

    while(thread_stuff->is_running) {
        msg_hdling(&msg_hdl, &thread_stuff->model_msg_queue);
        msg_hdling(&msg_hdl, &thread_stuff->jack_stuff->midi_msg_queue);

        size_t num_bytes = jack_ringbuffer_read_space(thread_stuff->jack_stuff->ringbuffer_audio);
        float data_buf[1024];

        if(num_bytes < 4800 * sizeof(float))
        {
            adsr_length = 0;
            synth_model_envelope_update(synth_model,
                                        adsr_values.attack,
                                        adsr_values.decay,
                                        adsr_values.sustain,
                                        adsr_values.release,
                                        // is_play_pressed);
                                        midi_msg.is_on);

            synth_model_update(synth_model,
                               data_buf,
                               vol,
                               //  freq,
                               midi_msg.freq,
                               &adsr_height,
                               &adsr_length);
            printf("model_gen_signal: adsr_length: %f height: %f state: %d,release: %f\n",adsr_length, adsr_height, synth_model->adsr_envelop.envelop_state, synth_model->adsr_envelop.release);
            // TODO msg send in better function
            int ret_adsr_height = lf_queue_push(&thread_stuff->raylib_msg_queue, "adsr_height", (void*)&adsr_height, sizeof(float));
            int ret_adsr_length = lf_queue_push(&thread_stuff->raylib_msg_queue, "adsr_length", (void*)&adsr_length, sizeof(float));

            jack_ringbuffer_write(thread_stuff->jack_stuff->ringbuffer_audio, (void *)data_buf, 1024*sizeof(float));
            size_t num_bytes = jack_ringbuffer_read_space(thread_stuff->jack_stuff->ringbuffer_video);
            jack_ringbuffer_write(thread_stuff->jack_stuff->ringbuffer_video, (void *)data_buf, 1024*sizeof(float));
            num_bytes = jack_ringbuffer_read_space(thread_stuff->jack_stuff->ringbuffer_video);
        } else {
            usleep(2000);
        }
    }
    synth_model_clear(synth_model);
    printf("model_gen_signal_thread ended, Good bye! \n");
    return NULL;
}
