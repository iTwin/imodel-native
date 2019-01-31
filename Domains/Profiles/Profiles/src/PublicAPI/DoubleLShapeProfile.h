/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/DoubleLShapeProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "CompositeProfile.h"
#include "LShapeProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! TODO Karolis: add description
//! @ingroup GROUP_CompositeProfiles
//=======================================================================================
enum class DoubleLShapeProfileType : int32_t
{
    LLBB = 0, //!< Long Legs Back to Back
    SLBB = 1, //!< Short Legs Back to Back

}; // DoubleLShapeProfileType


//=======================================================================================
//! A CompositeProfile comprised of back-to-back Ls with the horizontal legs at the top of the Profile.
//! @ingroup GROUP_CompositeProfiles
//=======================================================================================
struct DoubleLShapeProfile : CompositeProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_DoubleLShapeProfile, CompositeProfile);
    friend struct DoubleLShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (DoubleLShapeProfile)

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
    explicit DoubleLShapeProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private

private:
    bool ValidateType() const;

    virtual IGeometryPtr _CreateGeometry() const override; //!< @private
    virtual IGeometryPtr _UpdateGeometry (Profile const& relatedProfile) const override; //!< @private

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (DoubleLShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (DoubleLShapeProfile)

    //! Creates an instance of DoubleLShapeProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of DoubleLShapeProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static DoubleLShapeProfilePtr Create (CreateParams const& params) { return new DoubleLShapeProfile (params); }

public:
    PROFILES_EXPORT double GetSpacing() const; //!< Get the value of @ref CreateParams.spacing "Spacing"
    PROFILES_EXPORT void SetSpacing (double value); //!< Set the value for @ref CreateParams.spacing "Spacing"

    PROFILES_EXPORT DoubleLShapeProfileType GetType() const; //!< Get the value of @ref CreateParams.type "Type"
    PROFILES_EXPORT void SetType (DoubleLShapeProfileType value); //!< Set the value for @ref CreateParams.type "Type"

    PROFILES_EXPORT LShapeProfilePtr GetSingleProfile() const; //!< Get the value of @ref CreateParams.singleProfile "SingleProfile"
    PROFILES_EXPORT void SetSingleProfile (LShapeProfile const& singleProfile); //!< Set the value for @ref CreateParams.singleProfile "SingleProfile"
    PROFILES_EXPORT void SetSingleProfile (Dgn::DgnElementId const& singleProfileId); //!< Set the value for @ref CreateParams.singleProfile "SingleProfile"

    }; // DoubleLShapeProfile

//=======================================================================================
//! Handler for DoubleLShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DoubleLShapeProfileHandler : CompositeProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_DoubleLShapeProfile, DoubleLShapeProfile, DoubleLShapeProfileHandler, CompositeProfileHandler, PROFILES_EXPORT)

    }; // DoubleLShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
