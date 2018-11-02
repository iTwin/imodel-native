/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/CustomShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CustomShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(CustomShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CustomShapeProfilePtr CustomShapeProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr CustomShapeProfile::GetOuterCurve() const
    {
    ECN::ECValue ecValue;
    GetPropertyValue(ecValue, PRF_PROP_CustomShapeProfile_OuterCurve);
    return ecValue.GetIGeometry();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomShapeProfile::SetOuterCurve(IGeometryPtr val)
    {
    ECN::ECValue ecValue;
    ecValue.SetIGeometry(*val);
    SetPropertyValue(PRF_PROP_CustomShapeProfile_OuterCurve, ecValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr CustomShapeProfile::GetInnerCurves() const
    {
    ECN::ECValue ecValue;
    GetPropertyValue(ecValue, PRF_PROP_CustomShapeProfile_InnerCurves);
    return ecValue.GetIGeometry();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomShapeProfile::SetInnerCurves(IGeometryPtr val)
    {
    ECN::ECValue ecValue;
    ecValue.SetIGeometry(*val);
    SetPropertyValue(PRF_PROP_CustomShapeProfile_InnerCurves, ecValue);
    }

END_BENTLEY_PROFILES_NAMESPACE
