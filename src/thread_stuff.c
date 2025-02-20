#include "thread_stuff.h"
#include "jack_stuff.h"
#include "lf_queue.h"
#include "string.h"

ThreadStuff* create_thread_stuff(JackStuff* jack_stuff)
{
    ThreadStuff* thread_stuff = (ThreadStuff*)malloc(sizeof(ThreadStuff));
    memset(&thread_stuff->msg_queue, 0, sizeof(lf_queue_bss_state));
    memset(thread_stuff->element_array, 0, 16*sizeof(lf_queue_bss_element));
    lf_queue_init(&thread_stuff->msg_queue, thread_stuff->element_array, 16);

    thread_stuff->is_running = true;
    thread_stuff->attack = 0.0f;
    thread_stuff->decay = 0.0f;
    thread_stuff->sustain = 0.0f;
    thread_stuff->release = 0.0f;
    thread_stuff->is_play_pressed = false;

    thread_stuff->jack_stuff = jack_stuff;
    thread_stuff->vol = 0.0;
    thread_stuff->freq = 0.0;
    thread_stuff->adsr_height = 0.0f;
    thread_stuff->adsr_length = 0.0f;

    return thread_stuff;
}

void update_thread_stuff(ThreadStuff* thread_stuff, float attack, float decay, float sustain, float release, bool is_play_pressed, float vol, float freq, float* adsr_height, float* adsr_length)
{
    thread_stuff->attack = attack;
    thread_stuff->decay = decay;
    thread_stuff->sustain = sustain;
    thread_stuff->release = release;
    thread_stuff->is_play_pressed = is_play_pressed;
    thread_stuff->vol = vol;
    thread_stuff->freq = freq;
    *adsr_height = thread_stuff->adsr_height;
    *adsr_length = thread_stuff->adsr_length;
}
