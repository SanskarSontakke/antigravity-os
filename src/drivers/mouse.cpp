#include "mouse.h"
#include "../core/interrupts.h"
#include "../core/graphics/console.h"
#include "../core/gui/desktop.h"

// Helper ports
#define DATA_PORT 0x60
#define CMD_PORT 0x64

void Mouse::WaitWrite() {
    int timeout = 100000;
    while (timeout--) {
        if ((InterruptManager::ReadPort(CMD_PORT) & 2) == 0) return;
    }
}

void Mouse::WaitRead() {
    int timeout = 100000;
    while (timeout--) {
        if ((InterruptManager::ReadPort(CMD_PORT) & 1) == 1) return;
    }
}

void Mouse::Write(uint8_t data) {
    WaitWrite();
    InterruptManager::WritePort(CMD_PORT, 0xD4); // Tell CPU we are talking to Mouse
    WaitWrite();
    InterruptManager::WritePort(DATA_PORT, data);
}

uint8_t Mouse::Read() {
    WaitRead();
    return InterruptManager::ReadPort(DATA_PORT);
}

void Mouse::Init() {
    // 1. Enable Auxiliary Device (Mouse)
    WaitWrite();
    InterruptManager::WritePort(CMD_PORT, 0xA8);

    // 2. Enable Interrupts (IRQ 12) in Compaq Status Byte
    WaitWrite();
    InterruptManager::WritePort(CMD_PORT, 0x20); // Get Status
    WaitRead();
    uint8_t status = InterruptManager::ReadPort(DATA_PORT);
    status |= 2; // Enable IRQ 12
    status &= ~0x20; // Disable Mouse Clock (Active low?) - Standard init
    WaitWrite();
    InterruptManager::WritePort(CMD_PORT, 0x60); // Set Status
    WaitWrite();
    InterruptManager::WritePort(DATA_PORT, status);

    // 3. Set Defaults
    Write(0xF6);
    Read(); // Ack

    // 4. Enable Streaming
    Write(0xF4);
    Read(); // Ack
    
    Console::Print("[Kernel] Mouse Initialized.\n");
}

// Static State
static uint8_t cycle = 0;
static uint8_t packet[3];

// Called from InterruptHandler (Hook this up in interrupts.cpp or call it manually)
void Mouse::HandleInterrupt() {
    uint8_t status = InterruptManager::ReadPort(CMD_PORT);
    if (!(status & 0x20)) return; // Not mouse data

    uint8_t data = InterruptManager::ReadPort(DATA_PORT);

    // Simple 3-byte Packet Decoder
    if (cycle == 0) {
        if ((data & 0x08) == 0) return; // Sync bit check
        packet[0] = data;
        cycle++;
    } else if (cycle == 1) {
        packet[1] = data;
        cycle++;
    } else {
        packet[2] = data;
        cycle = 0;

        // Decode movement
        int8_t x_rel = (int8_t)packet[1];
        int8_t y_rel = (int8_t)packet[2];
        
        y_rel = -y_rel;

        // Decode Buttons
        // Byte 0: [Y_Ov][X_Ov][Y_S][X_S][1][Mid][Rig][Left]
        bool left_button = (packet[0] & 1);
        bool right_button = (packet[0] & 2);

        // Update Position
        static int mouse_x = 400;
        static int mouse_y = 300;

        mouse_x += x_rel;
        mouse_y += y_rel;
        
        // Clamp
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x >= 799) mouse_x = 799;
        if (mouse_y >= 599) mouse_y = 599;
        
        // Notify Desktop
        Desktop::OnMouseMove(mouse_x, mouse_y);
        
        if (left_button) Desktop::OnMouseDown(1);
        else Desktop::OnMouseUp(1);
    }
}
