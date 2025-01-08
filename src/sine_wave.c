#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
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

  size_t window_factor = 80;
  size_t screen_width = (16*window_factor);
  size_t screen_height = (9*window_factor);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(screen_width, screen_height, "sine_wave");
  SetTargetFPS(60);

  RayOutBuffer ray_out_buffer = create_ray_out_buffer(10000);

  float freq = 50.0f;
  float scroll_freq = 0.0f;
  bool scroll_dragging_freq = false;

  float vol = 0.0f;
  float scroll_vol = 0.0f;
  bool scroll_dragging_vol = false;

  while(!WindowShouldClose()) {
    size_t num_bytes = jack_ringbuffer_read_space(jack_stuff->ringbuffer_audio);
    if(num_bytes < 96000 * sizeof(float)) {
      freq = 50.0 + 1000.0 * scroll_freq;
      change_frequency(&osc, freq);
      gen_signal_in_buf(&osc,  data_buf, 1024);
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
      {
        int w = GetRenderWidth();
        int h = GetRenderHeight();

        float rw = w;
        float rh = h*2.0/3.0;
        float rx = 0;
        float ry = h/2.0 - rh/2.0;

        float scale = rh*0.01;
        float pad = rh*0.05;

        Vector2 size = { rw*(1.0/6.0), rh*0.02 };
        Vector2 position_freq = { rx + rw*2.0/3.0, ry + rh + pad };
        Vector2 position_vol = { rx + rw*1.0/3.0, ry + rh + pad };
        DrawRectangleV(position_freq, size, WHITE);
        DrawRectangleV(position_vol, size, WHITE);

        float knob_radius = rh*0.03;
        Vector2 knob_position_freq = { rx + rw*2.0/3.0 + (size.x*scroll_freq), position_freq.y + size.y*0.5f };
        Vector2 knob_position_vol = { rx + rw*1.0/3.0 + (size.x*scroll_vol), position_freq.y + size.y*0.5f };
        DrawCircleV(knob_position_freq, knob_radius, BLUE);
        DrawCircleV(knob_position_vol, knob_radius, BLUE);

        if (scroll_dragging_freq) {
          float x = GetMousePosition().x;
          if (x < position_freq.x) x = position_freq.x;
          if (x > position_freq.x + size.x) x = position_freq.x + size.x;
          scroll_freq = (x - position_freq.x)/size.x;
        }
        else if (scroll_dragging_vol) {
          float x = GetMousePosition().x;
          if (x < position_vol.x) x = position_vol.x;
          if (x > position_vol.x + size.x) x = position_vol.x + size.x;
          scroll_vol = (x - position_vol.x)/size.x;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          Vector2 mouse_position = GetMousePosition();
          if (Vector2Distance(mouse_position, knob_position_freq) <= knob_radius) {
            scroll_dragging_freq = true;
          }
          else if (Vector2Distance(mouse_position, knob_position_vol) <= knob_radius) {
            scroll_dragging_vol = true;
          }
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
          scroll_dragging_freq = false;
          scroll_dragging_vol = false;
        }

        for (size_t i = 0; i < ray_out_buffer.global_frames_count; ++i)
        {
          if (i % 100 == 0) {
            DrawCircle(i, screen_height/2 + (int)(ray_out_buffer.global_frames[i] * 100), 5, BLUE);
            //printf("%f", ray_out_buffer.global_frames[i]);
          }
        }
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Frequency: %f", freq);
        DrawText(buffer, 0, 0, h*0.04, WHITE);
      }
      EndDrawing();
    }
  }
  CloseWindow();
  jack_stuff_clear(jack_stuff);
  return 0;
}
