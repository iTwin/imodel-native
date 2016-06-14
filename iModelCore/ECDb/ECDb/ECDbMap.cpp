/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbMap.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <vector>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbMap::ECDbMap(ECDbCR ecdb) : m_ecdb(ecdb), m_dbSchema(ecdb), m_schemaImportContext(nullptr), m_lightweightCache(*this)
    {}

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
BentleyStatus ECDbMap::PurgeOrphanColumns() const
    {
    Statement stmt;
    if (stmt.Prepare(m_ecdb,
                     SqlPrintfString(
                         "SELECT ec_Column.Id, ec_Column.IsVirtual, ec_Column.Name, ec_Table.Name"
                         "   FROM ec_Column"
                         "        INNER JOIN ec_Table ON ec_Table.[Id] = ec_Column.TableId"
                         "   WHERE ec_Column.ColumnKind & %" PRId32 " = 0 AND" //Skip SharedColumns
                         "         ec_Column.ColumnKind & %" PRId32 " = 0 AND" //Skip ECClassId
                         "         ec_Table.[Type] != %" PRId32 " AND"         //Skip Existing Tables
                         "         ec_Column.Id NOT IN ("                       //Skip Column that are Mapped
                         "          SELECT ec_Column.Id"
                         "                 FROM ec_PropertyMap"
                         "                      INNER JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.[PropertyPathId]"
                         "                      INNER JOIN ec_Property ON ec_PropertyPath.RootPropertyId = ec_Property.Id"
                         "                      INNER JOIN ec_Column ON ec_PropertyMap.[ColumnId] = ec_Column.Id)",
                         Enum::ToInt(DbColumn::Kind::SharedDataColumn),
                         Enum::ToInt(DbColumn::Kind::ECClassId),
                         Enum::ToInt(DbTable::Type::Existing)).GetUtf8CP()) != BE_SQLITE_OK)
        {
        BeAssert(false && "system sql schema changed");
        return ERROR;
        }

    BeAssert(false && "WIP_NOT_IMPLEMENTED");
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        //!!!WIP_AFFAN
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2016
//---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECDbMap::PurgeOrphanTables() const
    {
    Statement stmt;
    if (stmt.Prepare(m_ecdb,
                     SqlPrintfString(
                     "SELECT ec_Table.Name, ec_Table.IsVirtual FROM ec_Table"
                     "    WHERE ec_Table.[Type] != %" PRId32 " AND" //Skip Existing tables
                     "    ec_Table.Id NOT IN ("                     //Skip Tables that is already Mapped
                     "        SELECT DISTINCT ec_Table.Id"
                     "        FROM ec_PropertyMap"
                     "          INNER JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.[PropertyPathId]"
                     "          INNER JOIN ec_Property ON ec_PropertyPath.RootPropertyId = ec_Property.Id"
                     "          INNER JOIN ec_Column ON ec_PropertyMap.[ColumnId] = ec_Column.Id"
                     "          INNER JOIN ec_Table ON ec_Column.TableId = ec_Table.Id"
                     "        ) AND Name != '" DBSCHEMA_NULLTABLENAME "'", Enum::ToInt(DbTable::Type::Existing)).GetUtf8CP()) != BE_SQLITE_OK)
        {
        BeAssert(false && "system sql schema changed");
        return ERROR;
        }
    
    std::vector<Utf8String> nonVirtualTables;
    std::vector<Utf8String> virtualTables;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        if (DbSchemaPersistenceManager::IsTrue(stmt.GetValueInt(1)))
            virtualTables.push_back(stmt.GetValueText(0));
        else
            nonVirtualTables.push_back(stmt.GetValueText(0));
        }

    stmt.Finalize();
    if (stmt.Prepare(m_ecdb, "DELETE FROM ec_Table WHERE Name = ?") != BE_SQLITE_OK)
        {
        BeAssert(false && "system sql schema changed");
        return ERROR;
        }

    for (Utf8StringCR name : virtualTables)
        {
        stmt.Reset();
        stmt.ClearBindings();
        stmt.BindText(1, name, Statement::MakeCopy::No);
        if (stmt.Step() != BE_SQLITE_DONE)
            {
            BeAssert(false && "constraint voilation");
            return ERROR;
            }
        }

    if (!nonVirtualTables.empty())
        {
        BeBriefcaseId briefcaseId = GetECDb().GetBriefcaseId();
        const bool allowDbSchemaChange = briefcaseId.IsMasterId() || briefcaseId.IsStandaloneId();
        if (!allowDbSchemaChange)
            {
            Issues().Report(ECDbIssueSeverity::Error, "Failed to import ECSchemas: Imported ECSchemas would change the database schema. "
                                                               "This is only allowed for standalone briefcases or the master briefcase. Briefcase id: %" PRIu32, briefcaseId.GetValue());
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
                BeAssert(false && "constraint voilation");
                return ERROR;
                }
            }
        }
    return SUCCESS;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      12/2015
