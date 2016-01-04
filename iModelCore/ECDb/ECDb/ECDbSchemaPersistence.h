/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaPersistence.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
struct DbECRelationshipConstraintInfo : DbInfoBase
    {
    public:
        enum Columns
            {
            COL_RelationshipClassId = 0x01,
            COL_RelationshipEnd = 0x02,
            COL_MultiplicityLowerLimit = 0x04,
            COL_MultiplicityUpperLimit = 0x08,
            COL_RoleLabel = 0x10,
            COL_IsPolymorphic = 0x20,
            COL_ALL = 0xffffffff
            };

        ECClassId         m_relationshipClassId;
        ECRelationshipEnd m_ecRelationshipEnd;
        uint32_t          m_multiplicityLowerLimit;
        uint32_t          m_multiplicityUpperLimit;
        Utf8String        m_roleLabel;
        bool              m_isPolymorphic;
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
    ECSchemaId m_ecSchemaId;
    ECClassId m_ecClassId;
    ECN::ECClassP m_cachedECClass;

    DbECClassEntry(ECN::ECSchemaId schemaId, ECN::ECClassCR ecClass) : m_ecSchemaId(schemaId), m_ecClassId(ecClass.GetId())
        {
        m_cachedECClass = const_cast<ECN::ECClassP> (&ecClass);
        }
    };

typedef int64_t ECEnumerationId;

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      12/2015
//+===============+===============+===============+===============+===============+======
struct DbECEnumEntry
    {
public:
    ECSchemaId m_ecSchemaId;
    Utf8CP m_enumName;
    ECN::ECEnumerationP m_cachedECEnum;

    DbECEnumEntry(ECN::ECSchemaId schemaId, ECN::ECEnumerationCR ecEnum) : m_ecSchemaId(schemaId), m_enumName(ecEnum.GetName().c_str()) 
        {
        m_cachedECEnum = const_cast<ECN::ECEnumerationP> (&ecEnum);
        }
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
    uint32_t         m_nTypesInSchema; //This is read from Db
    uint32_t         m_nTypesLoaded;   //Every time a class or enum is loaded from db for this schema it is incremented
    ECN::ECSchemaPtr m_cachedECSchema;    //Contain ECSchema which might be not complete
    bool             IsFullyLoaded() {return m_nTypesInSchema == m_nTypesLoaded;} 

    DbECSchemaEntry() : m_cachedECSchema(nullptr), m_nTypesInSchema(0), m_nTypesLoaded(0) {}

    explicit DbECSchemaEntry(ECN::ECSchemaCR schema) : m_ecSchemaId(schema.GetId()), m_schemaName(schema.GetName()), m_versionMajor(schema.GetVersionMajor()), m_versionMinor(schema.GetVersionMinor()),
        m_nTypesInSchema(0), m_nTypesLoaded(0)
        {
        m_cachedECSchema = const_cast<ECSchemaP>(&schema);
        }
        
    };


//=======================================================================================
// @bsienum                                                Krischan.Eberle      12/2015
//+===============+===============+===============+===============+===============+======
enum class ECPropertyKind
    {
    Primitive = 0,
    Struct = 1,
    PrimitiveArray = 2,
    StructArray = 3,
    Navigation = 4
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbSchemaPersistence
    {
    public:
        typedef bvector<ECN::ECClassId>   ECClassIdList;
        typedef bvector<DbECSchemaEntry>  ECSchemaKeyList;

        //Insert new item
        static  BentleyStatus InsertBaseClass(ECDbCR, DbBaseClassInfo const&);
        static  BentleyStatus InsertECRelationshipConstraint(ECDbCR, DbECRelationshipConstraintInfo const&);
        static  BentleyStatus InsertECRelationshipConstraintClass(ECDbCR, ECN::ECClassId relClassId, ECN::ECRelationshipConstraintClassCR, ECN::ECRelationshipEnd);
        static  BentleyStatus InsertCustomAttribute(ECDbCR, DbCustomAttributeInfo const&);
        static  BentleyStatus InsertECSchemaReference(ECDbCR, DbECSchemaReferenceInfo const&);

        //Find BaseClass
        static  BentleyStatus FindBaseClass(BeSQLite::CachedStatementPtr&, ECDbCR, DbBaseClassInfo const&);
        static  BeSQLite::DbResult Step(DbBaseClassInfo&, BeSQLite::Statement&);
        //Find ECRelationConstraintInfo 
        static  BentleyStatus FindECRelationshipConstraint(BeSQLite::CachedStatementPtr&, ECDbCR, DbECRelationshipConstraintInfo const&);
        static  BeSQLite::DbResult Step(DbECRelationshipConstraintInfo&, BeSQLite::Statement&);
        //Find CustomAttributeInfo
        static  BentleyStatus FindCustomAttribute(BeSQLite::CachedStatementPtr&, ECDbCR, DbCustomAttributeInfo const&);
        static  BeSQLite::DbResult Step(DbCustomAttributeInfo&, BeSQLite::Statement&);

        //Helper
        static BentleyStatus GetReferencedSchemas(bvector<ECSchemaId>&, ECDbCR, ECSchemaId);
        static bool ContainsECSchemaReference(ECDbCR, ECSchemaId ecPrimarySchemaId, ECSchemaId ecReferencedSchemaId);
        static bool ContainsECClass(ECDbCR, ECClassCR);
        static bool ContainsECSchemaWithId(ECDbCR, ECSchemaId);
        static ECSchemaId GetECSchemaId(ECDbCR, Utf8CP schemaName);
        static ECSchemaId GetECSchemaId(ECDbCR, ECSchemaCR);
        static ECClassId GetECClassId(ECDbCR, Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema);
        static ECPropertyId GetECPropertyId(ECDbCR, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName);

        static BentleyStatus InitializeSystemTables(ECDbCR);
        static bool RequiredSystemTablesExist(ECDbCR);
        static BentleyStatus GetDerivedECClasses(ECClassIdList& classIds, ECClassId baseClassId, ECDbCR);
        static BentleyStatus GetBaseECClasses(ECClassIdList& baseClassIds, ECClassId ecClassId, ECDbCR);
        static BentleyStatus ResolveECSchemaId(DbECSchemaEntry& key, ECSchemaId ecSchemaId, ECDbCR);

        static BentleyStatus GetECSchemaKeys(ECSchemaKeys&, ECDbCR);
        static BentleyStatus GetECClassKeys(ECClassKeys&, ECSchemaId, ECDbCR);
        static bool IsECSchemaMapped(bool* schemaNotFound, ECN::ECSchemaCR, ECDbCR);
        static bool IsCustomAttributeDefined(ECDbCR, ECClassId caClassId, ECContainerId caSourceContainerId, ECContainerType caContainerType);
        static ECDbPropertyPathId GetECPropertyPathId(ECPropertyId rootECPropertyId, Utf8CP accessString, ECDbCR);

        static BentleyStatus GetSchemaNamespacePrefixes(bvector<Utf8String>& prefixes, ECDbCR);
    };



END_BENTLEY_SQLITE_EC_NAMESPACE
