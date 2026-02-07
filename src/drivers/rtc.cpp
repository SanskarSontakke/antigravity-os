#include "rtc.h"
#include "../core/interrupts.h"

uint8_t RTC::Read(uint8_t reg) {
    InterruptManager::WritePort(0x70, reg);
    uint8_t val = InterruptManager::ReadPort(0x71);
    // Convert BCD to Binary
    return (val & 0x0F) + ((val / 16) * 10);
}

uint8_t RTC::GetSecond() { return Read(0x00); }
uint8_t RTC::GetMinute() { return Read(0x02); }
uint8_t RTC::GetHour()   { return Read(0x04); }
