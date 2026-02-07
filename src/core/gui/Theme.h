#ifndef THEME_H
#define THEME_H

#include <stdint.h>

namespace Theme {
    // Dracula Palette
    constexpr uint32_t BACKGROUND    = 0x282a36; // Dark Grey/Blue
    constexpr uint32_t CURRENT_LINE  = 0x44475a; // Light Grey/Purple (Window Title)
    constexpr uint32_t FOREGROUND    = 0xf8f8f2; // Off-white (Text)
    constexpr uint32_t COMMENT       = 0x6272a4; // Grey/Blue (Button Hover, Button Base)

    constexpr uint32_t CYAN          = 0x8be9fd;
    constexpr uint32_t GREEN         = 0x50fa7b;
    constexpr uint32_t ORANGE        = 0xffb86c;
    constexpr uint32_t PINK          = 0xff79c6;
    constexpr uint32_t PURPLE        = 0xbd93f9;
    constexpr uint32_t RED           = 0xff5555;
    constexpr uint32_t YELLOW        = 0xf1fa8c;

    // UI Mapping
    constexpr uint32_t WINDOW_TITLE  = CURRENT_LINE;
    constexpr uint32_t WINDOW_BODY   = 0x000000; // Pitch Black for Terminal
    constexpr uint32_t WINDOW_BORDER = 0x6272a4; // Comment color
    constexpr uint32_t TASKBAR       = 0x191a21; // Darker Grey
    constexpr uint32_t BUTTON_TEXT   = 0xFFFFFF;
    constexpr uint32_t BUTTON_BASE   = COMMENT;

    // Window Controls
    constexpr uint32_t WIN_CLOSE_BTN = RED;
    constexpr uint32_t WIN_MAX_BTN   = GREEN;
    constexpr uint32_t WIN_MIN_BTN   = YELLOW;

    // ANSI Colors (0-7, 8-15)
    static uint32_t GetAnsiColor(int code) {
        switch(code) {
            case 30: return 0x000000; // Black
            case 31: return RED;
            case 32: return GREEN;
            case 33: return YELLOW;
            case 34: return PURPLE; // Mapping Blue to Purple for Dracula vibe
            case 35: return PINK;
            case 36: return CYAN;
            case 37: return FOREGROUND;
            default: return FOREGROUND;
        }
    }
}

#endif
