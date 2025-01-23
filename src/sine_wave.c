#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <jack/ringbuffer.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "oscillator.h"
#include "ray_out_buffer.h"
#include "jack_stuff.h"

#define DA_INIT_CAP 256
#define da_append(da, item)                                                    \
  do {                                                                         \
    if ((da)->count >= (da)->capacity) {                                       \
      (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity * 2; \
      (da)->items =                                                            \
          realloc((da)->items, (da)->capacity * sizeof(*(da)->items));         \
      assert((da)->items != NULL && "Buy more RAM lol");                       \
    }                                                                          \
                                                                               \
    (da)->items[(da)->count++] = (item);                                       \
  } while (0)

typedef struct {
  float x;
  float y;
  float w;
  float h;
} Ui_Rect;

Ui_Rect ui_rect(float x, float y, float w, float h) {
  Ui_Rect r = {0};
  r.x = x;
  r.y = y;
  r.w = w;
  r.h = h;
  return r;
}

typedef enum {
  LO_HORZ,
  LO_VERT,
} Layout_Orient;

typedef struct {
  Layout_Orient orient;
  Ui_Rect rect;
  size_t count;
  size_t i;
  float gap;
} Layout;

Ui_Rect layout_slot_loc(Layout *l, const char *file_path, int line);

typedef struct {
  Layout *items;
  size_t count;
  size_t capacity;
} Layout_Stack;

Ui_Rect layout_slot_loc(Layout *l, const char *file_path, int line) {
  if (l->i >= l->count) {
    fprintf(stderr, "%s:%d: ERROR: Layout overflow\n", file_path, line);
    exit(1);
  }

  Ui_Rect r = {0};

  switch (l->orient) {
  case LO_HORZ:
    r.w = l->rect.w / l->count;
    r.h = l->rect.h;
    r.x = l->rect.x + l->i * r.w;
    r.y = l->rect.y;

    if (l->i == 0) { // First
      r.w -= l->gap / 2;
    } else if (l->i >= l->count - 1) { // Last
      r.x += l->gap / 2;
      r.w -= l->gap / 2;
    } else { // Middle
      r.x += l->gap / 2;
      r.w -= l->gap;
    }

    break;

  case LO_VERT:
    r.w = l->rect.w;
    r.h = l->rect.h / l->count;
    r.x = l->rect.x;
    r.y = l->rect.y + l->i * r.h;

    if (l->i == 0) { // First
      r.h -= l->gap / 2;
    } else if (l->i >= l->count - 1) { // Last
      r.y += l->gap / 2;
      r.h -= l->gap / 2;
    } else { // Middle
      r.y += l->gap / 2;
      r.h -= l->gap;
    }

    break;

  default:
    assert(0 && "Unreachable");
  }

  l->i += 1;

  return r;
}

void layout_stack_push(Layout_Stack *ls, Layout_Orient orient, Ui_Rect rect,
                       size_t count, float gap) {
  Layout l = {0};
  l.orient = orient;
  l.rect = rect;
  l.count = count;
  l.gap = gap;
  da_append(ls, l);
}

#define layout_stack_slot(ls)                                                  \
  (assert((ls)->count > 0),                                                    \
   layout_slot_loc(&(ls)->items[(ls)->count - 1], __FILE__, __LINE__))
#define layout_stack_pop(ls)                                                   \
  do {                                                                         \
    assert((ls)->count > 0);                                                   \
    (ls)->count -= 1;                                                          \
  } while (0)

void widget(Ui_Rect r, Color c) { DrawRectangle(r.x, r.y, r.w, r.h, c); }

typedef struct {
  bool scroll_dragging;
  float scroll;
} SliderState;

void slider_widget(Ui_Rect r, SliderState *slider_state) {
  float rw = r.w;
  float rh = r.h;
  float rx = r.x;
  float ry = r.y;

  float scale = rh * 0.01;
  float pad = rh * 0.05;

  Vector2 size_bar = {rw * 0.8, rh * 0.02};
  Vector2 position_bar = {rx + 0.1 * rw, ry + 0.5 * rh + pad};
  Vector2 knob_position = {rx + 0.1 * rw + (size_bar.x * slider_state->scroll),
                           position_bar.y + size_bar.y * 0.5};
  float knob_radius = rh * 0.05;

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    Vector2 mouse_position = GetMousePosition();
    if (Vector2Distance(mouse_position, knob_position) <= knob_radius) {
      slider_state->scroll_dragging = true;
      printf("YEAH\n");
    }
  }

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    slider_state->scroll_dragging = false;
  }

  if (slider_state->scroll_dragging) {
    float x = GetMousePosition().x;
    if (x < position_bar.x)
      x = position_bar.x;
    if (x > position_bar.x + size_bar.x)
      x = position_bar.x + size_bar.x;
    slider_state->scroll = (x - position_bar.x) / size_bar.x;
  }

  DrawRectangleV(position_bar, size_bar, WHITE);
  DrawCircleV(knob_position, knob_radius, BLUE);
}

typedef struct Tone {
  float current_vol;
} Tone;

