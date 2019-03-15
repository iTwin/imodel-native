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
    //! Default constructor.
    //! @details All members are initialized to empty strings.
    StandardCatalogCode() = default;
    //! Constructor to initialize members of the struct.
    //! @details See @ref designation "Designation" for why its not included in the parameters.
    //! @param pManufacturer See @ref manufacturer "Manufacturer"
    //! @param pStandardsOrganization See @ref pStandardsOrganization "Standards Organization"
    //! @param pRevision See @ref pRevision "Revision"
    StandardCatalogCode (Utf8CP pManufacturer, Utf8CP pStandardsOrganization, Utf8CP pRevision)
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
//! 2D location on the profile used to place and offset profiles when extruding them on curve members.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CardinalPoint
    {
    //! Default constructor.
    CardinalPoint()
        : name(), location {0.0, 0.0} {}
    //! Constructor to initialize members of the struct.
    //! @param pName See @ref name "Name"
    //! @param location See @ref location "Location"
    CardinalPoint (Utf8CP pName, DPoint2d const& location)
        : name (pName), location (location) {}

    //! Name of the cardinal point. @details Used to reference and distinquish cardinal points. Unique in the scope of a profile.
    Utf8String name;
    //! 2D offset from the start of profiles coordinate system (0, 0) defining the location for the cardinal point.
    DPoint2d location;
    };

//=======================================================================================
//! Enum of published standard cardinal points.
//! @ingroup GROUP_Profiles
//=======================================================================================
enum class StandardCardinalPoint : uint32_t
    {
    BottomLeft = 0,                     //!< Bottom left corner of the profiles bounding box
    BottomCenter,                       //!< Middle point of bottom line of the profiles bounding box
    BottomRight,                        //!< Bottom right corner of the profiles bounding box
    MidDepthLeft,                       //!< Middle point of left line of the profiles bounding box
    MidDepthCenter,                     //!< Center point of the profiles bounding box
    MidDepthRight,                      //!< Middle point of right line of the profiles bounding box
    TopLeft,                            //!< Top left corner of the profiles bounding box
    TopCenter,                          //!< Middle point of top line of the profiles bounding box
    TopRight,                           //!< Top right corner of the profiles bounding box
    GeometricCentroid,                  //!< Geometric centroid of the profiles geometry
    BottomInLineWithGeometricCentroid,  //!< Most bottom point of the profiles geometry thats in-line with @ref GeometricCentroid
    LeftInLineWithGeometricCentroid,    //!< Most left point of the profiles geometry thats in-line with @ref GeometricCentroid
    RightInLineWithGeometricCentroid,   //!< Most right point of the profiles geometry thats in-line with @ref GeometricCentroid
    TopInLineWithGeometricCentroid,     //!< Most top point of the profiles geometry thats in-line with @ref GeometricCentroid
    ShearCenter,                        //!< Shear center of the profiles geometry
    BottomInLineWithShearCenter,        //!< Most bottom point of the profiles geometry thats in-line with @ref ShearCenter
    LeftInLineWithShearCenter,          //!< Most left point of the profiles geometry thats in-line with @ref ShearCenter
    RightInLineWithShearCenter,         //!< Most right point of the profiles geometry thats in-line with @ref ShearCenter
    TopInLineWithShearCenter            //!< Most top point of the profiles geometry thats in-line with @ref ShearCenter
    };

//! Returns string representation of the StandardCardinalPoint enumeration.
//! @details This function always returns a valid pointer.
PROFILES_EXPORT extern Utf8CP StandardCardinalPointToString (StandardCardinalPoint standardCardinalPoint);

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

        //! @private
        virtual ~CreateParams() = default;

    protected:
        //! @private
        explicit CreateParams (Dgn::DefinitionModel const& model, Dgn::DgnClassId const& classId, Utf8CP pName);

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

    //! @beginGroup
    //! Get the value of @ref CreateParams.name "Name".
    PROFILES_EXPORT Utf8String GetName() const;
    //! Set the value for @ref CreateParams.name "Name".
    PROFILES_EXPORT void SetName (Utf8String val);
    //! @endGroup

    //! @beginGroup
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
    //! @endGroup

    //! @beginGroup
    //! Get all CardinalPoint's of this profile.
    //! @returns Empty vector if profile has zero CardinalPoint's.
    PROFILES_EXPORT bvector<CardinalPoint> GetCardinalPoints() const;
    //! Get standard CardinalPoint by @ref StandardCardinalPoint "enumeration".
    //! @returns DgnDbStatus::Success if CardinalPoint exists, error code otherwise.
    PROFILES_EXPORT Dgn::DgnDbStatus GetCardinalPoint (StandardCardinalPoint standardType, CardinalPoint& cardinalPoint) const;
    //! Get any CardinalPoint by its name.
    //! @details This method can be used to query user-defined/custom CardinalPoint's.
    //! @returns DgnDbStatus::Success if CardinalPoint exists, error code otherwise.
    PROFILES_EXPORT Dgn::DgnDbStatus GetCardinalPoint (Utf8String const& name, CardinalPoint& cardinalPoint) const;
    //! Add a user-defined/custom CardinalPoint for this profile.
    //! @details CardinalPoint's name should be unique within this profiles scope.
    //! @returns DgnDbStatus::Success if CardinalPoints was successfully added, error code otherwise.
    PROFILES_EXPORT Dgn::DgnDbStatus AddCustomCardinalPoint (CardinalPoint const& customCardinalPoint);
    //! Remove a user-defined/custom CardinalPoint from this profile by name.
    //! @details Trying to remove standard CardinalPoint's will result in DeletionProhibited error code.
    //! @returns DgnDbStatus::Success if CardinalPoint was removed successfully, error code otherwise.
    PROFILES_EXPORT Dgn::DgnDbStatus RemoveCustomCardinalPoint (Utf8String const& name);
    //! @endGroup

    //! Get the IGeometry defining the shape of this Profile.
    //! @details Geometry is created during a db Insert or Update operation.
    //! @attention A copy of IGeometry is returned with no external references to it meaning that the user must keep
    //! the reference count alive in order to use it.
    //! @code{.cpp}
    //! CurveVectorPtr curvesPtr = profile.GetShape()->GetAsCurveVector();
    //! foo (curvesPtr); // BAD: curvesPtr might be invalid at this point!
    //! bar (*profile.GetShape()); // BAD: reference will be invalid as the newly returned IGeometryPtr goes out of scope instantly!
    //! @endcode
    //! Valid use:
    //! @code{.cpp}
    //! IGeometryPtr shapePtr = profile.GetShape(); // Will be valid until shapePtr goes out of scope (unless referenced again)
    //! foo (shapePtr->GetAsCurveVector()); // OK
    //! bar (*shapePtr); // OK
    //! @endcode
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
