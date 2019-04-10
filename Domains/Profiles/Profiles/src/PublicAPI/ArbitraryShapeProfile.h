/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/ArbitraryShapeProfile.h $
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
//! TODO Karolis: Add description
//! @ingroup GROUP_SinglePerimeterProfiles
//=======================================================================================
struct ArbitraryShapeProfile : SinglePerimeterProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_ArbitraryShapeProfile, SinglePerimeterProfile);
    friend struct ArbitraryShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (ArbitraryShapeProfile)

    protected:
        //! @private
        explicit CreateParams (Dgn::DefinitionModel const& model, Dgn::DgnClassId const& classId, Utf8CP pName, IGeometryPtr const& geometryPtr);

    public:
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        //! @param[in] geometryPtr IGeometry to set for this profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, IGeometryPtr const& geometryPtr);

    public:
        //! Geometry of the profile
        IGeometryPtr geometryPtr = nullptr;
        };

protected:
    //! @private
    explicit ArbitraryShapeProfile (CreateParams const& params);

        //! @private
    virtual bool _Validate() const override;

    //! @private
    PROFILES_EXPORT virtual void _CopyFrom (Dgn::DgnElement const& source, CopyFromOptions const& opts) override;

private:
    virtual IGeometryPtr _CreateShapeGeometry() const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (ArbitraryShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (ArbitraryShapeProfile)

    //! Creates an instance of ArbitraryShapeProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of ArbitraryShapeProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static ArbitraryShapeProfilePtr Create (CreateParams const& params) { return new ArbitraryShapeProfile (params); }

protected:
    IGeometryPtr m_geometryPtr;

    }; // ArbitraryShapeProfile

//=======================================================================================
//! Handler for ArbitraryShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArbitraryShapeProfileHandler : SinglePerimeterProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_ArbitraryShapeProfile, ArbitraryShapeProfile, ArbitraryShapeProfileHandler, SinglePerimeterProfileHandler, PROFILES_EXPORT)

    }; // ArbitraryShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
