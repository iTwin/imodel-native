/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbMapSchemaHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECObjects/ECObjectsAPI.h>
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

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
struct ECDbMapCustomAttributeHelper final
    {
    private:
        ECDbMapCustomAttributeHelper();
        ~ECDbMapCustomAttributeHelper();

    public:
        //! Tries to retrieve the SchemaMap custom attribute from the specified ECSchema.
        //! @param[out] schemaMap Retrieved schema map
        //! @param[in] schema ECSchema to retrieve the custom attribute from.
        //! @return true if @p schema has the custom attribute. false, if @p schema doesn't have the custom attribute
        static bool TryGetSchemaMap(ECDbSchemaMap& schemaMap, ECN::ECSchemaCR schema);

        //! Tries to retrieve the ClassMap custom attribute from the specified ECClass.
        //! @param[out] classMap Retrieved class map
        //! @param[in] ecClass ECClass to retrieve the custom attribute from.
        //! @return true if @p ecClass has the custom attribute. false, if @p ecClass doesn't have the custom attribute
        static bool TryGetClassMap(ECDbClassMap& classMap, ECN::ECClassCR ecClass);

        //! Tries to retrieve the ShareColumns custom attribute from the specified ECClass.
        //! @param[out] shareColumns Retrieved ShareColumns
        //! @param[in] ecClass ECClass to retrieve the custom attribute from.
        //! @return true if @p ecClass has the custom attribute. false, if @p ecClass doesn't have the custom attribute
        static bool TryGetShareColumns(ShareColumns& shareColumns, ECN::ECClassCR ecClass);

        //! Indicates whether the specified ECClass has the JoinedTablePerDirectSubclass custom attribute or not.
        //! @param[in] ecClass ECClass to retrieve the custom attribute from.
        //! @return true if @p ecClass has the custom attribute. false, if @p ecClass doesn't have the custom attribute
        static bool HasJoinedTablePerDirectSubclass(ECN::ECEntityClassCR ecClass);

        //! Tries to retrieve the DbIndexList custom attribute from the specified ECClass.
        //! @param[out] dbIndexList Retrieved property map
        //! @param[in] ecClass ECClass to retrieve the custom attribute from.
        //! @return true if @p ecClass has the custom attribute. false, if @p ecClass doesn't have the custom attribute
        static bool TryGetDbIndexList(DbIndexList& dbIndexList, ECN::ECClassCR ecClass);

        //! Tries to retrieve the PropertyMap custom attribute from the specified ECProperty.
        //! @param[out] propertyMap Retrieved property map
        //! @param[in] ecProperty ECProperty to retrieve the custom attribute from.
        //! @return true if @p ecProperty has the custom attribute. false, if @p ecProperty doesn't have the custom attribute
        static bool TryGetPropertyMap(ECDbPropertyMap& propertyMap, ECN::PrimitiveECPropertyCR ecProperty);

        //! Tries to retrieve the LinkTableRelationshipMap custom attribute from the specified ECRelationshipClass.
        //! @param[out] linkTableRelationshipMap Retrieved link table relationship map
        //! @param[in] ecRelationship ECRelationshipClass to retrieve the custom attribute from.
        //! @return true if @p ecRelationship has the custom attribute. false, if @p ecRelationship doesn't have the custom attribute
        static bool TryGetLinkTableRelationshipMap(ECDbLinkTableRelationshipMap& linkTableRelationshipMap, ECN::ECRelationshipClassCR ecRelationship);

        //! Tries to retrieve the ForeignKeyConstraint custom attribute from the specified ECRelationshipClass.
        //! @param[out] foreignKeyConstraint Retrieved foreign key constraint CA
        //! @param[in] ecRelationship ECRelationshipClass to retrieve the custom attribute from.
        //! @return true if @p ecRelationship has the custom attribute. false, if @p ecRelationship doesn't have the custom attribute
        static bool TryGetForeignKeyConstraint(ECDbForeignKeyConstraint& foreignKeyConstraint, ECN::ECRelationshipClassCR ecRelationship);

        //! Indicates whether the specified ECRelationshipClass has the UseECInstanceIdAsForeignKey custom attribute or not.
        //! @param[in] relClass ECRelationshipClass to retrieve the custom attribute from.
        //! @return true if @p relClass has the custom attribute. false, if @p relClass doesn't have the custom attribute
        static bool HasUseECInstanceIdAsForeignKey(ECN::ECRelationshipClassCR relClass);
    };

