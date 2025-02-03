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
#include "ffmpeg_stuff.h"

int main(void) {
  Envelop adsr_envelop = {0};
  size_t window_factor = 80;
  size_t screen_width = (16*window_factor);
  size_t screen_height = (9*window_factor);

  FfmpegStuff ffmpeg_stuff = {0};
  ffmpeg_stuff.enable = false;
  ffmpeg_stuff.fps = 60;

  printf("init jack stuff\n");
  JackStuff* jack_stuff = create_jack_stuff("SineWaveWithJack", 192000);
  float data_buf[1024];
  Oscillator osc = {.amp = 1.0, .freq = 440, .phase = 0};

  // init raylib stuff
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(screen_width, screen_height, "sine_wave");

  printf("set target fps: %d\n", ffmpeg_stuff.fps);
  SetTargetFPS(ffmpeg_stuff.fps);

  int ret = ffmpeg_start_rendering(&ffmpeg_stuff, screen_width, screen_height);
  if (ret != 0) {
    return -1;
  }
  ffmpeg_stuff.screen = LoadRenderTexture(screen_width, screen_height);

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

  float adsr_height = 0.0f;
  float sum_ads = 0.0f;
  float sum_adsr = 0.0f;
  float adsr_length = 0.0f;
  ADSR adsr = {{.scroll=0.05f},{.scroll=0.25f},{.scroll=0.5f},{.scroll=0.2}};

  while(!WindowShouldClose()) {
    size_t num_bytes = jack_ringbuffer_read_space(jack_stuff->ringbuffer_audio);
    if(num_bytes < 48000 * sizeof(float)) {
      text.freq = 50.0 + 1000.0 * slider_freq.scroll;
      change_frequency(&osc, text.freq);
      text.vol = 1.0 * slider_vol.scroll;
      change_amp(&osc, text.vol);
      gen_signal_in_buf(&osc,  data_buf, 1024, &adsr_envelop);

      // adsr x,y0,y1 values
      adsr_height = adsr_envelop.current_value;
      sum_ads = 48000.0f *(adsr_envelop.attack + adsr_envelop.decay + 0.5);
      sum_adsr = 48000.0f *(adsr_envelop.attack + adsr_envelop.decay + 0.5 + adsr_envelop.release);
      adsr_length = 0;
      if((adsr_envelop.sample_count < sum_ads) && (adsr_envelop.envelop_state ==  PRESSED_ATTACK
                                           || adsr_envelop.envelop_state == PRESSED_DECAY
                                           || adsr_envelop.envelop_state == PRESSED_SUSTAIN)){
        adsr_length = adsr_envelop.sample_count/sum_adsr;
      } else if ((adsr_envelop.sample_count > sum_adsr) && adsr_envelop.envelop_state ==  PRESSED_ATTACK
                                           || adsr_envelop.envelop_state == PRESSED_DECAY
                                           || adsr_envelop.envelop_state == PRESSED_SUSTAIN) {
        adsr_length = (sum_ads/sum_adsr);
      } else if (adsr_envelop.envelop_state == RELEASED) {
        adsr_length =  adsr_envelop.attack + adsr_envelop.decay + 0.5 + (adsr_envelop.sample_count_release / (48000.0f * adsr_envelop.release));

      }

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
    }

    float w = GetRenderWidth();
    float h = GetRenderHeight();

    bool is_play_pressed = false;
    bool is_reset_pressed = false;

    BeginDrawing();
    // TODO check record toggle on
      BeginTextureMode(ffmpeg_stuff.screen);
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
        adsr_widget(layout_stack_slot(&ls), &adsr, adsr_height, adsr_length);
        slider_widget(layout_stack_slot(&ls), &slider_freq);
        layout_stack_pop(&ls);
        layout_stack_pop(&ls);
      EndTextureMode();

      Vector2 pos_rect = {0,0};
      Rectangle flip_rect = {0, 0, screen_width, -1 * (int)screen_height};

      DrawTextureRec(ffmpeg_stuff.screen.texture,
                     flip_rect,
                     pos_rect,
                     WHITE);

    EndDrawing();
    assert(ls.count == 0);

    Image image = LoadImageFromTexture(ffmpeg_stuff.screen.texture);
    ffmpeg_send_frame(&ffmpeg_stuff, image.data, screen_width, screen_height);
    // TODO check if screen_width and screen_height could change <- window resizable

    UnloadImage(image);

    //tone.current_vol =  is_play_pressed ? 1.0 : 0.0;

    if(is_reset_pressed) {
      adsr.attack.scroll = 0.05f;
      adsr.decay.scroll = 0.25f;
      adsr.sustain.scroll = 0.5f;
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
  CloseWindow();
  ffmpeg_end_rendering(&ffmpeg_stuff);
  jack_stuff_clear(jack_stuff);
  return 0;
}
