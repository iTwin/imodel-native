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
