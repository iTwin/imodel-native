/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaImportContext.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDbSchemaManager.h>
#include "ClassMap.h"
#include "DbSchemaPersistenceManager.h"
#include "ECSchemaComparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECSchemaCompareContext
    {
private:
    bvector<ECN::ECSchemaCP> m_existingSchemaList;
    bvector<ECN::ECSchemaCP> m_importedSchemaList;
    ECSchemaChanges m_changes;
    bool m_prepared;

    bool AssertIfNotPrepared() const;

public:
    ECSchemaCompareContext() : m_prepared(false) {}
    ~ECSchemaCompareContext() {}

    BentleyStatus Prepare(ECDbSchemaManager const& schemaManager, bvector<ECN::ECSchemaP> const& dependencyOrderedPrimarySchemas);
    bvector<ECN::ECSchemaCP>  const& GetImportingSchemas() const { return m_importedSchemaList; }
    ECN::ECSchemaCP FindExistingSchema(Utf8CP schemaName) const;
    bool IsPrepared() const { return m_prepared; }
    ECSchemaChanges& GetChanges() { return m_changes; }
    bool HasNoSchemasToImport() const { return m_importedSchemaList.empty(); }
    bool RequiresUpdate() const;

    BentleyStatus ReloadECSchemaIfRequired(ECDbSchemaManager const&);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2014
//+===============+===============+===============+===============+===============+======
struct SchemaImportContext
    {
public:
    struct ClassMapSaveContext : NonCopyableClass
        {
        private:
            std::set<ClassMap const*> m_alreadySavedClassMaps;

        public:
            ClassMapSaveContext() {}

            //!Checks whether the specified class map was already saved, i.e. whether
            //!it is held by the context.
            //!If not, true is returned and the class map is added to the context
            bool NeedsSaving(ClassMap const&);
        };

private:
    mutable std::map<ECN::ECClassCP, std::unique_ptr<ClassMappingCACache>> m_classMappingCACache;
    std::map<ClassMap const*, std::unique_ptr<ClassMappingInfo>> m_classMappingInfoCache;
    bset<ECN::ECRelationshipClassCP> m_relationshipClassesWithSingleNavigationProperty;
    ClassMapLoadContext m_loadContext;
    ClassMapSaveContext m_saveContext;
    ECSchemaCompareContext m_compareContext;

public:
    SchemaImportContext() {}
    BentleyStatus Initialize(DbSchema& dbSchema, ECDbCR ecdb);

    ClassMappingCACache const* GetClassMappingCACache(ECN::ECClassCR) const;
    ClassMappingCACache* GetClassMappingCACacheP(ECN::ECClassCR) const;

    void CacheClassMapInfo(ClassMap const&, std::unique_ptr<ClassMappingInfo>&);
    std::map<ClassMap const*, std::unique_ptr<ClassMappingInfo>> const& GetClassMappingInfoCache() const { return m_classMappingInfoCache; }

    void AddNRelationshipRelationshipClassWithSingleNavigationProperty(ECN::ECRelationshipClassCR relClass) { m_relationshipClassesWithSingleNavigationProperty.insert(&relClass); }
    bool IsRelationshipClassWithSingleNavigationProperty(ECN::ECRelationshipClassCR relClass) const { return m_relationshipClassesWithSingleNavigationProperty.find(&relClass) != m_relationshipClassesWithSingleNavigationProperty.end(); }
    ClassMapLoadContext& GetClassMapLoadContext() { return m_loadContext; }
    ClassMapSaveContext& GetClassMapSaveContext() { return m_saveContext; }
    ECSchemaCompareContext& GetECSchemaCompareContext() { return m_compareContext; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE