#include "ui_stuff.h"
#include "ray_out_buffer.h"
#include <stdio.h>
#include <assert.h>
#include <rlgl.h>
#include "helpers.h"

UiRect ui_rect(float x, float y, float w, float h) {
  UiRect r = {0};
  r.x = x;
  r.y = y;
  r.w = w;
  r.h = h;
  return r;
}

UiRect layout_slot_loc(Layout *l, const char *file_path, int line)
{
  if (l->i >= l->count) {
    fprintf(stderr, "%s:%d: ERROR: Layout overflow\n", file_path, line);
    exit(1);
  }

  UiRect r = {0};

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

void layout_stack_push(LayoutStack *ls, LayoutOrient orient, UiRect rect, size_t count, float gap)
{
  Layout l = {0};
  l.orient = orient;
  l.rect = rect;
  l.count = count;
  l.gap = gap;
  da_append(ls, l);
}

void layout_stack_delete(LayoutStack *ls){
    ls->capacity = 0;
    ls->count = 0;
    free(ls->items);
}

void widget(UiRect r, Color c) { DrawRectangle(r.x, r.y, r.w, r.h, c); }

void slider_widget(UiRect r, SliderState *slider_state) {
  float rw = r.w;
  float rh = r.h;
  float rx = r.x;
  float ry = r.y;

  float scale = rh * 0.01;
  float pad = rh * 0.05;

  Vector2 size_bar = {rw * 0.8, rh * 0.01};
  Vector2 position_bar = {rx + 0.1 * rw, ry + 0.5 * rh + pad};
  Vector2 knob_position = {rx + 0.1 * rw + (size_bar.x * slider_state->scroll),
                           position_bar.y + size_bar.y * 0.5};
  float knob_radius = rh * 0.035;
  Color knob_color = BLUE;
  Color low_color = {0xFF, 0x00, 0xFF, 0xFF};
  Color high_color = {0x00, 0x00, 0xFF, 0xFF};
  high_color.a = floorf(255.f*slider_state->scroll);

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    Vector2 mouse_position = GetMousePosition();
    if (Vector2Distance(mouse_position, knob_position) <= knob_radius) {
      slider_state->scroll_dragging = true;
      knob_color = ColorBrightness(knob_color, 0.5f);
    }
  }

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    slider_state->scroll_dragging = false;
  }

  if (slider_state->scroll_dragging) {
    float x = GetMousePosition().x;
    if (x < position_bar.x) {
      x = position_bar.x;
    }
    if (x > position_bar.x + size_bar.x) {
      x = position_bar.x + size_bar.x;
    }
    slider_state->scroll = (x - position_bar.x) / size_bar.x;
  }

  DrawRectangleV(position_bar, size_bar, ColorAlphaBlend(low_color, high_color, WHITE));
  DrawCircleV(knob_position, knob_radius, knob_color);
}

