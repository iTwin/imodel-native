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
struct ArbitraryCompositeProfileComponent
    {
public:
    PROFILES_EXPORT ArbitraryCompositeProfileComponent() = default;
    PROFILES_EXPORT ArbitraryCompositeProfileComponent (Dgn::DgnElementId const& singleProfileId, DPoint2d const& offset,
                                               Angle const& rotation = Angle::FromRadians (0.0), bool mirrorAboutYAxis = false);

    int GetMemberPriority() const { return m_memberPriority; }

public:
    Dgn::DgnElementId singleProfileId = Dgn::DgnElementId();
    DPoint2d offset = DPoint2d::From (0.0, 0.0);
    Angle rotation = Angle::FromRadians (0.0);
    bool mirrorAboutYAxis = false;

private:
    friend struct ArbitraryCompositeProfile;
    int m_memberPriority = -1;
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
    typedef bvector<ArbitraryCompositeProfileComponent> ComponentVector;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(ArbitraryCompositeProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, ComponentVector const& components);

    public:
        //! Required properties
        ComponentVector components;
        };

protected:
    explicit ArbitraryCompositeProfile (CreateParams const& params);

    virtual bool _Validate() const override;

    PROFILES_EXPORT virtual Dgn::DgnDbStatus _InsertInDb() override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnUpdate (Dgn::DgnElement const& original) override;
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _LoadFromDb() override;
    PROFILES_EXPORT virtual void _CopyFrom (Dgn::DgnElement const& source) override;

private:
    bool ValidateComponent (ArbitraryCompositeProfileComponent const& component, int componentIndex) const;

    virtual IGeometryPtr _CreateGeometry() const override;
    virtual IGeometryPtr _UpdateGeometry (Profile const& relatedProfile) const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (ArbitraryCompositeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (ArbitraryCompositeProfile)

    PROFILES_EXPORT static ArbitraryCompositeProfilePtr Create (CreateParams const& params) { return new ArbitraryCompositeProfile (params); }

    PROFILES_EXPORT ComponentVector GetComponents() const;
    PROFILES_EXPORT void SetComponents (ComponentVector const& components);

private:
    ComponentVector m_components;

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
