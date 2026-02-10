#include "desktop.h"
#include "window.h"
#include "TerminalWindow.h"
#include "Theme.h"
#include "../graphics/console.h"
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
static bool resizing = false;

// Input State
static bool ctrl_pressed = false;

// --- Drawing Helpers ---
void DrawRect(int x, int y, int w, int h, uint32_t col) {
    for(int j=0; j<h; j++)
        for(int i=0; i<w; i++)
            Console::PutPixel(x+i, y+j, col);
}

void DrawButton(int x, int y, int w, int h, const char* text, bool pressed) {
    uint32_t base = Theme::BUTTON_BASE;
    uint32_t light = 0xFFFFFF;
    uint32_t shadow = 0x000000;
    
    if (pressed) { base = Theme::COMMENT; }

    DrawRect(x, y, w, h, base);
    // Simple Border
    DrawRect(x, y, w, 1, light);
    DrawRect(x, y, 1, h, light);
    DrawRect(x, y+h-1, w, 1, shadow);
    DrawRect(x+w-1, y, 1, h, shadow);
    
    Console::PutStringAt(text, x + 10, y + 8, Theme::BUTTON_TEXT);
}

void DrawTitleButton(int x, int y, int w, int h, uint32_t color) {
    DrawRect(x, y, w, h, color);
    DrawRect(x, y, w, 1, 0xFFFFFF);
    DrawRect(x, y, 1, h, 0xFFFFFF);
    DrawRect(x+w-1, y, 1, h, 0x000000);
    DrawRect(x, y+h-1, w, 1, 0x000000);
}

static void ShowBootScreen() {
    Console::Clear(0x000000);
    int cx = 150;
    int cy = 250;
    uint32_t col = Theme::GREEN;

    Console::PutStringAt("    _    _   _ _____ ___ ____ ____      _ __     _____ _______   __", cx, cy, col); cy+=10;
    Console::PutStringAt("   / \\  | \\ | |_   _|_ _/ ___|  _ \\    / \\\\ \\   / /_ _|_   _\\ \\ / /", cx, cy, col); cy+=10;
    Console::PutStringAt("  / _ \\ |  \\| | | |  | | |  _| |_) |  / _ \\\\ \\ / / | |  | |  \\ V / ", cx, cy, col); cy+=10;
    Console::PutStringAt(" / ___ \\| |\\  | | |  | | |_| |  _ <  / ___ \\\\ V /  | |  | |   | |  ", cx, cy, col); cy+=10;
    Console::PutStringAt("/_/   \\_\\_| \\_| |_| |___\\____|_| \\_\\/_/   \\_\\_/  |___| |_|   |_|  ", cx, cy, col); cy+=10;

    Console::Swap();

    // Delay
    for(volatile int i=0; i<300000000; i++);
}

// --- Main Logic ---
void Desktop::Init() {
    Console::InitDoubleBuffer(); 
    
    ShowBootScreen();

    window_count = 0;
    active_window = 0;
    
    AddWindow(new TerminalWindow(150, 100, 400, 300, "Terminal"));
    Draw();
}

void Desktop::AddWindow(Window* w) {
    if(window_count < 10) windows[window_count++] = w;
    active_window = w;
}

void Desktop::RemoveWindow(Window* w) {
    int idx = -1;
    for(int i=0; i<window_count; i++) {
        if (windows[i] == w) {
            idx = i;
            break;
        }
    }

    if (idx != -1) {
        for(int j=idx; j<window_count-1; j++) {
            windows[j] = windows[j+1];
        }
        window_count--;
        if (active_window == w) {
             if (window_count > 0) active_window = windows[window_count-1];
             else active_window = 0;
        }
        delete w;
    }
}

