#include <stdio.h>
#include <jack/ringbuffer.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "synth_model.h"
#include "thread_stuff.h"

void* model_gen_signal_thread_fct(void* thread_stuff_raw)
{
  ThreadStuff* thread_stuff = (ThreadStuff*)thread_stuff_raw;

  SynthModel* synth_model = create_synth_model();

  while(thread_stuff->is_running) {
    size_t num_bytes = jack_ringbuffer_read_space(thread_stuff->jack_stuff->ringbuffer_audio);
    float data_buf[1024];

    if(num_bytes < 4800 * sizeof(float))
    {
      thread_stuff->adsr_length = 0;
      synth_model_envelope_update(synth_model,
                                  thread_stuff->attack,
                                  thread_stuff->decay,
                                  thread_stuff->sustain,
                                  thread_stuff->release,
                                  thread_stuff->is_play_pressed);

      synth_model_update(synth_model,
                         data_buf,
                         thread_stuff->vol,
                         thread_stuff->freq,
                         &thread_stuff->adsr_height,
                         &thread_stuff->adsr_length);

      jack_ringbuffer_write(thread_stuff->jack_stuff->ringbuffer_audio, (void *)data_buf, 1024*sizeof(float));
      jack_ringbuffer_write(thread_stuff->jack_stuff->ringbuffer_video, (void *)data_buf, 1024*sizeof(float));
    } else {
      usleep(4680);
    }
  }
  synth_model_clear(synth_model);
  printf("model_gen_signal_thread ended, Good bye! \n");
  return NULL;
}
