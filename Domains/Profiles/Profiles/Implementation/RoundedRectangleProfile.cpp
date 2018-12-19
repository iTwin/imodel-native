/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/RoundedRectangleProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\RoundedRectangleProfile.h>
#include <Profiles\ProfilesGeometry.h>

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
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RoundedRectangleProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isWidthValid = ValidateWidth();
    bool const isDepthValid = ValidateDepth();
    bool const isRoundingRadiusValid = ValidateRoundingRadius();

    return isWidthValid && isDepthValid && isRoundingRadiusValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr RoundedRectangleProfile::_CreateGeometry() const
    {
    return ProfilesGeomApi::CreateRoundedRectangle (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RoundedRectangleProfile::ValidateWidth() const
    {
    double const width = GetWidth();

    return std::isfinite (width) && width > 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RoundedRectangleProfile::ValidateDepth() const
    {
    double const depth = GetDepth();

    return std::isfinite (depth) && depth > 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RoundedRectangleProfile::ValidateRoundingRadius() const
    {
    double const roundingRadius = GetRoundingRadius();
    bool const isPositive = std::isfinite (roundingRadius) && roundingRadius > 0.0;
    bool const isLessOrEqualToHalfWidth = roundingRadius <= GetWidth() / 2.0;
    bool const isLessOrEqualToHalfDepth = roundingRadius <= GetDepth() / 2.0;

    return isPositive && isLessOrEqualToHalfWidth && isLessOrEqualToHalfDepth;
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
