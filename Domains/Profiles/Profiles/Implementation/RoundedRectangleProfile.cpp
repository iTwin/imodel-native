/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/RoundedRectangleProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\RoundedRectangleProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (RoundedRectangleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RoundedRectangleProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RoundedRectangleProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double width, double depth, double roundingRadius)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , width (width)
    , depth (depth)
    , roundingRadius (roundingRadius)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RoundedRectangleProfile::RoundedRectangleProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetWidth (params.width);
    SetDepth (params.depth);
    SetRoundingRadius (params.roundingRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double RoundedRectangleProfile::GetWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_RoundedRectangleProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RoundedRectangleProfile::SetWidth (double val)
    {
    SetPropertyValue (PRF_PROP_RoundedRectangleProfile_Width, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double RoundedRectangleProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_RoundedRectangleProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RoundedRectangleProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_RoundedRectangleProfile_Depth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double RoundedRectangleProfile::GetRoundingRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_RoundedRectangleProfile_RoundingRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RoundedRectangleProfile::SetRoundingRadius (double val)
    {
    SetPropertyValue (PRF_PROP_RoundedRectangleProfile_RoundingRadius, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
