#include "ui.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../fs/filesystem.h"

static int ui_mode = 0;

void ui_init(void) {
    ui_mode = 0;
}

static void draw_border(int x, int y, int width, int height) {
    // Draw top border
    vga_set_cursor(x, y);
    vga_putchar('+');
    for (int i = 1; i < width - 1; i++) {
        vga_putchar('-');
    }
    vga_putchar('+');
    
    // Draw side borders
    for (int i = 1; i < height - 1; i++) {
        vga_set_cursor(x, y + i);
        vga_putchar('|');
        vga_set_cursor(x + width - 1, y + i);
        vga_putchar('|');
    }
    
    // Draw bottom border
    vga_set_cursor(x, y + height - 1);
    vga_putchar('+');
    for (int i = 1; i < width - 1; i++) {
        vga_putchar('-');
    }
    vga_putchar('+');
}

void ui_draw_window(window_t* window) {
    if (!window->is_visible) return;
    
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    
    // Clear window area
    for (int y = window->y; y < window->y + window->height; y++) {
        for (int x = window->x; x < window->x + window->width; x++) {
            vga_set_cursor(x, y);
            vga_putchar(' ');
        }
    }
    
    // Draw border
    draw_border(window->x, window->y, window->width, window->height);
    
    // Draw title bar
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_set_cursor(window->x + 2, window->y);
    vga_putchar(' ');
    vga_print(window->title);
    vga_putchar(' ');
    
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void ui_draw_button(button_t* button) {
    vga_set_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
    vga_set_cursor(button->x, button->y);
    vga_putchar('[');
    vga_print(button->text);
    vga_putchar(']');
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void ui_launch(void) {
    vga_clear();
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    
    // Draw desktop background
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_set_cursor(x, y);
            vga_putchar(' ');
        }
    }
    
    // Draw title bar
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_set_cursor(0, 0);
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_putchar(' ');
    }
    vga_print_at("MinimalOS Desktop Environment", 2, 0);
    vga_print_at("Press ESC to exit", VGA_WIDTH - 20, 0);
    
    // Draw taskbar
    vga_set_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
    vga_set_cursor(0, VGA_HEIGHT - 1);
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_putchar(' ');
    }
    vga_print_at("Start", 2, VGA_HEIGHT - 1);
    
    // Create desktop icons/buttons
    button_t file_manager_btn = {"File Manager", 5, 3, ui_file_manager};
    button_t text_editor_btn = {"Text Editor", 5, 5, ui_text_editor};
    button_t calculator_btn = {"Calculator", 5, 7, ui_calculator};
    button_t about_btn = {"About", 5, 9, ui_about};
    
    ui_draw_button(&file_manager_btn);
    ui_draw_button(&text_editor_btn);
    ui_draw_button(&calculator_btn);
    ui_draw_button(&about_btn);
    
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    ui_mode = 1;
    
    // UI event loop
    while (ui_mode) {
        char c = keyboard_getchar();
        if (c == 27) { // ESC key
            ui_mode = 0;
            vga_clear();
            break;
        } else if (c == '1') {
            ui_file_manager();
        } else if (c == '2') {
            ui_text_editor();
        } else if (c == '3') {
            ui_calculator();
        } else if (c == '4') {
            ui_about();
        }
    }
}

void ui_file_manager(void) {
    window_t fm_window = {10, 2, 60, 20, "File Manager", 1, 1};
    ui_draw_window(&fm_window);
    
    // Display file list
    vga_set_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE);
    vga_print_at("Directory: /", fm_window.x + 2, fm_window.y + 2);
    vga_print_at("Files:", fm_window.x + 2, fm_window.y + 4);
    vga_print_at("  readme.txt", fm_window.x + 4, fm_window.y + 5);
    vga_print_at("  hello.txt", fm_window.x + 4, fm_window.y + 6);
    vga_print_at("  [DIR] bin", fm_window.x + 4, fm_window.y + 7);
    vga_print_at("  [DIR] etc", fm_window.x + 4, fm_window.y + 8);
    
    vga_print_at("Press any key to close...", fm_window.x + 2, fm_window.y + fm_window.height - 3);
    
    keyboard_getchar();
    ui_launch(); // Redraw desktop
}

