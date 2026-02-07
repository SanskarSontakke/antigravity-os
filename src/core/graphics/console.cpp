#include "console.h"
#include "font.h"
#include "../mm/kheap.h"

#include "console.h"
#include "font.h"

static uint32_t* framebuffer = 0;
static uint32_t* back_buffer = 0; // The hidden RAM buffer
static uint32_t screen_width = 0;
static uint32_t screen_height = 0;
static uint32_t cursor_x = 0;
static uint32_t cursor_y = 0;

#include "cursor.h"

// ... (Mouse State) ...

// --- State for Mouse ---
static uint32_t mouse_backbuffer[12*18]; // Stores pixels BEHIND the mouse
static int prev_mouse_x = -1;
static int prev_mouse_y = -1;

// Helper: Read a pixel (Direct Memory Access)
uint32_t GetPixel(int x, int y) {
    if(x < 0 || x >= (int)screen_width || y < 0 || y >= (int)screen_height) return 0;
    return back_buffer[y * screen_width + x];
}

void Console::DrawCursor(int x, int y) {
    // 1. Restore Previous Background (Erase old mouse)
    if (prev_mouse_x != -1) {
        for(int fy=0; fy<18; fy++) {
            for(int fx=0; fx<12; fx++) {
                 PutPixel(prev_mouse_x + fx, prev_mouse_y + fy, mouse_backbuffer[fy*12 + fx]);
            }
        }
    }

    // 2. Save Current Background (Under new position)
    for(int fy=0; fy<18; fy++) {
        for(int fx=0; fx<12; fx++) {
             mouse_backbuffer[fy*12 + fx] = GetPixel(x + fx, y + fy);
        }
    }

    // 3. Draw New Mouse
    for(int fy=0; fy<18; fy++) {
        for(int fx=0; fx<12; fx++) {
            uint8_t type = mouse_pointer[fy*12 + fx];
            
            if (type == 1) PutPixel(x + fx, y + fy, 0x000000); // Black Border
            else if (type == 2) PutPixel(x + fx, y + fy, 0xFFFFFF); // White Fill
            // type 0 = Transparent (Do nothing)
        }
    }

    // 4. Update Position Tracker
    prev_mouse_x = x;
    prev_mouse_y = y;
}

void Console::Init(uint32_t* fb, uint32_t width, uint32_t height) {
    framebuffer = fb;
    screen_width = width;
    screen_height = height;
    // Default to single buffering until InitDoubleBuffer is called
    back_buffer = framebuffer; 
    Clear(0x000080); // Navy Blue
}

void Console::InitDoubleBuffer() {
    // Allocate 800*600*4 bytes (~1.9MB)
    // Since our Heap is 10MB, this is safe.
    back_buffer = (uint32_t*)kmalloc(screen_width * screen_height * 4);
}

void Console::Swap() {
    if (back_buffer == framebuffer) return;
    
    // Copy RAM -> VRAM
    // Fast loop
    uint32_t size = screen_width * screen_height;
    for(uint32_t i=0; i<size; i++) {
        framebuffer[i] = back_buffer[i];
    }
}

void Console::Clear(uint32_t color) {
    for(uint32_t i=0; i < screen_width * screen_height; i++) {
        back_buffer[i] = color;
    }
    cursor_x = 0;
    cursor_y = 0;
}

void Console::PutPixel(int x, int y, uint32_t color) {
    if(x < 0 || x >= (int)screen_width || y < 0 || y >= (int)screen_height) return;
    back_buffer[y * screen_width + x] = color;
}

// DRAW CHAR (The Magic Function)
void Console::PutChar(char c, uint32_t color, uint32_t x, uint32_t y) {
    // Offset: our array starts at Space (32).
    // If c < 32, ignore or map to 0.
    int index = c - 32;
    if (index < 0 || index >= 96) return; // Out of bounds

    const uint8_t* row = font8x8_basic[index];

    for(int fy = 0; fy < 8; fy++) {
        // Get the byte for this row
        uint8_t bits = row[fy];
        
        for(int fx = 0; fx < 8; fx++) {
            // Check if bit 7-fx is set (Standard Bitmap order)
            // (0x80 >> fx) checks the bit at that position
            bool on = (bits & (0x80 >> fx));
            
            if (on) PutPixel(x + fx, y + fy, color);
            // Optional: Draw background color if !on
        }
    }
}

void Console::PutStringAt(const char* str, int x, int y, uint32_t color) {
    int cx = x;
    int cy = y;
    for(int i=0; str[i]!=0; i++) {
        PutChar(str[i], color, cx, cy);
        cx += 8;
    }
}

void Console::Print(const char* str) {
    // Handle newlines and auto-scroll
    for(int i=0; str[i] != 0; i++) {
        if(str[i] == '\n') {
            cursor_x = 0;
            cursor_y += 10; // 8px char + 2px padding
        } else if (str[i] == '\b') {
            if (cursor_x >= 8) cursor_x -= 8;
            // Draw black box to erase
            for(int fy=0; fy<8; fy++) 
                for(int fx=0; fx<8; fx++) 
                    PutPixel(cursor_x + fx, cursor_y + fy, 0x000080);
        } else {
            PutChar(str[i], 0xFFFFFF, cursor_x, cursor_y);
            cursor_x += 8;
            if(cursor_x >= screen_width) {
                cursor_x = 0;
                cursor_y += 10;
            }
        }
    }
}
