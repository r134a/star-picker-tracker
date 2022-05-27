/**
 * \file power.cpp
 * \brief power management
 *
 * These routines rely mostly on the LowPower.h library, with a few extra's
 * - if Serial is used, it is ended / restored at sleep / wakeup
 * - TwoWire (I2C) is disabled / enabled at sleep / wakeup
 * - TwoWire lines pullup resistors are disabled / enabled at sleep / wakeup
 * - Power to the RTC is removed / reinstalled at sleep / wakeup
 * The power routines are only called by sleep() (and once power_on in setup),
 * so there is no attempt to do smart power management by the (very short)
 * wakeup periods.
 *
 * battery_check () is a function that checks the batery voltage.
 */

#include "power.h"

/**
 * Sleep the processor for 40 seconds
 */
void sleep()
{
	power_off();
	for (int n = 10; n--;)
	{
		pinMode(calibrate_pin, INPUT);
		digitalWrite(calibrate_pin, HIGH); // enable pullup calibrate
		if (!digitalRead(calibrate_pin))   // calibrate button pushed?
		{
			power_on();
			// ensure power is on and calibrate pin is operational
			calibration_mode();
		}
		digitalWrite(calibrate_pin, LOW); // disable pullup calibrate
		LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
	}
	power_on();
}

/**
 * Power on
 */
void power_on()
{
	delay(1); // let hardware stabilize
#ifdef SERIAL_BPS
	Serial.begin(SERIAL_BPS); // reinitialize serial
#endif
	digitalWrite(A4, HIGH);					  // enable pull-up resistors
	digitalWrite(A5, HIGH);					  // from the I2C pins
	TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA); // re-initialize twowire.
	digitalWrite(rtc_power_pin, HIGH);		  // power up RTC NVRAM chip
	delay(1);								  // let hardware stabilize
}

/**
 * Power off
 */
void power_off()
{
	delay(1);						  // let hardware stabilize
	TWCR = 0;						  // disable twowire
	digitalWrite(rtc_power_pin, LOW); // power down RTC NVRAM chip
	digitalWrite(A4, LOW);			  // disable pull-up resistors
	digitalWrite(A5, LOW);			  // from the I2C pins
#ifdef SERIAL_BPS
	Serial.end(); // close Serial
#endif
	delay(1); // let hardware stabilize
}

/**
 * Check if battery voltage is OK (true) or not
 */
boolean battery_check()
{
	// set reference to VCC and the measurement to the internal 1.1V reference
	if (ADMUX != ADMUX_VCCWRT1V1)
	{
		ADMUX = ADMUX_VCCWRT1V1;
		delayMicroseconds(350); // wait for Vref to settle
	}
	ADCSRA |= _BV(ADSC); // objectt conversion
	while (bit_is_set(ADCSRA, ADSC))
	{
	};					// wait for it to finish
	return (ADC < 351); // false when Vcc < 3.2 V
}
