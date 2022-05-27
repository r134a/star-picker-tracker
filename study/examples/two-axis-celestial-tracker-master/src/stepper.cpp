/**
 * \file stepper.cpp
 * \brief Stepper motor functions
 * 
 * Standard stepper.h is useless as it keeps the coils engaged.
 *
 * https://grahamwideman.wikispaces.com/Motors-+28BYJ-48+Stepper+motor+notes
 *
 * However, the motors I obtained, were 2048 steps per rotation indeed. Make
 * sure you test the motor for at least 10 revolutions.
 */

#include "stepper.h"

#include "ds3231.h"

int step_delay_ms = step_delay_fast;

/**
 * Set the stepper delay time in ms
 * @param [in] delay in ms
 */
void set_step_delay(unsigned long delay)
{
	step_delay_ms = delay;
}

/**
 * Step to a new position. Save in NVRAM if not calibrating
 * 
 * No shortest path calculation is used. This is delibarate!!
 * First, every movement is just a few steps, so no need, and it avoids
 * cummulative errors. Second, this allows for cabling to the altitude
 * motor, avoiding slip contacts
 *
 * Every new position is saved to the NVRAM. This way, even if power is
 * interrupted during a movement, calibration is maintained as good as
 * reasonably possible (within one step).
 * 
 * @param [in] new_position_azimuth in steps
 * @param [in] new_position_altitude in steps
 * @param [in] calibrating boolean
 *  
 */
void step_to(int new_position_azimuth, int new_position_altitude, bool calibrating)
{
	int step_position_azimuth;
	int step_delta_azimuth; // how to get there
	int step_position_altitude;
	int step_delta_altitude; // how to get there

	unsigned long time_ms; // time to next step

	step_position_azimuth = rtcRead(0); // get current position
	step_position_altitude = rtcRead(1);

#ifdef SERIAL_DEBUG
	Serial.print(now());
	Serial.print(", nr:");
	Serial.print(new_position_azimuth);
	Serial.print(", ni:");
	Serial.println(new_position_altitude);
#endif

	// get the delta, process slop compensation if needed. Slop compensation
	// is enabled on a counter clockwise rotation and when calibration is
	// false. slop_steps may be 0, which has the same effect as no slop
	// compensation at all
	step_delta_azimuth = new_position_azimuth - step_position_azimuth;
	if (step_delta_azimuth < 0 && !calibrating)
	{
		step_delta_azimuth -= slop_steps;
	}
	step_delta_altitude = new_position_altitude - step_position_altitude;
	if (step_delta_altitude < 0 && !calibrating)
	{
		step_delta_altitude -= slop_steps;
	}

	// handle 'do nothing'. Saves a bit of battery life
	if (step_delta_azimuth == 0 && step_delta_altitude == 0)
		return;

	// sync to the next ms
	time_ms = millis() + 1;
	while ((long)(millis() < time_ms) > 0)
	{
	};

	if (step_delta_azimuth != 0)
		step_energize(motor_pin_azimuth, step_position_azimuth & 0x3); // ensure last pos
	if (step_delta_altitude != 0)
		step_energize(motor_pin_altitude, step_position_altitude & 0x3); // ensure last pos

	time_ms += step_delay_ms;
	while ((long)(millis() < time_ms) > 0)
	{
	};

#ifdef SERIAL_DEBUG
	Serial.print(now());
	Serial.print(", dr:");
	Serial.print(step_delta_azimuth);
	Serial.print(", di:");
	Serial.println(step_delta_altitude);
#endif

	// a motor may be energized here, but in that case its delta is non zero
	// so the stepper loop will be entered and it will be de-energized
	while (step_delta_azimuth != 0 || step_delta_altitude != 0)
	{
		if (step_delta_azimuth > 0) // clockwise
		{
			step_position_azimuth++;
			step_energize(motor_pin_azimuth, step_position_azimuth & 0x3);
			if (!calibrating)
				rtcWrite(0, step_position_azimuth); // save
			step_delta_azimuth--;
		}
		else if (step_delta_azimuth < 0) // counter clockwise
		{
			step_position_azimuth--;
			step_energize(motor_pin_azimuth, step_position_azimuth & 0x3);
			if (!calibrating)
				rtcWrite(0, step_position_azimuth); // save
			if (++step_delta_azimuth == 0 && !calibrating)
				step_delta_azimuth += slop_steps;
		}

		if (step_delta_altitude > 0) // clockwise
		{
			step_position_altitude++;
			step_energize(motor_pin_altitude, step_position_altitude & 0x3);
			if (!calibrating)
				rtcWrite(1, step_position_altitude); // save
			step_delta_altitude--;
		}
		else if (step_delta_altitude < 0) // counter clockwise
		{
			step_position_altitude--;
			step_energize(motor_pin_altitude, step_position_altitude & 0x3);
			if (!calibrating)
				rtcWrite(1, step_position_altitude); // save
			if (++step_delta_altitude == 0 && !calibrating)
				step_delta_altitude += slop_steps;
		}

		// wait until
		time_ms += step_delay_ms;
		while ((long)(millis() < time_ms) > 0)
		{
		};

		// de-energize motor
		if (step_delta_azimuth == 0)
			step_energize(motor_pin_azimuth, 99);
		if (step_delta_altitude == 0)
			step_energize(motor_pin_altitude, 99);
	}
}

/**
 * Energize in one of 4 step positions, or remove power
 * @param [in] motor_pin_first first IO pin of the motor
 * @param [in] mode 0-3 is step position, any other is off
 */
void step_energize(int motor_pin_first, int mode)
{
	switch (mode)
	{
	case 0:
		digitalWrite(motor_pin_first, HIGH);
		digitalWrite(motor_pin_first + 1, HIGH);
		digitalWrite(motor_pin_first + 2, LOW);
		digitalWrite(motor_pin_first + 3, LOW);
		break;
	case 1:
		digitalWrite(motor_pin_first, HIGH);
		digitalWrite(motor_pin_first + 1, LOW);
		digitalWrite(motor_pin_first + 2, LOW);
		digitalWrite(motor_pin_first + 3, HIGH);
		break;
	case 2:
		digitalWrite(motor_pin_first, LOW);
		digitalWrite(motor_pin_first + 1, LOW);
		digitalWrite(motor_pin_first + 2, HIGH);
		digitalWrite(motor_pin_first + 3, HIGH);
		break;
	case 3:
		digitalWrite(motor_pin_first, LOW);
		digitalWrite(motor_pin_first + 1, HIGH);
		digitalWrite(motor_pin_first + 2, HIGH);
		digitalWrite(motor_pin_first + 3, LOW);
		break;
	default:
		digitalWrite(motor_pin_first, LOW);
		digitalWrite(motor_pin_first + 1, LOW);
		digitalWrite(motor_pin_first + 2, LOW);
		digitalWrite(motor_pin_first + 3, LOW);
	}
}
