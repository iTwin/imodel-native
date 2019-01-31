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
        DEFINE_T_SUPER(DoubleCShapeProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double spacing, CShapeProfile const& singleProfile);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double spacing, Dgn::DgnElementId const& singleProfileId);

    public:
        //! Required properties
        double spacing = 0.0;
        Dgn::DgnElementId singleProfileId = Dgn::DgnElementId();
        };

protected:
    explicit DoubleCShapeProfile (CreateParams const& params);

    virtual bool _Validate() const override;

private:
    virtual IGeometryPtr _CreateGeometry() const override;
    virtual IGeometryPtr _UpdateGeometry (Profile const& relatedProfile) const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (DoubleCShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (DoubleCShapeProfile)

    PROFILES_EXPORT static DoubleCShapeProfilePtr Create (CreateParams const& params) { return new DoubleCShapeProfile (params); }

public:
    PROFILES_EXPORT double GetSpacing() const;
    PROFILES_EXPORT void SetSpacing (double value);

    PROFILES_EXPORT CShapeProfilePtr GetSingleProfile() const;
    PROFILES_EXPORT void SetSingleProfile (CShapeProfile const& singleProfile);
    PROFILES_EXPORT void SetSingleProfile (Dgn::DgnElementId const& singleProfileId);

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
