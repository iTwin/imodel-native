/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/DoubleCShapeProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "CompositeProfile.h"
#include "CShapeProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A CompositeProfile comprised of back-to-back Cs.
//! @ingroup GROUP_CompositeProfiles
//=======================================================================================
struct DoubleCShapeProfile : CompositeProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_DoubleCShapeProfile, CompositeProfile);
    friend struct DoubleCShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (DoubleCShapeProfile)

    public:
        //! Constructor to initialize members.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        //! @param[in] singleProfile Reference to CShapeProfile that will be used to construct the profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, double spacing, CShapeProfile const& singleProfile);
        //! Constructor to initialize members.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        //! @param[in] singleProfileId Id of CShapeProfile that will be used to construct the profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, double spacing, Dgn::DgnElementId const& singleProfileId);

    public:
        //! @beginGroup
        double spacing = 0.0; //!< Distance separating the individual back to back CShapeProfiles.
        Dgn::DgnElementId singleProfileId = Dgn::DgnElementId(); //!< Id of LShapeProfile that will be used to construct the profile.
        //! @endGroup
        };

protected:
    explicit DoubleCShapeProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private

private:
    virtual IGeometryPtr _CreateShapeGeometry() const override;
    virtual IGeometryPtr _UpdateShapeGeometry (Profile const& relatedProfile) const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (DoubleCShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (DoubleCShapeProfile)

    //! Creates an instance of DoubleCShapeProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of DoubleCShapeProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static DoubleCShapeProfilePtr Create (CreateParams const& params) { return new DoubleCShapeProfile (params); }

public:
    //! @beginGroup
    PROFILES_EXPORT double GetSpacing() const; //!< Get the value of @ref CreateParams.spacing "Spacing"
    PROFILES_EXPORT void SetSpacing (double value); //!< Set the value for @ref CreateParams.spacing "Spacing"

    PROFILES_EXPORT CShapeProfilePtr GetSingleProfile() const; //!< Get the CShapeProfile instance referenced by @ref CreateParams.singleProfileId "SingleProfileId"
    PROFILES_EXPORT Dgn::DgnDbStatus SetSingleProfile (CShapeProfile const& singleProfile); //!< Set the value for @ref CreateParams.singleProfileId "SingleProfileId"
    PROFILES_EXPORT Dgn::DgnDbStatus SetSingleProfile (Dgn::DgnElementId const& singleProfileId); //!< Set the value for @ref CreateParams.singleProfileId "SingleProfileId"
    //! @endGroup

    }; // DoubleCShapeProfile

//=======================================================================================
//! Handler for DoubleCShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DoubleCShapeProfileHandler : CompositeProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_DoubleCShapeProfile, DoubleCShapeProfile, DoubleCShapeProfileHandler, CompositeProfileHandler, PROFILES_EXPORT)

    }; // DoubleCShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
