/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/ArbitraryCompositeProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CompositeProfile.h"
#include "SinglePerimeterProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CompositeProfileComponent
    {
    PROFILES_EXPORT CompositeProfileComponent() = default;
    PROFILES_EXPORT CompositeProfileComponent (SinglePerimeterProfile const& singleProfile, bool mirrorAboutYAxis, DPoint2d const& offset, Angle const& rotation);
    PROFILES_EXPORT CompositeProfileComponent (Dgn::DgnElementId const& singleProfileId, bool mirrorAboutYAxis, DPoint2d const& offset, Angle const& rotation);

    SinglePerimeterProfilePtr singleProfilePtr = nullptr;
    Dgn::DgnElementId singleProfileId = Dgn::DgnElementId();
    bool mirrorAboutYAxis = false;
    DPoint2d offset = DPoint2d::From (0.0, 0.0);
    Angle rotation = Angle::FromRadians (0.0);
    };


//=======================================================================================
//! A Profile comprised of multiple SinglePerimeterProfiles.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct ArbitraryCompositeProfile : CompositeProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_ArbitraryCompositeProfile, CompositeProfile);
    friend struct ArbitraryCompositeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(ArbitraryCompositeProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, bvector<CompositeProfileComponent> const& components);

    public:
        //! Required properties
        bvector<CompositeProfileComponent> components;
        };

protected:
    explicit ArbitraryCompositeProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

    PROFILES_EXPORT virtual Dgn::DgnDbStatus _InsertInDb() override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnUpdate (Dgn::DgnElement const& original) override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnDelete() const override;
    PROFILES_EXPORT virtual void _CopyFrom (Dgn::DgnElement const& source) override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _LoadFromDb() override;

private:
    Dgn::DgnDbStatus InsertRelationship (CompositeProfileComponent const& component, int memberPriority);

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (ArbitraryCompositeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (ArbitraryCompositeProfile)

    PROFILES_EXPORT static ArbitraryCompositeProfilePtr Create (CreateParams const& params) { return new ArbitraryCompositeProfile (params); }

public:
    PROFILES_EXPORT bvector<CompositeProfileComponent> GetComponents() const;
    PROFILES_EXPORT void SetComponents (bvector<CompositeProfileComponent> const& components);

private:
    bvector<CompositeProfileComponent> m_components;

    }; // ArbitraryCompositeProfile

//=======================================================================================
//! Handler for ArbitraryCompositeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArbitraryCompositeProfileHandler : CompositeProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_ArbitraryCompositeProfile, ArbitraryCompositeProfile, ArbitraryCompositeProfileHandler, CompositeProfileHandler, PROFILES_EXPORT)

    }; // ArbitraryCompositeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
