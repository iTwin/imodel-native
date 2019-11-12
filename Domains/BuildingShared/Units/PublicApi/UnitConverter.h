/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

struct UnitConverter
    {
    BUILDINGSHAREDUNITS_EXPORT static double ToMeters (double value);
    BUILDINGSHAREDUNITS_EXPORT static double ToSquareMeters (double value);
    BUILDINGSHAREDUNITS_EXPORT static double ToCubicMeters (double value);

    BUILDINGSHAREDUNITS_EXPORT static double FromMeters (double value);
    BUILDINGSHAREDUNITS_EXPORT static double FromSquareMeters (double value);
    BUILDINGSHAREDUNITS_EXPORT static double FromCubicMeters (double value);

    BUILDINGSHAREDUNITS_EXPORT static double ToFeet (double value);
    BUILDINGSHAREDUNITS_EXPORT static double ToSquareFeet (double value);
    BUILDINGSHAREDUNITS_EXPORT static double ToCubicFeet (double value);

    BUILDINGSHAREDUNITS_EXPORT static double FromFeet (double value);
    BUILDINGSHAREDUNITS_EXPORT static double FromSquareFeet (double value);
    BUILDINGSHAREDUNITS_EXPORT static double FromCubicFeet (double value);
    
    //should be in some utils thing..
    BUILDINGSHAREDUNITS_EXPORT static Utf8String PrepareNumberForDisplay (double value);
    
    BUILDINGSHAREDUNITS_EXPORT static StatusInt MeetsAndBoundsStringToDouble(double& angle, Utf8CP string);
    BUILDINGSHAREDUNITS_EXPORT static void DirectionToMeetsAndBoundsString(Utf8String& string, DVec3d direction);
};

END_BUILDING_SHARED_NAMESPACE