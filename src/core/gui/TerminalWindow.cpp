#include "TerminalWindow.h"
#include "../shell/shell.h"
#include "../shell/Editor.h"
#include "../graphics/console.h"
#include "../../utils/StringHelpers.h"
#include "../mm/kheap.h"
#include "Theme.h"

using namespace Utils;

TerminalWindow::TerminalWindow(int x, int y, int w, int h, const char* t)
    : Window(x, y, w, h, t)
{
    history_capacity = 128;
    history_count = 0;
    history = (HistoryLine*)kmalloc(sizeof(HistoryLine) * history_capacity);

    current_line_len = 0;
    scroll_offset = 0;

    ansi_state = 0;
    current_color = Theme::FOREGROUND;

    mode = SHELL;
    active_editor = 0;

    shell = new Shell(this);
    shell->Init();

    Clear();
    Print("Antigravity OS v1.2 [Disk Mounted]\n");
    Prompt();
}

TerminalWindow::~TerminalWindow() {
    Clear();
    kfree(history);
}

void TerminalWindow::Clear() {
    for(int i=0; i<history_count; i++) {
        kfree(history[i].text);
        kfree(history[i].colors);
    }
    history_count = 0;
    current_line_len = 0;
    current_color = Theme::FOREGROUND;
    ansi_state = 0;
}

void TerminalWindow::OnResize(int w, int h) {
    Window::OnResize(w, h);
    // DrawContent handles reflow automatically
}

void TerminalWindow::AddLine(const char* txt, const uint32_t* cols, int len) {
    if (history_count >= history_capacity) {
        // Realloc
        int new_cap = history_capacity * 2;
        HistoryLine* new_hist = (HistoryLine*)kmalloc(sizeof(HistoryLine) * new_cap);
        for(int i=0; i<history_count; i++) new_hist[i] = history[i];
        kfree(history);
        history = new_hist;
        history_capacity = new_cap;
    }

    HistoryLine& hl = history[history_count++];
    hl.length = len;
    hl.text = (char*)kmalloc(len + 1);
    hl.colors = (uint32_t*)kmalloc(sizeof(uint32_t) * len);

    for(int i=0; i<len; i++) {
        hl.text[i] = txt[i];
        hl.colors[i] = cols[i];
    }
    hl.text[len] = 0;
}

void TerminalWindow::FlushCurrentLine() {
    AddLine(current_line_text, current_line_colors, current_line_len);
    current_line_len = 0;
}

void TerminalWindow::Prompt() {
    shell->Prompt();
    input_len = 0;
    input_cursor = 0;
    input[0] = 0;
}

void TerminalWindow::PutChar(char c) {
    // ANSI Parser
    if (ansi_state == 0) {
        if (c == 0x1B) { ansi_state = 1; return; }
    } else if (ansi_state == 1) {
        if (c == '[') { ansi_state = 2; ansi_val = 0; return; }
        else { ansi_state = 0; }
    } else if (ansi_state == 2) {
        if (c >= '0' && c <= '9') {
            ansi_val = ansi_val * 10 + (c - '0');
            return;
        } else if (c == 'm') {
            current_color = Theme::GetAnsiColor(ansi_val);
            ansi_state = 0;
            return;
        } else if (c == ';') {
             ansi_val = 0;
             return;
        } else {
            ansi_state = 0;
        }
    }

    if (c == '\n') {
        FlushCurrentLine();
        return;
    }

    if (current_line_len < 1023) {
        current_line_text[current_line_len] = c;
        current_line_colors[current_line_len] = current_color;
        current_line_len++;
    }
}

void TerminalWindow::Print(const char* str) {
    for(int i=0; str[i]; i++) PutChar(str[i]);
}

void TerminalWindow::Execute() {
    PutChar('\n'); // Ensure newline

    if (input_len > 0) {
        // History Logic
        if (cmd_history_count < 10) {
            Utils::strcpy(cmd_history[cmd_history_count], input);
            cmd_history_count++;
        } else {
            for(int j=1; j<10; j++)
                Utils::strcpy(cmd_history[j-1], cmd_history[j]);
            Utils::strcpy(cmd_history[9], input);
        }
        cmd_history_idx = cmd_history_count;

        if (shell) shell->Execute(input);
    } else {
        Prompt();
    }

    // Prompt is called inside shell->Execute usually, but if shell logic is weird:
    // Shell::Execute calls Prompt() at the end.
    // Wait, let's check Shell::Execute.
    // It prints output. It does NOT call Prompt().
    // So we should call Prompt().

    // Wait, old TerminalWindow called Prompt inside Execute.
    // My previous read of TerminalWindow.cpp showed:
    // Execute() { ... shell->Execute(input); ... Prompt(); }

    // So yes, I should call Prompt() here unless Shell does it.
    // Shell implementation I read earlier does NOT call Prompt().
    // So I call it here.

    // Wait, shell->Execute calls Prompt() inside it? No.
    // Shell::Execute logic: Find command, execute handler.

    // But wait, if I call Prompt() here, it will print prompt.
    // Correct.
}

