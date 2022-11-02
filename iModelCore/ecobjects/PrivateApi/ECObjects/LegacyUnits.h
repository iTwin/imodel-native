/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECSchema.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//=======================================================================================
//! This class Gets V8i/EC2 based unit names and format string from the old units custom attributes.  
//! It is only intended for use during Schema conversion from EC2 to EC 3
//! @bsiclass
//=======================================================================================
struct LegacyUnits
{
ECOBJECTS_EXPORT static bool GetUnitForECProperty (Utf8StringR unitName, ECPropertyCR ecProp);
ECOBJECTS_EXPORT static bool GetDisplayUnitAndFormatForECProperty (Utf8StringR displayUnitName, Utf8StringR displayFormat, Utf8StringCR storedUnitName, ECPropertyCR ecProp);
};

END_BENTLEY_ECOBJECT_NAMESPACE
