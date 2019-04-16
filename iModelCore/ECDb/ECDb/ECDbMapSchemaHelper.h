/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <Bentley/Nullable.h>
#include <ECObjects/ECObjectsAPI.h>
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct SchemaMapCustomAttribute;
struct ClassMapCustomAttribute;
struct ShareColumnsCustomAttribute;
struct DbIndexListCustomAttribute;
struct PropertyMapCustomAttribute;
struct LinkTableRelationshipMapCustomAttribute;
struct ForeignKeyConstraintCustomAttribute;

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
        static bool TryGetSchemaMap(SchemaMapCustomAttribute& schemaMap, ECN::ECSchemaCR schema);

        //! Tries to retrieve the ClassMap custom attribute from the specified ECClass.
        //! @param[out] classMap Retrieved class map
        //! @param[in] ecClass ECClass to retrieve the custom attribute from.
        //! @return true if @p ecClass has the custom attribute. false, if @p ecClass doesn't have the custom attribute
        static bool TryGetClassMap(ClassMapCustomAttribute& classMap, ECN::ECClassCR ecClass);

        //! Tries to retrieve the ShareColumns custom attribute from the specified ECClass.
        //! @param[out] shareColumns Retrieved ShareColumns
        //! @param[in] ecClass ECClass to retrieve the custom attribute from.
        //! @return true if @p ecClass has the custom attribute. false, if @p ecClass doesn't have the custom attribute
        static bool TryGetShareColumns(ShareColumnsCustomAttribute& shareColumns, ECN::ECClassCR ecClass);

        //! Indicates whether the specified ECClass has the JoinedTablePerDirectSubclass custom attribute or not.
        //! @param[in] ecClass ECClass to retrieve the custom attribute from.
        //! @return true if @p ecClass has the custom attribute. false, if @p ecClass doesn't have the custom attribute
        static bool HasJoinedTablePerDirectSubclass(ECN::ECEntityClassCR ecClass);

        //! Tries to retrieve the DbIndexList custom attribute from the specified ECClass.
        //! @param[out] dbIndexList Retrieved property map
        //! @param[in] ecClass ECClass to retrieve the custom attribute from.
        //! @return true if @p ecClass has the custom attribute. false, if @p ecClass doesn't have the custom attribute
        static bool TryGetDbIndexList(DbIndexListCustomAttribute& dbIndexList, ECN::ECClassCR ecClass);

        //! Tries to retrieve the PropertyMap custom attribute from the specified ECProperty.
        //! @param[out] propertyMap Retrieved property map
        //! @param[in] ecProperty ECProperty to retrieve the custom attribute from.
        //! @return true if @p ecProperty has the custom attribute. false, if @p ecProperty doesn't have the custom attribute
        static bool TryGetPropertyMap(PropertyMapCustomAttribute& propertyMap, ECN::PrimitiveECPropertyCR ecProperty);

        //! Tries to retrieve the LinkTableRelationshipMap custom attribute from the specified ECRelationshipClass.
        //! @param[out] linkTableRelationshipMap Retrieved link table relationship map
        //! @param[in] ecRelationship ECRelationshipClass to retrieve the custom attribute from.
        //! @return true if @p ecRelationship has the custom attribute. false, if @p ecRelationship doesn't have the custom attribute
        static bool TryGetLinkTableRelationshipMap(LinkTableRelationshipMapCustomAttribute& linkTableRelationshipMap, ECN::ECRelationshipClassCR ecRelationship);

        //! Tries to retrieve the ForeignKeyConstraint custom attribute from the specified navigation property.
        //! @param[out] foreignKeyConstraint Retrieved foreign key constraint CA
        //! @param[in] navProp Navigation property to retrieve the custom attribute from.
        //! @return true if @p navProp has the custom attribute. false, if @p navProp doesn't have the custom attribute
        static bool TryGetForeignKeyConstraint(ForeignKeyConstraintCustomAttribute& foreignKeyConstraint, ECN::NavigationECPropertyCR navProp);
    };

