/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/DerivedProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "SinglePerimeterProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! This class represents a SinglePerimeterProfile that has been converted into a different
//! shape via a matrix transform. Usage outside of IFC compatability is not recommended.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct DerivedProfile : SinglePerimeterProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_DerivedProfile, SinglePerimeterProfile);
    friend struct DerivedProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(DerivedProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, DPoint2d const& offset,
                                               DPoint2d const& scale, Angle const& rotation, bool mirrorAboutYAxis);

    public:
        //! Required properties
        DPoint2d offset;
        DPoint2d scale;
        Angle rotation;
        bool mirrorAboutYAxis;
        };

protected:
    explicit DerivedProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (DerivedProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (DerivedProfile)

    PROFILES_EXPORT static DerivedProfilePtr Create (CreateParams const& params) { return new DerivedProfile (params); }

public:
    PROFILES_EXPORT SinglePerimeterProfilePtr GetBaseProfile() const;
    PROFILES_EXPORT Dgn::DgnElementId GetBaseProfileId() const;
    PROFILES_EXPORT void SetBaseProfile (SinglePerimeterProfile const& baseProfile);
    PROFILES_EXPORT void SetBaseProfile (Dgn::DgnElementId const& baseProfileId);

    PROFILES_EXPORT DPoint2d GetOffset() const;
    PROFILES_EXPORT void SetOffset (DPoint2d const& value);

    PROFILES_EXPORT DPoint2d GetScale() const;
    PROFILES_EXPORT void SetScale (DPoint2d const& value);

    PROFILES_EXPORT Angle GetRotation() const;
    PROFILES_EXPORT void SetRotation (Angle const& value);

    PROFILES_EXPORT bool GetMirrorAboutYAxis() const;
    PROFILES_EXPORT void SetMirrorAboutYAxis (bool value);

    }; // DerivedProfile

//=======================================================================================
//! Handler for DerivedProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DerivedProfileHandler : SinglePerimeterProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_DerivedProfile, DerivedProfile, DerivedProfileHandler, SinglePerimeterProfileHandler, PROFILES_EXPORT)

    }; // DerivedProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
