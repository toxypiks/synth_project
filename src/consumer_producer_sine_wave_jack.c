#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <jack/ringbuffer.h>
#include "jack_stuff.h"
#include "oscillator.h"

typedef struct ThreadStuff{
  size_t index;
  float data[1024];
  jack_ringbuffer_t* ringbuffer_audio;
  jack_ringbuffer_t* ringbuffer_video;
  bool running;
  Oscillator osc;
  jack_client_t* client;
  jack_port_t* output_port;
} ThreadStuff;

void* print_data_thread_fct(void* thread_stuff_raw) {
  ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;
  float read_data[1024] = {0};
  while(thread_stuff->running) {
    size_t num_bytes = jack_ringbuffer_read_space(thread_stuff->ringbuffer_video);
    if(num_bytes >= 1024* sizeof(float)){
      jack_ringbuffer_read(thread_stuff->ringbuffer_video, (char*)&read_data, 1024 * sizeof(float));
      for (size_t i = 0; i < 1024; i+=100) {
        printf("%f\n", read_data[i]);
      }
    }
    usleep(46875);
  }
}

void* gen_data_thread_fct(void* thread_stuff_raw) {
  float cache_buf[1024] = {0};
  ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;

  while(thread_stuff->running){

    // now jack ringbuffer_data

    // now jack ringbuffer_audio
    // 1. check if space
    size_t num_bytes = jack_ringbuffer_read_space(thread_stuff->ringbuffer_audio);
    if(num_bytes > 1024 * sizeof(float)) {
          usleep(46800); // 0.01 sec
    } else {
      gen_signal_in_buf(&thread_stuff->osc, cache_buf, 1024);
      //memcopy from buffer -> ringbuffer
      jack_ringbuffer_write(
        thread_stuff->ringbuffer_audio,
        (void *) &cache_buf,
        1024 * sizeof(float)
        );
    }
    // TODO need to sync
    size_t num_bytes_video = jack_ringbuffer_read_space(thread_stuff->ringbuffer_video);
    if(num_bytes_video <= 1024 * sizeof(float)) {
      jack_ringbuffer_write(
        thread_stuff->ringbuffer_video,
        (void *) &cache_buf,
        1024 * sizeof(float)
      );
    }
  }
}

int jack_process(jack_nframes_t nframes, void* thread_stuff_raw){
  ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;
  float* output_buffer= (float*)jack_port_get_buffer(thread_stuff->output_port, nframes);
  if(thread_stuff->ringbuffer_audio){ // is buffer even there?
    // see if theres enough data in buffer to read nframes out of it (num_bytes >nframes)
    size_t num_bytes = jack_ringbuffer_read_space (thread_stuff->ringbuffer_audio);
    if(num_bytes >= (nframes* sizeof(float))) {
      jack_ringbuffer_read(thread_stuff->ringbuffer_audio, (char*)output_buffer, nframes * sizeof(float));
    } else {
      for (int i = 0; i < (int) nframes; i++)
      {
        output_buffer[i] = 0.0;
      }
    }
  }
  return 0;
}

int main(void) {
  ThreadStuff thread_stuff = {
    .index = 0,
    .data = {0},
    .ringbuffer_audio = jack_ringbuffer_create(4096* sizeof(float)),
    .ringbuffer_video = jack_ringbuffer_create(4096* sizeof(float)),
    .running = true,
    .osc = {0},
    .client = NULL,
    .output_port = NULL
  };
  thread_stuff.client = jack_client_open ("consumer_producer_sine_wave_jack2",
                                        JackNullOption,
                                        0,
                                        0 );

  thread_stuff.output_port = jack_port_register (thread_stuff.client,
                                                  "output",
                                                  JACK_DEFAULT_AUDIO_TYPE,
                                                  JackPortIsOutput,
                                                  0 );
  jack_set_process_callback(thread_stuff.client, jack_process, (void*)&thread_stuff);


  change_frequency(&thread_stuff.osc, 100);
  pthread_t gen_data_thread;
  pthread_create(&gen_data_thread, NULL, gen_data_thread_fct, (void*) &thread_stuff );

  pthread_t print_data_thread;
  pthread_create(&print_data_thread, NULL, print_data_thread_fct, (void*) &thread_stuff );

  jack_activate(thread_stuff.client);

  usleep(20000000); // 20 sekunden warten
  thread_stuff.running = false;

  pthread_join(gen_data_thread, NULL);
  pthread_join(print_data_thread, NULL);

  jack_deactivate(thread_stuff.client);
  jack_client_close(thread_stuff.client);
  jack_ringbuffer_free(thread_stuff.ringbuffer_audio);
  jack_ringbuffer_free(thread_stuff.ringbuffer_video);

  return 0;
}
