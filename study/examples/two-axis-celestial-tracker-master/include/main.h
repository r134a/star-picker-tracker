#include <Arduino.h>

#include "debug.h"

#include "stepper.h"
#include "ds3231.h"
#include "location.h"
#include "astronomy.h"
#include "calibrate.h"
#include "power.h"

void set_pointer(double sidereal_r);
void set_lang_long(double lat, double lng);
