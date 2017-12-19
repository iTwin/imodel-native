/*--------------------------------------------------------------------------------------+
|
|     $Source: Units/PublicApi/UnitConverter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingShared/BuildingSharedMacros.h>
#include <DgnPlatform/DgnDb.h>
#define _USE_MATH_DEFINES
#include <cmath> 
#include <cstring>

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

BUILDINGSHAREDUNITS_EXPORT void BuildingElement_notifyFail(Utf8CP pOperation, Dgn::DgnElement& elm, Dgn::DgnDbStatus* stat);

BUILDINGSHAREDUNITS_EXPORT Dgn::RepositoryStatus BuildingLocks_LockElementForOperation (Dgn::DgnElementCR el, BeSQLite::DbOpcode op, Utf8CP pOperation);

END_BUILDING_SHARED_NAMESPACE