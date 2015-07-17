/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PrivateAPI/base/bcConversion.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <dtmevars.h>
#define dc_180overpi (180.0 / DTM_PYE)
#define dc_piover2 DTM_PYE
#define dc_2pi DTM_2PYE
#define dc_zero 0

inline static double bcConversion_radianToDegree
(
    double angleInRadian
)
{
    double angleInDegree;

    // Commented by spu on 02/04/11 think that conversion should not make side effect
    //angleInRadian = bcMath_normalizeAngle(angleInRadian);

    angleInDegree  = angleInRadian * dc_180overpi;
    return angleInDegree;
}
static inline double bcMath_normalizeAngle	/* <=  Fixed angle in radians  */
(
double		angle						/*  => Raw angle in radians    */
)
{
	angle = fmod(angle, dc_2pi);
	if ( angle < dc_zero ) angle += dc_2pi;
	return angle;
}

static inline double bcConversion_directionToAzimuth
( 
    double    direction
)
{
    double azimuth;
    azimuth = dc_piover2 - direction;
    azimuth = bcMath_normalizeAngle(azimuth);
    return azimuth;   //Angle in radians
}

#ifdef notdef
/*=========================== HTML doc =============================*//**
* @memo     converts conventional direction to azimuth degree.
* @doc      converts conventional direction to azimuth degree.
* @notes    NONE
* @author   Vince Unrau / July-19-2001 / vince@bentley.com
* @param    direction => azimuth in degrees
* @return   azimuth angle degreees.
* @version  $Header: 
* @see      bcConversion_azimuthToDirection
*======================================================================*/
BENTLEYDTM_EXPORT double bcConversion_directionToAzimuthDegree
( 
    double    directionDegrees
);

/*=========================== HTML doc =============================*//**
* @memo     converts conventional direction to azimuth.
* @doc      converts conventional direction to azimuth.
* @notes    NONE
* @author   Maher Salameh / July-19-2001 / maher.salameh@bentley.com
* @param    direction => conventional direction in radians
* @return   azimuth angle.
* @version  $Header: 
* @see      bcConversion_azimuthToDirection
*======================================================================*/
BENTLEYDTM_EXPORT double bcConversion_directionToAzimuth
( 
    double    direction
);

/*=========================== HTML doc =============================*//**
* @memo     converts conventional azimuth to direction.
* @doc      converts conventional azimuth to direction.
* @notes    NONE
* @author   Maher Salameh / July-19-2001 / maher.salameh@bentley.com
* @param    azimuth => azimuth
* @return   conventional direction.
* @version  $Header: 
* @see      bcConversion_directionToAzimuth
*======================================================================*/
BENTLEYDTM_EXPORT double bcConversion_azimuthToDirection
(
    double  azimuth
);

/*=========================== HTML doc =============================*//**
* @memo     converts conventional azimuth to direction in degrees.
* @doc      converts conventional azimuth to direction in degrees.
* @notes    NONE
* @author   Vince Unrau / July-19-2001 / vince@bentley.com
* @param    azimuth => azimuth
* @return   conventional direction.
* @version  $Header: 
* @see      bcConversion_directionToAzimuth
*======================================================================*/
BENTLEYDTM_EXPORT double bcConversion_azimuthToDirectionDegree
(
    double  azimuthDegree
);

/*=========================== HTML doc =============================*//**
* @memo     gets the number of the quadrant that the angle is in.
* @doc      gets the number of the quadrant that the angle is in.
* @notes    all angles are radians.
* @author   Maher Salameh / July-27-2001 / maher.salameh@bentley.com
* @param    angle => angle to get quadrant number of.
* @return   the number of the quadrant (from 1 to 4).
* @version  $Header:
* @see      None
*======================================================================*/
BENTLEYDTM_EXPORT int bcConversion_quadrantNumber
(
    double angle
);

/*=========================== HTML doc =============================*//**
* @memo     converts degrees to radians.
* @doc      converts degrees to radians.
* @notes    None
* @author   Maher Salameh / July-27-2001 / maher.salameh@bentley.com
* @param    angleInDegree => angle to be converted to radians
* @return   angle converted from degrees to radians.
* @version  $Header:
* @see      bcConversion_radianToDegree
*======================================================================*/
BENTLEYDTM_EXPORT double bcConversion_degreeToRadian
(
    double angleInDegree
);

