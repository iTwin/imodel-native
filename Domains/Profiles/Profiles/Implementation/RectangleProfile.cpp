/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/RectangleProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\RectangleProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (RectangleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RectangleProfilePtr RectangleProfile::Create (DgnModelCR model)
    {
    CreateParams params (model.GetDgnDb(), model.GetModelId(), QueryClassId (model.GetDgnDb()));
    return new RectangleProfile (params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double RectangleProfile::GetWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_RectangleProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RectangleProfile::SetWidth (double val)
    {
    SetPropertyValue (PRF_PROP_RectangleProfile_Width, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double RectangleProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_RectangleProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RectangleProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_RectangleProfile_Depth, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
