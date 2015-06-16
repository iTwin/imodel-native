/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandardCustomAttributeHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/NonCopyableClass.h>
#include <Bentley/DateTime.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/ECInstance.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//=======================================================================================    
//! DateTimeInfo contains the meta data held by the custom attribute \b %DateTimeInfo on an 
//! ECProperty of type DgnPlatform::PRIMITIVETYPE_DateTime.
//! @remarks 
//! Date time values in ECObjects are represented by the DateTime class. Each DateTime instance can 
//! contain metadata about the actual date time value (see DateTime::Info). 
//! In order to preserve the metadata when persisting a DateTime, clients can decorate the respective
//! ECProperty with the \b %DateTimeInfo custom attribute from the standard ECSchema \b Bentley_Standard_CustomAttributes.
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================    
struct DateTimeInfo
    {
private: 
    bool m_isKindNull;
    bool m_isComponentNull;
    DateTime::Info m_info;

    static const DateTime::Kind DEFAULT_KIND;
    static const DateTime::Component DEFAULT_COMPONENT;
    static const DateTime::Info s_default;

public:
    //***** Construction ******
    //Intentionally use the compiler-generated versions of copy constructor, assignment operator, and destructor

    //! Initializes a new instance of the DateTimeInfo type.
    ECOBJECTS_EXPORT DateTimeInfo ();

    //! Initializes a new instance of the DateTimeInfo type.
    //! @param[in] metadata object from which the DateTimeInfo will be initialized.
    ECOBJECTS_EXPORT explicit DateTimeInfo (DateTime::Info const& metadata);

    //__PUBLISH_SECTION_END__
    DateTimeInfo (bool isKindNull, DateTime::Kind kind, bool isComponentNull, DateTime::Component component); 
    //__PUBLISH_SECTION_START__

    //! Compares this DateTimeInfo to @p rhs.
    //! @param [in] rhs DateTimeInfo to compare this against
    //! @return true if both objects are equal, false otherwise
    ECOBJECTS_EXPORT bool operator== (DateTimeInfo const& rhs) const;

    //! Compares this DateTimeInfo to @p rhs for inequality.
    //! @param [in] rhs DateTimeInfo to compare this against
    //! @return true if the objects are not equal, false otherwise
    ECOBJECTS_EXPORT bool operator!= (DateTimeInfo const& rhs) const;

    //! Indicates whether the DateTime::Kind and DateTime::Component are both unset or not.
    //! @return true, if both DateTime::Kind and DateTime::Component are unset. false, if at least one of the two are not unset.
    ECOBJECTS_EXPORT bool IsNull () const;

    //! Indicates whether the DateTime::Kind is unset or not.
    //! @return true, if the DateTime::Kind is unset. false, otherwise
    ECOBJECTS_EXPORT bool IsKindNull () const;
    //! Indicates whether the DateTime::Component is unset or not.
    //! @return true, if the DateTime::Component is unset. false, otherwise
    ECOBJECTS_EXPORT bool IsComponentNull () const;

    //__PUBLISH_SECTION_END__
    //! Gets the content of this object as DateTime::Info.
    //! @remarks Should only be called if DateTimeInfo::IsKindNull and DateTimeInfo::IsComponentNull are not true.
    //! @return DateTime::Info representing the content of this object
    DateTime::Info const& GetInfo () const;
    //__PUBLISH_SECTION_START__

    //! Gets the content of this object as DateTime::Info.
    //! @remarks if \p useDefaultIfUnset is true, fills in default values for date time kind
    //!         and date time component if they are unset.
    //!         @see GetDefault
    //! @param[in] useDefaultIfUnset if true, default values are filled in, if a member of this object is unset,
    //!            if false, no default values are filled in. The value of unset members is undefined.
    //!            Callers have to check the unset status first using DateTimeInfo::IsKindNull and DateTimeInfo::IsComponentNull
    //! @return DateTime::Info representing the content of this object
    ECOBJECTS_EXPORT DateTime::Info GetInfo (bool useDefaultIfUnset) const; 
    
    //! Gets a DateTimeInfo object with the default values used by ECObjects.
    //! @remarks The default values are DateTime::Kind::Unspecified and DateTime::Component::DateAndTime.
    //! @return Default DateTime::Info
    ECOBJECTS_EXPORT static DateTime::Info const& GetDefault (); 

    //! Checks whether the RHS object matches this object.
    //! @remarks If one of the members
    //!          of this object is null, the RHS counterpart is ignored and the
    //!          members are considered matching.
    //! @param[in] rhs RHS
    //! @return true, if the RHS matches this object. false otherwise
    ECOBJECTS_EXPORT bool IsMatchedBy (DateTime::Info const& rhs) const;

    //! Generates a text representation of this object.
    //! @return Text representation of this object
    ECOBJECTS_EXPORT WString ToString () const;
    };

