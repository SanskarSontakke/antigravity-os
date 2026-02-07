// =============================================================================
// programs.cpp - Application Programs Implementation
// =============================================================================

#include "programs.h"
#include "graphics.h"
#include "serial.h"
#include "ext2.h"

// Colors
#define COLOR_BLACK     0x00000000
#define COLOR_WHITE     0x00FFFFFF
#define COLOR_GREEN     0x0000FF00
#define COLOR_RED       0x00FF0000
#define COLOR_GRAY      0x00C0C0C0
#define COLOR_DARK_GRAY 0x00606060
#define COLOR_DARK_BLUE 0x00000080
#define COLOR_TEAL      0x00008080

// Scancodes
#define SC_W        0x11
#define SC_A        0x1E
#define SC_S        0x1F
#define SC_D        0x20
#define SC_BACKSPACE 0x0E
#define SC_ENTER    0x1C
#define SC_SPACE    0x39

// =============================================================================
// Keyboard Scancode to ASCII Table
// =============================================================================
static const char SCANCODE_TO_ASCII[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' '
};

namespace Keyboard {
char ScancodeToChar(uint8_t scancode, bool shift) {
    if (scancode >= sizeof(SCANCODE_TO_ASCII)) return 0;
    char c = SCANCODE_TO_ASCII[scancode];
    if (shift && c >= 'a' && c <= 'z') c -= 32;  // Uppercase
    return c;
}
}

// =============================================================================
// Start Menu
// =============================================================================
namespace StartMenu {
bool isOpen = false;

void Toggle() {
    isOpen = !isOpen;
}

void Draw(int screenHeight) {
    if (!isOpen) return;
    
    int menuX = 4;
    int menuY = screenHeight - 36 - 100;
    int menuW = 150;
    int menuH = 100;
    
    // Menu background
    Graphics::DrawRect(menuX, menuY, menuW, menuH, COLOR_GRAY);
    Graphics::DrawRect(menuX, menuY, menuW, 2, COLOR_WHITE);
    Graphics::DrawRect(menuX, menuY, 2, menuH, COLOR_WHITE);
    Graphics::DrawRect(menuX + menuW - 2, menuY, 2, menuH, COLOR_DARK_GRAY);
    Graphics::DrawRect(menuX, menuY + menuH - 2, menuW, 2, COLOR_DARK_GRAY);
    
    // Menu items
    Graphics::DrawString(menuX + 10, menuY + 10, "F1: Snake", COLOR_BLACK);
    Graphics::DrawString(menuX + 10, menuY + 30, "F2: Notepad", COLOR_BLACK);
    Graphics::DrawString(menuX + 10, menuY + 50, "F3: Files", COLOR_DARK_GRAY);
    Graphics::DrawRect(menuX + 5, menuY + 70, menuW - 10, 1, COLOR_DARK_GRAY);
    Graphics::DrawString(menuX + 10, menuY + 78, "Esc: Close", COLOR_BLACK);
}
}

// =============================================================================
// Snake Game
// =============================================================================
namespace Snake {

static const int GRID_SIZE = 16;
static const int MAX_LENGTH = 100;

static int screenW, screenH;
static int gridW, gridH;
static int snakeX[MAX_LENGTH];
static int snakeY[MAX_LENGTH];
static int snakeLength;
static int direction;  // 0=up, 1=right, 2=down, 3=left
static int appleX, appleY;
static int score;
static bool gameOver;
static int frameCounter;

// Simple pseudo-random
static uint32_t randSeed = 12345;
static int Random(int max) {
    randSeed = randSeed * 1103515245 + 12345;
    return (randSeed / 65536) % max;
}

static void PlaceApple() {
    appleX = Random(gridW - 2) + 1;
    appleY = Random(gridH - 2) + 1;
}

void Init(int screenWidth, int screenHeight) {
    screenW = screenWidth;
    screenH = screenHeight;
    gridW = screenWidth / GRID_SIZE;
    gridH = screenHeight / GRID_SIZE;
    
    // Reset snake to center
    snakeLength = 3;
    int centerX = gridW / 2;
    int centerY = gridH / 2;
    for (int i = 0; i < snakeLength; i++) {
        snakeX[i] = centerX;
        snakeY[i] = centerY + i;
    }
    
    direction = 0;  // Up
    score = 0;
    gameOver = false;
    frameCounter = 0;
    
    PlaceApple();
    Serial::Log("Snake game started!\n");
}

void Update(uint8_t scancode) {
    if (gameOver) {
        // Any key restarts
        Init(screenW, screenH);
        return;
    }
    
    // Change direction
    switch (scancode) {
        case SC_W: if (direction != 2) direction = 0; break;
        case SC_D: if (direction != 3) direction = 1; break;
        case SC_S: if (direction != 0) direction = 2; break;
        case SC_A: if (direction != 1) direction = 3; break;
    }
    
    // Move snake (every few frames for slower movement)
    frameCounter++;
    if (frameCounter < 5) return;
    frameCounter = 0;
    
    // Calculate new head position
    int newX = snakeX[0];
    int newY = snakeY[0];
    switch (direction) {
        case 0: newY--; break;
        case 1: newX++; break;
        case 2: newY++; break;
        case 3: newX--; break;
    }
    
    // Wall collision
    if (newX < 0 || newX >= gridW || newY < 0 || newY >= gridH) {
        gameOver = true;
        return;
    }
    
    // Self collision
    for (int i = 0; i < snakeLength; i++) {
        if (snakeX[i] == newX && snakeY[i] == newY) {
            gameOver = true;
            return;
        }
    }
    
    // Apple collision
    bool ate = (newX == appleX && newY == appleY);
    if (ate) {
        score += 10;
        if (snakeLength < MAX_LENGTH - 1) snakeLength++;
        PlaceApple();
    }
    
    // Move body
    for (int i = snakeLength - 1; i > 0; i--) {
        snakeX[i] = snakeX[i-1];
        snakeY[i] = snakeY[i-1];
    }
    snakeX[0] = newX;
    snakeY[0] = newY;
}

void Draw() {
    // Black background
    Graphics::Clear(COLOR_BLACK);
    
    // Border
    Graphics::DrawRect(0, 0, screenW, 2, COLOR_GRAY);
    Graphics::DrawRect(0, screenH - 2, screenW, 2, COLOR_GRAY);
    Graphics::DrawRect(0, 0, 2, screenH, COLOR_GRAY);
    Graphics::DrawRect(screenW - 2, 0, 2, screenH, COLOR_GRAY);
    
    // Apple
    Graphics::DrawRect(appleX * GRID_SIZE, appleY * GRID_SIZE, 
                       GRID_SIZE - 2, GRID_SIZE - 2, COLOR_RED);
    
    // Snake
    for (int i = 0; i < snakeLength; i++) {
        uint32_t color = (i == 0) ? 0x0000AA00 : COLOR_GREEN;  // Head darker
        Graphics::DrawRect(snakeX[i] * GRID_SIZE, snakeY[i] * GRID_SIZE,
                           GRID_SIZE - 2, GRID_SIZE - 2, color);
    }
    
    // Score
    char scoreStr[32] = "Score: ";
    int s = score;
    int pos = 7;
    if (s == 0) {
        scoreStr[pos++] = '0';
    } else {
        char temp[10];
        int t = 0;
        while (s > 0) { temp[t++] = '0' + (s % 10); s /= 10; }
        while (t > 0) scoreStr[pos++] = temp[--t];
    }
    scoreStr[pos] = '\0';
    Graphics::DrawString(10, 10, scoreStr, COLOR_WHITE);
    
    if (gameOver) {
        Graphics::DrawString(screenW/2 - 60, screenH/2 - 20, "GAME OVER!", COLOR_RED);
        Graphics::DrawString(screenW/2 - 80, screenH/2 + 10, "Press any key...", COLOR_WHITE);
    }
    
    // Controls hint
    Graphics::DrawString(10, screenH - 20, "WASD: Move | Esc: Exit", COLOR_GRAY);
}

int GetScore() { return score; }

}

