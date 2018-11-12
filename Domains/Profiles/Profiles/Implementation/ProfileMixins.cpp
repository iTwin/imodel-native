/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/ProfileMixins.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\ProfileMixins.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ICenterLineProfile::GetCenterLine() const
    {
    ECN::ECValue ecValue;
    (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValue(ecValue, PRF_PROP_ICenterLineProfile_CenterLine);
    return ecValue.GetIGeometry();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ICenterLineProfile::SetCenterLine(IGeometryPtr val)
    {
    ECN::ECValue ecValue;
    ecValue.SetIGeometry(*val);
    (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_ICenterLineProfile_CenterLine, ecValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ICenterLineProfile::GetWallThickness() const
    {
    return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_ICenterLineProfile_WallThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ICenterLineProfile::SetWallThickness(double val)
    {
    (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_ICenterLineProfile_WallThickness, ECN::ECValue(val));
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IEllipseProfile::GetXRadius() const
    {
    return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_IEllipseProfile_XRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IEllipseProfile::SetXRadius(double val)
    {
    (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_IEllipseProfile_XRadius, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IEllipseProfile::GetYRadius() const
    {
    return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_IEllipseProfile_YRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IEllipseProfile::SetYRadius(double val)
    {
    (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_IEllipseProfile_YRadius, ECN::ECValue(val));
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ILShapeProfile::GetWidth() const
    {
    return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_ILShapeProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ILShapeProfile::SetWidth(double val)
    {
    (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_ILShapeProfile_Width, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double ILShapeProfile::GetDepth() const
    {
    return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_ILShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ILShapeProfile::SetDepth(double val)
    {
    (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_ILShapeProfile_Depth, ECN::ECValue(val));
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IRectangleShapeProfile::GetWidth() const
    {
    return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_IRectangleShapeProfile_Width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IRectangleShapeProfile::SetWidth(double val)
    {
    (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_IRectangleShapeProfile_Width, ECN::ECValue(val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double IRectangleShapeProfile::GetDepth() const
    {
    return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_IRectangleShapeProfile_Depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IRectangleShapeProfile::SetDepth(double val)
    {
    (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_IRectangleShapeProfile_Depth, ECN::ECValue(val));
    }

END_BENTLEY_PROFILES_NAMESPACE
