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

    MsgHdl msg_hdl = {0};
    char* key = "adsr";
    msg_hdl_add_key2fct(&msg_hdl, key, set_adsr_values, (void*)&adsr_values);


    while(thread_stuff->is_running) {
        msg_hdling(&msg_hdl, &thread_stuff->msg_queue);
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
                                        thread_stuff->is_play_pressed);

            synth_model_update(synth_model,
                               data_buf,
                               thread_stuff->vol,
                               thread_stuff->freq,
                               &thread_stuff->adsr_height,
                               &thread_stuff->adsr_length);

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
