/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/ICenterLineProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\ICenterLineProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ICenterLineProfile::GetCenterLine() const
    {
    ECN::ECValue ecValue;
    (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValue(ecValue, PRF_PROP_ICenterLineProfile_CenterLine);
    return ecValue.GetIGeometry();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ICenterLineProfile::SetCenterLine(IGeometryPtr val)
    {
    ECN::ECValue ecValue;
    ecValue.SetIGeometry(*val);
    (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_ICenterLineProfile_CenterLine, ecValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ICenterLineProfile::GetWallThickness() const
    {
    return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_ICenterLineProfile_WallThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ICenterLineProfile::SetWallThickness(double val)
    {
    (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_ICenterLineProfile_WallThickness, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