void ui_text_editor(void) {
    window_t editor_window = {5, 3, 70, 18, "Text Editor", 1, 1};
    ui_draw_window(&editor_window);
    
    vga_set_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE);
    vga_print_at("MinimalOS Text Editor v1.0", editor_window.x + 2, editor_window.y + 2);
    vga_print_at("", editor_window.x + 2, editor_window.y + 4);
    vga_print_at("This is a simple text editor.", editor_window.x + 2, editor_window.y + 5);
    vga_print_at("Type your text here...", editor_window.x + 2, editor_window.y + 6);
    vga_print_at("", editor_window.x + 2, editor_window.y + 7);
    vga_print_at("Features:", editor_window.x + 2, editor_window.y + 9);
    vga_print_at("- Basic text input", editor_window.x + 4, editor_window.y + 10);
    vga_print_at("- File operations", editor_window.x + 4, editor_window.y + 11);
    vga_print_at("- Syntax highlighting (planned)", editor_window.x + 4, editor_window.y + 12);
    
    vga_print_at("Press any key to close...", editor_window.x + 2, editor_window.y + editor_window.height - 3);
    
    keyboard_getchar();
    ui_launch(); // Redraw desktop
}

void ui_calculator(void) {
    window_t calc_window = {20, 5, 40, 15, "Calculator", 1, 1};
    ui_draw_window(&calc_window);
    
    vga_set_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE);
    vga_print_at("MinimalOS Calculator", calc_window.x + 10, calc_window.y + 2);
    
    // Draw calculator display
    vga_print_at("+------------------+", calc_window.x + 10, calc_window.y + 4);
    vga_print_at("|              0   |", calc_window.x + 10, calc_window.y + 5);
    vga_print_at("+------------------+", calc_window.x + 10, calc_window.y + 6);
    
    // Draw calculator buttons
    vga_print_at("[ 7 ][ 8 ][ 9 ][ / ]", calc_window.x + 8, calc_window.y + 8);
    vga_print_at("[ 4 ][ 5 ][ 6 ][ * ]", calc_window.x + 8, calc_window.y + 9);
    vga_print_at("[ 1 ][ 2 ][ 3 ][ - ]", calc_window.x + 8, calc_window.y + 10);
    vga_print_at("[ 0 ][ . ][ = ][ + ]", calc_window.x + 8, calc_window.y + 11);
    
    vga_print_at("Press any key to close...", calc_window.x + 2, calc_window.y + calc_window.height - 3);
    
    keyboard_getchar();
    ui_launch(); // Redraw desktop
}

void ui_about(void) {
    window_t about_window = {15, 6, 50, 12, "About MinimalOS", 1, 1};
    ui_draw_window(&about_window);
    
    vga_set_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE);
    vga_print_at("MinimalOS v1.0", about_window.x + 18, about_window.y + 2);
    vga_print_at("", about_window.x + 2, about_window.y + 4);
    vga_print_at("A minimalist operating system", about_window.x + 10, about_window.y + 5);
    vga_print_at("Features:", about_window.x + 2, about_window.y + 7);
    vga_print_at("- Custom kernel and bootloader", about_window.x + 4, about_window.y + 8);
    vga_print_at("- Shell with built-in commands", about_window.x + 4, about_window.y + 9);
    vga_print_at("- Basic filesystem", about_window.x + 4, about_window.y + 10);
    vga_print_at("- Network stack", about_window.x + 4, about_window.y + 11);
    vga_print_at("- Graphical user interface", about_window.x + 4, about_window.y + 12);
    
    vga_print_at("Press any key to close...", about_window.x + 2, about_window.y + about_window.height - 3);
    
    keyboard_getchar();
    ui_launch(); // Redraw desktop
}

void ui_handle_input(void) {
    // Handle UI input events
    char c = keyboard_getchar();
    if (c) {
        // Process input based on current UI state
    }
}
