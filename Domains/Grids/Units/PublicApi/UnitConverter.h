/*--------------------------------------------------------------------------------------+
|
|     $Source: Units/PublicApi/UnitConverter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingMacros.h>
#include <DgnPlatform/DgnDb.h>
#define _USE_MATH_DEFINES
#include <cmath> 
#include <cstring>

BEGIN_BUILDING_NAMESPACE

struct UnitConverter
    {
    BUILDINGUNITS_EXPORT static double ToMeters (double value);
    BUILDINGUNITS_EXPORT static double ToSquareMeters (double value);
    BUILDINGUNITS_EXPORT static double ToCubicMeters (double value);

    BUILDINGUNITS_EXPORT static double FromMeters (double value);
    BUILDINGUNITS_EXPORT static double FromSquareMeters (double value);
    BUILDINGUNITS_EXPORT static double FromCubicMeters (double value);

    BUILDINGUNITS_EXPORT static double ToFeet (double value);
    BUILDINGUNITS_EXPORT static double ToSquareFeet (double value);
    BUILDINGUNITS_EXPORT static double ToCubicFeet (double value);

    BUILDINGUNITS_EXPORT static double FromFeet (double value);
    BUILDINGUNITS_EXPORT static double FromSquareFeet (double value);
    BUILDINGUNITS_EXPORT static double FromCubicFeet (double value);
    
    //should be in some utils thing..
    BUILDINGUNITS_EXPORT static Utf8String PrepareNumberForDisplay (double value);
    
	BUILDINGUNITS_EXPORT static StatusInt MeetsAndBoundsStringToDouble(double& angle, Utf8CP string);
};

BUILDINGUNITS_EXPORT void BuildingElement_notifyFail(Utf8CP pOperation, Dgn::DgnElement& elm, Dgn::DgnDbStatus* stat);

BUILDINGUNITS_EXPORT Dgn::RepositoryStatus BuildingLocks_LockElementForOperation (Dgn::DgnElementCR el, BeSQLite::DbOpcode op, Utf8CP pOperation);

END_BUILDING_NAMESPACE