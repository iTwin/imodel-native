/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/SupplementalSchema.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECObjects/ECInstance.h>
#include <Bentley/RefCounted.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct SupplementalSchemaMetaData;
typedef RefCountedPtr<SupplementalSchemaMetaData> SupplementalSchemaMetaDataPtr;
typedef bmap<Utf8String, Utf8String> SchemaNamePurposeMap;

//=======================================================================================
//! @addtogroup ECObjectsGroup
//! @beginGroup
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
    Utf8String m_primarySchemaName;
    uint32_t  m_primarySchemaMajorVersion;
    uint32_t  m_primarySchemaMinorVersion;
    uint32_t  m_supplementalSchemaPrecedence;
    Utf8String m_supplementalSchemaPurpose;
    bool   m_isUserSpecific;

    static Utf8CP s_customAttributeAccessor;

public:
    //! Constructor for SupplementalSchemaMetaData
    ECOBJECTS_EXPORT SupplementalSchemaMetaData
        (
        Utf8String  primarySchemaName,              //!< Name of the primary schema that this schema supplements
        uint32_t    primarySchemaMajorVersion,      //!< Major version of the primary schema that this schema supplements
        uint32_t    primarySchemaMinorVersion,      //!< Minor version of the primary schema that this schema supplements
        uint32_t    supplementalSchemaPrecedence,   //!< custom attributes with higher precedence overrride custom attributes with lower precedence
        Utf8String  supplementalSchemaPurpose,      //!< Purpose of this supplemental schema
        bool        isUserSpecific                  //!< if true this schema supplements the primary schema on a per user basis
        );

    //! Creates an instance of SupplementalSchemaMetaData
    ECOBJECTS_EXPORT static SupplementalSchemaMetaDataPtr Create
        (
        Utf8String  primarySchemaName,              //!< Name of the primary schema that this schema supplements
        uint32_t    primarySchemaMajorVersion,      //!< Major version of the primary schema that this schema supplements
        uint32_t    primarySchemaMinorVersion,      //!< Minor version of the primary schema that this schema supplements
        uint32_t    supplementalSchemaPrecedence,   //!< custom attributes with higher precedence overrride custom attributes with lower precedence
        Utf8String  supplementalSchemaPurpose,      //!< Purpose of this supplemental schema
        bool        isUserSpecific                  //!< if true this schema supplements the primary schema on a per user basis
        );

    //! Constructor that takes the custom attribute itself
    ECOBJECTS_EXPORT SupplementalSchemaMetaData(IECInstanceCR supplementalSchemaMetaDataCustomAttribute);

    //! Creates an instance of SupplementalSchemaMetaData
    ECOBJECTS_EXPORT static SupplementalSchemaMetaDataPtr Create(IECInstanceCR supplementalSchemaMetaDataCustomAttribute);

    ECOBJECTS_EXPORT virtual ~SupplementalSchemaMetaData() {}
    //! Returns true if the input schema defines the SupplementalSchemaMetaData custom attribute
    //! @param[in]  supplementalSchema  The schema to test to see if it is a supplemental schema
    //! @returns    True if the schema defines the SupplementalSchemaMetaData custom attribute
    ECOBJECTS_EXPORT bool IsSupplemental(ECSchemaP supplementalSchema);

    //! Creates a Custom Attribute from the current SupplementalSchemaMetaData instance
    ECOBJECTS_EXPORT IECInstancePtr CreateCustomAttribute();

    //! Gets the primary schema name
    ECOBJECTS_EXPORT Utf8StringCR GetPrimarySchemaName() const;

    //! Sets the primary schema name
    ECOBJECTS_EXPORT void SetPrimarySchemaName(Utf8StringCR name);

    //! Gets the major version of the Primary schema
    ECOBJECTS_EXPORT uint32_t GetPrimarySchemaMajorVersion() const;

    //! Sets the major version of the Primary schema
    ECOBJECTS_EXPORT void SetPrimarySchemaMajorVersion(uint32_t major);

    //! Gets the minor version of the Primary schema
    ECOBJECTS_EXPORT uint32_t GetPrimarySchemaMinorVersion() const;

    //! Sets the minor version of the Primary schema
    ECOBJECTS_EXPORT void SetPrimarySchemaMinorVersion(uint32_t minor);

    //! Gets the supplemental schema precedence
    ECOBJECTS_EXPORT uint32_t GetSupplementalSchemaPrecedence() const;

    //! Sets the supplemental schema precedence
    ECOBJECTS_EXPORT void SetSupplementalSchemaPrecedence(uint32_t precedence);

    //! Gets the supplemental schema purpose
    ECOBJECTS_EXPORT Utf8StringCR GetSupplementalSchemaPurpose() const;

    //! Sets the supplemental schema purpose
    ECOBJECTS_EXPORT void SetSupplementalSchemaPurpose(Utf8StringCR purpose);

    //! Gets whether the supplemental schema is user specific
    ECOBJECTS_EXPORT bool IsUserSpecific() const;

    //! Sets whether the supplemental schema is user specific
    ECOBJECTS_EXPORT void SetUserSpecific(bool userSpecific);

    //! Returns true if the SupplmentalSchemaMetaData's back references matches the input values
    //! @param[in]  primarySchemaName           The name of the primary schema
    //! @param[in]  primarySchemaMajorVersion   The major version of the primary schema
    //! @param[in]  primarySchemaMinorVersion   The minor version of the primary schema
    //! @param[in]  matchType                   The matching requirements for the primary schema
    ECOBJECTS_EXPORT bool IsForPrimarySchema(Utf8StringCR primarySchemaName, uint32_t primarySchemaMajorVersion, uint32_t primarySchemaMinorVersion, SchemaMatchType matchType) const;

