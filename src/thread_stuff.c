#include "thread_stuff.h"
#include "jack_stuff.h"

ThreadStuff* create_thread_stuff(JackStuff* jack_stuff)
{
  ThreadStuff* thread_stuff = (ThreadStuff*)malloc(sizeof(ThreadStuff));
  thread_stuff->is_running = true;
  thread_stuff->attack = 0.0f;
  thread_stuff->decay = 0.0f;
  thread_stuff->sustain = 0.0f;
  thread_stuff->release = 0.0f;
  thread_stuff->is_play_pressed = false;

  thread_stuff->jack_stuff = jack_stuff;
  thread_stuff->vol = 0.0;
  thread_stuff->freq = 0.0;
  thread_stuff->adsr_height = 0.0f;
  thread_stuff->adsr_length = 0.0f;

  return thread_stuff;
}
