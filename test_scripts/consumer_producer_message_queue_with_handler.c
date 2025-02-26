#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "msg_handler.h"

typedef struct ThreadStuff {
  bool is_running;
  int index;
  lf_queue_bss_state msg_queue;
  lf_queue_bss_element* element_array;
} ThreadStuff;

ThreadStuff* create_thread_stuff(){
  ThreadStuff* thread_stuff = (ThreadStuff*)malloc(sizeof(ThreadStuff));
  thread_stuff->is_running = true;
  thread_stuff->index = 0;
  memset(&thread_stuff->msg_queue, 0, sizeof(lf_queue_bss_state));
  thread_stuff->element_array = malloc(16*sizeof(lf_queue_bss_element));
  memset(thread_stuff->element_array, 0, 10*sizeof(lf_queue_bss_element));
  lf_queue_init(&thread_stuff->msg_queue, thread_stuff->element_array, 16);

  return thread_stuff;
}

// TODO clean threadstuff function
void print_data(void* data, void* not_in_use){
    printf("float value: %f\n", *((float*)data));
}

void* print_data_thread_fct(void* thread_stuff_raw) {
    ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;
    int counter = 0;
    MsgHdl msg_hdl = {0};
    char* key = "float_value";

    msg_hdl_add_key2fct(&msg_hdl, key, print_data, NULL);

    while(thread_stuff->is_running) {
        msg_hdling(&msg_hdl, &thread_stuff->msg_queue);
        usleep(10000);
    }
    printf("print_data finished\n");
}

void* gen_data_thread_fct(void* thread_stuff_raw) {
    ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;
    int counter = 0;
    while(thread_stuff->is_running){

        float value = (float)counter * 10.0;

        int ret = lf_queue_push(&thread_stuff->msg_queue, "float_value", (void*)&value, sizeof(float));
        // lf_queue_push(... , counter);
        counter++;
        if(ret < 1){
            printf("msg_queue is full \n");
        }
        if(counter >= 1024){
            printf("generated 1024 values\n");
            break;
        }
        usleep(12000);
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

   printf("finished \n");
  return 0;
}
