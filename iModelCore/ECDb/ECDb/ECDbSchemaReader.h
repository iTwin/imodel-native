/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaReader.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECSchemaEntry
    {
public:
    ECN::ECSchemaPtr m_cachedECSchema;    //Contain ECSchema which might be not complete
    int m_typeCountInSchema; //This is read from Db
    uint32_t m_loadedTypeCount; //Every time a class or enum is loaded from db for this schema it is incremented

    explicit DbECSchemaEntry(ECN::ECSchemaR schema) : m_cachedECSchema(&schema), m_typeCountInSchema(-1), m_loadedTypeCount(0)
        {}

    DbECSchemaEntry(ECN::ECSchemaPtr& schema, int typeCountInSchema) : m_cachedECSchema(schema), m_typeCountInSchema(typeCountInSchema), m_loadedTypeCount(0)
        {}

    ECN::ECSchemaId GetId() const { return m_cachedECSchema->GetId(); }
    bool IsFullyLoaded() const { return m_typeCountInSchema == m_loadedTypeCount; }
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbSchemaReader: public RefCountedBase
    {
    typedef std::map<ECSchemaId, std::unique_ptr<DbECSchemaEntry>> DbECSchemaMap;
    typedef std::map<ECClassId, std::unique_ptr<DbECClassEntry>> DbECClassEntryMap;
    typedef std::map<Utf8CP, std::unique_ptr<DbECEnumEntry>, CompareUtf8> DbECEnumEntryMap;

private:
    ECDbCR m_db;
    mutable ECSchemaCache m_cache;
    mutable DbECSchemaMap m_ecSchemaCache;
    mutable DbECClassEntryMap m_ecClassCache;
    mutable DbECEnumEntryMap m_ecEnumCache;
    mutable BeMutex m_criticalSection;

    explicit ECDbSchemaReader(ECDbCR db) :m_db(db) {}

    BentleyStatus         LoadClassesAndEnumsFromDb(DbECSchemaEntry* ecSchemaKey, std::set<DbECSchemaEntry*>& fullyLoadedSchemas) const;
    BentleyStatus         LoadECSchemaFromDb(DbECSchemaEntry*&, ECSchemaId) const;
    BentleyStatus         LoadECPropertiesFromDb(ECClassP& ecClass, ECClassId ecClassId) const;
    BentleyStatus         LoadBaseClassesFromDb(ECClassP& ecClass, ECClassId ecClassId) const;
    BentleyStatus         LoadCAFromDb(ECN::IECCustomAttributeContainerR  caConstainer, ECContainerId containerId, ECContainerType containerType) const;
    BentleyStatus         LoadECRelationshipConstraintFromDb(ECRelationshipClassP&, ECClassId constraintClassId, ECRelationshipEnd) const;
    BentleyStatus         LoadECRelationshipConstraintClassesFromDb(ECRelationshipConstraintR, ECClassId relationshipClassId, ECRelationshipEnd) const;
    BentleyStatus         LoadECSchemaDefinition(DbECSchemaEntry*&, bvector<DbECSchemaEntry*>& newlyLoadedSchemas, ECSchemaId ctxECSchemaId) const;

    BentleyStatus         ReadECSchema(DbECSchemaEntry*&, ECSchemaId ctxECSchemaId, bool ensureAllClassesExist) const;
    BentleyStatus         ReadECSchema(ECSchemaP& ecSchemaOut, Utf8CP schemaName, uint32_t versionMajor, uint32_t versinMinor, bool partial) const;
    BentleyStatus         ReadECClass(ECClassP&, ECClassId) const;
    BentleyStatus         ReadECEnumeration(ECEnumerationP&, ECN::ECSchemaId, Utf8CP enumName) const;
                               
public:
    ~ECDbSchemaReader() {}

    BentleyStatus         GetECSchema(ECSchemaP& ecSchemaOut, ECSchemaId ecSchemaId, bool loadClasses) const;
    ECClassP              GetECClass (ECClassId ecClassId) const;
    BentleyStatus         GetECClass(/*OUT*/ ECClassP& ecClass, Utf8CP qualifiedName) const; //schema:classname
    bool                  TryGetECClassId(ECN::ECClassId& id, Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema) const;

    ECEnumerationCP       GetECEnumeration(Utf8CP schemaName, Utf8CP enumName) const;

    BentleyStatus         EnsureDerivedClassesExist(ECClassId baseClassId) const;
    void                  AddECSchemaToCache (ECSchemaCR schema);
    void                  ClearCache ();

    static ECDbSchemaReaderPtr Create(ECDbCR);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
