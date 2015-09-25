/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaPersistence.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbInfoBase
    {
    public:
        union
            {
            uint32_t ColsInsert;
            uint32_t ColsWhere;
            };
        union
            {
            uint32_t ColsUpdate;
            uint32_t ColsSelect;
            };
        uint32_t ColsNull;
        DbInfoBase() : ColsNull(0), ColsInsert(0), ColsUpdate(0) {}
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECSchemaInfo : DbInfoBase
    {
    public:
        enum Columns
            {
            COL_Id = 0x001,
            COL_Name = 0x002,
            COL_Description = 0x004,
            COL_NamespacePrefix = 0x008,
            COL_VersionMajor = 0x010,
            COL_VersionMinor = 0x020,
            COL_DisplayLabel = 0x100,
            COL_ALL = 0xffffffff
            };
        ECSchemaId          m_ecSchemaId;
        Utf8String          m_name;
        Utf8String          m_displayLabel;
        Utf8String          m_description;
        Utf8String          m_namespacePrefix;
        int32_t             m_versionMajor;
        int32_t             m_versionMinor;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECClassInfo : DbInfoBase
    {
    public:
        enum Columns
            {
            COL_Id = 0x000001,
            COL_Name = 0x000002,
            COL_Description = 0x000004,
            COL_IsDomainClass = 0x000008,
            COL_IsStruct = 0x000010,
            COL_IsCustomAttribute = 0x000020,
            COL_RelationStrength = 0x000800,
            COL_DisplayLabel = 0x004000,
            COL_SchemaId = 0x008000,
            COL_IsRelationship = 0x010000,
            COL_RelationStrengthDirection = 0x020000,
            COL_ALL = 0xffffffff
            };
        ECClassId                  m_ecClassId;
        ECSchemaId                 m_ecSchemaId;
        Utf8String                 m_name;
        Utf8String                 m_displayLabel;
        Utf8String                 m_description;
        bool                       m_isDomainClass;
        bool                       m_isStruct;
        bool                       m_isCustomAttribute;
        StrengthType               m_relationStrength;
        ECRelatedInstanceDirection m_relationStrengthDirection;
        bool                       m_isRelationship;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbBaseClassInfo : DbInfoBase
    {
    public:
        enum Columns
            {
            COL_ClassId = 0x1,
            COL_BaseClassId = 0x2,
            COL_Ordinal = 0x4,
            COL_ALL = 0xffffffff
            };
        ECClassId m_ecClassId;
        ECClassId m_baseECClassId;
        int32_t   m_ecIndex;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECPropertyInfo : DbInfoBase
    {
    public:
        enum Columns
            {
            COL_Id = 0x0001,
            COL_ClassId = 0x0002,
            COL_Name = 0x0004,
            COL_DisplayLabel = 0x0008,
            COL_Description = 0x0010,
            COL_IsArray = 0x0020,
            COL_PrimitiveType = 0x0040,
            COL_StructType = 0x0080,
            COL_Ordinal = 0x0100,
            COL_IsReadonly = 0x0200,
            COL_MinOccurs = 0x1000,
            COL_MaxOccurs = 0x2000,

            COL_ALL = 0xffffffff
            };

        ECClassId     m_ecClassId;
        ECPropertyId  m_ecPropertyId;
        Utf8String    m_name;
        Utf8String    m_displayLabel;
        Utf8String    m_description;
        bool          m_isArray;
        PrimitiveType m_primitiveType;
        ECClassId     m_structType;
        uint32_t      m_minOccurs;
        uint32_t      m_maxOccurs;
        int32_t       m_ordinal;

        bool          m_isReadOnly;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECRelationshipConstraintInfo : DbInfoBase
    {
    public:
        enum Columns
            {
            COL_RelationshipClassId = 0x01,
            COL_RelationshipEnd = 0x02,
            COL_CardinalityLowerLimit = 0x04,
            COL_CardinalityUpperLimit = 0x08,
            COL_RoleLabel = 0x10,
            COL_IsPolymorphic = 0x20,
            COL_ALL = 0xffffffff
            };

        ECClassId         m_relationshipClassId;
        ECRelationshipEnd m_ecRelationshipEnd;
        uint32_t          m_cardinalityLowerLimit;
        uint32_t          m_cardinalityUpperLimit;
        Utf8String        m_roleLabel;
        bool              m_isPolymorphic;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECRelationshipConstraintClassInfo : DbInfoBase
    {
    public:
        enum Columns
            {
            COL_RelationshipClassId = 0x01,
            COL_ConstraintClassId = 0x02,
            COL_RelationshipEnd = 0x04,
            COL_ALL = 0xffffffff
            };

        ECClassId         m_relationshipClassId;
        ECClassId         m_constraintClassId;
        ECRelationshipEnd m_ecRelationshipEnd;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECRelationshipConstraintClassKeyPropertyInfo : DbInfoBase
    {
    public:
        enum Columns
            {
            COL_RelationshipClassId = 0x01,
            COL_ConstraintClassId = 0x02,
            COL_RelationshipEnd = 0x04,
            COL_KeyPropertyName = 0x08,
            COL_ALL = 0xffffffff
            };

        ECClassId         m_relationECClassId;
        ECClassId         m_constraintClassId;
        ECRelationshipEnd m_ecRelationshipEnd;
        Utf8String        m_keyPropertyName;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbCustomAttributeInfo : DbInfoBase
    {
    public:
        enum Columns
            {
            COL_ContainerId = 0x01,
            COL_ContainerType = 0x02,
            COL_ClassId = 0x04,
            COL_Ordinal = 0x08,
            COL_Instance = 0x10,
            COL_ALL = 0xffffffff
            };

    private:
        Utf8String        m_caInstanceXml;

    public:
        //TODO should be made private eventually, too
        ECContainerId     m_containerId;
        ECContainerType   m_containerType;
        ECClassId         m_ecClassId;
        int32_t           m_index;

        BentleyStatus     SerializeCaInstance(IECInstanceR caInstance);
        BentleyStatus     DeserializeCaInstance(IECInstancePtr& caInstance, ECSchemaCR schema) const;

        //! Sets the CA instance XML string in this object.
        void SetCaInstanceXml(Utf8CP caInstanceXml);

        //! Gets the CA instance XML string
        Utf8CP GetCaInstanceXml() const;
        void Clear()
            {
            m_containerId = 0LL;
            m_containerType = ECContainerType::Schema;
            m_ecClassId = ECN::ECClass::UNSET_ECCLASSID;
            m_caInstanceXml.clear();
            }
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECSchemaReferenceInfo : DbInfoBase
    {
    public:
        enum Columns
            {
            COL_SchemaId = 0x01,
            COL_ReferencedSchemaId = 0x02,
            COL_ALL = 0xffffffff
            };

        ECSchemaId m_ecSchemaId;
        ECSchemaId m_referencedECSchemaId;
    };
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECClassEntry
    {
public:
    ECClassId    m_ecClassId;
    ECSchemaId   m_ecSchemaId;
    Utf8String   m_className;
    ECN::ECClassP m_resolvedECClass;
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECSchemaEntry
    {
public:
    ECSchemaId       m_ecSchemaId;
    Utf8String       m_schemaName;
    uint32_t         m_versionMajor;
    uint32_t         m_versionMinor;
    uint32_t         m_nClassesInSchema; //This is read from Db
    uint32_t         m_nClassesLoaded;   //Every time a class is loaded from db for this schema it is incremented
    ECN::ECSchemaPtr  m_resolvedECSchema;    //Contain ECSchema which might be not complete
    bool             IsFullyLoaded() {return m_nClassesInSchema == m_nClassesLoaded;} 
    };



/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbSchemaPersistence
    {
    private:
        static int ToInt(ECN::ECRelatedInstanceDirection direction);
        static ECN::ECRelatedInstanceDirection ToECRelatedInstanceDirection(int relatedInstanceDirection);


    public:
        typedef bvector<ECN::ECClassId>   ECClassIdList;
        typedef ECClassIdList*            ECClassIdListP;
        typedef ECClassIdList&            ECClassIdListR;
        typedef bvector<DbECSchemaEntry>  ECSchemaKeyList;
        typedef ECSchemaKeyList&          ECSchemaKeyListR;
        typedef ECSchemaKeyList*          ECSchemaKeyListP;

        //Insert new item
        static  DbResult InsertECSchema(ECDbCR, DbECSchemaInfo const&);
        static  BentleyStatus InsertECClass(ECDbCR, DbECClassInfo const&);
        static  BentleyStatus InsertBaseClass(ECDbCR, DbBaseClassInfo const&);
        static  BentleyStatus InsertECProperty(ECDbCR, DbECPropertyInfo const&);
        static  BentleyStatus InsertECRelationshipConstraint(ECDbCR, DbECRelationshipConstraintInfo const&);
        static  BentleyStatus InsertECRelationshipConstraintClass(ECDbCR, DbECRelationshipConstraintClassInfo const&);
        static  BentleyStatus InsertECRelationshipConstraintClassKeyProperty(ECDbCR, DbECRelationshipConstraintClassKeyPropertyInfo const&);
        static  BentleyStatus InsertCustomAttribute(ECDbCR, DbCustomAttributeInfo const&);
        static  BentleyStatus InsertECSchemaReference(ECDbCR, DbECSchemaReferenceInfo const&);

        //Find ECSchemaInfo
        static  BentleyStatus FindECSchema(BeSQLite::CachedStatementPtr&, ECDbCR, DbECSchemaInfo const&);
        static  BeSQLite::DbResult Step(DbECSchemaInfo&, BeSQLite::Statement&);
        //Find ECClassInfo
        static  BentleyStatus FindECClass(BeSQLite::CachedStatementPtr&, ECDbCR, DbECClassInfo const&);
        static  BeSQLite::DbResult Step(DbECClassInfo&, BeSQLite::Statement&);
        //Find BaseClass
        static  BentleyStatus FindBaseClass(BeSQLite::CachedStatementPtr&, ECDbCR, DbBaseClassInfo const&);
        static  BeSQLite::DbResult Step(DbBaseClassInfo&, BeSQLite::Statement&);
        //Find ECPropertyInfo
        static  BentleyStatus FindECProperty(BeSQLite::CachedStatementPtr&, ECDbCR, DbECPropertyInfo const&);
        static  BeSQLite::DbResult Step(DbECPropertyInfo&, BeSQLite::Statement&);
        //Find ECRelationConstraintInfo 
        static  BentleyStatus FindECRelationshipConstraint(BeSQLite::CachedStatementPtr&, ECDbCR, DbECRelationshipConstraintInfo const&);
        static  BeSQLite::DbResult Step(DbECRelationshipConstraintInfo&, BeSQLite::Statement&);
        //Find ECRelationConstraintInfo 
        static  BentleyStatus FindECRelationshipConstraintClass(BeSQLite::CachedStatementPtr&, ECDbCR, DbECRelationshipConstraintClassInfo const&);
        static  BeSQLite::DbResult Step(DbECRelationshipConstraintClassInfo&, BeSQLite::Statement&);
        //Find CustomAttributeInfo
        static  BentleyStatus FindCustomAttribute(BeSQLite::CachedStatementPtr&, ECDbCR, DbCustomAttributeInfo const&);
        static  BeSQLite::DbResult Step(DbCustomAttributeInfo&, BeSQLite::Statement&);
        //Find CustomAttributeInfo
        static  BentleyStatus FindECSchemaReference(BeSQLite::CachedStatementPtr& stmt, ECDbCR, DbECSchemaReferenceInfo const&);
        static  BeSQLite::DbResult Step(DbECSchemaReferenceInfo&, BeSQLite::Statement& stmt);

        //Helper
        static                bool ContainsECSchemaReference(ECDbCR, ECSchemaId ecPrimarySchemaId, ECSchemaId ecReferencedSchemaId);
        static                bool ContainsECClass(ECDbCR, ECClassCR);
        static                bool ContainsECSchemaWithId(ECDbCR, ECSchemaId);
        static          ECSchemaId GetECSchemaId(ECDbCR, Utf8CP schemaName);
        static          ECSchemaId GetECSchemaId(ECDbCR, ECSchemaCR);
        static           ECClassId GetECClassId(ECDbCR, Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema);
        static        ECPropertyId GetECPropertyId(ECDbCR, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName);

        static  BentleyStatus InitializeSystemTables(ECDbCR);
        static                bool RequiredSystemTablesExist(ECDbCR);
        static  BentleyStatus ResolveECClassId(DbECClassEntry& key, ECClassId ecClassId, ECDbCR);
        static  BentleyStatus GetDerivedECClasses(ECClassIdListR classIds, ECClassId baseClassId, ECDbCR);
        static  BentleyStatus GetBaseECClasses(ECClassIdListR baseClassIds, ECClassId ecClassId, ECDbCR);
        static  BentleyStatus ResolveECSchemaId(DbECSchemaEntry& key, ECSchemaId ecSchemaId, ECDbCR);

        static  BentleyStatus GetECSchemaKeys(ECSchemaKeys&, ECDbCR);
        static  BentleyStatus GetECClassKeys(ECClassKeys&, ECSchemaId, ECDbCR);
        static                bool IsECSchemaMapped(bool* schemaNotFound, ECN::ECSchemaCR, ECDbCR);
        static  BentleyStatus GetClassesMappedToTable(std::vector<ECClassId>& classIds, ECDbSqlTable const& table, bool skipRelationships, ECDbCR);
        static                bool IsCustomAttributeDefined(ECDbCR, ECClassId caClassId, ECContainerId caSourceContainerId, ECContainerType caContainerType);
        static  ECDbPropertyPathId GetECPropertyPathId(ECPropertyId rootECPropertyId, Utf8CP accessString, ECDbCR);

        static BentleyStatus GetSchemaNamespacePrefixes(bvector<Utf8String>& prefixes, ECDbCR);
    };



END_BENTLEY_SQLITE_EC_NAMESPACE
