#ifndef UI_H
#define UI_H

#include "../kernel/kernel.h"

#define UI_WIDTH 80
#define UI_HEIGHT 25
#define MAX_WINDOWS 8

typedef struct {
    int x, y;
    int width, height;
    char title[32];
    int is_active;
    int is_visible;
} window_t;

typedef struct {
    char text[64];
    int x, y;
    void (*callback)(void);
} button_t;

void ui_init(void);
void ui_launch(void);
void ui_draw_window(window_t* window);
void ui_draw_button(button_t* button);
void ui_handle_input(void);
void ui_file_manager(void);
void ui_text_editor(void);
void ui_calculator(void);
void ui_about(void);

#endif
