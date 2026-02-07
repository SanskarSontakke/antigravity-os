#ifndef TERMINAL_WINDOW_H
#define TERMINAL_WINDOW_H

#include "window.h"

// Forward Declaration to avoid circular dependency
class Shell;
class Editor;

#define TERM_W 60
#define TERM_H 25

class TerminalWindow : public Window {
public:
    char buffer[TERM_H][TERM_W];
    uint32_t color_buffer[TERM_H][TERM_W]; // Store colors
    int cursor_row = 0;
    int cursor_col = 0;

    // ANSI State
    int ansi_state = 0;
    int ansi_val = 0;
    uint32_t current_color = 0xFFFFFF; // Default White

    // Modes
    enum Mode {
        SHELL,
        EDITOR
    };
    Mode mode;
    Editor* active_editor;

    char input[128];
    int input_len = 0;
    int input_cursor = 0;

    char history[10][128];
    int history_count = 0;
    int history_idx = 0;

    Shell* shell;

    TerminalWindow(int x, int y, int w, int h, const char* t);

    void Clear();
    void Prompt();
    void Scroll();
    void PutChar(char c);
    void Print(const char* str);
    void Execute();

    virtual void OnKeyDown(int scancode, char ascii) override;
    virtual void DrawContent() override;
};

#endif
