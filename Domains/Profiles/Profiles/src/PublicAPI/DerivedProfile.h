/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/DerivedProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "SinglePerimeterProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! This class represents a SinglePerimeterProfile that has been converted into a different
//! shape via a matrix transform. Usage outside of IFC compatability is not recommended.
//! @ingroup GROUP_SinglePerimeterProfiles
//=======================================================================================
struct DerivedProfile : SinglePerimeterProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_DerivedProfile, SinglePerimeterProfile);
    friend struct DerivedProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (DerivedProfile)

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, Dgn::DgnElementId const& baseProfileId);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, SinglePerimeterProfile const& baseProfile);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, Dgn::DgnElementId const& baseProfileId,
                                               DPoint2d const& offset, DPoint2d const& scale, Angle const& rotation, bool mirrorAboutYAxis = false);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, SinglePerimeterProfile const& baseProfile,
                                               DPoint2d const& offset, DPoint2d const& scale, Angle const& rotation, bool mirrorAboutYAxis = false);

    public:
        //! Required properties
        Dgn::DgnElementId baseProfileId;
        DPoint2d offset;
        DPoint2d scale;
        Angle rotation;
        bool mirrorAboutYAxis;
        };

protected:
    explicit DerivedProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private

private:
    virtual IGeometryPtr _CreateGeometry() const override; //!< @private
    virtual IGeometryPtr _UpdateGeometry (Profile const& baseProfile) const override; //!< @private

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (DerivedProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (DerivedProfile)

    //! Creates an instance of DerivedProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of DerivedProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static DerivedProfilePtr Create (CreateParams const& params) { return new DerivedProfile (params); }

public:
    PROFILES_EXPORT SinglePerimeterProfilePtr GetBaseProfile() const; //!< Get the value of @ref CreateParams.baseProfile "BaseProfile"
    PROFILES_EXPORT Dgn::DgnElementId GetBaseProfileId() const; //!< Get the value of @ref CreateParams.baseProfileId "BaseProfileId"
    PROFILES_EXPORT void SetBaseProfile (SinglePerimeterProfile const& baseProfile); //!< Set the value for @ref CreateParams.baseProfile "BaseProfile"
    PROFILES_EXPORT void SetBaseProfile (Dgn::DgnElementId const& baseProfileId); //!< Set the value for @ref CreateParams.baseProfile "BaseProfile"

    PROFILES_EXPORT DPoint2d GetOffset() const; //!< Get the value of @ref CreateParams.offset "Offset"
    PROFILES_EXPORT void SetOffset (DPoint2d const& value); //!< Set the value for @ref CreateParams.offset "Offset"

    PROFILES_EXPORT DPoint2d GetScale() const; //!< Get the value of @ref CreateParams.scale "Scale"
    PROFILES_EXPORT void SetScale (DPoint2d const& value); //!< Set the value for @ref CreateParams.scale "Scale"

    PROFILES_EXPORT Angle GetRotation() const; //!< Get the value of @ref CreateParams.rotation "Rotation"
    PROFILES_EXPORT void SetRotation (Angle const& value); //!< Set the value for @ref CreateParams.rotation "Rotation"

    PROFILES_EXPORT bool GetMirrorAboutYAxis() const; //!< Get the value of @ref CreateParams.mirrorAboutYAxis "MirrorAboutYAxis"
    PROFILES_EXPORT void SetMirrorAboutYAxis (bool value); //!< Set the value for @ref CreateParams.mirrorAboutYAxis "MirrorAboutYAxis"

    }; // DerivedProfile

//=======================================================================================
//! Handler for DerivedProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DerivedProfileHandler : SinglePerimeterProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_DerivedProfile, DerivedProfile, DerivedProfileHandler, SinglePerimeterProfileHandler, PROFILES_EXPORT)

    }; // DerivedProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
