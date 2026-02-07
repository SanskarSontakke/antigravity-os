#ifndef MOUSE_H
#define MOUSE_H
#include <stdint.h>

class Mouse {
public:
    static void Init();
    static void HandleInterrupt();
    
private:
    static void WaitWrite();
    static void WaitRead();
    static void Write(uint8_t data);
    static uint8_t Read();
};
#endif
