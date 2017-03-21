/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbMap.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ClassMap.h"
#include "SchemaImportContext.h"
#include "DbSchema.h"
#include "IssueReporter.h"
#include "LightweightCache.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
/*=================================================================================**//**
* @bsiclass                                                     Casey.Mullen      11/2011
+===============+===============+===============+===============+===============+======*/
struct ECDbMap final : NonCopyableClass
    {
    public:
        typedef bmap<DbTable*, bset<ClassMap*>> ClassMapsByTable;

    private:
        ECDbCR m_ecdb;
        DbSchema m_dbSchema;
        mutable bmap<ECN::ECClassId, ClassMapPtr> m_classMapDictionary;
        mutable LightweightCache m_lightweightCache;
        mutable SchemaImportContext* m_schemaImportContext;

        ClassMapPtr DoGetClassMap(ECN::ECClassCR) const;
        BentleyStatus TryLoadClassMap(ClassMapPtr&, ClassMapLoadContext& ctx, ECN::ECClassCR) const;
        BentleyStatus DoMapSchemas() const;
        ClassMappingStatus MapClass(ECN::ECClassCR) const;
        BentleyStatus SaveDbSchema() const;
        BentleyStatus CreateOrUpdateRequiredTables() const;
        BentleyStatus CreateOrUpdateIndexesInDb() const;
        BentleyStatus PurgeOrphanTables() const;
        BentleyStatus FinishTableDefinitions(bool onlyCreateClassIdColumns = false) const;
        BentleyStatus UpdateECClassIdColumnIfRequired(DbTable&, bset<ClassMap*> const&) const;
        ClassMappingStatus AddClassMap(ClassMapPtr&) const;
        ClassMapsByTable GetClassMapsByTable() const;
        BentleyStatus GetClassMapsFromRelationshipEnd(std::set<ClassMap const*>&, ECN::ECClassCR, bool recursive) const;
        std::vector<ECN::ECClassCP> GetBaseClassesNotAlreadyMapped(ECN::ECClassCR ecclass) const;
        static void GatherRootClasses(ECN::ECClassCR ecclass, std::set<ECN::ECClassCP>& doneList, std::set<ECN::ECClassCP>& rootClassSet, std::vector<ECN::ECClassCP>& rootClassList, std::vector<ECN::ECRelationshipClassCP>& rootRelationshipList, std::vector<ECN::ECEntityClassCP>& rootMixins);

        BentleyStatus LogInvalidDbMappings() const;

    public:
        explicit ECDbMap(ECDbCR ecdb);
        ~ECDbMap() {}
        void ClearCache() const;

        ClassMap const* GetClassMap(ECN::ECClassCR) const;
        BentleyStatus TryGetClassMap(ClassMapPtr&, ClassMapLoadContext&, ECN::ECClassCR) const;

        std::vector<ECN::ECClassCP> GetFlattenListOfClassesFromRelationshipEnd(ECN::ECRelationshipConstraintCR) const;
        std::set<ClassMap const*> GetClassMapsFromRelationshipEnd(ECN::ECRelationshipConstraintCR, bool* hasAnyClass) const;
        //!Loads the class maps if they were not loaded yet
        size_t GetTableCountOnRelationshipEnd(ECN::ECRelationshipConstraintCR) const;
        BentleyStatus MapSchemas(SchemaImportContext&) const;

        bool IsImportingSchema() const;
        SchemaImportContext* GetSchemaImportContext() const;
        bool AssertIfIsNotImportingSchema() const;
        DbSchema const& GetDbSchema() const { return m_dbSchema; }
        DbSchema& GetDbSchemaR() const { return const_cast<DbSchema&> (m_dbSchema); }
        LightweightCache const& GetLightweightCache() const { return m_lightweightCache; }
        ECDbCR GetECDb() const { return m_ecdb; }
        IssueReporter const& Issues() const;

    };

END_BENTLEY_SQLITE_EC_NAMESPACE
