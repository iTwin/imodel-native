/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/HollowRectangleProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\HollowRectangleProfile.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesProperty.h>

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

    bool const isWidthValid = ProfilesProperty::IsGreaterThanZero (GetWidth());
    bool const isDepthValid = ProfilesProperty::IsGreaterThanZero (GetDepth());
    bool const isWallThicknessValid = ValidateWallThickness();
    bool const isInnerFilletRadiusValid = ValidateInnerFilletRadius();
    bool const isOuterFilletRadiusValid = ValidateOuterFilletRadius();

    return isWidthValid && isDepthValid && isWallThicknessValid && isInnerFilletRadiusValid && isOuterFilletRadiusValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr HollowRectangleProfile::_CreateGeometry() const
    {
    return ProfilesGeometry::CreateHollowRectangle (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool HollowRectangleProfile::ValidateWallThickness() const
    {
    double const wallThickness = GetWallThickness();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (wallThickness);
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
    if (ProfilesProperty::IsEqualToZero (innerRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (innerRadius);
    bool const fitsInWidth = innerRadius <= GetWidth() / 2.0 - GetWallThickness();
    bool const fitsInDepth = innerRadius <= GetDepth() / 2.0 - GetWallThickness();

    return isPositive && fitsInWidth && fitsInDepth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool HollowRectangleProfile::ValidateOuterFilletRadius() const
    {
    double const outerRadius = GetOuterFilletRadius();
    if (ProfilesProperty::IsEqualToZero (outerRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (outerRadius);
    bool const fitsInWidth = outerRadius <= GetWidth() / 2.0;
    bool const fitsInDepth = outerRadius <= GetDepth() / 2.0;
    bool const doesNotIntersectWithInnerCorners = outerRadius - GetInnerFilletRadius() < (2.0 + std::sqrt (2.0)) * GetWallThickness();

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
void HollowRectangleProfile::SetWidth (double value)
    {
    SetPropertyValue (PRF_PROP_HollowRectangleProfile_Width, ECN::ECValue (value));
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
void HollowRectangleProfile::SetDepth (double value)
    {
    SetPropertyValue (PRF_PROP_HollowRectangleProfile_Depth, ECN::ECValue (value));
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
void HollowRectangleProfile::SetInnerFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_HollowRectangleProfile_InnerFilletRadius, ECN::ECValue (value));
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
void HollowRectangleProfile::SetOuterFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_HollowRectangleProfile_OuterFilletRadius, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
