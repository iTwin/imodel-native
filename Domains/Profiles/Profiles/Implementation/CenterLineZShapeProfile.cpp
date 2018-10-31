/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/CenterLineZShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CenterLineZShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(CenterLineZShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineZShapeProfilePtr CenterLineZShapeProfile::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineZShapeProfile::SetFlangeWidth(double val)
    {
    SetPropertyValue(PRF_PROP_CenterLineZShapeProfile_FlangeWidth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineZShapeProfile::SetDepth(double val)
    {
    SetPropertyValue(PRF_PROP_CenterLineZShapeProfile_Depth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineZShapeProfile::SetFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_CenterLineZShapeProfile_FilletRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineZShapeProfile::SetLipLength(double val)
    {
    SetPropertyValue(PRF_PROP_CenterLineZShapeProfile_LipLength, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
