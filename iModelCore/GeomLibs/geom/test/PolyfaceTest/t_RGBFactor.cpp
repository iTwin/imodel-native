/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/test/PolyfaceTest/t_RGBFactor.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>
#include <Bentley/BeConsole.h>


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RgbFactor,Methods)
    {
    double s = 255.0;
    auto rgbA = RgbFactor::From (10.0 / s, 12.0 / s, 38.0/ s);
    double scale = 2;
    auto rgbB = RgbFactor::From (rgbA.red * scale, rgbA.green * scale, rgbA.blue * scale);
    auto rgbA2 = rgbA;
    rgbA2.ScaleInPlace (scale);
    Check::True (rgbA2.EqualsInt (rgbB));
    Check::False (rgbA2.Equals (rgbA));
    int colorA = rgbA.ToIntColor ();
    auto rgbC = RgbFactor::FromIntColor (colorA);
    Check::True (rgbC.Equals (rgbA));
    auto shift = DPoint3d::From (1,2,3);
    auto rgbShift = RgbFactor::From (shift);

    auto rgbD = rgbC;
    Check::True (rgbC.Equals (rgbD));
    rgbC.AddInPlace (rgbShift);
    Check::False (rgbC.Equals (rgbD));
    shift.Scale (-1.0);
    rgbC.AddInPlace (RgbFactor::From (shift));
    Check::True (rgbC.EqualsInt (rgbD));



    }