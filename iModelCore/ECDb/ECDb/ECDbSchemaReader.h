/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaReader.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECSchemaEntry
    {
    public:
        ECN::ECSchemaPtr m_cachedECSchema;    //Contain ECSchema which might be not complete
        int m_typeCountInSchema; //This is read from Db
        uint32_t m_loadedTypeCount; //Every time a class or enum or KOQ is loaded from db for this schema it is incremented

        explicit DbECSchemaEntry(ECN::ECSchemaR schema) : m_cachedECSchema(&schema), m_typeCountInSchema(-1), m_loadedTypeCount(0)
            {}

        DbECSchemaEntry(ECN::ECSchemaPtr& schema, int typeCountInSchema) : m_cachedECSchema(schema), m_typeCountInSchema(typeCountInSchema), m_loadedTypeCount(0)
            {}

        ECN::ECSchemaId GetId() const { return m_cachedECSchema->GetId(); }
        bool IsFullyLoaded() const { return m_typeCountInSchema == m_loadedTypeCount; }
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECClassEntry
    {
    public:
        ECN::ECClassId m_ecClassId;
        ECN::ECClassP m_cachedECClass;

        explicit DbECClassEntry(ECN::ECClassCR ecClass) : m_ecClassId(ecClass.GetId())
            {
            m_cachedECClass = const_cast<ECN::ECClassP> (&ecClass);
            }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      12/2015
//+===============+===============+===============+===============+===============+======
struct DbECEnumEntry
    {
public:
    ECN::ECEnumerationId m_enumId;
    ECN::ECEnumerationP m_cachedECEnum;

    DbECEnumEntry(ECN::ECEnumerationId enumId, ECN::ECEnumerationCR ecEnum) : m_enumId(enumId)
        {
        m_cachedECEnum = const_cast<ECN::ECEnumerationP> (&ecEnum);
        }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2016
//+===============+===============+===============+===============+===============+======
struct DbKindOfQuantityEntry
    {
public:
    ECN::KindOfQuantityId m_koqId;
    ECN::KindOfQuantityP m_cachedKoq;

    DbKindOfQuantityEntry(ECN::KindOfQuantityId koqId, ECN::KindOfQuantityCR koq) : m_koqId(koqId)
        {
        m_cachedKoq = const_cast<ECN::KindOfQuantityP> (&koq);
        }
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbSchemaReader
    {
    private:
        struct Context : NonCopyableClass
            {
            private:
                std::vector<ECN::NavigationECProperty*> m_navProps;
                std::vector<ECN::ECSchema*> m_schemasToLoadCAInstancesFor;

            public:
                Context() {}
                void AddNavigationProperty(ECN::NavigationECProperty& navProp) { m_navProps.push_back(&navProp); }
                void AddSchemaToLoadCAInstanceFor(ECN::ECSchemaR schema) { m_schemasToLoadCAInstancesFor.push_back(&schema); }
                BentleyStatus Postprocess(ECDbSchemaReader const&) const;
            };

        ECDbCR m_db;
        mutable ECN::ECSchemaCache m_cache;
        mutable std::map<ECN::ECSchemaId, std::unique_ptr<DbECSchemaEntry>> m_ecSchemaCache;
        mutable std::map<ECN::ECClassId, std::unique_ptr<DbECClassEntry>> m_ecClassCache;
        mutable std::map<ECN::ECEnumerationId, std::unique_ptr<DbECEnumEntry>> m_ecEnumCache;
        mutable std::map<ECN::KindOfQuantityId, std::unique_ptr<DbKindOfQuantityEntry>> m_koqCache;
        mutable bmap<Utf8String, bmap<Utf8String, ECN::ECClassId, CompareIUtf8Ascii>, CompareIUtf8Ascii> m_classIdCache;
        mutable BeMutex m_criticalSection;

        ECN::ECSchemaCP GetECSchema(Context&, ECN::ECSchemaId, bool loadSchemaEntities) const;
        ECN::ECClassP GetECClass(Context&, ECN::ECClassId) const;
        bool TryGetECClassFromCache(ECN::ECClassP&, ECN::ECClassId) const;
        ECN::ECEnumerationCP GetECEnumeration(Context&, Utf8CP schemaName, Utf8CP enumName) const;
        ECN::KindOfQuantityCP GetKindOfQuantity(Context&, Utf8CP schemaName, Utf8CP koqName) const;

        BentleyStatus LoadSchemaEntitiesFromDb(DbECSchemaEntry*, Context&, std::set<DbECSchemaEntry*>& fullyLoadedSchemas) const;
        BentleyStatus LoadECSchemaFromDb(DbECSchemaEntry*&, ECN::ECSchemaId) const;
        BentleyStatus LoadECPropertiesFromDb(ECN::ECClassP&, Context&, ECN::ECClassId) const;
        BentleyStatus LoadBaseClassesFromDb(ECN::ECClassP&, Context&, ECN::ECClassId) const;
        BentleyStatus LoadCAFromDb(ECN::IECCustomAttributeContainerR, Context&, ECContainerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType) const;
        BentleyStatus LoadECRelationshipConstraintFromDb(ECN::ECRelationshipClassP&, Context&, ECN::ECClassId constraintClassId, ECN::ECRelationshipEnd) const;
        BentleyStatus LoadECRelationshipConstraintClassesFromDb(ECN::ECRelationshipConstraintR, Context&, ECRelationshipConstraintId constraintId) const;
        BentleyStatus LoadECSchemaDefinition(DbECSchemaEntry*&, bvector<DbECSchemaEntry*>& newlyLoadedSchemas, ECN::ECSchemaId) const;

        BentleyStatus ReadECSchema(DbECSchemaEntry*&, Context&, ECN::ECSchemaId, bool loadSchemaEntities) const;
        BentleyStatus ReadECEnumeration(ECN::ECEnumerationP&, Context&, ECN::ECEnumerationId) const;
        BentleyStatus ReadKindOfQuantity(ECN::KindOfQuantityP&, Context&, ECN::KindOfQuantityId) const;

        BentleyStatus EnsureDerivedClassesExist(Context&, ECN::ECClassId) const;

    public:
        explicit ECDbSchemaReader(ECDbCR db) :m_db(db) {}
        ~ECDbSchemaReader() {}

        ECN::ECSchemaCP GetECSchema(ECN::ECSchemaId, bool loadSchemaEntities) const;
        ECN::ECSchemaId GetECSchemaId(ECN::ECSchemaCR) const;

        ECN::ECClassCP GetECClass(ECN::ECClassId) const;
        ECN::ECClassId GetECClassId(ECN::ECClassCR) const;
        ECN::ECClassId GetECClassId(Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema) const;

        ECN::ECEnumerationCP GetECEnumeration(Utf8CP schemaName, Utf8CP enumName) const;
        ECN::ECEnumerationId GetECEnumerationId(ECN::ECEnumerationCR) const;

        ECN::KindOfQuantityCP GetKindOfQuantity(Utf8CP schemaName, Utf8CP koqName) const;
        ECN::KindOfQuantityId GetKindOfQuantityId(ECN::KindOfQuantityCR) const;

        ECN::ECPropertyId GetECPropertyId(ECN::ECPropertyCR) const;

        BentleyStatus EnsureDerivedClassesExist(ECN::ECClassId) const;

        void ClearCache() const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
