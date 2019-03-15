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
        //! Minimal constructor that initializes all transformation properties to default values.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        //! @param[in] baseProfileId Id of SinglePerimeterProfile that will be used to construct the profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, Dgn::DgnElementId const& baseProfileId);
        //! Minimal constructor that initializes all transformation properties to default values.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        //! @param[in] baseProfile Reference to SinglePerimeterProfile that will be used to construct the profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, SinglePerimeterProfile const& baseProfile);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        //! @param[in] baseProfileId Id of SinglePerimeterProfile that will be used to construct the profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, Dgn::DgnElementId const& baseProfileId,
                                               DPoint2d const& offset, DPoint2d const& scale, Angle const& rotation, bool mirrorAboutYAxis = false);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        //! @param[in] baseProfile Reference to SinglePerimeterProfile that will be used to construct the profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, SinglePerimeterProfile const& baseProfile,
                                               DPoint2d const& offset, DPoint2d const& scale, Angle const& rotation, bool mirrorAboutYAxis = false);

    public:
        //! @beginGroup
        //! Id of the base SinglePerimetrProfile that will be used to construct the profile.
        Dgn::DgnElementId baseProfileId;
        //! 2D vector specifying how the profiles geometry will be offseted from it's original coordinates.
        DPoint2d offset = DPoint2d::From (0.0, 0.0);
        //! A non-uniform scale factor.
        DPoint2d scale = DPoint2d::From (1.0, 1.0);
        //! Angle by which the profiles geometry will be rotated.
        Angle rotation = Angle::FromRadians (0.0);
        //! Flag indicating whether the profiles geometry should be mirrored around the Y axis.
        bool mirrorAboutYAxis = false;
        //! @endGroup
        };

protected:
    explicit DerivedProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private

private:
    virtual IGeometryPtr _CreateShapeGeometry() const override;
    virtual IGeometryPtr _UpdateShapeGeometry (Profile const& baseProfile) const override; //!< @private

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (DerivedProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (DerivedProfile)

    //! Creates an instance of DerivedProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of DerivedProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static DerivedProfilePtr Create (CreateParams const& params) { return new DerivedProfile (params); }

public:
    //! @beginGroup
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
    //! @endGroup

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