void Desktop::Draw() {
    // 1. Wallpaper
    Console::Clear(Theme::BACKGROUND);

    // 2. Windows
    for(int i=0; i<window_count; i++) {
        Window* w = windows[i];
        if (w->minimized) continue;

        // Border
        DrawRect(w->x-1, w->y-1, w->width+2, w->height+2, Theme::WINDOW_BORDER);
        
        // Title Bar
        uint32_t title_col = Theme::WINDOW_TITLE;
        if (w == active_window) title_col += 0x101010;
        DrawRect(w->x, w->y, w->width, 20, title_col);
        
        // Title Text
        Console::PutStringAt(w->title, w->x + 5, w->y + 5, Theme::FOREGROUND);

        // Window Controls
        int btn_w = 15;
        int btn_h = 15;
        int btn_y = w->y + 2;
        int start_x = w->x + w->width - 5 - btn_w;
        
        // Close
        DrawTitleButton(start_x, btn_y, btn_w, btn_h, Theme::WIN_CLOSE_BTN);
        // Maximize
        DrawTitleButton(start_x - 20, btn_y, btn_w, btn_h, Theme::WIN_MAX_BTN);
        // Minimize
        DrawTitleButton(start_x - 40, btn_y, btn_w, btn_h, Theme::WIN_MIN_BTN);

        // Body
        DrawRect(w->x, w->y+20, w->width, w->height-20, Theme::WINDOW_BODY);
        
        // Content
        w->DrawContent();

        // Resize Grip
        if (!w->maximized) {
            int gx = w->x + w->width - 10;
            int gy = w->y + w->height - 10;
            for(int i=0; i<10; i++) {
                 Console::PutPixel(gx+i, gy+i, 0xFFFFFF); // Diagonal line
            }
        }
    }

    // 3. Taskbar
    int taskbar_y = 600 - 40;
    DrawRect(0, taskbar_y, 800, 40, Theme::TASKBAR);
    DrawRect(0, taskbar_y, 800, 1, 0xFFFFFF);

    // 4. Start Button
    DrawButton(5, taskbar_y + 5, 80, 30, "Start", show_start_menu);

    // 5. Taskbar Items
    int tb_x = 100;
    for(int i=0; i<window_count; i++) {
        Window* w = windows[i];
        bool active = (w == active_window && !w->minimized);
        DrawButton(tb_x, taskbar_y + 5, 100, 30, w->title, active);
        tb_x += 105;
    }

    // 6. Clock
    uint8_t h = RTC::GetHour();
    uint8_t m = RTC::GetMinute();
    char timeStr[6];
    timeStr[0] = '0' + (h / 10);
    timeStr[1] = '0' + (h % 10);
    timeStr[2] = ':';
    timeStr[3] = '0' + (m / 10);
    timeStr[4] = '0' + (m % 10);
    timeStr[5] = 0;
    Console::PutStringAt(timeStr, 800-70, taskbar_y+11, Theme::GREEN);

    // 7. Start Menu
    if (show_start_menu) {
        int menu_y = taskbar_y - 150;
        // Background
        DrawRect(0, menu_y, 150, 150, Theme::BACKGROUND);
        
        // Border
        DrawRect(0, menu_y, 150, 1, Theme::WINDOW_BORDER);
        DrawRect(0, menu_y, 1, 150, Theme::WINDOW_BORDER);
        DrawRect(149, menu_y, 1, 150, Theme::WINDOW_BORDER);
        DrawRect(0, menu_y+149, 150, 1, Theme::WINDOW_BORDER);

        // Item 1: Terminal
        int item_y = menu_y + 10;
        uint32_t bg = Theme::BACKGROUND;
        if (mouse_x < 150 && mouse_y >= item_y && mouse_y < item_y + 20) {
            bg = Theme::COMMENT;
        }
        DrawRect(1, item_y, 148, 20, bg);
        Console::PutStringAt("Terminal", 10, item_y + 5, Theme::FOREGROUND);

        // Item 2: About
        item_y += 25;
        bg = Theme::BACKGROUND;
        if (mouse_x < 150 && mouse_y >= item_y && mouse_y < item_y + 20) {
            bg = Theme::COMMENT;
        }
        DrawRect(1, item_y, 148, 20, bg);
        Console::PutStringAt("About", 10, item_y + 5, Theme::FOREGROUND);
    }

    // 8. Mouse
    Console::DrawCursor(mouse_x, mouse_y);
    Console::Swap(); 
}

