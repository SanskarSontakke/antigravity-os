#include "desktop.h"
#include "window.h"
#include "TerminalWindow.h"
#include "../graphics/console.h"
#include "../graphics/cursor.h"
#include "../graphics/cursor.h"
#include "../../drivers/rtc.h"
#include "../../drivers/keyboard.h"

// --- State ---
static Window* windows[10];
static int window_count = 0;
static Window* active_window = 0;
static int mouse_x = 400, mouse_y = 300;

// GUI State
static bool show_start_menu = false;
static Window* drag_window = 0;
static int drag_offset_x = 0;
static int drag_offset_y = 0;

// --- Drawing Helpers ---
void DrawRect(int x, int y, int w, int h, uint32_t col) {
    for(int j=0; j<h; j++)
        for(int i=0; i<w; i++)
            Console::PutPixel(x+i, y+j, col);
}

// Draw a "3D" Button
void DrawButton(int x, int y, int w, int h, const char* text, bool pressed) {
    uint32_t base = 0xC0C0C0;
    uint32_t light = 0xFFFFFF;
    uint32_t shadow = 0x404040;
    
    if (pressed) { uint32_t t=light; light=shadow; shadow=t; }

    DrawRect(x, y, w, h, base);
    DrawRect(x, y, w-1, 1, light); // Top
    DrawRect(x, y, 1, h-1, light); // Left
    DrawRect(x, y+h-1, w, 1, shadow); // Bottom
    DrawRect(x+w-1, y, 1, h, shadow); // Right
    
    Console::PutStringAt(text, x + 10, y + 8, 0x000000);
}

// --- Main Logic ---
void Desktop::Init() {
    // 1. Setup Buffering
    Console::InitDoubleBuffer(); 
    
    window_count = 0;
    active_window = 0;
    
    // Create Default Window
    AddWindow(new TerminalWindow(150, 100, 400, 300, "Terminal"));
    Draw();
}

void Desktop::AddWindow(Window* w) {
    if(window_count < 10) windows[window_count++] = w;
    active_window = w; // Auto-focus new windows
}

void Desktop::Draw() {
    // 1. Wallpaper (Teal)
    Console::Clear(0x008080);

    // 2. Windows
    for(int i=0; i<window_count; i++) {
        Window* w = windows[i];
        // Border
        DrawRect(w->x-1, w->y-1, w->width+2, w->height+2, 0xC0C0C0);
        
        // Title Bar (Change color if active)
        uint32_t title_col = (w == active_window) ? 0x000080 : 0x808080;
        DrawRect(w->x, w->y, w->width, 20, title_col);
        
        // Title Text (White) - Simple Hack: Draw 3 chars
        // (Real text rendering needs the previous step's font logic)
        Console::PutChar('X', 0xFFFFFF, w->x + w->width - 15, w->y + 5); // Close Button
        
        // Draw Title
        Console::PutStringAt(w->title, w->x + 5, w->y + 6, 0xFFFFFF);

        // Body (White)
        DrawRect(w->x, w->y+20, w->width, w->height-20, 0xFFFFFF);
        
        // NEW: Draw Text Content
        w->DrawContent();
    }

    // 3. Taskbar (Grey, Bottom)
    int taskbar_y = 600 - 40;
    DrawRect(0, taskbar_y, 800, 40, 0xC0C0C0);
    DrawRect(0, taskbar_y, 800, 2, 0xFFFFFF); // Highlight

    // 4. Start Button
    DrawButton(5, taskbar_y + 5, 80, 30, "Start", show_start_menu);

    // 5. Clock (Right Side)
    // Read RTC
    uint8_t h = RTC::GetHour();
    uint8_t m = RTC::GetMinute();
    // Draw Box
    DrawRect(800-80, taskbar_y+5, 70, 30, 0x000000);
    
    // Convert to String Helper
    char timeStr[6];
    timeStr[0] = '0' + (h / 10);
    timeStr[1] = '0' + (h % 10);
    timeStr[2] = ':';
    timeStr[3] = '0' + (m / 10);
    timeStr[4] = '0' + (m % 10);
    timeStr[5] = 0;
    
    Console::PutStringAt(timeStr, 800-70, taskbar_y+11, 0x00FF00);

    // 6. Start Menu (Popup)
    if (show_start_menu) {
        int menu_y = taskbar_y - 150;
        DrawRect(0, menu_y, 150, 150, 0xC0C0C0);
        DrawRect(2, menu_y+2, 146, 146, 0xFFFFFF); // Inner
        DrawRect(2, menu_y+2, 20, 146, 0x000080); // Side Stripe
        
        Console::PutStringAt("Shutdown", 30, menu_y + 120, 0x000000);
    }

    // 7. Mouse
    Console::DrawCursor(mouse_x, mouse_y);
    
    // 8. Present to Screen
    Console::Swap(); 
}

void Desktop::OnMouseDown(int btn) {
    if (btn == 1) { // Left Click
        // Taskbar Hit Detection
        if (mouse_y > 560) { 
            if (mouse_x < 90) { // Start Button
                show_start_menu = !show_start_menu;
                Draw();
                return;
            }
        }

        // Window Selection Logic
        // Loop Backwards (Top window first)
        for(int i=window_count-1; i>=0; i--) {
            Window* w = windows[i];
            if (mouse_x >= w->x && mouse_x < w->x + w->width &&
                mouse_y >= w->y && mouse_y < w->y + w->height) {
                
                active_window = w; // Focus Clicked Window
                
                // Title Bar Drag Check
                if (mouse_y < w->y + 20) {
                    drag_window = w;
                    drag_offset_x = mouse_x - w->x;
                    drag_offset_y = mouse_y - w->y;
                }
                Draw(); // Redraw to show active color change
                return;
            }
        }
    }
}

void Desktop::OnMouseUp(int btn) {
    drag_window = 0;
}

void Desktop::OnMouseMove(int x, int y) {
    mouse_x = x;
    mouse_y = y;

    if (drag_window) {
        drag_window->x = mouse_x - drag_offset_x;
        drag_window->y = mouse_y - drag_offset_y;
        Draw(); // Redraw only on drag
        return;
    }
    
    // Optimization: Don't redraw whole desktop on every mouse move if not dragging
    // But we need to update cursor cursor.
    // For now, simple Draw() is safest to ensure no artifacts.
    Draw();
}

// Input State
static bool ctrl_pressed = false;

bool Desktop::IsCtrlPressed() {
    return ctrl_pressed;
}

void Desktop::OnKeyUp(int scancode) {
    if (scancode == 0x1D) ctrl_pressed = false; // Left Ctrl Release
}

void Desktop::OnKeyDown(int scancode) {
    if (scancode == 0x1D) { ctrl_pressed = true; return; } // Left Ctrl Press
    
    if (active_window) {
        char c = Keyboard::ScancodeToAscii(scancode);
        // Pass BOTH scancode (for arrows) and ascii (for text)
        active_window->OnKeyDown(scancode, c);
        Draw();
    }
}