//---------------+---------------+---------------+---------------+---------------+--------
DbTable const* ECDbMap::GetPrimaryTable(DbTable const& joinedTable) const
    {
    if (joinedTable.GetType() != DbTable::Type::Joined)
        return &joinedTable;

    for (ECClassId firstClassId : GetLightweightCache().GetClassesForTable(joinedTable))
        {
        ClassMapCP classMap = GetClassMap(firstClassId);
        if (classMap != nullptr)
            {
            if (!classMap->IsMappedToSingleTable())
                {
                return &classMap->GetPrimaryTable();
                }
            }
        }

    BeAssert(false && "Must find a classmap");
    return nullptr;
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
MappingStatus ECDbMap::MapSchemas(SchemaImportContext& schemaImportContext)
    {
    if (m_schemaImportContext != nullptr)
        {
        BeAssert(false && "MapSchemas is expected to be called if no other schema import is running.");
        return MappingStatus::Error;
        }

    if (schemaImportContext.GetECSchemaCompareContext().HasNoSchemasToImport())
        return MappingStatus::Success;

    m_schemaImportContext = &schemaImportContext;

    const MappingStatus stat = DoMapSchemas();
    if (MappingStatus::Success != stat)
        {
        m_schemaImportContext = nullptr;
        return stat;
        }

    if (SUCCESS != SaveDbSchema())
        {
        ClearCache();
        m_schemaImportContext = nullptr;
        return MappingStatus::Error;
        }

    if (SUCCESS != CreateOrUpdateRequiredTables())
        {
        ClearCache();
        m_schemaImportContext = nullptr;
        return MappingStatus::Error;
        }

    if (SUCCESS != CreateOrUpdateIndexesInDb())
        {
        ClearCache();
        m_schemaImportContext = nullptr;
        return MappingStatus::Error;
        }

    if (GetSchemaImportContext()->GetECSchemaCompareContext().RequiresUpdate())
        {
        //Following step is simply done to process navigation properties for classMap loaded via above step where we sync index information.
        //If we do not do following then we must clear cache in case of schema upgrade which we like to avoid.
        if (SUCCESS != GetSchemaImportContext()->GetClassMapLoadContext().Postprocess(*this))
            {
            ClearCache();
            m_schemaImportContext = nullptr;
            return MappingStatus::Error;
            }
        }
    
    if (PurgeOrphanTables() != SUCCESS)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    m_schemaImportContext = nullptr;
    return MappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan         12/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMap::CreateECClassViewsInDb() const
    {
    return ViewGenerator::CreateECClassViews(GetECDb());
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
void ECDbMap::GatherRootClasses(ECClassCR ecclass, std::set<ECClassCP>& doneList, std::set<ECClassCP>& rootClassSet, std::vector<ECClassCP>& rootClassList, std::vector<ECRelationshipClassCP>& rootRelationshipList)
    {
    if (doneList.find(&ecclass) != doneList.end())
        return;

    doneList.insert(&ecclass);
    if (!ecclass.HasBaseClasses())
        {
        if (rootClassSet.find(&ecclass) == rootClassSet.end())
            {
            rootClassSet.insert(&ecclass);
            if (auto relationship = ecclass.GetRelationshipClassCP())
                rootRelationshipList.push_back(relationship);
            else
                rootClassList.push_back(&ecclass);
            }

        return;
        }

    for (ECClassCP baseClass : ecclass.GetBaseClasses())
        {
        if (baseClass == nullptr)
            continue;

        if (doneList.find(baseClass) != doneList.end())
            return;

        GatherRootClasses(*baseClass, doneList, rootClassSet, rootClassList, rootRelationshipList);
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan         09/2012
//---------------------------------------------------------------------------------------
MappingStatus ECDbMap::DoMapSchemas()
    {
    if (AssertIfIsNotImportingSchema())
        return MappingStatus::Error;

    // Identify root classes/relationship-classes
    ECSchemaCompareContext& ctx = GetSchemaImportContext()->GetECSchemaCompareContext();

    std::set<ECClassCP> doneList;
    std::set<ECClassCP> rootClassSet;
    std::vector<ECClassCP> rootClassList;
    std::vector<ECRelationshipClassCP> rootRelationshipList;

    for (ECSchemaCP schema : ctx.GetImportingSchemas())
        {
        if (schema->IsSupplementalSchema())
            continue; // Don't map any supplemental schemas

        for (ECClassCP ecClass : schema->GetClasses())
            {
            if (doneList.find(ecClass) != doneList.end())
                continue;

            GatherRootClasses(*ecClass, doneList, rootClassSet, rootClassList, rootRelationshipList);
            }
        }
    
    if (GetDbSchemaR().SynchronizeExistingTables() != SUCCESS)
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Synchronizing existing table to which classes are mapped failed.");
        return MappingStatus::Error;
        }

    // Starting with the root, recursively map the entire class hierarchy. 
    MappingStatus status = MappingStatus::Success;
    for (ECClassCP rootClass : rootClassList)
        {
        status = MapClass(*rootClass);
        if (status == MappingStatus::Error)
            return status;
        }

    //need to add classid cols where necessary for classes before processing relationships
    if (SUCCESS != FinishTableDefinitions(true))
        return MappingStatus::Error;

    BeAssert(status != MappingStatus::BaseClassesNotMapped && "Expected to resolve all class maps by now.");
    for (ECRelationshipClassCP rootRelationshipClass : rootRelationshipList)
        {
        status = MapClass(*rootRelationshipClass);
        if (status == MappingStatus::Error)
            return status;
        }

    //NavigationPropertyMaps can only be finished after all relationships have been mapped
    if (SUCCESS != GetSchemaImportContext()->GetClassMapLoadContext().Postprocess(*this))
        return MappingStatus::Error;

    BeAssert(status != MappingStatus::BaseClassesNotMapped && "Expected to resolve all class maps by now.");
    for (auto& kvpair : GetSchemaImportContext()->GetClassMappingInfoCache())
        {
        if (SUCCESS != kvpair.first->CreateUserProvidedIndexes(*GetSchemaImportContext(), kvpair.second->GetIndexInfos()))
            return MappingStatus::Error;
        }

    //now create class id cols for the relationship classes, and ensure shared col min count
    if (SUCCESS != FinishTableDefinitions())
        return MappingStatus::Error;

    return MappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
//* @bsimethod                                 Affan.Khan                           07 / 2012
//---------------------------------------------------------------------------------------
 BentleyStatus ECDbMap::TryLoadClassMap(ClassMapPtr& classMap, ClassMapLoadContext& ctx, ECN::ECClassCR ecClass) const
    {
    BeMutexHolder lock(m_mutex);
    classMap = nullptr;

    if (!Enum::Contains(m_dbSchema.GetLoadState(), DbSchema::LoadState::Core))
        {
        if (DbSchemaPersistenceManager::Load(GetDbSchemaR(), m_ecdb, DbSchema::LoadState::Core) != SUCCESS)
            {
            BeAssert(false);
            return ERROR;
            }
        }

    std::vector<ClassDbMapping const*> const* classMaps = m_dbSchema.GetDbMappings().FindClassMappings(ecClass.GetId());
    if (classMaps == nullptr)
        return SUCCESS;

    if (classMaps->empty())
        {
        BeAssert(false && "Failed to find ClassDbMapping for given ECClass");
        return ERROR;
        }

    if (classMaps->size() > 1)
        {
        BeAssert(false && "Feature of nested class map not implemented");
        return ERROR;
        }

    ClassDbMapping const& classMapInfo = *classMaps->front();
    ClassDbMapping const* baseClassMapInfo = classMapInfo.GetBaseClassMapping();
    ECClassCP baseClass = baseClassMapInfo == nullptr ? nullptr : GetECDb().Schemas().GetECClass(baseClassMapInfo->GetClassId());
    ClassMap const* baseClassMap = nullptr;
    if (baseClass != nullptr)
        {
        ClassMapPtr baseClassMapPtr = nullptr;
        if (SUCCESS != TryGetClassMap(baseClassMapPtr, ctx, *baseClass))
            return ERROR;

        baseClassMap = baseClassMapPtr.get();
        }

    bool setIsDirty = false;
    ECDbMapStrategy const& mapStrategy = classMapInfo.GetMapStrategy();
    ClassMapPtr classMapTmp = nullptr;
    if (mapStrategy.IsNotMapped())
        classMapTmp = UnmappedClassMap::Create(ecClass, *this, mapStrategy, setIsDirty);
    else
        {
        ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP();
        if (ecRelationshipClass != nullptr)
            {
            if (mapStrategy.IsForeignKeyMapping())
                classMapTmp = RelationshipClassEndTableMap::Create(*ecRelationshipClass, *this, mapStrategy, setIsDirty);
            else
                classMapTmp = RelationshipClassLinkTableMap::Create(*ecRelationshipClass, *this, mapStrategy, setIsDirty);
            }
        else
            classMapTmp = ClassMap::Create(ecClass, *this, mapStrategy, setIsDirty);
        }
    classMapTmp->SetId(classMapInfo.GetId());

    if (MappingStatus::Error == AddClassMap(classMapTmp))
        return ERROR;

    std::set<ClassMap const*> loadGraph;
    if (SUCCESS != classMapTmp->Load(loadGraph, ctx, classMapInfo, baseClassMap))
        return ERROR;

    ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP();
    // Construct and initialize the class map
    if (ecRelationshipClass != nullptr)
        {
        for (ECClassCP endECClassToLoad : GetClassesFromRelationshipEnd(ecRelationshipClass->GetSource()))
            {
            ctx.AddConstraintClass(*endECClassToLoad);
            }

        for (ECClassCP endECClassToLoad : GetClassesFromRelationshipEnd(ecRelationshipClass->GetTarget()))
            {
            ctx.AddConstraintClass(*endECClassToLoad);
            }
        }

    classMap = classMapTmp;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
MappingStatus ECDbMap::MapClass(ECClassCR ecClass)
    {
    if (AssertIfIsNotImportingSchema())
        return MappingStatus::Error;

    if (!ECDbSchemaPersistenceHelper::GetECClassId(m_ecdb, ecClass).IsValid())
        {
        LOG.errorv("ECClass %s does not exist in ECDb. Import ECSchema containing the class first", ecClass.GetFullName());
        BeAssert(false);
        return MappingStatus::Error;
        }

    ClassMapPtr existingClassMap = nullptr;
    if (SUCCESS != TryGetClassMap(existingClassMap, m_schemaImportContext->GetClassMapLoadContext(), ecClass))
        return MappingStatus::Error;

    if (existingClassMap == nullptr)
        {
        MappingStatus status = MappingStatus::Success;
        std::unique_ptr<ClassMappingInfo> classMapInfo = ClassMappingInfoFactory::Create(status, ecClass, *this);
        if ((status == MappingStatus::BaseClassesNotMapped || status == MappingStatus::Error))
            return status;

        BeAssert(classMapInfo != nullptr);

        ECDbMapStrategy const& mapStrategy = classMapInfo->GetMapStrategy();

        ClassMapPtr classMap = nullptr;
        if (mapStrategy.IsNotMapped())
            classMap = UnmappedClassMap::Create(ecClass, *this, mapStrategy, true);
        else
            {
            auto ecRelationshipClass = ecClass.GetRelationshipClassCP();
            if (ecRelationshipClass != nullptr)
                {
                if (mapStrategy.IsForeignKeyMapping())
                    classMap = RelationshipClassEndTableMap::Create(*ecRelationshipClass, *this, mapStrategy, true);
                else
                    classMap = RelationshipClassLinkTableMap::Create(*ecRelationshipClass, *this, mapStrategy, true);
                }
            else
                classMap = ClassMap::Create(ecClass, *this, mapStrategy, true);
            }

        status = AddClassMap(classMap);
        if (status == MappingStatus::Error)
            return status;

        status = classMap->Map(*GetSchemaImportContext(), *classMapInfo);
        GetSchemaImportContext()->CacheClassMapInfo(*classMap, classMapInfo);

        //error
        if (status == MappingStatus::BaseClassesNotMapped || status == MappingStatus::Error)
            return status;
        }

    for (ECClassP childClass : ecClass.GetDerivedClasses())
        {
        MappingStatus status = MapClass(*childClass);
        if (status == MappingStatus::Error)
            return status;
        }

    return MappingStatus::Success;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MappingStatus ECDbMap::AddClassMap(ClassMapPtr& classMap) const
    {
    BeMutexHolder lock(m_mutex);
    ECClassCR ecClass = classMap->GetClass();
    if (m_classMapDictionary.end() != m_classMapDictionary.find(ecClass.GetId()))
        {
        LOG.errorv("Attempted to add a second ClassMap for ECClass %s", ecClass.GetFullName());
        BeAssert(false && "Attempted to add a second ClassMap for the same ECClass");
        return MappingStatus::Error;
        }

    m_classMapDictionary[ecClass.GetId()] = classMap;
    return MappingStatus::Success;
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
// @bsimethod                                                    Krischan.Eberle   01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMap const* ECDbMap::GetClassMap(ECN::ECClassId classId) const
    {
    ECClassCP ecClass = GetECDb().Schemas().GetECClass(classId);
    if (ecClass == nullptr)
        return nullptr;

    return GetClassMap(*ecClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   02/2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMap::TryGetClassMap(ClassMapPtr& classMap, ClassMapLoadContext& ctx, ECN::ECClassCR ecClass) const
    {
    BeMutexHolder lock(m_mutex);
    if (!ECDbSchemaPersistenceHelper::GetECClassId(m_ecdb, ecClass).IsValid())
        {
        BeAssert(false && "TryGetClassMap fails because ecClass does not exist in the file.");
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

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbTable* ECDbMap::FindOrCreateTable(SchemaImportContext* schemaImportContext, Utf8CP tableName, DbTable::Type tableType, bool isVirtual, Utf8CP primaryKeyColumnName, DbTable const* baseTable)
    {
    if (AssertIfIsNotImportingSchema())
        return nullptr;

    BeMutexHolder lock(m_mutex);
    DbTable* table = m_dbSchema.FindTableP(tableName);
    if (table != nullptr)
        {
        if (table->GetType() != tableType)
            {
            std::function<Utf8CP(bool)> toStr = [] (bool val) { return val ? "true" : "false"; };
            LOG.warningv("Multiple classes are mapped to the table %s although the classes require mismatching table metadata: "
                         "Metadata IsMappedToExistingTable: Expected=%s - Actual=%s. Actual value is ignored.",
                         tableName,
                         toStr(tableType == DbTable::Type::Existing), toStr(!table->IsOwnedByECDb()));
            BeAssert(false && "ECDb uses a table for two classes although the classes require mismatching table metadata.");
            }

        return table;
        }

    if (tableType != DbTable::Type::Existing)
        {
        table = m_dbSchema.CreateTable(tableName, tableType, isVirtual ? PersistenceType::Virtual : PersistenceType::Persisted, baseTable);
        if (Utf8String::IsNullOrEmpty(primaryKeyColumnName))
            primaryKeyColumnName = ECDB_COL_ECInstanceId;

        DbColumn* column = table->CreateColumn(primaryKeyColumnName, DbColumn::Type::Integer, DbColumn::Kind::ECInstanceId, PersistenceType::Persisted);
        if (table->GetPersistenceType() == PersistenceType::Persisted)
            {
            std::vector<DbColumn*> pkColumns {column};
            if (SUCCESS != table->CreatePrimaryKeyConstraint(pkColumns))
                return nullptr;
            }
        }
    else
        {
        table = m_dbSchema.CreateTableAndColumnsForExistingTableMapStrategy(tableName);
        if (table == nullptr)
            return nullptr;

        if (!Utf8String::IsNullOrEmpty(primaryKeyColumnName))
            {
            const bool canEdit = table->GetEditHandle().CanEdit();
            if (!canEdit)
                table->GetEditHandleR().BeginEdit();

            DbColumn* idColumn = table->FindColumnP(primaryKeyColumnName);
            if (idColumn == nullptr)
                {
                LOG.errorv("Primary key column '%s' does not exist in table '%s' which was specified in ClassMap custom attribute together with ExistingTable MapStrategy.", primaryKeyColumnName, tableName);
                return nullptr;
                }

            idColumn->SetKind(DbColumn::Kind::ECInstanceId);
            if (!canEdit)
                table->GetEditHandleR().EndEdit();
            }
        }

    return table;
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
            entry.second->GetType() == ClassMap::Type::Unmapped)
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
// @bsimethod                                                  Krischan.Eberle    02/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMap::EvaluateColumnNotNullConstraints() const
    {
    //NOT NULL constraints (either implied from (1,1) multiplicity of
    //relationship or because of PropertyMap CA) can only be enforced
    //if classes other than subclasses of the respective class are mapped to the same column.
    //If it can be enforced, a NOT NULL constraint will be added. Otherwise it is dropped
    //and a warning is logged.

    //put relevant classids in a local vector as processing will imply loading more class maps
    //into the cache, hence modifying m_classMapDictionary, hence invalidating the iterator.
    std::vector<ECClassId> endTableRelClassIds;
    for (bpair<ECClassId, ClassMapPtr> const& kvPair : m_classMapDictionary)
        {
        ClassMapCR classMap = *kvPair.second;
        if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
            {
            RelationshipClassEndTableMap const& relClassMap = static_cast<RelationshipClassEndTableMap const&> (classMap);
            const bool impliesNotNullOnFkCol = relClassMap.GetConstraintMap(relClassMap.GetReferencedEnd()).GetRelationshipConstraint().GetCardinality().GetLowerLimit() > 0;
            if (impliesNotNullOnFkCol)
                endTableRelClassIds.push_back(kvPair.first);
            }
        }

    for (ECClassId relClassId : endTableRelClassIds)
        {
        ClassMap const* classMap = GetClassMap(relClassId);
        if (classMap == nullptr || classMap->GetType() != ClassMap::Type::RelationshipEndTable)
            {
            BeAssert(false);
            return ERROR;
            }

        RelationshipClassEndTableMap const& relClassMap = static_cast<RelationshipClassEndTableMap const&> (*classMap);

        ECRelationshipConstraintCR foreignEndConstraint = relClassMap.GetConstraintMap(relClassMap.GetForeignEnd()).GetRelationshipConstraint();
        const bool isPolymorphicConstraint = foreignEndConstraint.GetIsPolymorphic();

        std::set<ClassMap const*> constraintClassMaps;
        for (ECRelationshipConstraintClassCP constraintClass : relClassMap.GetConstraintMap(relClassMap.GetForeignEnd()).GetRelationshipConstraint().GetConstraintClasses())
            {
            if (SUCCESS != GetClassMapsFromRelationshipEnd(constraintClassMaps, constraintClass->GetClass(), isPolymorphicConstraint))
                {
                BeAssert(false);
                return ERROR;
                }
            }

        bmap<DbTable const*, bset<ClassMap const*>> constraintClassesPerTable;
        for (ClassMap const* constraintClassMap : constraintClassMaps)
            {
            //only non-abstract classes need to be considered as NOT NULL can be applied if base classes are all abstract
            //as there will not be rows for those base classes
            if (constraintClassMap->GetClass().GetClassModifier() == ECClassModifier::Abstract)
                continue;

            for (DbTable const* table : constraintClassMap->GetTables())
                {
                constraintClassesPerTable[table].insert(constraintClassMap);
                }
            }

        LightweightCache const& lwc = GetLightweightCache();
        for (DbTable const* fkTable : relClassMap.GetTables())
            {
            std::vector<ECClassId> allClassIds = lwc.GetNonAbstractClassesForTable(*fkTable);
            bset<ClassMap const*> const& constraintClassIds = constraintClassesPerTable[fkTable];

            DbColumn const* fkColumn = relClassMap.GetReferencedEndECInstanceIdPropMap()->GetSingleColumn(*fkTable, true);
            if (fkColumn == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }

            //If FK column is a key property pointing to the ECInstanceId, which is the PK, it is already NOT NULL implicitly
            if (fkColumn->DoNotAllowDbNull())
                continue;

            if (allClassIds.size() == constraintClassIds.size())
                {
                DbColumn* fkColumnP = const_cast<DbColumn*> (fkColumn);
                fkColumnP->GetConstraintsR().SetNotNullConstraint();
                continue;
                }

            Issues().Report(ECDbIssueSeverity::Warning, "The cardinality of the ECRelationshipClass '%s' "
                                                            "would imply the foreign key column to be 'not nullable'. ECDb cannot enforce that though for the "
                                                            "foreign key column '%s' in table '%s' because other classes not involved in the ECRelationshipClass map to that table. "
                                                            "Therefore the column is created without NOT NULL constraint.",
                                                            relClassMap.GetClass().GetFullName(), fkColumn->GetName().c_str(), fkTable->GetName().c_str());
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      12/2011
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMap::CreateOrUpdateRequiredTables() const
    {
    if (AssertIfIsNotImportingSchema())
        return ERROR;

    BeMutexHolder lock(m_mutex);
    m_ecdb.GetStatementCache().Empty();
    StopWatch timer(true);

    if (SUCCESS != EvaluateColumnNotNullConstraints())
        return ERROR;

    int nCreated = 0;
    int nUpdated = 0;
    int nWasUpToDate = 0;

    for (auto& pair : GetDbSchemaR().GetTables())
        {
        DbTable* table = pair.second.get();
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

    BeAssert(Enum::Contains(m_dbSchema.GetLoadState(), DbSchema::LoadState::Indexes));

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

        DbTable const& indexTable = index->GetTable();

        ClassMap const* classMap = GetClassMap(classId);
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

        for (Partition const& horizPartition : horizPartitions)
            {
            if (&indexTable == &horizPartition.GetTable())
                continue;

            bset<DbTable const*> alreadyProcessedTables;
            //table of index doesn't need to be processed again either, so put it in the set, too
            alreadyProcessedTables.insert(&indexTable);
            for (ECClassId derivedClassId : horizPartition.GetClassIds())
                {
                ClassMap const* derivedClassMap = GetClassMap(derivedClassId);
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

    return DbSchemaPersistenceManager::CreateOrUpdateIndexes(m_ecdb, m_dbSchema);
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

        if (SUCCESS != CreateClassIdColumnIfNecessary(*table, classMaps))
            return ERROR;

        if (onlyCreateClassIdColumns)
            continue;

        if (SUCCESS != table->EnsureMinimumNumberOfSharedColumns())
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMap::CreateClassIdColumnIfNecessary(DbTable& table, bset<ClassMap*> const& classMaps) const
    {
    if (table.GetFilteredColumnFirst(DbColumn::Kind::ECClassId) != nullptr ||
        table.GetPersistenceType() != PersistenceType::Persisted ||
        table.GetType() == DbTable::Type::Existing)
        return SUCCESS;

    bool addClassIdCol = false;
    if (classMaps.size() == 1)
        addClassIdCol = (*classMaps.begin())->GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::SharedTable;
    else
        addClassIdCol = classMaps.size() > 1;

    if (!addClassIdCol)
        return SUCCESS;

    DbColumn* ecClassIdColumn = table.CreateColumn(ECDB_COL_ECClassId, DbColumn::Type::Integer, 1, DbColumn::Kind::ECClassId, PersistenceType::Persisted);
    if (ecClassIdColumn == nullptr)
        return ERROR;

    ecClassIdColumn->GetConstraintsR().SetNotNullConstraint();
    //whenever we create a class id column, we index it to speed up the frequent class id look ups
    Utf8String indexName("ix_");
    indexName.append(table.GetName()).append("_ecclassid");
    return GetDbSchemaR().CreateIndex(table, indexName.c_str(), false, {ecClassIdColumn},
                                      false, true, ECClassId()) != nullptr ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<ECClassCP> ECDbMap::GetClassesFromRelationshipEnd(ECRelationshipConstraintCR relationshipEnd) const
    {
    //for recursive lambdas, iOS requires us to define the lambda variable before assigning the actual function to it.
    std::function<void(std::vector<ECClassCP>&, ECClassP, bool)> gatherClassesDelegate;
    gatherClassesDelegate =
        [this, &gatherClassesDelegate] (std::vector<ECClassCP>& classes, ECClassP ecClass, bool includeSubclasses)
        {
        classes.push_back(ecClass);
        if (includeSubclasses)
            {
            for (auto childClass : ecClass->GetDerivedClasses())
                {
                gatherClassesDelegate(classes, childClass, includeSubclasses);
                }
            }
        };

    bool isPolymorphic = relationshipEnd.GetIsPolymorphic();
    std::vector<ECClassCP> classes;
    for (ECClassCP ecClass : relationshipEnd.GetClasses())
        {
        ECClassP ecClassP = const_cast<ECClassP> (ecClass);
        gatherClassesDelegate(classes, ecClassP, isPolymorphic);
        }

    return classes;
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
    bool abstractEndPoint = relationshipEnd.GetClasses().size() == 1 && relationshipEnd.GetClasses().front()->GetClassModifier() == ECClassModifier::Abstract;
    std::vector<ECClassCP> classes = GetClassesFromRelationshipEnd(relationshipEnd);
    for (ClassMap const* classMap : classMaps)
        {
        if (abstractEndPoint)
            tables[classMap->GetPrimaryTable().GetPersistenceType()].insert(&classMap->GetJoinedTable());
        else
            tables[classMap->GetPrimaryTable().GetPersistenceType()].insert(&classMap->GetPrimaryTable());
        }

    if (tables[PersistenceType::Persisted].size() > 0)
        return tables[PersistenceType::Persisted].size();

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
    for (ECClassCP ecClass : constraint.GetClasses())
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

        const bool recursive = classMap->GetMapStrategy().GetStrategy() != ECDbMapStrategy::Strategy::SharedTable && constraint.GetIsPolymorphic();
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

    if (classMap->GetMapStrategy().IsNotMapped())
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
void ECDbMap::ClearCache()
    {
    BeMutexHolder lock(m_mutex);
    m_classMapDictionary.clear();
    m_dbSchema.Reset();
    m_lightweightCache.Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMap::SaveDbSchema() const
    {
    BeMutexHolder lock(m_mutex);
    StopWatch stopWatch(true);
    int i = 0;
    std::set<ClassMap const*> doneList;
    for (bpair<ECClassId, ClassMapPtr> const& kvPair : m_classMapDictionary)
        {
        ClassMapR classMap = *kvPair.second;
        ECClassCR ecClass = classMap.GetClass();
        if (classMap.IsDirty())
            {
            i++;
            if (SUCCESS != classMap.Save(doneList))
                {
                Issues().Report(ECDbIssueSeverity::Error, "Failed to save mapping for ECClass %s: %s", ecClass.GetFullName(), m_ecdb.GetLastError().c_str());
                return ERROR;
                }
            }
        }

    if (SUCCESS != DbSchemaPersistenceManager::Save(m_ecdb, m_dbSchema))
        return ERROR;

    m_lightweightCache.Reset();
    stopWatch.Stop();

    LOG.debugv("Saving ECDbMap for %d ECClasses took %.4lf msecs.", i, stopWatch.GetElapsedSeconds() * 1000.0);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                      06/2016
//+---------------+---------------+---------------+---------------+---------------+------
IssueReporter const& ECDbMap::Issues() const { return m_ecdb.GetECDbImplR().GetIssueReporter(); }

END_BENTLEY_SQLITE_EC_NAMESPACE