//=======================================================================================    
//! StandardCustomAttributeHelper provides APIs to access items of the Bentley standard schemas
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================    
struct StandardCustomAttributeHelper : NonCopyableClass
    {
private:
    static WCharCP const SYSTEMSCHEMA_CA_NAME;
    static WCharCP const DYNAMICSCHEMA_CA_NAME;

    //static class
    StandardCustomAttributeHelper ();
    ~StandardCustomAttributeHelper ();

public:
    //! Retrieves the DateTimeInfo metadata from the specified date time ECProperty.
    //! @remarks The DateTimeInfo metadata is defined through the \b %DateTimeInfo custom attribute (defined in the standard schema 
    //! @b Bentley_Standard_CustomAttributes) on a date time ECProperty.
    //! See also DateTimeInfo.
    //! @param[out] dateTimeInfo the retrieved content of the %DateTimeInfo custom attribute. If the property did not
    //!             carry the %DateTimeInfo custom attribute, the resulting @p dateTimeInfo's 'IsXXXNull' flags are set to true.
    //! @param[in] dateTimeProperty the date time ECProperty from which the custom attribute is to be retrieved
    //! @return ::ECOBJECTS_STATUS_Success in case of success, error codes in case of parsing errors or if @p dateTimeProperty 
    //! is not of type ::PRIMITIVETYPE_DateTime. 
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTimeInfo (DateTimeInfoR dateTimeInfo, ECPropertyCR dateTimeProperty);

    //! Indicates whether the specified schema is a @b system schema (in contrast to a user-supplied schema) by
    //! checking whether the @b %SystemSchema custom attribute from the standard schema @b Bentley_Standard_CustomAttributes
    //! is assigned to the schema.
    //! @remarks A system schema is a schema used and managed internally by the software.
    //! @param[in] schema Schema to check
    //! @return true, if @p schema is a system schema. false, otherwise
    ECOBJECTS_EXPORT static bool IsSystemSchema (ECSchemaCR schema);

    //! Indicates whether the specified schema is a so-called @b dynamic schema by
    //! checking whether the @b %DynamicSchema custom attribute from the standard schema @b Bentley_Standard_CustomAttributes
    //! is assigned to the schema.
    //! @remarks A dynamic schema is an application-generated schema where schema name is used as namespace for classes.
    //! @param[in] schema Schema to check
    //! @return true, if @p schema is a dynamic schema. false, otherwise
    ECOBJECTS_EXPORT static bool IsDynamicSchema (ECSchemaCR schema);

    //! Marks a schema as @b dynamic schema by adding the custom attribute @b DynamicSchema from the standard schema @b 
    //! Bentley_Standard_CustomAttributes to it.
    //! If the standard schema is not yet referenced, an error will be returned.
    //! @remarks A dynamic schema is an application-generated schema where schema name is used as namespace for classes.
    //! @param[in] schema Schema to mark as dynamic
    //! @param[in] isDynamicSchema true, if this schema should be marked as dynamic schema. false, otherwise.
    //! @return A status code indicating success or error
    ECOBJECTS_EXPORT static ECObjectsStatus SetIsDynamicSchema (ECSchemaR schema, bool isDynamicSchema);

    //! Returns the specified CustomAttribute ECClass
    //! @param[in] attributeName The name of the CustomAttribute ECClass
    //! @return An ECClassCP, if the attribute is found.  NULL otherwise.
    ECOBJECTS_EXPORT static ECClassCP GetCustomAttributeClass(WCharCP attributeName);

    //! Creates a custom attribute instance for the given attribute
    //! @param[in] attributeName The name of the custom attribute to create
    //! @return An instance of the given custom attribute
    ECOBJECTS_EXPORT static IECInstancePtr CreateCustomAttributeInstance(WCharCP attributeName);
    };

