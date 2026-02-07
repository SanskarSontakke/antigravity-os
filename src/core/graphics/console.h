#ifndef CONSOLE_H
#define CONSOLE_H
#include <stdint.h>

class Console {
public:
    static void Init(uint32_t* fb, uint32_t width, uint32_t height);
    
    // NEW: Double Buffering Methods
    static void InitDoubleBuffer();
    static void Swap(); 
    
    static void Clear(uint32_t color);
    static void PutPixel(int x, int y, uint32_t color);
    static void PutChar(char c, uint32_t color, uint32_t x, uint32_t y);
    static void PutStringAt(const char* str, int x, int y, uint32_t color);
    static void Print(const char* str);
    
    // NEW: Mouse Methods
    static void DrawCursor(int x, int y);
};
#endif
