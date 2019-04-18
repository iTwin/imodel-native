/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "CompositeProfile.h"
#include "SinglePerimeterProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! Struct representing a single profile of an ArbitraryCompositeProfile.
//! @details ArbitraryCompositeProfile is composed out of components, where each component
//! has a reference to a SinglePerimeterProfile and transformation properties that will be
//! aplied to that profile when creating the geometry of ArbitraryCompositeProfile.
//! @ingroup GROUP_CompositeProfiles
//=======================================================================================
struct ArbitraryCompositeProfileComponent
    {
public:
    //! Default constructor, all properties are initialized to zero.
    PROFILES_EXPORT ArbitraryCompositeProfileComponent() = default;
    //! Constructor to initialize members.
    //! @param[in] singleProfile Reference to a SinglePerimetrProfile that will be used to construct the profile.
    PROFILES_EXPORT ArbitraryCompositeProfileComponent (SinglePerimeterProfile const& singleProfile, DPoint2d const& offset,
                                               Angle const& rotation = Angle::FromRadians (0.0), bool mirrorAboutYAxis = false);
    //! Constructor to initialize members.
    //! @param[in] singleProfile Reference to a SinglePerimetrProfile that will be used to construct the profile.
    PROFILES_EXPORT ArbitraryCompositeProfileComponent (Dgn::DgnElementId const& singleProfileId, DPoint2d const& offset,
                                               Angle const& rotation = Angle::FromRadians (0.0), bool mirrorAboutYAxis = false);

    //! Get the MemberPriority of this component.
    //! @details MemberPriority is an index used to strictly order components.
    //! MemberPriority cannot be set explicitly - it is assigned when constructing ArbitraryCompositeProfile from
    //! a vector of components. First component in the vector will get MemberPriority assigned to 0, second components MemberPriority
    //! will be assigned to 1 and so forth.
    int GetMemberPriority() const { return m_memberPriority; }

public:
    //! @beginGroup
    Dgn::DgnElementId singleProfileId = Dgn::DgnElementId(); //!< Id of a SinglePerimeterProfile that will be used to construct the profile.
    DPoint2d offset = DPoint2d::From (0.0, 0.0); //!< 2D vector specifying how the profiles geometry will be offseted from it's original coordinates.
    Angle rotation = Angle::FromRadians (0.0); //!< Angle by which the profiles geometry will be rotated.
    bool mirrorAboutYAxis = false; //!< Flag indicating whether the profiles geometry should be mirrored around the Y axis.
    //! @endGroup

private:
    friend struct ArbitraryCompositeProfile;
    int m_memberPriority = -1;
    };


//=======================================================================================
//! A Profile comprised of multiple SinglePerimeterProfiles.
//! @ingroup GROUP_CompositeProfiles
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
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (ArbitraryCompositeProfile)

    public:
        //! Constructor to initialize members.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        //! @param[in] component Vector of ArbitraryCompositeProfileComponent, see @ref components.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, ComponentVector const& components);

    public:
        ComponentVector components; //!< Vector of ArbitraryCompositeProfileComponent's. @details Order of components in the vector is importatnt, see ArbitraryCompositeProfileComponent.GetMemberPriority
        };

protected:
    explicit ArbitraryCompositeProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private

    PROFILES_EXPORT virtual Dgn::DgnDbStatus _InsertInDb() override; //!< @private
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnUpdate (Dgn::DgnElement const& original) override; //!< @private
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _LoadFromDb() override; //!< @private
    PROFILES_EXPORT virtual void _CopyFrom (Dgn::DgnElement const& source, CopyFromOptions const& opts) override; //!< @private

private:
    bool ValidateComponent (ArbitraryCompositeProfileComponent const& component, int componentIndex) const;

    virtual IGeometryPtr _CreateShapeGeometry() const override;
    virtual IGeometryPtr _UpdateShapeGeometry (Profile const& relatedProfile) const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (ArbitraryCompositeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (ArbitraryCompositeProfile)

    //! Creates an instance of ArbitraryCompositeProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of ArbitraryCompositeProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static ArbitraryCompositeProfilePtr Create (CreateParams const& params) { return new ArbitraryCompositeProfile (params); }

    //! @beginGroup
    PROFILES_EXPORT ComponentVector GetComponents() const; //!< Get the value of @ref CreateParams.components "Components"
    PROFILES_EXPORT void SetComponents (ComponentVector const& components); //!< Set the value for @ref CreateParams.components "Components"
    //! @endGroup

private:
    ComponentVector m_components;

    }; // ArbitraryCompositeProfile

//=======================================================================================
//! Handler for ArbitraryCompositeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArbitraryCompositeProfileHandler : CompositeProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_ArbitraryCompositeProfile, ArbitraryCompositeProfile, ArbitraryCompositeProfileHandler, CompositeProfileHandler, PROFILES_EXPORT)

    }; // ArbitraryCompositeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
