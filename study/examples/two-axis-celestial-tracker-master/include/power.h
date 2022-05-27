#include <Arduino.h>

#include "debug.h"

#include "LowPower.h"
#include "calibrate.h"

#define rtc_power_pin 11 // powers the RTC chip
#define ADMUX_VCCWRT1V1 (_BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1))

void sleep();
void power_on();
void power_off();
boolean battery_check();
