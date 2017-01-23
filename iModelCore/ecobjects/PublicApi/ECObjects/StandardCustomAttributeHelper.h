/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandardCustomAttributeHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/NonCopyableClass.h>
#include <Bentley/DateTime.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/ECInstance.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//! @addtogroup ECObjectsGroup
//! @beginGroup

//=======================================================================================    
//! StandardCustomAttributeHelper provides APIs to access items of the Bentley standard schemas
//! @remarks Deprecated. Only use to access legacy schema, Bentley_Standard_CustomAttributes.
//! @bsiclass
//=======================================================================================    
struct StandardCustomAttributeHelper : NonCopyableClass
    {
private:
    //static class
    StandardCustomAttributeHelper ();
    ~StandardCustomAttributeHelper ();

public:
    //! Retrieves the DateTimeInfo metadata from the specified date time ECProperty.
    //! @remarks The DateTimeInfo metadata is defined through the \b %DateTimeInfo custom attribute (defined in the standard schema 
    //! @b Bentley_Standard_CustomAttributes) on a date time ECProperty.
    //! See also DateTimeInfo.
    //! @param[out] dateTimeInfo the retrieved content of the %DateTimeInfo custom attribute. If the property did not
    //!             carry the %DateTimeInfo custom attribute, the parameter remains unmodified.
    //! @param[in] dateTimeProperty the date time ECProperty from which the custom attribute is to be retrieved
    //! @return ECObjectsStatus::Success in case of success, error codes in case of parsing errors or if @p dateTimeProperty 
    //! is not of type ::PRIMITIVETYPE_DateTime. 
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTimeInfo (DateTime::Info& dateTimeInfo, ECPropertyCR dateTimeProperty);

    //! Returns the specified CustomAttribute ECClass
    //! @param[in] attributeName The name of the CustomAttribute ECClass
    //! @return An ECClassCP, if the attribute is found.  NULL otherwise.
    ECOBJECTS_EXPORT static ECClassCP GetCustomAttributeClass(Utf8CP attributeName);

    //! Creates a custom attribute instance for the given attribute
    //! @param[in] attributeName The name of the custom attribute to create
    //! @return An instance of the given custom attribute
    ECOBJECTS_EXPORT static IECInstancePtr CreateCustomAttributeInstance(Utf8CP attributeName);
    };

//=======================================================================================    
//! CoreCustomAttributeHelper provides APIs to access items of the Bentley standard schema, 
//! CoreCustomAttributes
//! @bsiclass
//=======================================================================================    
struct CoreCustomAttributeHelper final
    {
private:
    //static class
    CoreCustomAttributeHelper();
    ~CoreCustomAttributeHelper();

public:
    //! Retrieves the DateTimeInfo metadata from the specified date time ECProperty.
    //! @remarks The DateTimeInfo metadata is defined through the \b %DateTimeInfo custom attribute (defined in the standard schema 
    //! @b CoreCustomAttributes) on a date time ECProperty.
    //! See also DateTimeInfo.
    //! @param[out] dateTimeInfo the retrieved content of the %DateTimeInfo custom attribute. If the property did not
    //!             carry the %DateTimeInfo custom attribute, the parameter remains unmodified.
    //! @param[in] dateTimeProperty the date time ECProperty from which the custom attribute is to be retrieved
    //! @return ECObjectsStatus::Success in case of success, error codes in case of parsing errors or if @p dateTimeProperty 
    //! is not of type ::PRIMITIVETYPE_DateTime. 
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTimeInfo (DateTime::Info& dateTimeInfo, ECPropertyCR dateTimeProperty);

    //! Returns the specified ECCustomAttributeClass from the CoreCustomAttributes schema
    //! @param[in] attributeName The name of the ECCustomAttributeClass
    //! @return An ECCustomAttributeClass, if the class is found in the CoreCustomAttributes schema. Otherwise, nullptr will be returned.
    ECOBJECTS_EXPORT static ECCustomAttributeClassCP GetCustomAttributeClass(Utf8CP attributeName);

    //! Creates a custom attribute instance for the given custom attribute from the CoreCustomAttributes schema
    //! @remarks The only supported custom attributes at this time are SupplementalSchemaMetaData, SupplementalProvenance, and
    //! IsMixin. If any other custom attributes are desired, use GetCustomAttributeClass and create an instance from the resulting
    //! class.
    //! @param[in] attributeName The name of the ECCustomAttributeClass to create an ECInstance of.
    //! @return An IECInstance of the given custom attribute name, if it is one of the supported custom attributes. Otherwise, nullptr will be returned.
    ECOBJECTS_EXPORT static IECInstancePtr CreateCustomAttributeInstance(Utf8CP attributeName);
    };