struct ECDbSchemaMap;
struct ECDbClassMap;
struct ECDbPropertyMap;
struct ECDbLinkTableRelationshipMap;
struct ECDbForeignKeyRelationshipMap;

//=======================================================================================    
//! ECDbMapCustomAttributeHelper is a convenience API for the custom attributes defined
//! in the ECDbMap standard ECSchema
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================    
struct ECDbMapCustomAttributeHelper
    {
private:
    ECDbMapCustomAttributeHelper();
    ~ECDbMapCustomAttributeHelper();

public:
    //! Tries to retrieve the SchemaMap custom attribute from the specified ECSchema.
    //! @param[out] schemaMap Retrieved schema map
    //! @param[in] schema ECSchema to retrieve the custom attribute from.
    //! @return true if @p schema has the custom attribute. false, if @p schema doesn't have the custom attribute
    ECOBJECTS_EXPORT static bool TryGetSchemaMap(ECDbSchemaMap& schemaMap, ECSchemaCR schema);
    
    //! Tries to retrieve the ClassMap custom attribute from the specified ECClass.
    //! @param[out] classMap Retrieved class map
    //! @param[in] ecClass ECClass to retrieve the custom attribute from.
    //! @return true if @p ecClass has the custom attribute. false, if @p ecClass doesn't have the custom attribute
    ECOBJECTS_EXPORT static bool TryGetClassMap(ECDbClassMap& classMap, ECClassCR ecClass);

    //! Tries to retrieve the PropertyMap custom attribute from the specified ECProperty.
    //! @param[out] propertyMap Retrieved property map
    //! @param[in] ecProperty ECProperty to retrieve the custom attribute from.
    //! @return true if @p ecProperty has the custom attribute. false, if @p ecProperty doesn't have the custom attribute
    ECOBJECTS_EXPORT static bool TryGetPropertyMap(ECDbPropertyMap& propertyMap, ECPropertyCR ecProperty);

    //! Tries to retrieve the LinkTableRelationshipMap custom attribute from the specified ECRelationshipClass.
    //! @param[out] linkTableRelationshipMap Retrieved link table relationship map
    //! @param[in] ecRelationship ECRelationshipClass to retrieve the custom attribute from.
    //! @return true if @p ecRelationship has the custom attribute. false, if @p ecRelationship doesn't have the custom attribute
    ECOBJECTS_EXPORT static bool TryGetLinkTableRelationshipMap(ECDbLinkTableRelationshipMap& linkTableRelationshipMap, ECRelationshipClassCR ecRelationship);

    //! Tries to retrieve the ForeignKeyRelationshipMap custom attribute from the specified ECRelationshipClass.
    //! @param[out] foreignKeyTableRelationshipMap Retrieved foreign key relationship map
    //! @param[in] ecRelationship ECRelationshipClass to retrieve the custom attribute from.
    //! @return true if @p ecRelationship has the custom attribute. false, if @p ecRelationship doesn't have the custom attribute
    ECOBJECTS_EXPORT static bool TryGetForeignKeyRelationshipMap(ECDbForeignKeyRelationshipMap& foreignKeyTableRelationshipMap, ECRelationshipClassCR ecRelationship);

    };

//=======================================================================================    
//! ECDbSchemaMap is a convenience wrapper around the SchemaMap custom attribute that simplifies
//! reading the values of that custom attribute
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================    
struct ECDbSchemaMap
    {
    friend struct ECDbMapCustomAttributeHelper;

private:
    ECSchemaCP m_schema;
    IECInstanceCP m_ca;

    ECDbSchemaMap(ECSchemaCR, IECInstanceCP ca);

public:
    ECDbSchemaMap() : m_schema(nullptr), m_ca(nullptr) {}
    //! Tries to get the value of the TablePrefix property in the SchemaMap.
    //! @param[out] tablePrefix Table prefix. It remains unchanged, if the TablePrefix property wasn't set in the SchemaMap.
    //! @return ECOBJECTSTATUS_Success if TablePrefix was set or unset in the SchemaMap. Error codes if TablePrefix didn't have a valid value,
    //! e.g. didn't comply to the naming conventions for table names.
    ECOBJECTS_EXPORT ECObjectsStatus TryGetTablePrefix(Utf8StringR tablePrefix) const;
    };

