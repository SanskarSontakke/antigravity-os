#include "TerminalWindow.h"
#include "../shell/shell.h"
#include "../graphics/console.h"

TerminalWindow::TerminalWindow(int x, int y, int w, int h, const char* t)
    : Window(x, y, w, h, t)
{
    // Pass this (TerminalWindow*) to Shell constructor
    shell = new Shell(this);
    shell->Init();

    Clear();
    Print("Antigravity OS v1.1 [Disk Mounted]\n");
    Prompt();
}

void TerminalWindow::Clear() {
    for(int r=0; r<TERM_H; r++)
        for(int c=0; c<TERM_W; c++) buffer[r][c] = 0;
    cursor_row = 0;
    cursor_col = 0;
}

void TerminalWindow::Prompt() {
    Print("root@fs:~$ ");
    input_len = 0;
    input_cursor = 0;
    for(int i=0; i<128; i++) input[i] = 0;
}

void TerminalWindow::Scroll() {
    for(int r=1; r<TERM_H; r++)
        for(int c=0; c<TERM_W; c++) buffer[r-1][c] = buffer[r][c];
    for(int c=0; c<TERM_W; c++) buffer[TERM_H-1][c] = 0;
    cursor_row--;
}

void TerminalWindow::PutChar(char c) {
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
    buffer[cursor_row][cursor_col++] = c;
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
        // Draw non-empty lines or cursor line
        if (buffer[r][0] != 0 || r == cursor_row)
            Console::PutStringAt(buffer[r], x+5, y+25+(r*10), 0x000000);
    }
    // Cursor
    int cx = x + 5 + (cursor_col * 8);
    int cy = y + 25 + (cursor_row * 10);
    for(int h=0; h<10; h++) Console::PutPixel(cx, cy+h, 0x000000);
}
