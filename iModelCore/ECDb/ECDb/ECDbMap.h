/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbMap.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
struct ECDbMap :NonCopyableClass
    {
    public:
        typedef bmap<DbTable*, bset<ClassMap*>> ClassMapsByTable;

    private:
        mutable BeMutex m_mutex;
        mutable bmap<ECN::ECClassId, ClassMapPtr> m_classMapDictionary;
        mutable LightweightCache m_lightweightCache;

        ECDbCR m_ecdb;
        DbSchema m_dbSchema;
        SchemaImportContext* m_schemaImportContext;
    private:
        BentleyStatus TryGetClassMap(ClassMapPtr&, ClassMapLoadContext&, ECN::ECClassCR) const;
        ClassMapPtr DoGetClassMap(ECN::ECClassCR) const;
        BentleyStatus TryLoadClassMap(ClassMapPtr&, ClassMapLoadContext& ctx, ECN::ECClassCR) const;
        MappingStatus DoMapSchemas();
        MappingStatus MapClass(ECN::ECClassCR);
        BentleyStatus SaveDbSchema() const;
        BentleyStatus CreateOrUpdateRequiredTables() const;
        BentleyStatus EvaluateColumnNotNullConstraints() const;
        BentleyStatus CreateOrUpdateIndexesInDb() const;
        BentleyStatus PurgeOrphanTables() const;
        BentleyStatus PurgeOrphanColumns() const;
        BentleyStatus FinishTableDefinitions(bool onlyCreateClassIdColumns = false) const;
        DbColumn const* CreateClassIdColumn(DbTable&, bset<ClassMap*> const&) const;
        MappingStatus AddClassMap(ClassMapPtr&) const;
        ClassMapsByTable GetClassMapsByTable() const;
        BentleyStatus GetClassMapsFromRelationshipEnd(std::set<ClassMap const*>&, ECN::ECClassCR, bool recursive) const;
        std::vector<ECN::ECClassCP> GetBaseClassesNotAlreadyMapped(ECN::ECClassCR ecclass) const;
        static void GatherRootClasses(ECN::ECClassCR ecclass, std::set<ECN::ECClassCP>& doneList, std::set<ECN::ECClassCP>& rootClassSet, std::vector<ECN::ECClassCP>& rootClassList, std::vector<ECN::ECRelationshipClassCP>& rootRelationshipList);

    public:
        explicit ECDbMap(ECDbCR ecdb);
        ~ECDbMap() {}

        ClassMap const* GetClassMap(ECN::ECClassCR) const;
        ClassMap const* GetClassMap(ECN::ECClassId) const;
        std::vector<ECN::ECClassCP> GetFlattenListOfClassesFromRelationshipEnd(ECN::ECRelationshipConstraintCR) const;
        std::set<ClassMap const*> GetClassMapsFromRelationshipEnd(ECN::ECRelationshipConstraintCR, bool* hasAnyClass) const;
        DbTable const* GetPrimaryTable(DbTable const& joinedTable) const;
        //!Loads the class maps if they were not loaded yet
        size_t GetTableCountOnRelationshipEnd(ECN::ECRelationshipConstraintCR) const;
        MappingStatus MapSchemas(SchemaImportContext&);
        BentleyStatus CreateECClassViewsInDb() const;
        DbTable* FindOrCreateTable(SchemaImportContext*, Utf8CP tableName, DbTable::Type, bool isVirtual, Utf8CP primaryKeyColumnName);
        void ClearCache();
        bool IsImportingSchema() const;
        SchemaImportContext* GetSchemaImportContext() const;
        bool AssertIfIsNotImportingSchema() const;
        DbTable* FindOrCreateTable(SchemaImportContext*, Utf8CP tableName, DbTable::Type, bool isVirtual, Utf8CP primaryKeyColumnName, DbTable const* baseTable);
        DbSchema const& GetDbSchema() const { return m_dbSchema; }
        DbSchema& GetDbSchemaR() const { return const_cast<DbSchema&> (m_dbSchema); }
        LightweightCache const& GetLightweightCache() const { return m_lightweightCache; }
        ECDbCR GetECDb() const { return m_ecdb; }
        IssueReporter const& Issues() const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
