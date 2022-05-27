/**
 * @file astronomy.cpp
 * @brief Implemenation basic astronomy functions.
 *
 * A sidereal day is defined as the time for the earth to make a complete
 * revolution with respect to the sky. This is not the same as a solar day
 * as that includes the motion of the earth around the sun, which adds one
 * complete revolution per year. Other than the one axis star tracker, we're
 * adding sidereal_ms_offset to get the absolute sidereal time.
 * 
 * https://en.wikipedia.org/wiki/Sidereal_time states a mean sidereal day is
 * 23:56:4.0916 solar seconds. A solar day is 86400 solar seconds. Note that
 * a sidereal day is 86400 sidereal seconds!
 */

#include "astronomy.h"

/**
 * This function will compute the RA an DE of any of the solar systems objects
 * as seen from the earth. It needs only the time (supplied by now) and will
 * set the global variables object_ra and object_dc
 * * object is one of
 * 0: sun (earth)
 * 1: mercury
 * 2: venus
 * 3: moon
 * 4: mars
 * 5: jupiter
 * 6: saturn
 * 7: uranus
 * 8: neptune
 *
 * All math kindly obtained from http://www.stjarnhimlen.se/comp/ppcomp.html
 *
 *  We do not calculate the topocentric position of the moon
 * (www.stjarnhimlen.se/comp/ppcomp.html#13). We also do not provide code for
 * - pluto (www.stjarnhimlen.se/comp/ppcomp.html#14)
 * - the elongation and physical ephemerides of the planets
 *   (www.stjarnhimlen.se/comp/ppcomp.html#15)
 * - asteroids (www.stjarnhimlen.se/comp/ppcomp.html#16)
 * - comets (www.stjarnhimlen.se/comp/ppcomp.html#17 and
 *   www.stjarnhimlen.se/comp/ppcomp.html#18 and
 *   www.stjarnhimlen.se/comp/ppcomp.html#19)
 * - planet moons.
 * 
 * @param [in] object index of sun/planet/moon, 0 indexed
 * @param [in] now current time in UTC
 * @param [out] object_ra Right Ascention in hours
 * @param [out] object_dc Declination in degrees
 */
void getPlanet(int object, time_t now, double *object_ra, double *object_dc)
{
	double d; // days to Dec 31st 0h00 1999 - note, this is NOT same as J2000

	double No;	// Longitude of the object's ascending node
				// double Ns;      // Longitude of the Sun's ascending node
	double io;	// Object's inclination to the ecliptic (Earth's orbit plane)
				// double is;      // Sun's inclination to ecliptic (plane of the Earth's orbit)
	double wo;	// Argument of perihelion for the object
	double ws;	// Argument of perihelion for the Sun
	double ao;	// Object semi-major axis, or mean distance from Earth (AU, Earth radii for the Moon)
	double as;	// Sun (earth) semi-major axis, or mean distance from Sun (AU)
	double eco; // Object eccentricity (0=circle, 0-1=ellipse, 1=parabola)
	double ecs; // Sun (earth) eccentricity (0=circle, 0-1=ellipse, 1=parabola)
	double Mo;	// Mean Anomaly of the object
	double Ms;	// Mean Anomaly of the Sun (earth)
	double Mj;	// Mean anomaly of Jupiter
	double Ma;	// Mean anomaly of sAturn
	double Mu;	// Mean anomaly of Uranus

	double lat;		// Object geocentric lat
	double lon;		// Object geocentric long
	double Eo;		// Object eccentric anomaly
	double Eo1;		// Next iteration of the eccentric anomaly
	double EoDelta; // Temp for iteration
	double xv, yv;	// Temp vector to compute distance and true anomaly
	double lonsun;	// Sun's true longitude

	double ddo; // Mean elongation of the object
	double F;	// Argument of latitude for the object
	double Lo;	// Mean longitude of the object
	double Ls;	// Mean Longitude of the Sun  (Ns=0)
	double vo;	// Object true anomaly
	double ro;	// Object distance

	double xh, yh, zh; // vector position of object seen from the sun
	double xg, yg, zg; // h-vector + Sun (=earth) position
	double xe, ye, ze; // g-vector converted after ecliptic rotation

	double ecl; // obliquity of the ecliptic, i.e. the "tilt" of the Earth's axis of rotation

	// http://www.stjarnhimlen.se/comp/ppcomp.html#3

	d = (now - 946684800.0) / 86400.0 + 1; // days since 31-12-1999T00:00:00

	// sun elements
	// http://www.stjarnhimlen.se/comp/ppcomp.html#4

	// Ns = 0.0;                                         // 0.0
	// is = 0.0;                                         // 0.0
	ws = 4.93824156690976 + 8.219366312880E-7 * d; // 282.9404 + 4.70935E-05 * d
	as = 1.0;									   // (AU)
	ecs = 0.016709 - 1.151E-09 * d;
	Ms = 6.21419244184825 + 1.720196961933E-2 * d; // 356.047 + 0.9856002585 * d

	// http://www.stjarnhimlen.se/comp/ppcomp.html#5

	Eo = Ms + ecs * sin(Ms) * (1.0 + ecs * cos(Ms));
	xv = as * (cos(Eo) - ecs);
	yv = as * (sqrt(1.0 - ecs * ecs) * sin(Eo));
	vo = atan2(yv, xv);
	ro = sqrt(xv * xv + yv * yv);

	lonsun = vo + ws;	   // lonsun is used for planetary positions
	xg = ro * cos(lonsun); // xg, yg and zg are converted to RA and DE
	yg = ro * sin(lonsun);
	zg = 0;

	// Sun's equatorial coordinates, RA and Dec are computed and the end of this fuunction
	// if not the sun, get the object elements
	// http://www.stjarnhimlen.se/comp/ppcomp.html#4

	switch (object)
	{
	case 1:												 // Orbital elements of Mercury
		No = 0.843540316769135 + 5.665111859171E-7 * d;	 // 48.3313 + 3.24587E-5 * d;
		io = 0.122255078114447 + 8.726646259972E-10 * d; //  7.0047 + 5.00E-8 * d;
		wo = 0.508311436680081 + 1.770531806393E-7 * d;	 // 29.1241 + 1.01444E-5 * d;
		ao = 0.387098;									 // (AU)
		eco = 0.205635 + 5.59E-10 * d;
		Mo = 2.94360599390206 + 7.142471001491E-2 * d; // 168.6562 + 4.0923344368 * d;
		break;

	case 2:												  // Orbital elements of Venus
		No = 1.3383167251 + 4.303807402493E-7 * d;		  // 76.6799 + 2.46590E-5 * d;
		io = 0.0592469467881995 + 4.799655442984E-10 * d; // 3.3946 + 2.75E-8 * d;
		wo = 0.958028679712207 + 2.415081899155E-7 * d;	  // 54.8910 + 1.38374E-5 * d;
		ao = 0.723330;									  // (AU)
		eco = 0.006773 - 1.302E-9 * d;
		Mo = 0.837848798078382 + 2.796244746150E-2 * d; // 48.0052 + 1.6021302244 * d;
		break;

	case 3:											   // Orbital elements of the Moon
		No = 2.18380482931436 - 9.242183063049E-4 * d; // 125.1228 - 0.0529538083# * d
		io = 0.0898041713321162;					   // 5.1454
		wo = 5.55125356008773 + 2.868576423897E-3 * d; // 318.0634 + 0.1643573223# * d
		ao = 60.2666;								   // (Earth radii)
		eco = 0.0549;
		Mo = 2.01350607288027 + 2.280271437431E-1 * d; // 115.3654 + 13.0649929509 * d
		break;

	case 4:													// Orbital elements of Mars
		No = 0.864939798727838 + 3.68405843840215E-7 * d;	// 49.5574 + 2.11081E-5 * d;
		io = 0.0322833551741391 - 3.10668606854991E-10 * d; // 1.8497 - 1.78E-8 * d;
		wo = 5.00039623223179 + 5.11313402993511E-7 * d;	// 286.5016 + 2.92961E-5 * d;
		ao = 1.523688;										// (AU)
		eco = 0.093405 + 2.516E-9 * d;
		Mo = 0.324667892785237 + 9.14588790052766E-3 * d; // 18.6021 + 0.5240207766 * d;
		break;

	case 5:												 // Orbital elements of Jupiter
		No = 1.75325653745689 + 4.832013847316E-7 * d;	 // 100.4542 + 2.76854E-5 * d;
		io = 0.0227416401534861 - 2.717477645355E-9 * d; // 1.3030 - 1.557E-7 * d;
		wo = 4.78006761278927 + 2.871153885993E-7 * d;	 // 273.8777 + 1.64505E-5 * d;
		ao = 5.20256;									 // (AU)
		eco = 0.048498 + 4.469E-9 * d;
		Mo = 0.347233254684272 + 1.450112046753E-3 * d; // 19.8950 + 0.0830853001 * d;
		break;

	case 6:												 // Orbital elements of Saturn
		No = 1.98380056901132 + 4.170987846416E-7 * d;	 // 113.6634 + 2.38980E-5 * d;
		io = 0.0434342637651309 - 1.886700921406E-9 * d; // 2.4886 - 1.081E-7 * d;
		wo = 5.92354101618438 + 5.195164504779E-7 * d;	 // 339.3939 + 2.97661E-5 * d;
		ao = 9.55475;									 // (AU)
		eco = 0.055546 - 9.499E-9 * d;
		Mo = 5.53211777016887 + 5.837118978783E-4 * d; // 316.9670 + 0.0334442282 * d;
		break;

	case 7:												  // Orbital elements of Uranus
		No = 1.29155237312206 + 2.439621228438E-7 * d;	  // 74.0005 + 1.3978E-5 * d;
		io = 0.0134966311056722 + 3.316125578789E-10 * d; // 0.7733 + 1.9E-8 * d;
		wo = 1.68705619892874 + 5.334598858721E-7 * d;	  // 96.6612 + 3.0565E-5 * d;
		ao = 19.18171 - 1.55E-8 * d;					  // (AU)
		eco = 0.047318 + 7.45E-9 * d;
		Mo = 2.48867370706497 + 2.046539221501E-4 * d; // 142.5905 + 0.011725806 * d;
		break;

	case 8:												 // Orbital elements of Neptune
		No = 2.30000536025364 + 5.266181952043E-7 * d;	 // 131.7806 + 3.0173E-5 * d;
		io = 0.0308923277602996 - 4.450589592586E-9 * d; // 1.7700 - 2.55E-7 * d;
		wo = 4.7620627962257 - 1.051909940177E-7 * d;	 // 272.8461 - 6.027E-6 * d;
		ao = 30.05826 + 3.313E-8 * d;					 // (AU)
		eco = 0.008606 + 2.15E-9 * d;
		Mo = 4.54216876376693 + 1.046350542911E-4 * d; // 260.2471 + 0.005995147 * d;
		break;
	}

	if (object != 0)
	{ // for all objects, except the sun

		// eccentric anomaly of the object
		// http://www.stjarnhimlen.se/comp/ppcomp.html#6

		Eo = Mo + eco * sin(Mo) * (1.0 + eco * cos(Mo));
		EoDelta = 1.0;
		while (EoDelta < -0.000872664 || EoDelta > 0.000872664)
		{
			Eo1 = Eo - (Eo - eco * sin(Eo) - Mo) / (1 - eco * cos(Eo));
			EoDelta = Eo - Eo1;
			Eo = Eo1;
		}

		// objects vector
		xv = ao * (cos(Eo) - eco);
		yv = ao * (sqrt(1.0 - eco * eco) * sin(Eo));

		// object true anomaly an distance
		vo = atan2(yv, xv);
		ro = sqrt(xv * xv + yv * yv);

		// object geocentric position in 3D space
		// http://www.stjarnhimlen.se/comp/ppcomp.html#7

		xh = ro * (cos(No) * cos(vo + wo) - sin(No) * sin(vo + wo) * cos(io));
		yh = ro * (sin(No) * cos(vo + wo) + cos(No) * sin(vo + wo) * cos(io));
		zh = ro * (sin(vo + wo) * sin(io));

		// objects geocentric long and lat
		lon = atan2(yh, xh);
		lat = atan2(zh, sqrt(xh * xh + yh * yh));

		// No precession corrections
		// http://www.stjarnhimlen.se/comp/ppcomp.html#8
		// Pertubations for the moon, jupiter, saturn and uranus

		if (object == 3)
		{ // moon

			// http://www.stjarnhimlen.se/comp/ppcomp.html#9

			// first calculate arguments below, which should be in radians
			Ls = Ms + ws;	   // Mean Longitude of the Sun  (Ns=0)
			Lo = Mo + wo + No; // Mean longitude of the Moon
			ddo = Lo - Ls;	   // Mean elongation of the Moon
			F = Lo - No;	   // Argument of latitude for the Moon

			// then add the following terms to the longitude
			lon = lon - 0.022235495 * sin(Mo - 2 * ddo);	  // -1.274 (the Evection)
			lon = lon + 0.011484266 * sin(2 * ddo);			  //  0.658 (the Variation)
			lon = lon - 0.003246312 * sin(Ms);				  // -0.186 (the Yearly Equation)
			lon = lon - 0.001029744 * sin(2 * Mo - 2 * ddo);  // -0.059
			lon = lon - 0.000994838 * sin(Mo - 2 * ddo + Ms); // -0.057
			lon = lon + 0.000925025 * sin(Mo + 2 * ddo);	  //  0.053
			lon = lon + 0.000802851 * sin(2 * ddo - Ms);	  //  0.046
			lon = lon + 0.000715585 * sin(Mo - Ms);			  //  0.041
			lon = lon - 0.000610865 * sin(ddo);				  // -0.035 (the Parallactic Equation)
			lon = lon - 0.000541052 * sin(Mo + Ms);			  // -0.031
			lon = lon - 0.000261799 * sin(2 * F - 2 * ddo);	  // -0.015
			lon = lon + 0.000191986 * sin(Mo - 4 * ddo);	  //  0.011

			// latitude terms
			lat = lat - 0.003019420 * sin(F - 2 * ddo);		 // -0.173
			lat = lat - 0.000959931 * sin(Mo - F - 2 * ddo); // -0.055
			lat = lat - 0.000802851 * sin(Mo + F - 2 * ddo); // -0.046
			lat = lat + 0.000575959 * sin(F + 2 * ddo);		 //  0.033
			lat = lat + 0.000296706 * sin(2 * Mo + F);		 //  0.017

			// distance terms earth radii
			ro = ro - 0.58 * cos(Mo - 2 * ddo);
			ro = ro - 0.46 * cos(2 * ddo);
		}

		// http://www.stjarnhimlen.se/comp/ppcomp.html#10
		// jupiter || saturn || uranus)
		if (object == 5 || object == 6 || object == 7)
		{
			Mj = 0.347233254684272 + 1.450112046753E-3 * d; // 19.8950 + 0.0830853001 * d
			Ma = 5.53211777016887 + 5.837118978783E-4 * d;	// 316.9670 + 0.0334442282 * d
			Mu = 2.48867370706497 + 2.046539221501E-4 * d;	// 142.5905 + 0.011725806  * d; //deg
		}
		if (object == 5)
		{																  // jupiter
			lon = lon - 0.332 * sin(2 * Mj - 5 * Ma - 1.17984257434817);  // 67.6
			lon = lon - 0.056 * sin(2 * Mj - 2 * Ma + 0.366519142918809); // 21
			lon = lon + 0.042 * sin(3 * Mj - 5 * Ma + 0.366519142918809); // 21
			lon = lon - 0.036 * sin(Mj - 2 * Ma);
			lon = lon + 0.022 * cos(Mj - Ma);
			lon = lon + 0.023 * sin(2 * Mj - 3 * Ma + 0.907571211037051); // 52
			lon = lon - 0.016 * sin(Mj - 5 * Ma - 1.20427718387609);	  // 69
		}
		else if (object == 6)
		{																   // saturn
			lon = lon + 0.812 * sin(2 * Mj - 5 * Ma - 1.17984257434817);   // 67.6
			lon = lon - 0.229 * cos(2 * Mj - 4 * Ma - 0.0349065850398866); // 2
			lon = lon + 0.119 * sin(Mj - 2 * Ma - 0.0523598775598299);	   // 3
			lon = lon + 0.046 * sin(2 * Mj - 6 * Ma - 1.17984257434817);   // 67.6
			lon = lon + 0.014 * sin(Mj - 3 * Ma + 0.558505360638185);	   // 32
			lat = lat - 0.020 * cos(2 * Mj - 4 * Ma - 0.0349065850398866); // 2
			lat = lat + 0.018 * sin(2 * Mj - 6 * Ma - 0.855211333477221);  // 49
		}
		else if (object == 7)
		{															  // uranus
			lon = lon + 0.040 * sin(Ma - 2 * Mu + 0.10471975511966);  // 6
			lon = lon + 0.035 * sin(Ma - 3 * Mu + 0.575958653158129); // 33
			lon = lon - 0.015 * sin(Mj - Mu + 0.349065850398866);	  // 20
		}

		// recalculate planets position in 3D space after pertubations
		// and compute Geocentric (Earth-centered) coordinates

		// http://www.stjarnhimlen.se/comp/ppcomp.html#11

		xh = ro * cos(lon) * cos(lat);
		yh = ro * sin(lon) * cos(lat);
		zh = ro * sin(lat);

		if (object == 3)
		{ // moon is viewed directly from the earth
			xg = xh;
			yg = yh;
			zg = zh;
		}
		else
		{ // add the sun's (=earth) position
			xg = xh + as * cos(lonsun);
			yg = yh + as * sin(lonsun);
			zg = zh;
		}
	}

	// rotate to equatorial coords
	// http://www.stjarnhimlen.se/comp/ppcomp.html#12

	// obliquity of ecliptic of date
	ecl = 0.409092959362707 + 6.218608124856E-9 * d; // 23.4393 - 0.0000003563 * d;
	xe = xg;
	ye = yg * cos(ecl) - zg * sin(ecl);
	ze = yg * sin(ecl) + zg * cos(ecl);

	// geocentric RA and Dec
	*object_ra = atan2(ye, xe) * 12.0 / pi;
	if (*object_ra < 0.0)
		*object_ra += 24.0;
	*object_dc = atan(ze / sqrt(xe * xe + ye * ye)) * 180 / pi;

#ifdef SERIAL_DEBUG
	Serial.println("* object *************");
	Serial.print("o ");
	Serial.println(object);
	Serial.print("d ");
	Serial.println(d);
	Serial.print("Ms ");
	Serial.println(Ms);
	Serial.print("Mo ");
	Serial.println(Mo);
	Serial.print("Eo ");
	Serial.println(Eo);
	Serial.print("xv ");
	Serial.println(xv);
	Serial.print("yv ");
	Serial.println(yv);
	Serial.print("xh ");
	Serial.println(xh);
	Serial.print("yh ");
	Serial.println(yh);
	Serial.print("zh ");
	Serial.println(zh);
	Serial.print("ra ");
	Serial.println(*object_ra);
	Serial.print("dc ");
	Serial.println(*object_dc);
#endif
}