//=======================================================================================    
//! ECDbSchemaMap is a convenience wrapper around the SchemaMap custom attribute that simplifies
//! reading the values of that custom attribute
//! @bsiclass
//=======================================================================================    
struct ECDbSchemaMap final
    {
    friend struct ECDbMapCustomAttributeHelper;

    private:
        ECN::ECSchemaCP m_schema;
        ECN::IECInstanceCP m_ca;

        ECDbSchemaMap(ECN::ECSchemaCR, ECN::IECInstanceCP ca);

    public:
        ECDbSchemaMap() : m_schema(nullptr), m_ca(nullptr) {}
        //! Tries to get the value of the TablePrefix property in the SchemaMap.
        //! @param[out] tablePrefix Table prefix. It remains unchanged, if the TablePrefix property wasn't set in the SchemaMap.
        //! @return SUCCESS if TablePrefix was set or unset in the SchemaMap. ERROR if TablePrefix didn't have a valid value,
        //! e.g. didn't comply to the naming conventions for table names.
        BentleyStatus TryGetTablePrefix(Utf8StringR tablePrefix) const;
    };

//=======================================================================================    
//! ECDbClassMap is a convenience wrapper around the ClassMap custom attribute that simplifies
//! reading the values of that custom attribute
//! @bsiclass
//=======================================================================================    
struct ECDbClassMap final
    {
    friend struct ECDbMapCustomAttributeHelper;

    private:
        ECN::ECClassCP m_class;
        ECN::IECInstanceCP m_ca;

        ECDbClassMap(ECN::ECClassCR, ECN::IECInstanceCP ca);

    public:
        ECDbClassMap() : m_class(nullptr), m_ca(nullptr) {}

        //! @return true if the ClassMap CA exists on the ECClass, false if it doesn't exist on the ECClass.
        bool IsValid() const { return m_class != nullptr && m_ca != nullptr; }

        //! Tries to get the value of the MapStrategy property from the ClassMap.
        //! @param[out] mapStrategy MapStrategy. It remains unchanged, if the MapStrategy property wasn't set in the ClassMap.
        //! @return SUCCESS if MapStrategy was set or unset in the ClassMap. ERROR otherwise
        BentleyStatus TryGetMapStrategy(Utf8String& mapStrategy) const;
        //! Tries to get the value of the TableName property in the ClassMap.
        //! @param[out] tableName Table name. It remains unchanged, if the TableName property wasn't set in the ClassMap.
        //! @return ECOBJECTSTATUS_Success if TableName was set or unset in the ClassMap, ERROR otherwise
        BentleyStatus TryGetTableName(Utf8String& tableName) const;
        //! Tries to get the value of the ECInstanceIdColumn property from the ClassMap.
        //! @param[out] ecInstanceIdColumnName Name of the ECInstanceId column. It remains unchanged, if the ECInstanceIdColumn property wasn't set in the ClassMap.
        //! @return ECOBJECTSTATUS_Success if ECInstanceIdColumn was set or unset in the ClassMap, ERROR otherwise
        BentleyStatus TryGetECInstanceIdColumn(Utf8String& ecInstanceIdColumnName) const;
    };

//=======================================================================================    
//! ShareColumns is a convenience wrapper around the ShareColumns custom attribute in the ECDbMap ECSchema
//! @bsiclass
//=======================================================================================    
struct ShareColumns final
    {
    friend struct ECDbMapCustomAttributeHelper;

    private:
        ECN::ECClassCP m_class;
        ECN::IECInstanceCP m_ca;

        ShareColumns(ECN::ECClassCR ecClass, ECN::IECInstanceCP ca) : m_class(&ecClass), m_ca(ca) {}

    public:
        ShareColumns() : m_class(nullptr), m_ca(nullptr) {}

        //! @return true if ShareColumns exists on the ECClass, false if it doesn't exist on the ECClass.
        bool IsValid() const { return m_class != nullptr && m_ca != nullptr; }

        //! Tries to get the value of the ApplyToSubclassesOnly property from the ShareColumns custom attribute.
        //! @param[out] applyToSubclassesOnly ApplyToSubclassesOnly flag. It remains unchanged, if the ApplyToSubclassesOnly property wasn't set.
        //! @return ECOBJECTSTATUS_Success if ApplyToSubclassesOnly was set or unset in the ShareColumns custom attribute, ERROR otherwise
        BentleyStatus TryGetApplyToSubclassesOnly(bool& applyToSubclassesOnly) const;

        //! Tries to get the value of the SharedColumnCount property from the ShareColumns custom attribute.
        //! The SharedColumnCount includes the overflow column.
        //! @param[out] sharedColumnCount Number of shared columns to use. It remains unchanged, if the SharedColumnCount property wasn't set.
        //! @return ECOBJECTSTATUS_Success if SharedColumnCount was set or unset in the ShareColumns custom attribute, ERROR otherwise
        BentleyStatus TryGetSharedColumnCount(int& sharedColumnCount) const;
    };