//=======================================================================================    
//! ConversionCustomAttributeHelper provides APIs to access items of the Bentley standard schema, 
//! ECv3ConversionAttributes
//! @bsiclass
//=======================================================================================    
struct ConversionCustomAttributeHelper final
    {
    private:
        //static class
        ConversionCustomAttributeHelper();
        ~ConversionCustomAttributeHelper();

    public:
        //! Creates a custom attribute instance for the given custom attribute from the ECv3ConversionAttributes schema
        //! @remarks The only supported custom attribute at this time is PropertyRenamed. 
        //! @param[in] attributeName The name of the ECCustomAttributeClass to create an ECInstance of.
        //! @return An IECInstance of the given custom attribute name, if it is a supported custom attribute. Otherwise, nullptr will be returned.
        static IECInstancePtr CreateCustomAttributeInstance(Utf8CP attributeName);
    };

struct ECDbSchemaMap;
struct ECDbClassMap;
struct ShareColumns;
struct DbIndexList;
struct ECDbPropertyMap;
struct ECDbLinkTableRelationshipMap;
struct ECDbForeignKeyConstraint;

//=======================================================================================    
//! ECDbMapCustomAttributeHelper is a convenience API for the custom attributes defined
//! in the ECDbMap standard ECSchema
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

    //! Tries to retrieve the ShareColumns custom attribute from the specified ECClass.
    //! @param[out] shareColumns Retrieved ShareColumns
    //! @param[in] ecClass ECClass to retrieve the custom attribute from.
    //! @return true if @p ecClass has the custom attribute. false, if @p ecClass doesn't have the custom attribute
    ECOBJECTS_EXPORT static bool TryGetShareColumns(ShareColumns& shareColumns, ECClassCR ecClass);

    //! Indicates whether the specified ECClass has the JoinedTablePerDirectSubclass custom attribute or not.
    //! @param[in] ecClass ECClass to retrieve the custom attribute from.
    //! @return true if @p ecClass has the custom attribute. false, if @p ecClass doesn't have the custom attribute
    ECOBJECTS_EXPORT static bool HasJoinedTablePerDirectSubclass(ECEntityClassCR ecClass);

    //! Tries to retrieve the DbIndexList custom attribute from the specified ECClass.
    //! @param[out] dbIndexList Retrieved property map
    //! @param[in] ecClass ECClass to retrieve the custom attribute from.
    //! @return true if @p ecClass has the custom attribute. false, if @p ecClass doesn't have the custom attribute
    ECOBJECTS_EXPORT static bool TryGetDbIndexList(DbIndexList& dbIndexList, ECClassCR ecClass);

    //! Tries to retrieve the PropertyMap custom attribute from the specified ECProperty.
    //! @param[out] propertyMap Retrieved property map
    //! @param[in] ecProperty ECProperty to retrieve the custom attribute from.
    //! @return true if @p ecProperty has the custom attribute. false, if @p ecProperty doesn't have the custom attribute
    ECOBJECTS_EXPORT static bool TryGetPropertyMap(ECDbPropertyMap& propertyMap, PrimitiveECPropertyCR ecProperty);

    //! Tries to retrieve the LinkTableRelationshipMap custom attribute from the specified ECRelationshipClass.
    //! @param[out] linkTableRelationshipMap Retrieved link table relationship map
    //! @param[in] ecRelationship ECRelationshipClass to retrieve the custom attribute from.
    //! @return true if @p ecRelationship has the custom attribute. false, if @p ecRelationship doesn't have the custom attribute
    ECOBJECTS_EXPORT static bool TryGetLinkTableRelationshipMap(ECDbLinkTableRelationshipMap& linkTableRelationshipMap, ECRelationshipClassCR ecRelationship);

    //! Tries to retrieve the ForeignKeyConstraint custom attribute from the specified ECRelationshipClass.
    //! @param[out] foreignKeyConstraint Retrieved foreign key constraint CA
    //! @param[in] ecRelationship ECRelationshipClass to retrieve the custom attribute from.
    //! @return true if @p ecRelationship has the custom attribute. false, if @p ecRelationship doesn't have the custom attribute
    ECOBJECTS_EXPORT static bool TryGetForeignKeyConstraint(ECDbForeignKeyConstraint& foreignKeyConstraint, ECRelationshipClassCR ecRelationship);

    //! Indicates whether the specified ECRelationshipClass has the UseECInstanceIdAsForeignKey custom attribute or not.
    //! @param[in] relClass ECRelationshipClass to retrieve the custom attribute from.
    //! @return true if @p relClass has the custom attribute. false, if @p relClass doesn't have the custom attribute
    ECOBJECTS_EXPORT static bool HasUseECInstanceIdAsForeignKey(ECRelationshipClassCR relClass);
    };

