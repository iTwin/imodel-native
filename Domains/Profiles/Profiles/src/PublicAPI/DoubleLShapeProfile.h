/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/DoubleLShapeProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CompositeProfile.h"
#include "LShapeProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
enum class DoubleLShapeProfileType : int32_t
{
    LLBB = 0, // Long Legs Back to Back
    SLBB = 1, // Short Legs Back to Back

}; // DoubleLShapeProfileType


//=======================================================================================
//! A CompositeProfile comprised of back-to-back Ls with the horizontal legs at the top of the Profile.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct DoubleLShapeProfile : CompositeProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_DoubleLShapeProfile, CompositeProfile);
    friend struct DoubleLShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(DoubleLShapeProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double spacing, LShapeProfile const& singleProfile,
                                               DoubleLShapeProfileType type = DoubleLShapeProfileType::LLBB);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double spacing, Dgn::DgnElementId const& singleProfileId,
                                               DoubleLShapeProfileType type = DoubleLShapeProfileType::LLBB);

    public:
        //! Required properties
        double spacing = 0.0;
        Dgn::DgnElementId singleProfileId = Dgn::DgnElementId();
        DoubleLShapeProfileType type = DoubleLShapeProfileType::LLBB;
        };

protected:
    explicit DoubleLShapeProfile (CreateParams const& params);

    virtual bool _Validate() const override;

private:
    bool ValidateType() const;

    virtual IGeometryPtr _CreateGeometry() const override;
    virtual IGeometryPtr _UpdateGeometry (Profile const& relatedProfile) const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (DoubleLShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (DoubleLShapeProfile)

    PROFILES_EXPORT static DoubleLShapeProfilePtr Create (CreateParams const& params) { return new DoubleLShapeProfile (params); }

public:
    PROFILES_EXPORT double GetSpacing() const;
    PROFILES_EXPORT void SetSpacing (double value);

    PROFILES_EXPORT DoubleLShapeProfileType GetType() const;
    PROFILES_EXPORT void SetType (DoubleLShapeProfileType value);

    PROFILES_EXPORT LShapeProfilePtr GetSingleProfile() const;
    PROFILES_EXPORT void SetSingleProfile (LShapeProfile const& singleProfile);
    PROFILES_EXPORT void SetSingleProfile (Dgn::DgnElementId const& singleProfileId);

    }; // DoubleLShapeProfile

//=======================================================================================
//! Handler for DoubleLShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DoubleLShapeProfileHandler : CompositeProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_DoubleLShapeProfile, DoubleLShapeProfile, DoubleLShapeProfileHandler, CompositeProfileHandler, PROFILES_EXPORT)

    }; // DoubleLShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