// Statics
    //! Returns the string used to get the SupplementalSchemaMetaData custom attribute.
    ECOBJECTS_EXPORT static Utf8CP GetCustomAttributeAccessor();

    //! Returns the string used to access the PrimarySchemaName property on the
    //! SupplementalSchemaMetaData custom attribute.
    ECOBJECTS_EXPORT static Utf8CP GetPrimarySchemaNamePropertyAccessor();

    //! Returns the string used to access the PrimarySchemaMajorVersion property on the
    //! SupplementalSchemaMetaData custom attribute.
    ECOBJECTS_EXPORT static Utf8CP GetPrimarySchemaMajorVersionPropertyAccessor();

    //! Returns the string used to access the PrimarySchemaMinorVersion property on the
    //! SupplementalSchemaMetaData custom attribute.
    ECOBJECTS_EXPORT static Utf8CP GetPrimarySchemaMinorVersionPropertyAccessor();

    //! Returns the string used to access the Precedence property on the
    //! SupplementalSchemaMetaData custom attribute.
    ECOBJECTS_EXPORT static Utf8CP GetPrecedencePropertyAccessor();

    //! Returns the string used to access the Purpose property on the
    //! SupplementalSchemaMetaData custom attribute.
    ECOBJECTS_EXPORT static Utf8CP GetPurposePropertyAccessor();

    //! Returns the string used to access the IsUserSpecific property on the
    //! SupplementalSchemaMetaData custom attribute.
    ECOBJECTS_EXPORT static Utf8CP GetIsUserSpecificPropertyAccessor();

    //! Tries to get and build the SupplementalSchemaMetaData from the passed in supplementalSchemaMetadata
    //! @param[out] supplementalSchemaMetadata  The variable that gets the SupplementalSchemaMetaData from the supplementalSchema.
    //! @param[in]  supplementalSchema          The schema that the supplementalSchemeMetaData is to be retrieved from
    //! @returns    true                        If the custom attribute was successfully found on the schema
    ECOBJECTS_EXPORT static bool TryGetFromSchema(SupplementalSchemaMetaDataPtr& supplementalSchemaMetadata, ECSchemaCR supplementalSchema);

    //! Sets the input supplementalSchemaMetadata as a custom attribute on the input supplementalSchema
    //! @param[in]  supplementalSchema    The schema to set the attribute on.
    //! @param[in]  supplementalSchemaMetadata  The attribute to set on the schema
    ECOBJECTS_EXPORT static void SetMetadata(ECSchemaR supplementalSchema, SupplementalSchemaMetaDataR supplementalSchemaMetadata);

    //! Represents the Standard Purposes a supplemental schema can have
    struct StandardPurpose
        {
        //! The Standard Purpose for a Supplemental schema that contains Units information
        static Utf8CP Units () { return "Units"; }

        //! The Standard Purpose for a Supplemental schema that contains Localization information
        static Utf8CP Localization() { return "Localization"; }

        //! The Standard Purpose for a Supplemental schema that contains ChangeTracking information
        static Utf8CP ChangeTracking() { return "ChangeTracking"; }
        };
};

