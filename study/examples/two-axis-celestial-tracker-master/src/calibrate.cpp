/**
 * @file calibrate.cpp
 * @brief Mode to set pointer calibration, object index, location index and demo
 */

#include "calibrate.h"

#include "main.h"
#include <EEPROM.h>

/**
 * Calibration mode is activated by holding the calibration button. To resume
 * normal operation a reboot is required.
 */
void calibration_mode()
{

	int direction = 4; // 4 step increment needed to match enery state
	int mode = 0;	   // switch to azimuth
	int waitcount = 0;
	int count; // used to speed up after 80 steps
	int object_index;
	int location_index;
	double lat;
	double lng;

	beep_calibrate(1, 0); // label = 1 beep

	// Move the pointer position 0 (rollover position) to hint absolute position
	step_to(0, 0, false);
	delay(250);

	while (!digitalRead(calibrate_pin))
	{
	} // wait until the button is released

	while (true)
	{
		waitcount++;
		// mode is supposed to be in at least fourths. For clarity we use tenths.
		// The following line adds
		// 0: for the first run in loop for this mode
		// 1: for all subsequent loops in this mode
		// 2: when there is a timeout in this mode (500 times delay in mode +1)
		// 3: if key is pressed (always overrules previous 3)
		switch (mode + (!digitalRead(calibrate_pin) ? 3 : (waitcount == 1 ? 0 : (waitcount <= 500 ? 1 : 2))))
		{
		case 0:			   // Azimuth, indicated by left-right swing at minimum altitude
			direction = 4; // reset direction
			step_to(0, altitude_limit, false);
			step_to(48, altitude_limit, false);
			step_to(0, altitude_limit, false);
			break;
		case 1:
			delay(10);
			break;
		case 2:
			waitcount = 0; // reset timer
			mode = 10;	   // switch to altitude
			break;
		case 3: // azimuth, key pressed
			waitcount = 1;
			count = 0;
			set_step_delay(step_delay_slow);
			while (!digitalRead(calibrate_pin))
			{
				// calibrate motor, do not save position
				step_to(direction, altitude_limit, true);
				// speed up if pressed for a longer time
				if (count++ == 20)
					set_step_delay(step_delay_fast);
			}
			set_step_delay(step_delay_fast);
			direction = -direction; // switch direction
			break;

		case 10:		   // Altitude, indicated by up-down swing at horizontal altitude
			direction = 4; // reset direction
			step_to(0, 0, false);
			step_to(0, 48, false); // show altitude
			step_to(0, 0, false);
			break;
		case 11:
			delay(10);
			break;
		case 12:
			waitcount = 0; // reset timer
			mode = 20;	   // switch to select object
			break;
		case 13: // key pressed
			waitcount = 1;
			count = 0;
			set_step_delay(step_delay_slow);
			while (!digitalRead(calibrate_pin))
			{
				// calibrate motor, do not save position
				step_to(0, direction, true);
				// speed up if pressed for a longer time
				if (count++ == 20)
					set_step_delay(step_delay_fast);
			}
			set_step_delay(step_delay_fast);
			direction = -direction; // switch direction
			break;

		case 20:								   // Object 0-8 (1-9) sun ... moon ... neptune, 9-11 (10-12) stars
			step_to(0, 0, false);				   // horizontal
			beep_calibrate(2, EEPROM.read(1) + 1); // label = 2 beeps
			break;
		case 21:
			delay(10);
			break;
		case 22:		   // timeout
			waitcount = 0; // reset timer
			mode = 30;	   // switch to select location
			break;
		case 23:		   // key pressed
			waitcount = 1; // reset the timeout
			while (!digitalRead(calibrate_pin))
			{
			} // wait until key is released
			object_index = EEPROM.read(1) + 1;
			if (object_index > max_object)
				object_index = 0;
			EEPROM.write(1, object_index);
			beep_calibrate(0, object_index + 1);
			break;

		case 30: // Location 0 (1) Utrecht, 1 (2) Norkapp, 2 (3) Singapore
			step_to(0, 0, false);
			beep_calibrate(3, EEPROM.read(2) + 1); // label = 3 beeps
			break;
		case 31:
			delay(10);
			break;
		case 32:
			waitcount = 0; // reset timer
			mode = 100;	   // switch to select demo
			break;
		case 33:		   // key pressed
			waitcount = 1; // reset the timeout
			while (!digitalRead(calibrate_pin))
			{
			} // wait until key is released
			location_index = EEPROM.read(2) + 1;
			if (location_index > max_location)
				location_index = 0;
			EEPROM.write(2, location_index);
			beep_calibrate(0, location_index + 1);
			break;

		case 100: // Demo
			step_to(0, 0, false);
			beep_calibrate(4, 0); // label = 4 beeps
			break;
		case 101:
			delay(10);
			break;
		case 102:
			waitcount = 0; // reset timer
			mode = 0;	   // switch to select azimuth
			break;
		case 103: // key pressed
			while (!digitalRead(calibrate_pin))
			{
			}
			mode = 104; // switch to rundemo, fall through

		case 104:
			getLocation(EEPROM.read(2), &lat, &lng);
			set_lang_long(lat, lng);
			setSyncProvider(RTC.get);				   // set the time to the RTC
			setSyncProvider(0);						   // but disable the sync provider
		case 105:									   // rundemo, no key pressed
			waitcount = 1;							   // avoid timeout, so state 106 not reached
			set_pointer(getSiderealAngle(now(), lng)); // update the pointer
			setTime(now() + 300);					   // advance 5 minutes (but don't update the RTC)
			break;
		case 107: // key pressed
			while (!digitalRead(calibrate_pin))
			{
			}			   // wait until key is released
			waitcount = 0; // reset timer
			mode = 20;	   // switch to select object
			break;
		}
	}
}

void beep_calibrate(int header, int beeps)
/**
 * Beep sequence
 * @param [in] headers is number of fast beeps to indicate a label
 * @param [in] beeps the number of slow beeps to indicate a value, in groups of 3
 */
{
	if (header)
	{
		for (header--; header--;)
		{
			beep(50, 50);
		}
		beep(50, 300);
	}

	for (int count = 0; count < beeps; count++)
	{
		beep(100, 100);
		if (count % 3 == 2)
			delay(100);
	}
}

/**
 * One beep
 * @param [in] ontime in ms
 * @param [in] offtime in ms
 */
void beep(int ontime, int offtime)
{
	if (ontime > 0)
	{
		pinMode(beep_pin, OUTPUT);
		digitalWrite(beep_pin, HIGH); // beeper on
		delay(ontime);
		digitalWrite(beep_pin, LOW); // beeper off
		pinMode(beep_pin, INPUT);
	}
	if (offtime > 0)
	{
		delay(offtime);
	}
}
