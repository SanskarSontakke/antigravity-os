#ifndef WINDOW_H
#define WINDOW_H
#include "../graphics/console.h"

class Window {
public:
    int x, y, width, height;
    char title[32];
    
    Window(int x, int y, int w, int h, const char* t) 
        : x(x), y(y), width(w), height(h) 
    {
        // Init Title
        for(int i=0; i<31 && t[i]; i++) title[i] = t[i];
        title[31] = 0;
    }

    virtual ~Window() {}

    virtual void DrawContent() {
        // Default: Draw nothing or white background
        for(int j=0; j<height-20; j++)
            for(int i=0; i<width; i++)
                Console::PutPixel(x+i, y+20+j, 0xFFFFFF);
    }

    virtual void OnKeyDown(int scancode, char ascii) {
        // Default: Do nothing
    }
};
#endif