//=======================================================================================    
//! SchemaMapCustomAttribute is a convenience wrapper around the SchemaMap custom attribute that simplifies
//! reading the values of that custom attribute
//! @bsiclass
//=======================================================================================    
struct SchemaMapCustomAttribute final
    {
    friend struct ECDbMapCustomAttributeHelper;

    private:
        ECN::ECSchemaCP m_schema = nullptr;
        ECN::IECInstancePtr m_ca = nullptr;

        SchemaMapCustomAttribute(ECN::ECSchemaCR schema, ECN::IECInstancePtr ca) : m_schema(&schema), m_ca(ca) {}

    public:
        SchemaMapCustomAttribute() {}
        //! Tries to get the value of the TablePrefix property in the SchemaMap.
        //! @param[out] tablePrefix Table prefix. IsNull() is true, if the TablePrefix property wasn't set in the SchemaMap.
        //! @return SUCCESS if TablePrefix was set or unset in the SchemaMap. ERROR if TablePrefix didn't have a valid value,
        //! e.g. didn't comply to the naming conventions for table names.
        BentleyStatus TryGetTablePrefix(Nullable<Utf8String>& tablePrefix) const;
    };

//=======================================================================================    
//! ClassMapCustomAttribute is a convenience wrapper around the ClassMap custom attribute that simplifies
//! reading the values of that custom attribute
//! @bsiclass
//=======================================================================================    
struct ClassMapCustomAttribute final
    {
    friend struct ECDbMapCustomAttributeHelper;

    private:
        ECN::ECClassCP m_class = nullptr;
        ECN::IECInstancePtr m_ca = nullptr;

        ClassMapCustomAttribute(ECN::ECClassCR ecClass, ECN::IECInstancePtr ca) : m_class(&ecClass), m_ca(ca) {}

    public:
        ClassMapCustomAttribute() {}

        //! @return true if the ClassMap CA exists on the ECClass, false if it doesn't exist on the ECClass.
        bool IsValid() const { return m_class != nullptr && m_ca != nullptr; }

        //! Tries to get the value of the MapStrategy property from the ClassMap.
        //! @param[out] mapStrategy MapStrategy. IsNull() is true, if the MapStrategy property wasn't set in the ClassMap.
        //! @return SUCCESS if MapStrategy was set or unset in the ClassMap. ERROR otherwise
        BentleyStatus TryGetMapStrategy(Nullable<Utf8String>& mapStrategy) const;
        //! Tries to get the value of the TableName property in the ClassMap.
        //! @param[out] tableName Table name. IsNull() is true, if the TableName property wasn't set in the ClassMap.
        //! @return SUCCESS if TableName was set or unset in the ClassMap, ERROR otherwise
        BentleyStatus TryGetTableName(Nullable<Utf8String>& tableName) const;
        //! Tries to get the value of the ECInstanceIdColumn property from the ClassMap.
        //! @param[out] ecInstanceIdColumnName Name of the ECInstanceId column. IsNull() is true, if the ECInstanceIdColumn property wasn't set in the ClassMap.
        //! @return SUCCESS if ECInstanceIdColumn was set or unset in the ClassMap, ERROR otherwise
        BentleyStatus TryGetECInstanceIdColumn(Nullable<Utf8String>& ecInstanceIdColumnName) const;
    };

//=======================================================================================    
//! ShareColumnsCustomAttribute is a convenience wrapper around the ShareColumns custom attribute in the ECDbMap ECSchema
//! @bsiclass
//=======================================================================================    
struct ShareColumnsCustomAttribute final
    {
    friend struct ECDbMapCustomAttributeHelper;

    private:
        ECN::ECClassCP m_class = nullptr;
        ECN::IECInstancePtr m_ca = nullptr;

        ShareColumnsCustomAttribute(ECN::ECClassCR ecClass, ECN::IECInstancePtr ca) : m_class(&ecClass), m_ca(ca) {}

    public:
        ShareColumnsCustomAttribute() {}

        //! @return true if ShareColumns exists on the ECClass, false if it doesn't exist on the ECClass.
        bool IsValid() const { return m_class != nullptr && m_ca != nullptr; }

        //! Tries to get the value of the ApplyToSubclassesOnly property from the ShareColumns custom attribute.
        //! @param[out] applyToSubclassesOnly ApplyToSubclassesOnly flag. IsNull() is true, if the ApplyToSubclassesOnly property wasn't set.
        //! @return SUCCESS if ApplyToSubclassesOnly was set or unset in the ShareColumns custom attribute, ERROR otherwise
        BentleyStatus TryGetApplyToSubclassesOnly(Nullable<bool>& applyToSubclassesOnly) const;

        //! Tries to get the value of the MaxSharedColumnsBeforeOverflow property from the ShareColumns custom attribute.
        //! @param[out] maxSharedColumnsBeforeOverflow Maximum number of shared columns to use before creating an overflow table. IsNull() is true, if the MaxSharedColumnsBeforeOverflow property wasn't set.
        //! @return SUCCESS if MaxSharedColumnsBeforeOverflow was set or unset in the ShareColumns custom attribute, ERROR otherwise
        BentleyStatus TryGetMaxSharedColumnsBeforeOverflow(Nullable<uint32_t>& maxSharedColumnsBeforeOverflow) const;
    };

