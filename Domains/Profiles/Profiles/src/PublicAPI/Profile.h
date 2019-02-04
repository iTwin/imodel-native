/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/Profile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A resource defining one or more 2D areas that may have voids.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct Profile : Dgn::DefinitionElement
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_Profile, Dgn::DefinitionElement);
    friend struct ProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (Profile)

    public:
        virtual ~CreateParams() = default;
        explicit CreateParams (Dgn::DgnModel const& model, Dgn::DgnClassId const& classId, Utf8CP pName);

    public:
        Utf8String name;
        };

protected:
    explicit Profile (CreateParams const& params); //!< @private

    virtual bool _Validate() const;

    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnInsert() override; //!< @private
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnUpdate (Dgn::DgnElement const& original) override; //!< @private
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _UpdateInDb() override; //!< @private
    PROFILES_EXPORT virtual void _CopyFrom (Dgn::DgnElement const& source) override; //!< @private

public:
    //! @private
    Dgn::DgnDbStatus UpdateGeometry (Profile const& relatedProfile);

protected:
    virtual bool _CreateGeometry();

private:
    virtual IGeometryPtr _CreateShapeGeometry() const { return nullptr; }
    virtual IGeometryPtr _UpdateShapeGeometry (Profile const& relatedProfile) const { return nullptr; }

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (Profile)
    DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS (Profile)

    PROFILES_EXPORT Dgn::DgnDbStatus Validate() const;

    PROFILES_EXPORT Utf8String GetName() const; //!< Get the value of @ref CreateParams.name "Name"
    PROFILES_EXPORT void SetName (Utf8String val); //!< Set the value for @ref CreateParams.name "Name"

    PROFILES_EXPORT IGeometryPtr GetShape() const; //!< Get the value of @ref CreateParams.shape "Shape"

private:
    void SetShape (IGeometry const& val);

private:
    bool m_geometryUpdated;
    }; // Profile

//=======================================================================================
//! Handler for Profile class.
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ProfileHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS_ABSTRACT (PRF_CLASS_Profile, Profile, ProfileHandler, Dgn::dgn_ElementHandler::Definition, PROFILES_EXPORT)

    }; // ProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