//=======================================================================================    
//! ECDbClassMap is a convenience wrapper around the ClassMap custom attribute that simplifies
//! reading the values of that custom attribute
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================    
struct ECDbClassMap
    {
friend struct ECDbMapCustomAttributeHelper;

public:
    //=======================================================================================    
    //! DbIndex is a convenience wrapper around the DbIndex struct in the ECDbMap ECSchema
    //! that simplifies reading the values of that struct
    //! @ingroup ECObjectsGroup
    //! @bsiclass
    //=======================================================================================    
    struct DbIndex
        {
    friend struct ECDbClassMap;

    private:
        Utf8String m_name;
        bool m_isUnique;
        Utf8String m_whereClause;
        bvector<Utf8String> m_properties;

        DbIndex(Utf8CP name, bool isUnique = false, Utf8CP whereClause = nullptr) : m_name(name), m_isUnique(isUnique), m_whereClause(whereClause) {}
        void AddProperty(Utf8StringCR propertyName) { m_properties.push_back(propertyName); }

    public:
        //! Gets the index name.
        //! @return Index name or nullptr if not set (This indicates that the name of the index should be auto-generated)
        Utf8CP GetName() const { return m_name.c_str(); }
        //! Gets a value indicating whether the index is a unique index or not.
        //! @return true if the index is a unique index. false otherwise
        bool IsUnique() const { return m_isUnique; }
        //! Gets the where clause if the index a partial index.
        //! @return Where clause or nullptr if the index is not partial
        Utf8CP GetWhereClause() const { return m_whereClause.c_str(); }
        //! Gets the list of property names on which the index is to be defined.
        //! @return Properties on which the index is defined.
        bvector<Utf8String> const& GetProperties() const { return m_properties; }
        };

private:
    ECClassCP m_class;
    IECInstanceCP m_ca;

    ECDbClassMap(ECClassCR, IECInstanceCP ca);

public:
    ECDbClassMap() : m_class(nullptr), m_ca(nullptr) {}

    //! Tries to get the values of the MapStrategy and MapStrategyOptions properties from the ClassMap.
    //! @param[out] mapStrategy MapStrategy. It remains unchanged, if the MapStrategy property wasn't set in the ClassMap.
    //! @param[out] mapStrategyOptions MapStrategy options. It remains unchanged, if the MapStrategyOptions property wasn't set in the ClassMap.
    //! @return ECOBJECTSTATUS_Success if MapStrategy and MapStrategyOptions were set or unset in the ClassMap. Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetMapStrategy(Utf8StringR mapStrategy, Utf8StringR mapStrategyOptions) const;
    //! Tries to get the value of the TableName property in the ClassMap.
    //! @param[out] tableName Table name. It remains unchanged, if the TableName property wasn't set in the ClassMap.
    //! @return ECOBJECTSTATUS_Success if TableName was set or unset in the ClassMap, Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetTableName(Utf8String& tableName) const;
    //! Tries to get the value of the ECInstanceIdColumn property from the ClassMap.
    //! @param[out] ecInstanceIdColumnName Name of the ECInstanceId column. It remains unchanged, if the ECInstanceIdColumn property wasn't set in the ClassMap.
    //! @return ECOBJECTSTATUS_Success if ECInstanceIdColumn was set or unset in the ClassMap, Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetECInstanceIdColumn(Utf8String& ecInstanceIdColumnName) const;
    //! Tries to get the value of the Indexes property from the ClassMap.
    //! @param[out] indexes List of DbIndexes as defined in the ClassMap. Is empty, if no indexes were defined in the ClassMap.
    //! @return SUCCESS if Indexes property is set or unset in ClassMap. Error codes if Indexes property has invalid values
    ECOBJECTS_EXPORT ECObjectsStatus TryGetIndexes(bvector<DbIndex>& indexes) const;
    };

