/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbMap.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <vector>
#include "SqlNames.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbMap::ECDbMap(ECDbCR ecdb) : m_ecdb(ecdb), m_dbSchema(ecdb), m_schemaImportContext(nullptr), m_lightweightCache(ecdb) {}

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   05/2015
//---------------+---------------+---------------+---------------+---------------+--------
SchemaImportContext* ECDbMap::GetSchemaImportContext() const
    {
    if (AssertIfIsNotImportingSchema())
        return nullptr;

    return m_schemaImportContext;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      02/2015
//---------------+---------------+---------------+---------------+---------------+--------
bool ECDbMap::IsImportingSchema() const
    {
    return m_schemaImportContext != nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2016
//---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECDbMap::PurgeOrphanTables() const
    {
    //skip ExistingTable and NotMapped
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, "SELECT t.Name, t.IsVirtual FROM ec_Table t "
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

    std::vector<Utf8String> nonVirtualTables;
    std::vector<Utf8String> virtualTables;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        if (stmt.GetValueBoolean(1))
            virtualTables.push_back(stmt.GetValueText(0));
        else
            nonVirtualTables.push_back(stmt.GetValueText(0));
        }

    if (nonVirtualTables.empty() && virtualTables.empty())
        return SUCCESS;

    stmt.Finalize();
    if (stmt.Prepare(m_ecdb, "DELETE FROM ec_Table WHERE Name = ?") != BE_SQLITE_OK)
        {
        BeAssert(false && "ECDb profile changed");
        return ERROR;
        }

    for (Utf8StringCR name : virtualTables)
        {
        stmt.Reset();
        stmt.ClearBindings();
        stmt.BindText(1, name, Statement::MakeCopy::No);
        if (stmt.Step() != BE_SQLITE_DONE)
            {
            BeAssert(false && "constraint violation");
            return ERROR;
            }
        }

    if (nonVirtualTables.empty())
        return SUCCESS;

    if (!m_ecdb.GetECDbImplR().GetSettings().AllowChangesetMergingIncompatibleECSchemaImport())
        {
        Utf8String tableNames;
        bool isFirstTable = true;
        for (Utf8StringCR tableName : nonVirtualTables)
            {
            if (!isFirstTable)
                tableNames.append(",");

            tableNames.append(tableName);
            isFirstTable = false;
            }

        m_ecdb.GetECDbImplR().GetIssueReporter().Report(
            "Failed to import ECSchemas: it would change the database schema in a changeset-merging incompatible way. ECDb would have to delete these tables: %s", tableNames.c_str());
        return ERROR;
        }

    for (Utf8StringCR name : nonVirtualTables)
        {
        if (m_ecdb.DropTable(name.c_str()) != BE_SQLITE_OK)
            {
            BeAssert(false && "failed to drop a table");
            return ERROR;
            }

        stmt.Reset();
        stmt.ClearBindings();
        stmt.BindText(1, name, Statement::MakeCopy::No);
        if (stmt.Step() != BE_SQLITE_DONE)
            {
            BeAssert(false && "constraint violation");
            return ERROR;
            }
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      02/2015
//---------------+---------------+---------------+---------------+---------------+--------
bool ECDbMap::AssertIfIsNotImportingSchema() const
    {
    BeAssert(IsImportingSchema() && "ECDb is in currently in schema import mode. Which was not expected");
    return !IsImportingSchema();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    04/2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMap::MapSchemas(SchemaImportContext& ctx) const
    {
    if (m_schemaImportContext != nullptr)
        {
        BeAssert(false && "MapSchemas is expected to be called if no other schema import is running.");
        return ERROR;
        }

    if (ctx.GetECSchemaCompareContext().HasNoSchemasToImport())
        return SUCCESS;

    m_schemaImportContext = &ctx;

    if (SUCCESS != DoMapSchemas())
        {
        m_schemaImportContext = nullptr;
        return ERROR;
        }

    if (SUCCESS != SaveDbSchema())
        {
        ClearCache();
        m_schemaImportContext = nullptr;
        return ERROR;
        }

    if (SUCCESS != CreateOrUpdateRequiredTables())
        {
        ClearCache();
        m_schemaImportContext = nullptr;
        return ERROR;
        }

    if (SUCCESS != CreateOrUpdateIndexesInDb())
        {
        ClearCache();
        m_schemaImportContext = nullptr;
        return ERROR;
        }
    
    if (SUCCESS != PurgeOrphanTables())
        {
        BeAssert(false);
        ClearCache();
        m_schemaImportContext = nullptr;
        return ERROR;
        }

    ClearCache();
    m_schemaImportContext = nullptr;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan         03/2016
//---------------------------------------------------------------------------------------
std::vector<ECClassCP> ECDbMap::GetBaseClassesNotAlreadyMapped(ECClassCR ecclass) const
    {
    //!This does not work due to navigation properties
    std::vector<ECClassCP> baseClasses;
    for (ECClassCP baseClass : ecclass.GetBaseClasses())
        {
        ClassMapCP baseClassMap = GetClassMap(*baseClass);
        if (baseClassMap == nullptr)
            {
            baseClasses.push_back(baseClass);
            }
        }

    return baseClasses;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan         03/2016
//---------------------------------------------------------------------------------------
void ECDbMap::GatherRootClasses(ECClassCR ecclass, std::set<ECClassCP>& doneList, std::set<ECClassCP>& rootClassSet, std::vector<ECClassCP>& rootClassList, std::vector<ECRelationshipClassCP>& rootRelationshipList, std::vector<ECN::ECEntityClassCP>& rootMixIns)
    {
    if (doneList.find(&ecclass) != doneList.end())
        return;

    doneList.insert(&ecclass);
    if (!ecclass.HasBaseClasses())
        {
        ECEntityClassCP entityClass = ecclass.IsEntityClass() ? static_cast<ECEntityClassCP>(&ecclass) : nullptr;
        if (rootClassSet.find(&ecclass) == rootClassSet.end())
            {
            rootClassSet.insert(&ecclass);
            if (auto relationship = ecclass.GetRelationshipClassCP())
                rootRelationshipList.push_back(relationship);
            else
                {
                if (entityClass && entityClass->IsMixin())
                    rootMixIns.push_back(entityClass);
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

        GatherRootClasses(*baseClass, doneList, rootClassSet, rootClassList, rootRelationshipList, rootMixIns);
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan         09/2012
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMap::DoMapSchemas() const
    {
    if (AssertIfIsNotImportingSchema())
        return ERROR;

    // Identify root classes/relationship-classes
    ECSchemaCompareContext& ctx = GetSchemaImportContext()->GetECSchemaCompareContext();

    std::set<ECClassCP> doneList;
    std::set<ECClassCP> rootClassSet;
    std::vector<ECClassCP> rootClassList;
    std::vector<ECN::ECEntityClassCP> rootMixIns;
    std::vector<ECRelationshipClassCP> rootRelationshipList;

    for (ECSchemaCP schema : ctx.GetImportingSchemas())
        {
        if (schema->IsSupplementalSchema())
            continue; // Don't map any supplemental schemas

        for (ECClassCP ecClass : schema->GetClasses())
            {
            if (doneList.find(ecClass) != doneList.end())
                continue;

            GatherRootClasses(*ecClass, doneList, rootClassSet, rootClassList, rootRelationshipList, rootMixIns);
            }
        }
    
    if (GetDbSchemaR().SynchronizeExistingTables() != SUCCESS)
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report("Synchronizing existing table to which classes are mapped failed.");
        return ERROR;
        }

    // Map mixin hiearchy before everything else. It does not map primary hiearchy and all classes map to virtual tables.
    for (ECEntityClassCP mixIn : rootMixIns)
        {
        if (ClassMappingStatus::Error == MapClass(*mixIn))
            return ERROR;
        }

    // Starting with the root, recursively map the entire class hierarchy. 
    for (ECClassCP rootClass : rootClassList)
        {
        if (ClassMappingStatus::Error == MapClass(*rootClass))
            return ERROR;
        }

    //need to add classid cols where necessary for classes before processing relationships
    if (SUCCESS != FinishTableDefinitions(true))
        return ERROR;

    for (ECRelationshipClassCP rootRelationshipClass : rootRelationshipList)
        {
        if (ClassMappingStatus::Error == MapClass(*rootRelationshipClass))
            return ERROR;
        }

    //NavigationPropertyMaps can only be finished after all relationships have been mapped
    if (SUCCESS != GetSchemaImportContext()->GetClassMapLoadContext().Postprocess(*this))
        return ERROR;

    for (auto& kvpair : GetSchemaImportContext()->GetClassMappingInfoCache())
        {
        if (SUCCESS != kvpair.first->CreateUserProvidedIndexes(*GetSchemaImportContext(), kvpair.second->GetIndexInfos()))
            return ERROR;
        }

    //now create class id cols for the relationship classes, and ensure shared col min count
    return FinishTableDefinitions();
    }

//---------------------------------------------------------------------------------------
//* @bsimethod                                 Affan.Khan                           07 / 2012
//---------------------------------------------------------------------------------------
 BentleyStatus ECDbMap::TryLoadClassMap(ClassMapPtr& classMap, ClassMapLoadContext& ctx, ECN::ECClassCR ecClass) const
    {
    classMap = nullptr;

    DbClassMapLoadContext classMapLoadContext;
    if (DbClassMapLoadContext::Load(classMapLoadContext, ctx, GetECDb(), ecClass) != SUCCESS)
        {
        //Failed to find classmap
        return SUCCESS;
        }

    bool setIsDirty = false;
    MapStrategyExtendedInfo const& mapStrategy = classMapLoadContext.GetMapStrategy();
    ClassMapPtr classMapTmp = nullptr;
    if (mapStrategy.GetStrategy() == MapStrategy::NotMapped)
        classMapTmp = NotMappedClassMap::Create(m_ecdb, ecClass, mapStrategy, setIsDirty);
    else
        {
        ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP();
        if (ecRelationshipClass != nullptr)
            {
            if (MapStrategyExtendedInfo::IsForeignKeyMapping(mapStrategy))
                classMapTmp = RelationshipClassEndTableMap::Create(m_ecdb, *ecRelationshipClass, mapStrategy, setIsDirty);
            else
                classMapTmp = RelationshipClassLinkTableMap::Create(m_ecdb, *ecRelationshipClass, mapStrategy, setIsDirty);
            }
        else
            classMapTmp = ClassMap::Create(m_ecdb, ecClass, mapStrategy, setIsDirty);
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

/*---------------------------------------------------------------------------------------
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
 ClassMappingStatus ECDbMap::MapClass(ECClassCR ecClass) const
     {
     if (AssertIfIsNotImportingSchema())
         return ClassMappingStatus::Error;

     ClassMapPtr existingClassMap = nullptr;
     if (SUCCESS != TryGetClassMap(existingClassMap, m_schemaImportContext->GetClassMapLoadContext(), ecClass))
         return ClassMappingStatus::Error;

     if (existingClassMap == nullptr)
         {
         ClassMappingStatus status = ClassMappingStatus::Success;
         std::unique_ptr<ClassMappingInfo> classMapInfo = ClassMappingInfoFactory::Create(status, m_ecdb, ecClass);
         if ((status == ClassMappingStatus::BaseClassesNotMapped || status == ClassMappingStatus::Error))
             return status;

         BeAssert(classMapInfo != nullptr);

         MapStrategyExtendedInfo const& mapStrategy = classMapInfo->GetMapStrategy();

         ClassMapPtr classMap = nullptr;
         if (mapStrategy.GetStrategy() == MapStrategy::NotMapped)
             classMap = NotMappedClassMap::Create(m_ecdb, ecClass, mapStrategy, true);
         else
             {
             auto ecRelationshipClass = ecClass.GetRelationshipClassCP();
             if (ecRelationshipClass != nullptr)
                 {
                 if (MapStrategyExtendedInfo::IsForeignKeyMapping(mapStrategy))
                     classMap = RelationshipClassEndTableMap::Create(m_ecdb, *ecRelationshipClass, mapStrategy, true);
                 else
                     classMap = RelationshipClassLinkTableMap::Create(m_ecdb, *ecRelationshipClass, mapStrategy, true);
                 }
             else
                 classMap = ClassMap::Create(m_ecdb, ecClass, mapStrategy, true);
             }

         status = AddClassMap(classMap);
         if (status == ClassMappingStatus::Error)
             return status;

         status = classMap->Map(*GetSchemaImportContext(), *classMapInfo);
         GetSchemaImportContext()->CacheClassMapInfo(*classMap, classMapInfo);

         //error
         if (status == ClassMappingStatus::BaseClassesNotMapped || status == ClassMappingStatus::Error)
             return status;

         }

     bool isCurrentIsMixIn = false;
     if (ecClass.IsEntityClass())
         {
         ECEntityClassCP entityClass = static_cast<ECEntityClassCP>(&ecClass);
         if (entityClass->IsMixin())
             {
             isCurrentIsMixIn = true;
             }
         }

     for (ECClassP childClass : ecClass.GetDerivedClasses())
         {
         bool isChildIsMixIn = false;
         if (isCurrentIsMixIn)
             {
             if (childClass->IsEntityClass())
                 {
                 ECEntityClassCP entityClass = static_cast<ECEntityClassCP>(childClass);
                 if (entityClass->IsMixin())
                     {
                     isChildIsMixIn = true;
                     }
                 }
             }

         //Only map mixIn hiearchy but stop if you find a none-mixin class.
         if (isCurrentIsMixIn && !isChildIsMixIn)
             continue;

         ClassMappingStatus status = MapClass(*childClass);
         if (status == ClassMappingStatus::Error)
             return status;
         }

     return ClassMappingStatus::Success;
     }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ClassMappingStatus ECDbMap::AddClassMap(ClassMapPtr& classMap) const
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
// @bsimethod                                                    Krischan.Eberle   01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMap const* ECDbMap::GetClassMap(ECN::ECClassCR ecClass) const
    {
    if (m_schemaImportContext == nullptr)
        {
        ClassMapLoadContext ctx;
        ClassMapPtr classMap = nullptr;
        if (SUCCESS != TryGetClassMap(classMap, ctx, ecClass))
            {
            BeAssert(false && "Error during TryGetClassMap");
            return nullptr;
            }

        if (classMap == nullptr)
            return nullptr;

        if (SUCCESS != ctx.Postprocess(*this))
            return nullptr;

        return classMap.get();
        }

    ClassMapPtr classMap = nullptr;
    if (SUCCESS != TryGetClassMap(classMap, m_schemaImportContext->GetClassMapLoadContext(), ecClass))
        return nullptr;

    return classMap.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   02/2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMap::TryGetClassMap(ClassMapPtr& classMap, ClassMapLoadContext& ctx, ECN::ECClassCR ecClass) const
    {
    //we must use this method here and cannot just see whether ecClass has already an id
    //because the ecClass object can come from an ECSchema deserialized from disk, hence
    //not having the id set, and already imported in the ECSchema. In that case
    //ECDb does not set the ids on the ECClass objects
    if (!m_ecdb.Schemas().GetReader().GetECClassId(ecClass).IsValid())
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
ClassMapPtr ECDbMap::DoGetClassMap(ECClassCR ecClass) const
    {
    auto it = m_classMapDictionary.find(ecClass.GetId());
    if (m_classMapDictionary.end() == it)
        return nullptr;
    else
        return it->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbMap::ClassMapsByTable ECDbMap::GetClassMapsByTable() const
    {
    ClassMapsByTable map;
    for (auto const& entry : m_classMapDictionary)
        {
        if (entry.second->GetType() == ClassMap::Type::RelationshipEndTable ||
            entry.second->GetType() == ClassMap::Type::NotMapped)
            continue;

        DbTable* primaryTable = &entry.second->GetPrimaryTable();
        DbTable* joinedTable = &entry.second->GetJoinedTable();
        map[primaryTable].insert(entry.second.get());
        if (primaryTable != joinedTable)
            map[joinedTable].insert(entry.second.get());
        }

    return map;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      12/2011
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMap::CreateOrUpdateRequiredTables() const
    {
    if (AssertIfIsNotImportingSchema())
        return ERROR;

    m_ecdb.GetStatementCache().Empty();
    StopWatch timer(true);

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

    timer.Stop();
    LOG.debugv("Created %d tables, updated %d tables, and %d tables were up-to-date (%.4f seconds).", nCreated, nUpdated, nWasUpToDate, timer.GetElapsedSeconds());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2016
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMap::CreateOrUpdateIndexesInDb() const
    {
    if (AssertIfIsNotImportingSchema())
        return ERROR;

    std::vector<DbIndex const*> indexes;
    for (std::unique_ptr<DbIndex> const& indexPtr : m_dbSchema.GetIndexes())
        {
        indexes.push_back(indexPtr.get());
        }

    IndexMappingInfoCache indexInfoCache(m_ecdb, *m_schemaImportContext);
    for (DbIndex const* index : indexes)
        {
        const ECClassId classId = index->GetClassId();
        if (!classId.IsValid())
            continue;

        ECClassCP ecClass = m_ecdb.Schemas().GetECClass(classId);
        if (ecClass == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        ClassMap const* classMap = GetClassMap(*ecClass);
        if (classMap == nullptr)
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
            for (ECClassId derivedClassId : horizPartition.GetClassIds())
                {
                ECClassCP derivedClass = m_ecdb.Schemas().GetECClass(derivedClassId);
                if (derivedClass == nullptr)
                    {
                    BeAssert(false);
                    return ERROR;
                    }

                ClassMap const* derivedClassMap = GetClassMap(*derivedClass);
                if (derivedClassMap == nullptr)
                    {
                    BeAssert(false);
                    return ERROR;
                    }

                DbTable const& joinedOrSingleTable = derivedClassMap->GetJoinedTable();
                if (alreadyProcessedTables.find(&joinedOrSingleTable) != alreadyProcessedTables.end())
                    continue;

                std::vector<IndexMappingInfoPtr> indexMappingInfos;
                for (IndexMappingInfoPtr const& indexMappingInfo : *baseClassIndexInfos)
                    {
                    Utf8String indexName;
                    if (!Utf8String::IsNullOrEmpty(indexMappingInfo->GetName()))
                        indexName.append(indexMappingInfo->GetName()).append("_").append(joinedOrSingleTable.GetName());

                    indexMappingInfos.push_back(IndexMappingInfo::Clone(indexName.c_str(), *indexMappingInfo));
                    }

                if (SUCCESS != derivedClassMap->CreateUserProvidedIndexes(*m_schemaImportContext, indexMappingInfos))
                    return ERROR;

                alreadyProcessedTables.insert(&joinedOrSingleTable);
                }
            }
        }

    return m_dbSchema.CreateOrUpdateIndexes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbMap::FinishTableDefinitions(bool onlyCreateClassIdColumns) const
    {
    AssertIfIsNotImportingSchema();
    const ClassMapsByTable classMapsByTable = GetClassMapsByTable();
    for (bpair<DbTable*, bset<ClassMap*>> const& kvPair : classMapsByTable)
        {
        DbTable* table = kvPair.first;
        bset<ClassMap*> const& classMaps = kvPair.second;
        bool canEdit = table->GetEditHandleR().CanEdit();
        if (!canEdit) table->GetEditHandleR().BeginEdit();
        if (UpdateECClassIdColumnIfRequired(*table, classMaps) != SUCCESS)
            return ERROR;

        if (!canEdit) table->GetEditHandleR().EndEdit();
        if (onlyCreateClassIdColumns)
            continue;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMap::UpdateECClassIdColumnIfRequired(DbTable& table, bset<ClassMap*> const& classMaps) const
    {
    if (table.GetPersistenceType() == PersistenceType::Virtual || table.GetType() == DbTable::Type::Existing ||
        table.GetECClassIdColumn().GetPersistenceType() != PersistenceType::Virtual)
        return SUCCESS;

    ClassMap const* firstClassMap = *classMaps.begin();
    bool makeNonVirtual = false;
    if (classMaps.size() == 1)
        makeNonVirtual = firstClassMap->GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy;
    else
        makeNonVirtual = classMaps.size() > 1;

    if (!makeNonVirtual)
        return SUCCESS;

    if (const_cast<DbColumn*> (&table.GetECClassIdColumn())->MakeNonVirtual() != SUCCESS)
        {
        BeAssert(false && "Changing persistence type from virtual to persisted failed");
        return ERROR;
        }

    Utf8String indexName("ix_");
    indexName.append(table.GetName()).append("_ecclassid");
    return GetDbSchemaR().CreateIndex(table, indexName.c_str(), false, {&table.GetECClassIdColumn()},
                                      false, true, ECClassId()) != nullptr ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<ECClassCP> ECDbMap::GetFlattenListOfClassesFromRelationshipEnd(ECRelationshipConstraintCR relationshipEnd) const
    {
    std::vector<ECClassCP> constraintClasses;
    LightweightCache::RelationshipEnd endOfInterest = &relationshipEnd.GetRelationshipClass().GetSource() == &relationshipEnd ? LightweightCache::RelationshipEnd::Source : LightweightCache::RelationshipEnd::Target;
    for (auto const& constraintKey : GetLightweightCache().GetConstraintClassesForRelationshipClass(relationshipEnd.GetRelationshipClass().GetId()))
        {
        if (Enum::Contains(endOfInterest, constraintKey.second))
            {
            ECClassCP constrantClass = GetECDb().Schemas().GetECClass(constraintKey.first);
            if (constrantClass == nullptr)
                {
                BeAssert(false && "Failed to read ECClass");
                constraintClasses.clear();
                return constraintClasses;
                }

            constraintClasses.push_back(constrantClass);
            }
        }

    return constraintClasses;
    }

/*---------------------------------------------------------------------------------**//**
* Gets the count of tables at the specified end of a relationship class.
* @param  relationshpEnd [in] Constraint at the end of the relationship
* @return Number of tables at the specified end of the relationship. Returns
*         UINT_MAX if the end is AnyClass.
* @bsimethod                                 Ramanujam.Raman                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ECDbMap::GetTableCountOnRelationshipEnd(ECRelationshipConstraintCR relationshipEnd) const
    {
    bool hasAnyClass = false;
    std::set<ClassMap const*> classMaps = GetClassMapsFromRelationshipEnd(relationshipEnd, &hasAnyClass);

    if (hasAnyClass)
        return SIZE_MAX;

    std::map<PersistenceType, std::set<DbTable const*>> tables;
    bool abstractEndPoint = relationshipEnd.GetConstraintClasses().size() == 1 && relationshipEnd.GetConstraintClasses().front()->GetClassModifier() == ECClassModifier::Abstract;
    for (ClassMap const* classMap : classMaps)
        {
        if (abstractEndPoint)
            tables[classMap->GetPrimaryTable().GetPersistenceType()].insert(&classMap->GetJoinedTable());
        else
            tables[classMap->GetPrimaryTable().GetPersistenceType()].insert(&classMap->GetPrimaryTable());
        }

    if (tables[PersistenceType::Physical].size() > 0)
        return tables[PersistenceType::Physical].size();

    if (tables[PersistenceType::Virtual].size() > 0)
        return 1;

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2015
//+---------------+---------------+---------------+---------------+---------------+------
std::set<ClassMap const*> ECDbMap::GetClassMapsFromRelationshipEnd(ECRelationshipConstraintCR constraint, bool* hasAnyClass) const
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

        ClassMap const* classMap = GetClassMap(*ecClass);
        if (classMap == nullptr)
            {
            BeAssert(false);
            classMaps.clear();
            return classMaps;
            }

        const bool recursive = classMap->GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy && constraint.GetIsPolymorphic();
        if (SUCCESS != GetClassMapsFromRelationshipEnd(classMaps, *ecClass, recursive))
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
BentleyStatus ECDbMap::GetClassMapsFromRelationshipEnd(std::set<ClassMap const*>& classMaps, ECClassCR ecClass, bool recursive) const
    {
    ClassMap const* classMap = GetClassMap(ecClass);
    if (classMap == nullptr)
        {
        BeAssert(classMap != nullptr && "ClassMap should not be null");
        return ERROR;
        }

    if (classMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        return SUCCESS;

    classMaps.insert(classMap);

    if (!recursive)
        return SUCCESS;

    for (ECClassCP subclass : m_ecdb.Schemas().GetDerivedECClasses(ecClass))
        {
        if (SUCCESS != GetClassMapsFromRelationshipEnd(classMaps, *subclass, recursive))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMap::SaveDbSchema() const
    {
    if (m_schemaImportContext == nullptr)
        {
        BeAssert(false && "ECDbMap::SaveDbSchema must only be called during schema import");
        return ERROR;
        }

    StopWatch stopWatch(true);
    if (m_dbSchema.SaveOrUpdateTables() != SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    DbMapSaveContext ctx(GetECDb());
    for (bpair<ECClassId, ClassMapPtr> const& kvPair : m_classMapDictionary)
        {
        ClassMap& classMap = *kvPair.second;
        if (SUCCESS != classMap.Save(ctx))
            {
            Issues().Report("Failed to save mapping for ECClass %s: %s", classMap.GetClass().GetFullName(), m_ecdb.GetLastError().c_str());
            return ERROR;
            }
        }

    if (SUCCESS != DbSchemaPersistenceManager::RepopulateClassHasTableCacheTable(GetECDb()))
        return ERROR;

    m_lightweightCache.Reset();
    stopWatch.Stop();

    LOG.debugv("Saving EC-DB mapping took %.4lf msecs.", stopWatch.GetElapsedSeconds() * 1000.0);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2012
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMap::ClearCache() const
    {
    m_classMapDictionary.clear();
    m_dbSchema.Reset();
    m_lightweightCache.Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                      06/2016
//+---------------+---------------+---------------+---------------+---------------+------
IssueReporter const& ECDbMap::Issues() const { return m_ecdb.GetECDbImplR().GetIssueReporter(); }

END_BENTLEY_SQLITE_EC_NAMESPACE

