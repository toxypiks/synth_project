#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct ThreadStuff{
  size_t index;
  int data[1024];
  bool running;
} ThreadStuff;

void* print_data_thread_fct(void* thread_stuff_raw) {
  ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;
  while(thread_stuff->running) {
    printf("data[%d]: %d \n",thread_stuff->index, thread_stuff->data[thread_stuff->index - 1]);
    usleep(500000);
  }
  printf("print_data finished\n");
}

void* gen_data_thread_fct(void* thread_stuff_raw) {
  ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;
  int counter = 0;
  while(thread_stuff->running){
    counter++;

    thread_stuff->data[thread_stuff->index++] = counter;

    usleep(10000); // 0.01 sec

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
    .running = true,
  };
  pthread_t gen_data_thread;
  pthread_create(&gen_data_thread, NULL, gen_data_thread_fct, (void*) &thread_stuff );

  pthread_t print_data_thread;
  pthread_create(&print_data_thread, NULL, print_data_thread_fct, (void*) &thread_stuff );

  usleep(4000000); // 2 sekunden warten
  thread_stuff.running = false;

  pthread_join(gen_data_thread, NULL);
  pthread_join(print_data_thread, NULL);

  printf("finished data[%d]: %d \n",thread_stuff.index,thread_stuff.data[thread_stuff.index - 1]);
  return 0;
}