//=======================================================================================    
//! DbIndexList is a convenience wrapper around the DbIndexList custom attribute in the ECDbMap ECSchema
//! @bsiclass
//=======================================================================================    
struct DbIndexList final
    {
    friend struct ECDbMapCustomAttributeHelper;

    public:
        //=======================================================================================    
        //! DbIndex is a convenience wrapper around the DbIndex struct in the ECDbMap ECSchema
        //! that simplifies reading the values of that struct
        //! @bsiclass
        //=======================================================================================    
        struct DbIndex final
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
        ECN::ECClassCP m_class;
        ECN::IECInstanceCP m_ca;

        DbIndexList(ECN::ECClassCR, ECN::IECInstanceCP ca);

    public:
        DbIndexList() : m_class(nullptr), m_ca(nullptr) {}

        //! @return true if the ClassMap CA exists on the ECClass, false if it doesn't exist on the ECClass.
        bool IsValid() const { return m_class != nullptr && m_ca != nullptr; }

        //! Get the value of the Indexes property from the DbIndexList.
        //! @param[out] indexes List of DbIndexes as defined in the DbIndexList.
        //! @return SUCCESS or ERROR if Indexes property has invalid values
        BentleyStatus GetIndexes(bvector<DbIndex>& indexes) const;
    };

//=======================================================================================    
//! ECDbPropertyMap is a convenience wrapper around the PropertyMap custom attribute that simplifies
//! reading the values of that custom attribute
//! @bsiclass
//=======================================================================================    
struct ECDbPropertyMap final
    {
    friend struct ECDbMapCustomAttributeHelper;

    private:
        ECN::ECPropertyCP m_property;
        ECN::IECInstanceCP m_ca;

        ECDbPropertyMap(ECN::ECPropertyCR, ECN::IECInstanceCP ca);

    public:
        ECDbPropertyMap() : m_property(nullptr), m_ca(nullptr) {}

        //! Tries to get the value of the ColumnName property from the PropertyMap.
        //! @param[out] columnName Column name. It remains unchanged, if the ColumnName property wasn't set in the PropertyMap.
        //! @return ECOBJECTSTATUS_Success if ColumnName was set or unset in the PropertyMap, ERROR otherwise
        BentleyStatus TryGetColumnName(Utf8StringR columnName) const;
        //! Tries to get the value of the IsNullable property from the PropertyMap.
        //! @param[out] isNullable IsNullable flag. It remains unchanged, if the IsNullable property wasn't set in the PropertyMap.
        //! @return ECOBJECTSTATUS_Success if IsNullable was set or unset in the PropertyMap, ERROR otherwise
        BentleyStatus TryGetIsNullable(bool& isNullable) const;
        //! Tries to get the value of the IsUnique property from the PropertyMap.
        //! @param[out] isUnique IsUnique flag. It remains unchanged, if the IsUnique property wasn't set in the PropertyMap.
        //! @return ECOBJECTSTATUS_Success if IsUnique was set or unset in the PropertyMap, ERROR otherwise
        BentleyStatus TryGetIsUnique(bool& isUnique) const;
        //! Tries to get the value of the Collation property from the PropertyMap.
        //! @param[out] collation Collation. It remains unchanged, if the Collation property wasn't set in the PropertyMap.
        //! @return ECOBJECTSTATUS_Success if Collation was set or unset in the PropertyMap, ERROR otherwise
        BentleyStatus TryGetCollation(Utf8StringR collation) const;
    };