/*================================ HTML doc =============================*//**
* @memo     converts gradients to radians.
* @doc      converts gradients to radians.
* @notes    None
* @author   Maher Salameh / July-27-2001 / maher.salameh@bentley.com
* @param    angleInGrad => angle in gradients to be converted to radians
* @return   angle converted form gradients to radians.
* @version  $Header:
* @see      bcConversion_radianToGrad
*==============================================================================*/
BENTLEYDTM_EXPORT double bcConversion_gradToRadian
(
    double angleInGrad
);

/*=========================== HTML doc =============================*//**
* @memo     converts radians to degrees.
* @doc      converts radians to degrees.
* @notes    None
* @author   Maher Salameh / July-27-2001 / maher.salameh@bentley.com
* @param    angleInRadian => angle to be converted to degrees
* @return   angle converted from radians to degrees.
* @version  $Header:
* @see      bcConversion_degreeToRadian
*======================================================================*/
BENTLEYDTM_EXPORT double bcConversion_radianToDegree
(
    double angleInRadian
);

/*=========================== HTML doc =============================*//**
* @memo     converts radians to gradients.
* @doc      converts radians to gradients.
* @notes    None
* @author   Maher Salameh / July-27-2001 / maher.salameh@bentley.com
* @param    angleInRadian => angle to be converted to gradients
* @return   angle converted from radians to gradients.
* @version  $Header:
* @see      bcConversion_gradToRadian
*======================================================================*/
BENTLEYDTM_EXPORT double bcConversion_radianToGrad
(
    double angleInRadian
);

/*=========================== HTML doc =============================*//**
* @memo     converts decimal degrees to degree minute.
* @doc      converts decimal degrees to degree minute.
* @notes    None
* @author   Maher Salameh / July-27-2001 / maher.salameh@bentley.com
* @param    degree        <= degrees
* @param    minute        <= minutes
* @param    angleInDegree => angle in decimal degrees to be converted to degree minute.
* @return   SUCCESS always.
* @version  $Header:
* @see      bcConversion_degreeToDMS, bcConversion_DMToDegree, 
            bcConversion_DMSTodegree
*==========================================================================*/
BENTLEYDTM_EXPORT int bcConversion_degreeToDM
(
    int *degree,
    double *minute,
    double angleInDegree
);

/*=========================== HTML doc =============================*//**
* @memo     converts decimal degree to degree minute second.
* @doc      converts decimal degree to degree minute second.
* @notes    None
* @author   Maher Salameh / July-27-2001 / maher.salameh@bentley.com
* @param    degree        <= degrees 
* @param    minute        <= minutes 
* @param    second        <= seconds
* @param    angleInDegree => angle in decimal degrees to be converted 
            to degree minute second
* @return   SUCCESS always.
* @version  $Header:
* @see      bcConversion_DMToDegree, bcConversion_DMSTodegree, 
            bcConversion_degreeToDM
*======================================================================*/
BENTLEYDTM_EXPORT int bcConversion_degreeToDMS
(
    int *degree,
    int *minute,
    double *second,
    double angleInDegree
);

/*=========================== HTML doc =============================*//**
* @memo     converts degree minute to decimal degrees.
* @doc      converts degree minute to decimal degrees.
* @notes    None
* @author   Maher Salameh / July-27-2001 / maher.salameh@bentley.com
* @param    angleInDegree <= angle converted to decimal degrees
* @param    degree        <= degrees
* @param    minute        <= minutes
* @return   angle converted from degree minute to decimal degrees
* @version  $Header:
* @see      bcConversion_degreeToDMS, bcConversion_degreeToDMS,
            bcConversion_DMSTodegree
*======================================================================*/
BENTLEYDTM_EXPORT double bcConversion_DMToDegree
(
    double *angleInDegree, 
    int degree,
    double minute
);

/*=========================== HTML doc =============================*//**
* @memo     converts degree minute second to decimal degrees.
* @doc      converts degree minute second to decimal degrees.
* @notes    None
* @author   Maher Salameh / July-27-2001 / maher.salameh@bentley.com
* @param    angleInDegree <= angle that we want to get
* @param    degree        => degrees
* @param    minute        => minutes
* @param    second        => seconds
* @return   angle converted from degree minute second to decimal degrees.
* @version  $Header:
* @see      bcConversion_degreeToDMS, bcConversion_degreeToDMS,
            bcConversion_DMToDegree
*==================================================================================*/
BENTLEYDTM_EXPORT double bcConversion_DMSTodegree
(
    double *angleInDegree,
    int degree,
    int minute,
    double second
);

