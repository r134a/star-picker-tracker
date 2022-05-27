// debugging *******************************************************************
// SERIAL_POS only prints the exact Azimuth and Altitude of the pointer
// #define SERIAL_POS

// SERIAL_DEBUG prints a lot of calculatory results and is only useful when
// debugging the math
// #define SERIAL_DEBUG

#if defined(SERIAL_DEBUG) || defined(SERIAL_POS)
#define SERIAL_BPS 9600
#endif
