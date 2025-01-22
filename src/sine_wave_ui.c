#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include <assert.h>
#include <stdlib.h>

#define DA_INIT_CAP 256
#define da_append(da, item)                                                          \
    do {                                                                             \
        if ((da)->count >= (da)->capacity) {                                         \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy more RAM lol");                       \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while (0)

typedef struct {
    float x;
    float y;
    float w;
    float h;
} Ui_Rect;

Ui_Rect ui_rect(float x, float y, float w, float h)
{
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

Ui_Rect layout_slot_loc(Layout *l, const char *file_path, int line)
{
    if (l->i >= l->count) {
        fprintf(stderr, "%s:%d: ERROR: Layout overflow\n", file_path, line);
        exit(1);
    }

    Ui_Rect r = {0};

    switch (l->orient) {
    case LO_HORZ:
        r.w = l->rect.w/l->count;
        r.h = l->rect.h;
        r.x = l->rect.x + l->i*r.w;
        r.y = l->rect.y;

        if (l->i == 0) { // First
            r.w -= l->gap/2;
        } else if (l->i >= l->count - 1) { // Last
            r.x += l->gap/2;
            r.w -= l->gap/2;
        } else { // Middle
            r.x += l->gap/2;
            r.w -= l->gap;
        }

        break;

    case LO_VERT:
        r.w = l->rect.w;
        r.h = l->rect.h/l->count;
        r.x = l->rect.x;
        r.y = l->rect.y + l->i*r.h;

        if (l->i == 0) { // First
            r.h -= l->gap/2;
        } else if (l->i >= l->count - 1) { // Last
            r.y += l->gap/2;
            r.h -= l->gap/2;
        } else { // Middle
            r.y += l->gap/2;
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

#define layout_stack_slot(ls) (assert((ls)->count > 0), layout_slot_loc(&(ls)->items[(ls)->count - 1], __FILE__, __LINE__))
#define layout_stack_pop(ls) do { assert((ls)->count > 0); (ls)->count -= 1; } while (0)

void widget(Ui_Rect r, Color c) {
  DrawRectangle(r.x, r.y, r.w, r.h, c);
}

typedef struct {
  bool scroll_dragging;
  Vector2 size_bar;
  Vector2 position_bar;
  float knob_radius;
  Vector2 knob_position;
} Slider;

void slider_widget(Ui_Rect r, float scroll, Slider slider) {
  float rw = r.w;
  float rh = r.h;
  float rx = r.x;
  float ry = r.y;

  float scale = rh*0.01;
  float pad = rh*0.05;

  Vector2 size_bar = { rw*0.8, rh*0.02 };
  Vector2 position_bar = { rx + 0.1*rw, ry + 0.5*rh + pad };
  Vector2 knob_position = {rx + 0.1*rw + (slider.size_bar.x*scroll), position_bar.y + size_bar.y*0.5};
  float knob_radius = rh*0.05;

  slider.size_bar = size_bar;
  slider.position_bar = position_bar;
  slider.knob_radius = knob_radius;
  slider.knob_position = knob_position;

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    Vector2 mouse_position = GetMousePosition();
    if (Vector2Distance(mouse_position, slider.knob_position) <= slider.knob_radius) {
      slider.scroll_dragging = true;
      printf("YEAH\n");
    }
  }

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    slider.scroll_dragging = false;
  }

  if (slider.scroll_dragging) {
    float x = GetMousePosition().x;
    if (x < slider.position_bar.x) x = slider.position_bar.x;
    if (x > slider.position_bar.x + slider.size_bar.x) x = slider.position_bar.x + slider.size_bar.x;
    scroll = (x - slider.position_bar.x)/slider.size_bar.x;
  }

  DrawRectangleV(slider.position_bar, slider.size_bar, WHITE);
  DrawCircleV(slider.knob_position, slider.knob_radius, BLUE);
}

int main (void)
{
  size_t width = 800;
  size_t height = 600;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(width, height, "Sine Wave Ui");
  SetTargetFPS(60);

  Layout_Stack ls = {0};
  Slider slider = {0};
  float scroll = 0.0f;

  while(!WindowShouldClose()) {
    float w = GetRenderWidth();
    float h = GetRenderHeight();

    BeginDrawing();
            ClearBackground(BLACK);
            layout_stack_push(&ls, LO_VERT, ui_rect(0, 0, w, h), 3, 0);
                layout_stack_push(&ls, LO_HORZ, layout_stack_slot(&ls), 2, 0);
                widget(layout_stack_slot(&ls), RED);
                widget(layout_stack_slot(&ls), PURPLE);
                layout_stack_pop(&ls);
                widget(layout_stack_slot(&ls), BLUE);
                layout_stack_push(&ls, LO_HORZ, layout_stack_slot(&ls), 3, 0);
                slider_widget(layout_stack_slot(&ls), scroll, slider);
                widget(layout_stack_slot(&ls), PINK);
                slider_widget(layout_stack_slot(&ls), scroll, slider);
                layout_stack_pop(&ls);
                layout_stack_pop(&ls);
    EndDrawing();

    assert(ls.count == 0);
  }

  CloseWindow();
  return 0;
}
