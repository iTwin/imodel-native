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
//! Structure that holds required information about a catalog entry defining a standard profile.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct StandardCatalogCode
    {
    PROFILES_EXPORT StandardCatalogCode() = default;
    PROFILES_EXPORT StandardCatalogCode (Utf8CP pManufacturer, Utf8CP pStandardsOrganization, Utf8CP pRevision)
        : manufacturer (pManufacturer)
        , standardsOrganization (pStandardsOrganization)
        , revision (pRevision)
        , designation()
        {}

    //! Manufacturer of the section.
    Utf8String manufacturer;
    //! Standards Organization.
    Utf8String standardsOrganization;
    //! Revision or version of the @ref standardsOrganization.
    Utf8String revision;
    //! Name (identifier) of the section that uniquely identifies the section withing a @ref standardsOrganization.
    //! @details When setting a StandardCatalogCode for a Profile, @ref Profile.CreateParams.name "Profiles Name" is automatically used for designation.
    //! This member is used for convenience when retrieving StandardCatalog. See @ref Profile.GetStandardCatalogCode().
    Utf8String designation;
    };

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
        //! Name of the Profile.
        Utf8String name;
        };

protected:
    explicit Profile (CreateParams const& params); //!< @private

    virtual bool _Validate() const; //!< @private

    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnInsert() override; //!< @private
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnUpdate (Dgn::DgnElement const& original) override; //!< @private
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _UpdateInDb() override; //!< @private
    PROFILES_EXPORT virtual void _CopyFrom (Dgn::DgnElement const& source) override; //!< @private

public:
    Dgn::DgnDbStatus UpdateGeometry (Profile const& relatedProfile); //!< @private

protected:
    virtual bool _CreateGeometry(); //!< @private

private:
    virtual IGeometryPtr _CreateShapeGeometry() const { return nullptr; }
    virtual IGeometryPtr _UpdateShapeGeometry (Profile const& relatedProfile) const { return nullptr; }

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (Profile)
    DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS (Profile)

    //! Performs validation for the Profile validating all properties.
    //! @details This method is called internally when inserting or updating the profile
    //! in the database. If validation fails, db operation will fail too.
    //! return If profile is valid - DgnDbStatus::Success, otherwise DgnDbStatus::ValidationFailed.
    PROFILES_EXPORT Dgn::DgnDbStatus Validate() const;

    //! Get the value of @ref CreateParams.name "Name".
    PROFILES_EXPORT Utf8String GetName() const;
    //! Set the value for @ref CreateParams.name "Name".
    PROFILES_EXPORT void SetName (Utf8String val);

    //! Get StandardCatalogCode associated with this profile.
    //! @details StandardCatalogCode is populated from profiles `CodeValue` property, see @ref GetCode()
    //! @returns StandardCatalogCode. If profile has no `CodeValue` set, StandardCatalogCode with empty stirngs is returned.
    PROFILES_EXPORT StandardCatalogCode GetStandardCatalogCode() const;
    //! Set `CodeValue` for this profile using "StandardCatalogProfile" `CodeSpec`.
    //! @param catalogCode String property set used to construct the `CodeValue`. See @ref StandardCatalogCode.
    //! @returns DgnDbStatus::Success if `CodeValue` is successfully set for the profile, error code otherwise.
    PROFILES_EXPORT Dgn::DgnDbStatus SetStandardCatalogCode (StandardCatalogCode const& catalogCode);
    //! Remove `CodeValue` from this profile.
    //! @param removeCatalogCode Pass 'nullptr' to call this method.
    //! @returns DgnDbStatus::Success if `CodeValue` is successfully removed for the profile, error code otherwise.
    PROFILES_EXPORT Dgn::DgnDbStatus SetStandardCatalogCode (nullptr_t);

    //! Get the IGeometry defining the shape of this Profile.
    //! @details Geometry is created during a db Insert or Update operation.
    PROFILES_EXPORT IGeometryPtr GetShape() const;

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
