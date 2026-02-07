#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <stdint.h>

class Keyboard {
public:
    static volatile char lastKey;
    static char ScancodeToAscii(uint8_t scancode);
    static char GetChar();
};
#endif
