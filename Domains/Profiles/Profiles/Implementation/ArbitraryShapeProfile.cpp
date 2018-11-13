/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/ArbitraryShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\ArbitraryShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(ArbitraryShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryShapeProfilePtr ArbitraryShapeProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ArbitraryShapeProfile::GetOuterCurve() const
    {
    ECN::ECValue ecValue;
    GetPropertyValue(ecValue, PRF_PROP_ArbitraryShapeProfile_OuterCurve);
    return ecValue.GetIGeometry();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryShapeProfile::SetOuterCurve(IGeometryPtr val)
    {
    ECN::ECValue ecValue;
    ecValue.SetIGeometry(*val);
    SetPropertyValue(PRF_PROP_ArbitraryShapeProfile_OuterCurve, ecValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ArbitraryShapeProfile::GetInnerCurves() const
    {
    ECN::ECValue ecValue;
    GetPropertyValue(ecValue, PRF_PROP_ArbitraryShapeProfile_InnerCurves);
    return ecValue.GetIGeometry();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryShapeProfile::SetInnerCurves(IGeometryPtr val)
    {
    ECN::ECValue ecValue;
    ecValue.SetIGeometry(*val);
    SetPropertyValue(PRF_PROP_ArbitraryShapeProfile_InnerCurves, ecValue);
    }

END_BENTLEY_PROFILES_NAMESPACE
