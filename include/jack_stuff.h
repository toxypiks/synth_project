#ifndef JACK_STUFF_H_
#define JACK_STUFF_H_

#include "ray_out_buffer.h"
#include <jack/ringbuffer.h>
#include <jack/jack.h>
#include <stddef.h>
#include "lf_queue.h"

typedef struct JackStuff {
  jack_port_t* output_port;
  jack_port_t* midi_in_port;
  jack_ringbuffer_t* ringbuffer_audio;
  jack_ringbuffer_t* ringbuffer_video;
  jack_client_t* client;
  lf_queue_bss_state midi_msg_queue;
  lf_queue_bss_element midi_msg_array[16];
} JackStuff;

int process(jack_nframes_t nframes, void* jack_stuff_raw);
JackStuff* create_jack_stuff(char* client_name,size_t buffer_size);
void jack_stuff_clear(JackStuff* jack_stuff);

#endif // JACK_STUFF_H_
