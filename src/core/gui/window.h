#ifndef WINDOW_H
#define WINDOW_H
#include "../graphics/console.h"
#include "../fs/sfs.h"
#include "../fs/ext4.h"
#include "../shell/shell.h"

#define TERM_ROWS 25
#define TERM_COLS 60

#define TERM_W 60
#define TERM_H 25

class Window {
public:
    int x, y, width, height;
    char title[32];
    
    // Terminal Buffer
    char buffer[TERM_H][TERM_W];
    int cursor_row = 0;
    int cursor_col = 0;
    
    // Command Line State
    char input[128];
    int input_len = 0;
    int input_cursor = 0; // Where the user is editing within the line
    
    // History
    char history[10][128];
    int history_count = 0;
    int history_idx = 0;
    
    Shell* shell;

    Window(int x, int y, int w, int h, const char* t) 
        : x(x), y(y), width(w), height(h) 
    {
        // Init Shell
        shell = new Shell(this);
        shell->Init();
        
        // Init Title
        for(int i=0; i<32 && t[i]; i++) title[i] = t[i];
        
        Clear();
        Print("Antigravity OS v1.1 [Disk Mounted]\n");
        Prompt();
        
        Prompt();
    }

    void Clear() {
        for(int r=0; r<TERM_H; r++)
            for(int c=0; c<TERM_W; c++) buffer[r][c] = 0;
        cursor_row = 0;
        cursor_col = 0;
    }

    void Prompt() {
        Print("root@fs:~$ ");
        input_len = 0;
        input_cursor = 0;
        for(int i=0; i<128; i++) input[i] = 0;
    }

    void Scroll() {
        for(int r=1; r<TERM_H; r++)
            for(int c=0; c<TERM_W; c++) buffer[r-1][c] = buffer[r][c];
        for(int c=0; c<TERM_W; c++) buffer[TERM_H-1][c] = 0;
        cursor_row--;
    }

    void PutChar(char c) {
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

    void Print(const char* str) {
        for(int i=0; str[i]; i++) PutChar(str[i]);
    }

    void SerialLog(const char* str) {
        // Simple polling driver for COM1 (0x3F8)
        // We assume COM1 was initialized by GRUB or defaults working for QEMU
        for(int i=0; str[i]; i++) {
             while ((InterruptManager::ReadPort(0x3FD) & 0x20) == 0);
             InterruptManager::WritePort(0x3F8, str[i]);
        }
    }

    // --- Command Execution ---
    void Execute() {
        PutChar('\n');
        
        if (input_len == 0) { Prompt(); return; }

        // 1. Save to History
        if (history_count < 10) {
            for(int i=0; i<128; i++) history[history_count][i] = input[i];
            history_count++;
        } else {
            for(int j=1; j<10; j++)
                for(int i=0; i<128; i++) history[j-1][i] = history[j][i];
            for(int i=0; i<128; i++) history[9][i] = input[i];
        }
        history_idx = history_count;

        // 2. Delegate to Shell
        if (shell) {
            shell->Execute(input);
        } else {
            Print("Error: Shell not initialized.\n");
        }
        
        Prompt();
        
        // Reset Input
        input_len = 0;
        input_cursor = 0;
        input[0] = 0;
    }

    // --- Input Handling ---
    void OnKeyDown(int scancode, char ascii) {
        // ENTER
        if (scancode == 0x1C) { Execute(); return; }
        
        // BACKSPACE (0x0E)
        if (scancode == 0x0E) {
            if (input_cursor > 0) {
                // Shift left in buffer
                for(int i=input_cursor; i<input_len; i++) 
                    input[i-1] = input[i];
                input_len--;
                input_cursor--;
                input[input_len] = 0;
                
                // Visual Backspace (Hacky but works for append-only visual)
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
                // Load History
                // 1. Erase current line visually (dumb way: backspace loop)
                while(input_len > 0) { OnKeyDown(0x0E, 0); } 
                
                // 2. Copy history to input
                for(int i=0; i<128 && history[history_idx][i]; i++) {
                        char c = history[history_idx][i];
                        OnKeyDown(0, c); // Type it out
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

    void DrawContent() {
        // Render Text Buffer
        for(int r=0; r<TERM_H; r++) {
            if (buffer[r][0] != 0 || r == cursor_row)
                Console::PutStringAt(buffer[r], x+5, y+25+(r*10), 0x000000);
        }
        // Blinking Cursor
        int cx = x + 5 + (cursor_col * 8);
        int cy = y + 25 + (cursor_row * 10);
        for(int h=0; h<10; h++) Console::PutPixel(cx, cy+h, 0x000000);
    }
};
#endif
