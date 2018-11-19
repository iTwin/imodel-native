/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/CenterLineZShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\CenterLineZShapeProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (CenterLineZShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CenterLineZShapeProfilePtr CenterLineZShapeProfile::Create (DgnModelCR model)
    {
    CreateParams params (model.GetDgnDb(), model.GetModelId(), QueryClassId (model.GetDgnDb()));
    return new CenterLineZShapeProfile (params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineZShapeProfile::GetFlangeWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineZShapeProfile_FlangeWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineZShapeProfile::SetFlangeWidth (double val)
    {
    SetPropertyValue (PRF_PROP_CenterLineZShapeProfile_FlangeWidth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineZShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineZShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineZShapeProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_CenterLineZShapeProfile_Depth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineZShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineZShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineZShapeProfile::SetFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_CenterLineZShapeProfile_FilletRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double CenterLineZShapeProfile::GetGirth() const
    {
    return GetPropertyValueDouble (PRF_PROP_CenterLineZShapeProfile_Girth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CenterLineZShapeProfile::SetGirth (double val)
    {
    SetPropertyValue (PRF_PROP_CenterLineZShapeProfile_Girth, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
