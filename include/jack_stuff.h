#ifndef JACK_STUFF_H_
#define JACK_STUFF_H_

#include "ray_out_buffer.h"
#include <jack/ringbuffer.h>
#include <jack/jack.h>
#include <stddef.h>

typedef struct JackStuff {
  jack_port_t* output_port;
  jack_ringbuffer_t* ringbuffer_audio;
  jack_ringbuffer_t* ringbuffer_video;
  jack_client_t* client;
} JackStuff;

int process(jack_nframes_t nframes, void* jack_stuff_raw);
JackStuff* create_jack_stuff(char* client_name,size_t buffer_size);
void jack_stuff_clear(JackStuff* jack_stuff);

#endif // JACK_STUFF_H_
