#ifndef THREAD_STUFF_H_
#define THREAD_STUFF_H_

#include "jack_stuff.h"
#include <stdbool.h>
#include "lf_queue.h"

typedef struct ThreadStuff {
    lf_queue_bss_state model_msg_queue;
    lf_queue_bss_element model_element_array[16];
    lf_queue_bss_state raylib_msg_queue;
    lf_queue_bss_element raylib_element_array[16];
    bool is_running;
    // exchange variables
    float attack;
    float decay;
    float sustain;
    float release;
    bool is_play_pressed;
    JackStuff* jack_stuff;
    float vol;
    float freq;
    float adsr_height;
    float adsr_length;
} ThreadStuff;

ThreadStuff* create_thread_stuff(JackStuff* jack_stuff);
//void update_thread_stuff(ThreadStuff* thread_stuff, float attack, float decay, float sustain, float release, bool is_play_pressed, float vol, float freq, float* adsr_height, float* adsr_lenght);

#endif // THREAD_STUFF_H_