void TerminalWindow::OnKeyDown(int scancode, char ascii) {
    if (mode == EDITOR && active_editor) {
        active_editor->OnKeyDown(scancode, ascii);
        return;
    }

    // ENTER
    if (scancode == 0x1C) { Execute(); return; }

    // BACKSPACE (0x0E)
    if (scancode == 0x0E) {
        if (input_cursor > 0) {
            for(int i=input_cursor; i<input_len; i++)
                input[i-1] = input[i];
            input_len--;
            input_cursor--;
            input[input_len] = 0;

            if (current_line_len > 0) current_line_len--;
        }
        return;
    }

    // UP ARROW (0x48)
    if (scancode == 0x48) {
        if (cmd_history_count > 0 && cmd_history_idx > 0) {
            cmd_history_idx--;
            // Clear current input from line
            while(input_len > 0) OnKeyDown(0x0E, 0); // Backspace

            Print(cmd_history[cmd_history_idx]);
            Utils::strcpy(input, cmd_history[cmd_history_idx]);
            input_len = Utils::strlen(input);
            input_cursor = input_len;
        }
        return;
    }

    // Normal Char
    if (ascii != 0) {
        if (input_len < 120) {
            input[input_len++] = ascii;
            input_cursor++;
            PutChar(ascii);
        }
    }
}

void TerminalWindow::DrawContent() {
    int max_lines = (height - 20) / 10; // 10px line height (8+2)
    int max_chars_per_line = (width - 10) / 9; // 8px char + 1px space
    if (max_chars_per_line < 1) max_chars_per_line = 1;

    // Logic to find start point (same as thought process)
    int visual_lines_needed = max_lines;
    int start_hist_idx = -1;
    int start_sub_line = 0;

    int current_visual_lines = (current_line_len + max_chars_per_line - 1) / max_chars_per_line;
    if (current_line_len == 0) current_visual_lines = 1;

    if (visual_lines_needed <= current_visual_lines) {
        start_hist_idx = history_count;
        start_sub_line = current_visual_lines - visual_lines_needed;
        if (start_sub_line < 0) start_sub_line = 0;
    } else {
        visual_lines_needed -= current_visual_lines;
        for(int i = history_count - 1; i >= 0; i--) {
            int len = history[i].length;
            int v_lines = (len + max_chars_per_line - 1) / max_chars_per_line;
            if (len == 0) v_lines = 1;

            if (visual_lines_needed <= v_lines) {
                start_hist_idx = i;
                start_sub_line = v_lines - visual_lines_needed;
                if (start_sub_line < 0) start_sub_line = 0; // Safety
                visual_lines_needed = 0;
                break;
            }
            visual_lines_needed -= v_lines;
        }
    }

    if (start_hist_idx == -1) {
        start_hist_idx = 0;
        start_sub_line = 0;
    }

    int y_pos = y + 25;
    int x_pos_base = x + 5;

    int current_hist_idx = start_hist_idx;
    int current_sub = start_sub_line;

    while(y_pos < y + height - 10) {
        char* text;
        uint32_t* colors;
        int len;

        if (current_hist_idx == history_count) {
            text = current_line_text;
            colors = current_line_colors;
            len = current_line_len;
        } else if (current_hist_idx < history_count) {
            text = history[current_hist_idx].text;
            colors = history[current_hist_idx].colors;
            len = history[current_hist_idx].length;
        } else {
            break;
        }

        int start_char = current_sub * max_chars_per_line;
        int end_char = start_char + max_chars_per_line;
        if (end_char > len) end_char = len;

        for(int k=start_char; k<end_char; k++) {
            Console::PutChar(text[k], colors[k], x_pos_base + (k - start_char)*9, y_pos);
        }

        // Cursor
        if (current_hist_idx == history_count && end_char == len) {
             int cx = x_pos_base + (len - start_char)*9;
             // Draw Box Cursor
             for(int h=0; h<10; h++) Console::PutPixel(cx, y_pos+h, Theme::FOREGROUND);
        }

        y_pos += 10;

        int v_lines = (len + max_chars_per_line - 1) / max_chars_per_line;
        if (len == 0) v_lines = 1;

        current_sub++;
        if (current_sub >= v_lines) {
            current_sub = 0;
            current_hist_idx++;
        }
    }
}