void button_widget(Ui_Rect r, Color c, bool *is_pressed)
{
  Vector2 button_position = {r.w/2, r.h/2};
  float button_radius = r.h * 0.1f;
  *is_pressed = false;
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    Vector2 mouse_position = GetMousePosition();
    if (CheckCollisionPointCircle(GetMousePosition(), button_position, button_radius)) {
      c = ColorBrightness(c, 0.75f);
      *is_pressed = true;
    }
  }

  DrawCircleV(button_position, button_radius, c);
  DrawCircleV(button_position, button_radius * 0.85, BLACK);
  Vector2 v1 = {button_position.x + button_radius * 0.5, button_position.y};
  Vector2 v2 = {button_position.x - button_radius * 0.2, button_position.y-button_radius * 0.35};
  Vector2 v3 = {button_position.x - button_radius * 0.2, button_position.y+button_radius * 0.35};
  DrawTriangle(v1, v2, v3, c);
}

void signal_widget(Ui_Rect r, RayOutBuffer *ray_out_buffer, Color c)
{
  for (size_t i = 0; i < ray_out_buffer->global_frames_count; ++i)
  {
    if (i % 100 == 0) {
      float xoffset = ((float)i)/((float)(ray_out_buffer->global_frames_count));
      float yoffset = ray_out_buffer->global_frames[i] * 0.5 + 0.5;

      float circle_x = r.x + xoffset * r.w;
      float circle_y = r.y + yoffset * r.h;
      DrawCircle(circle_x, circle_y, 2, BLUE);
    }
  }
}

typedef struct ADSR {
  SliderState attack;
  SliderState decay;
  SliderState sustain;
  SliderState release;
} ADSR;

void adsr_display_widget(Ui_Rect rect, ADSR *adsr, Color c) {
  float x = rect.x;
  float y = rect.y;
  float w = rect.w;
  float h = rect.h;
  float sum = adsr->attack.scroll + adsr->decay.scroll + 0.5f + adsr->release.scroll;;
  float al  = adsr->attack.scroll / sum;
  float dl  = adsr->decay.scroll/sum;
  float sl  = 0.5 / sum;
  float rl  = adsr->release.scroll / sum;
  float s   = adsr->sustain.scroll;
  Vector2 p0 = {0.0f + x         , 1.0f * h + y};
  Vector2 p1 = {al *w + x        , 0.0f + y };
  Vector2 p2 = {(al+dl)*w + x    , (1.0f - s)*h + y};
  Vector2 p3 = {(al+dl+sl)*w + x , (1.0f - s)*h + y};
  Vector2 p4 = {1.0f*w + x       , 1.0f*h + y};
  DrawLineV(p0, p1, c);
  DrawLineV(p1, p2, c);
  DrawLineV(p2, p3, c);
  DrawLineV(p3, p4, c);
  DrawLineV(p0, p4, WHITE);
}

void adsr_widget(Ui_Rect rect, ADSR *adsr)
{
  Layout_Stack ls = {0};
  layout_stack_push(&ls, LO_VERT, rect, 2, 0);
  adsr_display_widget(layout_stack_slot(&ls), adsr, BLUE);
  layout_stack_push(&ls, LO_HORZ, rect, 4, 0);
  slider_widget(layout_stack_slot(&ls), &(adsr->attack));
  slider_widget(layout_stack_slot(&ls), &(adsr->decay));
  slider_widget(layout_stack_slot(&ls), &(adsr->sustain));
  slider_widget(layout_stack_slot(&ls), &(adsr->release));
  layout_stack_pop(&ls);
  layout_stack_pop(&ls);
}

typedef struct {
  float freq;
  float vol;
} Text;

void text_widget(Ui_Rect r, Text *text)
{
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "Volume: %f           Frequency: %f", text->vol, text->freq);
  DrawText(buffer, r.x, r.y, r.h*0.1, WHITE);
}

int main(void) {
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

  Tone tone = {
    .current_vol = 0.0f,
  };

  ADSR adsr = {{.scroll=0.05f},{.scroll=0.25f},{.scroll=0.3f},{.scroll=0.2}};

  while(!WindowShouldClose()) {
    size_t num_bytes = jack_ringbuffer_read_space(jack_stuff->ringbuffer_audio);
    if(num_bytes < 48000 * sizeof(float)) {
      text.freq = 50.0 + 1000.0 * slider_freq.scroll;
      change_frequency(&osc, text.freq);
      text.vol = 1.0 * slider_vol.scroll;
      change_amp(&osc, text.vol * tone.current_vol);
      gen_signal_in_buf(&osc,  data_buf, 1024);
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
      // float scale = rh*0.01;
      //float pad = rh*0.05;

      bool is_play_pressed = false;

      BeginDrawing();
      ClearBackground(BLACK);

      layout_stack_push(&ls, LO_VERT, ui_rect(0, 0, w, h), 3, 0);
      layout_stack_push(&ls, LO_HORZ, layout_stack_slot(&ls), 2, 0);
      button_widget(layout_stack_slot(&ls), PINK, &is_play_pressed);
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

      tone.current_vol =  is_play_pressed ? 1.0 : 0.0;
    }
  }
  CloseWindow();
  jack_stuff_clear(jack_stuff);
  return 0;
}
