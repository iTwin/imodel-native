/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaReader.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ECDbClassDependencyAnalyzer.h"
USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbSchemaReader: public RefCountedBase, public virtual IECClassResolver, public virtual ECDbClassDependencyAnalyzer::ICallback
    {
typedef bmap<ECClassId, DbECClassEntryP>  DbECClassEntryMap;
typedef bmap<ECSchemaId, DbECSchemaEntryP> DbECSchemaMap;
private:
    ECSchemaCache              m_cache;
    Db&                        m_db;
    DbECClassEntryMap          m_ecClassKeyByECClassIdLookup;
    DbECSchemaMap              m_ecSchemaByECSchemaIdLookup;
    bool                       m_loadOnlyPrimaryCustomAttributes;
    mutable BeCriticalSection  m_criticalSection;

    BeSQLite::DbResult         LoadECSchemaClassesFromDb               (DbECSchemaEntryP ecSchemaKey, std::set<DbECSchemaEntryP>& fullyLoadedSchemas);
    BeSQLite::DbResult         LoadECSchemaFromDb                      (ECSchemaPtr& ecSchemaOut, ECSchemaId ecSchemaId);
    BeSQLite::DbResult         LoadECClassFromDb                       (ECClassP& ecClassOut, ECClassId ecClassId, ECSchemaR ecSchemaIn);
    BeSQLite::DbResult         LoadECPropertiesFromDb                  (ECClassP& ecClass, ECClassId ecClassId);
    BeSQLite::DbResult         LoadBaseClassesFromDb                   (ECClassP& ecClass, ECClassId ecClassId);
    BeSQLite::DbResult         LoadCAFromDb                            (ECN::IECCustomAttributeContainerR  caConstainer, ECContainerId containerId, ECContainerType containerType);
    BeSQLite::DbResult         LoadECRelationConstraintFromDb          (ECRelationshipClassP& ecRelationship, ECClassId ecClassId, ECRelationshipEnd relationshipEnd);
    BeSQLite::DbResult         LoadECRelationConstraintClassesFromDb   (ECRelationshipConstraintR ecRelationship, ECClassId ecClassId, ECRelationshipEnd relationshipEnd);
    //Interface api
    BeSQLite::DbResult         LoadECSchemaDefinition                  (DbECSchemaEntryP& outECSchemaKey, bvector<DbECSchemaEntryP>& newlyLoadedSchemas, ECSchemaId ctxECSchemaId);
    BeSQLite::DbResult         ReadECClass                             (ECClassP& ecClass, ECClassId ecClassId);
    BeSQLite::DbResult         ReadECSchema                            (DbECSchemaEntryP& outECSchemaKey, ECSchemaId ctxECSchemaId, bool ensureAllClassesExist);

    BeSQLite::DbResult         ReadECSchema                            (ECSchemaP& ecSchemaOut, Utf8CP schemaName, uint32_t versionMajor, uint32_t versinMinor, bool partial);
                               ECDbSchemaReader                        (Db& db);
    DbECSchemaEntryP           FindDbECSchemaEntry                     (ECSchemaId ecSchemaId);
    DbECClassEntryP            FindDbECClassEntry                      (ECClassId ecClassid);
    void                       AddECSchemaToCacheInternal              (ECSchemaCR schema);

public:
    BeSQLite::DbResult         FindECSchemaIdInDb                      (ECSchemaId& ecSchemaId, Utf8CP schemaName) const;
    BeSQLite::DbResult         GetECSchema                             (ECSchemaP& ecSchema, Utf8CP schemaName, bool loadClasses);
    BeSQLite::DbResult         GetECSchema                             (ECSchemaP& ecSchemaOut, ECSchemaId ecSchemaId, bool loadClasses);
    virtual   ECClassP         GetECClass                              (ECClassId ecClassId) override;
    virtual   ECClassP         GetECClass                              (Utf8StringCR schemaNameOrPrefix, Utf8String className) override;

    BeSQLite::DbResult         GetECClass                              (/*OUT*/ECClassP& ecClass, ECClassId ecClassId);
    BeSQLite::DbResult         GetECClassBySchemaName                  (/*OUT*/ ECClassP& ecClass, Utf8CP schemaName, Utf8CP className);
    BeSQLite::DbResult         GetECClassBySchemaNameSpacePrefix       (/*OUT*/ ECClassP& ecClass, Utf8CP schemaName, Utf8CP className);
    BeSQLite::DbResult         GetECClass                              (/*OUT*/ ECClassP& ecClass, Utf8CP qualifiedName); //schema:classname
    // Get the names of the class for given schema from database
    static ECDbSchemaReaderPtr Create                                  (Db& db);
    // add a existing in memory schema into cache and build key maps for schema and classes.
    void                       AddECSchemaToCache                      (ECSchemaCR schema);
    virtual bool               CanAnalyze                              (ECClassId classId)const  override;

#if 0 //RemoveClass is not supported in EC
    BeSQLite::DbResult         DeleteECClass                           (ECClassId ecClassId, bool deleteDerivedClasses /*only delete derive if its only inherited from specified class*/);
#endif
    void                       ClearCache                              ();
    BeSQLite::DbResult         TransformAllCABlobsToECInstances        ();
                               ~ECDbSchemaReader                       ();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
