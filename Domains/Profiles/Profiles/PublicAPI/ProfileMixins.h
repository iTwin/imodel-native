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
struct ICenterLineProfile : NonCopyableClass
    {
protected:
    virtual ~ICenterLineProfile() = default;

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
struct ICShapeProfile : NonCopyableClass
    {
protected:
    virtual ~ICShapeProfile() = default;

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
struct ICustomProfile : NonCopyableClass
    {
protected:
    virtual ~ICustomProfile() = default;

    }; // ICustomProfile

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct IEllipseProfile : NonCopyableClass
    {
protected:
    virtual ~IEllipseProfile() = default;

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
struct ILShapeProfile : NonCopyableClass
    {
protected:
    virtual ~ILShapeProfile() = default;

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
struct IRectangleShapeProfile : NonCopyableClass
    {
protected:
    virtual ~IRectangleShapeProfile() = default;

public:
    double GetWidth() const { return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_IRectangleShapeProfile_Width); }
    void SetWidth(double val) { (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_IRectangleShapeProfile_Width, ECN::ECValue(val)); }
    double GetDepth() const { return (dynamic_cast<Dgn::DgnElement const*> (this))->GetPropertyValueDouble(PRF_PROP_IRectangleShapeProfile_Depth); }
    void SetDepth(double val) { (dynamic_cast<Dgn::DgnElement*> (this))->SetPropertyValue(PRF_PROP_IRectangleShapeProfile_Depth, ECN::ECValue(val)); }

    }; // IRectangleShapeProfile

END_BENTLEY_PROFILES_NAMESPACE
