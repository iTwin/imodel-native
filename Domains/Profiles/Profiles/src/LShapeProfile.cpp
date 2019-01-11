/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/LShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <ProfilesInternal\ProfilesPrivateApi.h>
#include <Profiles\LShapeProfile.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
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
                                           double filletRadius, double edgeRadius, Angle const& legSlope)
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
bool LShapeProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isWidthValid = ProfilesProperty::IsGreaterThanZero (GetWidth());
    bool const isDepthValid = ProfilesProperty::IsGreaterThanZero (GetDepth());
    bool const isThicknessValid = ValidateThickness();
    bool const isFilletRadiusValid = ValidateFilletRadius();
    bool const isEdgeRadiusValid = ValidateEdgeRadius();
    bool const isLegSlopeValid  = ValidateLegSlope();

    return isWidthValid && isDepthValid && isThicknessValid && isFilletRadiusValid && isEdgeRadiusValid && isLegSlopeValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr LShapeProfile::_CreateGeometry() const
    {
    return ProfilesGeometry::CreateLShape (*this);
    }

/*---------------------------------------------------------------------------------**//**
* Prohibit deletion of this profile if it is referenced by any DoubleLShapeProfile.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LShapeProfile::_OnDelete() const
    {
    ECSqlStatement sqlStatement;
    Utf8CP pSqlString = "SELECT ECInstanceId FROM " PRF_SCHEMA (PRF_CLASS_DoubleLShapeProfile)
                        " WHERE " PRF_PROP_DoubleLShapeProfile_SingleProfile ".Id=? LIMIT 1";

    ECSqlStatus status = sqlStatement.Prepare (GetDgnDb(), pSqlString);
    if (status != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;

    status = sqlStatement.BindId (1, GetElementId());
    if (status != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;

    if (sqlStatement.Step() == DbResult::BE_SQLITE_ROW)
        {
        Utf8Char thisIdBuffer[DgnElementId::ID_STRINGBUFFER_LENGTH], otherIdBuffer[DgnElementId::ID_STRINGBUFFER_LENGTH];
        GetElementId().ToString (thisIdBuffer, BeInt64Id::UseHex::Yes);
        sqlStatement.GetValueId<DgnElementId> (0).ToString (otherIdBuffer, BeInt64Id::UseHex::Yes);

        PROFILES_LOG.errorv ("Failed to delete LShapeProfile instance (id: %s), because it is being referenced by DoubleLShapeProfile instance (id: %#010x).",
                             thisIdBuffer, otherIdBuffer);
        return DgnDbStatus::DeletionProhibited;
        }

    return T_Super::_OnDelete();
    }

/*---------------------------------------------------------------------------------**//**
* Update all DoubleLShapeProfiles that are referencing this profile.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LShapeProfile::_UpdateInDb()
    {
    ECSqlStatement sqlStatement;
    Utf8CP pSqlString = "SELECT ECInstanceId FROM " PRF_SCHEMA (PRF_CLASS_DoubleLShapeProfile)
                        " WHERE " PRF_PROP_DoubleLShapeProfile_SingleProfile ".Id=?";

    ECSqlStatus status = sqlStatement.Prepare (GetDgnDb(), pSqlString);
    if (status != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;

    status = sqlStatement.BindId (1, GetElementId());
    if (status != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;

    while (sqlStatement.Step() == DbResult::BE_SQLITE_ROW)
        {
        DgnElementId doubleProfileId = sqlStatement.GetValueId<DgnElementId> (0);
        DoubleLShapeProfilePtr doubleProfilePtr = m_dgndb.Elements().GetForEdit<DoubleLShapeProfile> (doubleProfileId);
        if (doubleProfilePtr.IsNull())
            {
            BeAssert (false && "Failed to get element");
            return DgnDbStatus::BadElement;
            }

        DgnDbStatus status;
        status = doubleProfilePtr->UpdateGeometry (*this);
        if (status != DgnDbStatus::Success)
            return status;

        doubleProfilePtr->Update (&status);
        if (status != DgnDbStatus::Success)
            return status;
        }

    return T_Super::_UpdateInDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool LShapeProfile::ValidateThickness() const
    {
    double const thickness = GetThickness();
    bool const isPositive = ProfilesProperty::IsGreaterThanZero (thickness);
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
    if (ProfilesProperty::IsEqualToZero (filletRadius))
        return true;

    double const availableWebLength = GetInnerWebFaceLength() / 2.0 - GetHorizontalLegSlopeHeight();
    double const availableFlangeLength = GetInnerFlangeFaceLength() / 2.0 - GetVerticalLegSlopeHeight();

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (filletRadius);
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
    if (ProfilesProperty::IsEqualToZero (edgeRadius))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (edgeRadius);
    bool const isLessThanHalfThickness = edgeRadius <= GetThickness() / 2.0;
    bool const isLessThanAvailableFlangeLength = edgeRadius <= GetInnerFlangeFaceLength() / 2.0;

    return isPositive && isLessThanHalfThickness && isLessThanAvailableFlangeLength;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool LShapeProfile::ValidateLegSlope() const
    {
    double const legSlope = GetLegSlope().Radians();
    if (ProfilesProperty::IsEqualToZero (legSlope))
        return true;

    bool const isPositive = ProfilesProperty::IsGreaterOrEqualToZero (legSlope);
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
void LShapeProfile::SetWidth (double value)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_Width, ECN::ECValue (value));
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
void LShapeProfile::SetDepth (double value)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_Depth, ECN::ECValue (value));
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
void LShapeProfile::SetThickness (double value)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_Thickness, ECN::ECValue (value));
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
void LShapeProfile::SetFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_FilletRadius, ECN::ECValue (value));
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
void LShapeProfile::SetEdgeRadius (double value)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_EdgeRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Angle LShapeProfile::GetLegSlope() const
    {
    return Angle::FromRadians (GetPropertyValueDouble (PRF_PROP_LShapeProfile_LegSlope));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LShapeProfile::SetLegSlope (Angle const& value)
    {
    SetPropertyValue (PRF_PROP_LShapeProfile_LegSlope, ECN::ECValue (value.Radians()));
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
    double const flangeSlopeCos = GetLegSlope().Cos();
    if (flangeSlopeCos <= DBL_EPSILON)
        return 0.0;

    return (GetInnerFlangeFaceLength() / flangeSlopeCos) * GetLegSlope().Sin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double LShapeProfile::GetVerticalLegSlopeHeight() const
    {
    double const webSlopeCos = GetLegSlope().Cos();
    if (webSlopeCos <= DBL_EPSILON)
        return 0.0;

    return (GetInnerWebFaceLength() / webSlopeCos) * GetLegSlope().Sin();
    }

END_BENTLEY_PROFILES_NAMESPACE