//=======================================================================================
//! The SupplementedSchemaBuilder merges ECCustomAttributes from multiple schemas into one combined Schema
//! @remarks
//!
//! The input schemas consist of one primary schema and any number of supplemental schemas.  The primary
//! schema contains all of the actual class definitions.  The supplemental schemas only contain ECCustomAttributes applied
//! to a skeleton of classes and properties.  Each supplemental schema is identified by a custom attribute instance of class
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
public:
    //! An enumeration used to pass precedence information around.  It provides enough information
    //! to merge the supplemental schemas into the primary schema so long as the supplemental schemas
    //! are properly ordered by precedence.
    enum SchemaPrecedence
        {
        //! The supplemental schema has lower precedence than the consolidated schema
        SCHEMA_PRECEDENCE_Lower,
        //! The supplemental schema has the same precedence as the consolidated schema. This is a special case where two schemas have the same precedence value.
        SCHEMA_PRECEDENCE_Equal,
        //! The supplemental schema has greater precedence than the consolidated schema.
        SCHEMA_PRECEDENCE_Greater
        };

private:
    Utf8String m_primarySchemaName;
    ECSchemaP m_schemaToSupplement;
    SchemaNamePurposeMap m_supplementalSchemaNamesAndPurposes;
    ECSchemaCachePtr m_schemaCache;
    bool m_createCopyOfSupplementalCustomAttribute;
    static const int PRECEDENCE_THRESHOLD = 199;

    SupplementedSchemaStatus OrderSupplementalSchemas(bmap<uint32_t, ECSchemaP>& schemasByPrecedence, ECSchemaR primarySchema, const bvector<ECSchemaP>& supplementalSchemaList, bvector<ECSchemaP>& localizationSchemas );

    void ApplyLocalizationSupplementals(ECSchemaR primarySchema, Utf8CP locale, bvector<ECSchemaP>& localizationSchemas);

    //! Merges two schemas of the same precedence into one schema.
    //! @remarks Used internally if two schemas are input that have the same precedence
    //! @param[out] mergedSchema    The resulting merged schema between the two input schemas
    //! @param[in]  schema1 A schema with equal precedence to the input schema2.  If one of the schemas may have already been copied,
    //! it must be passed in as schema1
    //! @param[in]  schema2 A schema with equal precedence to the input schema1
    SupplementedSchemaStatus CreateMergedSchemaFromSchemasWithEqualPrecedence(ECSchemaP schema1, ECSchemaP schema2);

    SupplementedSchemaStatus MergeClassesWithEqualPrecedence(ECSchemaP mergedSchema, ECClassP supplementalClass, Utf8StringCR supplementalSchemaFullName, Utf8StringCR mergedSchemaFullName);

    //! Takes a map of supplemental schemas sorted by precedence and merges them one by one to create the consolidated schema
    //! @param[in,out]  primarySchema   The schema to merge the supplemental schemas into
    //! @param[in]      schemasByPrecedence A map of supplemental schemas sorted by precedence
    SupplementedSchemaStatus MergeSchemasIntoSupplementedSchema (ECSchemaR primarySchema, bmap<uint32_t, ECSchemaP> schemasByPrecedence);

    //! Merges a supplemental schema into the consolidated schema
    //! @param[in,out]  primarySchema   The consolidated schema that will get supplemented
    //! @param[in]      supplemental    The supplemental schema being merged
    //! @param[in]      precedence      The SchemaPrecedence relative to the consolidated schema.  Because the schemas are passed in the correct order, the actual value of precedence is no longer needed.
    SupplementedSchemaStatus MergeIntoSupplementedSchema(ECSchemaR primarySchema, ECSchemaP supplementalSchema, SchemaPrecedence precedence);

    //! Takes a list of the custom attributes that need to be consolidated to a specific IECCustomAttributeContainer,
    //! determines if each custom attribute has a class specific delegate to merge it.  Then it calls either the standard or specific delegate
    //! @param[in,out]  consolidatedCustomAttributeContainer    The ECSchema, ECClass, or ECProperty that holds the consolidated custom attributes
    //! @param[in]      supplementalCustomAttributes            The custom attributes from the supplemental schema currently being merged
    //! @param[in]      precedence                              Determines if the precedence is greater, lower, or equal to the consolidated schema
    //! @param[in]      supplementalSchemaFullName              The name of the schema the supplemental custom attributes come from.  This parameter should be null
    //!                                                         unless this method is being called in the context of an UpdateSchema call
    //! @param[in]      consolidatedSchemaFullName              The name of the schema that the consolidated custom attributes come from.  This parameter should
    //!                                                         be null unless this method is being called in the context of merging schemas of equal precedence during an UpdateSchema call
    SupplementedSchemaStatus MergeCustomAttributeClasses(IECCustomAttributeContainerR consolidatedCustomAttributeContainer, ECCustomAttributeInstanceIterable supplementalCustomAttributes, SchemaPrecedence precedence, Utf8StringCP supplementalSchemaFullName, Utf8StringCP consolidatedSchemaFullName);

    //! Merges the source and target constraints of an ECRelationshipClass
    //! @param[in,out]  consolidatedECClass The consolidated class we are going to merge the custom attributes into
    //! @param[in]      supplementalECClass The supplemental class we are going to get the custom attributes from
    //! @param[in]      precedence          The precedence relative to the consolidated schema.h  Because the schemas are passed in the correct order, the actual value of precedence is no longer needed
    SupplementedSchemaStatus MergeRelationshipClassConstraints(ECClassP consolidatedECClass, ECRelationshipClassP supplementalECRelationshipClass, SchemaPrecedence precedence);

    //! Merges the custom attributes for each property in a class
    //! @param[in,out]  consolidatedECClass The consolidated class we are going to merge the custom attributes into
    //! @param[in]      supplementalECClass The supplemental class we are going to get the custom attributes from
    //! @param[in]      precedence          The precedence relative to the consolidated schema.h  Because the schemas are passed in the correct order, the actual value of precedence is no longer needed
    SupplementedSchemaStatus MergePropertyCustomAttributes(ECClassP consolidatedECClass, ECClassP supplementalECClass, SchemaPrecedence precedence);

    SupplementedSchemaStatus SupplementClass(ECSchemaR primarySchema, ECSchemaP supplementalSchema, ECClassP supplementalECClass, SchemaPrecedence precedence, Utf8StringCP supplementalSchemaFullName);

    //! The default merging "delegate"
    //! @param[in,out] consolidatedCustomAttributeContainer The ECSchema, ECClass, or ECProperty that holds the consolidation custom attributes
    //! @param[in]  consolidatedCustomAttribute The custom attribute from the consolidated schema
    //! @param[in]  supplementalCustomAttribute The custom attribute from the supplemental schema
    //! @param[in]  precedence  Determines if the precedence is greater, lower, or equal to the consolidated schema
    SupplementedSchemaStatus MergeStandardCustomAttribute(IECCustomAttributeContainerR consolidatedCustomAttributeContainer, IECInstanceR supplementalCustomAttribute, IECInstanceP consolidatedCustomAttribute, SchemaPrecedence precedence);

    //! The merging "delegate" for merging the UnitSpecification custom attribute
    //! @param[in,out] consolidatedCustomAttributeContainer The ECSchema, ECClass, or ECProperty that holds the consolidation custom attributes
    //! @param[in]  consolidatedCustomAttribute The custom attribute from the consolidated schema
    //! @param[in]  supplementalCustomAttribute The custom attribute from the supplemental schema
    //! @param[in]  precedence  Determines if the precedence is greater, lower, or equal to the consolidated schema
    SupplementedSchemaStatus MergeUnitSpecificationCustomAttribute(IECCustomAttributeContainerR consolidatedCustomAttributeContainer, IECInstanceR supplementalCustomAttribute, IECInstanceP consolidatedCustomAttribute, SchemaPrecedence precedence);

    //! The merging "delegate" for merging the UnitSpecifications custom attribute
    //! @param[in,out] consolidatedCustomAttributeContainer The ECSchema, ECClass, or ECProperty that holds the consolidation custom attributes
    //! @param[in]  consolidatedCustomAttribute The custom attribute from the consolidated schema
    //! @param[in]  supplementalCustomAttribute The custom attribute from the supplemental schema
    //! @param[in]  precedence  Determines if the precedence is greater, lower, or equal to the consolidated schema
    SupplementedSchemaStatus MergeUnitSpecificationsCustomAttribute(IECCustomAttributeContainerR consolidatedCustomAttributeContainer, IECInstanceR supplementalCustomAttribute, IECInstanceP consolidatedCustomAttribute, SchemaPrecedence precedence);

    //! This is safe function to set consolidated custom attribute. If the customAttributeInstance requires a new ECSchema to be referenced
    //! the the container's ECSchema, the ECSchemaReference will be added automatically
    //! @param[in,out] container Container on which consolidated custom attribute needs to be set
    //! @param[in]  customAttributeInstance The custom attribute from the consolidated schema
    //! @param[in] precedence If SCHEMA_PRECEDENCE_Equal, sets into list of primary ECCustomAttributes, otherwise into list of consolidated
    static ECObjectsStatus  SetMergedCustomAttribute(IECCustomAttributeContainerR container, IECInstanceR customAttributeInstance, SchemaPrecedence precedence);