//=======================================================================================    
//! DbIndexListCustomAttribute is a convenience wrapper around the DbIndexList custom attribute in the ECDbMap ECSchema
//! @bsiclass
//=======================================================================================    
struct DbIndexListCustomAttribute final
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
            friend struct DbIndexListCustomAttribute;

            private:
                Utf8String m_name;
                bool m_isUnique = false;
                std::vector<Utf8String> m_properties;
                Nullable<Utf8String> m_whereClause;

                DbIndex(Utf8StringCR name, bool isUnique, Nullable<Utf8String> const& whereClause) : m_name(name), m_isUnique(isUnique), m_whereClause(whereClause) {}

            public:
                DbIndex() {}
                DbIndex(Utf8StringCR name, bool isUnique, std::vector<Utf8String> const& props, Nullable<Utf8String> const& whereClause) : m_name(name), m_isUnique(isUnique), m_properties(props), m_whereClause(whereClause) {}
                DbIndex(DbIndex const& rhs) : m_name(rhs.m_name), m_isUnique(rhs.m_isUnique), m_properties(rhs.m_properties), m_whereClause(rhs.m_whereClause){}
                DbIndex(DbIndex&& rhs) : m_name(std::move(rhs.m_name)), m_isUnique(std::move(rhs.m_isUnique)), m_properties(std::move(rhs.m_properties)), m_whereClause(std::move(rhs.m_whereClause)) {}

                bool IsValid() const { return !m_name.empty(); }

                DbIndex& operator=(DbIndex const& rhs)
                    {
                    if (this != &rhs)
                        {
                        m_name.assign(rhs.m_name);
                        m_isUnique = rhs.m_isUnique;
                        m_properties = rhs.m_properties;
                        m_whereClause = rhs.m_whereClause;
                        }

                    return *this;
                    }

                DbIndex& operator=(DbIndex&& rhs)
                    {
                    if (this != &rhs)
                        {
                        m_name = std::move(rhs.m_name);
                        m_isUnique = std::move(rhs.m_isUnique);
                        m_properties = std::move(rhs.m_properties);
                        m_whereClause = std::move(rhs.m_whereClause);
                        }

                    return *this;
                    }

                //! Gets the index name.
                //! @return Index name.
                Utf8StringCR GetName() const { return m_name; }
                //! Gets a value indicating whether the index is a unique index or not.
                //! @return true if the index is a unique index. false otherwise
                bool IsUnique() const { return m_isUnique; }
                //! Gets the where clause if the index a partial index.
                //! @return Where clause or nullptr if the index is not partial
                Nullable<Utf8String> const& GetWhereClause() const { return m_whereClause; }
                //! Gets the list of property names on which the index is to be defined.
                //! @return Properties on which the index is defined.
                std::vector<Utf8String> const& GetProperties() const { return m_properties; }
            };

    private:
        ECN::ECClassCP m_class = nullptr;
        ECN::IECInstancePtr m_ca = nullptr;

        DbIndexListCustomAttribute(ECN::ECClassCR ecClass, ECN::IECInstancePtr ca) : m_class(&ecClass), m_ca(ca) {}

    public:
        DbIndexListCustomAttribute() {}

        //! @return true if the ClassMap CA exists on the ECClass, false if it doesn't exist on the ECClass.
        bool IsValid() const { return m_class != nullptr && m_ca != nullptr; }

        //! Get the value of the Indexes property from the DbIndexList.
        //! @param[out] indexes List of DbIndexes as defined in the DbIndexList.
        //! @return SUCCESS or ERROR if Indexes property has invalid values
        BentleyStatus GetIndexes(bvector<DbIndex>& indexes) const;
    };

