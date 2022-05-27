#include <Arduino.h>

#include "debug.h"

#include <Time.h>
#include "DS3232RTC.h"

int rtcRead(int adr);
void rtcWrite(int adr, int value);