/*=========================== HTML doc =============================*//**
* @memo     rounds a number by a certain increment, to a specific 
            direction.
* @doc      rounds a number by a certain increment
            to a specific direction, that is up or down, or to the
            nearest.
* @notes    None
* @author   Maher Salameh / July-27-2001 / maher.salameh@bentley.com
* @param    roundedValue  <= rounded value
* @param    value         => value to be rounded
* @param    increment     => rounding factor
* @param    method        => rounding direction: up, down or nearest
* @return   the rounded value.
* @version  $Header:
* @see      bcConversion_roundDoubleByDecimalDigits
*=============================================================================*/
BENTLEYDTM_EXPORT double bcConversion_roundDouble
(
    double *roundedValue,
    double value,
    double increment,
    int	method
);

/*=========================== HTML doc =============================*//**
* @memo     rounds a  decimal number by a certain factor, to a specific 
            direction.
* @doc      rounds a decimal number by a certain factor
            to a specific direction, that is up or down, or to the
            nearest.
* @notes    None
* @author   Maher Salameh / July-27-2001 / maher.salameh@bentley.com
* @param    roundedValue          <= rounded value that we want to get
* @param    value                 => value to be rounded
* @param    numberOfDecimalDigits => rounding factor of the decimal
* @param    method                => rounding direction: up, down or nearest
* @return   rounded number by numberOfDecimalDigits.
* @version  $Header:
* @see      bcConversion_roundDouble
*=============================================================================*/
BENTLEYDTM_EXPORT double bcConversion_roundDoubleByDecimalDigits
(
    double *roundedValue,
    double value,
    int    numberOfDecimalDigits,
    int	method
);

/*=============================== HTML doc ================================*//**
* @memo     converts US foot to meter.
* @doc      converts a linear unit US foot to meter.
* @notes    None
* @author   Maher Salameh / July-31-2001 / maher.salameh@bentley.com
* @param    valueInUsFoot     => value to be converted to meters
* @return   the converted number in meters.
* @version  $Header:
* @see      bcConversion_internationalFootToMeter, bcConversion_meterToUsFoot,
            bcConversion_meterToInternationalFoot,
*=============================================================================*/
BENTLEYDTM_EXPORT double bcConversion_usfootToMeter
(
double valueInUsFoot
);

/*=============================== HTML doc ================================*//**
* @memo     converts International foot to meter.
* @doc      converts a linear unit International foot to meter.
* @notes    None
* @author   Maher Salameh / July-31-2001 / maher.salameh@bentley.com
* @param    valueInInternationalFoot     => value to be converted to meters
* @return   converted value from International feet to meters.
* @version  $Header:
* @see      bcConversion_usfootToMeter, bcConversion_meterToUsFoot,
            bcConversion_meterToInternationalFoot,
*=============================================================================*/
BENTLEYDTM_EXPORT double bcConversion_internationalFootToMeter
(
    double valueInInternationalFoot
);

/*=============================== HTML doc ================================*//**
* @memo     converts meter to US foot.
* @doc      converts a linear unit meter to US foot.
* @notes    None
* @author   Maher Salameh / July-31-2001 / maher.salameh@bentley.com
* @param    valueInMeter     => value to be converted to US foot.
* @return   converted value from meter to US feet.
* @version  $Header:
* @see      bcConversion_usfootToMeter, bcConversion_meterToInternationalFoot, 
            bcConversion_internationalFootToMeter
*=============================================================================*/
BENTLEYDTM_EXPORT double bcConversion_meterToUsFoot
(
    double valueInMeter
);

/*=============================== HTML doc ================================*//**
* @memo     converts meter to International foot.
* @doc      converts a linear unit meter to International foot.
* @notes    None
* @author   Maher Salameh / July-31-2001 / maher.salameh@bentley.com
* @param    valueInMeter     => value to be converted to International foot.
* @return   converted value from meter to International feet.
* @version  $Header:
* @see      bcConversion_usfootToMeter, bcConversion_meterToUsFoot,
            bcConversion_internationalFootToMeter
*=============================================================================*/
BENTLEYDTM_EXPORT double bcConversion_meterToInternationalFoot
(
    double valueInMeter
);

