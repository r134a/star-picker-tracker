#include <Arduino.h>

#include "debug.h"

#define steps_per_revolution 2048
#define motor_pin_azimuth 2  // motor driver pins 2-5
#define motor_pin_altitude 6 // motor driver pins 6-9
#define step_delay_fast 4    // normaly used
#define step_delay_slow 20   // calibration
#define slop_steps 0         // was 25
#define altitude_limit -200  // lowest step altitude (-35 deg)

void set_step_delay(unsigned long delay);
void step_to(int new_position_azimuth, int new_position_altitude, bool calibrating);
void step_energize(int motor_pin_first, int mode);