void start_button_widget(UiRect r, Color c, bool *is_pressed)
{
  Vector2 button_position = {r.x + r.w/2, r.y + r.h/2};
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

void reset_button_widget(UiRect r, Color c, bool *is_pressed)
{
  Vector2 button_position = {r.x + r.w/2, r.y + r.h/2};
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
  DrawCircleV(button_position, button_radius * 0.95, BLACK);
  const char *text = "Reset";
  float font_size = 4.0f;
  DrawText(text, button_position.x - button_radius*0.7, button_position.y - button_radius*0.2, 12, c);
}

void oct_trans_button_widget(UiRect r, int *octave, bool *left_is_pressed, bool *right_is_pressed)
{
    Color low_color = {0xFF, 0x00, 0xFF, 0xFF};
    Color high_color = {0x00, 0x00, 0xFF, 0xFF};
    high_color.a = floorf(255.f*0.15);
    Color color1 = ColorAlphaBlend(low_color, high_color, WHITE);
    high_color.a = floorf(255.f*0.25);
    Color color2 = ColorAlphaBlend(low_color, high_color, WHITE);
    high_color.a = floorf(255.f*0.35);
    Color color3 = ColorAlphaBlend(low_color, high_color, WHITE);

    Color left[7] = { WHITE };
    left[0] = color1;
    left[1] = color2;
    left[2] = color3;
    left[3] = BLUE;
    left[4] = BLUE;
    left[5] = BLUE;
    left[6] = BLUE;
    Color right[7] = {WHITE};
    right[0] = BLUE;
    right[1] = BLUE;
    right[2] = BLUE;
    right[3] = BLUE;
    right[4] = color3;
    right[5] = color2;
    right[6] = color1;

    Rectangle left_button = {0};
    Rectangle right_button = {0};

    Vector2 left_button_pos = {r.x + r.w/2, r.y + r.h/2};
    Vector2 right_button_pos = {r.x + r.w/2 + 55, r.y + r.h/2};

    Vector2 inner_size = {47, 23};

    left_button.x = r.x + r.w/2;
    left_button.y = r.y + r.h/2;
    left_button.width = 50;
    left_button.height = 25;

    right_button.x = r.x + r.w/2 + 55;
    right_button.y = r.y + r.h/2;
    right_button.width = 50;
    right_button.height = 25;

    Color left_outer_color = PINK;
    Color left_inner_color = BLUE;

    Color right_outer_color = PINK;
    Color right_inner_color = BLUE;

    Vector2 mouse_position = {-1.0, -1.0};

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        mouse_position = GetMousePosition();

        if (CheckCollisionPointRec(mouse_position, right_button)) {
            *right_is_pressed = true;
            *octave = (*octave >= 6) ? 6 : (*octave + 1);;
        }

        if (CheckCollisionPointRec(mouse_position, left_button)) {
            *left_is_pressed = true;
            *octave = (*octave <= 0) ? 0 : (*octave - 1);
        }
    }
    printf("octave: %d\n", *octave);
    right_inner_color = right[*octave];
    right_outer_color = ColorBrightness(right[*octave], -0.5);
    left_inner_color = left[*octave];
    left_outer_color = ColorBrightness(left[*octave], -0.5);

    DrawRectangleRec(left_button, left_outer_color);
    DrawRectangleV(left_button_pos, inner_size, left_inner_color);
    DrawRectangleRec(right_button, right_outer_color);
    DrawRectangleV(right_button_pos, inner_size, right_inner_color);
    const char *text = "oct/trans";
    float font_size = 5.0f;
    DrawText(text, left_button_pos.x + 25, left_button_pos.y - 30, font_size, PINK);
}

void signal_widget(UiRect r, RayOutBuffer *ray_out_buffer, Color c)
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

void adsr_display_widget(UiRect rect, UiADSR *adsr, Color c, float adsr_height, float adsr_width_raw) {
  float adsr_width = MIN(adsr_width_raw, 1.0);
  float x = rect.x;
  float y = rect.y;
  float w = rect.w;
  float h = rect.h;
  float sum = adsr->attack.scroll + adsr->decay.scroll + 0.5 + adsr->release.scroll;
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

  Vector2 progress_p0 = {adsr_width * w + x, 1.0f*h + y};
  Vector2 progress_p1 = {adsr_width * w + x, (1 - adsr_height) * h + y};
  float thick = w * 0.010f;
  float surrounding = 2.5f * thick;
  Rectangle rec = {
    .x = progress_p0.x - surrounding,
    .y = progress_p1.y - surrounding,
    .width = 2.0f * surrounding,
    .height = surrounding + progress_p0.y - progress_p1.y,
  };

  Rectangle tip = {
    .x = progress_p1.x - surrounding,
    .y = progress_p1.y - surrounding,
    .width = 2.0f * surrounding,
    .height = 2.0f * surrounding,
  };

  Texture2D texture = { rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};


  BeginShaderMode(adsr->circ_shader);
  Rectangle source = {0.0f, 0.0f, 1.0f, 1.0f};
  Vector2 origin = { 0.0f, 0.0f };
  DrawTexturePro(texture, source, tip, origin, 0.0, BLUE);
  EndShaderMode();

  BeginShaderMode(adsr->rec_shader);
  DrawTexturePro(texture, source, rec, origin, 0.0, BLUE);
  EndShaderMode();

  DrawLineEx(progress_p0, progress_p1, thick, MAGENTA);
}