/*=============================== HTML doc ================================*//**
* @memo     converts US to International foot.
* @doc      converts a linear unit US foot to International.
* @notes    None
* @author   Maher Salameh / July-31-2001 / maher.salameh@bentley.com
* @param    valueInUsFoot     => value to be converted to International foot.
* @return   converted value from US feet to International feet.
* @version  $Header:
* @see      bcConversion_internationalFootToUsFoot
*=============================================================================*/
BENTLEYDTM_EXPORT double bcConversion_usFootToInternationalFoot
(
double valueInUsFoot
);

/*=============================== HTML doc ================================*//**
* @memo     converts from International to US foot.
* @doc      converts a linear unit from International to US foot.
* @notes    None
* @author   Maher Salameh / July-31-2001 / maher.salameh@bentley.com
* @param    valueInUsFoot     => value to be converted to US foot.
* @return   converted value from International feet to US feet.
* @version  $Header:
* @see      bcConversion_usFootToInternationalFoot
*=============================================================================*/
BENTLEYDTM_EXPORT double bcConversion_internationalFootToUsFoot
(
    double valueInInternationalFoot
);

/*=============================== HTML doc ================================*//**
* @memo     converts radius to degree of curvature.
* @doc      converts radius to degree of curvature.
* @notes    None
* @author   Maher Salameh / July-31-2001 / maher.salameh@bentley.com
* @param    radius     => value to be converted to degrees.
* @return   value converted from radius to degrees.
* @version  $Header:
* @see      bcConversion_degreeToRadiusChord, bcConversion_degreeToRadius,
            bcConversion_radiusToDegreeChord,
*=============================================================================*/
BENTLEYDTM_EXPORT double bcConversion_radiusToDegree
(
	double radius
);

/*=============================== HTML doc ================================*//**
* @memo     converts chord to degree of curvature.
* @doc      converts chord to degree of curvature.
* @notes    None
* @author   Maher Salameh / July-31-2001 / maher.salameh@bentley.com
* @param    radius     => value to be converted to degree from chord.
* @return   value converted from radius chord to in degrees.
* @version  $Header:
* @see      bcConversion_degreeToRadiusChord, bcConversion_degreeToRadius,
            bcConversion_radiusToDegree,
*=============================================================================*/
BENTLEYDTM_EXPORT double bcConversion_radiusToDegreeChord
(
	double radius
);

/*=============================== HTML doc ================================*//**
* @memo     converts degree of curvature to radius.
* @doc      converts degree of curvature to radius.
* @notes    None
* @author   Maher Salameh / July-31-2001 / maher.salameh@bentley.com
* @param    radius     => value to be converted to radius.
* @return   value converted from degrees to radius.
* @version  $Header:
* @see      bcConversion_degreeToRadiusChord, bcConversion_radiusToDegreeChord,
            bcConversion_radiusToDegree,
*=============================================================================*/
BENTLEYDTM_EXPORT double bcConversion_degreeToRadius
(
	double degree 
);

/*=============================== HTML doc ================================*//**
* @memo     converts degree of curvature to radius.
* @doc      converts degree of curvature to radius.
* @notes    None
* @author   Maher Salameh / July-31-2001 / maher.salameh@bentley.com
* @param    radius     => The value to be converted to chord.
* @return   value converted from degree to chord.
* @version  $Header:
* @see      bcConversion_degreeToRadius, bcConversion_radiusToDegreeChord,
            bcConversion_radiusToDegree,
*=============================================================================*/
BENTLEYDTM_EXPORT double bcConversion_degreeToRadiusChord
(
	double degree 
);

/*=============================== HTML doc ================================*//**
* @memo     converts double to string.
* @doc      converts double to string.
* @notes    None
* @author   Vince Unrau / July-31-2001 / vince.unrauh@bentley.com
* @param    stringP,  <= Output String 
* @param    dValue,   => double Value 
* @param    precision => Number of Decimals 
* @version  $Header:
*=============================================================================*/
BENTLEYDTM_EXPORT void bcConversion_doubleToString
(
char   *stringP,  /* <= Output String */
double  dValue,   /* => double Value */
int     precision /* => Number of Decimals */
);

#undef dc_180overpi
#undef dc_piover2
#undef dc_2pi
#undef dc_zero

#endif