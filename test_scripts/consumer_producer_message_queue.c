#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct ThreadStuff {
  bool is_running;
  int index;
  int data[1024];
} ThreadStuff;

ThreadStuff* create_thread_stuff(){
  ThreadStuff* thread_stuff = (ThreadStuff*)malloc(sizeof(ThreadStuff));
  thread_stuff->is_running = true;
  thread_stuff->index = 0;
  //thread_stuff->data = {0};
  for (size_t i = 0; i < 1024; ++i) {
    thread_stuff->data[i] = 0;
  }
  return thread_stuff;
}

void* print_data_thread_fct(void* thread_stuff_raw) {
  ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;
  while(thread_stuff->is_running) {
    printf("data[%d]: %d \n",thread_stuff->index, thread_stuff->data[thread_stuff->index - 1]);
    usleep(500000);
  }
  printf("print_data finished\n");
}

void* gen_data_thread_fct(void* thread_stuff_raw) {
  ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;
  int counter = 0;
  while(thread_stuff->is_running){
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
  ThreadStuff* thread_stuff = create_thread_stuff();

  pthread_t gen_data_thread;
  pthread_create(&gen_data_thread, NULL, gen_data_thread_fct, (void*)thread_stuff);

  pthread_t print_data_thread;
  pthread_create(&print_data_thread, NULL, print_data_thread_fct, (void*)thread_stuff);

  usleep(4000000); // 4 wait seconds
  thread_stuff->is_running = false;

  pthread_join(gen_data_thread, NULL);
  pthread_join(print_data_thread, NULL);

   printf("finished data[%d]: %d \n",thread_stuff->index,thread_stuff->data[thread_stuff->index - 1]);
  return 0;
}
