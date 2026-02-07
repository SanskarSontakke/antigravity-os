#ifndef DESKTOP_H
#define DESKTOP_H
#include "window.h"

class Desktop {
public:
    static void Init();
    static void Draw();
    static void OnMouseMove(int x, int y);
    static void OnMouseDown(int btn);
    static void OnMouseUp(int btn);
    static void OnKeyDown(int scancode);
    static void OnKeyUp(int scancode);
    static void AddWindow(Window* win);
    static bool IsCtrlPressed();
};
#endif