public:
    //! Gets the primary schema name
    ECOBJECTS_EXPORT Utf8StringCR GetPrimarySchemaName() const;

    //! Default Constructor
    SupplementedSchemaBuilder() { m_schemaCache = ECSchemaCache::Create(); }

    //! Calling this method supplements the custom attributes of the primarySchema and all sub-containers, and applies the
    //! supplemented custom attributes back to the primarySchema
    //! @remarks This method updates the custom attribute container on the primarySchema in addition to the input classes.
    //! @param[in,out]  primarySchema   The schema containing the actual class and property definitions.  This schema will
    //! have the supplemented custom attributes added to it, and it will be marked as supplemented
    //! @param[in]  supplementalSchemaList  A list of schemas that contain a skeleton structure containing only the classes
    //! and properties needed to hold the supplementary custom attributes
    //! @param[in]  createCopyOfSupplementalCustomAttribute Create copy of supplemental custom attribute before putting it on
    //! the primary schema.
    //! @returns A status code indicating whether the primarySchema was successfully supplemented
    ECOBJECTS_EXPORT SupplementedSchemaStatus UpdateSchema(ECSchemaR primarySchema, bvector<ECSchemaP>& supplementalSchemaList, bool createCopyOfSupplementalCustomAttribute = true);

    //! Calling this method supplements the custom attributes of the primarySchema and all sub-containers, applies the
    //! supplemented custom attributes back to the primarySchema, and applies the localizations from the supplemental with the matching locale.
    //! @remarks This method updates the custom attribute container on the primarySchema in addition to the input classes.
    //! @param[in,out]  primarySchema   The schema containing the actual class and property definitions.  This schema will
    //! have the supplemented custom attributes added to it, and it will be marked as supplemented
    //! @param[in]  supplementalSchemaList  A list of schemas that contain a skeleton structure containing only the classes
    //! and properties needed to hold the supplementary custom attributes
    //! @param[in]  locale  The localization supplemental with this locale will be applied if found.
    //! @returns A status code indicating whether the primarySchema was successfully supplemented
    ECOBJECTS_EXPORT SupplementedSchemaStatus UpdateSchema(ECSchemaR primarySchema, bvector<ECSchemaP>& supplementalSchemaList, Utf8CP locale, bool createCopyOfSupplementalCustomAttribute = true);
    }; // SupplementalSchemaBuilder