//=======================================================================================    
//! ECDbSchemaMap is a convenience wrapper around the SchemaMap custom attribute that simplifies
//! reading the values of that custom attribute
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
//! @bsiclass
//=======================================================================================    
struct ECDbClassMap
    {
friend struct ECDbMapCustomAttributeHelper;

private:
    ECClassCP m_class;
    IECInstanceCP m_ca;

    ECDbClassMap(ECClassCR, IECInstanceCP ca);

public:
    ECDbClassMap() : m_class(nullptr), m_ca(nullptr) {}

    //! @return true if the ClassMap CA exists on the ECClass, false if it doesn't exist on the ECClass.
    bool IsValid() const { return m_class != nullptr && m_ca != nullptr; }

    //! Tries to get the value of the MapStrategy property from the ClassMap.
    //! @param[out] mapStrategy MapStrategy. It remains unchanged, if the MapStrategy property wasn't set in the ClassMap.
    //! @return ECOBJECTSTATUS_Success if MapStrategy was set or unset in the ClassMap. Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetMapStrategy(Utf8String& mapStrategy) const;
    //! Tries to get the value of the TableName property in the ClassMap.
    //! @param[out] tableName Table name. It remains unchanged, if the TableName property wasn't set in the ClassMap.
    //! @return ECOBJECTSTATUS_Success if TableName was set or unset in the ClassMap, Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetTableName(Utf8String& tableName) const;
    //! Tries to get the value of the ECInstanceIdColumn property from the ClassMap.
    //! @param[out] ecInstanceIdColumnName Name of the ECInstanceId column. It remains unchanged, if the ECInstanceIdColumn property wasn't set in the ClassMap.
    //! @return ECOBJECTSTATUS_Success if ECInstanceIdColumn was set or unset in the ClassMap, Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetECInstanceIdColumn(Utf8String& ecInstanceIdColumnName) const;
    };

//=======================================================================================    
//! ShareColumns is a convenience wrapper around the ShareColumns custom attribute in the ECDbMap ECSchema
//! @bsiclass
//=======================================================================================    
struct ShareColumns
    {
friend struct ECDbMapCustomAttributeHelper;

private:
    ECClassCP m_class;
    IECInstanceCP m_ca;

    ShareColumns(ECClassCR ecClass, IECInstanceCP ca) : m_class(&ecClass), m_ca(ca) {}

public:
    ShareColumns() : m_class(nullptr), m_ca(nullptr) {}

    //! @return true if ShareColumns exists on the ECClass, false if it doesn't exist on the ECClass.
    bool IsValid() const { return m_class != nullptr && m_ca != nullptr; }

    //! Tries to get the value of the SharedColumnCount property from the ShareColumns custom attribute.
    //! The SharedColumnCount includes the overflow column.
    //! @param[out] sharedColumnCount Number of shared columns to use. It remains unchanged, if the SharedColumnCount property wasn't set.
    //! @return ECOBJECTSTATUS_Success if SharedColumnCount was set or unset in the ShareColumns custom attribute, Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetSharedColumnCount(int& sharedColumnCount) const;

    //! Tries to get the value of the OverflowColumnName property in the ShareColumns custom attribute.
    //! @param[out] overflowColumnName Name of excess column. It remains unchanged, if the OverflowColumnName property wasn't set.
    //! @return ECOBJECTSTATUS_Success if OverflowColumnName was set or unset in the ShareColumns custom attribute, error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetOverflowColumnName(Utf8String& overflowColumnName) const;

    //! Tries to get the value of the ApplyToSubclassesOnly property from the ShareColumns custom attribute.
    //! @param[out] applyToSubclassesOnly ApplyToSubclassesOnly flag. It remains unchanged, if the ApplyToSubclassesOnly property wasn't set.
    //! @return ECOBJECTSTATUS_Success if ApplyToSubclassesOnly was set or unset in the ShareColumns custom attribute, error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetApplyToSubclassesOnly(bool& applyToSubclassesOnly) const;
    };

