/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/IShapeProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\IShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS(IShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IShapeProfilePtr IShapeProfile::Create(Dgn::DgnDbR db, Dgn::DgnModelId const& modelId, Dgn::DgnClassId const& classId)
    {
    return new IShapeProfile(CreateParams(db, modelId, classId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetWidth() const
    {
    return GetPropertyValueDouble(PRF_PROP_IShapeProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetWidth(double val)
    {
    SetPropertyValue(PRF_PROP_IShapeProfile_Width, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetDepth() const
    {
    return GetPropertyValueDouble(PRF_PROP_IShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetDepth(double val)
    {
    SetPropertyValue(PRF_PROP_IShapeProfile_Depth, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetFlangeThickness() const
    {
    return GetPropertyValueDouble(PRF_PROP_IShapeProfile_FlangeThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetFlangeThickness(double val)
    {
    SetPropertyValue(PRF_PROP_IShapeProfile_FlangeThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetWebThickness() const
    {
    return GetPropertyValueDouble(PRF_PROP_IShapeProfile_WebThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetWebThickness(double val)
    {
    SetPropertyValue(PRF_PROP_IShapeProfile_WebThickness, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_IShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetFilletRadius(double val)
    {
    SetPropertyValue(PRF_PROP_IShapeProfile_FilletRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetFlangeEdgeRadius() const
    {
    return GetPropertyValueDouble(PRF_PROP_IShapeProfile_FlangeEdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetFlangeEdgeRadius(double val)
    {
    SetPropertyValue(PRF_PROP_IShapeProfile_FlangeEdgeRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IShapeProfile::GetFlangeSlope() const
    {
    return GetPropertyValueDouble(PRF_PROP_IShapeProfile_FlangeSlope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IShapeProfile::SetFlangeSlope(double val)
    {
    SetPropertyValue(PRF_PROP_IShapeProfile_FlangeSlope, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