//=======================================================================================    
//! PropertyMapCustomAttribute is a convenience wrapper around the PropertyMap custom attribute that simplifies
//! reading the values of that custom attribute
//! @bsiclass
//=======================================================================================    
struct PropertyMapCustomAttribute final
    {
    friend struct ECDbMapCustomAttributeHelper;

    private:
        ECN::PrimitiveECPropertyCP m_property = nullptr;
        ECN::IECInstancePtr m_ca = nullptr;

        PropertyMapCustomAttribute(ECN::PrimitiveECPropertyCR prop, ECN::IECInstancePtr ca) : m_property(&prop), m_ca(ca) {}

    public:
        PropertyMapCustomAttribute() {}

        //! Tries to get the value of the ColumnName property from the PropertyMap.
        //! @param[out] columnName Column name. IsNull() is true, if the ColumnName property wasn't set in the PropertyMap.
        //! @return SUCCESS if ColumnName was set or unset in the PropertyMap, ERROR otherwise
        BentleyStatus TryGetColumnName(Nullable<Utf8String>& columnName) const;
        //! Tries to get the value of the IsNullable property from the PropertyMap.
        //! @param[out] isNullable IsNullable flag. IsNull() is true, if the IsNullable property wasn't set in the PropertyMap.
        //! @return SUCCESS if IsNullable was set or unset in the PropertyMap, ERROR otherwise
        BentleyStatus TryGetIsNullable(Nullable<bool>& isNullable) const;
        //! Tries to get the value of the IsUnique property from the PropertyMap.
        //! @param[out] isUnique IsUnique flag. IsNull() is true, if the IsUnique property wasn't set in the PropertyMap.
        //! @return SUCCESS if IsUnique was set or unset in the PropertyMap, ERROR otherwise
        BentleyStatus TryGetIsUnique(Nullable<bool>& isUnique) const;
        //! Tries to get the value of the Collation property from the PropertyMap.
        //! @param[out] collation Collation. IsNull() is true, if the Collation property wasn't set in the PropertyMap.
        //! @return SUCCESS if Collation was set or unset in the PropertyMap, ERROR otherwise
        BentleyStatus TryGetCollation(Nullable<Utf8String>& collation) const;
    };

//=======================================================================================    
//! ForeignKeyConstraintCustomAttribute is a convenience wrapper around the ForeignKeyConstraint 
//! custom attribute that simplifies reading the values of that custom attribute
//! @bsiclass
//=======================================================================================    
struct ForeignKeyConstraintCustomAttribute final
    {
    friend struct ECDbMapCustomAttributeHelper;

    private:
        ECN::NavigationECPropertyCP m_navProp = nullptr;
        ECN::IECInstancePtr m_ca = nullptr;

        ForeignKeyConstraintCustomAttribute(ECN::NavigationECPropertyCR navProp, ECN::IECInstancePtr ca) : m_navProp(&navProp), m_ca(ca) {}

    public:
        ForeignKeyConstraintCustomAttribute() {}

        bool operator==(ForeignKeyConstraintCustomAttribute const& rhs) const;
        bool operator!=(ForeignKeyConstraintCustomAttribute const& rhs) const { return !(*this == rhs); }

        //! @return true if the ForeignKeyConstraint CA exists on the navigation property, false if it doesn't exist on the navigation property.
        bool IsValid() const { return m_navProp != nullptr && m_ca != nullptr; }

        //! Tries to get the value of the OnDeleteAction property from the ForeignKeyConstraint.
        //! @param[out] onDeleteAction OnDelete action. IsNull() is true, if the OnDeleteAction property 
        //! wasn't set in the ForeignKeyConstraint.
        //! @return SUCCESS if OnDeleteAction was set or unset in the ForeignKeyConstraint, ERROR otherwise
        BentleyStatus TryGetOnDeleteAction(Nullable<Utf8String>& onDeleteAction) const;

        //! Tries to get the value of the OnUpdateAction property from the ForeignKeyConstraint.
        //! @param[out] onUpdateAction OnUpdate action. IsNull() is true, if the OnUpdateAction property 
        //! wasn't set in the ForeignKeyConstraint.
        //! @return SUCCESS if OnUpdateAction was set or unset in the ForeignKeyConstraint, ERROR otherwise
        BentleyStatus TryGetOnUpdateAction(Nullable<Utf8String>& onUpdateAction) const;
    };