//=======================================================================================    
//! ECDbPropertyMap is a convenience wrapper around the PropertyMap custom attribute that simplifies
//! reading the values of that custom attribute
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================    
struct ECDbPropertyMap
    {
friend struct ECDbMapCustomAttributeHelper;

private:
    ECPropertyCP m_property;
    IECInstanceCP m_ca;

    ECDbPropertyMap(ECPropertyCR, IECInstanceCP ca);

public:
    ECDbPropertyMap() : m_property(nullptr), m_ca(nullptr) {}

    //! Tries to get the value of the ColumnName property from the PropertyMap.
    //! @param[out] columnName Column name. It remains unchanged, if the ColumnName property wasn't set in the PropertyMap.
    //! @return ECOBJECTSTATUS_Success if ColumnName was set or unset in the PropertyMap, Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetColumnName(Utf8StringR columnName) const;
    //! Tries to get the value of the IsNullable property from the PropertyMap.
    //! @param[out] isNullable IsNullable flag. It remains unchanged, if the IsNullable property wasn't set in the PropertyMap.
    //! @return ECOBJECTSTATUS_Success if IsNullable was set or unset in the PropertyMap, Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetIsNullable(bool& isNullable) const;
    //! Tries to get the value of the IsUnique property from the PropertyMap.
    //! @param[out] isUnique IsUnique flag. It remains unchanged, if the IsUnique property wasn't set in the PropertyMap.
    //! @return ECOBJECTSTATUS_Success if IsUnique was set or unset in the PropertyMap, Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetIsUnique(bool& isUnique) const;
    //! Tries to get the value of the Collation property from the PropertyMap.
    //! @param[out] collation Collation. It remains unchanged, if the Collation property wasn't set in the PropertyMap.
    //! @return ECOBJECTSTATUS_Success if Collation was set or unset in the PropertyMap, Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetCollation(Utf8StringR collation) const;
    };


//=======================================================================================    
//! ECDbPropertyMap is a convenience wrapper around the PropertyMap custom attribute that simplifies
//! reading the values of that custom attribute
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================    
struct ECDbRelationshipConstraintMap
    {
friend struct ECDbRelationshipConstraintMapHelper;

private:
    Utf8String m_ecInstanceIdColumn;
    Utf8String m_ecClassIdColumn;
    bool m_createDefaultIndex;
    bool m_enforceReferentialIntegrity;
    Utf8String m_onDeleteAction;
    Utf8String m_onUpdateAction;

    ECDbRelationshipConstraintMap(Utf8CP ecInstanceIdColumn, Utf8CP ecClassIdColumn, bool createDefaultIndex, bool enforceReferentialIntegrity, Utf8CP onDeleteAction, Utf8CP onUpdateAction)
        : m_ecInstanceIdColumn(ecInstanceIdColumn), m_ecClassIdColumn(ecClassIdColumn), m_createDefaultIndex(createDefaultIndex), m_enforceReferentialIntegrity(enforceReferentialIntegrity), m_onDeleteAction(onDeleteAction), m_onUpdateAction(onUpdateAction)
        {}

public:
    ECDbRelationshipConstraintMap() : m_createDefaultIndex(true), m_enforceReferentialIntegrity(false) {}

    //! Gets the name of the constraint's ECInstanceId column.
    //! @return Constraint's ECInstanceId column or nullptr if not specified in the custom attribute
    Utf8CP GetECInstanceIdColumn() const { return m_ecInstanceIdColumn.c_str(); }
    //! Gets the name of the constraint's ECClassId column.
    //! @return Constraint's ECClassId column or nullptr if not specified in the custom attribute
    Utf8CP GetECClassIdColumn() const { return m_ecClassIdColumn.c_str(); }
    //! Gets a value indicating whether a default index on the constraint's instance id and class id columns
    //! should be created or not
    //! @return true, if the default index should be created. false otherwise
    bool CreateDefaultIndex() const { return m_createDefaultIndex; }
    //! Gets a value indicating whether referential integrity should be enforced or not.
    //! @return true, if referential integrity should be enforced. false otherwise
    bool EnforceReferentialIntegrity() const { return m_enforceReferentialIntegrity; }
    //! Gets the OnDelete action if ECDbRelationshipConstraintMap::EnforceReferentialIntegrity is true.
    //! @return OnDelete action or nullptr if not specified in the custom attribute
    Utf8CP GetOnDeleteAction() const { return m_onDeleteAction.c_str(); }
    //! Gets the OnUpdate action if ECDbRelationshipConstraintMap::EnforceReferentialIntegrity is true.
    //! @return OnUpdate action or nullptr if not specified in the custom attribute
    Utf8CP GetOnUpdateAction() const { return m_onUpdateAction.c_str(); }
    };

