/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaReader.h $
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
struct ECDbSchemaReader: public RefCountedBase
    {
typedef bmap<ECClassId, DbECClassEntry*>  DbECClassEntryMap;
typedef bmap<ECSchemaId, DbECSchemaEntry*> DbECSchemaMap;
private:
    ECSchemaCache              m_cache;
    ECDbCR                     m_db;
    DbECClassEntryMap          m_ecClassKeyByECClassIdLookup;
    DbECSchemaMap              m_ecSchemaByECSchemaIdLookup;
    bool                       m_loadOnlyPrimaryCustomAttributes;
    mutable BeMutex m_criticalSection;

    explicit ECDbSchemaReader(ECDbCR db) :m_db(db), m_loadOnlyPrimaryCustomAttributes(false) {}

    BentleyStatus         LoadECSchemaClassesFromDb(DbECSchemaEntry* ecSchemaKey, std::set<DbECSchemaEntry*>& fullyLoadedSchemas);
    BentleyStatus         LoadECSchemaFromDb(ECSchemaPtr& ecSchemaOut, ECSchemaId ecSchemaId);
    BentleyStatus         LoadECClassFromDb(ECClassP& ecClassOut, ECClassId ecClassId, ECSchemaR ecSchemaIn);
    BentleyStatus         LoadECPropertiesFromDb(ECClassP& ecClass, ECClassId ecClassId);
    BentleyStatus         LoadBaseClassesFromDb(ECClassP& ecClass, ECClassId ecClassId);
    BentleyStatus         LoadCAFromDb(ECN::IECCustomAttributeContainerR  caConstainer, ECContainerId containerId, ECContainerType containerType);
    BentleyStatus         LoadECRelationshipConstraintFromDb(ECRelationshipClassP&, ECClassId constraintClassId, ECRelationshipEnd);
    BentleyStatus         LoadECRelationshipConstraintClassesFromDb(ECRelationshipConstraintR, ECClassId relationshipClassId, ECRelationshipEnd);
    //Interface api
    BentleyStatus         LoadECSchemaDefinition(DbECSchemaEntry*& outECSchemaKey, bvector<DbECSchemaEntry*>& newlyLoadedSchemas, ECSchemaId ctxECSchemaId);
    BentleyStatus         ReadECClass(ECClassP& ecClass, ECClassId ecClassId);
    BentleyStatus         ReadECSchema(DbECSchemaEntry*& outECSchemaKey, ECSchemaId ctxECSchemaId, bool ensureAllClassesExist);

    BentleyStatus         ReadECSchema(ECSchemaP& ecSchemaOut, Utf8CP schemaName, uint32_t versionMajor, uint32_t versinMinor, bool partial);
                               
    DbECSchemaEntry*           FindDbECSchemaEntry                     (ECSchemaId ecSchemaId);
    DbECClassEntry*            FindDbECClassEntry                      (ECClassId ecClassid);
    void                       AddECSchemaToCacheInternal              (ECSchemaCR schema);

public:
    ~ECDbSchemaReader();

    BentleyStatus         FindECSchemaIdInDb(ECSchemaId& ecSchemaId, Utf8CP schemaName) const;
    BentleyStatus         GetECSchema(ECSchemaP& ecSchemaOut, ECSchemaId ecSchemaId, bool loadClasses);
    ECClassP              GetECClass (ECClassId ecClassId);
    ECClassP              GetECClass (Utf8CP schemaNameOrPrefix, Utf8CP className);

    BentleyStatus         GetECClass(/*OUT*/ECClassP& ecClass, ECClassId ecClassId);
    BentleyStatus         GetECClassBySchemaName(/*OUT*/ ECClassP& ecClass, Utf8CP schemaName, Utf8CP className);
    BentleyStatus         GetECClassBySchemaNameSpacePrefix(/*OUT*/ ECClassP& ecClass, Utf8CP schemaName, Utf8CP className);
    BentleyStatus         GetECClass(/*OUT*/ ECClassP& ecClass, Utf8CP qualifiedName); //schema:classname
    // Get the names of the class for given schema from database
    // add a existing in memory schema into cache and build key maps for schema and classes.
    void                       AddECSchemaToCache                      (ECSchemaCR schema);

    void                       ClearCache                              ();
    BentleyStatus              TransformAllCABlobsToECInstances        ();

    static ECDbSchemaReaderPtr Create(ECDbCR);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
