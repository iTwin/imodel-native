/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaImportContext.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/SchemaManager.h>
#include "ClassMap.h"
#include "DbSchemaPersistenceManager.h"
#include "SchemaComparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct SchemaCompareContext final
    {
private:
    bvector<ECN::ECSchemaCP> m_existingSchemaList;
    bvector<ECN::ECSchemaCP> m_importedSchemaList;
    SchemaChanges m_changes;
    bool m_prepared;

    bool AssertIfNotPrepared() const;

public:
    SchemaCompareContext() : m_prepared(false) {}
    ~SchemaCompareContext() {}

    BentleyStatus Prepare(SchemaManager const& schemaManager, bvector<ECN::ECSchemaCP> const& dependencyOrderedPrimarySchemas);
    bvector<ECN::ECSchemaCP>  const& GetImportingSchemas() const { return m_importedSchemaList; }
    ECN::ECSchemaCP FindExistingSchema(Utf8CP schemaName) const;
    bool IsPrepared() const { return m_prepared; }
    SchemaChanges& GetChanges() { return m_changes; }
    bool HasNoSchemasToImport() const { return m_importedSchemaList.empty(); }
    bool RequiresUpdate() const;

    BentleyStatus ReloadContextECSchemas(SchemaManager const&);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2014
//+===============+===============+===============+===============+===============+======
struct SchemaImportContext final
    {
public:
    struct ClassMapSaveContext final: NonCopyableClass
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
    SchemaCompareContext m_compareContext;
    bset<ECN::ECClassId> m_classMapsToSave;

    SchemaManager::SchemaImportOptions m_options;

public:
    explicit SchemaImportContext(SchemaManager::SchemaImportOptions options) : m_options(options) {}

    ClassMappingCACache const* GetClassMappingCACache(ECN::ECClassCR) const;
    ClassMappingCACache* GetClassMappingCACacheP(ECN::ECClassCR) const;

    void CacheClassMapInfo(ClassMap const&, std::unique_ptr<ClassMappingInfo>&);
    std::map<ClassMap const*, std::unique_ptr<ClassMappingInfo>> const& GetClassMappingInfoCache() const { return m_classMappingInfoCache; }

    void AddNRelationshipRelationshipClassWithSingleNavigationProperty(ECN::ECRelationshipClassCR relClass) { m_relationshipClassesWithSingleNavigationProperty.insert(&relClass); }
    bool IsRelationshipClassWithSingleNavigationProperty(ECN::ECRelationshipClassCR relClass) const { return m_relationshipClassesWithSingleNavigationProperty.find(&relClass) != m_relationshipClassesWithSingleNavigationProperty.end(); }
    ClassMapLoadContext& GetClassMapLoadContext() { return m_loadContext; }
    ClassMapSaveContext& GetClassMapSaveContext() { return m_saveContext; }
    SchemaCompareContext& GetECSchemaCompareContext() { return m_compareContext; }

    void AddClassMapForSaving(ECN::ECClassId classId) { m_classMapsToSave.insert(classId); }
    bool ClassMapNeedsSaving(ECN::ECClassId classId) const { return m_classMapsToSave.find(classId) != m_classMapsToSave.end(); }
    SchemaManager::SchemaImportOptions GetOptions() const { return m_options; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE