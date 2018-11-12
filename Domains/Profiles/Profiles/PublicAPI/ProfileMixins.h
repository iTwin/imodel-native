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
    PROFILES_EXPORT IGeometryPtr GetCenterLine() const;
    PROFILES_EXPORT void SetCenterLine(IGeometryPtr val);

    PROFILES_EXPORT double GetWallThickness() const;
    PROFILES_EXPORT void SetWallThickness(double val);

    }; // ICenterLineProfile

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct IEllipseProfile : NonCopyableClass
    {
protected:
    virtual ~IEllipseProfile() = default;

public:
    PROFILES_EXPORT double GetXRadius() const;
    PROFILES_EXPORT void SetXRadius(double val);

    PROFILES_EXPORT double GetYRadius() const;
    PROFILES_EXPORT void SetYRadius(double val);

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
    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth(double val);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth(double val);

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
    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth(double val);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth(double val);

    }; // IRectangleShapeProfile

END_BENTLEY_PROFILES_NAMESPACE
