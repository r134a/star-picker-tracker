#include <Arduino.h>

#include "debug.h"

#include "DS3232RTC.h"

#include "stepper.h"
#include "astronomy.h"
#include "location.h"

#define calibrate_pin 10 // pushbutton to ground
#define beep_pin 12      // Piezo beeper

void calibration_mode();
void beep_calibrate(int header, int beeps);
void beep(int ontime, int offtime);
