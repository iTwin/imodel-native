/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/EllipseProfile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\EllipseProfile.h>
#include <Profiles\ProfilesGeometry.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (EllipseProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
EllipseProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
EllipseProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double xRadius, double yRadius)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , xRadius (xRadius)
    , yRadius (yRadius)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
EllipseProfile::EllipseProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetXRadius (params.xRadius);
    SetYRadius (params.yRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool EllipseProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    bool const isXRadiusValid = ValidateXRadius();
    bool const isYRadiusValid = ValidateYRadius();

    return isXRadiusValid && isYRadiusValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr EllipseProfile::_CreateGeometry() const
    {
    return ProfilesGeomApi::CreateEllipse (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool EllipseProfile::ValidateXRadius() const
    {
    double const xRadius = GetXRadius();
    bool const isPositive = std::isfinite (xRadius) && xRadius > 0.0;

    return isPositive;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool EllipseProfile::ValidateYRadius() const
    {
    double const yRadius = GetYRadius();
    bool const isPositive = std::isfinite (yRadius) && yRadius > 0.0;

    return isPositive;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double EllipseProfile::GetXRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_EllipseProfile_XRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void EllipseProfile::SetXRadius (double val)
    {
    SetPropertyValue (PRF_PROP_EllipseProfile_XRadius, ECN::ECValue (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double EllipseProfile::GetYRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_EllipseProfile_YRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void EllipseProfile::SetYRadius (double val)
    {
    SetPropertyValue (PRF_PROP_EllipseProfile_YRadius, ECN::ECValue (val));
    }

END_BENTLEY_PROFILES_NAMESPACE
