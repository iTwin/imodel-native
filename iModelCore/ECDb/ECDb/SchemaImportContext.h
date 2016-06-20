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
private:
    mutable std::map<ECN::ECClassCP, std::unique_ptr<UserECDbMapStrategy>> m_userStrategyCache;
    std::map<ClassMap const*, std::unique_ptr<ClassMappingInfo>> m_classMappingInfoCache;
    bset<ECN::ECRelationshipClassCP> m_relationshipClassesWithSingleNavigationProperty;
    ClassMapLoadContext m_loadContext;
    ECSchemaCompareContext m_compareContext;

    UserECDbMapStrategy* GetUserStrategyP(ECN::ECClassCR, ECN::ECDbClassMap const*) const;

public:
    SchemaImportContext() {}
    BentleyStatus Initialize(DbSchema& dbSchema, ECDbCR ecdb);

    //! Gets the user map strategy for the specified ECClass.
    //! @return User map strategy. If the class doesn't have one a default strategy is returned. Only in 
    //! case of error, nullptr is returned
    UserECDbMapStrategy const* GetUserStrategy(ECN::ECClassCR, ECN::ECDbClassMap const* = nullptr) const;
    UserECDbMapStrategy* GetUserStrategyP(ECN::ECClassCR) const;

    void CacheClassMapInfo(ClassMap const&, std::unique_ptr<ClassMappingInfo>&);
    std::map<ClassMap const*, std::unique_ptr<ClassMappingInfo>> const& GetClassMappingInfoCache() const { return m_classMappingInfoCache; }

    void AddNRelationshipRelationshipClassWithSingleNavigationProperty(ECN::ECRelationshipClassCR relClass) { m_relationshipClassesWithSingleNavigationProperty.insert(&relClass); }
    bool IsRelationshipClassWithSingleNavigationProperty(ECN::ECRelationshipClassCR relClass) const { return m_relationshipClassesWithSingleNavigationProperty.find(&relClass) != m_relationshipClassesWithSingleNavigationProperty.end(); }
    ClassMapLoadContext& GetClassMapLoadContext() { return m_loadContext; }
    ECSchemaCompareContext& GetECSchemaCompareContext() { return m_compareContext; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE