#ifndef UI_STUFF_H_
#define UI_STUFF_H_

#include <raylib.h>
#include <raymath.h>
#include <stddef.h>
#include "ray_out_buffer.h"

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

#define layout_stack_slot(ls)                                                  \
  (assert((ls)->count > 0),                                                    \
   layout_slot_loc(&(ls)->items[(ls)->count - 1], __FILE__, __LINE__))
#define layout_stack_pop(ls)                                                   \
  do {                                                                         \
    assert((ls)->count > 0);                                                   \
    (ls)->count -= 1;                                                          \
  } while (0)

typedef struct Ui_Rect {
  float x;
  float y;
  float w;
  float h;
} Ui_Rect;

typedef enum Layout_Orient {
  LO_HORZ,
  LO_VERT,
} Layout_Orient;

typedef struct Layout {
  Layout_Orient orient;
  Ui_Rect rect;
  size_t count;
  size_t i;
  float gap;
} Layout;

typedef struct Layout_Stack {
  Layout *items;
  size_t count;
  size_t capacity;
} Layout_Stack;

typedef struct SliderState {
  bool scroll_dragging;
  float scroll;
} SliderState;

typedef struct Tone {
  float current_vol;
} Tone;

typedef struct ADSR {
  SliderState attack;
  SliderState decay;
  SliderState sustain;
  SliderState release;
} ADSR;

typedef struct Text {
  float freq;
  float vol;
} Text;

Ui_Rect ui_rect(float x, float y, float w, float h);
Ui_Rect layout_slot_loc(Layout *l, const char *file_path, int line);
void layout_stack_push(Layout_Stack *ls, Layout_Orient orient, Ui_Rect rect, size_t count, float gap);

void widget(Ui_Rect r, Color c);
void slider_widget(Ui_Rect r, SliderState *slider_state);
void start_button_widget(Ui_Rect r, Color c, bool *is_pressed);
void reset_button_widget(Ui_Rect r, Color c, bool *is_pressed);
void signal_widget(Ui_Rect r, RayOutBuffer *ray_out_buffer, Color c);
void adsr_display_widget(Ui_Rect rect, ADSR *adsr, Color c, float adsr_height, float adsr_width);
void adsr_widget(Ui_Rect rect, ADSR *adsr, float adsr_height, float adsr_width);
void text_widget(Ui_Rect r, Text *text);



#endif // UI_STUFF_H