void adsr_widget(UiRect rect, UiADSR *adsr, float adsr_height, float adsr_width)
{
  LayoutStack ls = {0};
  layout_stack_push(&ls, LO_VERT, rect, 2, 0);
  adsr_display_widget(layout_stack_slot(&ls), adsr, BLUE, adsr_height, adsr_width);
  layout_stack_push(&ls, LO_HORZ, rect, 4, 0);
  slider_widget(layout_stack_slot(&ls), &(adsr->attack));
  slider_widget(layout_stack_slot(&ls), &(adsr->decay));
  slider_widget(layout_stack_slot(&ls), &(adsr->sustain));
  slider_widget(layout_stack_slot(&ls), &(adsr->release));
  layout_stack_pop(&ls);
  layout_stack_pop(&ls);
  layout_stack_delete(&ls);
}

void octave_widget(UiRect rect,
                   size_t* key_out,
                   bool* pressed_out,
                   size_t key_in,
                   bool pressed_in)
{
    float x = rect.x;
    float y = rect.y;
    float w = rect.w;
    float h = rect.h;
    float hl = 0.001*h;
    /*
 ___________________________
|  | | | |  |  | | | | | |  |
|  | | | |  |  | | | | |1|  |
|  |1| |3|  |  |6| |8| |0|  |
|  |_| |_|  |  |_| |_| |_|  |
|   |   |   |   |   |   |   |
| 0 | 2 | 4 | 5 | 7 | 9 | 11|
|___|___|___|___|___|___|___|
*/
   size_t key_lookup[12];
   key_lookup[0] = 0;
   key_lookup[1] = 2;
   key_lookup[2] = 4;
   key_lookup[3] = 5;
   key_lookup[4] = 7;
   key_lookup[5] = 9;
   key_lookup[6] = 11;
   key_lookup[7] = 1;
   key_lookup[8] = 3;
   key_lookup[9] = 6;
   key_lookup[10] = 8;
   key_lookup[11] = 10;

   size_t key_reverse_lookup[12];
   key_reverse_lookup[0] = 0;
   key_reverse_lookup[2] = 1;
   key_reverse_lookup[4] = 2;
   key_reverse_lookup[5] = 3;
   key_reverse_lookup[7] = 4;
   key_reverse_lookup[9] = 5;
   key_reverse_lookup[11] = 6;
   key_reverse_lookup[1] = 7;
   key_reverse_lookup[3] = 8;
   key_reverse_lookup[6] = 9;
   key_reverse_lookup[8] = 10;
   key_reverse_lookup[10] = 11;

   int collision_key_in = key_reverse_lookup[key_in];

   Rectangle white_key = {0};
   Color white_key_c = BLACK;
   float white_key_w = w/7.0f;

   white_key.y = 0.0f + y;
   white_key.width = white_key_w;
   white_key.height = h + y;

   Rectangle black_key = {0};
   Color black_key_c = BLUE;

   black_key.y = y;
   black_key.width = 0.5*white_key_w;
   black_key.height = 0.6*h;

   size_t key_index = 0;
   size_t collision_key = -1;
   Vector2 mouse_position = {-1.0, -1.0};

   if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
       mouse_position = GetMousePosition();
   }

   // "white" keys
   for(size_t i = 0; i < 7; ++i) {
       white_key.x =  i*white_key_w + x;
       if (CheckCollisionPointRec(mouse_position, white_key)) {
           collision_key = key_index;
       }
       ++key_index;
   }

   // "black" keys
   for(size_t i = 0; i < 6; ++i){
       if(i != 2){
           black_key.x = i*white_key_w + x + 0.75*white_key_w;
           if (CheckCollisionPointRec(mouse_position, black_key)) {
               collision_key = key_index;
           }
           ++key_index;
       }
   }

   key_index = 0;

   // Drawing keys
   for(size_t i = 0; i < 7; ++i) {
       white_key.x =  i*white_key_w + x;
       if(key_index == collision_key_in && pressed_in) {
           DrawRectangleRec(white_key, RED);
       } else {
           DrawRectangleRec(white_key, white_key_c);
       }
       ++key_index;
   }

   for(size_t i = 0; i < 8; ++i) {
       Vector2 p0 = {i*white_key_w + x, 0.0f + y};
       Vector2 p1 = {i*white_key_w + x, 1.0f * h + y };
       DrawLineV(p0, p1, BLUE);
   }

   for(size_t i = 0; i < 6; ++i) {
       if(i != 2) {
           black_key.x = i*white_key_w + x + 0.75*white_key_w;
           if(key_index == collision_key_in && pressed_in) {
               DrawRectangleRec(black_key, GREEN);
           } else {
              DrawRectangleRec(black_key, black_key_c);
           }
           ++key_index;
       }
   }

   Vector2 frame_p0 = {x, y};
   Vector2 frame_p1 = {x + w, y };
   DrawLineV(frame_p0, frame_p1, BLUE);

   Vector2 frame_p2 = {x, y + h - hl};
   Vector2 frame_p3 = {x + w, y + h - hl};
   DrawLineV(frame_p2, frame_p3, BLUE);

   if (collision_key == -1) {
       *key_out = 0;
       *pressed_out = false;
   } else {
       *key_out = key_lookup[collision_key];
       *pressed_out = true;
   }
}

