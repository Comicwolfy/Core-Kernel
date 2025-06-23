#ifndef RTC_H
#define RTC_H

#include <stdint.h>

void rtc_init();
uint8_t rtc_read(uint8_t reg);

#endif
