/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/ProfileMixins.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct ICenterLineProfile
{
protected:
    virtual Dgn::DgnElementR _ICenterLineProfileToDgnElement() = 0;

public:
    //IGeometryPtr GetCenterLine() const { return (dynamic_cast<Dgn::DgnElement*> (this))->GetPropertyValue(PRF_PROP_ICenterLineProfile_CenterLine); }
    //void SetCenterLine(IGeometryPtr val) { (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_ICenterLineProfile_CenterLine, ECN::ECValue(val)); }
    double GetWallThickness() const { return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_ICenterLineProfile_WallThickness); }
    void SetWallThickness(double val) { (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_ICenterLineProfile_WallThickness, ECN::ECValue(val)); }

}; // ICenterLineProfile

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct ICShapeProfile
{
protected:
    virtual Dgn::DgnElementR _ICShapeProfileToDgnElement() = 0;

public:
    double GetWidth() const { return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_ICShapeProfile_Width); }
    void SetWidth(double val) { (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_ICShapeProfile_Width, ECN::ECValue(val)); }
    double GetDepth() const { return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_ICShapeProfile_Depth); }
    void SetDepth(double val) { (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_ICShapeProfile_Depth, ECN::ECValue(val)); }

}; // ICShapeProfile

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct ICustomProfile
{
protected:
    virtual Dgn::DgnElementR _ICustomProfileToDgnElement() = 0;


}; // ICustomProfile

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct IEllipseProfile
{
protected:
    virtual Dgn::DgnElementR _IEllipseProfileToDgnElement() = 0;

public:
    double GetXRadius() const { return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_IEllipseProfile_XRadius); }
    void SetXRadius(double val) { (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_IEllipseProfile_XRadius, ECN::ECValue(val)); }
    double GetYRadius() const { return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_IEllipseProfile_YRadius); }
    void SetYRadius(double val) { (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_IEllipseProfile_YRadius, ECN::ECValue(val)); }

}; // IEllipseProfile

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct ILShapeProfile
{
protected:
    virtual Dgn::DgnElementR _ILShapeProfileToDgnElement() = 0;

public:
    double GetWidth() const { return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_ILShapeProfile_Width); }
    void SetWidth(double val) { (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_ILShapeProfile_Width, ECN::ECValue(val)); }
    double GetDepth() const { return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_ILShapeProfile_Depth); }
    void SetDepth(double val) { (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_ILShapeProfile_Depth, ECN::ECValue(val)); }

}; // ILShapeProfile

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct IRectangleShapeProfile
{
protected:
    virtual Dgn::DgnElementR _IRectangleShapeProfileToDgnElement() = 0;

public:
    double GetWidth() const { return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_IRectangleShapeProfile_Width); }
    void SetWidth(double val) { (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_IRectangleShapeProfile_Width, ECN::ECValue(val)); }
    double GetDepth() const { return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_IRectangleShapeProfile_Depth); }
    void SetDepth(double val) { (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_IRectangleShapeProfile_Depth, ECN::ECValue(val)); }

}; // IRectangleShapeProfile

END_BENTLEY_PROFILES_NAMESPACE
