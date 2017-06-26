/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbMap.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <vector>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
#ifdef INVOKE_RULE
#undef INVOKE_RULE
#endif
#define INVOKE_RULE(x) {if (IsError(x)) return m_status;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMap const* DbMap::GetClassMap(ECN::ECClassCR ecClass) const
    {
    ClassMapLoadContext ctx;
    ClassMap const* classMap = nullptr;
    if (SUCCESS != TryGetClassMap(classMap, ctx, ecClass))
        {
        BeAssert(false && "Error during TryGetClassMap");
        return nullptr;
        }

    if (classMap == nullptr)
        return nullptr;

    return classMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   02/2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::TryGetClassMap(ClassMapPtr& classMap, ClassMapLoadContext& ctx, ECN::ECClassCR ecClass) const
    {
    //we must use this method here and cannot just see whether ecClass has already an id
    //because the ecClass object can come from an ECSchema deserialized from disk, hence
    //not having the id set, and already imported in the ECSchema. In that case
    //ECDb does not set the ids on the ECClass objects
    if (!m_ecdb.Schemas().GetReader().GetClassId(ecClass).IsValid())
        {
        BeAssert(false && "ECClass must have an ECClassId when mapping to the ECDb.");
        return ERROR;
        }

    classMap = DoGetClassMap(ecClass);
    if (classMap != nullptr)
        return SUCCESS;

    //lazy loading the class map implemented with const-casting the actual loading so that the 
    //get method itself can remain const (logically const)
    return TryLoadClassMap(classMap, ctx, ecClass);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan         08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ClassMapPtr DbMap::DoGetClassMap(ECClassCR ecClass) const
    {
    auto it = m_classMapDictionary.find(ecClass.GetId());
    if (m_classMapDictionary.end() == it)
        return nullptr;
    else
        return it->second;
    }


//---------------------------------------------------------------------------------------
//* @bsimethod                                 Affan.Khan                           07 / 2012
//---------------------------------------------------------------------------------------
BentleyStatus DbMap::TryLoadClassMap(ClassMapPtr& classMap, ClassMapLoadContext& ctx, ECN::ECClassCR ecClass) const
    {
    classMap = nullptr;

    DbClassMapLoadContext classMapLoadContext;
    if (DbClassMapLoadContext::Load(classMapLoadContext, ctx, GetECDb(), ecClass) != SUCCESS)
        {
        //Failed to find classmap
        return SUCCESS;
        }

    MapStrategyExtendedInfo const& mapStrategy = classMapLoadContext.GetMapStrategy();
    ClassMapPtr classMapTmp = nullptr;
    if (mapStrategy.GetStrategy() == MapStrategy::NotMapped)
        classMapTmp = ClassMapFactory::CreateForLoading<NotMappedClassMap>(m_ecdb, ecClass, mapStrategy);
    else
        {
        ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP();
        if (ecRelationshipClass != nullptr)
            {
            if (MapStrategyExtendedInfo::IsForeignKeyMapping(mapStrategy))
                classMapTmp = ClassMapFactory::CreateForLoading<RelationshipClassEndTableMap>(m_ecdb, *ecRelationshipClass, mapStrategy);
            else
                classMapTmp = ClassMapFactory::CreateForLoading<RelationshipClassLinkTableMap>(m_ecdb, *ecRelationshipClass, mapStrategy);
            }
        else
            classMapTmp = ClassMapFactory::CreateForLoading<ClassMap>(m_ecdb, ecClass, mapStrategy);
        }

    if (ClassMappingStatus::Error == AddClassMap(classMapTmp))
        {
        BeAssert(false);
        return ERROR;
        }

    if (SUCCESS != classMapTmp->Load(ctx, classMapLoadContext))
        {
        BeAssert(false);
        return ERROR;
        }

    classMap = classMapTmp;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    04/2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::MapSchemas(SchemaImportContext& ctx, bvector<ECN::ECSchemaCP> const& schemas) const
    {
    if (schemas.empty())
        return SUCCESS;

    PERFLOG_START("ECDb", "Schema import> Map schemas");

    if (SUCCESS != DoMapSchemas(ctx, schemas))
        return ERROR;

    PERFLOG_START("ECDb", "Schema import> Persist mappings");
    if (SUCCESS != SaveDbSchema(ctx))
        {
        ClearCache();
        return ERROR;
        }
    PERFLOG_FINISH("ECDb", "Schema import> Persist mappings");

    PERFLOG_START("ECDb", "Schema import> Create or update tables");
    if (SUCCESS != CreateOrUpdateRequiredTables())
        {
        ClearCache();
        return ERROR;
        }
    PERFLOG_FINISH("ECDb", "Schema import> Create or update tables");
    PERFLOG_START("ECDb", "Schema import> Create or update indexes");
    if (SUCCESS != CreateOrUpdateIndexesInDb(ctx))
        {
        ClearCache();
        return ERROR;
        }
    PERFLOG_FINISH("ECDb", "Schema import> Create or update indexes");

    PERFLOG_START("ECDb", "Schema import> Purge orphan tables");
    if (SUCCESS != PurgeOrphanTables())
        {
        ClearCache();
        return ERROR;
        }
    PERFLOG_FINISH("ECDb", "Schema import> Purge orphan tables");

    const BentleyStatus stat = Validate(ctx, ctx.GetOptions() != SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues);
    ClearCache();
    PERFLOG_FINISH("ECDb", "Schema import> Map schemas");
    return stat;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan         03/2016
//---------------------------------------------------------------------------------------
void DbMap::GatherRootClasses(ECClassCR ecclass, std::set<ECClassCP>& doneList, std::set<ECClassCP>& rootClassSet, std::vector<ECClassCP>& rootClassList, std::vector<ECRelationshipClassCP>& rootRelationshipList, std::vector<ECN::ECEntityClassCP>& rootMixins)
    {
    if (doneList.find(&ecclass) != doneList.end())
        return;

    doneList.insert(&ecclass);
    if (!ecclass.HasBaseClasses())
        {
        ECEntityClassCP entityClass = ecclass.IsEntityClass() ? ecclass.GetEntityClassCP() : nullptr;
        if (rootClassSet.find(&ecclass) == rootClassSet.end())
            {
            rootClassSet.insert(&ecclass);
            if (ecclass.IsRelationshipClass())
                rootRelationshipList.push_back(ecclass.GetRelationshipClassCP());
            else
                {
                if (entityClass && entityClass->IsMixin())
                    rootMixins.push_back(entityClass);
                else
                    rootClassList.push_back(&ecclass);
                }
            }

        return;
        }

    for (ECClassCP baseClass : ecclass.GetBaseClasses())
        {
        if (baseClass == nullptr)
            continue;

        if (doneList.find(baseClass) != doneList.end())
            return;

        GatherRootClasses(*baseClass, doneList, rootClassSet, rootClassList, rootRelationshipList, rootMixins);
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan         09/2012
//---------------------------------------------------------------------------------------
BentleyStatus DbMap::DoMapSchemas(SchemaImportContext& ctx, bvector<ECN::ECSchemaCP> const& schemas) const
    {
    ctx.SetPhase(SchemaImportContext::Phase::MappingSchemas);
    // Identify root classes/relationship-classes
    std::set<ECClassCP> doneList;
    std::set<ECClassCP> rootClassSet;
    std::vector<ECClassCP> rootClassList;
    std::vector<ECN::ECEntityClassCP> rootMixins;
    std::vector<ECRelationshipClassCP> rootRelationshipList;

    for (ECSchemaCP schema : schemas)
        {
        if (schema->IsSupplementalSchema())
            continue; // Don't map any supplemental schemas

        for (ECClassCP ecClass : schema->GetClasses())
            {
            if (doneList.find(ecClass) != doneList.end())
                continue;

            GatherRootClasses(*ecClass, doneList, rootClassSet, rootClassList, rootRelationshipList, rootMixins);
            }
        }

    if (GetDbSchemaR().SynchronizeExistingTables() != SUCCESS)
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report("Synchronizing existing table to which classes are mapped failed.");
        return ERROR;
        }

    // Map mixin hierarchy before everything else. It does not map primary hierarchy and all classes map to virtual tables.
    PERFLOG_START("ECDb", "Schema import> Map mixins");
    ctx.SetPhase(SchemaImportContext::Phase::MappingMixins);
    for (ECEntityClassCP mixin : rootMixins)
        {
        if (ClassMappingStatus::Error == MapClass(ctx, *mixin))
            return ERROR;
        }
    PERFLOG_FINISH("ECDb", "Schema import> Map mixins");

    // Starting with the root, recursively map the entire class hierarchy. 
    PERFLOG_START("ECDb", "Schema import> Map entity classes");
    ctx.SetPhase(SchemaImportContext::Phase::MappingEntities);
    for (ECClassCP rootClass : rootClassList)
        {
        if (ClassMappingStatus::Error == MapClass(ctx, *rootClass))
            return ERROR;
        }
    PERFLOG_FINISH("ECDb", "Schema import> Map entity classes");

    PERFLOG_START("ECDb", "Schema import> Map relationships");
    ctx.SetPhase(SchemaImportContext::Phase::MappingRelationships);
    for (ECRelationshipClassCP rootRelationshipClass : rootRelationshipList)
        {
        if (ClassMappingStatus::Error == MapClass(ctx, *rootRelationshipClass))
            return ERROR;
        }

    if (ctx.FinishEndTableMapping() != ClassMappingStatus::Success)
        return ERROR;

    PERFLOG_FINISH("ECDb", "Schema import> Map relationships");

    ctx.SetPhase(SchemaImportContext::Phase::CreatingUserDefinedIndexes);
    for (auto& kvpair : ctx.GetClassMappingInfoCache())
        {
        if (SUCCESS != kvpair.first->CreateUserProvidedIndexes(ctx, kvpair.second->GetIndexInfos()))
            return ERROR;
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
 ClassMappingStatus DbMap::MapClass(SchemaImportContext& ctx, ECClassCR ecClass) const
     {
     ClassMapPtr existingClassMap = nullptr;
     if (SUCCESS != TryGetClassMap(existingClassMap, ctx.GetClassMapLoadContext(), ecClass))
         return ClassMappingStatus::Error;

     if (existingClassMap == nullptr)
         {
         ClassMappingStatus status = ClassMappingStatus::Success;
         std::unique_ptr<ClassMappingInfo> classMapInfo = ClassMappingInfoFactory::Create(status, ctx, m_ecdb, ecClass);
         if ((status == ClassMappingStatus::BaseClassesNotMapped || status == ClassMappingStatus::Error))
             return status;

         BeAssert(classMapInfo != nullptr);
         MapStrategyExtendedInfo const& mapStrategy = classMapInfo->GetMapStrategy();
         ClassMapPtr classMap = nullptr;
         if (mapStrategy.GetStrategy() == MapStrategy::NotMapped)
             classMap = ClassMapFactory::CreateForMapping<NotMappedClassMap>(m_ecdb, ecClass, mapStrategy);
         else
             {
             auto ecRelationshipClass = ecClass.GetRelationshipClassCP();
             if (ecRelationshipClass != nullptr)
                 {
                 if (MapStrategyExtendedInfo::IsForeignKeyMapping(mapStrategy))
                     classMap = ClassMapFactory::CreateForMapping<RelationshipClassEndTableMap>(m_ecdb, *ecRelationshipClass, mapStrategy);
                 else
                     classMap = ClassMapFactory::CreateForMapping<RelationshipClassLinkTableMap>(m_ecdb, *ecRelationshipClass, mapStrategy);
                 }
             else
                 classMap = ClassMapFactory::CreateForMapping<ClassMap>(m_ecdb, ecClass, mapStrategy);
             }

         status = AddClassMap(classMap);
         if (status == ClassMappingStatus::Error)
             return status;

         ctx.AddClassMapForSaving(ecClass.GetId());
         status = classMap->Map(ctx, *classMapInfo);
         ctx.CacheClassMapInfo(*classMap, classMapInfo);         
         if (status == ClassMappingStatus::BaseClassesNotMapped || status == ClassMappingStatus::Error)
             return status;
          
         }
     else
         {
         if (existingClassMap->Update(ctx) == ERROR)
             return ClassMappingStatus::Error;
         }

     const bool isCurrentIsMixin = ecClass.IsEntityClass() && ecClass.GetEntityClassCP()->IsMixin();
     for (ECClassCP childClass : ecClass.GetDerivedClasses())
         {
         const bool isChildIsMixin = childClass->IsEntityClass() && childClass->GetEntityClassCP()->IsMixin();
         //Only map mixin hierarchy but stop if you find a non-mixin class.
         if (isCurrentIsMixin && !isChildIsMixin)
             continue;

         ClassMappingStatus status = MapClass(ctx, *childClass);
         if (status == ClassMappingStatus::Error)
             return status;
         }

     return ClassMappingStatus::Success;
     }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ClassMappingStatus DbMap::AddClassMap(ClassMapPtr& classMap) const
    {
    ECClassCR ecClass = classMap->GetClass();
    if (m_classMapDictionary.end() != m_classMapDictionary.find(ecClass.GetId()))
        {
        LOG.errorv("Attempted to add a second ClassMap for ECClass %s", ecClass.GetFullName());
        BeAssert(false && "Attempted to add a second ClassMap for the same ECClass");
        return ClassMappingStatus::Error;
        }

    m_classMapDictionary[ecClass.GetId()] = classMap;
    return ClassMappingStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      12/2011
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::CreateOrUpdateRequiredTables() const
    {
    m_ecdb.GetStatementCache().Empty();

    int nCreated = 0;
    int nUpdated = 0;
    int nWasUpToDate = 0;

    for (DbTable const* table : GetDbSchemaR().GetCachedTables())
        {
        const DbSchemaPersistenceManager::CreateOrUpdateTableResult result = DbSchemaPersistenceManager::CreateOrUpdateTable(m_ecdb, *table);
        switch (result)
            {
                case DbSchemaPersistenceManager::CreateOrUpdateTableResult::Created:
                    nCreated++;
                    break;

                case DbSchemaPersistenceManager::CreateOrUpdateTableResult::Updated:
                    nUpdated++;
                    break;

                case DbSchemaPersistenceManager::CreateOrUpdateTableResult::WasUpToDate:
                    nWasUpToDate++;
                    break;

                case DbSchemaPersistenceManager::CreateOrUpdateTableResult::Error:
                    return ERROR;

                default:
                case DbSchemaPersistenceManager::CreateOrUpdateTableResult::Skipped:
                    continue;
            }
        }

    LOG.debugv("Created %d tables, updated %d tables, and %d tables were up-to-date.", nCreated, nUpdated, nWasUpToDate);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2016
//---------------------------------------------------------------------------------------
BentleyStatus DbMap::CreateOrUpdateIndexesInDb(SchemaImportContext& ctx) const
    {
    std::vector<DbIndex const*> indexes;
    for (std::unique_ptr<DbIndex> const& indexPtr : m_dbSchema.GetIndexes())
        {
        indexes.push_back(indexPtr.get());
        }

    IndexMappingInfoCache indexInfoCache(m_ecdb, ctx);
    for (DbIndex const* index : indexes)
        {
        const ECClassId classId = index->GetClassId();
        if (!classId.IsValid())
            continue;

        ECClassCP ecClass = m_ecdb.Schemas().GetClass(classId);
        if (ecClass == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        ClassMap const* classMap = nullptr;
        if (SUCCESS != TryGetClassMap(classMap, ctx.GetClassMapLoadContext(), *ecClass))
            {
            BeAssert(false);
            return ERROR;
            }

        StorageDescription const& storageDesc = classMap->GetStorageDescription();
        std::vector<Partition> const& horizPartitions = storageDesc.GetHorizontalPartitions();

        std::vector<IndexMappingInfoPtr> const* baseClassIndexInfos = nullptr;
        if (SUCCESS != indexInfoCache.TryGetIndexInfos(baseClassIndexInfos, *classMap))
            return ERROR;

        BeAssert(baseClassIndexInfos != nullptr);

        DbTable const& indexTable = index->GetTable();
        for (Partition const& horizPartition : horizPartitions)
            {
            if (&indexTable == &horizPartition.GetTable())
                continue;

            bset<DbTable const*> alreadyProcessedTables;
            //table of index doesn't need to be processed again either, so put it in the set, too
            alreadyProcessedTables.insert(&indexTable);

            bset<ECClassId> horizPartitionClassIds;
            horizPartitionClassIds.insert(horizPartition.GetClassIds().begin(), horizPartition.GetClassIds().end());
            for (ECClassId derivedClassId : horizPartition.GetClassIds())
                {
                ECClassCP derivedClass = m_ecdb.Schemas().GetClass(derivedClassId);
                if (derivedClass == nullptr)
                    {
                    BeAssert(false);
                    return ERROR;
                    }

                bool needsSeparateIndex = true;
                for (ECClassCP baseClass : derivedClass->GetBaseClasses())
                    {
                    //if derivedClass is a subclass of another class of this horiz partition
                    //we will not create an index for it. Indexes apply to subclasses implicitly, so 
                    //no need to create a separate index per subclass
                    if (horizPartitionClassIds.find(baseClass->GetId()) != horizPartitionClassIds.end())
                        {
                        needsSeparateIndex = false;
                        break;
                        }
                    }
                
                if (!needsSeparateIndex)
                    continue;

                ClassMap const* derivedClassMap = nullptr;
                if (SUCCESS != TryGetClassMap(derivedClassMap, ctx.GetClassMapLoadContext(), *derivedClass))
                    {
                    BeAssert(false);
                    return ERROR;
                    }

                DbTable const& joinedOrSingleTable = derivedClassMap->GetJoinedOrPrimaryTable();
                if (alreadyProcessedTables.find(&joinedOrSingleTable) != alreadyProcessedTables.end())
                    continue;

                std::vector<IndexMappingInfoPtr> indexMappingInfos;
                for (IndexMappingInfoPtr const& indexMappingInfo : *baseClassIndexInfos)
                    {
                    Utf8String indexName;
                    if (!indexMappingInfo->GetName().IsNull())
                        indexName.append(indexMappingInfo->GetName().Value()).append("_").append(joinedOrSingleTable.GetName());

                    indexMappingInfos.push_back(IndexMappingInfo::Clone(Nullable<Utf8String>(indexName), *indexMappingInfo));
                    }

                if (SUCCESS != derivedClassMap->CreateUserProvidedIndexes(ctx, indexMappingInfos))
                    return ERROR;

                alreadyProcessedTables.insert(&joinedOrSingleTable);
                }
            }
        }

    return m_dbSchema.CreateOrUpdateIndexes();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2016
//---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus DbMap::PurgeOrphanTables() const
    {
    //skip ExistingTable and NotMapped
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, "SELECT t.Id, t.Name, t.Type= " SQLVAL_DbTable_Type_Virtual " FROM ec_Table t "
                                     "WHERE t.Type<> " SQLVAL_DbTable_Type_Existing " AND t.Name<>'" DBSCHEMA_NULLTABLENAME "' AND t.Id NOT IN ("
                                     "SELECT DISTINCT ec_Table.Id FROM ec_PropertyMap "
                                     "INNER JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.PropertyPathId "
                                     "INNER JOIN ec_Property ON ec_PropertyPath.RootPropertyId = ec_Property.Id "
                                     "INNER JOIN ec_Column ON ec_PropertyMap.ColumnId = ec_Column.Id "
                                     "INNER JOIN ec_Table ON ec_Column.TableId = ec_Table.Id)"))
        {
        BeAssert(false && "ECDb profile changed");
        return ERROR;
        }

    IdSet<DbTableId> orphanTables;
    std::vector<Utf8String> tablesToDrop;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        orphanTables.insert(stmt.GetValueId<DbTableId>(0));
        if (!stmt.GetValueBoolean(2))
            tablesToDrop.push_back(stmt.GetValueText(1));
        }
    stmt.Finalize();

    if (orphanTables.empty())
        return SUCCESS;

    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, "DELETE FROM ec_Table WHERE InVirtualSet(?,Id)") ||
        BE_SQLITE_OK != stmt.BindVirtualSet(1, orphanTables) ||
        BE_SQLITE_DONE != stmt.Step())
        {
        BeAssert(false);
        return ERROR;
        }

    for (Utf8StringCR table : tablesToDrop)
        {
        GetDbSchema().RemoveCacheTable(table);
        }

    stmt.Finalize();

    if (tablesToDrop.empty())
        return SUCCESS;

    if (!m_ecdb.GetECDbImplR().GetSettings().AllowChangesetMergingIncompatibleSchemaImport())
        {
        Utf8String tableNames;
        bool isFirstTable = true;
        for (Utf8StringCR tableName : tablesToDrop)
            {
            if (!isFirstTable)
                tableNames.append(",");

            tableNames.append(tableName);
            isFirstTable = false;
            }

        m_ecdb.GetECDbImplR().GetIssueReporter().Report(
            "Failed to import schemas: it would change the database schema in a changeset-merging incompatible way. ECDb would have to delete these tables: %s", tableNames.c_str());
        return ERROR;
        }

    for (Utf8StringCR name : tablesToDrop)
        {
        if (m_ecdb.DropTable(name.c_str()) != BE_SQLITE_OK)
            {
            BeAssert(false && "failed to drop a table");
            return ERROR;
            }

        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// Gets the count of tables at the specified end of a relationship class.
// @param  relationshpEnd [in] Constraint at the end of the relationship
// @return Number of tables at the specified end of the relationship. Returns
//         std::numeric_limits<size_t>::max() if the end is AnyClass.
// @bsimethod                                 Ramanujam.Raman                05/2012
//---------------------------------------------------------------------------------------
size_t DbMap::GetTableCountOnRelationshipEnd(SchemaImportContext& ctx, ECRelationshipConstraintCR relationshipEnd) const
    {
    bool hasAnyClass = false;
    std::set<ClassMap const*> classMaps = GetClassMapsFromRelationshipEnd(ctx, relationshipEnd, &hasAnyClass);

    if (hasAnyClass)
        return std::numeric_limits<size_t>::max();

    const bool abstractEndPoint = relationshipEnd.GetConstraintClasses().size() == 1 && relationshipEnd.GetConstraintClasses().front()->GetClassModifier() == ECClassModifier::Abstract;
    
    std::set<DbTable const*> nonVirtualTables;
    bool hasAtLeastOneVirtualTable = false;
    for (ClassMap const* classMap : classMaps)
        {
        DbTable const* table = abstractEndPoint ? &classMap->GetJoinedOrPrimaryTable() : &classMap->GetPrimaryTable();
        if (classMap->GetPrimaryTable().GetType() == DbTable::Type::Virtual)
            hasAtLeastOneVirtualTable = true;
        else
            nonVirtualTables.insert(table);
        }

    if (!nonVirtualTables.empty())
        return nonVirtualTables.size();

    return hasAtLeastOneVirtualTable ? 1 : 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2015
//+---------------+---------------+---------------+---------------+---------------+------
std::set<ClassMap const*> DbMap::GetClassMapsFromRelationshipEnd(SchemaImportContext& ctx, ECRelationshipConstraintCR constraint, bool* hasAnyClass) const
    {
    if (hasAnyClass != nullptr)
        *hasAnyClass = false;

    std::set<ClassMap const*> classMaps;
    for (ECClassCP ecClass : constraint.GetConstraintClasses())
        {
        if (ClassMap::IsAnyClass(*ecClass))
            {
            if (hasAnyClass)
                *hasAnyClass = true;

            classMaps.clear();
            return classMaps;
            }

        ClassMap const* classMap = nullptr;
        if (SUCCESS != TryGetClassMap(classMap, ctx.GetClassMapLoadContext(), *ecClass))
            {
            BeAssert(false);
            classMaps.clear();
            return classMaps;
            }

        const bool recursive = classMap->GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy && constraint.GetIsPolymorphic();
        if (SUCCESS != GetClassMapsFromRelationshipEnd(ctx, classMaps, *ecClass, recursive))
            {
            BeAssert(false);
            classMaps.clear();
            return classMaps;
            }
        }

    return classMaps;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::GetClassMapsFromRelationshipEnd(SchemaImportContext& ctx, std::set<ClassMap const*>& classMaps, ECClassCR ecClass, bool recursive) const
    {
    ClassMap const* classMap = nullptr;
    if (SUCCESS != TryGetClassMap(classMap, ctx.GetClassMapLoadContext(), ecClass))
        {
        BeAssert(classMap != nullptr && "ClassMap should not be null");
        return ERROR;
        }

    if (classMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        return SUCCESS;

    classMaps.insert(classMap);

    if (!recursive)
        return SUCCESS;

    for (ECClassCP subclass : m_ecdb.Schemas().GetDerivedClasses(ecClass))
        {
        if (SUCCESS != GetClassMapsFromRelationshipEnd(ctx, classMaps, *subclass, recursive))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::SaveDbSchema(SchemaImportContext& ctx) const
    {
    if (m_dbSchema.SaveOrUpdateTables() != SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    DbMapSaveContext saveCtx(GetECDb());
    for (bpair<ECClassId, ClassMapPtr> const& kvPair : m_classMapDictionary)
        {
        ClassMap& classMap = *kvPair.second;
        if (classMap.GetState() == ObjectState::Persisted)
            continue;

        if (SUCCESS != classMap.Save(ctx, saveCtx))
            {
            Issues().Report("Failed to save mapping for ECClass %s: %s", classMap.GetClass().GetFullName(), m_ecdb.GetLastError().c_str());
            return ERROR;
            }

        if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
            classMap.GetAs<RelationshipClassEndTableMap>().RestPartitionCache();
        }

    if (SUCCESS != DbSchemaPersistenceManager::RepopulateClassHasTableCacheTable(GetECDb()))
        return ERROR;

    m_lightweightCache.Reset();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus DbMap::Validate(SchemaImportContext& ctx, bool failOnError) const
    {
    PERFLOG_START("ECDb", "Validate");
    BentleyStatus status = Validator(*this, ctx).CheckAndReportIssues(true, Validator::Filter::InMemory, Validator::Filter::InMemory);
    PERFLOG_FINISH("ECDb", "Validate");
    if (status == ERROR && failOnError)
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2012
//+---------------+---------------+---------------+---------------+---------------+------
void DbMap::ClearCache() const
    {
    m_classMapDictionary.clear();
    m_dbSchema.Reset();
    m_lightweightCache.Reset();
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckDbSchema(DbSchema const& dbschema, Filter filter) const
    {
    std::vector<DbTable const*> tables;
    if (filter == Filter::All)
        {
        CachedStatementPtr stmt = dbschema.GetECDb().GetCachedStatement("SELECT Id FROM ec_Table");
        PRECONDITION(stmt != nullptr, ERROR);
        while (stmt->Step() == BE_SQLITE_ROW)
            {
            DbTable const* table = dbschema.FindTable(stmt->GetValueId<DbTableId>(0));
            PRECONDITION(table != nullptr, ERROR);
            tables.push_back(table);
            }
        }
    else
        tables = dbschema.GetCachedTables();

    INVOKE_RULE(CheckDbTables(tables));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckDbMap(DbMap  const& map, Filter filter) const
    {
    INVOKE_RULE(CheckMapCount(map));
    std::vector<ClassMap const*> classMaps;
    if (filter == Filter::All)
        {
        CachedStatementPtr stmt = map.GetECDb().GetCachedStatement("SELECT Id FROM ec_Class");
        PRECONDITION(stmt != nullptr, ERROR);
        while (stmt->Step() == BE_SQLITE_ROW)
            {
            ECClassCP ecClass = map.GetECDb().Schemas().GetClass(stmt->GetValueId<ECClassId>(0));
            PRECONDITION(ecClass != nullptr, ERROR);
            ClassMap const* classMap = map.GetClassMap(*ecClass);
            PRECONDITION(classMap != nullptr, ERROR);
            classMaps.push_back(classMap);
            }
        }
    else
        {
        Statement relStmt;
        relStmt.Prepare(map.GetECDb(), "SELECT DISTINCT RelationshipClassId FROM ec_RelationshipConstraint");
        while (relStmt.Step() == BE_SQLITE_ROW)
            {
            if (ECClassCP ecClass = map.GetECDb().Schemas().GetClass(relStmt.GetValueId<ECClassId>(0)))
                PRECONDITION(map.GetClassMap(*ecClass) != nullptr, ERROR);
            }

        for (auto const& entry : map.m_classMapDictionary)
            classMaps.push_back(entry.second.get());
        }

    for (ClassMapCP classMap : classMaps)
        INVOKE_RULE(CheckClassMap(*classMap));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckMapCount(DbMap  const& map) const
    {
    CachedStatementPtr stmt0 = map.GetECDb().GetCachedStatement("SELECT COUNT(Id) FROM ec_Class");
    CachedStatementPtr stmt1 = map.GetECDb().GetCachedStatement("SELECT COUNT(ClassId) FROM ec_ClassMap");
    PRECONDITION(stmt0->Step() == BE_SQLITE_ROW, ERROR);
    PRECONDITION(stmt1->Step() == BE_SQLITE_ROW, ERROR);
    if (stmt0->GetValueInt(0) != stmt1->GetValueInt(0))
        return Error("Rows in ec_Class(rows = %d) table is not same as in ec_ClassMa p(rows = %d).", stmt0->GetValueInt(0), stmt1->GetValueInt(0));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckDbIndexes(DbSchema const& dbSchemas) const
    {
    for (auto const& index : dbSchemas.GetIndexes())
        INVOKE_RULE(CheckDbIndex(*index));

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckDbTables(std::vector<DbTable const*> const& tables) const
    {
    for (DbTable const* table : tables)
        INVOKE_RULE(CheckDbTable(*table));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckDbTable(DbTable const& table) const
    {
    if (table.GetName() == DBSCHEMA_NULLTABLENAME )
        {
        if (!table.GetColumns().empty() || table.GetType() != DbTable::Type::Virtual)
            {
            return Error("DbTable '%s' should have no column and must be virtual.", table.GetName().c_str());
            }

        return SUCCESS;
        }

    if (table.GetType() != DbTable::Type::Existing &&
        table.GetType() != DbTable::Type::Joined &&
        table.GetType() != DbTable::Type::Overflow &&
        table.GetType() != DbTable::Type::Primary &&
        table.GetType() != DbTable::Type::Virtual)
        {
        return Error("DbTable '%s' has a unsupported value for DbTable::Type.", table.GetName().c_str());
        }

    if (table.GetType() != DbTable::Type::Virtual)
        {
        if (!m_map.GetECDb().TableExists(table.GetName().c_str()))
            return Error("DbTable '%s' is of type none-virtual and must exist in db.", table.GetName().c_str());
        }

    if (table.GetColumns().size() == 0)
        {
        return Error("DbTable '%s' has no columns.", table.GetName().c_str());
        }

    bvector<Utf8String> columns;
    m_map.GetECDb().GetColumns(columns, table.GetName().c_str());
    bset<Utf8String, CompareIUtf8Ascii> existingColumns(columns.begin(), columns.end());
    size_t nPhysicalColumns = 0;
    for (DbColumn const* column : table.GetColumns())
        {
        if (!column->IsVirtual() && table.GetType() != DbTable::Type::Virtual)
            {
            if (existingColumns.find(column->GetName()) == existingColumns.end())
                return Error("DbTable '%s' has column '%s' which is not present in underlaying db.", table.GetName().c_str(), column->GetName().c_str());

            nPhysicalColumns++;
            }

        INVOKE_RULE(CheckDbColumn(*column));
        }

    if (table.FindFirst(DbColumn::Kind::ECInstanceId) == nullptr)
        {
            return Error("DbTable '%s'must have ECInstanceId type column.", table.GetName().c_str());
        }

    if (table.FindFirst(DbColumn::Kind::ECClassId) == nullptr)
        {
        return Error("DbTable '%s'must have ECClassId type column.", table.GetName().c_str());
        }

    if (table.GetType() != DbTable::Type::Existing && table.GetType() != DbTable::Type::Virtual)
        {
        if (nPhysicalColumns != columns.size())
            return Error("DbTable '%s'has column list that is is not identical to one in underlaying db.", table.GetName().c_str());
        }

    if (table.GetType() != DbTable::Type::Existing && table.GetType() != DbTable::Type::Virtual)
        {
        if (nPhysicalColumns != columns.size())
            return Error("DbTable '%s'has column list that is is not identical to one in underlaying db.", table.GetName().c_str());
        }

    if (table.GetType() == DbTable::Type::Existing)
        {
        if (table.GetLinkNode().GetParent() != nullptr)
            return Error("DbTable '%s'is of type 'Existing' and therefore it must not have parent table.", table.GetName().c_str());
        }

    if (table.GetType() == DbTable::Type::Virtual)
        {
        if (table.GetLinkNode().GetParent() != nullptr)
            return Error("DbTable '%s'is of type 'Virtual' and therefore it must not have parent table.", table.GetName().c_str());

        if (!table.GetLinkNode().GetChildren().empty())
            return Error("DbTable '%s'is of type 'Virtual' and therefore it must not have any child tables.", table.GetName().c_str());
        }

    if (table.GetType() == DbTable::Type::Primary)
        {
        if (table.GetLinkNode().GetParent() != nullptr)
            return Error("DbTable '%s'is of type 'Primary' and therefore it must not have parent table.", table.GetName().c_str());
        }

    if (table.GetType() == DbTable::Type::Joined)
        {
        if (table.GetLinkNode().GetParent() == nullptr)
            return Error("DbTable '%s'is of type 'Joined' and therefore it must have parent table.", table.GetName().c_str());

        if (table.GetLinkNode().GetChildren().size() > 1)
            return Error("DbTable '%s'is of type 'Joined' and therefore it must not have more than one child table.", table.GetName().c_str());
        }

    if (table.GetType() == DbTable::Type::Overflow)
        {
        if (table.GetLinkNode().GetParent() == nullptr)
            return Error("DbTable '%s'is of type 'Overflow' and therefore it must have parent table.", table.GetName().c_str());

        if (!table.GetLinkNode().GetChildren().empty())
            return Error("DbTable '%s'is of type 'Virtual' and therefore it must not have any child tables.", table.GetName().c_str());
        }

    INVOKE_RULE(CheckDbConstraints(table));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckDbConstraints(DbTable const& table) const
    {
    for (DbConstraint const* constraint : table.GetConstraints())
        INVOKE_RULE(CheckDbConstraint(*constraint));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckDbTriggers(DbTable const& table) const
    {
    for (DbTrigger const* trigger : table.GetTriggers())
        INVOKE_RULE(CheckDbTrigger(*trigger));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckDbConstraint(DbConstraint const& constraint) const
    {
    if (constraint.GetType() == DbConstraint::Type::ForeignKey)
        return CheckForeignKeyDbConstraint(static_cast<ForeignKeyDbConstraint const&> (constraint));

    return CheckPrimaryKeyDbConstraint(static_cast<PrimaryKeyDbConstraint const&> (constraint));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckClassMap(ClassMap const& classMap) const
    {
    if (classMap.GetType() == ClassMap::Type::NotMapped)
        return CheckNotMappedClassMap(classMap.GetAs<NotMappedClassMap>());

    if (classMap.GetType() != ClassMap::Type::Class && classMap.GetType() == ClassMap::Type::RelationshipEndTable && classMap.GetType() == ClassMap::Type::RelationshipLinkTable)
        return Error("Map Validation Rule Failed : Map for ECClass '%s' has unsupported value for  ClassMap::Type.", classMap.GetClass().GetFullName());

    if (classMap.GetPropertyMaps().Find(ECDBSYS_PROP_ECInstanceId) == nullptr)
        return Error("ECClass '%s' map must have '" ECDBSYS_PROP_ECInstanceId "' property map.", classMap.GetClass().GetFullName());

    if (classMap.GetPropertyMaps().Find(ECDBSYS_PROP_ECClassId) == nullptr)
        return Error("ECClass '%s' map must have '" ECDBSYS_PROP_ECClassId "' property map.", classMap.GetClass().GetFullName());


    INVOKE_RULE(CheckIfAllPropertiesAreMapped(classMap));
    INVOKE_RULE(CheckPropertyMaps(classMap.GetPropertyMaps()));
    if (classMap.IsRelationshipClassMap())
        INVOKE_RULE(CheckRelationshipClassMap(classMap.GetAs<RelationshipClassMap>()));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckRelationshipClassMap(RelationshipClassMap const& rel) const
    {
    if (rel.GetPropertyMaps().Find(ECDBSYS_PROP_SourceECInstanceId) == nullptr)
        return Error("Relationship '%s' map must have '" ECDBSYS_PROP_SourceECInstanceId "' property map.", rel.GetClass().GetFullName());

    if (rel.GetPropertyMaps().Find(ECDBSYS_PROP_SourceECClassId) == nullptr)
        return Error("Relationship '%s' map must have '" ECDBSYS_PROP_SourceECClassId "' property map.", rel.GetClass().GetFullName());

    if (rel.GetPropertyMaps().Find(ECDBSYS_PROP_TargetECInstanceId) == nullptr)
        return Error("Relationship '%s' map must have '" ECDBSYS_PROP_TargetECInstanceId "' property map.", rel.GetClass().GetFullName());

    if (rel.GetPropertyMaps().Find(ECDBSYS_PROP_TargetECClassId) == nullptr)
        return Error("Relationship '%s' map must have '" ECDBSYS_PROP_TargetECClassId "' property map.", rel.GetClass().GetFullName());

    if (rel.GetType() == ClassMap::Type::RelationshipEndTable)
        return CheckRelationshipClassEndTableMap(rel.GetAs<RelationshipClassEndTableMap>());

    return CheckRelationshipClassLinkTableMap(rel.GetAs<RelationshipClassLinkTableMap>());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckNotMappedClassMap(NotMappedClassMap const& classMap) const
    {
    if (classMap.GetPropertyMaps().Size() != 0)
        return Error("Class '%s' is not mapped and must have no property map.", classMap.GetClass().GetFullName());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckDbTrigger(DbTrigger const& trigger) const { return SUCCESS; }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckDbColumn(DbColumn const& column) const 
    { 
    //if (column.IsShared() && column.GetConstraints().HasNotNullConstraint())
    //    return Error("DbColumn '%s.%s' has constraint 'NOT NULL' and is also marked as 'Shared' column which is invalid.", column.GetTable().GetName().c_str(), column.GetName().c_str());

    //if (column.IsShared() && column.GetConstraints().HasUniqueConstraint())
    //    return Error("DbColumn '%s.%s' has constraint 'UNIOUE' and is also marked as 'Shared' column which is invalid.", column.GetTable().GetName().c_str(), column.GetName().c_str());
   

    if (column.GetType()!=DbColumn::Type::Any &&
        column.GetType() != DbColumn::Type::Blob &&
        column.GetType() != DbColumn::Type::Boolean &&
        column.GetType() != DbColumn::Type::Integer &&
        column.GetType() != DbColumn::Type::Real &&
        column.GetType() != DbColumn::Type::Text &&
        column.GetType() != DbColumn::Type::TimeStamp)
        return Error("DbColumn '%s.%s' has a unsupported value for DbColumn::Type.", column.GetTable().GetName().c_str(), column.GetName().c_str());

    if (
        column.GetConstraints().GetCollation() != DbColumn::Constraints::Collation::Binary &&
        column.GetConstraints().GetCollation() != DbColumn::Constraints::Collation::NoCase &&
        column.GetConstraints().GetCollation() != DbColumn::Constraints::Collation::RTrim &&
        column.GetConstraints().GetCollation() != DbColumn::Constraints::Collation::Unset)
        return Error("DbColumn '%s.%s' has a unsupported value for  DbColumn::Constraints::Collation.", column.GetTable().GetName().c_str(), column.GetName().c_str());

    return SUCCESS; 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckDbIndex(DbIndex const& index) const
    { 
    if (index.GetColumns().empty())
        return Error("Index '%s' must have alteast one column.", index.GetName().c_str());

    return SUCCESS; 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckForeignKeyDbConstraint(ForeignKeyDbConstraint const& constraint) const
    {
    if (constraint.GetFkColumns().empty())
        return Error("DbTable '%s' has ForeignKey constraint that has no column specified in it.", constraint.GetTable().GetName().c_str());

    if (constraint.GetReferencedTableColumns().empty())
        return Error("DbTable '%s' has Referenced side that has no column specified in it.", constraint.GetTable().GetName().c_str());

    if (constraint.GetFkColumns().size() != constraint.GetReferencedTableColumns().size())
        return Error("DbTable '%s' has ForeignKey constraint that has different number of columns in ForeignKey and Referenced side.", constraint.GetTable().GetName().c_str());
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckPrimaryKeyDbConstraint(PrimaryKeyDbConstraint const& constraint) const
    {
    if (constraint.GetColumns().empty())
        return Error("DbTable '%s' has primary key constraint that has no column specified in it.", constraint.GetTable().GetName().c_str());

    if (constraint.GetColumns().size() != 1)
        return Error("DbTable '%s' has primary key constraint has more than one column which is yet not supported in ECDb.", constraint.GetTable().GetName().c_str());

    if (constraint.GetColumns().front()->GetType() != DbColumn::Type::Integer)
        return Error("DbTable '%s' has primary key that has type other then Integer.", constraint.GetTable().GetName().c_str());
    
    if (constraint.GetColumns().front() != constraint.GetTable().FindFirst(DbColumn::Kind::ECInstanceId))
        return Error("DbTable '%s' has primary key that is other then the ECInstanceId.", constraint.GetTable().GetName().c_str());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckPropertyMap(PropertyMap const& propertyMap) const 
    { 
    switch (propertyMap.GetType())
        {
        //====================================================================================
            case PropertyMap::Type::ECClassId:
            {
            ECClassIdPropertyMap const& prop = propertyMap.GetAs<ECClassIdPropertyMap>();
            for (SystemPropertyMap::PerTableIdPropertyMap const* perTableProp : prop.GetDataPropertyMaps())
                {
                if (!Enum::Contains(perTableProp->GetColumn().GetKind(), DbColumn::Kind::ECClassId))
                    return Error("PropertyMap '%s.%s' is mapped to column %s.%s which is not of Kind 'DbColumn::Kind::ECClassId'. ", propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str(), perTableProp->GetColumn().GetTable().GetName().c_str(), perTableProp->GetColumn().GetName().c_str());
                }
            } break;
            //====================================================================================
            case PropertyMap::Type::ECInstanceId:
            {
            ECInstanceIdPropertyMap const& prop = propertyMap.GetAs<ECInstanceIdPropertyMap>();
            for (SystemPropertyMap::PerTableIdPropertyMap const* perTableProp : prop.GetDataPropertyMaps())
                {
                if (!Enum::Contains(perTableProp->GetColumn().GetKind(), DbColumn::Kind::ECInstanceId))
                    return Error("PropertyMap '%s.%s' is mapped to column %s.%s which is not of Kind 'DbColumn::Kind::ECInstanceId'. ", propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str(), perTableProp->GetColumn().GetTable().GetName().c_str(), perTableProp->GetColumn().GetName().c_str());

                if (Enum::Contains(perTableProp->GetColumn().GetKind(), DbColumn::Kind::SharedDataColumn))
                    return Error("PropertyMap '%s.%s' is mapped to column %s.%s is of kind 'DbColumn::Kind::ECInstanceId' and therefore it cannot be of kind SharedDataColumn at the same time. ", propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str(), perTableProp->GetColumn().GetTable().GetName().c_str(), perTableProp->GetColumn().GetName().c_str());

                //if (perTableProp->GetColumn().GetType() != DbColumn::Type::Integer)
                //    return Error("PropertyMap '%s.%s' is mapped to column %s.%s is of kind 'DbColumn::Kind::ECInstanceId' but column is not of type 'Integer'.", propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str(), perTableProp->GetColumn().GetTable().GetName().c_str(), perTableProp->GetColumn().GetName().c_str());
                }
            } break;
            //====================================================================================
            case PropertyMap::Type::ConstraintECInstanceId:
            {

            }break;
            //====================================================================================
            case PropertyMap::Type::ConstraintECClassId:
            {

            }break;
            //====================================================================================
            case PropertyMap::Type::Navigation:
            {
            NavigationPropertyMap const& prop = propertyMap.GetAs<NavigationPropertyMap>();
            if (prop.Size() != 2)
                return Error("NavigationPropertyMap '%s.%s' must have exactly two child property maps.", propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str());

            }break;
            //====================================================================================
            case PropertyMap::Type::Point2d:
            {
            Point2dPropertyMap const& prop = propertyMap.GetAs<Point2dPropertyMap>();
            if (prop.Size() != 2)
                return Error("Point2dPropertyMap '%s.%s' must have exactly two child property maps.", propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str());

            }break;
            //====================================================================================
            case PropertyMap::Type::Point3d:
            {
            Point3dPropertyMap const& prop = propertyMap.GetAs<Point3dPropertyMap>();
            if (prop.Size() != 3)
                return Error("Point3dPropertyMap '%s.%s' must have exactly three child property maps.", propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str());

            }break;
            //====================================================================================
            case PropertyMap::Type::Primitive:
            {

            }break;
            //====================================================================================
            case PropertyMap::Type::PrimitiveArray:
            {

            }break;
            //====================================================================================
            case PropertyMap::Type::StructArray:
            {

            }break;
            //====================================================================================
            case PropertyMap::Type::Struct:
            {
            StructPropertyMap const& prop = propertyMap.GetAs<StructPropertyMap>();
            const size_t expectedPropertyCount = prop.GetProperty().GetAsStructProperty()->GetType().GetPropertyCount();
            if (prop.Size() != expectedPropertyCount)
                return Error("StructPropertyMap '%s.%s' must have exactly '%"  PRIu64 "' child property maps.", propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str(), expectedPropertyCount);

            }break;
            //====================================================================================
            default:
                return Error("PropertyMap '%s.%s' is of unsupported date type.", propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str());
        }

    return SUCCESS; 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckIfAllPropertiesAreMapped(ClassMap const& classmap) const
    {
    int nDataProperties = 0;
    int nSystemProperties = 0;
    for (PropertyMap const* propertyMap : classmap.GetPropertyMaps())
        {
        if (propertyMap->IsData())
            nDataProperties++;
        else if (propertyMap->IsSystem())
            nSystemProperties++;
        }

    ECPropertyIterableCR properties = classmap.GetClass().GetProperties();
    const size_t nPropertyCount = std::distance(properties.begin(), properties.end());
    if (nDataProperties != nPropertyCount)
        return Error("ECClass'%s' properties are not all mapped. Some are left out.", classmap.GetClass().GetFullName());

    if (classmap.GetType()==ClassMap::Type::Class && nSystemProperties != 2)
        return Error("ECClass'%s' is of type 'lassMap::Type::Class' and must have two system property maps.", classmap.GetClass().GetFullName());

    if (classmap.GetType() == ClassMap::Type::RelationshipEndTable && (nSystemProperties != 6 || nDataProperties != 0))
        return Error("ECClass'%s' if of type 'lassMap::Type::RelationshipEndTable' and must have six system property maps and zero data property maps.", classmap.GetClass().GetFullName());

    if (classmap.GetType() == ClassMap::Type::RelationshipLinkTable && nSystemProperties != 6 )
        return Error("ECClass'%s' if of type 'lassMap::Type::RelationshipLinkTable' and must have six system property maps.", classmap.GetClass().GetFullName());
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckPropertyMaps(PropertyMapContainer const& container) const
    {
    std::map<DbColumn const*, PropertyMap const*> singleColumnPropertyMaps;
    SearchPropertyMapVisitor propertyMapVisitor(PropertyMap::Type::All,/* recurseIntoCompoundPropertyMaps =  */true);
    container.AcceptVisitor(propertyMapVisitor);
    for (PropertyMap const* propertyMap : propertyMapVisitor.Results())
        {
        if (propertyMap->GetParent() == nullptr)
            {
            INVOKE_RULE(CheckPropertyMap(*propertyMap));
            }

        if (Enum::Contains(PropertyMap::Type::SingleColumnData, propertyMap->GetType()))
            {
            DbColumn const& column = propertyMap->GetAs<SingleColumnDataPropertyMap>().GetColumn();
            auto itor = singleColumnPropertyMaps.find(&column);
            if (itor == singleColumnPropertyMaps.end())
                singleColumnPropertyMaps[&column] = propertyMap;
            else
                {
                INVOKE_RULE(Error("ECClass '%s', cannot map Column [%s].[%s] to property '%s' as it already mapped to  property'%s'. ",
                                propertyMap->GetClassMap().GetClass().GetFullName(),
                                column.GetTable().GetName().c_str(),
                                column.GetName().c_str(),
                                propertyMap->GetAccessString().c_str(),
                                itor->second->GetAccessString().c_str()));
                }
            }
        else if (Enum::Contains(PropertyMap::Type::System, propertyMap->GetType()))
            {
            if (propertyMap->GetAs<SystemPropertyMap>().GetDataPropertyMaps().empty())
                {
                INVOKE_RULE(Error("ECClass '%s', system property '%s' is not mapped to any column",
                                  propertyMap->GetClassMap().GetClass().GetFullName(),
                                propertyMap->GetAccessString().c_str()));
                }
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckRelationshipClassEndTableMap(RelationshipClassEndTableMap const& relMap) const
    { 
    const auto otherEndTables = RelationshipClassEndTableMap::PartitionView::GetOtherEndTables(relMap);
    if (otherEndTables.size() > 1)
        {
        Utf8String tableStr;
        for (DbTable const* table : otherEndTables)
            {
            if (table != otherEndTables.front())
                tableStr.append(", ");

            tableStr.append("[").append(table->GetName().c_str()).append("]");
            }

        return Error("RelationshipClassEndTableMap for ECClass '%s' map evaluation resulted in more than one primary tables (%s).", relMap.GetClass().GetFullName(), tableStr.c_str());
        }

    if (relMap.GetTables().size() != 1 || relMap.GetTables().front()->GetType() != DbTable::Type::Virtual)
        {
        return Error("RelationshipClassEndTableMap for ECClass '%s' must be map to single virtual table.", relMap.GetClass().GetFullName());
        }

    if (relMap.GetPropertyMaps().Size() != 6)
        {
        return Error("RelationshipClassEndTableMap for ECClass '%s'  must be have exactly six property maps.", relMap.GetClass().GetFullName());
        }

    if (relMap.GetConstraintMap(relMap.GetForeignEnd()).GetECClassIdPropMap() == nullptr
        || relMap.GetConstraintMap(relMap.GetForeignEnd()).GetECInstanceIdPropMap() == nullptr
        || relMap.GetConstraintMap(relMap.GetReferencedEnd()).GetECClassIdPropMap() == nullptr
        || relMap.GetConstraintMap(relMap.GetReferencedEnd()).GetECInstanceIdPropMap() == nullptr)
        {
        return Error("RelationshipClassEndTableMap for ECClass '%s' has invalid constraint maps.", relMap.GetClass().GetFullName());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckRelationshipClassLinkTableMap(RelationshipClassLinkTableMap const& relMap) const
    { 
    const auto sourceTables = RelationshipMappingInfo::GetTablesFromRelationshipEnd(relMap.GetDbMap(), m_schemaImportContext, relMap.GetRelationshipClass().GetSource(), true);
    if (sourceTables.size() > 1)
        {
        Utf8String tableStr;
        for (DbTable const* table : sourceTables)
            tableStr.append("[").append(table->GetName().c_str()).append("], ");

        return Error("RelationshipClassLinkTableMap for ECClass '%s' map evaluation resulted in more than one source tables (%s)", relMap.GetClass().GetFullName(), tableStr.c_str());
        }

    const auto targetTables = RelationshipMappingInfo::GetTablesFromRelationshipEnd(relMap.GetDbMap(), m_schemaImportContext, relMap.GetRelationshipClass().GetTarget(), true);
    if (targetTables.size() > 1)
        {
        Utf8String tableStr;
        for (DbTable const* table : targetTables)
            tableStr.append("[").append(table->GetName().c_str()).append("], ");

        return Error("RelationshipClassLinkTableMap for ECClass '%s'  map evaluation resulted in more than one target tables (%s)", relMap.GetClass().GetFullName(), tableStr.c_str());
        }

    if (relMap.GetConstraintMap(ECRelationshipEnd_Source).GetECClassIdPropMap() == nullptr
        || relMap.GetConstraintMap(ECRelationshipEnd_Source).GetECInstanceIdPropMap() == nullptr
        || relMap.GetConstraintMap(ECRelationshipEnd_Target).GetECClassIdPropMap() == nullptr
        || relMap.GetConstraintMap(ECRelationshipEnd_Target).GetECInstanceIdPropMap() == nullptr)
        {
        return Error("RelationshipClassLinkTableMap for ECClass '%s'  has invalid constraint maps.", relMap.GetClass().GetFullName());
        }

    return SUCCESS; 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::Error(Utf8CP format, ...) const
    {
    Utf8String msg;
    va_list args;
    va_start(args, format);
    msg.VSprintf(format, args);
    va_end(args);
    m_errors.push_back(msg);
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
bool DbMap::Validator::IsError(BentleyStatus st) const
    {
    if (st == SUCCESS)
        return false;

    m_status = st;
    return m_onErrorFail;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::ReportIssues() const
    {
    for (Utf8StringCR message : m_errors)
        {
        m_map.Issues().Report("ECDB_MAP_VALIDATION_ERROR: %s", message.c_str());
        }

    return m_status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::Validator::CheckAndReportIssues(bool onErrorFail, Filter classMapFilter , Filter tableFilter ) const
    {
    m_status = SUCCESS;
    m_errors.clear();
    m_onErrorFail = onErrorFail;

    CheckDbSchema(m_map.GetDbSchema(), tableFilter);
    if (m_onErrorFail && m_status == ERROR)
        return ReportIssues();

    CheckDbMap(m_map, classMapFilter);
    if (m_onErrorFail && m_status == ERROR)
        return ReportIssues();

    return ReportIssues();
    }
END_BENTLEY_SQLITE_EC_NAMESPACE

