/**
 * @file location.cpp
 * @brief Returns location in degrees lat-long of location index
 */

#include "location.h"

/**
 * Set location of the obeserver. Units are degrees
 *
 * Note that the "user interface" (using beeps) uses base 1 so i.e. one beep
 * is Utrecht NL
 * 
 * @param [in] index of location
 * @parm [out] loc_lat lattitude of location
 * @parm [out] loc_lng longitude of location
*/
void getLocation (int index, double *loc_lat, double *loc_lng)
{
  switch (index)
  {
    case 0: // Utrecht NL
      *loc_lat =  52.091702;
      *loc_lng =   5.119823;
      break;
    case 1: // Nordkapp NO
      *loc_lat =  71.169356;
      *loc_lng =  25.786090;
      break;
    case 2: // Singapore SG
      *loc_lat =   1.297808;
      *loc_lng = 103.853796;
      break;
    case 3: // Boise, IL
      *loc_lat =   43.6009089;
      *loc_lng = -116.3041092;
      break;
    case 4: // Atlanta, GA
      *loc_lat =   33.7678358;
      *loc_lng =  -84.4908155;
      break;
  }
}
