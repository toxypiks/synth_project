#include <stdio.h>
#include <raylib.h>
#include <math.h>
#include <jack/ringbuffer.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "oscillator.h"
#include "ray_out_buffer.h"
#include "jack_stuff.h"

int main(void) {
  JackStuff* jack_stuff = create_jack_stuff("SineWaveWithJack", 192000);
  float data_buf[1024];
  Oscillator osc = {.freq = 440, .time_step = 0};

  const int screen_width = 1024;
  const int screen_height = 768;
  InitWindow(screen_width, screen_height, "sine_wave");
  SetTargetFPS(60);

  RayOutBuffer ray_out_buffer = create_ray_out_buffer(10000);

  while(!WindowShouldClose()) {
    size_t num_bytes = jack_ringbuffer_read_space(jack_stuff->ringbuffer_audio);
    if(num_bytes < 96000 * sizeof(float)) {
      gen_signal_in_buf(&osc, data_buf, 1024);
      change_time_step(&osc, 1024);
      jack_ringbuffer_write(jack_stuff->ringbuffer_audio, (void *)data_buf, 1024*sizeof(float));
      jack_ringbuffer_write(jack_stuff->ringbuffer_video, (void *)data_buf, 1024*sizeof(float));

      if(jack_stuff->ringbuffer_video){
        float output_buffer[1024];
        size_t num_bytes = jack_ringbuffer_read_space(jack_stuff->ringbuffer_video);
        if(num_bytes >= (1024* sizeof(float))) {

          jack_ringbuffer_read(jack_stuff->ringbuffer_video, (char*)output_buffer, 1024 * sizeof(float));
        } else {
          for ( int i = 0; i < 1024; i++)
          {
            output_buffer[i] = 0.0;
          }
        }
        copy_to_ray_out_buffer(&ray_out_buffer, output_buffer, 1024);
      }
      BeginDrawing();
      ClearBackground(BLACK);
      for (size_t i = 0; i < ray_out_buffer.global_frames_count; ++i)
      {
        if (i % 100 == 0) {
          DrawCircle(i, screen_height/2 + (int)(ray_out_buffer.global_frames[i] * 100), 5, BLUE);
          printf("%d", (int)ray_out_buffer.global_frames[i]);
        }
      }
      EndDrawing();
    }
  }
  CloseWindow();
  jack_stuff_clear(jack_stuff);
  return 0;
}
