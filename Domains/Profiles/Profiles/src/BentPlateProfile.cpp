/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ProfilesProperty.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <Profiles\BentPlateProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (BentPlateProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentPlateProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentPlateProfile::CreateParams::CreateParams (DefinitionModel const& model, Utf8CP pName, double width, double wallThickness,
                                              Angle const& bendAngle, double bendOffset, double filletRadius)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , width (width)
    , wallThickness (wallThickness)
    , bendAngle (bendAngle)
    , bendOffset (bendOffset)
    , filletRadius (filletRadius)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentPlateProfile::BentPlateProfile (CreateParams const& params)
     : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetWidth (params.width);
    SetWallThickness (params.wallThickness);
    SetBendAngle (params.bendAngle);
    SetBendOffset (params.bendOffset);
    SetFilletRadius (params.filletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool BentPlateProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isWidthValid = ProfilesProperty::IsGreaterThanZero (GetWidth());
    bool const isBendAngleValid = ValidateBendAngle();
    bool const isBendOffsetValid = ValidateBendOffset();
    bool const isWallThicknessValid = ValidateWallThickness();
    bool const isFilletRadiusValid = ValidateFilletRadius();

    return isWidthValid && isBendAngleValid && isBendOffsetValid && isWallThicknessValid && isFilletRadiusValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool BentPlateProfile::ValidateWallThickness() const
    {
    double const wallThickness = GetWallThickness();

    bool const isPositive = ProfilesProperty::IsGreaterThanZero (wallThickness);
    bool const isLessThanMaxThickness = ProfilesProperty::IsLess (wallThickness, CalculateMaxWallThickness());

    return isPositive && isLessThanMaxThickness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool BentPlateProfile::ValidateBendAngle() const
    {
    double const angle = GetBendAngle().Radians();

    bool const isGreaterThanZero = ProfilesProperty::IsGreaterThanZero (angle);
    bool const isLessThanPi = ProfilesProperty::IsLess (angle, PI);

    return isGreaterThanZero && isLessThanPi;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool BentPlateProfile::ValidateBendOffset() const
    {
    double const bendOffset = GetBendOffset();

    bool const isGreaterThanZero = ProfilesProperty::IsGreaterThanZero (bendOffset);
    bool const isLessThanWidth = ProfilesProperty::IsLess (bendOffset, GetWidth());

    return isGreaterThanZero && isLessThanWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool BentPlateProfile::ValidateFilletRadius() const
    {
    double const filletRadius = GetFilletRadius();
    if (ProfilesProperty::IsEqualToZero (filletRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (filletRadius);
    bool const fitsInRemainingWallThickness = ProfilesProperty::IsLessOrEqual (filletRadius, CalculateMaxWallThickness() / 2.0 - GetWallThickness() / 2.0);

    return isPositive && fitsInRemainingWallThickness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double BentPlateProfile::CalculateMaxWallThickness() const
    {
    double const shortLegLength = std::min (GetWidth() - GetBendOffset(), GetBendOffset());
    Angle const halfAngle = GetBendAngle() * 0.5;

    if (BeNumerical::IsLessOrEqualToZero (halfAngle.Cos()))
        return 0.0;

    // Calculated wallThickness is maximum for the inner side of the bent plate.
    // Multiply result by 2.0 because the wall thickness is aplied in both ways from the CenterLine
    return (shortLegLength / halfAngle.Cos()) * halfAngle.Sin() * 2.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool BentPlateProfile::_CreateGeometry()
    {
    IGeometryPtr centerLineGeometryPtr = ProfilesGeometry::CreateBentPlateCenterLine (*this);
    if (centerLineGeometryPtr.IsNull())
        return false;
    SetCenterLine (*centerLineGeometryPtr);

    return T_Super::_CreateGeometry();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr BentPlateProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateBentPlateShape (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double BentPlateProfile::GetWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_BentPlateProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void BentPlateProfile::SetWidth (double value)
    {
    SetPropertyValue (PRF_PROP_BentPlateProfile_Width, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Angle BentPlateProfile::GetBendAngle() const
    {
    return Angle::FromRadians (GetPropertyValueDouble (PRF_PROP_BentPlateProfile_BendAngle));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void BentPlateProfile::SetBendAngle (Angle const& value)
    {
    SetPropertyValue (PRF_PROP_BentPlateProfile_BendAngle, ECValue (value.Radians()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double BentPlateProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_BentPlateProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void BentPlateProfile::SetFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_BentPlateProfile_FilletRadius, ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double BentPlateProfile::GetBendOffset() const
    {
    return GetPropertyValueDouble (PRF_PROP_BentPlateProfile_BendOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void BentPlateProfile::SetBendOffset (double value)
    {
    SetPropertyValue (PRF_PROP_BentPlateProfile_BendOffset, ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
