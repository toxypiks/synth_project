#ifndef UI_STUFF_H_
#define UI_STUFF_H_

#include <raylib.h>
#include <raymath.h>
#include <stddef.h>
#include "ray_out_buffer.h"

#define DA_INIT_CAP 256
#define da_append(da, item)                                                      \
  do {                                                                           \
    printf("realloc1 called %d %d\n",(da)->capacity, (da)->count);               \
    if ((da)->count >= (da)->capacity) {                                         \
      (da)->capacity = ((da)->capacity == 0) ? DA_INIT_CAP : (da)->capacity * 2; \
      (da)->items =                                                              \
          realloc((da)->items, (da)->capacity * sizeof(*(da)->items));           \
          printf("realloc2 called %d %d\n",(da)->capacity, (da)->count);         \
      assert((da)->items != NULL && "Buy more RAM lol");                         \
    }                                                                            \
                                                                                 \
    (da)->items[(da)->count++] = (item);                                         \
  } while (0)

#define layout_stack_slot(ls)                                                    \
  (assert((ls)->count > 0),                                                      \
   layout_slot_loc(&(ls)->items[(ls)->count - 1], __FILE__, __LINE__))
#define layout_stack_pop(ls)                                                     \
  do {                                                                           \
    assert((ls)->count > 0);                                                     \
    (ls)->count -= 1;                                                            \
  } while (0)

typedef struct UiRect {
  float x;
  float y;
  float w;
  float h;
} UiRect;

typedef enum LayoutOrient {
  LO_HORZ,
  LO_VERT,
} LayoutOrient;

typedef struct Layout {
  LayoutOrient orient;
  UiRect rect;
  size_t count;
  size_t i;
  float gap;
} Layout;

typedef struct LayoutStack {
  Layout *items;
  size_t count;
  size_t capacity;
} LayoutStack;

typedef struct SliderState {
  bool scroll_dragging;
  float scroll;
} SliderState;

typedef struct Tone {
  float current_vol;
} Tone;

typedef struct UiADSR {
  SliderState attack;
  SliderState decay;
  SliderState sustain;
  SliderState release;
  Shader rec_shader;
  Shader circ_shader;
} UiADSR;

typedef struct Text {
  float freq;
  float vol;
} Text;

typedef struct UiStuff {
  RenderTexture2D screen;
  SliderState slider_vol;
  SliderState slider_freq;
  Text text;
  UiADSR adsr;
} UiStuff;

UiStuff* create_ui_stuff(size_t screen_width, size_t screen_height);
void ui_stuff_clear(UiStuff*);

UiRect ui_rect(float x, float y, float w, float h);
UiRect layout_slot_loc(Layout *l, const char *file_path, int line);
void layout_stack_push(LayoutStack *ls, LayoutOrient orient, UiRect rect, size_t count, float gap);
void layout_stack_delete(LayoutStack *ls);
void widget(UiRect r, Color c);
void slider_widget(UiRect r, SliderState *slider_state);
void start_button_widget(UiRect r, Color c, bool *is_pressed);
void reset_button_widget(UiRect r, Color c, bool *is_pressed);
void signal_widget(UiRect r, RayOutBuffer *ray_out_buffer, Color c);
void adsr_display_widget(UiRect rect, UiADSR *adsr, Color c, float adsr_height, float adsr_width);
void adsr_widget(UiRect rect, UiADSR *adsr, float adsr_height, float adsr_width);
void octave_widget(UiRect rect, size_t* key, bool* pressed);
void text_widget(UiRect r, Text *text);



#endif // UI_STUFF_H
