#ifndef EDITOR_H
#define EDITOR_H

#include "../gui/TerminalWindow.h"

class Editor {
public:
    TerminalWindow* window;
    char filename[64];

    // Buffer
    char* content;
    int content_size;
    int cursor_idx;

    Editor(TerminalWindow* win);
    void Start(const char* fname);
    void OnKeyDown(int scancode, char ascii);
    void Render();
};
#endif
