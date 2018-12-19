/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/HollowRectangleProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\HollowRectangleProfile.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (HollowRectangleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HollowRectangleProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HollowRectangleProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double width, double depth, double wallThickness,
                                                    double innerFilletRadius, double outerFilletRadius)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , width (width)
    , depth (depth)
    , wallThickness (wallThickness)
    , innerFilletRadius (innerFilletRadius)
    , outerFilletRadius (outerFilletRadius)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HollowRectangleProfile::HollowRectangleProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetWidth (params.width);
    SetDepth (params.depth);
    SetWallThickness (params.wallThickness);
    SetInnerFilletRadius (params.innerFilletRadius);
    SetOuterFilletRadius (params.outerFilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool HollowRectangleProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isWidthValid = ValidateWidth();
    bool const isDepthValid = ValidateDepth();
    bool const isWallThicknessValid = ValidateWallThickness();
    bool const isInnerFilletRadiusValid = ValidateInnerFilletRadius();
    bool const isOuterFilletRadiusValid = ValidateOuterFilletRadius();

    return isWidthValid && isDepthValid && isWallThicknessValid && isInnerFilletRadiusValid && isOuterFilletRadiusValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool HollowRectangleProfile::ValidateWidth() const
    {
    double const width = GetWidth();

    return std::isfinite (width) && width > 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool HollowRectangleProfile::ValidateDepth() const
    {
    double const depth = GetDepth();

    return std::isfinite (depth) && depth > 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool HollowRectangleProfile::ValidateWallThickness() const
    {
    double const wallThickness = GetWallThickness();
    bool const isPositive = std::isfinite (wallThickness) && wallThickness > 0.0;
    bool const isLessThanHalfWidth = wallThickness < GetWidth() / 2.0;
    bool const isLessThanHalfDepth = wallThickness < GetDepth() / 2.0;

    return isPositive && isLessThanHalfWidth && isLessThanHalfDepth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool HollowRectangleProfile::ValidateInnerFilletRadius() const
    {
    double const innerRadius = GetInnerFilletRadius();
    if (std::isfinite (innerRadius) && innerRadius == 0.0)
        return true;

    bool const isPositive = std::isfinite (innerRadius) && innerRadius >= 0.0;
    bool const fitsInWidth = innerRadius < GetWidth() / 2.0 - GetWallThickness();
    bool const fitsInDepth = innerRadius < GetDepth() / 2.0 - GetWallThickness();

    return isPositive && fitsInWidth && fitsInDepth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool HollowRectangleProfile::ValidateOuterFilletRadius() const
    {
    double const outerRadius = GetOuterFilletRadius();
    if (std::isfinite (outerRadius) && outerRadius == 0.0)
        return true;

    bool const isPositive = std::isfinite (outerRadius) && outerRadius >= 0.0;
    bool const fitsInWidth = outerRadius < GetWidth() / 2.0;
    bool const fitsInDepth = outerRadius < GetDepth() / 2.0;
    bool const doesNotIntersectWithInnerCorners = outerRadius - GetInnerFilletRadius() < (2 + std::sqrt (2.0)) * GetWallThickness();

    return isPositive && fitsInWidth && fitsInDepth && doesNotIntersectWithInnerCorners;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowRectangleProfile::GetWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_HollowRectangleProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowRectangleProfile::SetWidth (double val)
    {
    SetPropertyValue (PRF_PROP_HollowRectangleProfile_Width, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowRectangleProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_HollowRectangleProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowRectangleProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_HollowRectangleProfile_Depth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowRectangleProfile::GetInnerFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_HollowRectangleProfile_InnerFilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowRectangleProfile::SetInnerFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_HollowRectangleProfile_InnerFilletRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double HollowRectangleProfile::GetOuterFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_HollowRectangleProfile_OuterFilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HollowRectangleProfile::SetOuterFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_HollowRectangleProfile_OuterFilletRadius, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
