// =============================================================================
// programs.h - Application Programs for Antigravity OS
// =============================================================================

#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <stdint.h>

// Application state enum
enum AppState {
    APP_DESKTOP = 0,
    APP_SNAKE,
    APP_NOTEPAD
};

// =============================================================================
// Start Menu
// =============================================================================
namespace StartMenu {
    extern bool isOpen;
    
    void Toggle();
    void Draw(int screenHeight);
}

// =============================================================================
// Snake Game
// =============================================================================
namespace Snake {
    void Init(int screenWidth, int screenHeight);
    void Update(uint8_t scancode);
    void Draw();
    int GetScore();
}

// =============================================================================
// Notepad
// =============================================================================
namespace Notepad {
    void Init();
    void Input(uint8_t scancode);
    void Draw(int screenWidth, int screenHeight);
    void Save(bool useFAT);  // Save to disk (F5)
}

// =============================================================================
// Keyboard Helpers
// =============================================================================
namespace Keyboard {
    // Get ASCII character from scancode (0 if not printable)
    char ScancodeToChar(uint8_t scancode, bool shift);
}

#endif // PROGRAMS_H
