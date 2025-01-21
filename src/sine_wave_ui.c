#include <stdio.h>
#include <raylib.h>
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

int main (void)
{
  size_t width = 800;
  size_t height = 600;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(width, height, "Sine Wave Ui");
  SetTargetFPS(60);

  Layout_Stack ls = {0};

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
                layout_stack_push(&ls, LO_HORZ, layout_stack_slot(&ls), 2, 0);
                widget(layout_stack_slot(&ls), YELLOW);
                widget(layout_stack_slot(&ls), PINK);
                layout_stack_pop(&ls);
                layout_stack_pop(&ls);
    EndDrawing();

    assert(ls.count == 0);
  }

  CloseWindow();
  return 0;
}