//=======================================================================================    
//! ECDbLinkTableRelationshipMap is a convenience wrapper around the LinkTableRelationshipMap 
//! custom attribute that simplifies reading the values of that custom attribute
//! @bsiclass
//=======================================================================================    
struct ECDbLinkTableRelationshipMap final
    {
    friend struct ECDbMapCustomAttributeHelper;

    private:
        ECN::ECRelationshipClassCP m_relClass;
        ECN::IECInstanceCP m_ca;

        ECDbLinkTableRelationshipMap(ECN::ECRelationshipClassCR, ECN::IECInstanceCP ca);

    public:
        ECDbLinkTableRelationshipMap() : m_relClass(nullptr), m_ca(nullptr) {}

        //! Tries to get the value of the SourceECInstanceId property from the LinkTableRelationshipMap.
        //! @param[out] sourceECInstanceIdColumnName Name of column to which SourceECInstanceId is mapped to. 
        //! It remains unchanged, if the SourceECInstanceId property wasn't set in the LinkTableRelationshipMap.
        //! @return ECOBJECTSTATUS_Success if SourceECInstanceId was set or unset in the LinkTableRelationshipMap, ERROR otherwise
        BentleyStatus TryGetSourceECInstanceIdColumn(Utf8StringR sourceECInstanceIdColumnName) const;

        //! Tries to get the value of the TargetECInstanceId property from the LinkTableRelationshipMap.
        //! @param[out] targetECInstanceIdColumnName Name of column to which TargetECInstanceId is mapped to. 
        //! It remains unchanged, if the TargetECInstanceId property wasn't set in the LinkTableRelationshipMap.
        //! @return ECOBJECTSTATUS_Success if TargetECInstanceId was set or unset in the LinkTableRelationshipMap, ERROR otherwise
        BentleyStatus TryGetTargetECInstanceIdColumn(Utf8StringR targetECInstanceIdColumnName) const;

        //! Tries to get the value of the AllowDuplicateRelationships property from the LinkTableRelationshipMap.
        //! @param[out] allowDuplicateRelationships AllowDuplicateRelationships flag. It remains unchanged, if the AllowDuplicateRelationships property wasn't set in the LinkTableRelationshipMap.
        //! @return ECOBJECTSTATUS_Success if AllowDuplicateRelationships was set or unset in the LinkTableRelationshipMap, ERROR otherwise
        BentleyStatus TryGetAllowDuplicateRelationships(bool& allowDuplicateRelationships) const;
    };

//=======================================================================================    
//! ECDbForeignKeyConstraint is a convenience wrapper around the ForeignKeyConstraint 
//! custom attribute that simplifies reading the values of that custom attribute
//! @bsiclass
//=======================================================================================    
struct ECDbForeignKeyConstraint final
    {
    friend struct ECDbMapCustomAttributeHelper;

    private:
        ECN::ECRelationshipClassCP m_relClass;
        ECN::IECInstanceCP m_ca;

        ECDbForeignKeyConstraint(ECN::ECRelationshipClassCR, ECN::IECInstanceCP ca);

    public:
        ECDbForeignKeyConstraint() : m_relClass(nullptr), m_ca(nullptr) {}

        //! Tries to get the value of the OnDeleteAction property from the ForeignKeyConstraint.
        //! @param[out] onDeleteAction OnDelete action.  @p onDeleteAction remains unchanged, if the OnDeleteAction property 
        //! wasn't set in the ForeignKeyConstraint.
        //! @return ECOBJECTSTATUS_Success if OnDeleteAction was set or unset in the ForeignKeyConstraint, ERROR otherwise
        BentleyStatus TryGetOnDeleteAction(Utf8StringR onDeleteAction) const;

        //! Tries to get the value of the OnUpdateAction property from the ForeignKeyConstraint.
        //! @param[out] onUpdateAction OnUpdate action. @p onDeleteAction remains unchanged, if the OnUpdateAction property 
        //! wasn't set in the ForeignKeyConstraint.
        //! @return ECOBJECTSTATUS_Success if OnUpdateAction was set or unset in the ForeignKeyConstraint, ERROR otherwise
        BentleyStatus TryGetOnUpdateAction(Utf8StringR onUpdateAction) const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