//=======================================================================================    
//! LinkTableRelationshipMapCustomAttribute is a convenience wrapper around the LinkTableRelationshipMap 
//! custom attribute that simplifies reading the values of that custom attribute
//! @bsiclass
//=======================================================================================    
struct LinkTableRelationshipMapCustomAttribute final
    {
    friend struct ECDbMapCustomAttributeHelper;

    private:
        ECN::ECRelationshipClassCP m_relClass = nullptr;
        ECN::IECInstancePtr m_ca = nullptr;

        LinkTableRelationshipMapCustomAttribute(ECN::ECRelationshipClassCR relClass, ECN::IECInstancePtr ca) : m_relClass(&relClass), m_ca(ca) {}

    public:
        LinkTableRelationshipMapCustomAttribute() {}

        //! @return true if the LinkTableRelationshipMap CA exists on the ECClass, false if it doesn't exist on the ECClass.
        bool IsValid() const { return m_relClass != nullptr && m_ca != nullptr; }

        //! Tries to get the value of the SourceECInstanceId property from the LinkTableRelationshipMap.
        //! @param[out] sourceECInstanceIdColumnName Name of column to which SourceECInstanceId is mapped to. 
        //! IsNull() is true, if the SourceECInstanceId property wasn't set in the LinkTableRelationshipMap.
        //! @return SUCCESS if SourceECInstanceId was set or unset in the LinkTableRelationshipMap, ERROR otherwise
        BentleyStatus TryGetSourceECInstanceIdColumn(Nullable<Utf8String>& sourceECInstanceIdColumnName) const;

        //! Tries to get the value of the TargetECInstanceId property from the LinkTableRelationshipMap.
        //! @param[out] targetECInstanceIdColumnName Name of column to which TargetECInstanceId is mapped to. 
        //! IsNull() is true, if the TargetECInstanceId property wasn't set in the LinkTableRelationshipMap.
        //! @return SUCCESS if TargetECInstanceId was set or unset in the LinkTableRelationshipMap, ERROR otherwise
        BentleyStatus TryGetTargetECInstanceIdColumn(Nullable<Utf8String>& targetECInstanceIdColumnName) const;

        //! Tries to get the value of the CreateForeignKeyConstraints property from the LinkTableRelationshipMap.
        //! @param[out] createForeignKeyConstraints CreateForeignKeyConstraints flag. IsNull() is true, if the CreateForeignKeyConstraints property wasn't set in the LinkTableRelationshipMap.
        //! @return SUCCESS if CreateForeignKeyConstraints was set or unset in the LinkTableRelationshipMap, ERROR otherwise
        BentleyStatus TryGetCreateForeignKeyConstraints(Nullable<bool>& createForeignKeyConstraints) const;

        //! Tries to get the value of the AllowDuplicateRelationships property from the LinkTableRelationshipMap.
        //! @param[out] allowDuplicateRelationships AllowDuplicateRelationships flag. IsNull() is true, if the AllowDuplicateRelationships property wasn't set in the LinkTableRelationshipMap.
        //! @return SUCCESS if AllowDuplicateRelationships was set or unset in the LinkTableRelationshipMap, ERROR otherwise
        BentleyStatus TryGetAllowDuplicateRelationships(Nullable<bool>& allowDuplicateRelationships) const;

    };

//*****************************************************************
//CustomAttributeReader
//*****************************************************************
struct CustomAttributeReader final
    {
    private:
        CustomAttributeReader();
        ~CustomAttributeReader();

        static BentleyStatus TryGetTrimmedValue(Nullable<Utf8String>& strVal, ECN::ECValueCR val);

    public:
        static ECN::IECInstancePtr Read(ECN::IECCustomAttributeContainer const& caContainer, Utf8CP customAttributeSchemaName, Utf8CP customAttributeName) { return caContainer.GetCustomAttributeLocal(customAttributeSchemaName, customAttributeName); }
        static BentleyStatus TryGetTrimmedValue(Nullable<Utf8String>& val, ECN::IECInstanceCR ca, Utf8CP ecPropertyAccessString);
        static BentleyStatus TryGetTrimmedValue(Nullable<Utf8String>& val, ECN::IECInstanceCR ca, uint32_t propIndex, uint32_t arrayIndex);
        static BentleyStatus TryGetIntegerValue(Nullable<int>&, ECN::IECInstanceCR ca, Utf8CP ecPropertyAccessString);
        static BentleyStatus TryGetBooleanValue(Nullable<bool>&, ECN::IECInstanceCR ca, Utf8CP ecPropertyAccessString);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