void Desktop::OnMouseDown(int btn) {
    if (btn == 1) { // Left Click
        if (mouse_y > 560) {
            if (mouse_x < 90) {
                show_start_menu = !show_start_menu;
                Draw();
                return;
            }
            int tb_x = 100;
            for(int i=0; i<window_count; i++) {
                if (mouse_x >= tb_x && mouse_x < tb_x + 100) {
                     Window* w = windows[i];
                     if (w->minimized) {
                         w->minimized = false;
                         active_window = w;
                     } else if (w == active_window) {
                         w->minimized = true;
                     } else {
                         active_window = w;
                         w->minimized = false; // Bring to front if obscured? (No z-order sort yet)
                     }
                     Draw();
                     return;
                }
                tb_x += 105;
            }
            return;
        }

        // Window Hit Test
        // Reverse order to check top-most first
        for(int i=window_count-1; i>=0; i--) {
            Window* w = windows[i];
            if (w->minimized) continue;

            if (mouse_x >= w->x && mouse_x < w->x + w->width &&
                mouse_y >= w->y && mouse_y < w->y + w->height) {
                
                active_window = w;
                
                if (mouse_y < w->y + 20) {
                    // Title Bar
                    int btn_w = 15;
                    int start_x = w->x + w->width - 5 - btn_w;

                    if (mouse_x >= start_x && mouse_x <= start_x + btn_w) {
                        w->OnClose();
                        RemoveWindow(w);
                        Draw(); return;
                    }
                    if (mouse_x >= start_x - 20 && mouse_x <= start_x - 20 + btn_w) {
                        w->OnMaximize();
                        Draw(); return;
                    }
                    if (mouse_x >= start_x - 40 && mouse_x <= start_x - 40 + btn_w) {
                        w->OnMinimize();
                        Draw(); return;
                    }

                    if (!w->maximized) {
                        drag_window = w;
                        drag_offset_x = mouse_x - w->x;
                        drag_offset_y = mouse_y - w->y;
                        resizing = false;
                    }
                } else {
                    // Resize Grip (Bottom Right 15x15)
                    if (!w->maximized &&
                        mouse_x > w->x + w->width - 15 &&
                        mouse_y > w->y + w->height - 15) {
                        drag_window = w;
                        drag_offset_x = mouse_x; // Just store start pos?
                        // No, store current w/h?
                        // Let's just track mouse relative to top-left of window?
                        // easier: new_w = mouse_x - w->x
                        resizing = true;
                    }
                }
                Draw();
                return;
            }
        }

        // Check Start Menu Clicks
        if (show_start_menu) {
             int menu_y = 600 - 40 - 150;
             if (mouse_x < 150 && mouse_y < 600-40 && mouse_y >= menu_y) {
                 // Check Items
                 if (mouse_y < menu_y + 25) { // Terminal
                     AddWindow(new TerminalWindow(150, 100, 400, 300, "Terminal"));
                     show_start_menu = false;
                     Draw();
                     return;
                 }
             } else {
                 show_start_menu = false;
                 Draw();
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
        if (resizing) {
            int new_w = mouse_x - drag_window->x;
            int new_h = mouse_y - drag_window->y;
            drag_window->OnResize(new_w, new_h);
        } else {
            drag_window->x = mouse_x - drag_offset_x;
            drag_window->y = mouse_y - drag_offset_y;
        }
        Draw();
        return;
    }
    
    Draw();
}

bool Desktop::IsCtrlPressed() {
    return ctrl_pressed;
}

void Desktop::OnKeyUp(int scancode) {
    if (scancode == 0x1D) ctrl_pressed = false;
}

void Desktop::OnKeyDown(int scancode) {
    if (scancode == 0x1D) { ctrl_pressed = true; return; }
    
    if (active_window) {
        char c = Keyboard::ScancodeToAscii(scancode);
        active_window->OnKeyDown(scancode, c);
        Draw();
    }
}
