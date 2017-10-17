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

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMap const* DbMap::GetClassMap(ECN::ECClassCR ecClass) const
    {
    ClassMapLoadContext ctx;
    ClassMap const* classMap = nullptr;
    if (SUCCESS != TryGetClassMap(classMap, ctx, ecClass))
        {
        BeAssert(false);
        return nullptr;
        }

    if (classMap == nullptr)
        return nullptr;

    return classMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan   06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMap::TryGetClassMap(ClassMap const*& classMap, ClassMapLoadContext& ctx, ECN::ECClassCR ecClass) const
    {
    ClassMapPtr classMapPtr = nullptr;
    if (SUCCESS != TryGetClassMap(classMapPtr, ctx, ecClass))
        return ERROR;

    classMap = classMapPtr.get();
    return SUCCESS;
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
    if (SUCCESS != DbClassMapLoadContext::Load(classMapLoadContext, ctx, GetECDb(), ecClass))
        return ERROR;

    if (!classMapLoadContext.ClassMapExists())
        return SUCCESS; //Class was not yet mapped in a previous import

    MapStrategyExtendedInfo const& mapStrategy = classMapLoadContext.GetMapStrategy();
    ClassMapPtr classMapTmp = nullptr;
    if (mapStrategy.GetStrategy() == MapStrategy::NotMapped)
        classMapTmp = ClassMapFactory::Create<NotMappedClassMap>(m_ecdb, ecClass, mapStrategy);
    else
        {
        ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP();
        if (ecRelationshipClass != nullptr)
            {
            if (MapStrategyExtendedInfo::IsForeignKeyMapping(mapStrategy))
                classMapTmp = ClassMapFactory::Create<RelationshipClassEndTableMap>(m_ecdb, *ecRelationshipClass, mapStrategy);
            else
                classMapTmp = ClassMapFactory::Create<RelationshipClassLinkTableMap>(m_ecdb, *ecRelationshipClass, mapStrategy);
            }
        else
            classMapTmp = ClassMapFactory::Create<ClassMap>(m_ecdb, ecClass, mapStrategy);
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

    PERFLOG_START("ECDb", "Schema import> Validate mappings");

    if (SUCCESS != DbMapValidator(*this, ctx).Validate())
        return ERROR;

    PERFLOG_FINISH("ECDb", "Schema import> Validate mappings");

    ClearCache();
    PERFLOG_FINISH("ECDb", "Schema import> Map schemas");
    return SUCCESS;
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

    //If all baseClasses are mixin then consider the class as root class.
    bool noMixin = false;
    for (ECClassCP baseClass : ecclass.GetBaseClasses())
        {
        if (ECEntityClassCP entityClass = baseClass->GetEntityClassCP())
            {
            if (entityClass->IsMixin())
                continue;
            }

        noMixin = true;
        break;
        }

    if (!noMixin)
        {
        rootClassList.push_back(&ecclass);
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
        m_ecdb.GetImpl().Issues().Report("Synchronizing existing table to which classes are mapped failed.");
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

    if (SUCCESS != DbMappingManager::FkRelationships::FinishMapping(ctx))
        return ERROR;

    PERFLOG_FINISH("ECDb", "Schema import> Map relationships");
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   06/12
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus DbMap::MapClass(SchemaImportContext& ctx, ECClassCR ecClass) const
    {
    ClassMapPtr existingClassMap = nullptr;
    if (SUCCESS != TryGetClassMap(existingClassMap, ctx.GetClassMapLoadContext(), ecClass))
        return ClassMappingStatus::Error;

    if (existingClassMap == nullptr)
        {
        ClassMappingInfo mappingInfo(ctx, ecClass);
        ClassMappingStatus status = mappingInfo.Initialize();
        if (status == ClassMappingStatus::BaseClassesNotMapped || status == ClassMappingStatus::Error)
            return status;

        return MapClass(ctx, mappingInfo);
        }

    if (SUCCESS != existingClassMap->Update(ctx))
        return ClassMappingStatus::Error;

    return MapDerivedClasses(ctx, ecClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle         10/2017
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus DbMap::MapClass(SchemaImportContext& ctx, ClassMappingInfo const& mappingInfo) const
    {
    MapStrategyExtendedInfo const& mapStrategy = mappingInfo.GetMapStrategy();
    ClassMapPtr classMap = nullptr;
    if (mapStrategy.GetStrategy() == MapStrategy::NotMapped)
        classMap = ClassMapFactory::Create<NotMappedClassMap>(m_ecdb, mappingInfo.GetClass(), mapStrategy);
    else
        {
        ECRelationshipClassCP ecRelationshipClass = mappingInfo.GetClass().GetRelationshipClassCP();
        if (ecRelationshipClass != nullptr)
            {
            if (MapStrategyExtendedInfo::IsForeignKeyMapping(mapStrategy))
                classMap = ClassMapFactory::Create<RelationshipClassEndTableMap>(m_ecdb, *ecRelationshipClass, mapStrategy);
            else
                classMap = ClassMapFactory::Create<RelationshipClassLinkTableMap>(m_ecdb, *ecRelationshipClass, mapStrategy);
            }
        else
            classMap = ClassMapFactory::Create<ClassMap>(m_ecdb, mappingInfo.GetClass(), mapStrategy);
        }

    ClassMappingStatus status = AddClassMap(classMap);
    if (status == ClassMappingStatus::Error)
        return status;

    ctx.AddClassMapForSaving(mappingInfo.GetClass().GetId());
    status = classMap->Map(ctx, mappingInfo);
    if (status == ClassMappingStatus::BaseClassesNotMapped || status == ClassMappingStatus::Error)
        return status;

    if (SUCCESS != DbMappingManager::Classes::MapUserDefinedIndexes(ctx, *classMap))
        return ClassMappingStatus::Error;

    return MapDerivedClasses(ctx, mappingInfo.GetClass());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle         10/2017
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus DbMap::MapDerivedClasses(SchemaImportContext& ctx, ECN::ECClassCR baseClass) const
    {
    const bool baseClassIsMixin = baseClass.IsEntityClass() && baseClass.GetEntityClassCP()->IsMixin();
    
    for (ECClassCP derivedClass : baseClass.GetDerivedClasses())
        {
        const bool derivedIsMixin = derivedClass->IsEntityClass() && derivedClass->GetEntityClassCP()->IsMixin();
        //Only map mixin hierarchy but stop if you find a non-mixin class.
        if (baseClassIsMixin && !derivedIsMixin)
            continue;

        if (ClassMappingStatus::Error == MapClass(ctx, *derivedClass))
            return ClassMappingStatus::Error;

        if (ClassMappingStatus::Error == MapDerivedClasses(ctx, *derivedClass))
            return ClassMappingStatus::Error;
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

    for (DbTable const* table : GetDbSchema().Tables().GetTablesInDependencyOrder())
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

    LOG.debugv("Schema Import>CreateOrUpdateRequiredTables> Created %d tables, updated %d tables, and %d tables were up-to-date.", nCreated, nUpdated, nWasUpToDate);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2016
//---------------------------------------------------------------------------------------
BentleyStatus DbMap::CreateOrUpdateIndexesInDb(SchemaImportContext& ctx) const
    {
    if (SUCCESS != m_dbSchema.LoadIndexDefs())
        return ERROR;

    if (BE_SQLITE_OK != m_ecdb.ExecuteSql("DELETE FROM " TABLE_Index))
        return ERROR;

    bmap<Utf8String, DbIndex const*, CompareIUtf8Ascii> comparableIndexDefs;
    bset<Utf8CP, CompareIUtf8Ascii> usedIndexNames;

    for (DbTable const* table : m_dbSchema.Tables())
        {
        for (std::unique_ptr<DbIndex> const& indexPtr : table->GetIndexes())
            {
            DbIndex const& index = *indexPtr;
            if (index.GetColumns().empty())
                {
                BeAssert(false && "Index definition is not valid");
                return ERROR;
                }

            {
            //drop index first if it exists, as we always have to recreate them to make sure the class id filter is up-to-date
            Utf8String dropIndexSql;
            dropIndexSql.Sprintf("DROP INDEX [%s]", index.GetName().c_str());
            m_ecdb.TryExecuteSql(dropIndexSql.c_str());
            }

            if (!index.IsAutoGenerated() && index.HasClassId())
                {
                ECClassCP ecClass = m_ecdb.Schemas().GetClass(index.GetClassId());
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
                if (storageDesc.HasMultipleNonVirtualHorizontalPartitions())
                    {
                    Issues().Report("Failed to map ECClass '%s'. The index '%s' defined on it spans multiple tables which is not supported. Consider applying the 'TablePerHierarchy' strategy to the ECClass.",
                                    ecClass->GetFullName(), index.GetName().c_str());
                    return ERROR;
                    }
                }

            if (usedIndexNames.find(index.GetName().c_str()) != usedIndexNames.end())
                {
                Issues().Report("Failed to create index %s on table %s. An index with the same name already exists.", index.GetName().c_str(), index.GetTable().GetName().c_str());
                return ERROR;
                }
            else
                usedIndexNames.insert(index.GetName().c_str());

            //indexes on virtual tables are ignored
            if (index.GetTable().GetType() != DbTable::Type::Virtual)
                {
                Utf8String ddl, comparableIndexDef;
                if (SUCCESS != DbSchemaPersistenceManager::BuildCreateIndexDdl(ddl, comparableIndexDef, m_ecdb, index))
                    return ERROR;

                auto it = comparableIndexDefs.find(comparableIndexDef);
                if (it != comparableIndexDefs.end())
                    {
                    Utf8CP errorMessage = "Index '%s'%s on table '%s' has the same definition as the already existing index '%s'%s. ECDb does not create this index.";

                    Utf8String provenanceStr;
                    if (index.HasClassId())
                        {
                        ECClassCP provenanceClass = m_ecdb.Schemas().GetClass(index.GetClassId());
                        if (provenanceClass == nullptr)
                            {
                            BeAssert(false);
                            return ERROR;
                            }
                        provenanceStr.Sprintf(" [Created for ECClass %s]", provenanceClass->GetFullName());
                        }

                    DbIndex const* existingIndex = it->second;
                    Utf8String existingIndexProvenanceStr;
                    if (existingIndex->HasClassId())
                        {
                        ECClassCP provenanceClass = m_ecdb.Schemas().GetClass(existingIndex->GetClassId());
                        if (provenanceClass == nullptr)
                            {
                            BeAssert(false);
                            return ERROR;
                            }
                        existingIndexProvenanceStr.Sprintf(" [Created for ECClass %s]", provenanceClass->GetFullName());
                        }

                    if (!index.IsAutoGenerated())
                        LOG.warningv(errorMessage, index.GetName().c_str(), provenanceStr.c_str(), index.GetTable().GetName().c_str(),
                                     existingIndex->GetName().c_str(), existingIndexProvenanceStr.c_str());
                    else
                        {
                        if (LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
                            LOG.debugv(errorMessage,
                                       index.GetName().c_str(), provenanceStr.c_str(), index.GetTable().GetName().c_str(),
                                       existingIndex->GetName().c_str(), existingIndexProvenanceStr.c_str());
                        }

                    continue;
                    }

                comparableIndexDefs[comparableIndexDef] = &index;

                if (SUCCESS != DbSchemaPersistenceManager::CreateIndex(m_ecdb, index, ddl))
                    return ERROR;
                }

            //populates the ec_Index table (even for indexes on virtual tables, as they might be necessary
            //if further schema imports introduce subclasses of abstract classes (which map to virtual tables))
            if (SUCCESS != m_dbSchema.PersistIndexDef(index))
                return ERROR;
            }
        }

    return SUCCESS;
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

    for (Utf8StringCR tableName : tablesToDrop)
        {
        GetDbSchema().Tables().Remove(tableName);
        }

    stmt.Finalize();

    if (tablesToDrop.empty())
        return SUCCESS;

    if (!m_ecdb.GetECDbSettings().AllowChangesetMergingIncompatibleSchemaImport())
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

        m_ecdb.GetImpl().Issues().Report(
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
// @return Number of tables at the specified end of the relationship. 
// @bsimethod                                 Ramanujam.Raman                05/2012
//---------------------------------------------------------------------------------------
size_t DbMap::GetRelationshipConstraintTableCount(SchemaImportContext& ctx, ECRelationshipConstraintCR constraint) const
    {
    std::set<ClassMap const*> classMaps = GetRelationshipConstraintClassMaps(ctx, constraint);
    const bool abstractEndPoint = constraint.GetConstraintClasses().size() == 1 && constraint.GetConstraintClasses().front()->GetClassModifier() == ECClassModifier::Abstract;
    
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
std::set<DbTable const*> DbMap::GetRelationshipConstraintPrimaryTables(SchemaImportContext& ctx, ECRelationshipConstraintCR constraint) const
    {
    //WIP_CLEANUP This looks over-complicated. Doing 3 loops to get the final result. E.g. why can't virtual tables be ignored right away?
    std::set<ClassMap const*> classMaps = GetRelationshipConstraintClassMaps(ctx, constraint);

    std::map<DbTable const*, std::set<DbTable const*>> joinedTablesPerPrimaryTable;
    std::set<DbTable const*> tables;
    for (ClassMap const* classMap : classMaps)
        {
        std::vector<DbTable const*> nonOverflowClassMapTables;
        for (DbTable const* table : classMap->GetTables())
            {
            if (table->GetType() != DbTable::Type::Overflow)
                nonOverflowClassMapTables.push_back(table);
            }

        if (nonOverflowClassMapTables.size() == 1)
            {
            tables.insert(nonOverflowClassMapTables[0]);
            continue;
            }

        for (DbTable const* table : nonOverflowClassMapTables)
            {
            if (table->GetType() == DbTable::Type::Joined)
                {
                DbTable::LinkNode const* primaryTable = table->GetLinkNode().GetParent();
                BeAssert(primaryTable != nullptr);

                joinedTablesPerPrimaryTable[&primaryTable->GetTable()].insert(table);
                tables.insert(table);
                }
            }
        }

    for (std::pair<DbTable const*, std::set<DbTable const*>> const& pair : joinedTablesPerPrimaryTable)
        {
        DbTable const* primaryTable = pair.first;
        for (DbTable::LinkNode const* nextTableNode : primaryTable->GetLinkNode().GetChildren())
            tables.erase(&nextTableNode->GetTable());

        tables.insert(primaryTable);
        continue;
        }

    std::set<DbTable const*> finalSetOfTables;
    for (DbTable const* table : tables)
        {
        if (table->GetType() != DbTable::Type::Virtual)
            finalSetOfTables.insert(table);
        }

    return finalSetOfTables;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2015
//+---------------+---------------+---------------+---------------+---------------+------
std::set<ClassMap const*> DbMap::GetRelationshipConstraintClassMaps(SchemaImportContext& ctx, ECRelationshipConstraintCR constraint) const
    {
    std::set<ClassMap const*> classMaps;
    for (ECClassCP ecClass : constraint.GetConstraintClasses())
        {
        ClassMap const* classMap = nullptr;
        if (SUCCESS != TryGetClassMap(classMap, ctx.GetClassMapLoadContext(), *ecClass))
            {
            BeAssert(false);
            classMaps.clear();
            return classMaps;
            }

        const bool recursive = classMap->GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy && constraint.GetIsPolymorphic();
        if (SUCCESS != GetRelationshipConstraintClassMaps(ctx, classMaps, *ecClass, recursive))
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
BentleyStatus DbMap::GetRelationshipConstraintClassMaps(SchemaImportContext& ctx, std::set<ClassMap const*>& classMaps, ECClassCR ecClass, bool recursive) const
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
        if (SUCCESS != GetRelationshipConstraintClassMaps(ctx, classMaps, *subclass, recursive))
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

        //if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
        //    classMap.GetAs<RelationshipClassEndTableMap>().ResetPartitionCache();
        }

    if (SUCCESS != DbSchemaPersistenceManager::RepopulateClassHasTableCacheTable(GetECDb()))
        return ERROR;

    m_lightweightCache.Reset();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2012
//+---------------+---------------+---------------+---------------+---------------+------
void DbMap::ClearCache() const
    {
    m_classMapDictionary.clear();
    m_dbSchema.ClearCache();
    m_lightweightCache.Reset();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE