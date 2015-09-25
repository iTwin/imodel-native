/*----------------------------------------------------------------------+
|
|   $Source: DgnCore/SolarUtility.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

enum
    {
    YEAR_epoch      = 1980,
    MAX_iterations  = 20,
    };

static double const     s_GmtToLst = 1.002738;
static double const     s_LstToGmt = 0.997270;
static double const     s_oneMinute = 1.0 / 60.0;
static double const     s_oneLstMinute = s_oneMinute * s_LstToGmt;

/*----------------------------------------------------------------------+
|                                                                       |
|   The algorithm for determining the position of the sun is based     |
|   on the basic program from the book "Basic Astronomy for the PC"     |
|                                                                       |
+----------------------------------------------------------------------*/

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/95
//---------------------------------------------------------------------------------------
static double degreesCos (double angle)
    {
    return cos (angle * msGeomConst_radiansPerDegree);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/95
//---------------------------------------------------------------------------------------
static double degreesTan (double angle)
    {
    return tan (angle * msGeomConst_radiansPerDegree);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/95
//---------------------------------------------------------------------------------------
static double degreesSin (double angle)
    {
    return sin (angle * msGeomConst_radiansPerDegree);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/95
//---------------------------------------------------------------------------------------
static double degreesAsin (double value)
    {
    return msGeomConst_degreesPerRadian * asin (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/95
//---------------------------------------------------------------------------------------
static double degreesAcos (double value)
    {
    return msGeomConst_degreesPerRadian * acos (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/95
//---------------------------------------------------------------------------------------
static double degreesAtan2 (double yValue, double xValue)
    {
    return msGeomConst_degreesPerRadian * atan2 (yValue, xValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      01/91
//---------------------------------------------------------------------------------------
static double decimalHours (int hour, int minute)
    {
    return ((double) hour + (double) minute/ 60.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/95
//---------------------------------------------------------------------------------------
static double julianDays (int month, int day, int year)
    {
    int     y = year - 1, m1 = month + 12;
    double  temp, a = 0.0, b = 0.0, jd, x1, x2, t;

    if (month > 2)
        {
        y = year;
        m1 = month;
        }
    t = (year < 0) ? .75 : 0.0;

    temp = year + month / 100.0  + day / 10000.0;
    if (temp >= 1582.1015)
        {
        a = (int) (y / 100);
        b = 2.0 - a + (int) (a / 4);
        }
    x1 = 365.25 * y - t;
    x2 = 30.6001 * (m1 + 1);
    jd = (int) x1 + (int) x2 + day + 1720994.5 + b;

    return ((int) ((jd + 5.000001E-03) * 100.0)) / 100.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/95
//---------------------------------------------------------------------------------------
static double greenwichSiderealTime (int month, int day, int year, int hour, int minute)
    {
    double  jd1 = julianDays (1, 0, year);
    double  days = julianDays (month, day, year) - jd1;
    double  t = (jd1 - 2415020.0) / 36525.0;
    double  r = 6.6460656 + t * 2400.051262 + t * t * .00002581;
    double  b = 24.0 - r + 24 * (year - 1900);
    double  t0 = .0657098 * days - b;
    double  gst = t0 + decimalHours (hour, minute) * s_GmtToLst;

    while (gst < 0.0)  gst += 24.0;
    while (gst > 24.0) gst -= 24.0;

    return gst;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/95
//---------------------------------------------------------------------------------------
static double localSiderealTime (double gmt, double longitude)
    {
    double gst = gmt - longitude / 15.0;

    while (gst < 0.0)  gst += 24.0;
    while (gst > 24.0) gst -= 24.0;

    return gst;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/95
//---------------------------------------------------------------------------------------
static bool isLeapYear (int year)
    {
    if (0.0 != fmod ((double) year, 4.0))
        return false;

    if (0.0 != fmod ((double) year, 100.0))
        return true;

    return 0.0 == fmod ((double) year, 400.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley     03/95
//---------------------------------------------------------------------------------------
static double fix360 (double value)
    {
    double multiple = value < 0.0 ? (int) (-value / 360.0) : - (int) (value / 360.00);

    return value + 360.0 * multiple;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/95
//---------------------------------------------------------------------------------------
static void eclipticToEquatorial (double& declination, double& rightAscension, double eclipticLongitude, double eclipticLatitude)
    {
    double  e, t, t1, y, x, jdEpoch = julianDays (1, 0, YEAR_epoch);

    t  = (jdEpoch - 2415020.0) / 36525.0;
    t = 46.845 * t + .0059 * t * t - .00181 * t * t * t;
    t = t / 3600.0;
    e = 23.452294 - t;
    t =  degreesSin (eclipticLatitude) * degreesCos (e);
    t1 = degreesCos (eclipticLatitude) * degreesSin (e) * degreesSin (eclipticLongitude);

    declination = degreesAsin (t + t1);

    t = degreesSin (eclipticLongitude) * degreesCos (e);
    t1 = degreesTan (eclipticLatitude) * degreesSin (e);
    y  = t - t1;
    x = degreesCos (eclipticLongitude);
    rightAscension = degreesAtan2 (y, x)/15.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/95
//---------------------------------------------------------------------------------------
static void equatorialToHorizon (double& altitude, double& azimuth, double hourAngle, double declination, double latitude)
    {
    double      ha, t1, t2, t3;

    ha = hourAngle * 15.0;
    t1 = degreesSin (declination) * degreesSin (latitude);
    t2 = degreesCos (declination) * degreesCos (latitude) * degreesCos (ha);
    altitude = degreesAsin (t1 + t2);

    t1 = degreesSin (declination) - degreesSin (latitude) * degreesSin (altitude);
    t2 = degreesCos (latitude);
    t3 = degreesCos (altitude);
    t2 = t1 / (t2 * t3);
    azimuth = degreesAcos (t2);
    if (degreesSin (ha) > 0.0)
        azimuth = 360.0 - azimuth;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   PeteSegal       08/07
//---------------------------------------------------------------------------------------
static double  dateTimeToEcliptic (double& lst, int year, int month0, int day, int hour, int minute, double eastLongitude)
    {
    // Limit testing
    if (year < 0)
        year = YEAR_epoch;
    else if (year > 3000)
        year = 2000;

    int     nLeapYears = 0;
    int     month = month0 + 1;
    double  jd = julianDays (month, day, year);
    double  gst = greenwichSiderealTime (month, day, year, hour, minute);
    double  yearDays = jd - julianDays (1, 0, year);

    lst = localSiderealTime (gst, -eastLongitude);

    if (year != YEAR_epoch)
        {
        int     step = year > YEAR_epoch ? 1 : -1;

        for (int testYear = step < 0 ? YEAR_epoch - 1 : YEAR_epoch;  testYear != year;  testYear += step)
            if (isLeapYear (testYear))
                nLeapYears += step;
        }

    double  d = nLeapYears + 365.0 * (year - YEAR_epoch) + yearDays + decimalHours (hour, minute) / 24.0;
    double  n = fix360 (d * (360.0 / 365.2422));
    double  m = n + 278.83354 - 282.596403;

    while (m < 0.0)
        m += 360.0;

    double  ec = (360.0 / msGeomConst_pi) * .016718 * degreesSin (m);

    double  eclLong = n + ec + 278.83354;
    if (eclLong > 360.0)
        eclLong -= 360.0;

    return eclLong;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   PeteSegal       08/07
//---------------------------------------------------------------------------------------
static void dateTimeToEquatorial (double& rightAscension, double& declination, double& hourAngle, int year, int month, int day, int hour, int minute, double longitude)
    {
    double  lst;
    double  eclLong = dateTimeToEcliptic (lst, year, month, day, hour, minute, longitude);

    eclipticToEquatorial (declination, rightAscension, eclLong, 0.0);

    hourAngle = lst - rightAscension;

    if (hourAngle < 0.0)
        hourAngle += 24.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/93
//---------------------------------------------------------------------------------------
DPoint3dR   SolarUtility::AzimuthAnglesToDirection (DPoint3dR direction, double azimuth, double altitude, DgnModelR model)
    {
    direction.x = sin (azimuth) * cos (altitude);
    direction.y = cos (azimuth) * cos (altitude);
    direction.z = sin (altitude);

    /* Compensate for true North */
    RotMatrix   azimuthRotMatrix;

    double      azimuthAngle = model.GetProperties().GetAzimuthAngle ();
    azimuthRotMatrix.InitFromPrincipleAxisRotations(RotMatrix::FromIdentity (),  0.0,  0.0,  azimuthAngle * msGeomConst_radiansPerDegree);
    azimuthRotMatrix.Multiply (direction);
    direction.Normalize ();

    return direction;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley      03/93
//---------------------------------------------------------------------------------------
void    SolarUtility::DirectionToAzimuthAngles (double& azimuth, double& altitude, DPoint3dCR direction, DgnModelR model)
    {
    /* Compensate for true North */
    RotMatrix   azimuthRotMatrix;
    double      azimuthAngle = model.GetProperties().GetAzimuthAngle ();
    azimuthRotMatrix.InitFromPrincipleAxisRotations(RotMatrix::FromIdentity (),  0.0,  0.0,  azimuthAngle * msGeomConst_radiansPerDegree);

    DPoint3d localDirection = direction;
    azimuthRotMatrix.MultiplyTranspose(localDirection);

    azimuth = msGeomConst_piOver2 - atan2 (localDirection.y, localDirection.x) * msGeomConst_degreesPerRadian;
    if (azimuth < 0.0)
        azimuth += 360.0;

    altitude = atan2 (localDirection.z, sqrt (localDirection.x * localDirection.x + localDirection.y * localDirection.y)) * msGeomConst_degreesPerRadian;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    Peter.Segal                     08/07
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   sunriseSunsetLST
(
double&             lstRise,            // <=  LST for sunrise
double&             lstSet,             // <=  LST for sunset
double              rightAscension,     //  => angle from 0 longitude
double              declination,        //  => angle from equator
double              latitude            //  => latitude
)
    {
    if (latitude > 89.0 || latitude < -89.0)
        return ERROR;       // star does not rise/set near poles

    double  altitudeRise = degreesSin (declination) / degreesCos (latitude);

    if (altitudeRise > 1.0 || altitudeRise < -1.0)
        return ERROR;       // star does not rise/set

    double  h1 = degreesTan (latitude) * degreesTan (declination);

    if (h1 > 1.0 || h1 < -1.0)
        return ERROR;       // star does not rise/set

    double  h2 = degreesAcos (-h1) * (1.0 / 15.0);

    if ((lstRise = 24.0 + rightAscension - h2) > 24.0)
        lstRise -= 24.0;

    if ((lstSet = rightAscension + h2) > 24.0)
        lstSet -= 24.0;

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    Peter.Segal                     08/07
+---------------+---------------+---------------+---------------+---------------+------*/
static double  sineAltitude
(
double          eclLong,
double          lst,
double          sinLatitude,
double          cosLatitude
)
    {
    double      rightAscension, declination;

    eclipticToEquatorial (declination, rightAscension, eclLong, 0.0);
    if (rightAscension < 0.0)
        rightAscension += 24.0;

    double      hourAngle = lst - rightAscension;
    if (hourAngle < 0.0)
        hourAngle += 24.0;

    return degreesSin (declination) * sinLatitude +
           degreesCos (declination) * cosLatitude * degreesCos (hourAngle * 15.0);
    }


/*----------------------------------------------------------------------------------*//**
* @bsimethod                    Peter.Segal                     08/07
+---------------+---------------+---------------+---------------+---------------+------*/
static double  roundTimeToMinute
(
double          rawTime,
bool            roundUp
)
    {
    static double const     fractionOfSecond  = (0.01 / 60.0) * s_oneMinute;
    static double const     nearlyWholeMinute = s_oneMinute - fractionOfSecond;


    while (rawTime < 0.0)
        rawTime += 24.0;

    rawTime += roundUp ? nearlyWholeMinute : fractionOfSecond;

    int     hour = (int)rawTime;
    while (hour > 23)
        hour -= 24;

    int     minute = (int)(60.0 * (rawTime - (int)rawTime));

    return hour + minute * s_oneMinute + fractionOfSecond;
    }

/*----------------------------------------------------------------------------------*//**
* Compute sunrise & sunset time 
*
* @bsimethod                    Peter.Segal                     08/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    SolarUtility::GetSolarTimeRange   // <= ERROR if sun does not rise/set
(
double*             sunriseP,       // <=  time (in decimal hours) of sunrise
double*             sunsetP,        // <=  time (in decimal hours) of sunset
int                 year,           //  => year
int                 month0,         //  => month (NOTE: 0 == January)
int                 day,            //  => day
int                 dst,            //  => daylight savings
double              gmtOffset,
GeoPoint2d const&   geoLocation     
)
    {
#if defined (debug)
    printf ("\nsunrise/senset\n");
#endif

    static double const     maxSunset = 23.0 + (59.0 / 60.0);                   // 23:59

    // compute the 'star' rising and setting times for midnight (GMT) of the requested date
    double  lstMidnightGMT;
    double  eclLongMidnight = dateTimeToEcliptic (lstMidnightGMT, year, month0, day, 0, 0, geoLocation.longitude);

    double      rightAscension, declination;
    eclipticToEquatorial (declination, rightAscension, eclLongMidnight, 0.0);
    if (rightAscension < 0.0)
        rightAscension += 24.0;

    // get rising/setting time for desired day
    double      lstRise1, lstSet1;              // sun rise/set times (lst) for desired date
    if (SUCCESS != sunriseSunsetLST (lstRise1, lstSet1, rightAscension, declination, geoLocation.latitude))
        return *sunriseP = 0.0, *sunsetP = maxSunset, ERROR;

    // compute the 'star' rising and setting times for midnight for the next day

    static double const     s_eclLongPerDay = 0.985647;                         // ecliptic longitude increment for one day (in degrees)
    double                  eclLongNext = eclLongMidnight + s_eclLongPerDay;
    if (eclLongNext > 360.0)
        eclLongNext -= 360.0;

    eclipticToEquatorial (declination, rightAscension, eclLongNext, 0.0);
    if (rightAscension < 0.0)
        rightAscension += 24.0;

    // get rising/setting time for next day
    double      lstRise2, lstSet2;              // sun rise/set times (lst) for next day
    if (SUCCESS != sunriseSunsetLST (lstRise2, lstSet2, rightAscension, declination, geoLocation.latitude))
        return *sunriseP = 0.0, *sunsetP = maxSunset, ERROR;

    // Interpolate the times using the formula: t = 24.07 * t1 / (24.07 * (t1 - t2))
    // Which is the same as the following simplified formula: t = t1 / (1 + (t1 - t2) * 1/24.07)

    double      deltaRise =  (lstRise1 - lstRise2);
    if (deltaRise > 23)         deltaRise -= 24.0;
    else if (deltaRise < -23)   deltaRise += 24.0;;

    double      deltaSet =  (lstSet1 - lstSet2);
    if (deltaSet > 23)         deltaSet -= 24.0;
    else if (deltaSet < -23)   deltaSet += 24.0;;


    double      lstRise = lstRise1 / (1.0 + deltaRise * (1.0 / 24.07));
    double      lstSet  = lstSet1  / (1.0 + deltaSet  * (1.0 / 24.07));


    double      gmtToLmt = dst ? gmtOffset + 1.0 : gmtOffset;

#if defined (debug)
    printf ("sunRise  LST: %f %hs, ", lstRise, prTime (gmtToLmt, lstRise, lstMidnightGMT));
    printf ("sunSet   LST: %f %hs\n", lstSet,  prTime (gmtToLmt, lstSet,  lstMidnightGMT));
#endif

    double      sinLatitude = degreesSin (geoLocation.latitude);        // valid for all iterations
    double      cosLatitude = degreesCos (geoLocation.latitude);

    // validate the sunrise time
    int         dayLocal    = day;
    double      gmt         = (lstRise - lstMidnightGMT) * s_LstToGmt;
    if (gmt < 0.0)
        --dayLocal, gmt += 24.0;
    else if (gmt > 24.0)
        ++dayLocal, gmt -= 24.0;

    int         hour        = (int)gmt;
    int         minute      = (int)(0.5 + 60.0 * (gmt - (int)gmt));
    double      eclLong     = dateTimeToEcliptic (lstRise, year, month0, dayLocal, hour, minute, geoLocation.longitude);
    double      sinAltitude = sineAltitude (eclLong, lstRise, sinLatitude, cosLatitude);

    int         iterations = 0;

#if defined (debug)
    printf ("%8d: %02d:%2d sunRise %hs, LST: %f, Alt %f\n", iterations, hour, minute, prTime (gmtToLmt, lstRise, lstMidnightGMT), lstRise, degreesAsin (sinAltitude));
#endif

    if (sinAltitude > 0.0)
        {
        // see if this is the earliest time where sun is above horizon
        while (sinAltitude > 0.0 && ++iterations < MAX_iterations)
            {
            if (--minute < 0)
                --hour, minute = 59;

            eclLong = dateTimeToEcliptic (lstRise, year, month0, dayLocal, hour, minute, geoLocation.longitude);

            sinAltitude = sineAltitude (eclLong, lstRise, sinLatitude, cosLatitude);
            }

        // back up to the last time above horizon
        if (++minute > 59)
            ++hour, minute = 0;
        }
    else        // sun is not yet above the horizon, keep looking
        {
        while (sinAltitude <= 0.0 && ++iterations < MAX_iterations)
            {
            if (++minute > 59)
                ++hour, minute = 0;

            eclLong = dateTimeToEcliptic (lstRise, year, month0, dayLocal, hour, minute, geoLocation.longitude);

            sinAltitude = sineAltitude (eclLong, lstRise, sinLatitude, cosLatitude);
            }
        }

    assert (iterations < MAX_iterations);
    if (iterations >= MAX_iterations)
        return *sunriseP = 0.0, *sunsetP = maxSunset, ERROR;

    gmt = hour + minute * s_oneMinute;
    *sunriseP = roundTimeToMinute (gmt + gmtToLmt, true);           // round sunrise up to prevent roundoff

    // validate the sunset time
    dayLocal    = day;
    gmt         = (lstSet - lstMidnightGMT) * s_LstToGmt;
    if (gmt < 0.0)
        --dayLocal, gmt += 24.0;
    else if (gmt > 24.0)
        ++dayLocal, gmt -= 24.0;

    hour        = (int)gmt;
    minute      = (int)(0.5 + 60.0 * (gmt - (int)gmt));
    eclLong     = dateTimeToEcliptic (lstRise, year, month0, dayLocal, hour, minute, geoLocation.longitude);
    sinAltitude = sineAltitude (eclLong, lstSet, sinLatitude, cosLatitude);

    iterations = 0;

#if defined (debug)
    printf ("%8d: %02d:%2d sunSet  %hs, LST: %f, Alt %f\n", iterations, hour, minute, prTime (gmtToLmt, lstSet, lstMidnightGMT), lstSet, degreesAsin (sinAltitude));
#endif

    if (sinAltitude > 0.0)
        {
        // sun has not yet dropped below horizon
        while (sinAltitude > 0.0 && ++iterations < MAX_iterations)
            {
            if (++minute > 59)
                ++hour, minute = 0;

            eclLong = dateTimeToEcliptic (lstSet, year, month0, dayLocal, hour, minute, geoLocation.longitude);

            sinAltitude = sineAltitude (eclLong, lstSet, sinLatitude, cosLatitude);
            }

        // back up to the last time above horizon
        if (--minute < 0)
            --hour, minute = 59;
        }
    else        // sun is already below the horizon, back up
        {
        while (sinAltitude <= 0.0 && ++iterations < MAX_iterations)
            {
            if (--minute < 0)
                --hour, minute = 59;

            eclLong = dateTimeToEcliptic (lstSet, year, month0, dayLocal, hour, minute, geoLocation.longitude);

            sinAltitude = sineAltitude (eclLong, lstSet, sinLatitude, cosLatitude);
            }
        }

    assert (iterations < MAX_iterations);
    if (iterations >= MAX_iterations)
        return *sunriseP = 0.0, *sunsetP = maxSunset, ERROR;

    gmt = hour + minute * s_oneMinute;
    *sunsetP  = roundTimeToMinute (gmt + gmtToLmt, false);      // round sunset down to prevent roundoff

#if defined (debug)
    printf ("sunRise  LST: %f, %f, %02d:%08.5f\n", lstRise, *sunriseP, (int)*sunriseP, 60.0 * (*sunriseP - (int)*sunriseP));
    printf ("sunSet   LST: %f, %f, %02d:%08.5f\n", lstSet,  *sunsetP,  (int)*sunsetP,  60.0 * (*sunsetP -  (int)*sunsetP));
#endif

    if (*sunsetP <= *sunriseP)      // time zone is wrong, so sunrise is tomorrow - just use whole range
        {
        *sunsetP += 24.0;
        return ERROR;
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   RayBentley       05/2012
//---------------------------------------------------------------------------------------
DVec3d SolarUtility::GetSolarDirection 
(
int                 year,          
int                 month,         
int                 day,           
int                 hour,
int                 minute,
bool                dst,           
double              gmtOffset,
GeoPoint2d const&   geoLocation,
DgnModelR           model,
double*             outAzimuth,
double*             outAltitude
) 
    {
    if (dst)
        --hour;

    hour    -= static_cast <int> (gmtOffset);
    minute  -= static_cast <int> (60.0 * fmod (gmtOffset, 1.0));

    double rightAscension, declination, hourAngle;
    dateTimeToEquatorial (rightAscension, declination, hourAngle, year, month, day, hour, minute, geoLocation.longitude);

    double altitude, azimuth;
    equatorialToHorizon (altitude, azimuth, hourAngle, declination, geoLocation.latitude);

    azimuth     *= msGeomConst_radiansPerDegree;
    altitude    *= msGeomConst_radiansPerDegree;

    DVec3d      outVector;
    AzimuthAnglesToDirection (outVector, azimuth, altitude, model);

    if (NULL != outAzimuth)
        *outAzimuth = azimuth;
    if (NULL != outAltitude)
        *outAltitude = altitude;

    return outVector;
    }

