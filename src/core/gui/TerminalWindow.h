#ifndef TERMINAL_WINDOW_H
#define TERMINAL_WINDOW_H

#include "window.h"

class Shell;
class Editor;

struct HistoryLine {
    char* text;
    uint32_t* colors; // Parallel array for colors
    int length;
};

class TerminalWindow : public Window {
public:
    // Dynamic History
    HistoryLine* history;
    int history_count;
    int history_capacity;

    // Current Input Line (Accumulator)
    char current_line_text[1024];
    uint32_t current_line_colors[1024];
    int current_line_len;

    // View State
    int scroll_offset; // 0 = at bottom. Positive = scrolled up lines.

    // ANSI State
    int ansi_state;
    int ansi_val;
    uint32_t current_color;

    // Input Buffer (for Shell)
    char input[128];
    int input_len;
    int input_cursor;

    // Shell History (Command History)
    char cmd_history[10][128];
    int cmd_history_count;
    int cmd_history_idx;

    Shell* shell;

    // Mode
    enum Mode { SHELL, EDITOR };
    Mode mode;
    Editor* active_editor;

    TerminalWindow(int x, int y, int w, int h, const char* t);
    virtual ~TerminalWindow();

    void Clear();
    void Prompt();
    void PutChar(char c);
    void Print(const char* str);
    void Execute();

    // Internal
    void AddLine(const char* txt, const uint32_t* cols, int len);
    void FlushCurrentLine();

    virtual void OnKeyDown(int scancode, char ascii) override;
    virtual void DrawContent() override;
    virtual void OnResize(int w, int h) override;
};

#endif
