/** \mainpage TwoAxisStarTracker
 * \section license_sec License
 * CC BY 4.0
 * Licensed under a Creative Commons Attribution 4.0 International license:
 * http://creativecommons.org/licenses/by/4.0/
 *
 * \section intro_sec Introduction
 * This is a two axis object finder. It points to a fixed point in the clestial sky.
 * This the companion software needed to build this project
 * http://www.instructables.com/id/xxx/
 *
 * Hardware required:
 * - Arduino mini pro (harder to upload to than a nano, but easier for low pwr)
 * - DS3231, DS3232 RTC clock connected through I2C to pin A4 and A5
 * - two 28BYJ-48 Stepper motor with ULN2003 driver board connected to pins 2-5 *
*/

/**
 * includes
 */
#include "main.h"
#include <EEPROM.h>

/**
 * object, ecliptic ra/dc coorinates in hours/degrees
 */
double object_ra;
double object_dc;

/**
 * observers location
 */
double loc_lat;
double loc_lng;

/**
 * battery and power
 */
#define ADMUX_VCCWRT1V1 (_BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1))
boolean stop = false;

/**
 *  setup
 */
void setup()
{
  if (EEPROM.read(0) != '#')
  { // check validity of EEPROM
    EEPROM.write(0, '#');
    EEPROM.write(1, 0);
    EEPROM.write(2, 0);
    EEPROM.write(3, 0);
  }
  beep_calibrate(0, EEPROM.read(1) + 1); // indicate the object
  delay(400);
  beep_calibrate(0, EEPROM.read(2) + 1); // and location

  for (int n = 0; n <= 3; n++) // enable motor pins
  {
    pinMode(motor_pin_azimuth + n, OUTPUT);
    pinMode(motor_pin_altitude + n, OUTPUT);
  }

  pinMode(rtc_power_pin, OUTPUT);     // enable RTC power pin
  power_on();                         // power up peripherals inc Serial
  RTC.writeRTC(0x10, EEPROM.read(3)); // get aging from EEPROM and write to RTC

#ifdef SERIAL_BPS
  Serial.println("start");
#endif

  step_to(steps_per_revolution / 2, 0, false);     // check for free running pointer
  delay(500);                                      // easier for altitude (horizontal)
  step_to(0, altitude_limit, false);               // check full (halfway, 0) calibration
  delay(500);                                      // easier for altitude (horizontal)
  step_to(-steps_per_revolution / 2, 0, false);    // check for free running pointer
  delay(500);                                      // also easier to see azimuth zero
  getLocation(EEPROM.read(2), &loc_lat, &loc_lng); // set lat/long of the observer
}

/**
 * loop
 */
void loop()
{
  if (stop) // a stop is final
  {
    beep(50, 0); // warn user
  }
  else if (battery_check()) // if there is enough power
  {
    // Set the plugin function to get the time from the RTC. As the processor
    // is powered down, including Timer0, the SyncProvider needs to be set
    // in the loop() as it forces a resync of the the Time.
    setSyncProvider(RTC.get);
    set_pointer(getSiderealAngle(now(), loc_lng)); // update the pointer
  }
  else
  {
    beep(50, 0);
    stop = true; // set the stop state
  }
  sleep(); // ultra low power state
}

/**
 * sets the lat and lang of the object to be pointed at
 */
void set_lang_long(double lat, double lng)
{
  loc_lat = lat;
  loc_lng = lng;
}

/**
 * the real work
 * - gets the RA and DC of the object
 * - transforms it to AZ and AL
 * - rotates the pointer
 */
void set_pointer(double sidereal_r)
{
  double object_az_d; // azimuth of object in degrees
  double object_al_d; // altitude of object in degrees
  int az_ticks;       // azimuth of object in motor ticks
  int al_ticks;       // altitude of object in motor ticks

  getObject(EEPROM.read(1), now(), &object_ra, &object_dc); // get object RA and DecC

  transform(object_ra, object_dc, // transform to AZ and AL
            loc_lat, sidereal_r,
            &object_az_d, &object_al_d);

#ifdef SERIAL_POS
  Serial.print("az/al:");
  Serial.print(object_az_d);
  Serial.print(", ");
  Serial.println(object_al_d);
  delay(20);
#endif

  // transform azimuth from 0 - 360 to 0 - 180, - 180 - 0
  // reason is stars hover in the northern azimuthal grid if they have
  // declinations above the locations latitude. This avoids
  // full rotation in those cases.
  if (object_az_d > 180.0)
    object_az_d -= 360.0;

  // motor rotates counter azimuth
  az_ticks = -object_az_d * steps_per_revolution / 360.0;
  al_ticks = object_al_d * steps_per_revolution / 360.0;
  // protect the pointer
  if (al_ticks < altitude_limit)
    al_ticks = altitude_limit;

  // set pointer
  step_to(az_ticks, al_ticks, false);
}
