#ifndef WINDOW_H
#define WINDOW_H
#include "../graphics/console.h"

class Window {
public:
    int x, y, width, height;
    char title[32];
    
    // New: State
    bool minimized;
    bool maximized;
    int prev_x, prev_y, prev_w, prev_h;

    // New: Constraints
    int min_width, min_height;

    Window(int x, int y, int w, int h, const char* t) 
        : x(x), y(y), width(w), height(h) 
    {
        // Init Title
        for(int i=0; i<31 && t[i]; i++) title[i] = t[i];
        title[31] = 0;

        minimized = false;
        maximized = false;
        prev_x = x; prev_y = y; prev_w = w; prev_h = h;
        min_width = 100; min_height = 80;
    }

    virtual ~Window() {}

    virtual void DrawContent() {
        // Default: Draw nothing or white background
        for(int j=0; j<height-20; j++)
            for(int i=0; i<width; i++)
                Console::PutPixel(x+i, y+20+j, 0xFFFFFF);
    }

    virtual void OnResize(int w, int h) {
        width = w; height = h;
        if (width < min_width) width = min_width;
        if (height < min_height) height = min_height;
    }

    virtual void OnMinimize() {
        minimized = true;
    }

    virtual void OnMaximize() {
        if (maximized) {
            // Restore
            x = prev_x; y = prev_y; width = prev_w; height = prev_h;
            maximized = false;
        } else {
            // Maximize
            prev_x = x; prev_y = y; prev_w = width; prev_h = height;
            x = 0; y = 0; width = 800; height = 600 - 40; // Desktop size minus taskbar
            maximized = true;
        }
        OnResize(width, height);
    }

    virtual void OnClose() {
        // Handled by Desktop usually to remove from list
    }

    virtual void OnKeyDown(int scancode, char ascii) {
        // Default: Do nothing
    }
};
#endif
