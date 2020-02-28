/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/UnitsApi.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstring>
#include <cctype>

BEGIN_BUILDING_SHARED_NAMESPACE

static const double FPU = 3.28084;
static const double MPU = 1.0;
static const double M_PI = 3.14159265358979323846;

#define MAXLEN_ANGLE_STRING 100

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Wouter.Rombouts                 09/10
+---------------+---------------+---------------+---------------+---------------+------*/
double roundDouble(double x, int precision = 2)
    {
    x *= (10 * precision);
    if (x >= 0.0)
        {
        x += 0.5;
        return floor(x) / (10 * precision);;
        }

    x -= 0.5;

    double rounded = ceil(x);

    if (rounded == -0.0)
        {
        rounded = 0.0; //avoid  -0.0
        }

    rounded /= (10 * precision);;
    return rounded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::ToMeters
(
double value
)
    {
    return roundDouble(value*MPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::ToSquareMeters
(
double value
)
    {
    return roundDouble (value*MPU*MPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::ToCubicMeters
(
double value
)
    {
    return roundDouble (value*MPU*MPU*MPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::FromMeters
(
double value
)
    {
    return roundDouble (value/MPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::FromSquareMeters
(
double value
)
    {
    return roundDouble ((value / MPU) / MPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::FromCubicMeters
(
double value
)
    {
    return roundDouble (((value / MPU) / MPU) / MPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::ToFeet
(
double value
)
    {
    return roundDouble (value*FPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::ToSquareFeet
(
double value
)
    {
    return roundDouble (value*FPU*FPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::ToCubicFeet
(
double value
)
    {
    return roundDouble (value*FPU*FPU*FPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::FromFeet
(
double value
)
    {
    return value/FPU;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::FromSquareFeet
(
double value
)
    {
    return ((value/FPU)/FPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::FromCubicFeet
(
double value
)
    {
    return (((value/FPU)/FPU)/FPU);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String          UnitConverter::PrepareNumberForDisplay
(
double value
)
    {
    Utf8String resultString = Utf8PrintfString("%.2f", value);
    resultString = resultString.substr (resultString.size() - 3);
    int number = static_cast<int>(floor(value));
    int result = 0;
    bool skipped1st = false;
    while ((result = number % 1000) != 0 || number >= 1000)
        {
        if (skipped1st)
            resultString = Utf8PrintfString (",%s", resultString.c_str());
        else
            skipped1st = true;

        Utf8String tempResult = "";
        if (result != 0)
            tempResult = result > 99 || result == number ? Utf8PrintfString("%d", result) : result > 9 ? Utf8PrintfString("0%d", result) : Utf8PrintfString("00%d", result);
        else if (number >= 1000)
            tempResult = "000";

        resultString = Utf8PrintfString ("%s%s", tempResult.c_str(), resultString.c_str());
        number = number / 1000;
        }

    if (value < 1)
        resultString = Utf8PrintfString ("0%s", resultString.c_str());
    return resultString;
    }

StatusInt UnitConverter::MeetsAndBoundsStringToDouble(double& angle, Utf8CP string)
    {
    char bearing[MAXLEN_ANGLE_STRING];
PUSH_DISABLE_DEPRECATION_WARNINGS
    std::strncpy(bearing, string, std::strlen(string) + 1); //copying chars of string starting with string[1] ending with string[strlen(string)-2]
POP_DISABLE_DEPRECATION_WARNINGS

    //trim whitespaces
    while (0 != std::isspace(bearing[0]))
        for (int i = 0; i < strlen(bearing) - 2; ++i)
            bearing[i] = bearing[i + 1]; //trim leading whitespaces
    if (0 != bearing) //not all whitespaces
        {
        int firstTrailingWhitespacaIndex = 0;
        for (int i = static_cast<int>(strlen(bearing)) - 1; i > 0; --i)
            if (0 == std::isspace(bearing[i]))
                {
                firstTrailingWhitespacaIndex = i + 1;
                break;
                }
        if (std::isspace(bearing[firstTrailingWhitespacaIndex]))
            bearing[firstTrailingWhitespacaIndex] = '\0'; //trim ending whitespaces
        }

    char first = bearing[0];
    char last = bearing[std::strlen(bearing) - 1];

    if (('n' != first && 's' != first) || ('e' != last && 'w' != last) || strlen(bearing)<3)
        return BentleyStatus::ERROR;

    char angleString[MAXLEN_ANGLE_STRING];
PUSH_DISABLE_DEPRECATION_WARNINGS
    std::strncpy(angleString, bearing + 1, std::strlen(bearing) - 2); //copying chars of string starting with string[1] ending with string[strlen(string)-2]
POP_DISABLE_DEPRECATION_WARNINGS
    angleString[std::strlen(bearing) - 2] = '\0';

    char* converted;
    strtod(angleString, &converted);
    if ('\0' != *converted || converted == angleString)
        return BentleyStatus::ERROR;


	angle = std::stod(angleString); //convert bearing to double value

	//bearing
	double degrees = std::floor(angle);
	double minutes = std::floor((angle - degrees) * 100);
	double seconds = (angle - degrees - minutes / 100) * 10000;

	angle = degrees + minutes / 60 + seconds / 3600;
	angle = angle * M_PI / 180;

	if (angle > M_PI / 2 || angle < 0)
		return BentleyStatus::ERROR;

	if (std::isnan(angle))
		return BentleyStatus::ERROR;

    if ('n' == first && 'e' == last)
        {
        angle = M_PI / 2 - angle;
        return BentleyStatus::SUCCESS;
        }
    else if ('n' == first && 'w' == last)
        {
        angle = M_PI / 2 + angle;
        return BentleyStatus::SUCCESS;
        }
    else if ('s' == first && 'e' == last)
        {
        angle = 3 * M_PI / 2 + angle;
        return BentleyStatus::SUCCESS;
        }
    else if ('s' == first && 'w' == last)
        {
        angle = 3 * M_PI / 2 - angle;
        return BentleyStatus::SUCCESS;
        }

	return BentleyStatus::ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Haroldas.Vitunskas                 08/17
//---------------------------------------------------------------------------------------
void UnitConverter::DirectionToMeetsAndBoundsString(Utf8String & string, DVec3d direction)
    {
    double angle = DVec3d::From(1.0, 0.0, 0.0).AngleToXY(direction);
    angle = std::fmod(angle, (2 * M_PI));
    if (angle < 0)
        angle = (2 * M_PI) + angle;

    // Get the direction parts of string
    char first, last;

    if (angle >= 0 && angle < M_PI)
        first = 'n';
    else
        first = 's';

    if (angle >= M_PI / 2 && angle < 3 * M_PI / 2)
        last = 'w';
    else
        last = 'e';

    // Get the angle part of string
    angle = std::fmod(angle, (M_PI / 2));
    angle = angle * 180 / M_PI;

    // readjust angle to considering the first and last chars
    if (('n' == first && 'e' == last) || ('s' == first && 'w' == last))
        angle = 90 - angle;

    char buffer[20];
    std::sprintf(buffer, "%c%.2f%c\0", first, angle, last);

    string = buffer;
    }

END_BUILDING_SHARED_NAMESPACE

