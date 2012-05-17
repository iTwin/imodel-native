/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/SupplementalSchema.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ECObjects\ECInstance.h>
#include <Bentley\RefCounted.h>

BEGIN_BENTLEY_EC_NAMESPACE
  
struct SupplementalSchemaMetaData;  
typedef RefCountedPtr<SupplementalSchemaMetaData> SupplementalSchemaMetaDataPtr;

//=======================================================================================
//! @ingroup ECObjectsGroup
//! Used to store the information from the SupplementalSchemaMetaData custom attribute.
//! An example of the custom attribute:
//! \code
//!     <SupplementalSchemaMetaData xmlns="Bentley_Standard_CustomAttributes.01.00">
//!         <PrimarySchemaName>primarySchemaName</PrimarySchemaName>
//!         <PrimarySchemaMajorVersion>01</PrimarySchemaMajorVersion>
//!         <PrimarySchemaMinorVersion>00</PrimarySchemaMinorVersion>
//!         <Precedence>400</Precedence>
//!         <Purpose>Localization:en-US</Purpose>
//!         <IsUserSpecific>true<IsUserSpecific>
//!     </SupplementalSchemaMetaData>
//! \endcode
//! Also defines constants for accessing the Custom Attribute directly.
// @bsistruct                                    Carole.MacDonald                03/2012
//=======================================================================================
struct SupplementalSchemaMetaData : RefCountedBase
{
private:
    WString m_primarySchemaName;
    UInt32    m_primarySchemaMajorVersion;
    UInt32    m_primarySchemaMinorVersion;
    UInt32    m_supplementalSchemaPrecedence;
    WString m_supplementalSchemaPurpose;
    bool   m_isUserSpecific;


public:
    //! Constructor for SupplementalSchemaMetaData
    ECOBJECTS_EXPORT SupplementalSchemaMetaData
        (
        WString primarySchemaName,
        UInt32    primarySchemaMajorVersion,
        UInt32    primarySchemaMinorVersion,
        UInt32    supplementalSchemaPrecedence,
        WString supplementalSchemaPurpose,
        bool   isUserSpecific
        );

    ECOBJECTS_EXPORT static SupplementalSchemaMetaDataPtr Create
        (
        WString primarySchemaName,
        UInt32    primarySchemaMajorVersion,
        UInt32    primarySchemaMinorVersion,
        UInt32    supplementalSchemaPrecedence,
        WString supplementalSchemaPurpose,
        bool   isUserSpecific
        );

    //! Constructor that takes the custom attribute itself
    ECOBJECTS_EXPORT SupplementalSchemaMetaData(IECInstanceCR supplementalSchemaMetaDataCustomAttribute);
    ECOBJECTS_EXPORT static SupplementalSchemaMetaDataPtr Create(IECInstanceCR supplementalSchemaMetaDataCustomAttribute);

    ECOBJECTS_EXPORT virtual ~SupplementalSchemaMetaData() {}
    //! Returns true if the input schema defines the SupplementalSchemaMetaData custom attribute
    //! @param[in]  supplementalSchema  The schema to test to see if it is a supplemental schema
    //! @returns    True if the schema defines the SupplementalSchemaMetaData custom attribute
    ECOBJECTS_EXPORT bool IsSupplemental(ECSchemaP supplementalSchema);

    //! Creates a Custom Attribute from the current SupplementalSchemaMetaData instance
    ECOBJECTS_EXPORT IECInstancePtr CreateCustomAttribute();

    //! Gets the primary schema name
    ECOBJECTS_EXPORT WStringCR GetPrimarySchemaName() const;

    //! Sets the primary schema name
    ECOBJECTS_EXPORT void SetPrimarySchemaName(WStringCR name);

    //! Gets the major version of the Primary schema
    ECOBJECTS_EXPORT UInt32 GetPrimarySchemaMajorVersion() const;

    //! Sets the major version of the Primary schema
    ECOBJECTS_EXPORT void SetPrimarySchemaMajorVersion(UInt32 major);

    //! Gets the minor version of the Primary schema
    ECOBJECTS_EXPORT UInt32 GetPrimarySchemaMinorVersion() const;

    //! Sets the minor version of the Primary schema
    ECOBJECTS_EXPORT void SetPrimarySchemaMinorVersion(UInt32 minor);

    //! Gets the supplemental schema precedence
    ECOBJECTS_EXPORT UInt32 GetSupplementalSchemaPrecedence() const;

    //! Sets the supplemental schema precedence
    ECOBJECTS_EXPORT void SetSupplementalSchemaPrecedence(UInt32 precedence);

