/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaImportContext.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SchemaImportContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************************************************************************
// SchemaImportContext
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
BentleyStatus SchemaImportContext::Initialize(ECDbSQLManager const& dbSchema, ECDbCR ecdb)
    {
    m_ecdbMapDb = std::unique_ptr<SchemaImportECDbMapDb>(new SchemaImportECDbMapDb(dbSchema.GetDbSchemaR()));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
UserECDbMapStrategy const* SchemaImportContext::GetUserStrategy(ECClassCR ecclass, ECDbClassMap const* classMapCA) const
    {
    return GetUserStrategyP(ecclass, classMapCA);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
UserECDbMapStrategy* SchemaImportContext::GetUserStrategyP(ECClassCR ecclass) const
    {
    return GetUserStrategyP(ecclass, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
UserECDbMapStrategy* SchemaImportContext::GetUserStrategyP(ECClassCR ecclass, ECDbClassMap const* classMapCA) const
    {
    auto it = m_userStrategyCache.find(&ecclass);
    if (it != m_userStrategyCache.end())
        return it->second.get();

    bool hasClassMapCA = true;
    ECDbClassMap classMap;
    if (classMapCA == nullptr)
        {
        hasClassMapCA = ECDbMapCustomAttributeHelper::TryGetClassMap(classMap, ecclass);
        classMapCA = &classMap;
        }

    std::unique_ptr<UserECDbMapStrategy> userStrategy = std::unique_ptr<UserECDbMapStrategy>(new UserECDbMapStrategy());

    if (hasClassMapCA)
        {
        ECDbClassMap::MapStrategy strategy;
        if (ECObjectsStatus::Success != classMapCA->TryGetMapStrategy(strategy))
            return nullptr; // error

        if (SUCCESS != UserECDbMapStrategy::TryParse(*userStrategy, strategy) || !userStrategy->IsValid())
            return nullptr; // error
        }

    UserECDbMapStrategy* userStrategyP = userStrategy.get();
    m_userStrategyCache[&ecclass] = std::move(userStrategy);
    return userStrategyP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   08/2015
//---------------------------------------------------------------------------------------
void SchemaImportContext::CacheClassMapInfo(ClassMap const& classMap, std::unique_ptr<ClassMapInfo>& info)
    {
    m_classMapInfoCache[&classMap] = std::move(info);
    }


//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   10/2015
//---------------------------------------------------------------------------------------
BentleyStatus SchemaImportECDbMapDb::Load()
    {
    //first load generic mapping meta data
    ECDbSQLManager& manager = m_coreMapDb.GetManagerR();
    if (!manager.IsLoaded())
        {
        if (SUCCESS != manager.Load())
            return ERROR;
        }
    
    //then load meta data only needed for schema import
    return ReadIndexInfosFromDb(manager.GetECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlIndex* SchemaImportECDbMapDb::CreateIndex(ECDbCR ecdb, ECDbSqlTable& table, Utf8CP indexName, bool isUnique, std::vector<ECDbSqlColumn const*> const& columns, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId classId, bool applyToSubclassesIfPartial)
    {
    if (columns.empty())
        {
        BeAssert(false && "Index must have at least one column defined.");
        return nullptr;
        }

    Utf8String generatedIndexName;
    if (Utf8String::IsNullOrEmpty(indexName))
        {
        do
            {
            m_coreMapDb.GetNameGenerator().Generate(generatedIndexName);
            } while (IsNameInUse(generatedIndexName.c_str()));

            indexName = generatedIndexName.c_str();
        }

    return CacheIndex(ecdb, m_coreMapDb.GetManagerR().GetIdGenerator().NextIndexId(),
                      table, indexName, isUnique, columns, addIsNotNullWhereExp, isAutoGenerated, classId, applyToSubclassesIfPartial);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlIndex* SchemaImportECDbMapDb::CreateIndex(ECDbCR ecdb, ECDbSqlTable& table, Utf8CP indexName, bool isUnique, std::vector<Utf8CP> const& columnNames, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId classId, bool applyToSubclassesIfPartial)
    {
    if (columnNames.empty())
        return nullptr;

    std::vector<ECDbSqlColumn const*> columns;
    for (Utf8CP colName : columnNames)
        {
        ECDbSqlColumn const* col = table.FindColumnCP(colName);
        if (col == nullptr)
            {
            BeAssert(false && "Failed to find index column");
            return nullptr;
            }

        columns.push_back(col);
        }

    return CreateIndex(ecdb, table, indexName, isUnique, columns, addIsNotNullWhereExp, isAutoGenerated, classId, applyToSubclassesIfPartial);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlIndex* SchemaImportECDbMapDb::CacheIndex(ECDbCR ecdb, ECDbIndexId id, ECDbSqlTable& table, Utf8CP indexName, bool isUnique, std::vector<ECDbSqlColumn const*> const& columns, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId classId, bool applyToSubclassesIfPartial) const
    {
    BeAssert(!columns.empty());

    auto it = m_usedIndexNames.find(indexName);
    if (it != m_usedIndexNames.end())
        {
        for (std::unique_ptr<ECDbSqlIndex>& index : m_indexes)
            {
            if (index->GetName() == indexName) 
                {
                if (&index->GetTable() == &table && index->GetIsUnique() == isUnique && index->IsAddColumnsAreNotNullWhereExp() == addIsNotNullWhereExp && 
                    index->IsAutoGenerated() == isAutoGenerated)
                    {
                    std::set<ECDbSqlColumn const*> s1(index->GetColumns().begin(), index->GetColumns().end());
                    std::set<ECDbSqlColumn const*> s2(columns.begin(), columns.end());
                    std::vector<ECDbSqlColumn const*> v3;
                    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), std::back_inserter(v3));

                    if (v3.size() == s1.size() && v3.size() == s2.size())
                        return index.get();
                    }
                }
            }
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Index with name '%s' already defined in the ECDb file.", indexName);
        return nullptr;
        }

    std::unique_ptr<ECDbSqlIndex> index (new ECDbSqlIndex(id, table, indexName, isUnique, columns, addIsNotNullWhereExp, isAutoGenerated, classId, applyToSubclassesIfPartial));
    ECDbSqlIndex* indexP = index.get();
    m_indexes.push_back(std::move(index));

    m_usedIndexNames.insert(indexP->GetName().c_str());
    return indexP;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  08/2015
//---------------------------------------------------------------------------------------
BentleyStatus SchemaImportECDbMapDb::CreateOrUpdateIndexesInDb(ECDbR ecdb) const
    {
    if (SUCCESS != TruncateIndexInfoTables(ecdb))
        return ERROR;

    bmap<Utf8String, ECDbSqlIndex const*> comparableIndexDefs;
    for (std::unique_ptr<ECDbSqlIndex>& indexPtr : m_indexes)
        {
        ECDbSqlIndex& index = *indexPtr;
        if (index.GetColumns().empty())
            {
            BeAssert(false && "Index definition is not valid");
            return ERROR;
            }

        //drop index first if it exists, as we always have to recreate them to make sure the class id filter is up-to-date
        Utf8String dropIndexSql("DROP INDEX [");
        dropIndexSql.append(index.GetName()).append("]");
        ecdb.TryExecuteSql(dropIndexSql.c_str());

        //indexes on virtual tables are ignored
        if (index.GetTable().GetPersistenceType() == PersistenceType::Persisted)
            {
            NativeSqlBuilder ddl;
            Utf8String comparableIndexDef;
            if (SUCCESS != BuildCreateIndexDdl(ddl, comparableIndexDef, ecdb, index))
                return ERROR;

            auto it = comparableIndexDefs.find(comparableIndexDef);
            if (it != comparableIndexDefs.end())
                {
                Utf8CP errorMessage = "Index '%s'%s on table '%s' has the same definition as the already existing index '%s'%s. ECDb does not create this index.";
                
                Utf8String provenanceStr;
                if (index.HasClassId())
                    {
                    ECClassCP provenanceClass = ecdb.Schemas().GetECClass(index.GetClassId());
                    if (provenanceClass == nullptr)
                        {
                        BeAssert(false);
                        return ERROR;
                        }
                    provenanceStr.Sprintf(" [Created for ECClass %s]", provenanceClass->GetFullName());
                    }
                
                ECDbSqlIndex const* existingIndex = it->second;
                Utf8String existingIndexProvenanceStr;
                if (existingIndex->HasClassId())
                    {
                    ECClassCP provenanceClass = ecdb.Schemas().GetECClass(existingIndex->GetClassId());
                    if (provenanceClass == nullptr)
                        {
                        BeAssert(false);
                        return ERROR;
                        }
                    existingIndexProvenanceStr.Sprintf(" [Created for ECClass %s]", provenanceClass->GetFullName());
                    }

                if (!index.IsAutoGenerated())
                    ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning, errorMessage,
                                                                  index.GetName().c_str(), provenanceStr.c_str(), index.GetTable().GetName().c_str(), 
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

            if (BE_SQLITE_OK != ecdb.ExecuteSql(ddl.ToString()))
                {
                ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to create index %s on table %s. Error: %s", index.GetName().c_str(), index.GetTable().GetName().c_str(),
                                                              ecdb.GetLastError().c_str());
                BeAssert(false && "Failed to create index");
                return ERROR;
                }
            }

        //populates the ec_Index table (even for indexes on virtual tables, as they might be necessary
        //if further schema imports introduce subclasses of abstract classes (which map to virtual tables))
        if (SUCCESS != InsertIndexInfoIntoDb(ecdb, index))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle 10/2015
//---------------------------------------------------------------------------------------
BentleyStatus SchemaImportECDbMapDb::BuildCreateIndexDdl(NativeSqlBuilder& ddl, Utf8StringR comparableIndexDef, ECDbCR ecdb, ECDbSqlIndex const& index) const
    {
    //this is a string that contains the pieces of the index definition that altogether make the definition unique.
    //that string leaves out keywords common to all indexes (e.g. CREATE, INDEX, WHERE).
    comparableIndexDef.clear();

    ddl.Append("CREATE ");
    if (index.GetIsUnique())
        {
        ddl.Append("UNIQUE ");
        comparableIndexDef.assign("u ");
        }

    Utf8CP indexName = index.GetName().c_str();
    Utf8CP tableName = index.GetTable().GetName().c_str();
    ddl.Append("INDEX ").AppendEscaped(indexName).Append(" ON ").AppendEscaped(tableName).Append(" (");

    comparableIndexDef.append(tableName).append("(");
    bool isFirstCol = true;
    for (ECDbSqlColumn const* col : index.GetColumns())
        {
        if (!isFirstCol)
            ddl.AppendComma(false);

        ddl.AppendEscaped(col->GetName().c_str());
        isFirstCol = false;

        comparableIndexDef.append(col->GetName()).append(" ");
        }

    ddl.AppendParenRight();
    comparableIndexDef.append(")");

    NativeSqlBuilder whereClause;
    if (SUCCESS != GenerateIndexWhereClause(whereClause, ecdb, index))
        return ERROR;

    if (!whereClause.IsEmpty())
        {
        ddl.Append(" WHERE ").Append(whereClause.ToString());
        comparableIndexDef.append(whereClause.ToString());
        }

    comparableIndexDef.ToLower();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  10/2015
//---------------------------------------------------------------------------------------
BentleyStatus SchemaImportECDbMapDb::GenerateIndexWhereClause(NativeSqlBuilder& whereClause, ECDbCR ecdb, ECDbSqlIndex const& index) const
    {
    if (index.IsAddColumnsAreNotNullWhereExp())
        {
        NativeSqlBuilder notNullWhereExp;
        bool isFirstCol = true;
        for (ECDbSqlColumn const* indexCol : index.GetColumns())
            {
            if (indexCol->GetConstraint().IsNotNull())
                continue;

            if (!isFirstCol)
                notNullWhereExp.AppendSpace().Append(BooleanSqlOperator::And, true);

            notNullWhereExp.AppendEscaped(indexCol->GetName().c_str()).AppendSpace().Append(BooleanSqlOperator::IsNot, true).Append("NULL", false);
            isFirstCol = false;
            }

        if (!notNullWhereExp.IsEmpty())
            whereClause.AppendParenLeft().Append(notNullWhereExp).AppendParenRight();
        }

    ECDbSqlColumn const* classIdCol = nullptr;
    if (!index.HasClassId() || !index.GetTable().TryGetECClassIdColumn(classIdCol))
        return SUCCESS;

    BeAssert(index.HasClassId());

    BeAssert(classIdCol != nullptr);

    ECClassCP ecclass = ecdb.Schemas().GetECClass(index.GetClassId());
    if (ecclass == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    ClassMapCP classMap = ecdb.GetECDbImplR().GetECDbMap().GetClassMap(*ecclass);
    if (classMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    StorageDescription const& storageDescription = classMap->GetStorageDescription();
    if (index.AppliesToSubclassesIfPartial() && storageDescription.HierarchyMapsToMultipleTables() && classMap->GetClass().GetRelationshipClassCP()== nullptr)
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                      "Index %s cannot be created for ECClass '%s' because the ECClass has subclasses in other tables and the index is defined to apply to subclasses.",
                                                      index.GetName().c_str(), ecclass->GetFullName());
        return ERROR;
        }

    NativeSqlBuilder classIdFilter;
    if (SUCCESS != storageDescription.GenerateECClassIdFilter(classIdFilter, index.GetTable(), *classIdCol, index.AppliesToSubclassesIfPartial()))
        return ERROR;

    if (classIdFilter.IsEmpty())
        return SUCCESS;

    //now we know the index would have to be partial.
    //non-unique indexes will never be made partial as they don't enforce anything and usually
    //are just there for performance - which a partial index will spoil.
    if (!index.GetIsUnique())
        return SUCCESS;

    //unique indexes are always created to not lose enforcement of the uniqueness
    const bool needsParens = !whereClause.IsEmpty();
    if (needsParens)
        whereClause.AppendSpace().Append(BooleanSqlOperator::And, true).AppendParenLeft();

    whereClause.Append(classIdFilter);

    if (needsParens)
        whereClause.AppendParenRight();

    return SUCCESS;
    }


#define EC_INDEX_TableName "ec_Index"
#define EC_INDEXCOLUMN_TableName "ec_IndexColumn"

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  08/2015
//---------------------------------------------------------------------------------------
BentleyStatus SchemaImportECDbMapDb::ReadIndexInfosFromDb(ECDbCR ecdb) const
    {
    CachedStatementPtr stmt = nullptr;
    ecdb.GetCachedStatement(stmt, "SELECT I.Id, T.Name, I.Name, I.IsUnique, I.AddNotNullWhereExp, I.IsAutoGenerated, I.ClassId, I.AppliesToSubclassesIfPartial FROM " EC_INDEX_TableName " I INNER JOIN ec_Table T ON T.Id = I.TableId");
    if (stmt == nullptr)
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        int64_t id = stmt->GetValueInt64(0);
        Utf8CP tableName = stmt->GetValueText(1);
        Utf8CP name = stmt->GetValueText(2);
        bool isUnique = stmt->GetValueInt(3) == 1;
        bool addNotNullWhereExp = stmt->GetValueInt(4) == 1;
        bool isAutoGenerated = stmt->GetValueInt(5) == 1;
        ECClassId classId = !stmt->IsColumnNull(6) ? stmt->GetValueInt64(6) : ECClass::UNSET_ECCLASSID;
        bool appliesToSubclassesIfPartial = stmt->GetValueInt(7) == 1;

        ECDbSqlTable* table = m_coreMapDb.FindTableP(tableName);
        if (table == nullptr)
            {
            BeAssert(false && "Failed to find table");
            return ERROR;
            }

        CachedStatementPtr indexColStmt = nullptr;
        ecdb.GetCachedStatement(indexColStmt, "SELECT C.Name FROM " EC_INDEXCOLUMN_TableName "  I INNER JOIN ec_Column C ON C.Id = I.ColumnId WHERE I.IndexId = ? ORDER BY I.Ordinal");
        if (indexColStmt == nullptr)
            return ERROR;

        indexColStmt->BindInt64(1, id);
        std::vector<ECDbSqlColumn const*> columns;
        while (indexColStmt->Step() == BE_SQLITE_ROW)
            {
            Utf8CP columnName = indexColStmt->GetValueText(0);
            ECDbSqlColumn const* col = table->FindColumnCP(columnName);
            if (col == nullptr)
                return ERROR;

            columns.push_back(col);
            }

        if (nullptr == CacheIndex(ecdb, id, *table, name, isUnique, columns, addNotNullWhereExp, isAutoGenerated, classId, appliesToSubclassesIfPartial))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
BentleyStatus SchemaImportECDbMapDb::InsertIndexInfoIntoDb(ECDbCR ecdb, ECDbSqlIndex const& index) const
    {
    CachedStatementPtr stmt = nullptr;
    ecdb.GetCachedStatement(stmt, "INSERT INTO " EC_INDEX_TableName "(Id,TableId,Name,IsUnique,AddNotNullWhereExp,IsAutoGenerated,ClassId,AppliesToSubclassesIfPartial) VALUES(?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindInt64(1, index.GetId());
    stmt->BindInt64(2, index.GetTable().GetId());
    stmt->BindText(3, index.GetName().c_str(), Statement::MakeCopy::No);
    stmt->BindInt(4, index.GetIsUnique() ? 1 : 0);
    stmt->BindInt(5, index.IsAddColumnsAreNotNullWhereExp() ? 1 : 0);

    stmt->BindInt(6, index.IsAutoGenerated() ? 1 : 0);
    if (index.HasClassId())
        stmt->BindInt64(7, index.GetClassId());

    stmt->BindInt(8, index.AppliesToSubclassesIfPartial() ? 1 : 0);

    DbResult stat = stmt->Step();
    if (stat != BE_SQLITE_DONE)
        {
        LOG.errorv("Failed to insert index metadata into " EC_INDEX_TableName " for index %s (Id: %lld): %s",
                   index.GetName().c_str(), index.GetId(), ecdb.GetLastError().c_str());
        return ERROR;
        }

    CachedStatementPtr indexColStmt = nullptr;
    ecdb.GetCachedStatement(indexColStmt, "INSERT INTO " EC_INDEXCOLUMN_TableName "(IndexId,ColumnId,Ordinal) VALUES(?,?,?)");
    if (indexColStmt == nullptr)
        return ERROR;

    int i = 0;
    for (ECDbSqlColumn const* col : index.GetColumns())
        {
        indexColStmt->BindInt64(1, index.GetId());
        indexColStmt->BindInt64(2, col->GetId());
        indexColStmt->BindInt64(3, i);

        stat = indexColStmt->Step();
        if (stat != BE_SQLITE_DONE)
            {
            LOG.errorv("Failed to insert index column metadata into " EC_INDEXCOLUMN_TableName " for index %s (Id: %lld) and column %s (Id: %lld): %s",
                       index.GetName().c_str(), index.GetId(), col->GetName().c_str(), col->GetId(), ecdb.GetLastError().c_str());
            return ERROR;
            }

        indexColStmt->Reset();
        indexColStmt->ClearBindings();
        i++;
        }

    return SUCCESS;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus SchemaImportECDbMapDb::TruncateIndexInfoTables(ECDbCR ecdb)
    {
    //ec_IndexColumn is truncated implicity through FK into ec_Index
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "DELETE FROM " EC_INDEX_TableName))
        {
        LOG.errorv("Failed to truncate table " EC_INDEX_TableName ": %s", ecdb.GetLastError().c_str());
        return ERROR;
        }

    if (BE_SQLITE_DONE != stmt.Step())
        {
        LOG.errorv("Failed to truncate table " EC_INDEX_TableName ": %s", ecdb.GetLastError().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  10/2015
//---------------------------------------------------------------------------------------
bool SchemaImportECDbMapDb::IsNameInUse(Utf8CP name) const
    {
    return m_usedIndexNames.find(name) != m_usedIndexNames.end() || m_coreMapDb.IsNameInUse(name);
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECSchemaCompareContext::ReloadECSchemaIfRequired(ECDbSchemaManager const& schemaManager)
    {
    if (HasNoSchemasToImport() || !RequiresUpdate())
        return SUCCESS;

    //save names
    std::vector<Utf8String> existingSchemaNames, importingSchemaNames;
    for (ECSchemaCP schema : m_existingSchemaList)
        existingSchemaNames.push_back(schema->GetName());

    for (ECSchemaCP schema : m_importedSchemaList)
        importingSchemaNames.push_back(schema->GetName());

    m_existingSchemaList.clear();
    m_importedSchemaList.clear();
    schemaManager.GetECDb().ClearECDbCache();

    for (Utf8StringCR name : existingSchemaNames)
        {
        ECSchemaCP schema = schemaManager.GetECSchema(name.c_str());
        if (schema == nullptr)
            {
            BeAssert(false && "Failed to reload a schema");
            return ERROR;
            }

        m_existingSchemaList.push_back(schema);
        }

    for (Utf8StringCR name : importingSchemaNames)
        {
        ECSchemaCP schema = schemaManager.GetECSchema(name.c_str());
        if (schema == nullptr)
            {
            BeAssert(false && "Failed to reload a schema");
            return ERROR;
            }

        m_importedSchemaList.push_back(schema);
        }

  /*  return DbSchemaPersistenceManager::Load(
        schemaManager.GetECDb().GetECDbImplR().GetECDbMap().GetDbSchemaR(), 
        schemaManager.GetECDb(), DbSchema::LoadState::ForSchemaImport);*/
    

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaCompareContext::AssertIfNotPrepared() const
    {
    if (m_prepared)
        return false;

    BeAssert(m_prepared && "Context is not prepared");
    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECSchemaCompareContext::FindExistingSchema(Utf8CP schemaName) const
    {
    if (AssertIfNotPrepared())
        return nullptr;

    for (auto schema : m_existingSchemaList)
        if (schema->GetName() == schemaName)
            return schema;

    return nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaCompareContext::RequiresUpdate() const
    {
    AssertIfNotPrepared();
    return !m_existingSchemaList.empty();
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECSchemaCompareContext::Prepare(ECDbSchemaManager const& schemaManager, bvector<ECSchemaP> const& dependencyOrderedPrimarySchemas)
    {
    if (m_prepared)
        {
        BeAssert(false && "Already prepared");
        return ERROR;
        }

    m_existingSchemaList.clear();
    m_importedSchemaList.clear();
    std::set<Utf8String> doneList;
    for (ECSchemaCP schema : dependencyOrderedPrimarySchemas)
        {
        if (doneList.find(schema->GetFullSchemaName()) != doneList.end())
            continue;

        doneList.insert(schema->GetFullSchemaName());
        if (ECSchemaCP existingSchema = schemaManager.GetECSchema(schema->GetName().c_str(), true))
            {
            if (existingSchema == schema)
                continue;

            m_existingSchemaList.push_back(existingSchema);
            }

        m_importedSchemaList.push_back(schema);
        }

    if (!m_existingSchemaList.empty())
        {
        ECSchemaComparer comparer;
        //We do not require detail if schema is added or deleted the name and version suffice
        ECSchemaComparer::Options options = ECSchemaComparer::Options(ECSchemaComparer::AppendDetailLevel::Partial, ECSchemaComparer::AppendDetailLevel::Partial);
        if (comparer.Compare(m_changes, m_existingSchemaList, m_importedSchemaList, options) != SUCCESS)
            return ERROR;
        
        /*
        Utf8String str;
        m_changes.WriteToString(str);
        printf("%s", str.c_str());
        */
        std::set<Utf8CP, CompareIUtf8> schemaOfInterest;
        if (m_changes.IsValid())
            {
            for (size_t i = 0; i < m_changes.Count(); i++)
                {
                schemaOfInterest.insert(m_changes.At(i).GetId());                
                }
            }
        //Remove any none interesting schemas
        auto importItor = m_importedSchemaList.begin();
        while (importItor != m_importedSchemaList.end())
            {
            if (schemaOfInterest.find((*importItor)->GetName().c_str()) == schemaOfInterest.end())
                importItor = m_importedSchemaList.erase(importItor);
            else
                ++importItor;
            }

        //Remove any none interesting schemas
        auto existingItor = m_existingSchemaList.begin();
        while (existingItor != m_existingSchemaList.end())
            {
            if (schemaOfInterest.find((*existingItor)->GetName().c_str()) == schemaOfInterest.end())
                existingItor = m_existingSchemaList.erase(existingItor);
            else
                ++existingItor;
            }
        }

    m_prepared = true;
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
