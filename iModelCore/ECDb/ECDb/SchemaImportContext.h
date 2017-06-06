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
    bvector<ECN::ECSchemaCP> m_existingSchemas;
    bvector<ECN::ECSchemaCP> m_schemasToImport;
    SchemaChanges m_changes;

public:
    SchemaCompareContext() {}
    ~SchemaCompareContext() {}

    BentleyStatus Prepare(SchemaManager const&, bvector<ECN::ECSchemaCP> const& dependencyOrderedPrimarySchemas);
    bvector<ECN::ECSchemaCP>  const& GetSchemasToImport() const { return m_schemasToImport; }
    SchemaChanges& GetChanges() { return m_changes; }

    ECN::ECSchemaCP FindExistingSchema(Utf8CP schemaName) const;

    BentleyStatus ReloadContextECSchemas(SchemaManager const&);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2014
//+===============+===============+===============+===============+===============+======
struct SchemaImportContext final
    {
    enum class Phase
        {
        ImportingSchemas,
        MappingSchemas,
        MappingMixins,
        MappingEntities,
        MappingRelationships,
        CreatingUserDefinedIndexes
        };
private:
    SchemaManager::SchemaImportOptions m_options;
    Phase m_phase = Phase::ImportingSchemas;

    ClassMapLoadContext m_loadContext;
    bset<ECN::ECClassId> m_classMapsToSave;
    mutable std::map<ECN::ECClassCP, std::unique_ptr<ClassMappingCACache>> m_classMappingCACache;
    std::map<ClassMap const*, std::unique_ptr<ClassMappingInfo>> m_classMappingInfoCache;
    bmap<ECN::ECClassId, ForeignKeyConstraintCustomAttribute> m_fkConstraintCACache;

public:
    explicit SchemaImportContext(SchemaManager::SchemaImportOptions options) : m_options(options) {}

    void SetPhase(Phase phase) { BeAssert(Enum::ToInt(m_phase) < Enum::ToInt(phase)); m_phase = phase; }
    Phase GetPhase() const { return m_phase; }

    SchemaManager::SchemaImportOptions GetOptions() const { return m_options; }

    ClassMappingCACache const* GetClassMappingCACache(ECN::ECClassCR) const;
    ClassMappingCACache* GetClassMappingCACacheP(ECN::ECClassCR) const;

    void CacheClassMapInfo(ClassMap const&, std::unique_ptr<ClassMappingInfo>&);
    std::map<ClassMap const*, std::unique_ptr<ClassMappingInfo>> const& GetClassMappingInfoCache() const { return m_classMappingInfoCache; }
    void CacheFkConstraintCA(ECN::NavigationECPropertyCR navProp);
    bmap<ECN::ECClassId, ForeignKeyConstraintCustomAttribute> const& GetFkConstraintCACache() const { return m_fkConstraintCACache; }
    ClassMapLoadContext& GetClassMapLoadContext() { return m_loadContext; }
    void AddClassMapForSaving(ECN::ECClassId classId) { m_classMapsToSave.insert(classId); }
    bool ClassMapNeedsSaving(ECN::ECClassId classId) const { return m_classMapsToSave.find(classId) != m_classMapsToSave.end(); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE