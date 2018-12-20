/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/TrapeziumProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\TrapeziumProfile.h>
#include <Profiles\ProfilesGeometry.h>
#include <Profiles\ProfilesProperty.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (TrapeziumProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TrapeziumProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TrapeziumProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double topWidth, double bottomWidth, double depth, double topOffset)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , topWidth (topWidth)
    , bottomWidth (bottomWidth)
    , depth (depth)
    , topOffset (topOffset)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TrapeziumProfile::TrapeziumProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetTopWidth (params.topWidth);
    SetBottomWidth (params.bottomWidth);
    SetDepth (params.depth);
    SetTopOffset (params.topOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool TrapeziumProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isTopWidthValid = ProfilesProperty::IsGreaterThanZero (GetTopWidth());
    bool const isBottomWidthValid = ProfilesProperty::IsGreaterThanZero (GetBottomWidth());
    bool const isDepthValid = ProfilesProperty::IsGreaterThanZero (GetDepth());
    bool const isTopOffsetValid = BeNumerical::BeFinite (GetTopOffset());

    return isTopWidthValid && isBottomWidthValid && isDepthValid && isTopOffsetValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr TrapeziumProfile::_CreateGeometry() const
    {
    return ProfilesGeomApi::CreateTrapezium (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TrapeziumProfile::GetTopWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_TrapeziumProfile_TopWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TrapeziumProfile::SetTopWidth (double val)
    {
    SetPropertyValue (PRF_PROP_TrapeziumProfile_TopWidth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TrapeziumProfile::GetBottomWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_TrapeziumProfile_BottomWidth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TrapeziumProfile::SetBottomWidth (double val)
    {
    SetPropertyValue (PRF_PROP_TrapeziumProfile_BottomWidth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TrapeziumProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_TrapeziumProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TrapeziumProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_TrapeziumProfile_Depth, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double TrapeziumProfile::GetTopOffset() const
    {
    return GetPropertyValueDouble (PRF_PROP_TrapeziumProfile_TopOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TrapeziumProfile::SetTopOffset (double val)
    {
    SetPropertyValue (PRF_PROP_TrapeziumProfile_TopOffset, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
