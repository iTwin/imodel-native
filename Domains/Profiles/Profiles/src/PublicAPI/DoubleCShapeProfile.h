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
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double spacing, CShapeProfile const& singleProfile);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double spacing, Dgn::DgnElementId const& singleProfileId);

    public:
        //! Required properties
        double spacing = 0.0;
        Dgn::DgnElementId singleProfileId = Dgn::DgnElementId();
        };

protected:
    explicit DoubleCShapeProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private

private:
    virtual IGeometryPtr _CreateGeometry() const override; //!< @private
    virtual IGeometryPtr _UpdateGeometry (Profile const& relatedProfile) const override; //!< @private

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (DoubleCShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (DoubleCShapeProfile)

    //! Creates an instance of DoubleCShapeProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of DoubleCShapeProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static DoubleCShapeProfilePtr Create (CreateParams const& params) { return new DoubleCShapeProfile (params); }

public:
    PROFILES_EXPORT double GetSpacing() const; //!< Get the value of @ref CreateParams.spacing "Spacing"
    PROFILES_EXPORT void SetSpacing (double value); //!< Set the value for @ref CreateParams.spacing "Spacing"

    PROFILES_EXPORT CShapeProfilePtr GetSingleProfile() const; //!< Get the value of @ref CreateParams.singleProfile "SingleProfile"
    PROFILES_EXPORT void SetSingleProfile (CShapeProfile const& singleProfile); //!< Set the value for @ref CreateParams.singleProfile "SingleProfile"
    PROFILES_EXPORT void SetSingleProfile (Dgn::DgnElementId const& singleProfileId); //!< Set the value for @ref CreateParams.singleProfile "SingleProfile"

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
