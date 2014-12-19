/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbTable.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
const std::map<ECN::PrimitiveType, Utf8String> DbTable::s_ecsqliteTypeMap =
    { 
    //! These are chosen to map well to SQLite, System.Data.SQLite, EC, and ODBC
    std::make_pair (PRIMITIVETYPE_DbKey, "INTEGER"), // SQLite Affinity: INTEGER (A "Fake" PRIMITIVETYPE to distinguish key columns)
    std::make_pair (PRIMITIVETYPE_Boolean, "BOOL"), // SQLite Affinity: NUMERIC
    std::make_pair (PRIMITIVETYPE_Binary, "BLOB"), // SQLite Affinity: NONE
    std::make_pair (PRIMITIVETYPE_DateTime, "TIMESTAMP"), // SQLite Affinity: NUMERIC
    std::make_pair (PRIMITIVETYPE_Double, "DOUBLE"), // SQLite Affinity: REAL
    std::make_pair (PRIMITIVETYPE_Integer, "INTEGER"), // SQLite Affinity: INTEGER
    std::make_pair (PRIMITIVETYPE_Long, "BIGINT"), // SQLite Affinity: INTEGER
    std::make_pair (PRIMITIVETYPE_String, "CHAR"), // SQLite Affinity: TEXT 
    std::make_pair (PRIMITIVETYPE_IGeometry, "BLOB") // SQLite Affinity: NONE 
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DbTable::Dump (Utf8StringR output) const
    {
    output.append ("Table: [").append (m_tableName).append("]\r\n");
    output.append ("  IsVirtual          : ").append ((this->m_isVirtual? "True" : "False")).append("\r\n");
    output.append ("  MapToExistingTable : ").append ((this->m_mapToExistingTable ? "True" : "False")).append ("\r\n");
    output.append ("  Columns            : ").append ("\r\n");
    for (auto column : GetColumns ())
        {
        output.append ("    ")
            .append (column->GetName())
            .append ( " : ")
            .append (GetSqliteType(*column));

        //TODO: Added rest of column properties
        output.append ("\n");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbTable::DbTable (Utf8CP tableName, bool isVirtual, bool mapToExistingTable, bool allowReplacingEmptyTableWithView) 
    : m_tableName (tableName), m_isVirtual (isVirtual), m_mapToExistingTable (mapToExistingTable), m_replaceEmptyTableWithEmptyView (allowReplacingEmptyTableWithView),
    m_afterSetClassIdHook (nullptr)
    {
    AddConstraint (*DbPrimaryKeyTableConstraint::Create());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbTable::~DbTable ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbTable::IsNullTable () const
    {
    return Utf8String::IsNullOrEmpty (m_tableName.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbTablePtr DbTable::Create (Utf8CP tableName, bool mapToSecondaryTable, bool isVirtual, Utf8CP primaryKeyColumnName, bool mapToExistingTable, bool allowReplacingEmptyTableWithView)
    {
    DbTablePtr dbTable = new DbTable (tableName, isVirtual, mapToExistingTable, allowReplacingEmptyTableWithView);

    //don't add PK if this is intended to become the null table (i.e. table name is empty)
    if (!dbTable->IsNullTable ())
        {
        dbTable->GeneratePrimaryKeyColumns (primaryKeyColumnName, mapToSecondaryTable);
        }

    return dbTable;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void DbTable::AppendColumnDDL( Utf8StringR sql, DbColumnCR column, bool firstColumn) const
    {
    if (column.IsVirtual())
        return;

    if (!firstColumn)

        sql.append (", ");
    sql.append ("[");
    sql.append (column.GetName());
    sql.append ("] ");

    Utf8String sqliteTypeName = GetSqliteType (column);
    sql.append (sqliteTypeName);

    if (!column.GetNullable())
        sql.append(" not null ");
        
    if (column.GetUnique())
        sql.append(" unique ");

    if (column.GetCollate() != Collate::Default && column.GetColumnType() == PRIMITIVETYPE_String)
        {
        switch(column.GetCollate())
            {
            case Collate::Binary: sql.append(" collate binary "); break;
            case Collate::NoCase: sql.append(" collate nocase "); break;
            case Collate::RTrim:  sql.append(" collate rtrim  "); break;
            }
        }
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbTable::HasPendingModifications(BeSQLiteDbR db)
    {
    bvector<Utf8String> existingDbColumns;
    if (!db.GetColumns(existingDbColumns, GetName()))
        {
        LOG.errorv("Failed to get column list for table %s", m_tableName.c_str());
        BeAssert (false && "Failed to get column list for table");
        return BE_SQLITE_ERROR;
        }

    //Create a fast hash set of existing db column list
    std::set<Utf8String> existingColumnSet;
    for(auto& existingDbColumn : existingDbColumns)
        {
        existingColumnSet.insert(existingDbColumn);
        }

    //Create a fast hash set of in-memory column list;
    std::map<Utf8CP, DbColumnCP, CompareIUtf8> modifiedColumnSet;
    for(auto column : GetColumns ())
        {
        modifiedColumnSet [column->GetName()] = column;
        }

    //compute new columns;
    for (auto const& modifiedColumn : modifiedColumnSet)
        {
        if (existingColumnSet.find(modifiedColumn.first) == existingColumnSet.end())
            return true;
        }

    //compute deleted columns;
    for (auto const& existingDbColumn : existingDbColumns)
        {
        if (modifiedColumnSet.find(existingDbColumn.c_str ()) == modifiedColumnSet.end())
            return true;
        }
    return false;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbTable::SyncWithDb (BeSQLiteDbR db)
    {
    auto type = DbMetaDataHelper::GetObjectType(db, GetName());
    //! Table/View doesnt exist in db. Action = Create it
    if (type == DbMetaDataHelper::ObjectType::None)
        {
        //could not find table so create it;
        return CreateTableInDb(db);
        }

    if (IsVirtual ())
        return DbResult::BE_SQLITE_OK;

    //! Object type is view and exist in db. Action = DROP it and recreate it.
    if (type == DbMetaDataHelper::ObjectType::View)
        {

        auto r = db.ExecuteSql(SqlPrintfString("DROP VIEW %s", GetName()));
        if ( r != BE_SQLITE_OK)
            {
            LOG.errorv("Failed to drop view '%s'", GetName());
            return r;
            }
        return CreateEmptyView(db);
        }
    //! Object type is view and exist in db. Action = DROP it and recreate it.
    if (type == DbMetaDataHelper::ObjectType::Table)
        {
        //Add new columns to table.
        //Strategies
        //1. Create backup of table and drop the main table. Then recreat it and re-insert data in it.
        //2. Add new columns and indexes on then table.
        //Here we are going with 2. Though 1. is far better solution.
        //ALTER /rename
        //Process NEW Columns add to db.

        bvector<Utf8String> existingDbColumns;
        if (!db.GetColumns(existingDbColumns, GetName()))
            {
            LOG.errorv("Failed to get column list for table %s", m_tableName.c_str());
            BeAssert (false && "Failed to get column list for table");
            return BE_SQLITE_ERROR;
            }

        //Create a fast hash set of existing db column list
        std::set<Utf8String> existingColumnSet;
        for(auto& existingDbColumn : existingDbColumns)
            {
            existingColumnSet.insert(existingDbColumn);
            }

        //Create a fast hash set of in-memory column list;
        std::map<Utf8CP, DbColumnCP, CompareIUtf8> modifiedColumnSet;
        for(auto column : GetColumns ())
            {
            modifiedColumnSet [column->GetName()] = column;
            }

        std::vector<Utf8CP> newColumns;
        //compute new columns;
        for (auto const& modifiedColumn : modifiedColumnSet)
            {
            if (existingColumnSet.find(modifiedColumn.first) == existingColumnSet.end())
                newColumns.push_back(modifiedColumn.first);
            }

        std::vector<Utf8String> deleteColumns;
        //compute deleted columns;
        for (auto const& existingDbColumn : existingDbColumns)
            {
            if (modifiedColumnSet.find(existingDbColumn.c_str ()) == modifiedColumnSet.end())
                deleteColumns.push_back(existingDbColumn);
            }

        //If we do not have any deleted column then we do not need the complicated operation of recreating table
        if (deleteColumns.empty())
            {
            return PersistNewColumns (newColumns, modifiedColumnSet, db);
            }
        else
            {
            //Recreate the table and move rows to it.
            // get rid of indexes. NONE UNIQUE index are global and need to be remove so new table is create without a error
            for(DbIndexPtr& index: m_tableIndices)
                {        
                auto type = DbMetaDataHelper::GetObjectType(db, index->GetName(true));
                if (type == DbMetaDataHelper::ObjectType::Index)
                    {
                    auto r = db.ExecuteSql (SqlPrintfString("DROP INDEX IF EXISTS [%s]", index->GetName(true)));
                    if (BE_SQLITE_OK != r)
                        {
                        LOG.errorv("Failed to drop index %s on table %s", index->GetName(), GetName());
                        BeAssert(false && "Failed to drop  index");
                        }
                    }
                }

            Utf8String workingTableName = "temp_ecdb_" + m_tableName;
            if (!db.RenameTable (m_tableName.c_str(), workingTableName.c_str()))
                {
                LOG.errorv("Failed to rename table %s to %s", m_tableName.c_str(), workingTableName.c_str());
                BeAssert (false && "Failed to rename the table");
                return BE_SQLITE_ERROR;
                }

            auto status = CreateTableInDb(db);
            if (status != BE_SQLITE_OK)
                {
                LOG.errorv("Failed to create table %s", m_tableName.c_str());
                BeAssert(false && "Failed to create table");
                return status;
                }

            bvector<Utf8String> columnList;
            if (!db.GetColumns(columnList, GetName()))
                {
                LOG.errorv("Failed to get column list for table %s", m_tableName.c_str());
                BeAssert (false && "Failed to get column list for table");
                return BE_SQLITE_ERROR;
                }

            columnList.erase(
                std::remove_if (columnList.begin (), columnList.end (),
                [&] ( const Utf8String& v)
                    { 
                    for (auto const& col : newColumns)
                        { 
                        if (v.EqualsI (col))
                            return true;
                        };                             
                    return false; 
                    }
            ), columnList.end());

            //Remove new columns from the list
            status = CopyRows (db, workingTableName.c_str(), columnList, GetName(), columnList);
            if (status != BE_SQLITE_OK)
                {
                LOG.errorv("Failed to copy rows from %s to %s", workingTableName.c_str(), m_tableName.c_str());
                BeAssert(false && "Failed to copy rows");
                return status;
                }
            
            status = db.DropTable(workingTableName.c_str());
            if (status != BE_SQLITE_OK)
                {
                LOG.errorv("Failed to drop temprory working table %s", workingTableName.c_str());
                BeAssert(false && "Failed to drop temprory working table ");
                return status;
                }
            }
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbTable::PersistNewColumns (std::vector<Utf8CP> const& newColumns, std::map<Utf8CP, DbColumnCP, CompareIUtf8> const& modifiedColumnSet, BeSQLiteDbR &db) const
    {
    if (newColumns.empty ())
        return BE_SQLITE_OK;

    Utf8String alterDDLTemplate;
    alterDDLTemplate.Sprintf ("ALTER TABLE [%s] ADD COLUMN ", GetName ());
    for (auto newColumn : newColumns)
        {
        //Limitation of ADD COLUMN http://www.sqlite.org/lang_altertable.html
        auto alterDDL = alterDDLTemplate;
        auto it = modifiedColumnSet.find (newColumn);
        BeAssert (it != modifiedColumnSet.end ());
        auto newColumnDef = it->second;
        BeAssert (newColumnDef != nullptr);
        AppendColumnDDL (alterDDL, *newColumnDef, true);
        auto r = db.ExecuteSql (alterDDL.c_str ());
        if (r != BE_SQLITE_OK)
            {
            LOG.errorv ("Failed to add new column. SQL: %s", alterDDLTemplate.c_str ());
            return r;
            }
        }

    return BE_SQLITE_OK;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbTable::CopyRows (BeSQLiteDbR db, Utf8CP sourceTable, bvector<Utf8String>& sourceColumns, Utf8CP targetTable, bvector<Utf8String>& targetColumns)
    { 
    Utf8String sourceColumnList;
    for(auto& column : sourceColumns)
        {
        sourceColumnList.append("[");
        sourceColumnList.append(column);
        sourceColumnList.append("]");
        if (column != *(sourceColumns.end() - 1))
            sourceColumnList.append(",");
        }

    Utf8String targetColumnList;
    for(auto& column : targetColumns)
        {
        targetColumnList.append("[");
        targetColumnList.append(column);
        targetColumnList.append("]");
        if (column != *(targetColumns.end() - 1))
            targetColumnList.append(",");
        }

    Utf8String sql;
    sql.Sprintf("INSERT INTO [%s] (%s) SELECT %s FROM %s", targetTable, targetColumnList.c_str(),  sourceColumnList.c_str(), sourceTable);
    LOG.infov("Copying rows from table %s to %s", sourceTable, targetTable);
    return db.ExecuteSql(sql.c_str());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbTable::CreateTableInDb (BeSQLiteDbR db)
    {    
    if (IsVirtual ( ))
        return DbResult::BE_SQLITE_OK;

    Utf8String sql;
    GetCreateTableSQL(sql);
    DbResult r = db.ExecuteSql (sql.c_str());
    if (BE_SQLITE_OK != r)
        {
        LOG.errorv ("SQL failed: %s", sql.c_str ());
        return r;
        }

    //Create indicies
    for (DbIndexPtr index : m_tableIndices)
        {
        if (index->BuildCreateIndexSql(sql))
            {
            DbResult r = db.ExecuteSql (sql.c_str());
            if (BE_SQLITE_OK != r)
                LOG.errorv("SQL failed: %s", sql.c_str());
            }
        else
            {
            LOG.errorv("Failed to generate SQL to create index %s on table %s", index->GetName(), GetName());
            }
        }

    return r;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DbTable::GetName() const
    {
    return m_tableName.c_str();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbColumnCP DbTable::GetClassIdColumn() const
    {
    return m_classIdColumn.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle      04/2014
//+---------------+---------------+---------------+---------------+---------------+------
DbColumnPtr DbTable::GetClassIdColumnPtr () const
    {
    return m_classIdColumn;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void DbTable::GeneratePrimaryKeyColumns (Utf8CP primaryKeyColumnName, bool mapToSecondaryTable)
    {
    auto& primaryKeyConstraint = GetPrimaryKeyTableConstraint ();

    if (!mapToSecondaryTable)
        {
        if (nullptr == primaryKeyColumnName)  // WIP_ECDB: figure out the table name before you call this method
            primaryKeyColumnName = ECDB_COL_ECInstanceId;

        //not null and unique constraints are set to false here, because the PK constraint will be added separately
        //which includes non null and unique
        auto primaryKeyColumn = DbColumn::Create (primaryKeyColumnName, PRIMITIVETYPE_DbKey, false, false);
        GetColumnsR ().Add (primaryKeyColumn);
        primaryKeyConstraint.AddColumn (*primaryKeyColumn);
        }
    else
        {
        //Array instance is identified with ECDB_COL_ECInstanceId (or hint-supplied column name),ECDB_COL_ECArrayIndex and ECDB_COL_ECPropertyId
        if (nullptr == primaryKeyColumnName)   // WIP_ECDB: figure out the table name before you call this method
            primaryKeyColumnName = ECDB_COL_ECInstanceId;

        auto primaryKeyColumn = DbColumn::Create (primaryKeyColumnName, PRIMITIVETYPE_DbKey, false, false);
        GetColumnsR ().Add (primaryKeyColumn);
        primaryKeyConstraint.AddColumn (*primaryKeyColumn);

        auto parentECIdColumn = DbColumn::Create (ECDB_COL_OwnerECInstanceId, PRIMITIVETYPE_DbKey, true, false);
        GetColumnsR ().Add (parentECIdColumn);
        primaryKeyConstraint.AddColumn (*parentECIdColumn);

        auto propertyIdColumn = DbColumn::Create (ECDB_COL_ECPropertyId, PRIMITIVETYPE_DbKey, true, false);
        GetColumnsR ().Add (propertyIdColumn);
        primaryKeyConstraint.AddColumn (*propertyIdColumn);

        auto arrayIndexColumn = DbColumn::Create (ECDB_COL_ECArrayIndex, PRIMITIVETYPE_Integer, true, false);
        GetColumnsR ().Add (arrayIndexColumn);
        primaryKeyConstraint.AddColumn (*arrayIndexColumn);
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbColumnCP DbTable::GenerateClassIdColumn()
    {
    if (m_classIdColumn.IsValid())
        return m_classIdColumn.get();

    if (GetColumns ().Contains (ECDB_COL_ECClassId))
        {
        LOG.errorv ("DbTable '%s' already contains a column named '" ECDB_COL_ECClassId "'", GetName ());
        return nullptr;
        }

    auto classIdCol = DbColumn::CreateClassId();
    AddClassIdColumn (classIdCol);
    DbIndexPtr indexOnClassId = DbIndex::Create (nullptr, *this, false);
    indexOnClassId->AddColumn (*classIdCol);
    AddIndex(*indexOnClassId);
    
    return classIdCol.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbPrimaryKeyTableConstraintR DbTable::GetPrimaryKeyTableConstraint()const
    {
    DbPrimaryKeyTableConstraintP primaryKeyConstraint = nullptr;
    for (DbTableConstraintPtr constraint : m_tableConstraints)
        {
        if ((primaryKeyConstraint = dynamic_cast<DbPrimaryKeyTableConstraintP>(constraint.get ())) != nullptr)
             return *primaryKeyConstraint;
        }
    BeAssert (false);
    return *primaryKeyConstraint;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbTable::GetCreateTableSQL (Utf8StringR sql)
    {
    sql = "create table [";
    sql.append (GetName ());
    sql.append ("] (");

    bvector<DbColumnP> primaryKeyColumns;
    GetPrimaryKeyTableConstraint ().GetColumns (primaryKeyColumns);
    for (DbColumnP primaryKeyColumn : primaryKeyColumns)
        if (primaryKeyColumn != nullptr)
            AppendColumnDDL (sql, *primaryKeyColumn, primaryKeyColumn == primaryKeyColumns.front ());

    auto classIdColumn = GetClassIdColumnPtr ();
    if (classIdColumn != nullptr)
        AppendColumnDDL (sql, *classIdColumn);

    for (auto column : GetColumns ())
        {
        bool skipColumn = false;
        for (DbColumnPtr primaryKeyColumn : primaryKeyColumns)
            if (primaryKeyColumn.get () == column || classIdColumn.get () == column)
                {
                skipColumn = true;
                break; // skip columns that we've already added
                }
        if (skipColumn || column->IsVirtual ())
            continue;

        AppendColumnDDL (sql, *column);
        }

    //Add classId temprory to primary key constraint
    if (classIdColumn != nullptr)
        GetPrimaryKeyTableConstraint ().AddColumn (*classIdColumn);

    //Add Table constraints
    bool firstOne = true;
    for (bvector<DbTableConstraintPtr>::iterator itr = m_tableConstraints.begin (); itr != m_tableConstraints.end (); itr++)
        {
        Utf8String constraintSql;
        if ((*itr)->GenerateSQLClause (constraintSql))
            {
            if (firstOne)
                {
                sql.append (", ");
                firstOne = false;
                }
            sql.append (constraintSql);
            if (itr != (m_tableConstraints.end () - 1))
                sql.append (", ");
            }
        }
    //remove classid from primary key constraint
    if (classIdColumn != nullptr)
        GetPrimaryKeyTableConstraint ().RemoveColumn (*classIdColumn);

    sql.append (")");
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbTable::AddConstraint(DbTableConstraintR constraint)
    {
    m_tableConstraints.push_back(&constraint);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DbTable::AddIndex(DbIndexR index)
    {
    auto existingIndex = GetIndexByName (index.GetName (true));
    if (existingIndex != nullptr)
        {
        Utf8String existingCreateIndexSql;
        existingIndex->BuildCreateIndexSql (existingCreateIndexSql);
        Utf8String newCreateIndexSql;
        index.BuildCreateIndexSql (newCreateIndexSql);

        if (existingCreateIndexSql.CompareToI (newCreateIndexSql) == 0)
            return SUCCESS;
        else
            {
            LOG.errorv ("Index %s already exists in DbTable %s but has a different definition. Index names must be case insensitive.", index.GetName (), GetName ());
            BeAssert (false && "Index with same name but different definition already defined in the DbTable. Index names must be case insensitive.");
            return ERROR;
            }
        }

    m_tableIndices.push_back(&index);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle      04/2014
//+---------------+---------------+---------------+---------------+---------------+------
void DbTable::TryAssign (bool newIsVirtual, bool newAllowToReplaceEmptyClassByEmptyView)
    {
    //If requested to make the table non-virtual, do it. A virtual table is not persisted and therefore
    //for two classes sharing the table non-virtuality is stronger than virtuality.
    if (!newIsVirtual && m_isVirtual != newIsVirtual)
        {
        LOG.debugv ("Multiple classes map to DbTable %s with one of the classes requiring a non-virtual table. Therefore DbTable::IsVirtual has been changed to false.", GetName ());
        m_isVirtual = newIsVirtual;
        }

    //If requested to not allow to replace the empty table, assign it. 
    //For two classes sharing the table not replacing the table is stronger than replacing it
    if (!newAllowToReplaceEmptyClassByEmptyView && m_replaceEmptyTableWithEmptyView != newAllowToReplaceEmptyClassByEmptyView)
        {
        LOG.debugv ("Multiple classes map to DbTable %s with one of the classes not allowing the empty table to be replaced by a view. Therefore DbTable::IsAllowedToReplaceEmptyTableWithEmptyView has been changed to false.", GetName ());
        m_replaceEmptyTableWithEmptyView = newAllowToReplaceEmptyClassByEmptyView;
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbIndexCP DbTable::GetIndexByName(Utf8CP indexName)
    {
    for (DbIndexPtr& index : m_tableIndices)
        {
        if (0 == BeStringUtilities::Stricmp (index->GetName (), indexName))
            return index.get ();
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle      12/2012
//+---------------+---------------+---------------+---------------+---------------+------
bool DbTable::IsEmpty (BeSQLiteDbR db, bool treatAsEmptyOnError) const
    {
    PRECONDITION (!m_tableName.empty (), treatAsEmptyOnError);

    Utf8CP tableName = GetName ();

    Utf8String sql;
    sql.Sprintf ("SELECT count(*) FROM [%s]", tableName);

    Statement statement;
    DbResult stat = statement.TryPrepare (db, sql.c_str ());
    //if preparation fails, it might be because the table doesn't exist. Return the value
    //the user specified.
    //Same procedure for any other error occurred when preparing the statement
    if (stat != BE_SQLITE_OK)
        {
        return treatAsEmptyOnError;
        }

    stat = statement.Step ();
    POSTCONDITION (stat == BE_SQLITE_ROW, treatAsEmptyOnError);
    return statement.GetValueInt (0) == 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle      12/2012
//+---------------+---------------+---------------+---------------+---------------+------
DbResult DbTable::CreateEmptyView (BeSQLiteDbR db) const
    {
    if (GetColumns ().IsEmpty ())
        return BE_SQLITE_OK;

    Utf8String sql;
    sql.Sprintf ("CREATE VIEW [%s] AS SELECT ", GetName ());

    //TODO: This is a copy and paste hack from GetCreateTableSql to ensure
    //that the columns in the view are in the same order as in the table.
    //This needs to be cleaned-up after the graphite01 release.
    bvector<DbColumnP> primaryKeyColumns;
    GetPrimaryKeyTableConstraint().GetColumns(primaryKeyColumns);
    for (DbColumnP primaryKeyColumn : primaryKeyColumns)
        {
        if (primaryKeyColumn != nullptr)
            {
            AppendColumnToCreateEmptyViewSql (sql, *primaryKeyColumn);
            }
        }

    auto classIdColumn = GetClassIdColumn();
    if (classIdColumn != nullptr)
        {
        AppendColumnToCreateEmptyViewSql (sql, *classIdColumn);
        }

    //now iterate over all columns skipping the PKs and class id columns which were already added
    for (auto column : GetColumns ())
        {
        if (classIdColumn == column)
            {
            continue;
            }

        bool skipColumn = false;
        for (auto primaryKeyColumn : primaryKeyColumns)
            {
            if (primaryKeyColumn == column)
                {
                skipColumn = true;
                break; // skip columns that we've already added
                }
            }

        if (skipColumn || column->IsVirtual())
            {
            continue;
            }

        AppendColumnToCreateEmptyViewSql (sql, *column);
        }
    //end of copy and paste hack

    sql.erase (sql.end() - 2); //delete last comma and space

    //the above definition of the view without a limit clause would always return one row. But we require an empty view,
    //so the limit clause is appended.
    sql.append (" LIMIT 0");

    DbResult r = db.TryExecuteSql (sql.c_str ());
    if (r != BE_SQLITE_OK)
        LOG.errorv ("Failed to create empty view %s. Error: %s - %s", sql.c_str (), Db::InterpretDbResult (r), db.GetLastError (nullptr));

    return r;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle      12/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
void DbTable::AppendColumnToCreateEmptyViewSql (Utf8StringR createEmptyViewSql, DbColumnCR column)
    {
    Utf8CP columnDDLTemplate = "NULL AS [%s], ";
    
    Utf8String columnDDL;
    columnDDL.Sprintf (columnDDLTemplate, column.GetName ());
    createEmptyViewSql.append (columnDDL.c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle      12/2013
//+---------------+---------------+---------------+---------------+---------------+------
void DbTable::AddClassIdColumn (DbColumnPtr classIdColumn)
    {
    BeAssert (!GetColumns ().Contains (classIdColumn->GetName ()) && m_classIdColumn == nullptr);
    GetColumnsR ().Add (classIdColumn);
    m_classIdColumn = classIdColumn;
    if (m_afterSetClassIdHook != nullptr)
        m_afterSetClassIdHook (classIdColumn);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle      12/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8String DbTable::GetSqliteType (DbColumnCR column)
    {
    auto it = s_ecsqliteTypeMap.find (column.GetColumnType ());
    if (s_ecsqliteTypeMap.end () == it)
        {
        LOG.errorv ("Column %s has type ECN::PrimitiveType x%x, which cannot be mapped to a SQLite data type.", column.GetName (), column.GetColumnType ());
        return ""; // no mapping found
        }

    return it->second;
    }

//*********************************************************************************************
//  DbTable::DbColumnCollection
//*********************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2014
//+---------------+---------------+---------------+---------------+---------------+------
void DbTable::DbColumnCollection::Add (DbColumnPtr& newColumn)
    {
    BeAssert (newColumn != nullptr);
    if (newColumn == nullptr)
        return;

    BeAssert (!Contains (newColumn->GetName ()));
    m_columns[newColumn->GetName ()] = newColumn;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool DbTable::DbColumnCollection::Contains (Utf8CP name) const
    {
    ColumnMap::const_iterator it;
    return TryGet (it, name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2014
//+---------------+---------------+---------------+---------------+---------------+------
DbColumnCP DbTable::DbColumnCollection::Get (Utf8CP name) const
    {
    auto colPtr = GetPtr (name);
    if (colPtr == nullptr)
        return nullptr;

    return colPtr.get ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2014
//+---------------+---------------+---------------+---------------+---------------+------
DbColumnPtr DbTable::DbColumnCollection::GetPtr (Utf8CP name) const
    {
    ColumnMap::const_iterator it;
    if (!TryGet (it, name))
        return nullptr;

    return it->second;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool DbTable::DbColumnCollection::TryGet (ColumnMap::const_iterator& it, Utf8CP name) const
    {
    it = m_columns.find (name);
    return it != m_columns.end ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool DbTable::DbColumnCollection::IsEmpty () const
    {
    return m_columns.empty ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2014
//+---------------+---------------+---------------+---------------+---------------+------
size_t DbTable::DbColumnCollection::Size () const
    {
    return m_columns.size ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2014
//+---------------+---------------+---------------+---------------+---------------+------
DbTable::DbColumnCollection::const_iterator DbTable::DbColumnCollection::begin () const
    {
    return DbColumnCollection::const_iterator (m_columns.begin ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2014
//+---------------+---------------+---------------+---------------+---------------+------
DbTable::DbColumnCollection::const_iterator DbTable::DbColumnCollection::end () const
    {
    return DbColumnCollection::const_iterator (m_columns.end ());
    }

//*********************************************************************************************
//  DbConstraint
//*********************************************************************************************
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbConstraint::DbConstraint()
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbConstraint::AppendConstraintNameIfAvaliable(Utf8StringR sql)
    {
    if (!Utf8String::IsNullOrEmpty (m_name.c_str ()))
        sql.append (m_name);
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR DbConstraint::GetName () const 
    { 
    return m_name;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void         DbConstraint::SetName (Utf8CP constraintName) 
    { 
    m_name = constraintName;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbConstraint::GenerateSQLClause(Utf8StringR sqlClause)
    {
    return _BuildSqlClause(sqlClause);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbTableConstraint::DbTableConstraint()
    {
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbTableConstraint::AppendColumnList(Utf8StringR sql)
    {
    sql.append(" (");
    for(bvector<DbColumnPtr>::iterator itr = m_columns.begin(); itr!= m_columns.end(); itr++)
        {
        sql.append((*itr)->GetName());
        if ( itr != (m_columns.end() - 1))
            sql.append(", ");
        }
    sql.append(")");
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbTableConstraint::GetColumns(bvector<DbColumnP>& columns)const
    {
    columns.clear();
    for (DbColumnPtr column : m_columns)
        columns.push_back(column.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2013
//+---------------+---------------+---------------+---------------+---------------+------
void DbTableConstraint::GetColumns(DbColumnPtrList& columns)const
    {
    //return a copy of the column list
    columns = m_columns;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbTableConstraint::AddColumn(DbColumnR column) 
    {
    m_columns.push_back(&column);
    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbTableConstraint::RemoveColumn(DbColumnR column)
    {
    for(bvector<DbColumnPtr>::iterator itor = m_columns.begin(); itor != m_columns.end(); ++itor)
        if ((*itor).get() == &column)
            {
            itor = m_columns.erase(itor);
            return true;
            }
        return false;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DbTableConstraint::GetColumnCount() 
    { 
    return m_columns.size();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbPrimaryKeyTableConstraint::DbPrimaryKeyTableConstraint()
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbPrimaryKeyTableConstraint::_BuildSqlClause(Utf8StringR sql)
    {
    if (GetColumnCount() == 0 )
        return false;

    sql = "PRIMARY KEY ";
    AppendConstraintNameIfAvaliable (sql);
    AppendColumnList(sql);
    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbPrimaryKeyTableConstraintPtr DbPrimaryKeyTableConstraint::Create()
    {
    return new DbPrimaryKeyTableConstraint();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbUniqueTableConstraint::DbUniqueTableConstraint()
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbUniqueTableConstraint::_BuildSqlClause(Utf8StringR sql)
    {
    if (GetColumnCount() == 0 )
        return false;

    sql = "UNIQUE ";
    AppendConstraintNameIfAvaliable (sql);
    AppendColumnList(sql);
    return true;        
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbUniqueTableConstraintPtr DbUniqueTableConstraint::Create()
    {
    return new DbUniqueTableConstraint();
    }

//********************************************************************
// DbIndex
//********************************************************************

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbIndex::DbIndex (Utf8CP name, DbTableR table, bool isUnique) 
: m_name (name), m_table (&table), m_isUnique (isUnique)
    {}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbIndexPtr DbIndex::Create (Utf8CP name, DbTableR table, bool isUnique)
    {
    return new DbIndex (name, table, isUnique);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DbIndex::GetName (bool generateName) const
    {
    if (generateName && Utf8String::IsNullOrEmpty (m_name.c_str ()))
        GenerateIndexName (m_name);

    return m_name.c_str();
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbIndex::AddColumn(DbColumnCR column)
    {
    m_columns.push_back(const_cast<DbColumnP>(&column));
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbIndex::GenerateIndexName(Utf8StringR name) const
    {
    if (m_isUnique)
        name = "UIDX_";
    else
        name = "IDX_";
    name.append (m_table->GetName ());
    name.append ("_");
    bool isFirstColumn = true;
    for (auto& column : m_columns)
        {
        if (!isFirstColumn)
            name.append ("_");

        name.append (column->GetName ());

        isFirstColumn = false;
        }

    name.ReplaceAll (".", "_"); // WIP_ECDB: Is this really necessary?
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbIndex::BuildCreateIndexSql(Utf8StringR sql) const
    {
    PRECONDITION(m_columns.size() > 0 && "Index must have at least one column", false);
    sql = "CREATE ";
    if (m_isUnique)
        sql.append("UNIQUE ");
    sql.append("INDEX ");
    sql.append(GetName(true));
    sql.append(" ON ");
    sql.append(m_table->GetName());
    sql.append(" (");
    bool isFirstColumn = true;
    for (auto& column : m_columns)
        {
        if (!isFirstColumn)
            sql.append (", ");

        sql.append("[");
        sql.append(column->GetName());
        sql.append("]");

        isFirstColumn = false;
        }
    sql.append(")");
    return true;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

