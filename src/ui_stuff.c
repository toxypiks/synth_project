#include "ui_stuff.h"
#include "ray_out_buffer.h"
#include <stdio.h>
#include <assert.h>

Ui_Rect ui_rect(float x, float y, float w, float h) {
  Ui_Rect r = {0};
  r.x = x;
  r.y = y;
  r.w = w;
  r.h = h;
  return r;
}

Ui_Rect layout_slot_loc(Layout *l, const char *file_path, int line)
{
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

void layout_stack_push(Layout_Stack *ls, Layout_Orient orient, Ui_Rect rect, size_t count, float gap)
{
  Layout l = {0};
  l.orient = orient;
  l.rect = rect;
  l.count = count;
  l.gap = gap;
  da_append(ls, l);
}

void widget(Ui_Rect r, Color c) { DrawRectangle(r.x, r.y, r.w, r.h, c); }

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

void start_button_widget(Ui_Rect r, Color c, bool *is_pressed)
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

void reset_button_widget(Ui_Rect r, Color c, bool *is_pressed)
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
  DrawCircleV(button_position, button_radius * 0.95, BLACK);
  const char *text = "Reset";
  float font_size = 4.0f;
  DrawText(text, button_position.x - button_radius*0.7, button_position.y - button_radius*0.2, 12, c);
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

void text_widget(Ui_Rect r, Text *text)
{
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "Volume: %f           Frequency: %f", text->vol, text->freq);
  DrawText(buffer, r.x, r.y, r.h*0.1, WHITE);
}