//=======================================================================================
//! Container for information about supplemental schemas
//! @remarks SupplementalSchemaInfo contains the following information:
//! @li Primary schema fullname
//! @li The name of each supplemental schema that was used to supplement the primary schema
//! @li the purpose of each of the supplemental schemas
// @bsistruct                                    Carole.MacDonald                05/2012
//=======================================================================================
struct SupplementalSchemaInfo : RefCountedBase
    {
private:
    Utf8String     m_primarySchemaFullName;
    Utf8String     m_supplementedKey;
    SchemaNamePurposeMap  m_supplementalSchemaNamesAndPurpose;
    static Utf8CP s_customAttributeAccessor;

public:
    //! Constructs an instance of the SupplementalSchemaInfo class
    //! @param[in]  primarySchemaFullName   The full name of the primary schema this SupplementalSchemaInfo instance relates to
    //! @param[in]  schemaFullNameToPurposeMapping  The bmap of schema full names and purposes used to supplement the primary schema.h  Schema Fullname is the key, Purpose is the value
    ECOBJECTS_EXPORT SupplementalSchemaInfo(Utf8StringCR primarySchemaFullName, SchemaNamePurposeMap& schemaFullNameToPurposeMapping);

    //! Creates an instance of SupplementalSchemaInfo
    ECOBJECTS_EXPORT static SupplementalSchemaInfoPtr Create(Utf8StringCR primarySchemaFullName, SchemaNamePurposeMap& schemaFullNameToPurposeMapping);

    //! Returns the full name of the primary schema this SupplementalSchemaInfo instance relates to
    ECOBJECTS_EXPORT Utf8StringCR GetPrimarySchemaFullName() const { return m_primarySchemaFullName; };

    //! Generates a list of supplemental schema full names
    //! @param[out] supplementalSchemaNames List of supplemental schema full names
    //! @returns ECOBJECTS_STATUS_SchemaNotSupplemented if the schema is not supplemented, otherwise ECOBJECTS_STATUS_Success
    ECOBJECTS_EXPORT ECObjectsStatus GetSupplementalSchemaNames(bvector<Utf8String>& supplementalSchemaNames) const;

    //! Returns the purpose of the supplemental schema with the given full name
    //! @param[in]  fullSchemaName  The full name of the schema whose purpose you want to know.
    //! @returns    NULL if there is no schema with that name, otherwise the purpose of the requested supplemental schema
    ECOBJECTS_EXPORT Utf8StringCP GetPurposeOfSupplementalSchema(Utf8StringCR fullSchemaName) const;

    //! Generates a list of supplemental schema full names that have the input purpose
    //! @param[out] supplementalSchemaNames A list of schema full names that have the input purpose
    //! @param[in]  purpose             Schemas with this purpose will be returned
    //! @returns ECOBJECTS_STATUS_SchemaNotSupplemented if the schema is not supplemented, otherwise ECOBJECTS_STATUS_Success (even if no matching schemas are found)
    ECOBJECTS_EXPORT ECObjectsStatus GetSupplementalSchemasWithPurpose(bvector<Utf8String>& supplementalSchemaNames, Utf8StringCR purpose) const;

    //! Returns true if the second schema has the same supplemental schemas as the current schema for the input purpose
    //! @remarks also returns true if neither schema is supplemented
    //! @param[in]  secondSchema    The schema that this schema will be compared to
    //! @param[in]  purpose         Schemas with this purpose will be compared
    //! @returns    True    If the input schema has the same supplemental schemas as the current schema for the given purpose, or if neither schema is supplemented.
    ECOBJECTS_EXPORT bool HasSameSupplementalSchemasForPurpose(ECSchemaCR secondSchema, Utf8StringCR purpose) const;

    //__PUBLISH_SECTION_END__
    //! Creates a custom attribute defining the supplemental schema names and their purposes
    IECInstancePtr CreateCustomAttribute();

    //! Returns the string used to get the SupplementalSchemaMetaData custom attribute.
    static Utf8CP GetCustomAttributeAccessor();
    //__PUBLISH_SECTION_START__

    };
/** @endGroup */	
END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
