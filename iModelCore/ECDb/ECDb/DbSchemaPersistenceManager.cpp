/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbSchemaPersistenceManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

#define EC_INDEX_TableName "ec_Index"
#define EC_INDEXCOLUMN_TableName "ec_IndexColumn"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::Load(DbSchema& dbSchema, ECDbCR ecdb, DbSchema::LoadState loadMode)
    {
    if (loadMode == DbSchema::LoadState::NotLoaded)
        {
        BeAssert(false && "Invalid load mode");
        return ERROR;
        }

    if (Enum::Contains(loadMode, DbSchema::LoadState::Core) &&
        !Enum::Contains(dbSchema.GetLoadState(), DbSchema::LoadState::Core))
        {
        if (BE_SQLITE_OK != ReadTables(dbSchema, ecdb))
            return ERROR;

        if (BE_SQLITE_OK != ReadPropertyPaths(dbSchema, ecdb))
            return ERROR;

        if (BE_SQLITE_OK != ReadClassMappings(dbSchema, ecdb))
            return ERROR;
        }

    if (Enum::Contains(loadMode, DbSchema::LoadState::Indexes) &&
        !Enum::Contains(dbSchema.GetLoadState(), DbSchema::LoadState::Indexes))
        {
        if (SUCCESS != ReadIndexes(dbSchema, ecdb))
            return ERROR;
        }

    dbSchema.SetLoadState(loadMode);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
DbResult DbSchemaPersistenceManager::ReadTables(DbSchema& dbSchema, ECDbCR ecdb)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT A.Id, A.Name, A.Type, A.IsVirtual, B.Name BaseTableName FROM ec_Table A LEFT JOIN ec_Table B ON A.PrimaryTableId = B.Id ORDER BY A.PrimaryTableId");
    if (stmt == nullptr)
        return BE_SQLITE_ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        DbTableId id = stmt->GetValueId<DbTableId>(0);
        Utf8CP name = stmt->GetValueText(1);
        DbTable::Type tableType = Enum::FromInt<DbTable::Type>(stmt->GetValueInt(2));
        PersistenceType persistenceType = IsTrue(stmt->GetValueInt(3)) ? PersistenceType::Virtual : PersistenceType::Persisted;
        Utf8CP primaryTableName = stmt->GetValueText(4);

        DbTable const* primaryTable = nullptr;
        if (!Utf8String::IsNullOrEmpty(primaryTableName))
            {
            primaryTable = dbSchema.FindTable(primaryTableName);
            BeAssert(primaryTable != nullptr && "Failed to find primary table");
            BeAssert(DbTable::Type::Joined == tableType && "Expecting JoinedTable");
            }

        DbTable* table = dbSchema.CreateTable(id, name, tableType, persistenceType, primaryTable);
        if (table == nullptr)
            {
            BeAssert(false && "Failed to create table definition");
            return BE_SQLITE_ERROR;
            }

        const bool canEdit = table->GetEditHandle().CanEdit();
        if (!canEdit)
            table->GetEditHandleR().BeginEdit();

        table->SetId(id);
        // Read columns
        const DbResult stat = ReadColumns(*table, ecdb);
        if (stat != BE_SQLITE_OK)
            return stat;

        if (!canEdit)
            table->GetEditHandleR().EndEdit();
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
DbResult DbSchemaPersistenceManager::ReadColumns(DbTable& table, ECDbCR ecdb)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT Id, Name, Type, IsVirtual, NotNullConstraint, UniqueConstraint, CheckConstraint, DefaultConstraint, CollationConstraint, OrdinalInPrimaryKey, ColumnKind FROM ec_Column WHERE TableId = ? ORDER BY Ordinal");
    if (stmt == nullptr)
        return BE_SQLITE_ERROR;

    DbResult stat = stmt->BindId(1, table.GetId());
    if (stat != BE_SQLITE_OK)
        return stat;

    const int notNullColIx = 4;
    const int uniqueColIx = 5;
    const int checkColIx = 6;
    const int defaultValueColIx = 7;
    const int collationColIx = 8;
    const int pkOrdinalColIx = 9;
    const int kindColIx = 10;

    std::vector<DbColumn*> pkColumns;
    std::vector<size_t> pkOrdinals;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        DbColumnId id = stmt->GetValueId<DbColumnId>(0);
        Utf8CP name = stmt->GetValueText(1);
        const DbColumn::Type type = Enum::FromInt<DbColumn::Type>(stmt->GetValueInt(2));
        const PersistenceType persistenceType = IsTrue(stmt->GetValueInt(3)) ? PersistenceType::Virtual : PersistenceType::Persisted;

        const bool hasNotNullConstraint = IsTrue(stmt->GetValueInt(notNullColIx));
        const bool hasUniqueConstraint = IsTrue(stmt->GetValueInt(uniqueColIx));
        Utf8CP constraintCheck = !stmt->IsColumnNull(checkColIx) ? stmt->GetValueText(checkColIx) : nullptr;
        Utf8CP constraintDefault = !stmt->IsColumnNull(defaultValueColIx) ? stmt->GetValueText(defaultValueColIx) : nullptr;
        const DbColumn::Constraints::Collation collationConstraint = Enum::FromInt<DbColumn::Constraints::Collation>(stmt->GetValueInt(collationColIx));

        int primaryKeyOrdinal = stmt->IsColumnNull(pkOrdinalColIx) ? -1 : stmt->GetValueInt(pkOrdinalColIx);
        const DbColumn::Kind columnKind = Enum::FromInt<DbColumn::Kind>(stmt->GetValueInt(kindColIx));

        DbColumn* column = table.CreateColumn(id, name, type, columnKind, persistenceType);
        if (column == nullptr)
            {
            BeAssert(false);
            return BE_SQLITE_ERROR;
            }

        if (hasNotNullConstraint)
            column->GetConstraintsR().SetNotNullConstraint();

        if (hasUniqueConstraint)
            column->GetConstraintsR().SetUniqueConstraint();

        column->GetConstraintsR().SetCollation(collationConstraint);

        if (!Utf8String::IsNullOrEmpty(constraintCheck))
            column->GetConstraintsR().SetCheckConstraint(constraintCheck);

        if (!Utf8String::IsNullOrEmpty(constraintDefault))
            column->GetConstraintsR().SetDefaultValueExpression(constraintDefault);

        if (primaryKeyOrdinal >= 0)
            {
            pkColumns.push_back(column);
            pkOrdinals.push_back((size_t) primaryKeyOrdinal);
            }
        }

    if (!pkColumns.empty())
        {
        if (SUCCESS != table.CreatePrimaryKeyConstraint(pkColumns, &pkOrdinals))
            return BE_SQLITE_ERROR;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  08/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::ReadIndexes(DbSchema& dbSchema, ECDbCR ecdb)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT I.Id, T.Name, I.Name, I.IsUnique, I.AddNotNullWhereExp, I.IsAutoGenerated, I.ClassId, I.AppliesToSubclassesIfPartial FROM " EC_INDEX_TableName " I INNER JOIN ec_Table T ON T.Id = I.TableId");
    if (stmt == nullptr)
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        DbIndexId id = stmt->GetValueId<DbIndexId>(0);
        Utf8CP tableName = stmt->GetValueText(1);
        Utf8CP name = stmt->GetValueText(2);
        bool isUnique = IsTrue(stmt->GetValueInt(3));
        bool addNotNullWhereExp = IsTrue(stmt->GetValueInt(4));
        bool isAutoGenerated = IsTrue(stmt->GetValueInt(5));
        ECClassId classId = !stmt->IsColumnNull(6) ? stmt->GetValueId<ECClassId>(6) : ECClassId();
        bool appliesToSubclassesIfPartial = IsTrue(stmt->GetValueInt(7));

        DbTable* table = dbSchema.FindTableP(tableName);
        if (table == nullptr)
            {
            BeAssert(false && "Failed to find table");
            return ERROR;
            }

        CachedStatementPtr indexColStmt = ecdb.GetCachedStatement("SELECT C.Name FROM " EC_INDEXCOLUMN_TableName " I INNER JOIN ec_Column C ON C.Id = I.ColumnId WHERE I.IndexId = ? ORDER BY I.Ordinal");
        if (indexColStmt == nullptr)
            return ERROR;

        indexColStmt->BindId(1, id);
        std::vector<DbColumn const*> columns;
        while (indexColStmt->Step() == BE_SQLITE_ROW)
            {
            Utf8CP columnName = indexColStmt->GetValueText(0);
            DbColumn const* col = table->FindColumn(columnName);
            if (col == nullptr)
                return ERROR;

            columns.push_back(col);
            }

        if (nullptr == dbSchema.CreateIndex(id, *table, name, isUnique, columns, addNotNullWhereExp, isAutoGenerated, classId, appliesToSubclassesIfPartial))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
DbResult DbSchemaPersistenceManager::ReadPropertyMappings(ClassDbMapping& classMapping, ECDbCR ecdb, DbSchema const& dbSchema)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT PropertyPathId, T.Name TableName, C.Name ColumnName FROM ec_PropertyMap P INNER JOIN ec_Column C ON C.Id = P.ColumnId INNER JOIN ec_Table T ON T.Id = C.TableId WHERE P.ClassMapId = ?");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return BE_SQLITE_ERROR;
        }

    stmt->BindId(1, classMapping.GetId());
    bmap<PropertyPathId, PropertyDbMapping*> cache;
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        PropertyPathId propertyPathId = stmt->GetValueId<PropertyPathId>(0);
        Utf8CP tableName = stmt->GetValueText(1);
        Utf8CP columnName = stmt->GetValueText(2);

        PropertyDbMapping::Path const* propertyPath = dbSchema.GetDbMappings().FindPropertyPath(propertyPathId);
        if (propertyPath == nullptr)
            {
            BeAssert(false && "Failed to resolve property path id");
            return BE_SQLITE_ERROR;
            }

        DbTable const* table = dbSchema.FindTable(tableName);
        if (table == nullptr)
            {
            BeAssert(false && "Failed to resolve table");
            return BE_SQLITE_ERROR;
            }

        DbColumn const* column = table->FindColumn(columnName);
        if (column == nullptr)
            {
            BeAssert(false && "Failed to resolve column");
            return BE_SQLITE_ERROR;
            }

        PropertyDbMapping* pm = nullptr;
        auto it = cache.find(propertyPathId);
        if (it == cache.end())
            {
            pm = classMapping.CreatePropertyMapping(*propertyPath);
            if (pm == nullptr)
                {
                BeAssert(false && "Failed to create propertyMap");
                return BE_SQLITE_ERROR;
                }

            cache[propertyPathId] = pm;
            }
        else
            pm = it->second;

        pm->GetColumnsR().push_back(column);
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
DbResult DbSchemaPersistenceManager::ReadClassMappings(DbSchema& dbSchema, ECDbCR ecdb)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT Id, ParentId, ClassId, MapStrategy, MapStrategyOptions, MapStrategyMinSharedColumnCount, MapStrategyAppliesToSubclasses FROM ec_ClassMap ORDER BY Id");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return BE_SQLITE_ERROR;
        }

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ClassMapId id = stmt->GetValueId<ClassMapId>(0);
        ClassMapId parentId = stmt->IsColumnNull(1) ? ClassMapId() : stmt->GetValueId<ClassMapId>(1);
        ECN::ECClassId classId = stmt->GetValueId<ECClassId>(2);

        const int minSharedColCount = stmt->IsColumnNull(5) ? ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT : stmt->GetValueInt(5);
        ECDbMapStrategy mapStrategy;
        if (SUCCESS != mapStrategy.Assign(Enum::FromInt<ECDbMapStrategy::Strategy>(stmt->GetValueInt(3)),
                                          Enum::FromInt<ECDbMapStrategy::Options>(stmt->GetValueInt(4)),
                                          minSharedColCount, IsTrue(stmt->GetValueInt(6))))
            {
            BeAssert(false && "Found invalid persistence values for ECDbMapStrategy");
            return BE_SQLITE_ERROR;
            }

        ClassDbMapping* classMapping = dbSchema.GetDbMappingsR().AddClassMapping(id, classId, mapStrategy, parentId);
        if (classMapping == nullptr)
            {
            BeAssert(false && "Failed to create classMap");
            return BE_SQLITE_ERROR;
            }

        if (ReadPropertyMappings(*classMapping, ecdb, dbSchema) != BE_SQLITE_OK)
            {
            BeAssert(false && "Failed to resolve property map");
            return BE_SQLITE_ERROR;
            }
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
DbResult DbSchemaPersistenceManager::ReadPropertyPaths(DbSchema& dbSchema, ECDbCR ecdb)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT Id, RootPropertyId, AccessString FROM ec_PropertyPath");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return BE_SQLITE_ERROR;
        }

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        PropertyPathId id = stmt->GetValueId<PropertyPathId>(0);
        ECPropertyId rootPropertyId = stmt->GetValueId<ECPropertyId>(1);
        Utf8CP accessString = stmt->GetValueText(2);

        PropertyDbMapping::Path const* pp = dbSchema.GetDbMappingsR().AddPropertyPath(id, rootPropertyId, accessString);
        if (pp == nullptr)
            {
            BeAssert(false && "Failed to resolve property path");
            return BE_SQLITE_ERROR;
            }
        }

    return BE_SQLITE_OK;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::Save(ECDbCR ecdb, DbSchema const& dbSchema)
    {
    if (ecdb.IsReadonly())
        {
        BeAssert(false && "ECDb is read-only");
        return ERROR;
        }

    if (BE_SQLITE_OK != ecdb.ExecuteSql("DELETE FROM ec_Table"))
        {
        BeAssert(false && "Truncating ec_Table failed");
        return ERROR;
        }

    if (BE_SQLITE_OK != ecdb.ExecuteSql("DELETE FROM ec_ClassMap"))
        {
        BeAssert(false && "Truncating ec_ClassMap failed");
        return ERROR;
        }

    if (BE_SQLITE_OK != ecdb.ExecuteSql("DELETE FROM ec_PropertyPath"))
        {
        BeAssert(false && "Truncating ec_PropertyPath failed");
        return ERROR;
        }

    //order tables so that joined tables go last. Because of FK constraints
    //they must be created after the primary tables
    std::vector<DbTable const*> tables;
    std::vector<DbTable const*> joinedTables;
    for (auto const& kvPair : dbSchema.GetTables())
        {
        DbTable const& table = *kvPair.second.get();
        if (table.GetType() == DbTable::Type::Joined)
            joinedTables.push_back(&table);
        else
            tables.push_back(&table);
        }

    tables.insert(tables.end(), joinedTables.begin(), joinedTables.end());

    for (DbTable const* table : tables)
        {
        if (BE_SQLITE_OK != InsertTable(ecdb, *table))
            return ERROR;
        }

    //PK or FK constraints are not saved in the metadata

    DbMappings const& dbMappings = dbSchema.GetDbMappings();
    for (auto const& kvPair : dbMappings.GetPropertyPaths())
        {
        if (BE_SQLITE_OK != InsertPropertyPath(ecdb, *kvPair.second))
            return ERROR;
        }

    for (auto const& kvPair : dbMappings.GetClassMappings())
        {
        if (BE_SQLITE_OK != InsertClassMapping(ecdb, *kvPair.second))
            return ERROR;
        }

    for (auto const& kvPair : dbMappings.GetClassMappings())
        {
        ClassDbMapping const& classMapping = *kvPair.second;
        std::vector<PropertyDbMapping const*> propMappings;
        classMapping.GetPropertyMappings(propMappings, false);
        for (PropertyDbMapping const* propertyMap : propMappings)
            {
            if (BE_SQLITE_OK != InsertPropertyMapping(ecdb, *propertyMap))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
DbResult DbSchemaPersistenceManager::InsertTable(ECDbCR ecdb, DbTable const& table)
    {
    if (!table.IsNullTable() && !table.IsValid())
        {
        BeAssert(false && "Table to insert is not valid");
        return BE_SQLITE_ERROR;
        }

    CachedStatementPtr stmt = ecdb.GetCachedStatement("INSERT INTO ec_Table(Id, Name, Type, IsVirtual, PrimaryTableId) VALUES (?, ?, ?, ?, ?)");
    if (stmt == nullptr)
        return BE_SQLITE_ERROR;

    stmt->BindId(1, table.GetId());
    stmt->BindText(2, table.GetName().c_str(), Statement::MakeCopy::No);
    stmt->BindInt(3, Enum::ToInt(table.GetType()));
    stmt->BindInt(4, table.GetPersistenceType() == PersistenceType::Virtual ? 1 : 0);
    if (auto primaryTable = table.GetParentOfJoinedTable())
        stmt->BindId(5, primaryTable->GetId());
    else
        stmt->BindNull(5);

    DbResult stat = stmt->Step();
    if (stat != BE_SQLITE_DONE)
        return stat;

    bmap<DbColumn const*, int> primaryKeys;
    int i = 0;
    if (PrimaryKeyDbConstraint const* pkConstraint = table.GetPrimaryKeyConstraint())
        {
        for (DbColumn const* pkCol : pkConstraint->GetColumns())
            {
            primaryKeys[pkCol] = i++;
            }
        }

    int columnOrdinal = 0;
    for (DbColumn const* column : table.GetColumns())
        {
        auto it = primaryKeys.find(column);
        const int primaryKeyOrdinal = it == primaryKeys.end() ? -1 : it->second;
        stat = InsertColumn(ecdb, *column, columnOrdinal, primaryKeyOrdinal);
        if (stat != BE_SQLITE_OK)
            return stat;

        columnOrdinal++;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
DbResult DbSchemaPersistenceManager::InsertColumn(ECDbCR ecdb, DbColumn const& column, int columnOrdinal, int primaryKeyOrdinal)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("INSERT INTO ec_Column (Id, TableId, Name, Type, IsVirtual, Ordinal, NotNullConstraint, UniqueConstraint, CheckConstraint, DefaultConstraint, CollationConstraint, OrdinalInPrimaryKey, ColumnKind) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    if (stmt == nullptr)
        return BE_SQLITE_ERROR;

    stmt->BindId(1, column.GetId());
    stmt->BindId(2, column.GetTable().GetId());
    stmt->BindText(3, column.GetName().c_str(), Statement::MakeCopy::No);
    stmt->BindInt(4, Enum::ToInt(column.GetType()));
    stmt->BindInt(5, BoolToSqlInt(column.GetPersistenceType() == PersistenceType::Virtual));
    stmt->BindInt64(6, columnOrdinal);
    stmt->BindInt(7, BoolToSqlInt(column.GetConstraints().HasNotNullConstraint()));
    stmt->BindInt(8, BoolToSqlInt(column.GetConstraints().HasUniqueConstraint()));

    if (!column.GetConstraints().GetCheckConstraint().empty())
        stmt->BindText(9, column.GetConstraints().GetCheckConstraint().c_str(), Statement::MakeCopy::No);

    if (!column.GetConstraints().GetDefaultValueConstraint().empty())
        stmt->BindText(10, column.GetConstraints().GetDefaultValueConstraint().c_str(), Statement::MakeCopy::No);

    stmt->BindInt(11, Enum::ToInt(column.GetConstraints().GetCollation()));
    if (primaryKeyOrdinal > -1)
        stmt->BindInt(12, primaryKeyOrdinal);

    stmt->BindInt(13, Enum::ToInt(column.GetKind()));
    const DbResult stat = stmt->Step();
    return stat == BE_SQLITE_DONE ? BE_SQLITE_OK : stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
DbResult DbSchemaPersistenceManager::InsertIndex(ECDbCR ecdb, DbIndex const& index)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("INSERT INTO " EC_INDEX_TableName "(Id,TableId,Name,IsUnique,AddNotNullWhereExp,IsAutoGenerated,ClassId,AppliesToSubclassesIfPartial) VALUES(?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return BE_SQLITE_ERROR;

    stmt->BindId(1, index.GetId());
    stmt->BindId(2, index.GetTable().GetId());
    stmt->BindText(3, index.GetName().c_str(), Statement::MakeCopy::No);
    stmt->BindInt(4, index.GetIsUnique() ? 1 : 0);
    stmt->BindInt(5, index.IsAddColumnsAreNotNullWhereExp() ? 1 : 0);

    stmt->BindInt(6, index.IsAutoGenerated() ? 1 : 0);
    if (index.HasClassId())
        stmt->BindId(7, index.GetClassId());

    stmt->BindInt(8, index.AppliesToSubclassesIfPartial() ? 1 : 0);

    DbResult stat = stmt->Step();
    if (stat != BE_SQLITE_DONE)
        {
        LOG.errorv("Failed to insert index metadata into " EC_INDEX_TableName " for index %s (Id: %s): %s",
                   index.GetName().c_str(), index.GetId().ToString().c_str(), ecdb.GetLastError().c_str());
        return stat;
        }

    CachedStatementPtr indexColStmt = ecdb.GetCachedStatement("INSERT INTO " EC_INDEXCOLUMN_TableName "(IndexId,ColumnId,Ordinal) VALUES(?,?,?)");
    if (indexColStmt == nullptr)
        return BE_SQLITE_ERROR;

    int i = 0;
    for (DbColumn const* col : index.GetColumns())
        {
        indexColStmt->BindId(1, index.GetId());
        indexColStmt->BindId(2, col->GetId());
        indexColStmt->BindInt(3, i);

        stat = indexColStmt->Step();
        if (stat != BE_SQLITE_DONE)
            {
            LOG.errorv("Failed to insert index column metadata into " EC_INDEXCOLUMN_TableName " for index %s (Id: %s) and column %s (Id: %s): %s",
                       index.GetName().c_str(), index.GetId().ToString().c_str(), col->GetName().c_str(), col->GetId().ToString().c_str(), ecdb.GetLastError().c_str());
            return stat;
            }

        indexColStmt->Reset();
        indexColStmt->ClearBindings();
        i++;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
DbResult DbSchemaPersistenceManager::InsertPropertyMapping(ECDbCR ecdb, PropertyDbMapping const& pm)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("INSERT INTO ec_PropertyMap(ClassMapId, PropertyPathId, ColumnId) VALUES (?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return BE_SQLITE_ERROR;
        }

    DbResult stat = BE_SQLITE_DONE;
    for (DbColumn const* column : pm.GetColumns())
        {
        stmt->BindId(1, pm.GetClassMapping().GetId());
        stmt->BindId(2, pm.GetPropertyPath().GetId());
        stmt->BindId(3, column->GetId());
        stat = stmt->Step();
        if (stat != BE_SQLITE_DONE)
            return stat;

        stmt->Reset();
        stmt->ClearBindings();
        }

    return stat == BE_SQLITE_DONE ? BE_SQLITE_OK : stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
DbResult DbSchemaPersistenceManager::InsertClassMapping(ECDbCR ecdb, ClassDbMapping const& cm)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("INSERT INTO ec_ClassMap(Id, ParentId, ClassId, MapStrategy, MapStrategyOptions, MapStrategyMinSharedColumnCount, MapStrategyAppliesToSubclasses) VALUES (?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return BE_SQLITE_ERROR;
        }

    stmt->BindId(1, cm.GetId());
    if (!cm.GetBaseClassMappingId().IsValid())
        stmt->BindNull(2);
    else
        stmt->BindId(2, cm.GetBaseClassMappingId());

    stmt->BindId(3, cm.GetClassId());
    stmt->BindInt(4, Enum::ToInt(cm.GetMapStrategy().GetStrategy()));
    stmt->BindInt(5, Enum::ToInt(cm.GetMapStrategy().GetOptions()));
    const int minSharedColCount = cm.GetMapStrategy().GetMinimumSharedColumnCount();
    if (minSharedColCount != ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT)
        stmt->BindInt(6, minSharedColCount);

    stmt->BindInt(7, cm.GetMapStrategy().AppliesToSubclasses() ? 1 : 0);

    const DbResult stat = stmt->Step();
    return stat == BE_SQLITE_DONE ? BE_SQLITE_OK : stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
DbResult DbSchemaPersistenceManager::InsertPropertyPath(ECDbCR ecdb, PropertyDbMapping::Path const& pp)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("INSERT INTO ec_PropertyPath(Id, RootPropertyId, AccessString) VALUES (?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return BE_SQLITE_ERROR;
        }

    stmt->BindId(1, pp.GetId());
    stmt->BindId(2, pp.GetRootPropertyId());
    stmt->BindText(3, pp.GetAccessString().c_str(), Statement::MakeCopy::No);
    const DbResult stat = stmt->Step();
    return stat == BE_SQLITE_DONE ? BE_SQLITE_OK : stat;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
DbSchemaPersistenceManager::CreateOrUpdateTableResult DbSchemaPersistenceManager::CreateOrUpdateTable(ECDbCR ecdb, DbTable const& table)
    {
    if (table.GetPersistenceType() == PersistenceType::Virtual || table.GetType() == DbTable::Type::Existing)
        return CreateOrUpdateTableResult::Skipped;

    Utf8CP tableName = table.GetName().c_str();
    BeBriefcaseId briefcaseId = ecdb.GetBriefcaseId();
    const bool allowDbSchemaChange = briefcaseId.IsMasterId() || briefcaseId.IsStandaloneId();

    CreateOrUpdateTableResult mode;
    if (ecdb.TableExists(tableName))
        mode = IsTableChanged(ecdb, table) ? CreateOrUpdateTableResult::Updated : CreateOrUpdateTableResult::WasUpToDate;
    else
        mode = CreateOrUpdateTableResult::Created;

    if (mode == CreateOrUpdateTableResult::WasUpToDate)
        return mode;

    if (!allowDbSchemaChange)
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECSchemas: Imported ECSchemas would change the database schema. "
                                                      "This is only allowed for standalone briefcases or the master briefcase. Briefcase id: %" PRIu32, briefcaseId.GetValue());
        return CreateOrUpdateTableResult::Error;
        }

    BentleyStatus stat = SUCCESS;
    if (mode == CreateOrUpdateTableResult::Created)
        stat = CreateTable(ecdb, table);
    else
        {
        BeAssert(mode == CreateOrUpdateTableResult::Updated);
        stat = UpdateTable(ecdb, table);
        }

    return SUCCESS == stat ? mode : CreateOrUpdateTableResult::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::CreateTable(ECDbCR ecdb, DbTable const& table)
    {
    if (!table.IsValid())
        {
        BeAssert(false && "Table definition is not valid");
        return ERROR;
        }

    if (table.GetType() == DbTable::Type::Existing || table.GetPersistenceType() == PersistenceType::Virtual)
        {
        BeAssert(false && "CreateTable must not be called on virtual table or table not owned by ECDb");
        return ERROR;
        }

    std::vector<DbColumn const*> columns;
    table.GetFilteredColumnList(columns, PersistenceType::Persisted);
    if (columns.empty())
        {
        BeAssert(false && "Table have no persisted columns");
        return ERROR;
        }

    Utf8String ddl("CREATE TABLE [");
    ddl.append(table.GetName()).append("](");

    bool isFirstCol = true;
    for (DbColumn const* col : columns)
        {
        if (!isFirstCol)
            ddl.append(", ");

        if (SUCCESS != AppendColumnDdl(ddl, *col))
            return ERROR;

        isFirstCol = false;
        }

    // Append constraints;
    PrimaryKeyDbConstraint const* pkConstraint = table.GetPrimaryKeyConstraint();
    if (pkConstraint != nullptr && pkConstraint->GetColumns().size() > 1)
        {
        //only use a PRIMARY KEY constraint clause for multi-column keys. Single column keys use the column constraint clause (handled above)
        ddl.append(", PRIMARY KEY(");
        AppendColumnNamesToDdl(ddl, pkConstraint->GetColumns());
        ddl.append(")");
        }

    std::vector<DbConstraint const*> tableConstraints = table.GetConstraints();
    for (DbConstraint const* tableConstraint : tableConstraints)
        {
        ddl.append(", ");

        if (tableConstraint->GetType() == DbConstraint::Type::ForeignKey)
            {
            ForeignKeyDbConstraint const* fkConstraint = static_cast<ForeignKeyDbConstraint const*>(tableConstraint);
            if (SUCCESS != AppendForeignKeyDdl(ddl, *fkConstraint))
                return ERROR;
            }
        else
            {
            BeAssert(false && "Unsupported DbConstraint type");
            }
        }
    ddl.append(")");

    if (ecdb.ExecuteSql(ddl.c_str()) != BE_SQLITE_OK)
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to create table %s: %s",
                                                      table.GetName().c_str(), ecdb.GetLastError().c_str());
        return ERROR;
        }

    return CreateTriggers(ecdb, table, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::UpdateTable(ECDbCR ecdb, DbTable const& table)
    {
    if (table.GetType() == DbTable::Type::Existing || table.GetPersistenceType() == PersistenceType::Virtual)
        {
        BeAssert(false && "UpdateTable must not be called on virtual table or table not owned by ECDb");
        return ERROR;
        }

    Utf8CP tableName = table.GetName().c_str();
    DbSchema::EntityType type = DbSchema::GetEntityType(ecdb, tableName);
    if (type == DbSchema::EntityType::None)
        {
        BeAssert(false && "Table is expected to exist already");
        return ERROR;
        }

    //! Object type is view and exist in db. Action = DROP it and recreate it.
    if (type == DbSchema::EntityType::View)
        {
        Utf8String sql("DROP VIEW [");
        sql.append(tableName).append("]");
        auto r = ecdb.ExecuteSql(sql.c_str());
        if (r != BE_SQLITE_OK)
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to drop view '%s'", tableName);
            return ERROR;
            }

        return CreateTable(ecdb, table);
        }

    BeAssert(type == DbSchema::EntityType::Table);

    bvector<Utf8String> existingColumnNamesInDb;
    if (!ecdb.GetColumns(existingColumnNamesInDb, tableName))
        {
        BeAssert(false && "Failed to get column list for table");
        return ERROR;
        }

    //Create a fast hash set of existing db column list
    bset<Utf8String, CompareIUtf8Ascii> existingColumnNamesInDbSet;
    for (Utf8StringCR existingDbColumn : existingColumnNamesInDb)
        {
        existingColumnNamesInDbSet.insert(existingDbColumn);
        }

    //Create a fast hash set of in-memory column list;
    std::vector<DbColumn const*> columns;
    table.GetFilteredColumnList(columns, PersistenceType::Persisted);

    std::vector<DbColumn const*> newColumns;
    //compute new columns;
    for (DbColumn const* col : columns)
        {
        if (existingColumnNamesInDbSet.find(col->GetName().c_str()) == existingColumnNamesInDbSet.end())
            newColumns.push_back(col);
        }


    if (SUCCESS != AlterTable(ecdb, table, newColumns))
        return ERROR;

    return CreateTriggers(ecdb, table, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::AlterTable(ECDbCR ecdb, DbTable const& table, std::vector<DbColumn const*> const& columnsToAdd)
    {
    if (columnsToAdd.empty())
        return SUCCESS;

    Utf8String alterDdlTemplate;
    alterDdlTemplate.Sprintf("ALTER TABLE [%s] ADD COLUMN ", table.GetName().c_str());
    for (DbColumn const* columnToAdd : columnsToAdd)
        {
        BeAssert(&table == &columnToAdd->GetTable());
        //Limitation of ADD COLUMN http://www.sqlite.org/lang_altertable.html
        
        Utf8String ddl(alterDdlTemplate);
        if (SUCCESS != AppendColumnDdl(ddl, *columnToAdd))
            return ERROR;

        //append FK constraints, if defined for this column
        for (DbConstraint const* constraint : table.GetConstraints())
            {
            if (constraint->GetType() != DbConstraint::Type::ForeignKey)
                continue;

            ForeignKeyDbConstraint const* fkConstraint = static_cast<ForeignKeyDbConstraint const*> (constraint);
            if (!fkConstraint->IsValid())
                return ERROR;

            if (fkConstraint->GetFkColumns().size() != 1 || fkConstraint->GetFkColumns()[0] != columnToAdd)
                continue;

            if (SUCCESS != AppendForeignKeyToColumnDdl(ddl, *fkConstraint, *columnToAdd))
                return ERROR;
            }

        if (BE_SQLITE_OK != ecdb.ExecuteSql(ddl.c_str()))
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to add new column (%s). Error message: %s", ddl.c_str(), ecdb.GetLastError().c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static
bool DbSchemaPersistenceManager::IsTableChanged(ECDbCR ecdb, DbTable const& table)
    {
    bvector<Utf8String> namesOfExistingColumns;
    if (!ecdb.GetColumns(namesOfExistingColumns, table.GetName().c_str()))
        {
        BeAssert(false && "Failed to get column list for table");
        return true;
        }

    //Create a fast hash set of existing db column list
    bset<Utf8String, CompareIUtf8Ascii> namesOfExistingColumnsSet;
    for (Utf8StringCR name : namesOfExistingColumns)
        {
        namesOfExistingColumnsSet.insert(name);
        }

    for (DbColumn const* col : table.GetColumns())
        {
        if (namesOfExistingColumnsSet.find(col->GetName()) == namesOfExistingColumnsSet.end())
            return true; //new column
        }

    //no columns were added. So difference in columns means that columns was deleted -> which also means that the table changed.
    return table.GetColumns().size() != namesOfExistingColumns.size();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  08/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::CreateOrUpdateIndexes(ECDbCR ecdb, DbSchema const& dbSchema)
    {
#ifndef NDEBUG
            {
            Statement stmt;
            if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT NULL FROM " EC_INDEX_TableName " LIMIT 1"))
                return ERROR;

            if (stmt.Step() == BE_SQLITE_ROW)
                {
                BeAssert(false && "ec_Index is expected to be empty");
                return ERROR;
                }
            }
#endif

    bmap<Utf8String, DbIndex const*, CompareIUtf8Ascii> comparableIndexDefs;
    for (std::unique_ptr<DbIndex> const& indexPtr : dbSchema.GetIndexes())
        {
        DbIndex& index = *indexPtr;
        if (index.GetColumns().empty())
            {
            BeAssert(false && "Index definition is not valid");
            return ERROR;
            }

        //drop index first if it exists, as we always have to recreate them to make sure the class id filter is up-to-date
        Utf8String dropIndexSql;
        dropIndexSql.Sprintf("DROP INDEX [%s]", index.GetName().c_str());
        ecdb.TryExecuteSql(dropIndexSql.c_str());

        //indexes on virtual tables are ignored
        if (index.GetTable().GetPersistenceType() == PersistenceType::Persisted)
            {
            Utf8String ddl, comparableIndexDef;
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

                DbIndex const* existingIndex = it->second;
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

            if (BE_SQLITE_OK != ecdb.ExecuteSql(ddl.c_str()))
                {
                ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to create index %s on table %s. Error: %s", index.GetName().c_str(), index.GetTable().GetName().c_str(),
                                                                ecdb.GetLastError().c_str());
                BeAssert(false && "Failed to create index");
                return ERROR;
                }
            }

        //populates the ec_Index table (even for indexes on virtual tables, as they might be necessary
        //if further schema imports introduce subclasses of abstract classes (which map to virtual tables))
        if (BE_SQLITE_OK != InsertIndex(ecdb, index))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle 10/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::BuildCreateIndexDdl(Utf8StringR ddl, Utf8StringR comparableIndexDef, ECDbCR ecdb, DbIndex const& index)
    {
    //this is a string that contains the pieces of the index definition that altogether make the definition unique.
    //that string leaves out index name and keywords common to all indexes (e.g. CREATE, INDEX, WHERE).
    comparableIndexDef.clear();

    ddl.assign("CREATE ");
    if (index.GetIsUnique())
        {
        ddl.append("UNIQUE ");
        comparableIndexDef.assign("u ");
        }

    Utf8CP indexName = index.GetName().c_str();
    Utf8CP tableName = index.GetTable().GetName().c_str();
    Utf8String columnsDdl;
    AppendColumnNamesToDdl(columnsDdl, index.GetColumns());

    ddl.append("INDEX [").append(indexName).append("] ON [").append(tableName).append("](").append(columnsDdl).append(")");
    comparableIndexDef.append(tableName).append("(").append(columnsDdl).append(")");

    Utf8String whereClause;
    if (SUCCESS != GenerateIndexWhereClause(whereClause, ecdb, index))
        return ERROR;

    if (!whereClause.empty())
        {
        ddl.append(" WHERE ").append(whereClause);
        comparableIndexDef.append(whereClause);
        }

    comparableIndexDef.ToLower();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  10/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::GenerateIndexWhereClause(Utf8StringR whereClause, ECDbCR ecdb, DbIndex const& index)
    {
    if (index.IsAddColumnsAreNotNullWhereExp())
        {
        Utf8String notNullWhereExp;
        bool isFirstCol = true;
        for (DbColumn const* indexCol : index.GetColumns())
            {
            if (indexCol->GetConstraints().HasNotNullConstraint())
                continue;

            if (!isFirstCol)
                notNullWhereExp.append(" AND ");

            notNullWhereExp.append("[").append(indexCol->GetName()).append("] IS NOT NULL");
            isFirstCol = false;
            }

        if (!notNullWhereExp.empty())
            whereClause.append("(").append(notNullWhereExp).append(")");
        }

    DbColumn const* classIdCol = nullptr;
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
    if (index.AppliesToSubclassesIfPartial() && storageDescription.HierarchyMapsToMultipleTables() && classMap->GetClass().GetRelationshipClassCP() == nullptr)
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                        "Index %s cannot be created for ECClass '%s' because the ECClass has subclasses in other tables and the index is defined to apply to subclasses.",
                                                        index.GetName().c_str(), ecclass->GetFullName());
        return ERROR;
        }

    Utf8String classIdFilter;
    if (SUCCESS != storageDescription.GenerateECClassIdFilter(classIdFilter, index.GetTable(), *classIdCol, index.AppliesToSubclassesIfPartial()))
        return ERROR;

    if (classIdFilter.empty())
        return SUCCESS;

    //now we know the index would have to be partial.
    //non-unique indexes will never be made partial as they don't enforce anything and usually
    //are just there for performance - which a partial index will spoil.
    if (!index.GetIsUnique())
        return SUCCESS;

    //unique indexes are always created to not lose enforcement of the uniqueness
    const bool needsParens = !whereClause.empty();
    if (needsParens)
        whereClause.append(" AND (");

    whereClause.append(classIdFilter);

    if (needsParens)
        whereClause.append(")");

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::CreateTriggers(ECDbCR ecdb, DbTable const& table, bool failIfExists)
    {
    for (DbTrigger const* trigger : table.GetTriggers())
        {
        if (TriggerExistsInDb(ecdb, *trigger))
            {
            if (failIfExists)
                {
                ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Trigger %s already exists on table %s.", trigger->GetName(), trigger->GetTable().GetName().c_str());
                return ERROR;
                }

            continue;
            }

        Utf8String ddl("CREATE TRIGGER [");
        ddl.append(trigger->GetName()).append("] ");

        switch (trigger->GetType())
            {
                case DbTrigger::Type::After:
                    ddl.append("AFTER");
                    break;
                case DbTrigger::Type::Before:
                    ddl.append("BEFORE");

                default:
                    break;
            }

        ddl.append(" UPDATE ON [").append(trigger->GetTable().GetName()).append("] WHEN ");
        ddl.append(trigger->GetCondition()).append(" ").append(trigger->GetBody());

        if (ecdb.ExecuteSql(ddl.c_str()) != BE_SQLITE_OK)
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to create trigger %s on table %s. Error: %s", trigger->GetName(), trigger->GetTable().GetName().c_str(),
                                                          ecdb.GetLastError().c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static
bool DbSchemaPersistenceManager::TriggerExistsInDb(ECDbCR ecdb, DbTrigger const& trigger)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != ecdb.GetCachedStatement(stmt, "select NULL from sqlite_master WHERE type='trigger' and name=?"))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindText(1, trigger.GetName(), Statement::MakeCopy::No);
    return BE_SQLITE_ROW == stmt->Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::AppendColumnDdl(Utf8StringR ddl, DbColumn const& col)
    {
    ddl.append("[").append(col.GetName()).append("] ");
    Utf8CP typeStr = DbColumn::TypeToSql(col.GetType());
    if (typeStr == nullptr)
        return ERROR;

    ddl.append(typeStr);

    if (col.IsOnlyColumnOfPrimaryKeyConstraint())
        {
        ddl.append(" PRIMARY KEY");
        BeAssert(!col.GetConstraints().HasNotNullConstraint() && !col.GetConstraints().HasUniqueConstraint());
        return SUCCESS;
        }

    DbColumn::Constraints const& colConstraint = col.GetConstraints();
    if (colConstraint.HasNotNullConstraint())
        ddl.append(" NOT NULL");

    if (colConstraint.HasUniqueConstraint())
        ddl.append(" UNIQUE");

    if (colConstraint.GetCollation() != DbColumn::Constraints::Collation::Default)
        ddl.append(" COLLATE ").append(DbColumn::Constraints::CollationToSql(colConstraint.GetCollation()));

    if (!colConstraint.GetDefaultValueConstraint().empty())
        ddl.append(" DEFAULT(").append(colConstraint.GetDefaultValueConstraint()).append(")");

    if (!colConstraint.GetCheckConstraint().empty())
        ddl.append(" CHECK('").append(colConstraint.GetCheckConstraint()).append("')");

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
void DbSchemaPersistenceManager::AppendColumnNamesToDdl(Utf8StringR ddl, std::vector<DbColumn const*> const& columns)
    {
    bool isFirstCol = true;
    for (DbColumn const* col : columns)
        {
        if (!isFirstCol)
            ddl.append(", ");

        ddl.append("[").append(col->GetName()).append("]");

        isFirstCol = false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::AppendForeignKeyDdl(Utf8StringR ddl, ForeignKeyDbConstraint const& fkConstraint)
    {
    if (!fkConstraint.IsValid())
        return ERROR;

    ddl.append("FOREIGN KEY(");
    AppendColumnNamesToDdl(ddl, fkConstraint.GetFkColumns());
    ddl.append(")");

    DoAppendForeignKeyDdl(ddl, fkConstraint);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::AppendForeignKeyToColumnDdl(Utf8StringR ddl, ForeignKeyDbConstraint const& fkConstraint, DbColumn const& singleFkColumn)
    {
    if (!fkConstraint.IsValid())
        return ERROR;

    if (fkConstraint.GetFkColumns().size() != 1 || fkConstraint.GetFkColumns()[0] != &singleFkColumn)
        return ERROR;

    DoAppendForeignKeyDdl(ddl, fkConstraint);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
void DbSchemaPersistenceManager::DoAppendForeignKeyDdl(Utf8StringR ddl, ForeignKeyDbConstraint const& fkConstraint)
    {
    ddl.append(" REFERENCES [").append(fkConstraint.GetReferencedTable().GetName()).append("](");
    AppendColumnNamesToDdl(ddl, fkConstraint.GetReferencedTableColumns());
    ddl.append(")");

    if (fkConstraint.GetOnDeleteAction() != ForeignKeyDbConstraint::ActionType::NotSpecified)
        ddl.append(" ON DELETE ").append(ForeignKeyDbConstraint::ActionTypeToSql(fkConstraint.GetOnDeleteAction()));

    if (fkConstraint.GetOnUpdateAction() != ForeignKeyDbConstraint::ActionType::NotSpecified)
        ddl.append(" ON UPDATE ").append(ForeignKeyDbConstraint::ActionTypeToSql(fkConstraint.GetOnUpdateAction()));
    }

//---------------------------------------------------------------------------------------
//! This method is not called from anywhere, because it provides compile-time asserts
//! for all enumerations that ECDb persists. This is a safe-guard to notify us whenever
//! somebody changes the enums. We then need to react to that so that old files don't get hornswaggled
// @bsimethod                                                    Krischan.Eberle  04/2016
//---------------------------------------------------------------------------------------
void AssertPersistedEnumsAreUnchanged()
    {
    static_assert((int) CustomAttributeContainerType::Any == 4095 &&
                  (int) CustomAttributeContainerType::AnyClass == 30 &&
                  (int) CustomAttributeContainerType::AnyProperty == 992 &&
                  (int) CustomAttributeContainerType::AnyRelationshipConstraint == 3072 &&
                  (int) CustomAttributeContainerType::ArrayProperty == 128 &&
                  (int) CustomAttributeContainerType::CustomAttributeClass == 4 &&
                  (int) CustomAttributeContainerType::EntityClass == 2 &&
                  (int) CustomAttributeContainerType::NavigationProperty == 512 &&
                  (int) CustomAttributeContainerType::PrimitiveProperty == 32 &&
                  (int) CustomAttributeContainerType::RelationshipClass == 16 &&
                  (int) CustomAttributeContainerType::Schema == 1 &&
                  (int) CustomAttributeContainerType::SourceRelationshipConstraint == 1024 &&
                  (int) CustomAttributeContainerType::StructProperty == 64 &&
                  (int) CustomAttributeContainerType::StructArrayProperty == 256 &&
                  (int) CustomAttributeContainerType::StructClass == 8 &&
                  (int) CustomAttributeContainerType::TargetRelationshipConstraint == 2048, "Persisted Enum has changed: ECN::CustomAttributeContainerType.");

    static_assert((int) DbColumn::Kind::DataColumn == 512 &&
                  (int) DbColumn::Kind::ECClassId == 2 &&
                  (int) DbColumn::Kind::ECInstanceId == 1 &&
                  (int) DbColumn::Kind::RelECClassId == 2048 &&
                  (int) DbColumn::Kind::SharedDataColumn == 1024 &&
                  (int) DbColumn::Kind::SourceECClassId == 64 &&
                  (int) DbColumn::Kind::SourceECInstanceId == 32 &&
                  (int) DbColumn::Kind::TargetECClassId == 256 &&
                  (int) DbColumn::Kind::TargetECInstanceId == 128 &&
                  (int) DbColumn::Kind::Unknown == 0, "Persisted Enum has changed: DbColumn::Kind.");

    static_assert((int) DbColumn::Type::Any == 0 &&
                  (int) DbColumn::Type::Blob == 2 &&
                  (int) DbColumn::Type::Boolean == 1 &&
                  (int) DbColumn::Type::Integer == 5 &&
                  (int) DbColumn::Type::Real == 4 &&
                  (int) DbColumn::Type::Text == 6 &&
                  (int) DbColumn::Type::TimeStamp == 3, "Persisted Enum has changed: DbColumn::Type.");

    static_assert((int) DbTable::Type::Existing == 2 &&
                  (int) DbTable::Type::Joined == 1 &&
                  (int) DbTable::Type::Primary == 0, "Persisted Enum has changed: DbTable::Type.");

    static_assert((int) ECClassModifier::Abstract == 1 &&
                  (int) ECClassModifier::None == 0 &&
                  (int) ECClassModifier::Sealed == 2, "Persisted Enum has changed: ECN::ECClassModifier.");

    static_assert((int) ECClassType::CustomAttribute == 3 &&
                  (int) ECClassType::Entity == 0 &&
                  (int) ECClassType::Relationship == 1 &&
                  (int) ECClassType::Struct == 2, "Persisted Enum has changed: ECN::ECClassType.");

    static_assert((int) ECDbMapStrategy::Options::JoinedTable == 4 &&
                  (int) ECDbMapStrategy::Options::None == 0 &&
                  (int) ECDbMapStrategy::Options::ParentOfJoinedTable == 2 &&
                  (int) ECDbMapStrategy::Options::SharedColumns == 1, "Persisted Enum has changed: ECDbMapStrategy::Options.");

    static_assert((int) ECDbMapStrategy::Strategy::ExistingTable == 3 &&
                  (int) ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable == 101 &&
                  (int) ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable == 100 &&
                  (int) ECDbMapStrategy::Strategy::NotMapped == 0 &&
                  (int) ECDbMapStrategy::Strategy::OwnTable == 1 &&
                  (int) ECDbMapStrategy::Strategy::SharedTable == 2, "Persisted Enum has changed: ECDbMapStrategy::Strategy.");

    static_assert((int) ECPropertyKind::Navigation == 4 &&
                  (int) ECPropertyKind::Primitive == 0 &&
                  (int) ECPropertyKind::PrimitiveArray == 2 &&
                  (int) ECPropertyKind::Struct == 1 &&
                  (int) ECPropertyKind::StructArray == 3, "Persisted Enum has changed: ECPropertyKind.");

    static_assert((int) StrengthType::Embedding == 2 &&
                  (int) StrengthType::Holding == 1 &&
                  (int) StrengthType::Referencing == 0, "Persisted Enum has changed: ECN::StrengthType.");

    static_assert((int) ECRelatedInstanceDirection::Backward == 2 &&
                  (int) ECRelatedInstanceDirection::Forward == 1, "Persisted Enum has changed: ECN::ECRelatedInstanceDirection.");

    static_assert((int) ECRelationshipEnd::ECRelationshipEnd_Source == 0 &&
                  (int) ECRelationshipEnd::ECRelationshipEnd_Target == 1, "Persisted Enum has changed: ECN::ECRelationshipEnd.");

    static_assert((int) ForeignKeyDbConstraint::ActionType::Cascade == 1 &&
                  (int) ForeignKeyDbConstraint::ActionType::NoAction == 2 &&
                  (int) ForeignKeyDbConstraint::ActionType::NotSpecified == 0 &&
                  (int) ForeignKeyDbConstraint::ActionType::Restrict == 5 &&
                  (int) ForeignKeyDbConstraint::ActionType::SetDefault == 4 &&
                  (int) ForeignKeyDbConstraint::ActionType::SetNull == 3, "Persisted Enum has changed: ForeignKeyDbConstraint::ActionType.");

    static_assert((int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class == 30 &&
                  (int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property == 992 &&
                  (int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema == 1 &&
                  (int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint == 1024 &&
                  (int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint == 2048, "Persisted Enum has changed: ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType.");

    static_assert((int) PrimitiveType::PRIMITIVETYPE_Binary == 0x101 &&
                  (int) PrimitiveType::PRIMITIVETYPE_Boolean == 0x201 &&
                  (int) PrimitiveType::PRIMITIVETYPE_DateTime == 0x301 &&
                  (int) PrimitiveType::PRIMITIVETYPE_Double == 0x401 &&
                  (int) PrimitiveType::PRIMITIVETYPE_IGeometry == 0xa01 &&
                  (int) PrimitiveType::PRIMITIVETYPE_Integer == 0x501 &&
                  (int) PrimitiveType::PRIMITIVETYPE_Long == 0x601 &&
                  (int) PrimitiveType::PRIMITIVETYPE_Point2D == 0x701 &&
                  (int) PrimitiveType::PRIMITIVETYPE_Point3D == 0x801 &&
                  (int) PrimitiveType::PRIMITIVETYPE_String == 0x901, "Persisted Enum has changed: ECN::PrimitiveType.");
    }

END_BENTLEY_SQLITE_EC_NAMESPACE