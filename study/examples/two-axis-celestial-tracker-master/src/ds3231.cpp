/**
 * @file ds3231.cpp
 * @brief Using NVRAM for motor positions
 *
 * We misuse the alarm registers of the DS3231 as NVRAM.
 * The very precise DS3231 has no additional NVRAM, so we are misusing the bcd
 * values in the alarm registers for that.
 *
 * https://github.com/JChristensen/DS3232RTC
 *
 * There are only two register sets (alarm register sets) are available so the
 * only valid values for adr are 0 and 1 (non zero). The allowed value range is
 * -2000 to 42639.
 *
 * All communication is done through the I2C bus, controlled using the
 * Wire.h library. Analog pin A4 (PC4) and A5 (PC5) are used as SDA and SCL
 * respectively.
 *
 * The actual time routines are implemented in the DS3232RTC.h library and no
 * additional code is needed here.
 * The RTC clock is a SyncProvider for the Time.h library implemented in
 * DS3232RTC.h library. The time.h library uses an interval to determine if
 * external sync is needed. However, this interval is measured using Timer0,
 * which is disabled during powerdown. Therefor, the syncprovider should be set
 * in loop () and before now() is called.
 *
 * To ensure power usage is low, the VCC of the DS3231 is wired to a digital
 * pin of the arduino. Whenever the RTC or NVRAM is used, call power_on() first
 * and power_off when done. Failing to do so will hang the program.
 *
 * To set the RTC clock to UTC, use the following code for this hardware:
 *
 * rtc_on ();              // don't forget to power on
 * setTime (12, 39, 0, 13, 8, 2016); // hour, min, sec, day, month, year
 * RTC.set (now ());         // transfer to RTC
 */

#include "ds3231.h"

#define ALRM0	0x08					      // Alarm register 0
#define ALRM1	0x0b					      // Alarm register 1

/**
 * mhd is maintained as a global to allow testing without a RTC present
 */
uint8_t mhd[3] = {0, 0, 0};				// minute-hour-day

/**
 * Get value
 * @param [in] adr register 0 or 1
 * @returns value
 */
int rtcRead (int adr)
{
  uint16_t uvalue;
  // read from alarm registers 1 or 2
  RTC.readRTC (adr == 0 ? ALRM0 : ALRM1, mhd, 3);
  //    day in month               hour           min
  uvalue = (mhd[2] - 1) * (60 * 24) + (mhd[1] * 60) + mhd[0];
  // slight negative allowed for slop processing
  return uvalue - 2000;
}

/**
 * Set value
 * @param [in] adr register 0 or 1
 * @param [in] value
 */
void rtcWrite (int adr, int value)
{
  uint16_t uvalue;
  // slight negative allowed for slop processing
  uvalue = value + 2000;
  mhd[0] = uvalue % 60;                 // min
  mhd[1] = (uvalue / 60) % 24;          // hour
  mhd[2] = 1 + (uvalue / (60 * 24));    // day in month
  // write to alarm registers 1 or 2
  RTC.writeRTC (adr == 0 ? ALRM0 : ALRM1, mhd, 3);
}