/**
 * Set the Right Ascention in decimal hours and the Declination in degrees of
 * the choosen object. 0=sun etc. If adding to the list, update max_object in
 * astronomy.h
 * 
 * Use https://www.astrouw.edu.pl/~jskowron/ra-dec/?q=03%3A47%3A29.1+24%3A6%3A18
 * as a converter and double check with it's link to wikisky.org
 * 
 * Even better, use http://server1.sky-map.org/search?star=alcyone to
 * directly find coordinates.
 *
 * Note that the "user interface" (using beeps) uses base 1 so i.e. one beep
 * is the Sun
 * @param [in] index number of the object. 1-8 are solar system objects. >8 stars
 * @param [in] now current time in UTC
 * @param [out] object_ra Right Ascention in hours
 * @param [out] object_dc Declination in degrees
 */
void getObject(int index, time_t now, double *object_ra, double *object_dc)
{
	switch (index)
	{
	case 9: // Arcturis
		*object_ra = 14.261;
		*object_dc = 19.182;
		break;
	case 10: // Vega
		*object_ra = 18.616;
		*object_dc = 38.784;
		break;
	case 11: // Dubhe (rotates within the northern half of the azimuthal grid)
		*object_ra = 11.062;
		*object_dc = 61.751;
		break;
	case 12: // Capella (rotates within the northern half of the azimuthal grid)
		*object_ra = 5.278;
		*object_dc = 45.998;
		break;
	case 13: // Castor
		*object_ra = 7.577;
		*object_dc = 31.888;
		break;
	case 14: // Alcyone
		*object_ra = 3.7914;
		*object_dc = 24.105;
		break;
	default:
		getPlanet(index, now, object_ra, object_dc);
		break;
	}
}

