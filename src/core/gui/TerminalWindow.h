#ifndef TERMINAL_WINDOW_H
#define TERMINAL_WINDOW_H

#include "window.h"

// Forward Declaration to avoid circular dependency
class Shell;

#define TERM_W 60
#define TERM_H 25

class TerminalWindow : public Window {
public:
    char buffer[TERM_H][TERM_W];
    int cursor_row = 0;
    int cursor_col = 0;

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
