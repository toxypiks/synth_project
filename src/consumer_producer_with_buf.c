#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <jack/ringbuffer.h>
#include "jack_stuff.h"

typedef struct ThreadStuff{
  size_t index;
  float data[1024];
  jack_ringbuffer_t* ringbuffer_data;
  bool running;
} ThreadStuff;

void* print_data_thread_fct(void* thread_stuff_raw) {
  ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;
  while(thread_stuff->running) {
    // printf("data[%d]: %f \n",thread_stuff->index, thread_stuff->data[thread_stuff->index-1]);
    // usleep(500000);

    // check for new data
    size_t num_bytes = jack_ringbuffer_read_space(thread_stuff->ringbuffer_data);
    if(num_bytes > 1* sizeof(float)){
      float read_data;
      jack_ringbuffer_read(thread_stuff->ringbuffer_data, (char*)&read_data, 1 * sizeof(float));
      printf("ringbuffer_data:  %f\n", read_data);
    } else {
      usleep(50000);
    }
  }
  printf("print_data finished\n");
}

void* gen_data_thread_fct(void* thread_stuff_raw) {
  ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;
  float counter = 0.0f;
  while(thread_stuff->running){
    counter += 1.0f;

    // write date in some buf
    thread_stuff->data[thread_stuff->index++] = counter;

    // now jack ringbuffer_data
    // 1. check if space
    size_t num_bytes = jack_ringbuffer_read_space(thread_stuff->ringbuffer_data);
    if(num_bytes < 1024 * sizeof(float)) {
      //memcopy from buffer -> ringbuffer
      jack_ringbuffer_write(
        thread_stuff->ringbuffer_data,
        (void *) &counter,
        1 * sizeof(float)
        );
    }

    usleep(100000); // 0.01 sec

    if(thread_stuff->index >= 1024){
      printf("generated 1024 values\n");
      break;
    }
  }
  printf("gen_signal finished\n");
}

int main(void) {
  ThreadStuff thread_stuff = {
    .index = 0,
    .data = {0},
    .ringbuffer_data = jack_ringbuffer_create(1024),
    .running = true
  };
  pthread_t gen_data_thread;
  pthread_create(&gen_data_thread, NULL, gen_data_thread_fct, (void*) &thread_stuff );

  pthread_t print_data_thread;
  pthread_create(&print_data_thread, NULL, print_data_thread_fct, (void*) &thread_stuff );

  usleep(4000000); // 2 sekunden warten
  thread_stuff.running = false;

  pthread_join(gen_data_thread, NULL);
  pthread_join(print_data_thread, NULL);

  printf("finished data[%d]: %f \n",thread_stuff.index,thread_stuff.data[thread_stuff.index - 1]);
  return 0;
}
