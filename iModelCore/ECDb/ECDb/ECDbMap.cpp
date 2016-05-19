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
ECDbMap::ECDbMap(ECDbR ecdb) : m_ecdb(ecdb), m_ecdbSqlManager(ecdb), m_schemaImportContext(nullptr), m_lightweightCache(*this)
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
bool ECDbMap::IsImportingSchema () const
    {
    return m_schemaImportContext != nullptr;
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
                     "        ) AND Name != '" DBSCHEMA_NULLTABLENAME "'", Enum::ToInt(TableType::Existing)).GetUtf8CP()) != BE_SQLITE_OK)
        {
        BeAssert(false && "system sql schema changed");
        return ERROR;
        }
    
    std::vector<Utf8String> noneVirtualTables;
    std::vector<Utf8String> virtualTables;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        if (stmt.GetValueInt(1) != 0)
            virtualTables.push_back(stmt.GetValueText(0));
        else
            noneVirtualTables.push_back(stmt.GetValueText(0));
        }

    stmt.Finalize();
    if (!virtualTables.empty())
        {
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
        }
    if (!noneVirtualTables.empty())
        {
        BeBriefcaseId briefcaseId = GetECDb().GetBriefcaseId();
        const bool allowDbSchemaChange = briefcaseId.IsMasterId() || briefcaseId.IsStandaloneId();
        if (!allowDbSchemaChange)
            {
            GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECSchemas: Imported ECSchemas would change the database schema. "
                                                          "This is only allowed for standalone briefcases or the master briefcase. Briefcase id: %" PRIu32, briefcaseId.GetValue());
            return ERROR;
            }

        for (Utf8StringCR name : noneVirtualTables)
            {
            if (m_ecdb.DropTable(name.c_str()) != BE_SQLITE_OK)
                {
                BeAssert(false && "failed to drop a table");
                return ERROR;
                }
            }
        }
    return SUCCESS;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      12/2015
//---------------+---------------+---------------+---------------+---------------+--------
ECDbSqlTable const* ECDbMap::GetPrimaryTable(ECDbSqlTable const& joinedTable) const
    {
    if (joinedTable.GetTableType() != TableType::Joined)
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
    m_schemaImportContext->GetECDbMapDb().Load();
    const MappingStatus stat = DoMapSchemas();
    if (MappingStatus::Success != stat)
        {
        m_schemaImportContext = nullptr;
        return stat;
        }

    if (SUCCESS != SaveMappings())
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

    ECDbMapAnalyser mapAnalyser(*this);
    if (mapAnalyser.Analyse(true /*apply changes*/) != SUCCESS)
        {
        m_schemaImportContext = nullptr;
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
    if (GetECDb().IsReadonly())
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Can only call ECDb::CreateECClassViewsInDb() on an ECDb file with read-write access.");
        return ERROR;
        }

    Utf8String sql;
    sql.Sprintf("SELECT c.Id FROM ec_Class c, ec_ClassMap cm WHERE c.Id = cm.ClassId AND c.Type IN (%d,%d) AND cm.MapStrategy<>%d", 
                Enum::ToInt(ECClassType::Entity),
                Enum::ToInt(ECClassType::Relationship),
                Enum::ToInt(ECDbMapStrategy::Strategy::NotMapped));
    
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), sql.c_str()))
        return ERROR;

    std::vector<ClassMapCP> classMaps;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ECClassId classId = (ECClassId)stmt.GetValueInt64(0);
        ClassMapCP classMap = GetClassMap(classId);
        if (classMap == nullptr)
            {
            BeAssert(classMap != nullptr);
            return ERROR;
            }

        BeAssert((classMap->GetClass().IsEntityClass() || classMap->GetClass().IsRelationshipClass()) && classMap->GetClassMapType() != IClassMap::Type::Unmapped);
        classMaps.push_back(classMap);
        }

    ECClassViewGenerator viewGenerator(*this);
    return viewGenerator.BuildViews(classMaps);
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

    StopWatch timer(true);

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

    //need to finish tables (e.g. add classid cols where necessary) for classes before processing relationships
    if (FinishTableDefinition() == ERROR)
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
    for (auto& kvpair : GetSchemaImportContext()->GetClassMapInfoCache())
        {
        if (SUCCESS != kvpair.first->CreateUserProvidedIndexes(*GetSchemaImportContext(), kvpair.second->GetIndexInfos()))
            return MappingStatus::Error;
        }

    //now finish tables for the relationship classes
    if (FinishTableDefinition() != SUCCESS)
        return MappingStatus::Error;

    return MappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
