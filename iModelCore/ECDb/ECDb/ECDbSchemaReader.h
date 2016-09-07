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
    typedef std::map<ECN::ECSchemaId, std::unique_ptr<DbECSchemaEntry>> DbECSchemaMap;
    typedef std::map<ECN::ECClassId, std::unique_ptr<DbECClassEntry>> DbECClassEntryMap;
    typedef std::map<ECN::ECEnumerationId, std::unique_ptr<DbECEnumEntry>> DbECEnumEntryMap;
    typedef std::map<ECN::KindOfQuantityId, std::unique_ptr<DbKindOfQuantityEntry>> DbKindOfQuantityEntryMap;

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
        mutable DbECSchemaMap m_ecSchemaCache;
        mutable DbECClassEntryMap m_ecClassCache;
        mutable DbECEnumEntryMap m_ecEnumCache;
        mutable DbKindOfQuantityEntryMap m_koqCache;
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
        ECN::ECClassCP GetECClass(ECN::ECClassId) const;
        ECN::ECEnumerationCP GetECEnumeration(Utf8CP schemaName, Utf8CP enumName) const;
        ECN::KindOfQuantityCP GetKindOfQuantity(Utf8CP schemaName, Utf8CP koqName) const;

        BentleyStatus EnsureDerivedClassesExist(ECN::ECClassId) const;
        bool TryGetECClassId(ECN::ECClassId&, Utf8CP schemaNameOrAlias, Utf8CP className, ResolveSchema) const;
        void ClearCache();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
