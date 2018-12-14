/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/LShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\LShapeProfile.h>
#include <Profiles\ProfilesGeometry.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (LShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double width, double depth, double thickness,
                                           double filletRadius, double edgeRadius, double legSlope)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , width (width)
    , depth (depth)
    , thickness (thickness)
    , filletRadius (filletRadius)
    , edgeRadius (edgeRadius)
    , legSlope (legSlope)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LShapeProfile::LShapeProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetWidth (params.width);
    SetDepth (params.depth);
    SetThickness (params.thickness);
    SetFilletRadius (params.filletRadius);
    SetEdgeRadius (params.edgeRadius);
    SetLegSlope (params.legSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LShapeProfile::_Validate() const
    {
    BentleyStatus status = T_Super::_Validate();
    if (status != BSISUCCESS)
        return status;

    bool const isWidthValid = ValidateWidth();
    bool const isDepthValid = ValidateDepth();
    bool const isThicknessValid = ValidateThickness();
    bool const isFilletRadiusValid = ValidateFilletRadius();
    bool const isEdgeRadiusValid = ValidateEdgeRadius();
    bool const isLegSlopeValid  = ValidateLegSlope();

    if (isWidthValid && isDepthValid && isThicknessValid && isFilletRadiusValid && isEdgeRadiusValid && isLegSlopeValid)
        return BSISUCCESS;

    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr LShapeProfile::_CreateGeometry() const
    {
    return ProfilesGeomApi::CreateLShape (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool LShapeProfile::ValidateWidth() const
    {
    double const width = GetWidth();
    return std::isfinite (width) && width > 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool LShapeProfile::ValidateDepth() const
    {
    double const depth = GetDepth();
    return std::isfinite (depth) && depth > 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool LShapeProfile::ValidateThickness() const
    {
    double const thickness = GetThickness();
    bool const isPositive = std::isfinite (thickness) && thickness > 0.0;
    bool const isLessThanWidth = thickness < GetWidth();
    bool const isLessThanDepth = thickness < GetDepth();

    return isPositive && isLessThanDepth && isLessThanWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool LShapeProfile::ValidateFilletRadius() const
    {
    double const filletRadius = GetFilletRadius();
    if (std::isfinite (filletRadius) && filletRadius == 0.0)
        return true;

    double const availableWebLength = GetInnerWebFaceLength() / 2.0 - GetHorizontalLegSlopeHeight();
    double const availableFlangeLength = GetInnerFlangeFaceLength() / 2.0 - GetVerticalLegSlopeHeight();

    bool const isPositive = std::isfinite (filletRadius) && filletRadius >= 0.0;
    bool const isLessThanAvailableWebLength = filletRadius <= availableWebLength;
    bool const isLessThanAvailableFlangeLength = filletRadius <= availableFlangeLength;

    return isPositive && isLessThanAvailableFlangeLength && isLessThanAvailableWebLength;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool LShapeProfile::ValidateEdgeRadius() const
    {
    double const edgeRadius = GetEdgeRadius();
    if (std::isfinite (edgeRadius) && edgeRadius == 0.0)
        return true;

    bool const isPositive = std::isfinite (edgeRadius) && edgeRadius >= 0.0;
    bool const isLessThanHalfThickness = edgeRadius <= GetThickness() / 2.0;
    bool const isLessThanAvailableFlangeLength = edgeRadius <= GetInnerFlangeFaceLength() / 2.0;

    return isPositive && isLessThanHalfThickness && isLessThanAvailableFlangeLength;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool LShapeProfile::ValidateLegSlope() const
    {
    double const legSlope = GetLegSlope();
    if (std::isfinite (legSlope) && legSlope == 0.0)
        return true;

    bool const isPositive = std::isfinite (legSlope) && legSlope >= 0.0;
    bool const isLessThan90Degrees = legSlope < PI / 2.0;
    bool const slopeHeightIsLessThanAvailableFlangeLength = GetVerticalLegSlopeHeight() <= GetInnerFlangeFaceLength() / 2.0;
    bool const slopeHeightIsLessThanAvailableWebLength = GetHorizontalLegSlopeHeight() <= GetInnerWebFaceLength() / 2.0;

    return isPositive && isLessThan90Degrees && slopeHeightIsLessThanAvailableFlangeLength && slopeHeightIsLessThanAvailableWebLength;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetWidth (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_Width, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_Depth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_Thickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetThickness (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_Thickness, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetFilletRadius (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_FilletRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_EdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetEdgeRadius (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_EdgeRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetLegSlope() const
    {
    return GetPropertyValueDouble (PRF_PROP_LShapeProfile_LegSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetLegSlope (double val)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_LegSlope, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetInnerFlangeFaceLength() const
    {
    return GetWidth() - GetThickness();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetInnerWebFaceLength() const
    {
    return GetDepth() - GetThickness();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetHorizontalLegSlopeHeight() const
    {
    double const flangeSlopeCos = std::cos (GetLegSlope());
    if (flangeSlopeCos <= DBL_EPSILON)
        return 0.0;

    return (GetInnerFlangeFaceLength() / flangeSlopeCos) * std::sin (GetLegSlope());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetVerticalLegSlopeHeight() const
    {
    double const webSlopeCos = std::cos (GetLegSlope());
    if (webSlopeCos <= DBL_EPSILON)
        return 0.0;

    return (GetInnerWebFaceLength() / webSlopeCos) * std::sin (GetLegSlope());
    }

END_BENTLEY_PROFILES_NAMESPACE
