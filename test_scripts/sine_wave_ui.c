#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include "ui_stuff.h"

void button_widget_simple(UiRect r, Color c) {
  float rw = r.w;
  float rh = r.h;
  float rx = r.x;
  float ry = r.y;

  Vector2 button_position = {r.w/2, r.h/2};
  float button_radius = rh * 0.1f;

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    Vector2 mouse_position = GetMousePosition();
    if (CheckCollisionPointCircle(GetMousePosition(), button_position, button_radius)) {
      c = ColorBrightness(c, 0.75f);
    }
  }

  DrawCircleV(button_position, button_radius, c);
}

typedef struct {
  float data_buf[1024];
} SignalState;

void gen_signal_in_buf(SignalState *signal_state) {
  for (size_t i = 0; i < 1024; ++i) {
    signal_state->data_buf[i] = sinf(fmod((2*M_PI*440*i/48000.0f), 2.0*M_PI));
  }
}

void signal_widget_simple(UiRect r, SignalState *signal_state, Color c) {
  float rw = r.w;
  float rh = r.h;
  float rx = r.x;
  float ry = r.y;

  for (size_t data = 0; data < 1024; ++data)
    if (data % 10 == 0) {
      DrawCircle(data, rh*1.5 + (int)(signal_state->data_buf[data] *100), 3, BLUE);
    }
}

int main(void) {
  size_t width = 800;
  size_t height = 600;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(width, height, "Sine Wave Ui");
  SetTargetFPS(60);

  LayoutStack ls = {0};
  SliderState slider_vol = {0};
  slider_vol.scroll = 0.0f;

  SliderState slider_freq = {0};
  slider_freq.scroll = 0.0f;

  SignalState signal_state = {0};

  while (!WindowShouldClose()) {
    float w = GetRenderWidth();
    float h = GetRenderHeight();

    gen_signal_in_buf(&signal_state);

    BeginDrawing();
    ClearBackground(BLACK);
    layout_stack_push(&ls, LO_VERT, ui_rect(0, 0, w, h), 3, 0);
    layout_stack_push(&ls, LO_HORZ, layout_stack_slot(&ls), 2, 0);
        button_widget_simple(layout_stack_slot(&ls), PINK);
        widget(layout_stack_slot(&ls), BLACK);
    layout_stack_pop(&ls);
    signal_widget_simple(layout_stack_slot(&ls), &signal_state, BLUE);
    layout_stack_push(&ls, LO_HORZ, layout_stack_slot(&ls), 3, 0);
        slider_widget(layout_stack_slot(&ls), &slider_vol);
        widget(layout_stack_slot(&ls), BLACK);
        slider_widget(layout_stack_slot(&ls), &slider_freq);
    layout_stack_pop(&ls);
    layout_stack_pop(&ls);
    EndDrawing();

    assert(ls.count == 0);
  }

  CloseWindow();
  return 0;
}