    //! Gets the supplemental schema purpose
    ECOBJECTS_EXPORT WStringCR GetSupplementalSchemaPurpose() const;

    //! Sets the supplemental schema purpose
    ECOBJECTS_EXPORT void SetSupplementalSchemaPurpose(WStringCR purpose);

    //! Gets whether the supplemental schema is user specific
    ECOBJECTS_EXPORT bool IsUserSpecific() const;

    //! Sets whether the supplemental schema is user specific
    ECOBJECTS_EXPORT void SetUserSpecific(bool userSpecific);

    //! Returns true if the SupplmentalSchemaMetaData's back references matches the input values
    //! @param[in]  primarySchemaName           The name of the primary schema
    //! @param[in]  primarySchemaMajorVersion   The major version of the primary schema
    //! @param[in]  primarySchemaMinorVersion   The minor version of the primary schema
    //! @param[in]  matchType                   The matching requirements for the primary schema
    ECOBJECTS_EXPORT bool IsForPrimarySchema(WStringCR primarySchemaName, UInt32 primarySchemaMajorVersion, UInt32 primarySchemaMinorVersion, SchemaMatchType matchType) const;

// Statics
    //! Returns the string used to get the SupplementalSchemaMetaData custom attribute.
    ECOBJECTS_EXPORT static WCharCP GetCustomAttributeAccessor();    

    //! Returns the string used to access the PrimarySchemaName property on the
    //! SupplementalSchemaMetaData custom attribute.
    ECOBJECTS_EXPORT static WCharCP GetPrimarySchemaNamePropertyAccessor();    

    //! Returns the string used to access the PrimarySchemaMajorVersion property on the
    //! SupplementalSchemaMetaData custom attribute.
    ECOBJECTS_EXPORT static WCharCP GetPrimarySchemaMajorVersionPropertyAccessor();    

    //! Returns the string used to access the PrimarySchemaMinorVersion property on the
    //! SupplementalSchemaMetaData custom attribute.
    ECOBJECTS_EXPORT static WCharCP GetPrimarySchemaMinorVersionPropertyAccessor();    

    //! Returns the string used to access the Precedence property on the
    //! SupplementalSchemaMetaData custom attribute.
    ECOBJECTS_EXPORT static WCharCP GetPrecedencePropertyAccessor(); 
       
    //! Returns the string used to access the Purpose property on the
    //! SupplementalSchemaMetaData custom attribute.
    ECOBJECTS_EXPORT static WCharCP GetPurposePropertyAccessor();    
    
    //! Returns the string used to access the IsUserSpecific property on the
    //! SupplementalSchemaMetaData custom attribute.
    ECOBJECTS_EXPORT static WCharCP GetIsUserSpecificPropertyAccessor();    

    //! Tries to get and build the SupplementalSchemaMetaData from the passed in supplementalSchemaMetadata
    //! @param[out] supplementalSchemaMetadata  The variable that gets the SupplementalSchemaMetaData from the supplementalSchema. 
    //! @param[in]  supplementalSchema          The schema that the supplementalSchemeMetaData is to be retrieved from
    //! @returns    true                        If the custom attribute was successfully found on the schema
    ECOBJECTS_EXPORT static bool TryGetFromSchema(SupplementalSchemaMetaDataPtr& supplementalSchemaMetadata, ECSchemaCR supplementalSchema);

    //! Sets the input supplementalSchemaMetadata as a custom attribute on the input supplementalSchema
    //! @param[in]  supplementalSchema    The schema to set the attribute on.
    //! @param[in]  supplementalSchemaMetadata  The attribute to set on the schema
    ECOBJECTS_EXPORT static void SetMetadata(ECSchemaR supplementalSchema, SupplementalSchemaMetaDataR supplementalSchemaMetadata);

    struct StandardPurpose
        {
        //! The Standard Purpose for a Supplemental schema that contains Units information
        static WCharCP Units () { return L"Units"; } 

        //! The Standard Purpose for a Supplemental schema that contains Localization information
        static WCharCP Localization() { return L"Localization"; } 

        //! The Standard Purpose for a Supplemental schema that contains ChangeTracking information
        static WCharCP ChangeTracking() { return L"ChangeTracking"; } 
        };
};