/**
 * http://www.stjarnhimlen.se/comp/ppcomp.html#12b
 *
 * Transform RA and DC to Azimuth and Altitude
 * @param [in] object_ra_h RA units: hours
 * @param [in] object_dc_d DC units: degrees
 * @param [in] loc_lat_d Latitude units: degrees
 * @param [in] siderial_r Sidereal time units: rads
 * @param [out] object_az_d Azimuth units: degrees (0 - 359)
 * @param [out] object_al_d Altitude units: degrees (-180 - 179)
 */
void transform(double object_ra_h, double object_dc_d,
			   double loc_lat_d, double sidereal_r,
			   double *object_az_d, double *object_al_d)
{

#ifdef SERIAL_DEBUG
	Serial.println("* object RA/DC in h/d **");
	Serial.println(object_ra_h);
	Serial.println(object_dc_d);
#endif

	// ecliptic ra/dc coorinates in rads
	double object_ra_r = object_ra_h / 12.00 * pi;
	double object_dc_r = object_dc_d / 180.0 * pi;

	// observer latitude in rads
	double loc_lat_r = loc_lat_d / 180.0 * pi;

#ifdef SERIAL_DEBUG
	Serial.println("* object RA/DC in rad **");
	Serial.println(object_ra_r);
	Serial.println(object_dc_r);
#endif

	// * Calculate the Hour Angle in rads **************************************
	double hour_angle_r = sidereal_r - object_ra_r;
#ifdef SERIAL_DEBUG
	Serial.println("* hour angle in hrs **");
	Serial.println(hour_angle_r / pi * 12, 4);
#endif

	// ecliptic xyz coorinates in vector coordinates length 1
	// (1,0,0) is ha=0, dc=0
	double object_ec_x = cos(hour_angle_r) * cos(object_dc_r);
	double object_ec_y = sin(hour_angle_r) * cos(object_dc_r);
	double object_ec_z = sin(object_dc_r);

#ifdef SERIAL_DEBUG
	Serial.println("* xyz ecliptic *********");
	Serial.println(object_ec_x);
	Serial.println(object_ec_y);
	Serial.println(object_ec_z);
#endif

	// * rotate over the latitude **********************************************

	// sin and cos of the latitude of the observer
	double csin = sin(loc_lat_r);
	double ccos = cos(loc_lat_r);

#ifdef SERIAL_DEBUG
	Serial.println("* sin cos latitude *****");
	Serial.println(csin);
	Serial.println(ccos);
#endif

	// rotate vector in the y plane
	double tmp_x = (object_ec_x * csin) - (object_ec_z * ccos);
	double tmp_y = object_ec_y;
	double tmp_z = (object_ec_x * ccos) + (object_ec_z * csin);

#ifdef SERIAL_DEBUG
	Serial.println("* xyz after latitude *");
	Serial.println(tmp_x);
	Serial.println(tmp_y);
	Serial.println(tmp_z);
#endif

	// * convert vector to angles **********************************************

	// vector to rads, mirror the x axis
	double object_az_r = atan2(tmp_y, tmp_x) + pi;
	double object_al_r = atan2(tmp_z, sqrt(tmp_x * tmp_x + tmp_y * tmp_y));

#ifdef SERIAL_DEBUG
	Serial.println("* az alt in deg ******");
	Serial.println(object_az_r / pi * 180);
	Serial.println(object_al_r / pi * 180);
#endif

	// rads to degrees and ensure 0 - 359 range for azimuth, -180 - 180 for altitude
	*object_az_d = object_az_r / pi * 180;
	*object_al_d = object_al_r / pi * 180;
}

