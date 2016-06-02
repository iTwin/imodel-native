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
            GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECSchemas: Imported ECSchemas would change the database schema. "
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

    if (!ecClass.HasId())
        {
        if (!ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema(GetECDb(), ecClass).IsValid())
            {
            LOG.errorv("ECClass %s does not exist in ECDb. Import ECSchema containing the class first", ecClass.GetFullName());
            BeAssert(false);
            return MappingStatus::Error;
            }
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
    if (!ecClass.HasId())
        {
        ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema(m_ecdb, ecClass);
        BeAssert(ecClass.HasId() && "ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema was not able to retrieve a class id.");
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

            m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning, "The cardinality of the ECRelationshipClass '%s' "
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
// @bsimethod                                Affan.Khan                      12/2015
//+---------------+---------------+---------------+---------------+---------------+------
std::set<DbTable const*> ECDbMap::GetTablesFromRelationshipEnd(ECRelationshipConstraintCR relationshipEnd, bool ignoreJoinedTables) const
    {
    bool hasAnyClass = false;
    std::set<ClassMap const*> classMaps = GetClassMapsFromRelationshipEnd(relationshipEnd, &hasAnyClass);

    if (hasAnyClass)
        return std::set<DbTable const*>();

    std::map<DbTable const*, std::set<DbTable const*>> joinedTables;
    std::set<DbTable const*> tables;
    for (ClassMap const* classMap : classMaps)
        {
        std::vector<DbTable*> const& classPersistInTables = classMap->GetTables();
        if (classPersistInTables.size() == 1)
            {
            tables.insert(classPersistInTables.front());
            continue;
            }

        for (DbTable const* table : classPersistInTables)
            {
            if (DbTable const* primaryTable = table->GetParentOfJoinedTable())
                {
                joinedTables[primaryTable].insert(table);
                tables.insert(table);
                }
            }
        }

    for (auto const& pair : joinedTables)
        {
        DbTable const* primaryTable = pair.first;
        std::set<DbTable const*> const& joinedTables = pair.second;

        bool isPrimaryTableSelected = tables.find(primaryTable) != tables.end();
        if (ignoreJoinedTables)
            {
            for (auto childTable : primaryTable->GetJoinedTables())
                tables.erase(childTable);

            tables.insert(primaryTable);
            continue;
            }

        if (isPrimaryTableSelected)
            {
            for (DbTable const* joinedTable : joinedTables)
                tables.erase(joinedTable);
            }
        }

    if (!ignoreJoinedTables)
        return tables;

    std::map<PersistenceType, std::set<DbTable const*>> finalListOfTables;
    for (DbTable const* table : tables)
        {
        finalListOfTables[table->GetPersistenceType()].insert(table);
        }

    return finalListOfTables[PersistenceType::Persisted];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan Khan                          08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbMap::ClearCache()
    {
    BeMutexHolder lock(m_mutex);
    m_classMapDictionary.clear();
    m_dbSchema.Reset();
    m_lightweightCache.Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan Khan                          08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
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
                m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to save mapping for ECClass %s: %s", ecClass.GetFullName(), m_ecdb.GetLastError().c_str());
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


//************************************************************************************
// LightweightCache
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::LoadClassIdsPerTable() const
    {
    if (m_loadedFlags.m_classIdsPerTableIsLoaded)
        return;

    Utf8String sql;
    sql.Sprintf("SELECT t.Id, t.Name, c.Id, c.Modifier FROM ec_Table t, ec_Class c, ec_PropertyMap pm, ec_ClassMap cm, ec_PropertyPath pp, ec_Column col "
                "WHERE cm.Id=pm.ClassMapId AND pp.Id=pm.PropertyPathId AND col.Id=pm.ColumnId AND (col.ColumnKind & %d = 0) AND "
                "c.Id=cm.ClassId AND t.Id=col.TableId AND "
                "cm.MapStrategy<>%d AND cm.MapStrategy<>%d "
                "GROUP BY t.Id, c.Id", Enum::ToInt(DbColumn::Kind::ECClassId),
                Enum::ToInt(ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable),
                Enum::ToInt(ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable));

    CachedStatementPtr stmt = m_map.GetECDb().GetCachedStatement(sql.c_str());
    DbTableId currentTableId;
    DbTable const* currentTable = nullptr;
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        DbTableId tableId = stmt->GetValueId<DbTableId>(0);
        if (currentTableId != tableId)
            {
            Utf8CP tableName = stmt->GetValueText(1);
            currentTable = m_map.GetDbSchema().FindTable(tableName);
            currentTableId = tableId;
            BeAssert(currentTable != nullptr);
            }

        ECClassId id = stmt->GetValueId<ECClassId>(2);
        ECClassModifier classModifier = Enum::FromInt<ECClassModifier>(stmt->GetValueInt(3));
        m_classIdsPerTable[currentTable].push_back(id);
        if (classModifier != ECClassModifier::Abstract)
            m_nonAbstractClassIdsPerTable[currentTable].push_back(id);

        m_tablesPerClassId[id].insert(currentTable);
        }

    m_loadedFlags.m_classIdsPerTableIsLoaded = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::LoadRelationshipCache() const
    {
    if (m_loadedFlags.m_relationshipCacheIsLoaded)
        return;

    Utf8CP sql0 =
        "WITH RECURSIVE DerivedClassList(RelationshipClassId, RelationshipEnd, IsPolymorphic, CurrentClassId, DerivedClassId) "
        "AS (SELECT RC.RelationshipClassId,RC.RelationshipEnd,RC.IsPolymorphic,RCC.ClassId,RCC.ClassId "
        "FROM ec_RelationshipConstraintClass RCC INNER JOIN ec_RelationshipConstraint RC ON RC.Id = RCC.ConstraintId "
        "UNION "
        "SELECT DCL.RelationshipClassId, DCL.RelationshipEnd, DCL.IsPolymorphic, BC.BaseClassId, BC.ClassId "
        "FROM DerivedClassList DCL INNER JOIN ec_ClassHasBaseClasses BC ON BC.BaseClassId = DCL.DerivedClassId "
        "WHERE IsPolymorphic = 1) "
        "SELECT DerivedClassId, RelationshipClassId, RelationshipEnd FROM DerivedClassList";

    auto stmt0 = m_map.GetECDb().GetCachedStatement(sql0);
    while (stmt0->Step() == BE_SQLITE_ROW)
        {
        ECClassId constraintClassId = stmt0->GetValueId<ECClassId>(0);
        ECClassId relationshipId = stmt0->GetValueId<ECClassId>(1);
        BeAssert(!stmt0->IsColumnNull(2));
        RelationshipEnd end = stmt0->GetValueInt(2) == 0 ? RelationshipEnd::Source : RelationshipEnd::Target;

        bmap<ECN::ECClassId, RelationshipEnd>& relClassIds = m_relationshipClassIdsPerConstraintClassIds[constraintClassId];
        auto relIt = relClassIds.find(relationshipId);
        if (relIt == relClassIds.end())
            relClassIds[relationshipId] = end;
        else
            relClassIds[relationshipId] = static_cast<RelationshipEnd>((int) (relIt->second) | (int) (end));


        bmap<ECN::ECClassId, RelationshipEnd>& constraintClassIds = m_constraintClassIdsPerRelClassIds[relationshipId];
        auto constraintIt = constraintClassIds.find(constraintClassId);
        if (constraintIt == constraintClassIds.end())
            constraintClassIds[constraintClassId] = end;
        else
            constraintClassIds[constraintClassId] = static_cast<RelationshipEnd>((int) (constraintIt->second) | (int) (end));
        }

    m_loadedFlags.m_relationshipCacheIsLoaded = true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::LoadHorizontalPartitions()  const
    {
    if (m_loadedFlags.m_horizontalPartitionsIsLoaded)
        return;

    Utf8String sql;
    sql.Sprintf(
        "WITH RECURSIVE DerivedClassList(RootClassId,CurrentClassId,DerivedClassId) "
        "AS (SELECT Id, Id, Id FROM ec_Class "
        "UNION "
        "SELECT RootClassId, BC.BaseClassId, BC.ClassId FROM DerivedClassList DCL "
        "INNER JOIN ec_ClassHasBaseClasses BC ON BC.BaseClassId = DCL.DerivedClassId), "
        "TableMapInfo AS ("
        "SELECT ec_Class.Id ClassId, ec_Table.Name TableName FROM ec_PropertyMap "
        "JOIN ec_Column ON ec_Column.Id = ec_PropertyMap.ColumnId AND (ec_Column.ColumnKind & %d = 0) "
        "JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.PropertyPathId "
        "JOIN ec_ClassMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId "
        "JOIN ec_Class ON ec_Class.Id = ec_ClassMap.ClassId "
        "JOIN ec_Table ON ec_Table.Id = ec_Column.TableId "
        "WHERE ec_ClassMap.MapStrategy<>%d AND ec_ClassMap.MapStrategy<>%d AND ec_Table.Type<>%d "
        "GROUP BY ec_Class.Id, ec_Table.Name) "
        "SELECT DISTINCT DCL.RootClassId, DCL.DerivedClassId, TMI.TableName FROM DerivedClassList DCL "
        "INNER JOIN TableMapInfo TMI ON TMI.ClassId=DCL.DerivedClassId ORDER BY DCL.RootClassId,TMI.TableName,DCL.DerivedClassId",
        Enum::ToInt(DbColumn::Kind::ECClassId), Enum::ToInt(ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable),
        Enum::ToInt(ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable), Enum::ToInt(DbTable::Type::Joined));

    CachedStatementPtr stmt = m_map.GetECDb().GetCachedStatement(sql.c_str());
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId rootClassId = stmt->GetValueId<ECClassId>(0);
        ECClassId derivedClassId = stmt->GetValueId<ECClassId>(1);

        Utf8CP tableName = stmt->GetValueText(2);
        DbTable const* table = m_map.GetDbSchema().FindTable(tableName);
        BeAssert(table != nullptr);
        std::vector<ECClassId>& horizontalPartition = m_horizontalPartitions[rootClassId][table];
        if (derivedClassId == rootClassId)
            horizontalPartition.insert(horizontalPartition.begin(), derivedClassId);
        else
            horizontalPartition.insert(horizontalPartition.end(), derivedClassId);
        }

    m_loadedFlags.m_horizontalPartitionsIsLoaded = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 08/2015
//---------------------------------------------------------------------------------------
bmap<ECN::ECClassId, ECDbMap::LightweightCache::RelationshipEnd> const& ECDbMap::LightweightCache::GetConstraintClassesForRelationshipClass(ECN::ECClassId relClassId) const
    {
    LoadRelationshipCache();
    return m_constraintClassIdsPerRelClassIds[relClassId];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
std::vector<ECClassId> const& ECDbMap::LightweightCache::GetClassesForTable(DbTable const& table) const
    {
    LoadClassIdsPerTable();
    return m_classIdsPerTable[&table];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 05/2016
//---------------------------------------------------------------------------------------
std::vector<ECClassId> const& ECDbMap::LightweightCache::GetNonAbstractClassesForTable(DbTable const& table) const
    {
    LoadClassIdsPerTable();
    return m_nonAbstractClassIdsPerTable[&table];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
bset<DbTable const*> const& ECDbMap::LightweightCache::GetVerticalPartitionsForClass(ECN::ECClassId classId) const
    {
    LoadClassIdsPerTable();
    return m_tablesPerClassId[classId];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      10/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::ClassIdsPerTableMap const& ECDbMap::LightweightCache::GetHorizontalPartitionsForClass(ECN::ECClassId classId) const
    {
    LoadHorizontalPartitions();
    return m_horizontalPartitions[classId];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::Reset()
    {
    m_loadedFlags.m_classIdsPerTableIsLoaded =
        m_loadedFlags.m_relationshipPerTableLoaded =
        m_loadedFlags.m_horizontalPartitionsIsLoaded =
        m_loadedFlags.m_relationshipCacheIsLoaded = false;

    m_horizontalPartitions.clear();
    m_classIdsPerTable.clear();
    m_nonAbstractClassIdsPerTable.clear();
    m_relationshipClassIdsPerConstraintClassIds.clear();
    m_constraintClassIdsPerRelClassIds.clear();
    m_storageDescriptions.clear();
    m_relationshipPerTable.clear();
    m_tablesPerClassId.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::LightweightCache(ECDbMapCR map) : m_map(map) { Reset(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
StorageDescription const& ECDbMap::LightweightCache::GetStorageDescription(ClassMap const& classMap)  const
    {
    const ECClassId classId = classMap.GetClass().GetId();
    auto it = m_storageDescriptions.find(classId);
    if (it == m_storageDescriptions.end())
        {
        auto des = StorageDescription::Create(classMap, *this);
        auto desP = des.get();
        m_storageDescriptions[classId] = std::move(des);
        return *desP;
        }

    return *(it->second.get());
    }


//****************************************************************************************
// StorageDescription
//****************************************************************************************

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    10 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus StorageDescription::GenerateECClassIdFilter(Utf8StringR filterSqlExpression, DbTable const& table, DbColumn const& classIdColumn, bool polymorphic, bool fullyQualifyColumnName, Utf8CP tableAlias) const
    {
    if (table.GetPersistenceType() != PersistenceType::Persisted)
        return SUCCESS; //table is virtual -> noop

    Partition const* partition = GetHorizontalPartition(table);
    if (partition == nullptr)
        {
        if (!GetVerticalPartitions().empty())
            {
            partition = GetVerticalPartition(table);
            }

        if (partition == nullptr)
            {
            BeAssert(false && "Should always find a partition for the given table");
            return ERROR;
            }
        }

    Utf8String classIdColSql;
    if (fullyQualifyColumnName)
        {
        classIdColSql.append("[");
        if (tableAlias)
            classIdColSql.append(tableAlias);
        else
            classIdColSql.append(table.GetName());

        classIdColSql.append("].");
        }

    classIdColSql.append(classIdColumn.GetName());

    if (!polymorphic)
        {
        //if partition's table is only used by a single class, no filter needed     
        if (partition->IsSharedTable())
            {
            Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
            m_classId.ToString(classIdStr);
            filterSqlExpression.append(classIdColSql).append("=").append(classIdStr);
            }

        return SUCCESS;
        }

    partition->AppendECClassIdFilterSql(filterSqlExpression, classIdColSql.c_str());
    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan    05 / 2015
//------------------------------------------------------------------------------------------
//static
std::unique_ptr<StorageDescription> StorageDescription::Create(ClassMap const& classMap, ECDbMap::LightweightCache const& lwmc)
    {
    const ECClassId classId = classMap.GetClass().GetId();
    std::unique_ptr<StorageDescription> storageDescription = std::unique_ptr<StorageDescription>(new StorageDescription(classId));
    std::set<ECClassId> derviedClassSet;
    if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
        {
        RelationshipClassEndTableMap const& relClassMap = static_cast<RelationshipClassEndTableMap const&> (classMap);
        for (DbTable const* endTable : relClassMap.GetTables())
            {
            const ECDbMap::LightweightCache::RelationshipEnd foreignEnd = relClassMap.GetForeignEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? ECDbMap::LightweightCache::RelationshipEnd::Source : ECDbMap::LightweightCache::RelationshipEnd::Target;

            Partition* hp = storageDescription->AddHorizontalPartition(*endTable, true);

            for (bpair<ECClassId, ECDbMap::LightweightCache::RelationshipEnd> const& kvpair : lwmc.GetConstraintClassesForRelationshipClass(classId))
                {
                ECClassId constraintClassId = kvpair.first;
                ECDbMap::LightweightCache::RelationshipEnd end = kvpair.second;

                if (end == ECDbMap::LightweightCache::RelationshipEnd::Both || end == foreignEnd)
                    hp->AddClassId(constraintClassId);
                }

            hp->GenerateClassIdFilter(lwmc.GetClassesForTable(*endTable));
            }

        }
    else
        {
        for (auto& kp : lwmc.GetHorizontalPartitionsForClass(classId))
            {
            auto table = kp.first;

            auto& deriveClassList = kp.second;
            derviedClassSet.insert(deriveClassList.begin(), deriveClassList.end());
            if (deriveClassList.empty())
                continue;

            Partition* hp = storageDescription->AddHorizontalPartition(*table, deriveClassList.front() == classId);
            for (ECClassId ecClassId : deriveClassList)
                {
                hp->AddClassId(ecClassId);
                }

            hp->GenerateClassIdFilter(lwmc.GetClassesForTable(*table));
            }
        }
    //add vertical partitions
    for (auto table : lwmc.GetVerticalPartitionsForClass(classId))
        {
        if (table->GetPersistenceType() == PersistenceType::Virtual)
            continue;

        Partition* vp = storageDescription->AddVerticalPartition(*table, storageDescription->GetHorizontalPartition(*table) != nullptr);
        for (ECClassId ecClassId : derviedClassSet)
            {
            vp->AddClassId(ecClassId);
            }

        vp->GenerateClassIdFilter(lwmc.GetClassesForTable(*table));
        }

    return storageDescription;
    }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    10 / 2015
//------------------------------------------------------------------------------------------
Partition const* StorageDescription::GetHorizontalPartition(bool polymorphic) const
    {
    if (!polymorphic || !HasNonVirtualPartitions())
        return &GetRootHorizontalPartition();

    if (HierarchyMapsToMultipleTables())
        return nullptr; //no single partition available

    size_t ix = m_nonVirtualHorizontalPartitionIndices[0];
    BeAssert(ix < m_horizontalPartitions.size());

    return &m_horizontalPartitions[ix];
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    10 / 2015
//------------------------------------------------------------------------------------------
Partition const* StorageDescription::GetHorizontalPartition(DbTable const& table) const
    {
    for (Partition const& part : m_horizontalPartitions)
        {
        if (&part.GetTable() == &table)
            return &part;
        }

    return nullptr;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.KHan    11 / 2015
//------------------------------------------------------------------------------------------
Partition const* StorageDescription::GetVerticalPartition(DbTable const& table) const
    {
    for (Partition const& part : m_verticalPartitions)
        {
        if (&part.GetTable() == &table)
            return &part;
        }

    return nullptr;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    10 / 2015
//------------------------------------------------------------------------------------------
Partition const& StorageDescription::GetRootHorizontalPartition() const
    {
    BeAssert(m_rootHorizontalPartitionIndex < m_horizontalPartitions.size());
    return m_horizontalPartitions[m_rootHorizontalPartitionIndex];
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
Partition* StorageDescription::AddHorizontalPartition(DbTable const& table, bool isRootPartition)
    {
    const bool isVirtual = table.GetPersistenceType() == PersistenceType::Virtual;
    m_horizontalPartitions.push_back(Partition(table));

    const size_t indexOfAddedPartition = m_horizontalPartitions.size() - 1;
    if (!isVirtual)
        m_nonVirtualHorizontalPartitionIndices.push_back(indexOfAddedPartition);

    if (isRootPartition)
        m_rootHorizontalPartitionIndex = indexOfAddedPartition;

    return &m_horizontalPartitions[indexOfAddedPartition];
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan    11 / 2015
//------------------------------------------------------------------------------------------
Partition* StorageDescription::AddVerticalPartition(DbTable const& table, bool isRootPartition)
    {
    BeAssert(table.GetPersistenceType() == PersistenceType::Persisted);
    if (table.GetPersistenceType() == PersistenceType::Virtual)
        return nullptr;

    m_verticalPartitions.push_back(Partition(table));

    const size_t indexOfAddedPartition = m_verticalPartitions.size() - 1;
    if (isRootPartition)
        m_rootVerticalPartitionIndex = indexOfAddedPartition;

    return &m_verticalPartitions[indexOfAddedPartition];
    }

//****************************************************************************************
// Partition
//****************************************************************************************
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    02 / 2016
//------------------------------------------------------------------------------------------
Partition::Partition(Partition const& rhs)
    : m_table(rhs.m_table), m_partitionClassIds(rhs.m_partitionClassIds),
    m_inversedPartitionClassIds(rhs.m_inversedPartitionClassIds), m_hasInversedPartitionClassIds(rhs.m_hasInversedPartitionClassIds)
    {}

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
Partition& Partition::operator=(Partition const& rhs)
    {
    if (this != &rhs)
        {
        m_table = rhs.m_table;
        m_partitionClassIds = rhs.m_partitionClassIds;
        m_inversedPartitionClassIds = rhs.m_inversedPartitionClassIds;
        m_hasInversedPartitionClassIds = rhs.m_hasInversedPartitionClassIds;
        }

    return *this;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
Partition::Partition(Partition&& rhs)
    : m_table(std::move(rhs.m_table)), m_partitionClassIds(std::move(rhs.m_partitionClassIds)),
    m_inversedPartitionClassIds(std::move(rhs.m_inversedPartitionClassIds)), m_hasInversedPartitionClassIds(std::move(rhs.m_hasInversedPartitionClassIds))
    {
    //nulling out the RHS m_table pointer is defensive, even if the destructor doesn't
    //free the table (as it is now owned by Partition). If the ownership ever changes,
    //this method is safe.
    rhs.m_table = nullptr;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
void Partition::GenerateClassIdFilter(std::vector<ECN::ECClassId> const& tableClassIds)
    {
    BeAssert(!m_partitionClassIds.empty());
    m_hasInversedPartitionClassIds = m_partitionClassIds.size() > tableClassIds.size() / 2;
    if (m_partitionClassIds.size() == tableClassIds.size())
        return;

    //tableClassIds list is already sorted
    auto sortedPartitionClassIds = m_partitionClassIds;
    std::sort(sortedPartitionClassIds.begin(), sortedPartitionClassIds.end());

    auto partitionClassIdsIt = sortedPartitionClassIds.begin();
    for (ECClassId candidateClassId : tableClassIds)
        {
        if (partitionClassIdsIt == sortedPartitionClassIds.end() || candidateClassId < *partitionClassIdsIt)
            m_inversedPartitionClassIds.push_back(candidateClassId);
        else
            ++partitionClassIdsIt;
        }
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
bool Partition::NeedsECClassIdFilter() const
    {
    BeAssert(!m_partitionClassIds.empty());
    //If class ids are not inversed, we always have a non-empty partition class id list. So filtering is needed.
    //if class ids are inversed, filtering is needed if the inversed list is not empty. If it is empty, it means
    //don't filter at all -> consider all class ids
    return !m_inversedPartitionClassIds.empty();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2015
//------------------------------------------------------------------------------------------
void Partition::AppendECClassIdFilterSql(Utf8StringR filterSqlExpression, Utf8CP classIdColName) const
    {
    BeAssert(!m_partitionClassIds.empty());

    std::vector<ECClassId> const* classIds = nullptr;
    Utf8CP equalOp = nullptr;
    Utf8CP setOp = nullptr;
    if (m_hasInversedPartitionClassIds)
        {
        classIds = &m_inversedPartitionClassIds;
        if (classIds->empty())
            return; //no filter needed, as all class ids should be considered

        equalOp = "<>";
        setOp = "AND";
        }
    else
        {
        classIds = &m_partitionClassIds;
        equalOp = "=";
        setOp = "OR";
        }

    bool isFirstItem = true;
    for (ECClassId classId : *classIds)
        {
        if (!isFirstItem)
            filterSqlExpression.append(" ").append(setOp).append(" ");

        Utf8Char classIdStr[BeInt64Id::ID_STRINGBUFFER_LENGTH];
        classId.ToString(classIdStr);
        filterSqlExpression.append(classIdColName).append(equalOp).append(classIdStr);

        isFirstItem = false;
        }
    }


END_BENTLEY_SQLITE_EC_NAMESPACE

