#include "jack_stuff.h"

int process(jack_nframes_t nframes, void* jack_stuff_raw)
{
  JackStuff* jack_stuff = (JackStuff*)jack_stuff_raw;
  float* output_buffer= (float*)jack_port_get_buffer (jack_stuff->output_port, nframes);

  if(jack_stuff->ringbuffer_audio){ // is buffer even there?
    // see if theres enough data in buffer to read nframes out of it (num_bytes >nframes)
    size_t num_bytes = jack_ringbuffer_read_space (jack_stuff->ringbuffer_audio);
    if(num_bytes >= (nframes* sizeof(float))) {
      jack_ringbuffer_read(jack_stuff->ringbuffer_audio, (char*)output_buffer, nframes * sizeof(float));
    } else {
      for (int i = 0; i < (int) nframes; i++)
      {
        output_buffer[i] = 0.0;
      }
    }
  }
  return 0;
}

JackStuff* create_jack_stuff(char* client_name,size_t buffer_size){
  JackStuff* jack_stuff = (JackStuff*)malloc(sizeof(JackStuff));

  jack_stuff->output_port = 0;
  jack_stuff->ringbuffer_audio = NULL;
  jack_stuff->ringbuffer_video = NULL;
  jack_stuff->client = NULL;

  jack_stuff->client = jack_client_open (client_name,
                                        JackNullOption,
                                        0,
                                        0 );

  jack_stuff->output_port = jack_port_register (jack_stuff->client,
                                    "output",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput,
                                    0 );

  size_t my_size = buffer_size * sizeof(float);
  jack_stuff->ringbuffer_audio = jack_ringbuffer_create(my_size);
  jack_stuff->ringbuffer_video = jack_ringbuffer_create(my_size);

  jack_set_process_callback(jack_stuff->client, process, (void*)jack_stuff);
  //client.set_sample_rate(48000);
  jack_activate(jack_stuff->client);
  return jack_stuff;
}

void jack_stuff_clear(JackStuff* jack_stuff) {
  jack_deactivate(jack_stuff->client);
  jack_client_close(jack_stuff->client);
  jack_ringbuffer_free(jack_stuff->ringbuffer_audio);
  free(jack_stuff);
}