//=======================================================================================    
//! ECDbLinkTableRelationshipMap is a convenience wrapper around the LinkTableRelationshipMap 
//! custom attribute that simplifies reading the values of that custom attribute
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================    
struct ECDbLinkTableRelationshipMap
    {
    friend struct ECDbMapCustomAttributeHelper;

private:
    ECRelationshipClassCP m_relClass;
    IECInstanceCP m_ca;

    ECDbLinkTableRelationshipMap(ECRelationshipClassCR, IECInstanceCP ca);

public:
    ECDbLinkTableRelationshipMap() : m_relClass(nullptr), m_ca(nullptr) {}

    //! Tries to get the value of the Source property from the LinkTableRelationshipMap.
    //! @param[out] sourceConstraint Mapping information for the source constraint of the relationship. 
    //! It is set to default values, if the Source property wasn't set in the LinkTableRelationshipMap.
    //! @return ECOBJECTSTATUS_Success if Source was set or unset in the LinkTableRelationshipMap, error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetSource(ECDbRelationshipConstraintMap& sourceConstraint) const;
    //! Tries to get the value of the Target property from the LinkTableRelationshipMap.
    //! @param[out] targetConstraint Mapping information for the target constraint of the relationship. 
    //! It is set to default values, if the Target property wasn't set in the LinkTableRelationshipMap.
    //! @return ECOBJECTSTATUS_Success if Target was set or unset in the LinkTableRelationshipMap, error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetTarget(ECDbRelationshipConstraintMap& targetConstraint) const;
    //! Tries to get the value of the AllowDuplicateRelationships property from the LinkTableRelationshipMap.
    //! @param[out] allowDuplicateRelationships AllowDuplicateRelationships flag. It remains unchanged, if the AllowDuplicateRelationships property wasn't set in the LinkTableRelationshipMap.
    //! @return ECOBJECTSTATUS_Success if AllowDuplicateRelationships was set or unset in the LinkTableRelationshipMap, Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetAllowDuplicateRelationships(bool& allowDuplicateRelationships) const;
    };

//=======================================================================================    
//! ECDbForeignKeyRelationshipMap is a convenience wrapper around the ForeignKeyRelationshipMap 
//! custom attribute that simplifies reading the values of that custom attribute
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================    
struct ECDbForeignKeyRelationshipMap
    {
    friend struct ECDbMapCustomAttributeHelper;

private:
    ECRelationshipClassCP m_relClass;
    IECInstanceCP m_ca;

    ECDbForeignKeyRelationshipMap(ECRelationshipClassCR, IECInstanceCP ca);

public:
    ECDbForeignKeyRelationshipMap() : m_relClass(nullptr), m_ca(nullptr) {}

    //! Tries to get the value of the ForeignKeyEnd property from the ForeignKeyRelationshipMap.
    //! @param[out] foreignKeyEnd End of the relationship which carries the foreign key. @p foreignKeyEnd remains unchanged, 
    //! if the ForeignKeyEnd property wasn't set in the ForeignKeyRelationshipMap.
    //! @return ECOBJECTSTATUS_Success if ForeignKeyEnd was set or unset in the ForeignKeyRelationshipMap, Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetForeignKeyEnd(ECRelationshipEnd& foreignKeyEnd) const;
    //! Tries to get the value of the ForeignKey property from the ForeignKeyRelationshipMap.
    //! @param[out] foreignKeyConstraint Mapping information for the foreign key constraint of the relationship. 
    //! It remains unchanged, if the ForeignKey property wasn't set in the ForeignKeyRelationshipMap.
    //! @return ECOBJECTSTATUS_Success if ForeignKey was set or unset in the ForeignKeyRelationshipMap, error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetForeignKey(ECDbRelationshipConstraintMap& foreignKeyConstraint) const;
    };

END_BENTLEY_ECOBJECT_NAMESPACE
