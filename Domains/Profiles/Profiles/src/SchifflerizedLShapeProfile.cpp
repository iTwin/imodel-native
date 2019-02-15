/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/SchifflerizedLShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ProfilesProperty.h>
#include <ProfilesInternal\ProfilesGeometry.h>
#include <Profiles\SchifflerizedLShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (SchifflerizedLShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SchifflerizedLShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SchifflerizedLShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double legLength, double thickness,
                                                        double legBendOffset, double filletRadius, double edgeRadius)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , legLength (legLength)
    , thickness (thickness)
    , legBendOffset (legBendOffset)
    , filletRadius (filletRadius)
    , edgeRadius (edgeRadius)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SchifflerizedLShapeProfile::SchifflerizedLShapeProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetLegLength (params.legLength);
    SetThickness (params.thickness);
    SetLegBendOffset (params.legBendOffset);
    SetFilletRadius (params.filletRadius);
    SetEdgeRadius (params.edgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchifflerizedLShapeProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isLegLengthValid = ProfilesProperty::IsGreaterThanZero (GetLegLength());
    bool const isThicknessValid = ValidateThickness();
    bool const isLegBendOffsetValid = ValidateLegBendOffset();
    bool const isFilletRadiusValid = ValidateFilletRadius();
    bool const isEdgeRadiusValid = ValidateEdgeRadius();

    return isLegLengthValid && isThicknessValid && isLegBendOffsetValid && isFilletRadiusValid && isEdgeRadiusValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr SchifflerizedLShapeProfile::_CreateShapeGeometry() const
    {
    return ProfilesGeometry::CreateSchifflerizedLShape (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchifflerizedLShapeProfile::ValidateThickness() const
    {
    double const thickness = GetThickness();
    bool const isPossitive = ProfilesProperty::IsGreaterThanZero (thickness);
    // Maximum thickness is calculated as if LegBendOffset is 0
    bool const fitsBetweenLegs = ProfilesProperty::IsLess (thickness, GetLegLength() / std::sqrt (3.0));

    return isPossitive && fitsBetweenLegs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchifflerizedLShapeProfile::ValidateLegBendOffset() const
    {
    double const legBendOffset = GetLegBendOffset();
    bool const isPossitive = ProfilesProperty::IsGreaterThanZero (legBendOffset);
    bool const isGreaterThanThickness = ProfilesProperty::IsGreaterOrEqual (legBendOffset, GetThickness());

    return isPossitive && isGreaterThanThickness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchifflerizedLShapeProfile::ValidateFilletRadius() const
    {
    double const filletRadius = GetFilletRadius();
    if (ProfilesProperty::IsEqualToZero (filletRadius))
        return true;

    double const availableFilletSpace = GetLegBendOffset() - GetThickness();

    bool const isPossitive = ProfilesProperty::IsGreaterOrEqualToZero (filletRadius);
    bool const fitsInAvailableSpace = ProfilesProperty::IsLessOrEqual (filletRadius, availableFilletSpace);

    return isPossitive && fitsInAvailableSpace;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchifflerizedLShapeProfile::ValidateEdgeRadius() const
    {
    double const edgeRadius = GetEdgeRadius();
    if (ProfilesProperty::IsEqualToZero (edgeRadius))
        return true;

    double const bentLegLength = GetLegLength() - GetLegBendOffset();

    bool const isPossitive = ProfilesProperty::IsGreaterOrEqualToZero (edgeRadius);
    bool const fitsInLegLength = ProfilesProperty::IsLessOrEqual (edgeRadius, bentLegLength);
    bool const fitsInThickness = ProfilesProperty::IsLessOrEqual (edgeRadius, GetThickness());

    return isPossitive && fitsInLegLength && fitsInThickness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetLegLength() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_LegLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetLegLength (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_LegLength, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_Thickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetThickness (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_Thickness, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetLegBendOffset() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_LegBendOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetLegBendOffset (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_LegBendOffset, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_FilletRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_EdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetEdgeRadius (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_EdgeRadius, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