/*void keyboard_widget(UiRect rect) {
    (void) rect;
    LayoutStack ls = {0};
    layout_stack_push(&ls, LO_VERT, rect, 10, 0);
}*/

void text_widget(UiRect r, Text *text)
{
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "Volume: %f", text->vol);
  DrawText(buffer, r.x, r.y, r.h*0.1, WHITE);
  snprintf(buffer, sizeof(buffer), "Frequency: %f", text->freq);
  DrawText(buffer, r.x, r.y + r.h/5, r.h*0.1, WHITE);
}

UiStuff* create_ui_stuff(size_t screen_width, size_t screen_height){
  UiStuff* ui_stuff = (UiStuff*)malloc(sizeof(UiStuff));
  // init part
  ui_stuff->screen = LoadRenderTexture(screen_width, screen_height);

  ui_stuff->slider_vol.scroll_dragging = false;
  ui_stuff->slider_vol.scroll = 0.0f;
  ui_stuff->slider_freq.scroll_dragging = false;
  ui_stuff->slider_freq.scroll = 0.0f;

  ui_stuff->text.freq = 50.f;
  ui_stuff->text.vol = 1.0f;

  ui_stuff->adsr.attack.scroll_dragging = false;
  ui_stuff->adsr.attack.scroll=0.05f;
  ui_stuff->adsr.decay.scroll_dragging = false;
  ui_stuff->adsr.decay.scroll=0.25f;
  ui_stuff->adsr.sustain.scroll_dragging = false;
  ui_stuff->adsr.sustain.scroll=0.5f;
  ui_stuff->adsr.release.scroll_dragging = false;
  ui_stuff->adsr.release.scroll=0.2;

  ui_stuff->adsr.rec_shader = LoadShader(NULL, "../shaders/rectangle.fs");
  ui_stuff->adsr.circ_shader = LoadShader(NULL, "../shaders/circle.fs");
  ui_stuff->adsr.rec_shader_color_param_loc = GetShaderLocation(ui_stuff->adsr.rec_shader, "colorParam");
  ui_stuff->adsr.circ_shader_color_param_loc = GetShaderLocation(ui_stuff->adsr.circ_shader, "colorParam");
  return ui_stuff;
}
void ui_stuff_clear(UiStuff* ui_stuff) {
  free(ui_stuff);
  return;
}