//=======================================================================================    
//! DbIndexList is a convenience wrapper around the DbIndexList custom attribute in the ECDbMap ECSchema
//! @bsiclass
//=======================================================================================    
struct DbIndexList
    {
    friend struct ECDbMapCustomAttributeHelper;

    public:
        //=======================================================================================    
        //! DbIndex is a convenience wrapper around the DbIndex struct in the ECDbMap ECSchema
        //! that simplifies reading the values of that struct
        //! @bsiclass
        //=======================================================================================    
        struct DbIndex
            {
            friend struct DbIndexList;

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

        DbIndexList(ECClassCR, IECInstanceCP ca);

    public:
        DbIndexList() : m_class(nullptr), m_ca(nullptr) {}

        //! @return true if the ClassMap CA exists on the ECClass, false if it doesn't exist on the ECClass.
        bool IsValid() const { return m_class != nullptr && m_ca != nullptr; }

        //! Get the value of the Indexes property from the DbIndexList.
        //! @param[out] indexes List of DbIndexes as defined in the DbIndexList.
        //! @return SUCCESS or Error codes if Indexes property has invalid values
        ECOBJECTS_EXPORT ECObjectsStatus GetIndexes(bvector<DbIndex>& indexes) const;
    };

//=======================================================================================    
//! ECDbPropertyMap is a convenience wrapper around the PropertyMap custom attribute that simplifies
//! reading the values of that custom attribute
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
//! ECDbLinkTableRelationshipMap is a convenience wrapper around the LinkTableRelationshipMap 
//! custom attribute that simplifies reading the values of that custom attribute
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

    //! Tries to get the value of the SourceECInstanceId property from the LinkTableRelationshipMap.
    //! @param[out] sourceECInstanceIdColumnName Name of column to which SourceECInstanceId is mapped to. 
    //! It remains unchanged, if the SourceECInstanceId property wasn't set in the LinkTableRelationshipMap.
    //! @return ECOBJECTSTATUS_Success if SourceECInstanceId was set or unset in the LinkTableRelationshipMap, error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetSourceECInstanceIdColumn(Utf8StringR sourceECInstanceIdColumnName) const;

    //! Tries to get the value of the TargetECInstanceId property from the LinkTableRelationshipMap.
    //! @param[out] targetECInstanceIdColumnName Name of column to which TargetECInstanceId is mapped to. 
    //! It remains unchanged, if the TargetECInstanceId property wasn't set in the LinkTableRelationshipMap.
    //! @return ECOBJECTSTATUS_Success if TargetECInstanceId was set or unset in the LinkTableRelationshipMap, error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetTargetECInstanceIdColumn(Utf8StringR targetECInstanceIdColumnName) const;

    //! Tries to get the value of the AllowDuplicateRelationships property from the LinkTableRelationshipMap.
    //! @param[out] allowDuplicateRelationships AllowDuplicateRelationships flag. It remains unchanged, if the AllowDuplicateRelationships property wasn't set in the LinkTableRelationshipMap.
    //! @return ECOBJECTSTATUS_Success if AllowDuplicateRelationships was set or unset in the LinkTableRelationshipMap, Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetAllowDuplicateRelationships(bool& allowDuplicateRelationships) const;
    };

//=======================================================================================    
//! ECDbForeignKeyConstraint is a convenience wrapper around the ForeignKeyConstraint 
//! custom attribute that simplifies reading the values of that custom attribute
//! @bsiclass
//=======================================================================================    
struct ECDbForeignKeyConstraint
    {
    friend struct ECDbMapCustomAttributeHelper;

private:
    ECRelationshipClassCP m_relClass;
    IECInstanceCP m_ca;

    ECDbForeignKeyConstraint(ECRelationshipClassCR, IECInstanceCP ca);

public:
    ECDbForeignKeyConstraint() : m_relClass(nullptr), m_ca(nullptr) {}

    //! Tries to get the value of the OnDeleteAction property from the ForeignKeyConstraint.
    //! @param[out] onDeleteAction OnDelete action.  @p onDeleteAction remains unchanged, if the OnDeleteAction property 
    //! wasn't set in the ForeignKeyConstraint.
    //! @return ECOBJECTSTATUS_Success if OnDeleteAction was set or unset in the ForeignKeyConstraint, Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetOnDeleteAction(Utf8StringR onDeleteAction) const;

    //! Tries to get the value of the OnUpdateAction property from the ForeignKeyConstraint.
    //! @param[out] onUpdateAction OnUpdate action. @p onDeleteAction remains unchanged, if the OnUpdateAction property 
    //! wasn't set in the ForeignKeyConstraint.
    //! @return ECOBJECTSTATUS_Success if OnUpdateAction was set or unset in the ForeignKeyConstraint, Error codes otherwise
    ECOBJECTS_EXPORT ECObjectsStatus TryGetOnUpdateAction(Utf8StringR onUpdateAction) const;
    };
	
/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE

//__PUBLISH_SECTION_END__
