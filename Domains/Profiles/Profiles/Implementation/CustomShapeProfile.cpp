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
void CustomShapeProfile::SetOuterCurve(IGeometryPtr val)
    {
    //SetPropertyValue(PRF_PROP_CustomShapeProfile_OuterCurve, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomShapeProfile::SetInnerCurves(IGeometryPtr val)
    {
    //SetPropertyValue(PRF_PROP_CustomShapeProfile_InnerCurves, val);
    }

END_BENTLEY_PROFILES_NAMESPACE
