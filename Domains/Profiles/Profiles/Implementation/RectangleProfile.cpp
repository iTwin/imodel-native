/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/RectangleProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\RectangleProfile.h>
#include <Profiles\ProfilesGeometry.h>
#include <Profiles\ProfilesProperty.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (RectangleProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RectangleProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RectangleProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double width, double depth)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , width (width)
    , depth (depth)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RectangleProfile::RectangleProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetWidth (params.width);
    SetDepth (params.depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RectangleProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isWidthValid = ProfilesProperty::IsGreaterThanZero (GetWidth());
    bool const isDepthValid = ProfilesProperty::IsGreaterThanZero (GetDepth());

    return isWidthValid && isDepthValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr RectangleProfile::_CreateGeometry() const
    {
    return ProfilesGeomApi::CreateRectangle (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double RectangleProfile::GetWidth() const
    {
    return GetPropertyValueDouble (PRF_PROP_RectangleProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RectangleProfile::SetWidth (double val)
    {
    SetPropertyValue (PRF_PROP_RectangleProfile_Width, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double RectangleProfile::GetDepth() const
    {
    return GetPropertyValueDouble (PRF_PROP_RectangleProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RectangleProfile::SetDepth (double val)
    {
    SetPropertyValue (PRF_PROP_RectangleProfile_Depth, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
