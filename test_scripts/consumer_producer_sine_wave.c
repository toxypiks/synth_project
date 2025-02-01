#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <jack/ringbuffer.h>
#include "jack_stuff.h"
#include "oscillator.h"
#include "envelop.h"

typedef struct ThreadStuff{
  size_t index;
  float data[1024];
  jack_ringbuffer_t* ringbuffer_data;
  bool running;
  Oscillator osc;
} ThreadStuff;

void* print_data_thread_fct(void* thread_stuff_raw) {
  ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;
  float read_data[1024] = {0};
  while(thread_stuff->running) {
    size_t num_bytes = jack_ringbuffer_read_space(thread_stuff->ringbuffer_data);
    if(num_bytes >= 1024* sizeof(float)){
      jack_ringbuffer_read(thread_stuff->ringbuffer_data, (char*)&read_data, 1024 * sizeof(float));
      for (size_t i = 0; i < 1024; ++i) {
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
    // 1. check if space
    size_t num_bytes = jack_ringbuffer_read_space(thread_stuff->ringbuffer_data);
    if(num_bytes > 1024 * sizeof(float)) {
          usleep(46800); // 0.01 sec
    } else {
      Envelop adsr_envelop = {0};
      adsr_envelop.envelop_state = PRESSED_ATTACK;
      gen_signal_in_buf(&thread_stuff->osc, cache_buf, 1024, &adsr_envelop);
      //memcopy from buffer -> ringbuffer
      jack_ringbuffer_write(
        thread_stuff->ringbuffer_data,
        (void *) &cache_buf,
        1024 * sizeof(float)
        );
    }
  }
}

int main(void) {
  ThreadStuff thread_stuff = {
    .index = 0,
    .data = {0},
    .ringbuffer_data = jack_ringbuffer_create(4096* sizeof(float)),
    .running = true,
    .osc = {0}
  };
  change_frequency(&thread_stuff.osc, 1);
  pthread_t gen_data_thread;
  pthread_create(&gen_data_thread, NULL, gen_data_thread_fct, (void*) &thread_stuff );

  pthread_t print_data_thread;
  pthread_create(&print_data_thread, NULL, print_data_thread_fct, (void*) &thread_stuff );

  usleep(4000000); // 2 sekunden warten
  thread_stuff.running = false;

  pthread_join(gen_data_thread, NULL);
  pthread_join(print_data_thread, NULL);

  return 0;
}