/**
 * Calculate siderial angle based in time an longitude
 * While not computed as documented in the following link, the result is the same
 *
 * http://www.stjarnhimlen.se/comp/ppcomp.html#5b
 *
 * @param [in] t a 32 bit value in solar seconds since midnight 01-01-1970.
 * @param [in] loc_lng_d the longitude of the observer in degrees
 * @returns sidereal time in rads

*/
double getSiderealAngle(time_t t, double loc_lng_d)

{
	uint64_t ms; // time in ms
	int32_t ln;	 // lonitude correction in ms
	double sidereal_r;

	ms = t * 1000ull;		  // t in ms units and 64 bits
	ms -= siderial_ms_offset; // Greenwich Sidereal

	ln = loc_lng_d * solar_ms_seconds_per_sidereal_day / 360.0;
	ms += ln; // longitude correction to Local Sidereal

	// Calculation introduces 1 ms cummulative drift per day, so 0.36 seconds
	// per year. As one step corresponds to roughly 40 seconds, drift by this
	// calculation is totally neglectable over the course of several decades
	ms = ms % solar_ms_seconds_per_sidereal_day; // ms in the sideral day

	sidereal_r = ms; // convert ms to angle
	sidereal_r = sidereal_r * 2.0 * pi / solar_ms_seconds_per_sidereal_day;
	return (sidereal_r);
}