// =============================================================================
// Notepad
// =============================================================================
namespace Notepad {

static char buffer[1024];
static int cursor;
static int scrollY;
static bool saved = false;

void Init() {
    for (int i = 0; i < 1024; i++) buffer[i] = 0;
    cursor = 0;
    scrollY = 0;
    saved = false;
    Serial::Log("Notepad opened!\n");
}

void Input(uint8_t scancode) {
    saved = false;  // File modified
    if (scancode == SC_BACKSPACE) {
        if (cursor > 0) {
            cursor--;
            buffer[cursor] = '\0';
        }
    } else if (scancode == SC_ENTER) {
        if (cursor < 1020) {
            buffer[cursor++] = '\n';
        }
    } else {
        char c = Keyboard::ScancodeToChar(scancode, false);
        if (c && cursor < 1020) {
            buffer[cursor++] = c;
            buffer[cursor] = '\0';
        }
    }
}

void Save(bool useEXT2) {
    if (useEXT2 && EXT2::IsMounted()) {
        EXT2::WriteFile("note.txt", buffer, cursor);
    }
    saved = true;
    Serial::Log("Notepad: File saved!\n");
}

void Draw(int screenWidth, int screenHeight) {
    int winX = 50;
    int winY = 30;
    int winW = screenWidth - 100;
    int winH = screenHeight - 80;
    
    // Window frame
    Graphics::DrawRect(winX, winY, winW, winH, COLOR_WHITE);
    Graphics::DrawRect(winX, winY, winW, 2, COLOR_DARK_GRAY);
    Graphics::DrawRect(winX, winY, 2, winH, COLOR_DARK_GRAY);
    Graphics::DrawRect(winX + winW - 2, winY, 2, winH, COLOR_DARK_GRAY);
    Graphics::DrawRect(winX, winY + winH - 2, winW, 2, COLOR_DARK_GRAY);
    
    // Title bar
    Graphics::DrawRect(winX + 2, winY + 2, winW - 4, 22, COLOR_DARK_BLUE);
    if (saved) {
        Graphics::DrawString(winX + 8, winY + 7, "Notepad - Saved!", COLOR_WHITE);
    } else {
        Graphics::DrawString(winX + 8, winY + 7, "Notepad - NOTE.TXT*", COLOR_WHITE);
    }
    
    // Close button
    Graphics::DrawRect(winX + winW - 22, winY + 5, 16, 16, COLOR_RED);
    Graphics::DrawString(winX + winW - 18, winY + 6, "X", COLOR_WHITE);
    
    // Text area
    int textX = winX + 10;
    int textY = winY + 32;
    Graphics::DrawString(textX, textY, buffer, COLOR_BLACK);
    
    // Cursor
    int cursorX = textX;
    int cursorY = textY;
    for (int i = 0; i < cursor; i++) {
        if (buffer[i] == '\n') {
            cursorX = textX;
            cursorY += 10;
        } else {
            cursorX += 8;
        }
    }
    Graphics::DrawRect(cursorX, cursorY, 2, 10, COLOR_BLACK);
    
    // Status bar
    Graphics::DrawRect(winX + 2, winY + winH - 20, winW - 4, 18, COLOR_GRAY);
    Graphics::DrawString(winX + 8, winY + winH - 16, "F5: Save | Esc: Exit", COLOR_BLACK);
}

}

