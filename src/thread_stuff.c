#include "thread_stuff.h"
#include "jack_stuff.h"
#include "lf_queue.h"
#include "string.h"

ThreadStuff* create_thread_stuff(JackStuff* jack_stuff)
{
    ThreadStuff* thread_stuff = (ThreadStuff*)malloc(sizeof(ThreadStuff));
    memset(&thread_stuff->model_msg_queue, 0, sizeof(lf_queue_bss_state));
    memset(thread_stuff->model_element_array, 0, 16*sizeof(lf_queue_bss_element));
    lf_queue_init(&thread_stuff->model_msg_queue, thread_stuff->model_element_array, 16);

    memset(&thread_stuff->raylib_msg_queue, 0, sizeof(lf_queue_bss_state));
    memset(thread_stuff->raylib_element_array, 0, 16*sizeof(lf_queue_bss_element));
    lf_queue_init(&thread_stuff->raylib_msg_queue, thread_stuff->raylib_element_array, 16);

    thread_stuff->is_running = true;
    thread_stuff->jack_stuff = jack_stuff;

    return thread_stuff;
}