//* @bsimethod                                 Affan.Khan                           07 / 2012
//---------------------------------------------------------------------------------------
ClassMapPtr ECDbMap::LoadClassMap(ClassMapLoadContext& ctx, ECN::ECClassCR ecClass) const
    {
    BeMutexHolder lock(m_mutex);
    if (!GetSQLManager().IsLoaded())
        {
        if (GetSQLManager().Load() != SUCCESS)
            {
            BeAssert(false && "Failed to map information");
            return nullptr;
            }
        }

    ECDbSchemaManager const& schemaManager = GetECDbR().Schemas();
    std::vector<ECDbClassMapInfo const*> const* classMaps = GetSQLManager().GetMapStorage().FindClassMapsByClassId(ecClass.GetId());
    if (classMaps == nullptr)
        return nullptr;

    if (classMaps->empty())
        {
        BeAssert(false && "Failed to find classMap info for give ECClass");
        return nullptr;
        }

    if (classMaps->size() > 1)
        {
        BeAssert(false && "Feature of nested class map not implemented");
        return nullptr;
        }

    ECDbClassMapInfo const& classMapInfo = *classMaps->front();
    ECDbClassMapInfo const* baseClassMapInfo = classMapInfo.GetBaseClassMap();
    ECClassCP baseClass = baseClassMapInfo == nullptr ? nullptr : schemaManager.GetECClass(baseClassMapInfo->GetClassId());
    
    ClassMap const* baseClassMap = nullptr;
    if (baseClass != nullptr)
        {
        ClassMapPtr baseClassMapPtr = nullptr;
        if (TryGetClassMap(baseClassMapPtr, ctx, *baseClass))
            baseClassMap = baseClassMapPtr.get();
        }

    bool setIsDirty = false;
    ECDbMapStrategy const& mapStrategy = classMapInfo.GetMapStrategy();
    ClassMapPtr classMap = nullptr;
    if (mapStrategy.IsNotMapped())
        classMap = UnmappedClassMap::Create(ecClass, *this, mapStrategy, setIsDirty);
    else
        {
        ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP();
        if (ecRelationshipClass != nullptr)
            {
            if (mapStrategy.IsForeignKeyMapping())
                classMap = RelationshipClassEndTableMap::Create(*ecRelationshipClass, *this, mapStrategy, setIsDirty);
            else
                classMap = RelationshipClassLinkTableMap::Create(*ecRelationshipClass, *this, mapStrategy, setIsDirty);
            }
        else if (IClassMap::MapsToStructArrayTable(ecClass))
            classMap = StructClassMap::Create(ecClass, *this, mapStrategy, setIsDirty);
        else
            classMap = ClassMap::Create(ecClass, *this, mapStrategy, setIsDirty);
        }
    classMap->SetId(classMapInfo.GetId());

    if (MappingStatus::Error == AddClassMap(classMap))
        return nullptr;

    std::set<ClassMap const*> loadGraph;
    if (SUCCESS != classMap->Load(loadGraph, ctx, classMapInfo, baseClassMap))
        return nullptr;

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

    return classMap;
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
        if (0 == ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema(GetECDbR(), ecClass))
            {
            LOG.errorv("ECClass %s does not exist in ECDb. Import ECSchema containing the class first", ecClass.GetFullName());
            BeAssert(false);
            return MappingStatus::Error;
            }
        }

    ClassMapPtr existingClassMap = nullptr;
    const bool classMapExists = TryGetClassMap(existingClassMap, m_schemaImportContext->GetClassMapLoadContext(), ecClass);
    if (!classMapExists)
        {
        MappingStatus status = MappingStatus::Success;
        std::unique_ptr<ClassMapInfo> classMapInfo = ClassMapInfoFactory::Create(status, *m_schemaImportContext, ecClass, *this);
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
            else if (IClassMap::MapsToStructArrayTable(ecClass))
                classMap = StructClassMap::Create(ecClass, *this, mapStrategy, true);
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
MappingStatus ECDbMap::AddClassMap (ClassMapPtr& classMap) const
    {
    BeMutexHolder lock (m_mutex);
    ECClassCR ecClass = classMap->GetClass();
    if (m_classMapDictionary.end() != m_classMapDictionary.find(ecClass.GetId()))
        {
        LOG.errorv ("Attempted to add a second ClassMap for ECClass %s", ecClass.GetFullName());
        BeAssert(false && "Attempted to add a second ClassMap for the same ECClass");
        return MappingStatus::Error;
        }

    m_classMapDictionary[ecClass.GetId()]= classMap;
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
        if (!TryGetClassMap(classMap, ctx, ecClass))
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
    if (!TryGetClassMap(classMap, m_schemaImportContext->GetClassMapLoadContext(), ecClass))
        return nullptr;

    return classMap.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMap const* ECDbMap::GetClassMap(ECN::ECClassId classId) const
    {
    ECClassCP ecClass = GetECDbR().Schemas().GetECClass(classId);
    if (ecClass == nullptr)
        return nullptr;

    return GetClassMap(*ecClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   02/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbMap::TryGetClassMap (ClassMapPtr& classMap, ClassMapLoadContext& ctx, ECN::ECClassCR ecClass) const
    {
    BeMutexHolder lock (m_mutex);
    if (!ecClass.HasId ())
        {
        ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema (m_ecdb, ecClass);
        BeAssert (ecClass.HasId () && "ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema was not able to retrieve a class id.");
        }

    classMap = DoGetClassMap (ecClass);
    if (classMap != nullptr)
        return true;

    //lazy loading the class map implemented with const-casting the actual loading so that the 
    //get method itself can remain const (logically const)
    classMap = LoadClassMap(ctx, ecClass);
    return classMap != nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan         08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ClassMapPtr ECDbMap::DoGetClassMap (ECClassCR ecClass) const
    {
    auto it = m_classMapDictionary.find (ecClass.GetId());
    if (m_classMapDictionary.end () == it)
        return nullptr;
    else
        return it->second;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbSqlTable* ECDbMap::FindOrCreateTable (SchemaImportContext* schemaImportContext, Utf8CP tableName, TableType tableType, bool isVirtual, Utf8CP primaryKeyColumnName, ECDbSqlTable const* baseTable)
    {
    if (AssertIfIsNotImportingSchema ())
        return nullptr;

    BeMutexHolder lock (m_mutex);
    ECDbSqlTable* table = GetSQLManager ().GetDbSchema ().FindTableP (tableName);
    if (table != nullptr)
        {
        if (table->GetTableType() != tableType)
            {
            std::function<Utf8CP (bool)> toStr = [] (bool val) {return val ? "true" : "false"; };
            LOG.warningv ("Multiple classes are mapped to the table %s although the classes require mismatching table metadata: "
                "Metadata IsMappedToExistingTable: Expected=%s - Actual=%s. Actual value is ignored.",
                tableName,
                toStr (tableType == TableType::Existing), toStr (!table->IsOwnedByECDb()));
            BeAssert (false && "ECDb uses a table for two classes although the classes require mismatching table metadata.");
            }

        return table;
        }

    if (tableType != TableType::Existing)
        {
        table = GetSQLManager().GetDbSchemaR().CreateTable(tableName, tableType, isVirtual ? PersistenceType::Virtual : PersistenceType::Persisted, baseTable);
        if (Utf8String::IsNullOrEmpty (primaryKeyColumnName))
            primaryKeyColumnName = ECDB_COL_ECInstanceId;

        auto column = table->CreateColumn (primaryKeyColumnName, ECDbSqlColumn::Type::Integer, ColumnKind::ECInstanceId, PersistenceType::Persisted);
        if (table->GetPersistenceType () == PersistenceType::Persisted)
            {
            column->GetConstraintR ().SetIsNotNull (true);
            table->GetPrimaryKeyConstraint ()->Add (primaryKeyColumnName);
            }

        if (tableType == TableType::StructArray)
            {
            table->CreateColumn (ECDB_COL_ParentECInstanceId, ECDbSqlColumn::Type::Integer, ColumnKind::ParentECInstanceId, PersistenceType::Persisted);
            table->CreateColumn (ECDB_COL_ECPropertyPathId, ECDbSqlColumn::Type::Integer, ColumnKind::ECPropertyPathId, PersistenceType::Persisted);
            table->CreateColumn (ECDB_COL_ECArrayIndex, ECDbSqlColumn::Type::Integer, ColumnKind::ECArrayIndex, PersistenceType::Persisted);
            if (table->GetPersistenceType() == PersistenceType::Persisted)
                {
                if (schemaImportContext != nullptr)
                    {
                    //indexes are only required at schema import time
                    //struct array indices don't get a class id
                    Utf8String indexName("uix_");
                    indexName.append(table->GetName()).append("_structarraykey");
                    if (nullptr == schemaImportContext->GetECDbMapDb().CreateIndex(m_ecdb, *table, indexName.c_str(), true,
                                                                            {ECDB_COL_ParentECInstanceId, 
                                                                             ECDB_COL_ECPropertyPathId, 
                                                                             ECDB_COL_ECArrayIndex, 
                                                                             primaryKeyColumnName},
                                                                             false,
                                                                             true, 
                                                                             ECClass::UNSET_ECCLASSID))
                        {
                        BeAssert(false);
                        return nullptr;
                        }
                    }
                }
            }
        }
    else
        {
        table = GetSQLManager().GetDbSchemaR().CreateTableForExistingTableMapStrategy(GetECDbR(), tableName);
        if (table == nullptr)
            return nullptr;

        if (!Utf8String::IsNullOrEmpty (primaryKeyColumnName))
            {
            auto editMode = table->GetEditHandle ().CanEdit ();
            if (!editMode)
                table->GetEditHandleR ().BeginEdit ();

            auto systemColumn = table->FindColumnP (primaryKeyColumnName);
            if (systemColumn == nullptr)
                {
                LOG.errorv("Primary key column '%s' does not exist in table '%s' which was specified in ClassMap custom attribute together with ExistingTable MapStrategy. Specify the column name in the ECInstanceIdColumn property in the ClassMap custom attribute, if it is not ECDb's default primary key column name.", primaryKeyColumnName, tableName);
                return nullptr;
                }

            systemColumn->SetKind (ColumnKind::ECInstanceId);
            if (!editMode)
                table->GetEditHandleR ().EndEdit ();
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
        if (entry.second->GetClassMapType() == IClassMap::Type::RelationshipEndTable ||
            entry.second->GetClassMapType() == IClassMap::Type::Unmapped)
            continue;

        ECDbSqlTable* primaryTable = &entry.second->GetPrimaryTable();
        ECDbSqlTable* joinedTable = &entry.second->GetJoinedTable();
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
        if (classMap.GetClassMapType() == IClassMap::Type::RelationshipEndTable)
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
        if (classMap == nullptr || classMap->GetClassMapType() != IClassMap::Type::RelationshipEndTable)
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

        bmap<ECDbSqlTable const*, bset<ClassMap const*>> constraintClassesPerTable;
        for (ClassMap const* constraintClassMap : constraintClassMaps)
            {
            //only non-abstract classes need to be considered as NOT NULL can be applied if base classes are all abstract
            //as there will not be rows for those base classes
            if (constraintClassMap->GetClass().GetClassModifier() == ECClassModifier::Abstract)
                continue;

            for (ECDbSqlTable const* table : constraintClassMap->GetTables())
                {
                constraintClassesPerTable[table].insert(constraintClassMap);
                }
            }

        LightweightCache const& lwc = GetLightweightCache();
        for (ECDbSqlTable const* fkTable : relClassMap.GetTables())
            {
            std::vector<ECClassId> allClassIds = lwc.GetNonAbstractClassesForTable(*fkTable);
            bset<ClassMap const*> const& constraintClassIds = constraintClassesPerTable[fkTable];

            ECDbSqlColumn const* fkColumn = relClassMap.GetReferencedEndECInstanceIdPropMap()->GetSingleColumn(*fkTable, true);
            if (fkColumn == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }

            //If FK column is a key property pointing to the ECInstanceId, it can already be NOT NULL
            if (fkColumn->GetConstraint().IsNotNull())
                continue;

            if (allClassIds.size() == constraintClassIds.size())
                {
                ECDbSqlColumn* fkColumnP = const_cast<ECDbSqlColumn*> (fkColumn);
                fkColumnP->GetConstraintR().SetIsNotNull(true);
                continue;
                }

            BeAssert(!fkColumn->GetConstraint().IsNotNull());
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

    BeBriefcaseId briefcaseId = GetECDb().GetBriefcaseId();
    const bool allowDbSchemaChange = briefcaseId.IsMasterId() || briefcaseId.IsStandaloneId();

    if (SUCCESS != EvaluateColumnNotNullConstraints())
        return ERROR;
    
    int nCreated = 0;
    int nUpdated = 0;
    int nSkipped = 0;

    //WIP_FOR_AFFAN: Why can't we use GetSQLManager().GetDbSchema().GetTables() instead
    //of creating a local bmap with class map information we don't need here?
    const ClassMapsByTable classMapsByTable = GetClassMapsByTable();
    for (bpair<ECDbSqlTable*, bset<ClassMap*>> const& kvPair : classMapsByTable)
        {
        ECDbSqlTable* table = kvPair.first;
        if (table->GetPersistenceType() == PersistenceType::Virtual || table->GetTableType() == TableType::Existing)
            continue;

        if (GetECDbR().TableExists(table->GetName().c_str()))
            {
            if (GetSQLManager().IsTableChanged(*table))
                {
                if (!allowDbSchemaChange)
                    {
                    GetECDbR().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECSchemas: Imported ECSchemas would change the database schema. "
                                                                  "This is only allowed for standalone briefcases or the master briefcase. Briefcase id: %" PRIu32, briefcaseId.GetValue());
                    return ERROR;
                    }

                if (table->GetPersistenceManager().CreateOrUpdate(GetECDbR()) != SUCCESS)
                    return ERROR;

                nUpdated++;
                }
            else
                nSkipped++;
            }
        else
            {
            if (!allowDbSchemaChange)
                {
                GetECDbR().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECSchemas: Imported ECSchemas would change the database schema. "
                                                                    "This is only allowed for standalone briefcases or the master briefcase. Briefcase id: %" PRIu32, briefcaseId.GetValue());
                return ERROR;
                }

            if (table->GetPersistenceManager().Create(GetECDbR()) != SUCCESS)
                return ERROR;

            nCreated++;
            }
        }

    timer.Stop();
    LOG.debugv("Created %d tables, skipped %d tables and updated %d table/view(s) in %.4f seconds", nCreated, nSkipped, nUpdated, timer.GetElapsedSeconds());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2016
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMap::CreateOrUpdateIndexesInDb() const
    {
    if (AssertIfIsNotImportingSchema())
        return ERROR;

    std::vector<ECDbSqlIndex const*> indexes;
    for (std::unique_ptr<ECDbSqlIndex> const& indexPtr : m_schemaImportContext->GetECDbMapDb().GetIndexes())
        {
        indexes.push_back(indexPtr.get());
        }

    ClassIndexInfoCache indexInfoCache (m_ecdb, *m_schemaImportContext);
    bmap<Utf8String, ECDbSqlIndex const*> comparableIndexDefs;
    for (ECDbSqlIndex const* index : indexes)
        {
        const ECClassId classId = index->GetClassId();
        if (ECClass::UNSET_ECCLASSID == classId)
            continue;

        ECDbSqlTable const& indexTable = index->GetTable();

        ClassMap const* classMap = GetClassMap(classId);
        if (classMap == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        StorageDescription const& storageDesc = classMap->GetStorageDescription();
        std::vector<Partition> const& horizPartitions = storageDesc.GetHorizontalPartitions();

        bvector<ClassIndexInfoPtr> const* baseClassIndexInfos = nullptr;
        if (SUCCESS != indexInfoCache.TryGetIndexInfos(baseClassIndexInfos, *classMap))
            return ERROR;

        BeAssert(baseClassIndexInfos != nullptr);

        for (Partition const& horizPartition : horizPartitions)
            {
            if (&indexTable == &horizPartition.GetTable())
                continue;

            bset<ECDbSqlTable const*> alreadyProcessedTables;
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

                ECDbSqlTable const& joinedOrSingleTable = derivedClassMap->GetJoinedTable();
                if (alreadyProcessedTables.find(&joinedOrSingleTable) != alreadyProcessedTables.end())
                    continue;

                bvector<ClassIndexInfoPtr> indexInfos;
                for (ClassIndexInfoPtr const& indexInfo : *baseClassIndexInfos)
                    {
                    Utf8String indexName;
                    if (!Utf8String::IsNullOrEmpty(indexInfo->GetName()))
                        indexName.append(indexInfo->GetName()).append("_").append(joinedOrSingleTable.GetName());

                    indexInfos.push_back(ClassIndexInfo::Clone(*indexInfo, indexName.c_str()));
                    }

                if (SUCCESS != derivedClassMap->CreateUserProvidedIndexes(*m_schemaImportContext, indexInfos))
                    return ERROR;

                alreadyProcessedTables.insert(&joinedOrSingleTable);
                }
            }
        }

    return m_schemaImportContext->GetECDbMapDb().CreateOrUpdateIndexesInDb(m_ecdb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbMap::FinishTableDefinition () const
    {
    AssertIfIsNotImportingSchema();
    const ClassMapsByTable classMapsByTable = GetClassMapsByTable();
    for (bpair<ECDbSqlTable*, bset<ClassMap*>> const& kvPair : classMapsByTable)
        {
        ECDbSqlTable* table = kvPair.first;
        bset<ClassMap*> const& classMapSet = kvPair.second;

        //Create ECClassId column if required
        if (table->GetFilteredColumnFirst(ColumnKind::ECClassId) == nullptr &&
            table->GetPersistenceType() == PersistenceType::Persisted &&
            table->GetTableType() != TableType::Existing)
            {
            bool addClassId = false;
            if (classMapSet.size() == 1)
                addClassId = (*classMapSet.begin())->GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::SharedTable;
            else
                addClassId = classMapSet.size() > 1;

            if (addClassId)
                {
                const size_t insertPosition = 1;
                ECDbSqlColumn * ecClassIdColumn = table->CreateColumn(ECDB_COL_ECClassId, ECDbSqlColumn::Type::Integer, insertPosition, ColumnKind::ECClassId, PersistenceType::Persisted);
                if (ecClassIdColumn == nullptr)
                    return ERROR;
                
                ecClassIdColumn->GetConstraintR().SetIsNotNull(true);
                //whenever we create a class id column, we index it to speed up the frequent class id look ups
                Utf8String indexName("ix_");
                indexName.append(table->GetName()).append("_ecclassid");
                m_schemaImportContext->GetECDbMapDb().CreateIndex(GetECDb(), *table, indexName.c_str(), false, {ecClassIdColumn}, false, true, ECClass::UNSET_ECCLASSID);
                }
            }
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<ECClassCP> ECDbMap::GetClassesFromRelationshipEnd (ECRelationshipConstraintCR relationshipEnd) const
    {
    //for recursive lambdas, iOS requires us to define the lambda variable before assigning the actual function to it.
    std::function<void (std::vector<ECClassCP>&, ECClassP, bool)> gatherClassesDelegate;
    gatherClassesDelegate =
        [this, &gatherClassesDelegate] (std::vector<ECClassCP>& classes, ECClassP ecClass, bool includeSubclasses)
        {
        classes.push_back (ecClass);
        if (includeSubclasses)
            {
            for (auto childClass : ecClass->GetDerivedClasses ())
                { 
                gatherClassesDelegate (classes, childClass, includeSubclasses);
                }
            }
        };

    bool isPolymorphic = relationshipEnd.GetIsPolymorphic();
    std::vector<ECClassCP> classes;
    for (ECClassCP ecClass : relationshipEnd.GetClasses ())
        {
        ECClassP ecClassP = const_cast<ECClassP> (ecClass);
        gatherClassesDelegate (classes, ecClassP, isPolymorphic);
        }
    
    return std::move (classes);
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

    std::map<PersistenceType, std::set<ECDbSqlTable const*>> tables;
    bool abstractEndPoint = relationshipEnd.GetClasses().size() == 1 && relationshipEnd.GetClasses().front()->GetClassModifier() == ECClassModifier::Abstract;
    std::vector<ECClassCP> classes = GetClassesFromRelationshipEnd(relationshipEnd);
    for (IClassMap const* classMap : classMaps)
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
        if (IClassMap::IsAnyClass(*ecClass))
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

    return std::move(classMaps);
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
std::set<ECDbSqlTable const*> ECDbMap::GetTablesFromRelationshipEnd(ECRelationshipConstraintCR relationshipEnd, EndTablesOptimizationOptions options) const
    {
    BeAssert(options != EndTablesOptimizationOptions::Skip);

    bool hasAnyClass = false;
    std::set<ClassMap const*> classMaps = GetClassMapsFromRelationshipEnd(relationshipEnd, &hasAnyClass);

    if (hasAnyClass)
        return std::set<ECDbSqlTable const*>();

    std::map<ECDbSqlTable const*, std::set<ECDbSqlTable const*>> joinedTables;
    std::set<ECDbSqlTable const*> tables;
    for (IClassMap const* classMap : classMaps)
        {
        IClassMap::TableListR classPersistInTables = classMap->GetTables();
        if (classPersistInTables.size() == 1)
            {
            tables.insert(classPersistInTables.front());
            }
        else
            {
            for (ECDbSqlTable const* table : classPersistInTables)
                {
                if (auto baseTable = table->GetParentOfJoinedTable())
                    {
                    joinedTables[baseTable].insert(table);
                    tables.insert(table);
                    }
                }
            }
        }

    for (auto const& pair : joinedTables)
        {
        ECDbSqlTable const* baseTable = pair.first;
        std::set<ECDbSqlTable const*> const& childTables = pair.second;
        bool isBaseTableSelected = tables.find(baseTable) != tables.end();
        if (options == EndTablesOptimizationOptions::ReferencedEnd)
            {
            for (auto childTable : baseTable->GetChildTables())
                tables.erase(childTable);

            tables.insert(baseTable);
            }
        else
            {
            if (isBaseTableSelected)
                {
                for (auto childTable : childTables)
                    tables.erase(childTable);
                }
            }
        }

    if (options == EndTablesOptimizationOptions::ForeignEnd)
        return tables;

    std::map<PersistenceType, std::set<ECDbSqlTable const*>> finalListOfTables;
    for (ECDbSqlTable const* table : tables)
        {
        finalListOfTables[table->GetPersistenceType()].insert(table);
        }


    return finalListOfTables[PersistenceType::Persisted];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2015
//+---------------+---------------+---------------+---------------+---------------+------
std::set<ECDbSqlTable const*> ECDbMap::GetTablesFromRelationshipEndWithColumn(ECRelationshipConstraintCR relationshipEnd, Utf8CP column) const
    {
    bool hasAnyClass = false;
    std::set<ClassMap const*> classMaps = GetClassMapsFromRelationshipEnd(relationshipEnd, &hasAnyClass);

    if (hasAnyClass)
        return std::set<ECDbSqlTable const*>();

    std::map<PersistenceType, std::set<ECDbSqlTable const*>> tables;
    std::vector<ECClassCP> classes = GetClassesFromRelationshipEnd(relationshipEnd);
    for (IClassMap const* classMap : classMaps)
        {
        ECDbSqlTable const* table = classMap->GetPrimaryTable().FindColumnCP(column) != nullptr ? &classMap->GetPrimaryTable() : nullptr;
        if (table == nullptr && !classMap->IsMappedToSingleTable())
            table = classMap->GetJoinedTable().FindColumnCP(column) != nullptr ? &classMap->GetJoinedTable() : nullptr;

        if (table)
            tables[table->GetPersistenceType()].insert(table);
        }


    if (tables[PersistenceType::Persisted].size() > 0)
        return tables[PersistenceType::Persisted];

    return tables[PersistenceType::Virtual];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbSqlTable const* ECDbMap::GetFirstTableFromRelationshipEnd(ECRelationshipConstraintCR relationshipEnd) const
    {
    std::map<PersistenceType, std::set<ECDbSqlTable const*>> tables;
    bool hasAnyClass;
    std::set<ClassMap const*> classMaps = GetClassMapsFromRelationshipEnd(relationshipEnd, &hasAnyClass);

    if (hasAnyClass)
        return nullptr;
        
    for (ClassMap const* classMap : classMaps)
        {
        tables[classMap->GetJoinedTable().GetPersistenceType()].insert(&classMap->GetJoinedTable());
        }

    std::set<ECDbSqlTable const*>& persistedTables = tables[PersistenceType::Persisted];
    std::set<ECDbSqlTable const*>& virtualTables = tables[PersistenceType::Virtual];

    if (persistedTables.size() > 0)
        {
        return *persistedTables.begin();
        }

    if (tables[PersistenceType::Virtual].size() > 0)
        return *virtualTables.begin();

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan Khan                          08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbMap::ClearCache()
    {
    BeMutexHolder lock(m_mutex);
    m_classMapDictionary.clear();
    GetSQLManager().Reset();
    m_lightweightCache.Reset();
    }

/*---------------------------------------------------------------------------------**//**
* Save map
* @bsimethod                                 Affan Khan                          08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbMap::SaveMappings() const
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
            if (SUCCESS != classMap.Save (doneList))
                {
                m_ecdb.GetECDbImplR().GetIssueReporter().Report (ECDbIssueSeverity::Error, "Failed to save mapping for ECClass %s: %s", ecClass.GetFullName(), m_ecdb.GetLastError().c_str());
                return ERROR;
                }
            }
        }

    if (SUCCESS != GetSQLManager().Save())
        return ERROR;

    m_lightweightCache.Reset();
    stopWatch.Stop();

    LOG.debugv("Saving ECDbMap for %d ECClasses took %.4lf msecs.", i, stopWatch.GetElapsedSeconds() * 1000.0);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    10/2015
//---------------------------------------------------------------------------------------
void ECDbMap::ParsePropertyAccessString(bvector<Utf8String>& tokens, Utf8CP propertyAccessString)
    {
    BeStringUtilities::Split(propertyAccessString, ".", nullptr, tokens);
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
    sql.Sprintf("SELECT t.Id, t.Name, c.Id, c.Modifier FROM ec_PropertyMap pm "
                "JOIN ec_Column col ON col.Id = pm.ColumnId AND (col.ColumnKind & %d = 0) "
                "JOIN ec_PropertyPath pp ON pp.Id = pm.PropertyPathId "
                "JOIN ec_ClassMap cm ON cm.Id = pm.ClassMapId "
                "JOIN ec_Class c ON c.Id = cm.ClassId "
                "JOIN ec_Table t ON t.Id = col.TableId "
                "WHERE cm.MapStrategy<>%d AND cm.MapStrategy<>%d "
                "GROUP BY t.Id, c.Id", Enum::ToInt(ColumnKind::ECClassId),
                Enum::ToInt(ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable),
                Enum::ToInt(ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable));

    CachedStatementPtr stmt = m_map.GetECDbR().GetCachedStatement(sql.c_str());
    ECDbTableId currentTableId = -1;
    ECDbSqlTable const* currentTable = nullptr;
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECDbTableId tableId = stmt->GetValueInt64(0);
        if (currentTableId != tableId)
            {
            Utf8CP tableName = stmt->GetValueText(1);
            currentTable = m_map.GetSQLManager().GetDbSchema().FindTable(tableName);
            currentTableId = tableId;
            BeAssert(currentTable != nullptr);
            }

        ECClassId id = stmt->GetValueInt64(2);
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
void ECDbMap::LightweightCache::LoadAnyClassRelationships () const
    {
    if (m_loadedFlags.m_anyClassRelationshipsIsLoaded)
        return;

    Utf8CP sql1 =
        "SELECT RCC.RelationshipClassId, RCC.RelationshipEnd FROM ec_RelationshipConstraintClass RCC "
        "WHERE RCC.ClassId IN (SELECT Id FROM ec_Class WHERE Name = 'AnyClass')";

    auto stmt1 = m_map.GetECDbR ().GetCachedStatement (sql1);
    while (stmt1->Step () == BE_SQLITE_ROW)
        {
        ECClassId id = stmt1->GetValueInt64 (0);
        RelationshipEnd filter = stmt1->GetValueInt (1) == 0 ? RelationshipEnd::Source : RelationshipEnd::Target;

        auto itor = m_anyClassRelationships.find (id);
        if (itor == m_anyClassRelationships.end ())
            m_anyClassRelationships.insert (make_bpair (id, filter));
        else
            {
            m_anyClassRelationships[id] = static_cast<RelationshipEnd>((int)(itor->second) & (int)(filter));
            }
        }

    m_loadedFlags.m_anyClassRelationshipsIsLoaded = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::LoadAnyClassReplacements () const
    {
    if (m_loadedFlags.m_anyClassReplacementsLoaded)
        return;

    Utf8String sql;
    sql.Sprintf("SELECT ec_Class.Id  FROM ec_PropertyMap "
        "JOIN ec_Column ON ec_Column.Id = ec_PropertyMap.ColumnId "
        "JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.PropertyPathId "
        "JOIN ec_ClassMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId "
        "JOIN ec_Class ON ec_Class.Id = ec_ClassMap.ClassId  "
        "JOIN ec_Table ON ec_Table.Id = ec_Column.TableId "
        "WHERE ec_ClassMap.MapStrategy <> 0 AND ec_Class.Type <> %d AND ec_Table.IsVirtual = 0 "
        "GROUP BY ec_Class.Id", Enum::ToInt(ECN::ECClassType::Relationship));


    auto stmt1 = m_map.GetECDbR ().GetCachedStatement (sql.c_str());
    while (stmt1->Step () == BE_SQLITE_ROW)
        {
        ECClassId id = stmt1->GetValueInt64 (0);
        m_anyClassReplacements.push_back (id);
        }

    m_loadedFlags.m_anyClassReplacementsLoaded = true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::LoadRelationshipCache () const
    {
    if (m_loadedFlags.m_relationshipCacheIsLoaded)
        return;

    Utf8CP sql0 =
        "WITH RECURSIVE DerivedClassList(RelationshipClassId, RelationshipEnd, IsPolymorphic, CurrentClassId, DerivedClassId) "
        "AS (SELECT RCC.RelationshipClassId,RCC.RelationshipEnd,RC.IsPolymorphic,RCC.ClassId,RCC.ClassId "
        "FROM ec_RelationshipConstraintClass RCC INNER JOIN ec_RelationshipConstraint RC ON RC.RelationshipClassId = RCC.RelationshipClassId AND RC.RelationshipEnd = RCC.RelationshipEnd "
        "UNION "
        "SELECT DCL.RelationshipClassId, DCL.RelationshipEnd, DCL.IsPolymorphic, BC.BaseClassId, BC.ClassId "
        "FROM DerivedClassList DCL INNER JOIN ec_BaseClass BC ON BC.BaseClassId = DCL.DerivedClassId "
        "WHERE IsPolymorphic = 1) "
        "SELECT DerivedClassId, RelationshipClassId, RelationshipEnd FROM DerivedClassList";

    auto stmt0 = m_map.GetECDbR ().GetCachedStatement (sql0);
    while (stmt0->Step () == BE_SQLITE_ROW)
        {
        ECClassId constraintClassId = stmt0->GetValueInt64 (0);
        ECClassId relationshipId = stmt0->GetValueInt64 (1);
        BeAssert (!stmt0->IsColumnNull (2));
        RelationshipEnd end = stmt0->GetValueInt (2) == 0 ? RelationshipEnd::Source : RelationshipEnd::Target;

        RelationshipClassIds& relClassIds = m_relationshipClassIdsPerConstraintClassIds[constraintClassId];
        auto relIt = relClassIds.find (relationshipId);
        if (relIt == relClassIds.end ())
            relClassIds[relationshipId] = end;
        else
            relClassIds[relationshipId] = static_cast<RelationshipEnd>((int)(relIt->second) | (int)(end));


        ConstraintClassIds& constraintClassIds = m_nonAnyClassConstraintClassIdsPerRelClassIds[relationshipId];
        auto constraintIt = constraintClassIds.find (constraintClassId);
        if (constraintIt == constraintClassIds.end ())
            constraintClassIds[constraintClassId] = end;
        else
            constraintClassIds[constraintClassId] = static_cast<RelationshipEnd>((int)(constraintIt->second) | (int)(end));
        }

    m_relationshipEndsByClassIdRev = m_nonAnyClassConstraintClassIdsPerRelClassIds;
    LoadAnyClassRelationships ();
    LoadAnyClassReplacements ();
    for (ECClassId constraintClassId : m_anyClassReplacements)
        {
        RelationshipClassIds& rels = m_relationshipClassIdsPerConstraintClassIds[constraintClassId];
        for (bpair<ECClassId, RelationshipEnd> const& pair : m_anyClassRelationships)
            {
            ECClassId relId = pair.first;
            RelationshipEnd end = pair.second;
            auto relsIt = rels.find (relId);
            if (relsIt == rels.end ())
                rels[relId] = end;
            else
                rels[relId] = static_cast<RelationshipEnd>((int)(relsIt->second) | (int)(end));
            }
        }
    for (auto& pair1 : m_anyClassRelationships)
        {
        auto& classes = m_relationshipEndsByClassIdRev[pair1.first];
        for (auto id : m_anyClassReplacements)
            {
            //ECClassId id = pair1.first;
            auto itor1 = classes.find (id);
            if (itor1 == classes.end ())
                {
                classes[id] = pair1.second;
                }
            else
                {
                classes[id] = static_cast<RelationshipEnd>((int)(itor1->second) | (int)(pair1.second));
                }
            }
        }

    m_loadedFlags.m_relationshipCacheIsLoaded = true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::LoadHorizontalPartitions ()  const
    {
    if (m_loadedFlags.m_horizontalPartitionsIsLoaded)
        return;

    auto anyClassId = GetAnyClassId ();
    Utf8String sql;
    sql.Sprintf(
        "WITH RECURSIVE DerivedClassList(RootClassId,CurrentClassId,DerivedClassId) "
        "AS (SELECT Id, Id, Id FROM ec_Class "
        "UNION "
        "SELECT RootClassId, BC.BaseClassId, BC.ClassId FROM DerivedClassList DCL "
        "INNER JOIN ec_BaseClass BC ON BC.BaseClassId = DCL.DerivedClassId), "
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
        Enum::ToInt(ColumnKind::ECClassId), Enum::ToInt(ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable),
        Enum::ToInt(ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable), Enum::ToInt(TableType::Joined));

    CachedStatementPtr stmt = m_map.GetECDbR ().GetCachedStatement (sql.c_str());
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId rootClassId = stmt->GetValueInt64(0);
        if (anyClassId == rootClassId)
            continue;

        ECClassId derivedClassId = stmt->GetValueInt64(1);

        Utf8CP tableName = stmt->GetValueText(2);
        ECDbSqlTable const* table = m_map.GetSQLManager().GetDbSchema().FindTable(tableName);
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
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::LoadRelationshipByTable()  const
    {
    if (m_loadedFlags.m_relationshipPerTableLoaded)
        return;

    Utf8String sql;
    sql.Sprintf("SELECT DISTINCT ec_Class.Id, ec_Table.Name, ec_ClassMap.MapStrategy FROM ec_Column "
                "INNER JOIN ec_Table ON ec_Table.Id = ec_Column.TableId "
                "INNER JOIN ec_PropertyMap ON  ec_PropertyMap.ColumnId = ec_Column.Id "
                "INNER JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.PropertyPathId "
                "INNER JOIN ec_Property ON ec_PropertyPath.RootPropertyId = ec_Property.Id "
                "INNER JOIN ec_ClassMap ON ec_PropertyMap.ClassMapId = ec_ClassMap.Id "
                "INNER JOIN ec_Class ON ec_Class.Id = ec_ClassMap.ClassId "
                "WHERE ec_ClassMap.MapStrategy  <> 0 AND "
                "(ec_Column.ColumnKind & %d = 0) AND (ec_Column.ColumnKind & %d = 0) AND "
                "ec_Class.Type=%d AND ec_Table.IsVirtual = 0",
                Enum::ToInt(ColumnKind::ECInstanceId),
                Enum::ToInt(ColumnKind::ECClassId),
                Enum::ToInt(ECClassType::Relationship));

    auto stmt = m_map.GetECDbR().GetCachedStatement(sql.c_str());
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        auto relationshipClassId = stmt->GetValueInt64(0);
        Utf8CP tableName = stmt->GetValueText(1);
        RelationshipType type = RelationshipType::Link;
        if (stmt->GetValueInt(2) == Enum::ToInt(RelationshipType::Source))
            type = RelationshipType::Source;
        else if (stmt->GetValueInt(2) == Enum::ToInt(RelationshipType::Target))
            type = RelationshipType::Target;

        auto table = m_map.GetSQLManager().GetDbSchema().FindTable(tableName);
        BeAssert(table != nullptr);
        m_relationshipPerTable[table][relationshipClassId] = type;
        }

    m_loadedFlags.m_relationshipPerTableLoaded = true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//--------------------------------------------------------------------------------------
ECN::ECClassId ECDbMap::LightweightCache::GetAnyClassId () const
    {
    if (m_anyClassId == ECClass::UNSET_ECCLASSID)
        {
        auto stmt = m_map.GetECDbR ().GetCachedStatement ("SELECT ec_Class.Id FROM ec_Class INNER JOIN ec_Schema ON ec_Schema.Id = ec_Class.SchemaId WHERE ec_Class.Name = 'AnyClass' AND ec_Schema.Name = 'Bentley_Standard_Classes'");
        if (stmt->Step () == BE_SQLITE_ROW)
            m_anyClassId = stmt->GetValueInt64 (0);
        }

    return m_anyClassId;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::RelationshipClassIds const& ECDbMap::LightweightCache::GetRelationshipsForConstraintClass(ECN::ECClassId constraintClassId) const
    {
    LoadRelationshipCache ();
    return m_relationshipClassIdsPerConstraintClassIds[constraintClassId];
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::RelationshipClassIds const& ECDbMap::LightweightCache::GetRelationships (ECN::ECClassId relationshipId) const
    {
    LoadRelationshipCache ();
    return m_relationshipEndsByClassIdRev[relationshipId];
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 08/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::ConstraintClassIds const& ECDbMap::LightweightCache::GetConstraintClassesForRelationship(ECN::ECClassId relClassId) const
    {
    LoadRelationshipCache();
    return m_nonAnyClassConstraintClassIdsPerRelClassIds[relClassId];
   }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::RelationshipClassIds const& ECDbMap::LightweightCache::GetAnyClassRelationships() const
    {
    LoadAnyClassRelationships ();
    return m_anyClassRelationships;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::RelationshipTypeByClassId ECDbMap::LightweightCache::GetRelationshipsMapToTable (ECDbSqlTable const& table) const
    {
    LoadRelationshipByTable ();
    return m_relationshipPerTable[&table];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::RelationshipPerTable ECDbMap::LightweightCache::GetRelationshipsMapToTables () const
    {
    LoadRelationshipByTable ();
    return m_relationshipPerTable;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
std::vector<ECClassId> const& ECDbMap::LightweightCache::GetClassesForTable (ECDbSqlTable const& table) const
    {
    LoadClassIdsPerTable ();
    return m_classIdsPerTable[&table];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 05/2016
//---------------------------------------------------------------------------------------
std::vector<ECClassId> const& ECDbMap::LightweightCache::GetNonAbstractClassesForTable(ECDbSqlTable const& table) const
    {
    LoadClassIdsPerTable();
    return m_nonAbstractClassIdsPerTable[&table];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
std::vector<ECClassId> const& ECDbMap::LightweightCache::GetAnyClassReplacements() const
    {
    LoadAnyClassReplacements ();
    return m_anyClassReplacements;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
bset<ECDbSqlTable const*> const& ECDbMap::LightweightCache::GetVerticalPartitionsForClass(ECN::ECClassId classId) const
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
void ECDbMap::LightweightCache::Load(bool forceReload)
    {
    if (forceReload)
        Reset();

    LoadAnyClassRelationships();
    LoadRelationshipCache();
    LoadClassIdsPerTable();
    LoadHorizontalPartitions();
    LoadAnyClassReplacements();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::Reset ()
    {
    m_loadedFlags.m_classIdsPerTableIsLoaded = 
        m_loadedFlags.m_relationshipPerTableLoaded =
        m_loadedFlags.m_horizontalPartitionsIsLoaded =
        m_loadedFlags.m_relationshipCacheIsLoaded =
        m_loadedFlags.m_anyClassReplacementsLoaded = 
        m_loadedFlags.m_anyClassRelationshipsIsLoaded = false;

    m_anyClassId = ECClass::UNSET_ECCLASSID;
    m_relationshipEndsByClassIdRev.clear ();
    m_horizontalPartitions.clear();
    m_classIdsPerTable.clear();
    m_nonAbstractClassIdsPerTable.clear();
    m_relationshipClassIdsPerConstraintClassIds.clear();
    m_nonAnyClassConstraintClassIdsPerRelClassIds.clear();
    m_anyClassRelationships.clear ();
    m_anyClassReplacements.clear ();
    m_storageDescriptions.clear ();
    m_relationshipPerTable.clear ();   
    m_tablesPerClassId.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::LightweightCache (ECDbMapCR map) : m_map(map) { Reset (); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
StorageDescription const& ECDbMap::LightweightCache::GetStorageDescription (IClassMap const& classMap)  const
    {
    const ECClassId classId = classMap.GetClass().GetId();
    auto it = m_storageDescriptions.find(classId);
    if (it == m_storageDescriptions.end())
        {
        auto des = StorageDescription::Create(classMap, *this);
        auto desP = des.get ();
        m_storageDescriptions[classId] = std::move(des);
        return *desP;
        }

    return *(it->second.get());
    }


//****************************************************************************************
// StorageDescription
//****************************************************************************************
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
StorageDescription::StorageDescription(StorageDescription&& rhs)
    : m_classId(std::move(rhs.m_classId)), m_horizontalPartitions(std::move(rhs.m_horizontalPartitions)),
    m_nonVirtualHorizontalPartitionIndices(std::move(rhs.m_nonVirtualHorizontalPartitionIndices)),
    m_rootHorizontalPartitionIndex(std::move(rhs.m_rootHorizontalPartitionIndex)),
    m_rootVerticalPartitionIndex(std::move(rhs.m_rootVerticalPartitionIndex)), m_verticalPartitions(std::move(rhs.m_verticalPartitions))
    {}

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
StorageDescription& StorageDescription::operator=(StorageDescription&& rhs)
    {
    if (this != &rhs)
        {
        m_classId = std::move(rhs.m_classId);
        m_horizontalPartitions = std::move(rhs.m_horizontalPartitions);
        m_nonVirtualHorizontalPartitionIndices = std::move(rhs.m_nonVirtualHorizontalPartitionIndices);
        m_rootHorizontalPartitionIndex = std::move(rhs.m_rootHorizontalPartitionIndex);
        m_rootVerticalPartitionIndex = std::move(rhs.m_rootVerticalPartitionIndex);
        m_verticalPartitions = std::move(rhs.m_verticalPartitions);
        }

    return *this;
    }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    10 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus StorageDescription::GenerateECClassIdFilter(NativeSqlBuilder& filter, ECDbSqlTable const& table, ECDbSqlColumn const& classIdColumn, bool polymorphic, bool fullyQualifyColumnName, Utf8CP tableAlias) const
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

    NativeSqlBuilder classIdColSql;
    if (fullyQualifyColumnName)
        {
        if (tableAlias)
            classIdColSql.AppendEscaped(tableAlias).AppendDot();
        else
            classIdColSql.AppendEscaped(table.GetName().c_str()).AppendDot();
        }
    classIdColSql.Append(classIdColumn.GetName().c_str(), false);

    if (!polymorphic)
        {
        //if partition's table is only used by a single class, no filter needed     
        if (partition->IsSharedTable())
            filter.Append(classIdColSql, false).Append(BooleanSqlOperator::EqualTo, false).Append(m_classId);

        return SUCCESS;
        }

    partition->AppendECClassIdFilterSql(classIdColSql.ToString(), filter);
    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan    05 / 2015
//------------------------------------------------------------------------------------------
//static
std::unique_ptr<StorageDescription> StorageDescription::Create(IClassMap const& classMap, ECDbMap::LightweightCache const& lwmc)
    {
    const ECClassId classId = classMap.GetClass().GetId();
    std::unique_ptr<StorageDescription> storageDescription = std::unique_ptr<StorageDescription>(new StorageDescription(classId));
    std::set<ECClassId> derviedClassSet;
    if (classMap.GetClassMapType() == IClassMap::Type::RelationshipEndTable)
        {
        RelationshipClassEndTableMap const& relClassMap = static_cast<RelationshipClassEndTableMap const&> (classMap);
        for (ECDbSqlTable const* endTable : relClassMap.GetTables())
            {
            const ECDbMap::LightweightCache::RelationshipEnd foreignEnd = relClassMap.GetForeignEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? ECDbMap::LightweightCache::RelationshipEnd::Source : ECDbMap::LightweightCache::RelationshipEnd::Target;

            Partition* hp = storageDescription->AddHorizontalPartition(*endTable, true);

            for (bpair<ECClassId, ECDbMap::LightweightCache::RelationshipEnd> const& kvpair : lwmc.GetConstraintClassesForRelationship(classId))
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

    return std::move(storageDescription);
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
Partition const* StorageDescription::GetHorizontalPartition(ECDbSqlTable const& table) const
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
Partition const* StorageDescription::GetVerticalPartition(ECDbSqlTable const& table) const
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
//@bsimethod                                                    Affan.Khan    11 / 2015
//------------------------------------------------------------------------------------------
Partition const& StorageDescription::GetRootVerticalPartition() const
    {
    BeAssert(m_rootVerticalPartitionIndex < m_verticalPartitions.size());
    return m_verticalPartitions[m_rootVerticalPartitionIndex];
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
Partition* StorageDescription::AddHorizontalPartition(ECDbSqlTable const& table, bool isRootPartition)
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
Partition* StorageDescription::AddVerticalPartition(ECDbSqlTable const& table, bool isRootPartition)
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
Partition& Partition::operator=(Partition&& rhs)
    {
    if (this != &rhs)
        {
        m_table = std::move(rhs.m_table);
        m_partitionClassIds = std::move(rhs.m_partitionClassIds);
        m_inversedPartitionClassIds = std::move(rhs.m_inversedPartitionClassIds);
        m_hasInversedPartitionClassIds = std::move(rhs.m_hasInversedPartitionClassIds);

        //nulling out the RHS m_table pointer is defensive, even if the destructor doesn't
        //free the table (as it is now owned by Partition). If the ownership ever changes,
        //this method is safe.
        rhs.m_table = nullptr;
        }

    return *this;
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
void Partition::AppendECClassIdFilterSql(Utf8CP classIdColName, NativeSqlBuilder& sqlBuilder) const
    {
    BeAssert(!m_partitionClassIds.empty());

    std::vector<ECClassId> const* classIds = nullptr;
    BooleanSqlOperator equalOp, setOp;
    if (m_hasInversedPartitionClassIds)
        {
        classIds = &m_inversedPartitionClassIds;
        if (classIds->empty())
            return; //no filter needed, as all class ids should be considered

        equalOp = BooleanSqlOperator::NotEqualTo;
        setOp = BooleanSqlOperator::And;
        }
    else
        {
        classIds = &m_partitionClassIds;
        equalOp = BooleanSqlOperator::EqualTo;
        setOp = BooleanSqlOperator::Or;
        }

    bool isFirstItem = true;
    for (ECClassId classId : *classIds)
        {
        if (!isFirstItem)
            sqlBuilder.AppendSpace().Append(setOp, true);

        sqlBuilder.Append(classIdColName).Append(equalOp, false).Append(classId);

        isFirstItem = false;
        }
    }

#define ECDB_HOLDING_VIEW "ec_RelationshipHoldingStatistics"
#define ECDB_HOLDING_VIEW_HELDID_COLNAME "HeldECInstanceId"

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan     01/2016
//---------------+---------------+---------------+---------------+---------------+---------
void RelationshipPurger::Finalize()
    {
    for (auto& stmt : m_stmts)
        {
        stmt->Finalize();
        }

    m_stmts.clear();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan     01/2016
//---------------+---------------+---------------+---------------+---------------+---------
BentleyStatus RelationshipPurger::Initialize(ECDbR ecdb)
    {
    if (ecdb.IsReadonly())
        return ERROR;

    if (!m_stmts.empty())
        return SUCCESS; //already initialized

    Utf8String sql;
    sql.Sprintf("SELECT C.Id FROM ec_Class C INNER JOIN ec_ClassMap CM ON CM.ClassId=C.Id WHERE C.RelationshipStrength=%d AND CM.MapStrategy!=%d",
                Enum::ToInt(StrengthType::Holding), Enum::ToInt(ECDbMapStrategy::Strategy::NotMapped));

    CachedStatementPtr stmt = ecdb.GetCachedStatement(sql.c_str());
    if (stmt == nullptr)
        return ERROR;

    ECDbMapCR map = ecdb.GetECDbImplR().GetECDbMap();
    std::vector<RelationshipClassMapCP> relationships;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId id = stmt->GetValueInt64(0);
        if (RelationshipClassMapCP relationshipClassMap = static_cast<RelationshipClassMapCP>(map.GetClassMap(id)))
            relationships.push_back(relationshipClassMap);
        }

    //reset stmt so that it doesn't hold resources
    stmt = nullptr;

    SqlPerTableMap deleteOrphansSqlPerTableMap;
    NativeSqlBuilder::List unionClauseList;
    bmap<ECDbSqlTable const*, ECDbMap::LightweightCache::RelationshipEnd> linkTables;
    for (RelationshipClassMapCP relClassMap : relationships)
        {
        ECRelationshipClassCR relClass = relClassMap->GetRelationshipClass();
        ECRelationshipConstraintCR heldConstraint = relClass.GetStrengthDirection() == ECRelatedInstanceDirection::Forward ? relClass.GetTarget() : relClass.GetSource();

        std::set<ECDbSqlTable const*> heldTables = map.GetTablesFromRelationshipEnd(heldConstraint, EndTablesOptimizationOptions::ForeignEnd);
        for (ECDbSqlTable const* table : heldTables)
            {
            ECDbSqlTable const* heldTable = table->GetParentOfJoinedTable() != nullptr ? table->GetParentOfJoinedTable() : table;
            Utf8CP heldTableName = heldTable->GetName().c_str();
            auto itor = deleteOrphansSqlPerTableMap.find(heldTableName);
            if (itor == deleteOrphansSqlPerTableMap.end())
                {
                ECDbSqlColumn const* idCol = heldTable->GetFilteredColumnFirst(ColumnKind::ECInstanceId);
                deleteOrphansSqlPerTableMap[heldTableName] = BuildSql(heldTableName, idCol->GetName().c_str());
                }
            }

        if (relClassMap->GetClassMapType() == IClassMap::Type::RelationshipEndTable)
            {
            RelationshipClassEndTableMapCP endTableRelClassMap = static_cast<RelationshipClassEndTableMapCP>(relClassMap);
            PropertyMapCP foreignIdPropMap = endTableRelClassMap->GetConstraintECInstanceIdPropMap(endTableRelClassMap->GetReferencedEnd());

            ColumnKind foreignIdColKind = endTableRelClassMap->GetReferencedEnd() == ECRelationshipEnd_Source ? ColumnKind::SourceECInstanceId : ColumnKind::TargetECInstanceId;
            NativeSqlBuilder unionClauseBuilder;
            for (ECDbSqlTable const* table : foreignIdPropMap->GetTables())
                {
                ECDbSqlColumn const* foreignIdCol = table->GetFilteredColumnFirst(foreignIdColKind);
                if (foreignIdCol == nullptr)
                    {
                    BeAssert(false);
                    return ERROR;
                    }

                unionClauseBuilder.AppendFormatted("SELECT [%s] " ECDB_HOLDING_VIEW_HELDID_COLNAME " FROM [%s] WHERE [%s] IS NOT NULL", foreignIdCol->GetName().c_str(), table->GetName().c_str(), foreignIdCol->GetName().c_str());
                }

            if (!unionClauseBuilder.IsEmpty())
                unionClauseList.push_back(std::move(unionClauseBuilder));
            }
        else
            {
            ECDbSqlTable const& primaryTable = static_cast<RelationshipClassLinkTableMapCP>(relClassMap)->GetPrimaryTable();

            ECDbMap::LightweightCache::RelationshipEnd heldEnd = relClass.GetStrengthDirection() == ECRelatedInstanceDirection::Forward ? ECDbMap::LightweightCache::RelationshipEnd::Target : ECDbMap::LightweightCache::RelationshipEnd::Source;
            auto it = linkTables.find(&primaryTable);
            if (it == linkTables.end())
                linkTables[&primaryTable] = heldEnd;
            else
                {
                if (Enum::Contains(it->second, heldEnd))
                    continue;

                linkTables[&primaryTable] = Enum::Or(linkTables[&primaryTable], heldEnd);
                }

            PropertyMapCP heldConstraintIdPropMap = relClass.GetStrengthDirection() == ECRelatedInstanceDirection::Forward ? relClassMap->GetTargetECInstanceIdPropMap() : relClassMap->GetSourceECInstanceIdPropMap();
            ECDbSqlColumn const* heldIdCol = heldConstraintIdPropMap->GetSingleColumn();
            if (heldIdCol == nullptr)
                return ERROR;

            NativeSqlBuilder unionClauseBuilder;
            unionClauseBuilder.AppendFormatted("SELECT [%s] " ECDB_HOLDING_VIEW_HELDID_COLNAME " FROM [%s]", heldIdCol->GetName().c_str(), primaryTable.GetName().c_str());
            unionClauseList.push_back(std::move(unionClauseBuilder));
            }
        }

    NativeSqlBuilder holdingViewBuilder("DROP VIEW IF EXISTS " ECDB_HOLDING_VIEW "; CREATE VIEW " ECDB_HOLDING_VIEW " AS ");
    if (!unionClauseList.empty())
        {
        bool isFirstItem = true;
        for (NativeSqlBuilder const& unionClause : unionClauseList)
            {
            if (!isFirstItem)
                holdingViewBuilder.Append(" UNION ALL ");

            holdingViewBuilder.Append(unionClause);
            isFirstItem = false;
            }

        holdingViewBuilder.Append(";");
        }
    else
        holdingViewBuilder.Append("SELECT NULL " ECDB_HOLDING_VIEW_HELDID_COLNAME " LIMIT 0;");

    if (BE_SQLITE_OK != ecdb.ExecuteSql(holdingViewBuilder.ToString()))
        {
        BeAssert(false && "Failed to create holding view");
        return ERROR;
        }


    for (bpair<Utf8CP,Utf8String> const& kvPair : deleteOrphansSqlPerTableMap)
        {
        std::unique_ptr<Statement> stmt = std::unique_ptr<Statement>(new Statement());
        if (BE_SQLITE_OK != stmt->Prepare(ecdb, kvPair.second.c_str()))
            {
            BeAssert(false && "Failed to prepare sql");
            return ERROR;
            }

        m_stmts.push_back(std::move(stmt));
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan     01/2016
//---------------+---------------+---------------+---------------+---------------+---------
BentleyStatus RelationshipPurger::Purge(ECDbR ecdb)
    {
    if (Initialize(ecdb) != SUCCESS)
        return ERROR;

    for (std::unique_ptr<Statement>& stmt : m_stmts)
        {
        if (BE_SQLITE_DONE != stmt->Step())
            return ERROR;
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan     01/2016
//---------------+---------------+---------------+---------------+---------------+---------
//static
Utf8String RelationshipPurger::BuildSql(Utf8CP tableName, Utf8CP pkColumnName)
    {
    Utf8String str;
    str.Sprintf("DELETE FROM [%s] WHERE [%s] NOT IN (SELECT " ECDB_HOLDING_VIEW_HELDID_COLNAME " FROM " ECDB_HOLDING_VIEW ")", tableName, pkColumnName);
    return str;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE

