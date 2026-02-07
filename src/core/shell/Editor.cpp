#include "Editor.h"
#include "../fs/ext4.h"
#include "../mm/kheap.h"
#include "../../utils/StringHelpers.h"
#include "../graphics/console.h"
#include "../gui/desktop.h" // For IsCtrlPressed

Editor::Editor(TerminalWindow* win) {
    this->window = win;
    this->content = 0;
    this->content_size = 0;
    this->cursor_idx = 0;
}

void Editor::Start(const char* fname) {
    Utils::strcpy(this->filename, fname);

    // Allocate Buffer
    int max_size = 1024 * 10;
    this->content = (char*)kmalloc(max_size);
    for(int i=0; i<max_size; i++) this->content[i] = 0;

    // Read File
    Ext4::ReadFile(fname, this->content);

    this->content_size = Utils::strlen(this->content);
    this->cursor_idx = 0;

    // Switch Mode
    window->mode = TerminalWindow::EDITOR;
    window->active_editor = this;

    Render();
}

void Editor::Render() {
    window->Clear();

    int row = 0;
    int col = 0;
    int cx = 0, cy = 0;

    for(int i=0; i<content_size; i++) {
        char c = content[i];
        if (i == cursor_idx) {
            cx = col; cy = row;
        }

        if (c == '\n') {
            row++;
            col = 0;
        } else {
            if (col < TERM_W) {
                window->buffer[row][col] = c;
                window->color_buffer[row][col] = 0xFFFFFF;
                col++;
            } else {
                row++;
                col = 0;
                window->buffer[row][col] = c;
                window->color_buffer[row][col] = 0xFFFFFF;
                col++;
            }
        }
        if (row >= TERM_H - 1) break;
    }

    if (cursor_idx == content_size) {
        cx = col; cy = row;
    }

    window->cursor_row = cy;
    window->cursor_col = cx;

    int status_row = TERM_H - 1;
    const char* status = "[EDIT] Ctrl+S: Save | Ctrl+Q: Quit";
    for(int i=0; status[i] && i < TERM_W; i++) {
        window->buffer[status_row][i] = status[i];
        window->color_buffer[status_row][i] = 0x00FF00;
    }
}

void Editor::OnKeyDown(int scancode, char ascii) {
    // Check Modifiers
    bool ctrl = Desktop::IsCtrlPressed();

    if (ctrl) {
        if (scancode == 0x1F) { // S
            Ext4::WriteFile(filename, content, content_size);
            // Visual feedback: Flash "SAVED"
            // For now just Render
            Render();
            return;
        }
        if (scancode == 0x10) { // Q
            window->mode = TerminalWindow::SHELL;
            window->active_editor = 0;
            window->Clear();
            window->Prompt();
            if (content) kfree(content);
            content = 0;
            return;
        }
    }

    // Navigation
    if (scancode == 0x4B) { // Left
        if (cursor_idx > 0) cursor_idx--;
        Render(); return;
    }
    if (scancode == 0x4D) { // Right
        if (cursor_idx < content_size) cursor_idx++;
        Render(); return;
    }
    if (scancode == 0x48) { // Up
        // Scan back for previous line
        // Simple logic: go back until newline, then count columns?
        // Basic: Move back 1 char
        if (cursor_idx > 0) cursor_idx--;
        Render(); return;
    }
    if (scancode == 0x50) { // Down
        if (cursor_idx < content_size) cursor_idx++;
        Render(); return;
    }

    // Backspace
    if (scancode == 0x0E) {
        if (cursor_idx > 0) {
            for(int i=cursor_idx; i<content_size; i++) {
                content[i-1] = content[i];
            }
            content_size--;
            cursor_idx--;
            content[content_size] = 0;
            Render();
        }
        return;
    }

    // Typing
    if (ascii != 0) {
        if (content_size < 10240 - 1) {
            for(int i=content_size; i>cursor_idx; i--) {
                content[i] = content[i-1];
            }
            content[cursor_idx] = ascii;
            cursor_idx++;
            content_size++;
            content[content_size] = 0;
            Render();
        }
    }
}
