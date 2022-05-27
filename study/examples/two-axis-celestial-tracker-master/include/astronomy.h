#include <Arduino.h>

#include "debug.h"
#include "Time.h"

#define pi 3.141592653589793

// https://en.wikipedia.org/wiki/Sidereal_time
#define solar_ms_seconds_per_sidereal_day 86164091ull
// Offset at Longitude 0 on 2019-01-01 00:00:00
// use http://neoprogrammics.com/sidereal_time_calculator/index.php
// to calculate sidereal angle at Greenwich, which is 6.6907020497
// (decimal hours). Divide by 24 and multiply by
// solar_ms_seconds_per_sidereal_day, 24020761
// now subtract the ms between 2019-01-01 and 1970-01-01: 1546300800000
#define siderial_ms_offset 1546276779239ull

#define max_object 14

void getObject(int index, time_t now, double *object_ra, double *object_dc);
void transform(double object_ra_h, double object_dc_d,
               double loc_lat_d, double sidereal_r,
               double *object_az_d, double *object_al_d);
double getSiderealAngle(time_t t, double loc_lng_d);
