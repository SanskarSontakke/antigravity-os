#include "TerminalWindow.h"
#include "../shell/shell.h"
#include "../shell/Editor.h"
#include "../graphics/console.h"
#include "../../utils/StringHelpers.h"

TerminalWindow::TerminalWindow(int x, int y, int w, int h, const char* t)
    : Window(x, y, w, h, t)
{
    // Pass this (TerminalWindow*) to Shell constructor
    shell = new Shell(this);
    shell->Init();

    mode = SHELL;
    active_editor = 0;

    Clear();
    Print("Antigravity OS v1.1 [Disk Mounted]\n");
    Prompt();
}

void TerminalWindow::Clear() {
    for(int r=0; r<TERM_H; r++) {
        for(int c=0; c<TERM_W; c++) {
             buffer[r][c] = 0;
             color_buffer[r][c] = 0xFFFFFF;
        }
    }
    cursor_row = 0;
    cursor_col = 0;
    current_color = 0xFFFFFF;
    ansi_state = 0;
}

void TerminalWindow::Prompt() {
    Print("root@fs:~$ ");
    input_len = 0;
    input_cursor = 0;
    for(int i=0; i<128; i++) input[i] = 0;
}

void TerminalWindow::Scroll() {
    for(int r=1; r<TERM_H; r++) {
        for(int c=0; c<TERM_W; c++) {
            buffer[r-1][c] = buffer[r][c];
            color_buffer[r-1][c] = color_buffer[r][c];
        }
    }
    for(int c=0; c<TERM_W; c++) {
        buffer[TERM_H-1][c] = 0;
        color_buffer[TERM_H-1][c] = 0xFFFFFF;
    }
    cursor_row--;
}

void TerminalWindow::PutChar(char c) {
    // ANSI Parser
    if (ansi_state == 0) {
        if (c == 0x1B) { ansi_state = 1; return; }
    } else if (ansi_state == 1) {
        if (c == '[') { ansi_state = 2; ansi_val = 0; return; }
        else { ansi_state = 0; } // Invalid
    } else if (ansi_state == 2) {
        if (c >= '0' && c <= '9') {
            ansi_val = ansi_val * 10 + (c - '0');
            return;
        } else if (c == 'm') {
            // Apply Color
            if (ansi_val == 0) current_color = 0xFFFFFF; // Reset
            else if (ansi_val == 30) current_color = 0x000000;
            else if (ansi_val == 31) current_color = 0xFF0000; // Red
            else if (ansi_val == 32) current_color = 0x00FF00; // Green
            else if (ansi_val == 33) current_color = 0xFFFF00; // Yellow
            else if (ansi_val == 34) current_color = 0x0000FF; // Blue
            else if (ansi_val == 35) current_color = 0xFF00FF; // Magenta
            else if (ansi_val == 36) current_color = 0x00FFFF; // Cyan
            else if (ansi_val == 37) current_color = 0xFFFFFF; // White

            ansi_state = 0;
            return;
        } else if (c == ';') {
             // Ignore multiple params for now, just reset val
             ansi_val = 0;
             return;
        } else {
            ansi_state = 0; // Abort
        }
    }

    // Normal Char
    if (c == '\n') {
        cursor_row++;
        cursor_col = 0;
        if(cursor_row >= TERM_H) Scroll();
        return;
    }
    if (cursor_col >= TERM_W) {
        cursor_row++;
        cursor_col = 0;
        if(cursor_row >= TERM_H) Scroll();
    }
    buffer[cursor_row][cursor_col] = c;
    color_buffer[cursor_row][cursor_col] = current_color;
    cursor_col++;
}

void TerminalWindow::Print(const char* str) {
    for(int i=0; str[i]; i++) PutChar(str[i]);
}

void TerminalWindow::Execute() {
    PutChar('\n');

    if (input_len == 0) { Prompt(); return; }

    // History Logic
    if (history_count < 10) {
        for(int i=0; i<128; i++) history[history_count][i] = input[i];
        history_count++;
    } else {
        for(int j=1; j<10; j++)
            for(int i=0; i<128; i++) history[j-1][i] = history[j][i];
        for(int i=0; i<128; i++) history[9][i] = input[i];
    }
    history_idx = history_count;

    if (shell) {
        shell->Execute(input);
    } else {
        Print("Error: Shell not initialized.\n");
    }

    Prompt();

    input_len = 0;
    input_cursor = 0;
    input[0] = 0;
}

void TerminalWindow::OnKeyDown(int scancode, char ascii) {
    if (mode == EDITOR && active_editor) {
        active_editor->OnKeyDown(scancode, ascii);
        return;
    }

    // TAB (0x0F) - Autocomplete
    if (scancode == 0x0F) {
        if (input_len > 0) {
            // Find prefix (last word)
            int start = 0;
            for(int i=input_len-1; i>=0; i--) {
                if (input[i] == ' ') { start = i + 1; break; }
            }
            char prefix[64];
            int p_len = 0;
            for(int i=start; i<input_len && p_len < 63; i++) prefix[p_len++] = input[i];
            prefix[p_len] = 0;

            if (p_len > 0) {
                const char* suggestion = shell->GetAutocompleteSuggestion(prefix);
                if (suggestion) {
                    // Backspace prefix
                    for(int i=0; i<p_len; i++) OnKeyDown(0x0E, 0);
                    // Type suggestion
                    for(int i=0; suggestion[i]; i++) OnKeyDown(0, suggestion[i]);
                }
            }
        }
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

            if (cursor_col > 0) {
                cursor_col--;
                buffer[cursor_row][cursor_col] = 0;
            }
        }
        return;
    }

    // ARROW UP (0x48)
    if (scancode == 0x48) {
        if (history_count > 0) {
            if (history_idx > 0) history_idx--;
            while(input_len > 0) { OnKeyDown(0x0E, 0); }

            for(int i=0; i<128 && history[history_idx][i]; i++) {
                    char c = history[history_idx][i];
                    OnKeyDown(0, c);
            }
        }
        return;
    }

    // ARROW DOWN (0x50)
    if (scancode == 0x50) {
        if (history_idx < history_count) {
            history_idx++;
            while(input_len > 0) { OnKeyDown(0x0E, 0); }
            if (history_idx < history_count) {
                for(int i=0; i<128 && history[history_idx][i]; i++) {
                        OnKeyDown(0, history[history_idx][i]);
                }
            }
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
    for(int r=0; r<TERM_H; r++) {
        int py = y + 25 + (r*10);
        int px = x + 5;

        // Clear Line Background (White)
        // Optimization: Could be done better but ensure no artifacts
        // DrawRect(px, py, TERM_W * 8, 10, 0xFFFFFF); // Provided by Desktop logic partly?
        // No, Desktop draws window white. We just overwrite text.
        // Wait, if we backspace, we need to clear.
        // Easiest: Draw space if 0.

        for(int c=0; c<TERM_W; c++) {
            char ch = buffer[r][c];
            uint32_t col = color_buffer[r][c];
            if (ch == 0) ch = ' ';
            Console::PutChar(ch, col, px + (c*8), py);
        }
    }
    // Cursor
    int cx = x + 5 + (cursor_col * 8);
    int cy = y + 25 + (cursor_row * 10);
    for(int h=0; h<10; h++) Console::PutPixel(cx, cy+h, 0x000000);
}
