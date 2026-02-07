#ifndef RTC_H
#define RTC_H
#include <stdint.h>

class RTC {
public:
    static uint8_t GetSecond();
    static uint8_t GetMinute();
    static uint8_t GetHour();

private:
    static uint8_t Read(uint8_t reg);
};
#endif