//=======================================================================================
//! @ingroup ECObjectsGroup
//! The SupplementedSchemaBuilder merges ECCustomAttributes from multiple schemas into one combined Schema
//! @remarks 
//!
//! The input schemas consit of one primary schema and any number of supplemental schemas.  The primary
//! schema contains all of the actual class definitions.  The supplemental schemas only contain ECCustomAttributes applied
//! to a skeleton of classes and properties.  Each supplemental achema is identified by a custom attribute instance of class
//! 'SupplementalSchemaMetaData/'  This class can be found in the 'Bentley_Standard_CustomAttributes' schema.h  It contains
//! entries that give a back reference to the primary schema, a precedence value, and a context.  
//! \n\n
//! The precedence value determines which custom attributes override the primary schema and which ones are overridden
//! by the primary schema.  Precedence is an integer value from 0-699.  It is split up into 7 levels, higher numbers
//! have higher precedence and override lower precedence.  All precedences from 0-199 have lower precedence than the
//! primary and all precedences from 200-699 have higher precedence than the primary.
//! \n\n
//! Precedence levels and values
//! @li ChangeTracking  42000
//! @li Localization    9900-9999
//! @li User            600-699
//! @li Discipline      500-599
//! @li Project         400-499
//! @li Site            300-399
//! @li Customer        200-299
//! @li --- Primary Schema ---
//! @li Application     100-199
//! @li Global          0-99
//! 
//! \n
//! The level names above correspond to the default level names for ProjectWise Managed Workspaces.  For example, give
//! a supplemental schema a precedence in the 'Discipline' range if it contains metadata that varies per Discipline.
//! \n\n
//! For an example of the custom attribute that needs to be defined in a supplemental schema see SupplementalSchemaMetaData
//!
//!
//! The purpose entry in a SupplementalScheaMetaData custom attribute gives extra information that is used when a plugin
//! gathers a list of supplemental schemas.  For standard Purposes, see SupplementalSchemaMetaData.StandardPurpose.
//!
//! <b>FAQ</b>
//! @li Supplemental schemas may <b>not</b> have the same precedence if they apply the same custom attribute class to
//! the same IECCustomAttributeContainer (ECSchema, ECClass, ECProperty).
//! @li The 'dummy' classes in a Supplemental schema may <b>not</b> have base classes or any hierarchy; this information
//! is contained in the primary schema.
//! @li During supplementation, only custom attributes defined directly on a container are taken into account.  Custom
//! attributes defined on a base class are not checked during the supplementation of a derived class.
//! @li Having multiple supplemental schemas with the same precedence can reduce performance, especially when using the
//! UpdateSchema method
//!
//! <b>Constructing a supplemented schema</b>
//! @li <b>Constructing</b>: There is one constructor.  It takes no arguments.
//! @li <b>Adding schemas and consolidating them</b>: Once the manager has been constructed, the
//! UpdateSchema method is used to add the primary and supplemental schemas.  Once called, the SupplementedSchemaBuilder
//! will run through the supplementation routine and turn the input primary schema into a supplemented schema.

// @bsistruct                                   Carole.MacDonald                04/2012
//=======================================================================================
struct SupplementedSchemaBuilder
{
private:
    WString m_primarySchemaName;

    SupplementedSchemaStatus OrderSupplementalSchemas( ECSchemaR primarySchema, const bvector<ECSchemaR>& supplementalSchemaList, bmap<UInt32, ECSchemaP> schemasByPrecedence, bvector<ECSchemaP> localizationSchemas );

public:
    //! Gets the primary schema name
    ECOBJECTS_EXPORT WStringCR GetPrimarySchemaName() const;

    //! An enumeration used to pass precedence information around.  It provides enough information
    //! to merge the supplemental schemas into the primary schema so long as the supplemental schemas
    //! are properly ordered by precedence.
    enum SchemaPrecedence
        {
        //! The supplemental schema has lower precedence than the consolidated schema
        Lower,  
        //! The supplemental schema has the same precedence as the consolidated schema. This is a special case where two schemas have the same precedence value.
        Equal,  
        //! The supplemental schema has greater precedence than the consolidated schema.
        Greater 
        };

    //! Calling this method supplements the custom attributes of the primarySchema and all sub-containers, and applies the
    //! supplemented custom attributes back to the primarySchema
    //! @remarks This method updates the custom attribute container on the primarySchema in addition to the input classes.
    //! @param[in,out]  primarySchema   The schema containing the actual class and property definitions.  This schema will
    //! have the supplemented custom attributes added to it, and it will be marked as supplemented
    //! @param[in]  supplementalSchemaList  A list of schemas that contain a skeleton structure containing only the classes
    //! and properties needed to hold the supplementary custom attributes
    //! @returns A status code indicating whether the primarySchema was successfully supplemented
    ECOBJECTS_EXPORT SupplementedSchemaStatus UpdateSchema(ECSchemaR primarySchema, const bvector<ECSchemaR>& supplementalSchemaList); 
    }; // SupplementalSchemaBuilder

END_BENTLEY_EC_NAMESPACE

//__PUBLISH_SECTION_END__
