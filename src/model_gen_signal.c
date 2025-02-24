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

void set_adsr_values(void* adsr_new_raw, void* adsr_values_raw){
    ADSR* adsr_values = (ADSR*)adsr_values_raw;
    ADSR* adsr_new = (ADSR*)adsr_new_raw;
    adsr_values->attack  = adsr_new->attack;
    adsr_values->decay   = adsr_new->decay;
    adsr_values->sustain = adsr_new->sustain;
    adsr_values->release = adsr_new->release;
    //free(adsr_new);
};

void* model_gen_signal_thread_fct(void* thread_stuff_raw)
{
    ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;
    SynthModel* synth_model = create_synth_model();

    ADSR adsr_values = {0};
    float vol = 0.0;
    float freq = 0.0;
    bool is_play_pressed = false;

    MsgHdl msg_hdl = {0};
    float adsr_height = 0.0;
    float adsr_length = 0.0;

    msg_hdl_add_key2fct(&msg_hdl, "adsr", set_adsr_values, (void*)&adsr_values);
    msg_hdl_add_key2fct(&msg_hdl, "vol", set_float_value, (void*)&vol);
    msg_hdl_add_key2fct(&msg_hdl, "freq", set_float_value, (void*)&freq);
    msg_hdl_add_key2fct(&msg_hdl, "is_play_pressed", set_bool_value, (void*)&is_play_pressed);

    while(thread_stuff->is_running) {
        msg_hdling(&msg_hdl, &thread_stuff->model_msg_queue);
        printf("attack: %f\n", adsr_values.attack);

        size_t num_bytes = jack_ringbuffer_read_space(thread_stuff->jack_stuff->ringbuffer_audio);
        float data_buf[1024];

        if(num_bytes < 4800 * sizeof(float))
        {
            thread_stuff->adsr_length = 0;
            synth_model_envelope_update(synth_model,
                                        adsr_values.attack,
                                        adsr_values.decay,
                                        adsr_values.sustain,
                                        adsr_values.release,
                                        is_play_pressed);

            synth_model_update(synth_model,
                               data_buf,
                               vol,
                               freq,
                               &adsr_height,
                               &adsr_length);

            // TODO msg send in better function
            float *adsr_height_msg = malloc(sizeof(float));
            *adsr_height_msg = adsr_height;
            int ret_adsr_height = lf_queue_push(&thread_stuff->raylib_msg_queue, "adsr_height", adsr_height_msg);

            float *adsr_length_msg = malloc(sizeof(float));
            *adsr_length_msg = adsr_length;
            int ret_adsr_length = lf_queue_push(&thread_stuff->raylib_msg_queue, "adsr_length", adsr_length_msg);
            jack_ringbuffer_write(thread_stuff->jack_stuff->ringbuffer_audio, (void *)data_buf, 1024*sizeof(float));
            jack_ringbuffer_write(thread_stuff->jack_stuff->ringbuffer_video, (void *)data_buf, 1024*sizeof(float));
        } else {
            usleep(4680);
        }
    }
    synth_model_clear(synth_model);
    printf("model_gen_signal_thread ended, Good bye! \n");
    return NULL;
}
