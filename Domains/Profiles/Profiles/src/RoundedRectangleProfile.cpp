/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <Profiles\RoundedRectangleProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (RoundedRectangleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RoundedRectangleProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RoundedRectangleProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName, double width, double depth, double roundingRadius)
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
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RoundedRectangleProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isWidthValid = ProfilesProperty::IsGreaterThanZero (GetWidth());
    bool const isDepthValid = ProfilesProperty::IsGreaterThanZero (GetDepth());
    bool const isRoundingRadiusValid = ValidateRoundingRadius();

    return isWidthValid && isDepthValid && isRoundingRadiusValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr RoundedRectangleProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateRoundedRectangle (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RoundedRectangleProfile::ValidateRoundingRadius() const
    {
    double const roundingRadius = GetRoundingRadius();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (roundingRadius);
    bool const isLessThanHalfWidth = ProfilesProperty::IsLess (roundingRadius, GetWidth() / 2.0);
    bool const isLessThanHalfDepth = ProfilesProperty::IsLess (roundingRadius, GetDepth() / 2.0);
    // Note: use a CapsuleProfile if you want "fully" rounded corners

    return isPositive && isLessThanHalfWidth && isLessThanHalfDepth;
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
void RoundedRectangleProfile::SetWidth (double value)
    {
    SetPropertyValue (PRF_PROP_RoundedRectangleProfile_Width, ECN::ECValue (value));
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
void RoundedRectangleProfile::SetDepth (double value)
    {
    SetPropertyValue (PRF_PROP_RoundedRectangleProfile_Depth, ECN::ECValue (value));
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
void RoundedRectangleProfile::SetRoundingRadius (double value)
    {
    SetPropertyValue (PRF_PROP_RoundedRectangleProfile_RoundingRadius, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
