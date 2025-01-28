#include <stdio.h>
#include <jack/ringbuffer.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "oscillator.h"
#include "ray_out_buffer.h"
#include "jack_stuff.h"
#include "ui_stuff.h"

int main(void) {
  Envelop adsr_envelop = {0};

  JackStuff* jack_stuff = create_jack_stuff("SineWaveWithJack", 192000);
  float data_buf[1024];
  Oscillator osc = {.amp = 1.0, .freq = 440, .phase = 0};

  size_t window_factor = 80;
  size_t screen_width = (16*window_factor);
  size_t screen_height = (9*window_factor);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(screen_width, screen_height, "sine_wave");
  SetTargetFPS(60);

  RayOutBuffer ray_out_buffer = create_ray_out_buffer(10000);

  Layout_Stack ls = {0};
  SliderState slider_vol = {0};
  slider_vol.scroll = 0.0f;

  SliderState slider_freq = {0};
  slider_freq.scroll = 0.0f;

  Text text = {
    .freq = 50.f,
    .vol = 1.0f
  };

  ADSR adsr = {{.scroll=0.05f},{.scroll=0.25f},{.scroll=0.3f},{.scroll=0.2}};

  while(!WindowShouldClose()) {
    size_t num_bytes = jack_ringbuffer_read_space(jack_stuff->ringbuffer_audio);
    if(num_bytes < 48000 * sizeof(float)) {
      text.freq = 50.0 + 1000.0 * slider_freq.scroll;
      change_frequency(&osc, text.freq);
      text.vol = 1.0 * slider_vol.scroll;
      change_amp(&osc, text.vol);
      gen_signal_in_buf(&osc,  data_buf, 1024, &adsr_envelop);
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

      float w = GetRenderWidth();
      float h = GetRenderHeight();

      bool is_play_pressed = false;
      bool is_reset_pressed = false;

      BeginDrawing();
      ClearBackground(BLACK);

      layout_stack_push(&ls, LO_VERT, ui_rect(0, 0, w, h), 3, 0);
      layout_stack_push(&ls, LO_HORZ, layout_stack_slot(&ls), 3, 0);
      start_button_widget(layout_stack_slot(&ls), PINK, &is_play_pressed);
      reset_button_widget(layout_stack_slot(&ls), PINK, &is_reset_pressed);
      text_widget(layout_stack_slot(&ls), &text);
      layout_stack_pop(&ls);
      signal_widget(layout_stack_slot(&ls), &ray_out_buffer, BLUE);
      layout_stack_push(&ls, LO_HORZ, layout_stack_slot(&ls), 3, 0);
      slider_widget(layout_stack_slot(&ls), &slider_vol);
      adsr_widget(layout_stack_slot(&ls), &adsr);
      slider_widget(layout_stack_slot(&ls), &slider_freq);
      layout_stack_pop(&ls);
      layout_stack_pop(&ls);

      EndDrawing();
      assert(ls.count == 0);

      //tone.current_vol =  is_play_pressed ? 1.0 : 0.0;

      if(is_reset_pressed) {
        adsr.attack.scroll = 0.05f;
        adsr.decay.scroll = 0.25f;
        adsr.sustain.scroll = 0.3f;
        adsr.release.scroll = 0.2;
      }

      // program logic - controller part
      // TODO: signal state via ringbuffer
      adsr_envelop.attack  = adsr.attack.scroll;
      adsr_envelop.decay   = adsr.decay.scroll;
      adsr_envelop.sustain = adsr.sustain.scroll;
      adsr_envelop.release = adsr.release.scroll;
      envelop_trigger(&adsr_envelop,is_play_pressed);
    }
  }
  CloseWindow();
  jack_stuff_clear(jack_stuff);
  return 0;
}
