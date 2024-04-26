/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

#define TABLE_SQLSCHEMA "sync_SqlSchema"
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
/*
SQL to reconstruct index from ec_tables


SELECT idx.id, idx.Name name, tb.Name tbl_name, 'index' type,
       FORMAT ('CREATE %s INDEX %s ON %s (%s)',
              IIF ([idx].[IsUnique], 'UNIQUE', ''),
              [idx].[Name],
              [tb].[Name],
              (SELECT GROUP_CONCAT ([col].[Name], ',')
                      FROM   [ec_IndexColumn] [ic]
                      JOIN [ec_Column] [col] ON [col].[Id] = [ic].[ColumnId]
                      AND [ic].[IndexId] = [idx].[Id]
                      ORDER  BY [ic].[Ordinal])) sql
FROM   [ec_index] [idx]
       JOIN [ec_table] [tb] ON [tb].[Id] = [idx].[TableId];
*/

//=======================================================================================
//     JsonNames
//+===============+===============+===============+===============+===============+======
//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct JsonNames {
    constexpr static char SyncId[] = "id";
    constexpr static char SyncDataVer[] = "dataVer";
    constexpr static char JNamespace[] = "ec_Db";
    constexpr static char JSyncDbInfo[] = "syncDbInfo";
    constexpr static char JLocalDbInfo[] = "localDbInfo";
};

//SchemaSyncHelper==============================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int SchemaSyncHelper::ForeignKeyCheck(DbCR conn, std::vector<std::string>const& tables, Utf8CP dbAlias) {
    int fkViolations = 0;
    for(auto& table : tables) {
        Statement stmt;
        stmt.Prepare(conn, SqlPrintfString("PRAGMA [%s].foreign_key_check(%s)", dbAlias, table.c_str()));
        while(BE_SQLITE_ROW == stmt.Step()) {
            ++fkViolations;
            printf("%s\n",
                SqlPrintfString("[table=%s], [rowid=%lld], [parent=%s], [fkid=%d]",
                                stmt.GetValueText(0),
                                stmt.GetValueInt64(1),
                                stmt.GetValueText(2),
                                stmt.GetValueInt(3))
                    .GetUtf8CP());
        }
    }
    return fkViolations;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::GetMetaTables(DbR conn, StringList& tables, Utf8CP dbAlias) {
    const auto queryECTableSql = Utf8String {
        SqlPrintfString(R"z(
            SELECT
                [name]
            FROM   [%s].[sqlite_master]
            WHERE  [tbl_name] LIKE 'ec\_%%' ESCAPE '\'
                    AND [type] = 'table'
        )z", dbAlias).GetUtf8CP()
    };

    Statement iuStmt;
    auto rc = iuStmt.Prepare(conn, queryECTableSql.c_str());
    if (rc != BE_SQLITE_OK) {
        return rc;
    }
    while((rc = iuStmt.Step()) == BE_SQLITE_ROW) {
        tables.push_back(iuStmt.GetValueText(0));
    }
    return rc == BE_SQLITE_DONE ? BE_SQLITE_OK : rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::DropDataTables(DbR conn) {
    if (!conn.TableExists("ec_Table")) {
        return BE_SQLITE_OK;
    }
    Statement stmt;
    StringList tables;
    auto rc = stmt.Prepare(conn, "SELECT [Name] FROM [ec_Table] WHERE [Type] IN (" SQLVAL_DbTable_Type_Primary "," SQLVAL_DbTable_Type_Joined "," SQLVAL_DbTable_Type_Overflow R"x() AND Name NOT LIKE 'ecdbf\_%' ESCAPE '\' ORDER BY [Type] DESC)x");
    if (rc != BE_SQLITE_OK) {
        return rc;
    }
    while(stmt.Step() == BE_SQLITE_ROW) {
        tables.push_back(stmt.GetValueText(0));
    }

    stmt.Finalize();
    for(auto& table : tables) {
        rc = conn.ExecuteSql(SqlPrintfString("DROP TABLE IF EXISTS [main].[%s];", table.c_str()).GetUtf8CP());
        if (rc != BE_SQLITE_OK) {
            return rc;
        }
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::DropMetaTables(DbR conn) {
    StringList tables;
    auto rc = GetMetaTables(conn, tables, "main");
    if (rc != BE_SQLITE_OK) {
        return rc;
    }
    std::reverse(tables.begin(), tables.end());
    for(auto& table: tables) {
        rc = conn.ExecuteSql(SqlPrintfString("DROP TABLE IF EXISTS [main].[%s];", table.c_str()).GetUtf8CP());
        if (rc != BE_SQLITE_OK) {
            return rc;
        }
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::CreateMetaTablesFrom(ECDbR fromDb, DbR syncDb) {
    const auto sql = Utf8String {R"z(
        SELECT
            [sql]
        FROM   [sqlite_master]
        WHERE  [tbl_name] LIKE 'ec\_%' ESCAPE '\'
                AND ([type] = 'table'
                OR [type] = 'index')
                AND [sql] IS NOT NULL
        ORDER  BY [RootPage];
    )z"};
    Statement stmt;
    auto rc = stmt.Prepare(fromDb, sql.c_str());
    if (rc != BE_SQLITE_OK) {
        return rc;
    }
    while ((rc = stmt.Step()) == BE_SQLITE_ROW) {
        const auto ddl = stmt.GetValueText(0);
        rc = syncDb.ExecuteSql(ddl);
        if(rc != BE_SQLITE_OK) {
            return rc;
        }
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::TryGetAttachDbs(AliasMap& aliasMap, ECDbR conn) {
    Statement stmt;
    auto rc = stmt.Prepare(conn, "pragma main.database_list");
    if (rc != BE_SQLITE_OK) {
        return rc;
    }

    while((rc = stmt.Step()) == BE_SQLITE_ROW) {
        const auto alias = stmt.GetValueText(1);
        const auto file = stmt.GetValueText(2);
        aliasMap.insert(make_bpair<Utf8String, Utf8String>(alias, file));
    }
    return rc == BE_SQLITE_DONE ? BE_SQLITE_OK : rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::VerifyAlias(ECDbR conn) {
    AliasMap aliasMap;
    auto rc = TryGetAttachDbs(aliasMap, conn);
    if (rc != BE_SQLITE_OK) {
        conn.GetImpl().Issues().Report(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0613,
            "Unable to query attach db from primary connection");
        return rc;
    }
    if (aliasMap.find(ALIAS_MAIN_DB) == aliasMap.end()) {
        conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0614,
            "Expecting '%s' attach db on primary connection", ALIAS_MAIN_DB);
        return rc;
    }

    if (aliasMap.find(ALIAS_SYNC_DB) != aliasMap.end()) {
        conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0615,
            "Db alias '%s' use by schema sync db is already in use", ALIAS_SYNC_DB);
        return rc;
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::GetColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, StringList& columnNames) {
    Statement stmt;
    const auto sql = Utf8String{SqlPrintfString("pragma %s.table_info(%s)", dbAlias, tableName).GetUtf8CP()};
    auto rc = stmt.Prepare(db, sql.c_str());
    if (BE_SQLITE_OK != rc)
        return rc;

    while((rc = stmt.Step()) == BE_SQLITE_ROW) {
        columnNames.push_back(stmt.GetValueText(1));
    }
    return rc == BE_SQLITE_DONE ? BE_SQLITE_OK : rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SchemaSyncHelper::Join(StringList const& list, Utf8String delimiter, Utf8String prefix, Utf8String postfix) {
    if (list.empty()) {
        return prefix + postfix;
    }
    return prefix + std::accumulate(
        std::next(list.begin()),
        std::end(list),
        Utf8String{list.front()},
        [&](Utf8String const& acc, const Utf8String& piece) {
            return acc + delimiter + piece;
        }
    ) + postfix;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SchemaSyncHelper::ToLower(Utf8String const& val) {
    Utf8String out;
    std::for_each(val.begin(), val.end(), [&](char const& ch) {
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    });
    return out;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::GetPrimaryKeyColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, StringList& columnNames) {
    Statement stmt;
    const auto sql = Utf8String{SqlPrintfString("pragma %s.table_info(%s)", dbAlias, tableName).GetUtf8CP()};
    auto rc = stmt.Prepare(db, sql.c_str());
    if (BE_SQLITE_OK != rc)
        return rc;

    while((rc = stmt.Step()) == BE_SQLITE_ROW) {
        if (stmt.GetValueInt(5) != 0) {
            columnNames.push_back(stmt.GetValueText(1));
        }
    }
    return rc == BE_SQLITE_DONE ? BE_SQLITE_OK : rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::SyncData(ECDbR conn, StringList const& tables, Utf8CP sourceDbAlias, Utf8CP targetDbAlias) {
    auto rc = conn.ExecuteSql("PRAGMA defer_foreign_keys=1");
    if (rc != BE_SQLITE_OK) {
        return rc;
    }
    for (auto& tbl : tables) {
        rc = SyncData(conn, tbl.c_str(), sourceDbAlias, targetDbAlias);
        if (rc != BE_SQLITE_OK) {
            return rc;
        }
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::SyncData(ECDbR conn, Utf8CP tableName, Utf8CP sourceDbAlias, Utf8CP targetDbAlias) {
    DbResult rc;
    auto sourceCols = StringList{};
    rc = GetColumnNames(conn, sourceDbAlias, tableName, sourceCols);
    if (BE_SQLITE_OK != rc)
        return rc;

    auto sourcePkCols = StringList{};
    rc = GetPrimaryKeyColumnNames(conn, sourceDbAlias, tableName, sourcePkCols);
    if (BE_SQLITE_OK != rc)
        return rc;

    auto targetCols = StringList{};
    rc = GetColumnNames(conn, targetDbAlias, tableName, targetCols);
    if (BE_SQLITE_OK != rc)
        return rc;

    auto targetPkCols = StringList{};
    rc = GetPrimaryKeyColumnNames(conn, targetDbAlias, tableName, targetPkCols);
    if (BE_SQLITE_OK != rc)
        return rc;

    std::sort(std::begin(sourceCols), std::end(sourceCols));
    std::sort(std::begin(sourcePkCols), std::end(sourcePkCols));
    std::sort(std::begin(targetCols), std::end(targetCols));
    std::sort(std::begin(targetPkCols), std::end(targetPkCols));

    const auto sourceColCount = sourceCols.size();
    const auto sourcePkColCount = sourcePkCols.size();

    if(sourceColCount != targetCols.size()) {
        return BE_SQLITE_SCHEMA;
    }
    if(sourcePkColCount != targetPkCols.size()) {
        return BE_SQLITE_SCHEMA;
    }
    for (auto i = 0; i < sourceColCount; ++i) {
        if (ToLower(sourceCols[i]) != ToLower(targetCols[i]))
            return BE_SQLITE_SCHEMA;
    }
    for (auto i = 0; i < sourcePkColCount; ++i) {
        if (ToLower(sourcePkCols[i]) != ToLower(targetPkCols[i]))
            return BE_SQLITE_SCHEMA;
    }

    const auto sourceTableSql = Utf8String {SqlPrintfString("[%s].[%s]", sourceDbAlias, tableName).GetUtf8CP()};
    const auto targetTableSql = Utf8String {SqlPrintfString("[%s].[%s]", targetDbAlias, tableName).GetUtf8CP()};
    const auto sourceColsSql = Join(sourceCols);
    const auto targetColsSql = Join(targetCols);

    StringList setClauseExprs;
    StringList delClauseExprs;
    for(auto& col : targetCols) {
        setClauseExprs.push_back(SqlPrintfString("%s=excluded.%s", col.c_str(), col.c_str()).GetUtf8CP());
    }
    for(auto& col : sourcePkCols) {
        delClauseExprs.push_back(SqlPrintfString("[T].%s=[S].%s", col.c_str(), col.c_str()).GetUtf8CP());
    }

    const auto updateColsSql = Join(setClauseExprs);
    const auto targetPkColsSql = Join(targetPkCols);
    const auto deleteColsSql = Join(delClauseExprs, " AND ");

    const auto allowDelete = Utf8String(tableName).StartsWith("ec_");
    if (allowDelete) {
        const auto deleteTargetSql = Utf8String {
            SqlPrintfString("DELETE FROM %s AS [T] WHERE NOT EXISTS (SELECT 1 FROM %s [S] WHERE %s)",
                targetTableSql.c_str(),
                sourceTableSql.c_str(),
                deleteColsSql.c_str()
                ).GetUtf8CP()
            };

        Statement stmt;
        rc = stmt.Prepare(conn, deleteTargetSql.c_str());
        if (rc != BE_SQLITE_OK) {
            return rc;
        }
        rc = stmt.Step();
        if (rc != BE_SQLITE_DONE) {
            return rc;
        }
    }

    Utf8String sql = SqlPrintfString(
        "insert into %s(%s) select %s from %s where 1 on conflict do update set %s",
        targetTableSql.c_str(),
        targetColsSql.c_str(),
        sourceColsSql.c_str(),
        sourceTableSql.c_str(),
        updateColsSql.c_str()
    ).GetUtf8CP();

    Statement stmt;
    rc = stmt.Prepare(conn, sql.c_str());
    if (rc != BE_SQLITE_OK) {
        return rc;
    }
    rc = stmt.Step();
    if (rc != BE_SQLITE_DONE) {
        return rc;
    }
    return BE_SQLITE_OK;
}

//SchemaSync===================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::SetDefaultSyncDbUri(SyncDbUri syncDbUri) {
    auto rc = VerifySyncDb(syncDbUri, false);
    if (rc != SchemaSync::Status::OK) {
        return rc;
    }
    m_defaultSyncDbUri = syncDbUri;
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::Init(SyncDbUri const& syncDbUri, Utf8StringCR containerId, bool overrideContainer, TableList additionTables) {
    auto const info = syncDbUri.GetInfo();
    if (!info.IsEmpty()) {
        BeJsDocument doc;
        info.To(BeJsValue(doc));
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0616,
            "Sync db (%a) already initalized. %s", doc.Stringify().c_str());
        return Status::ERROR_SCHEMA_SYNC_DB_ALREADY_INITIALIZED;
    }

    auto const localInfo = GetInfo();
    if (!localInfo.IsEmpty() && !overrideContainer) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0616,
            "Local db already initialized to schema sync (container-id: %s)", localInfo.GetSyncId().c_str());
        return Status::ERROR_SCHEMA_SYNC_DB_ALREADY_INITIALIZED;
    }

    if (Utf8String(containerId).Trim().empty()){
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0617,
            "ContainerId provided cannot be empty %s.", syncDbUri.GetUri().c_str());
    }

    Db sharedDb;
    Db::OpenParams openParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes);
    ParseQueryParams(openParams, syncDbUri);
    auto rc = sharedDb.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams);
    if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0617,
            "Fail to open schema sync db %s. %s", syncDbUri.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_SCHEMA_SYNC_DB_ALREADY_INITIALIZED;
    }

    sharedDb.GetStatementCache().Empty();

    rc = SchemaSyncHelper::DropDataTables(sharedDb);
    if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0618,
            "Fail to drop data table(s) from schema sync db (%s). %s", syncDbUri.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB;
    }

    rc = SchemaSyncHelper::DropMetaTables(sharedDb);
    if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0619,
            "Fail to drop meta table(s) from schema sync db (%s). %s", syncDbUri.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB;
    }

    rc = SchemaSyncHelper::CreateMetaTablesFrom(m_conn, sharedDb);
    if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0620,
            "Fail to re-create meta table(s) in schema sync db (%s). %s", syncDbUri.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB;
    }
    rc = sharedDb.TryExecuteSql("create table if not exists " TABLE_SQLSCHEMA "(id integer primary key, Type text, Name text, TableName text, Sql text)");
    if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0621,
            "Fail to create sync table (" TABLE_SQLSCHEMA ") in schema sync db (%s). %s", syncDbUri.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB;
    }

    SyncDbInfo syncInfo;
    LocalDbInfo localDbInfo = GetInfo();
    if (!localDbInfo.IsEmpty() && overrideContainer) {
        syncInfo.m_dataVer = localDbInfo.GetDataVersion();
    }
    localDbInfo.m_syncId = containerId;
    syncInfo.m_syncId = containerId;

    if (SaveSyncDbInfo(sharedDb,syncInfo) != Status::OK) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0622,
            "Fail to save sync db info in (%s). %s", syncDbUri.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB;
    }

    const auto sharedInfo = SyncDbInfo::From(sharedDb);
    rc = sharedDb.SaveChanges();
    if (rc != BE_SQLITE_OK || sharedInfo.IsEmpty()) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0623,
            "Fail to save changes to schema sync db (%s). %s", syncDbUri.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB;
    }

    if (SaveLocalDbInfo(m_conn, localDbInfo) != Status::OK) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0624,
            "Fail to save sync db info to local db. %s", BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR;
    }

    sharedDb.CloseDb();
    const auto pullResult = PushInternal(syncDbUri, additionTables);
    if (pullResult != Status::OK)
        return pullResult;

    if (std::find(additionTables.begin(),additionTables.end(), SchemaSyncHelper::TABLE_BE_PROP) != additionTables.end()){
        // after BE_PROP pull into syncdb it include JLocalDbInfo which
        // need to be deleted as its confusing as it should only be in the briefcase.
        rc = sharedDb.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams);
        if (rc != BE_SQLITE_OK) {
            m_conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0617,
                "Fail to open schema sync db %s. %s", syncDbUri.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
            return Status::ERROR_SCHEMA_SYNC_DB_ALREADY_INITIALIZED;
        }
        const auto propSpec = PropertySpec(JsonNames::JLocalDbInfo, JsonNames::JNamespace);
        sharedDb.DeleteProperty(propSpec);
        sharedDb.SaveChanges();
        sharedDb.CloseDb();
    }
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaSync::ParseQueryParams(Db::OpenParams& params, SyncDbUri const& uri){
    const auto n = uri.GetUri().find("?");
    if (n == Utf8String::npos)
        return;

    Utf8String queryParamsStr = uri.GetUri().substr(n + 1);
    bvector<Utf8String> queryParams;
    BeStringUtilities::Split(queryParamsStr.c_str(), "&", queryParams);
    for(auto& queryParam: queryParams)
        params.AddQueryParam(queryParam.c_str());

    params.m_fromContainer = true;
    params.m_skipFileCheck = true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::VerifySyncDb(SyncDbUri const& syncDbUri, bool isPull) const{
    if (m_conn.IsReadonly()) {
        m_conn.GetImpl().Issues().Report(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0625,
            "Primary connection is readonly. It must be in read/write mode.");
        return Status::ERROR_READONLY;
    }

    ECDb sharedDb;
    DbResult rc = BE_SQLITE_OK;
    if (isPull) {
        Db::OpenParams openParams(Db::OpenMode::Readonly);
        ParseQueryParams(openParams, syncDbUri);
        rc = sharedDb.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams);
        if (rc != BE_SQLITE_OK) {
                m_conn.GetImpl().Issues().ReportV(
                    IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0626,
                    "Fail to to open schema sync db db in readonly mode: (%s)", syncDbUri.GetUri().c_str());
            return Status::ERROR_OPENING_SCHEMA_SYNC_DB;
        }
    } else {
        Db::OpenParams openParams(Db::OpenMode::ReadWrite);
        ParseQueryParams(openParams, syncDbUri);
        rc = sharedDb.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams);
        if (rc != BE_SQLITE_OK) {
                m_conn.GetImpl().Issues().ReportV(
                    IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0626,
                    "Fail to to open schema sync db db in readonly mode: (%s)", syncDbUri.GetUri().c_str());
            return Status::ERROR_OPENING_SCHEMA_SYNC_DB;
        }

    }
    const auto syncDbInfo = SyncDbInfo::From(sharedDb);
    if (syncDbInfo.IsEmpty()) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0627,
            "Invalid schema sync db (%s). Schema sync info not found.", syncDbUri.GetUri().c_str());
        return Status::ERROR_INVALID_SCHEMA_SYNC_DB;
    }

    const auto localDbInfo = LocalDbInfo::From(m_conn);
    if (localDbInfo.IsEmpty()) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0628,
            "Local db is not set to use schema sync db (%s).", syncDbUri.GetUri().c_str());
        return Status::ERROR_INVALID_LOCAL_SYNC_DB;
    }

    if (syncDbInfo.GetSyncId() != localDbInfo.GetSyncId()) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0629,
            "Sync id does not match (local) %s <> (SyncDb) %s.",
                localDbInfo.GetSyncId().c_str(),
                syncDbInfo.GetSyncId().c_str());
        return Status::ERROR_SCHEMA_SYNC_INFO_DONOT_MATCH;
    }
    sharedDb.CloseDb();
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::PullInternal(SyncDbUri const& syncDbUri, TableList additionTables) {
    const auto vrc = VerifySyncDb(syncDbUri, true);
    if  (vrc != Status::OK) {
        return vrc;
    }

    const auto syncDbInfo = syncDbUri.GetInfo();
    auto localDbInfo = GetInfo();
    if (syncDbInfo.GetDataVersion() == localDbInfo.GetDataVersion()) {
        return Status::OK;
    }

    if (syncDbInfo.GetDataVersion() < localDbInfo.GetDataVersion()) {
        // this should never happen.
        return Status::ERROR;
    }

    if (SchemaSyncHelper::VerifyAlias(m_conn) != BE_SQLITE_OK) {
        return Status::ERROR;
    }

    auto rc = m_conn.AttachDb(syncDbUri.GetDbAttachUri().c_str(), SchemaSyncHelper::ALIAS_SYNC_DB);
    if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0630,
            "Unable to attach sync db '%s' as '%s' to primary connection: %s",
            syncDbUri.GetUri().c_str(),
            SchemaSyncHelper::ALIAS_SYNC_DB,
            m_conn.GetLastError().c_str());
        return Status::ERROR_UNABLE_TO_ATTACH;
    }

    rc = PullSqlSchema(m_conn);
    if (rc != BE_SQLITE_OK) {
        m_conn.AbandonChanges();
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return Status::ERROR_SYNC_SQL_SCHEMA;
    }

    // pull changes ================================================
    const auto fromAlias = SchemaSyncHelper::ALIAS_SYNC_DB;
    const auto toAlias = SchemaSyncHelper::ALIAS_MAIN_DB;

    TableList tables;
    rc = SchemaSyncHelper::GetMetaTables(m_conn, tables, fromAlias);
    if (rc != BE_SQLITE_OK) {
        m_conn.AbandonChanges();
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return Status::ERROR;
    }

    tables.insert(tables.end(), additionTables.begin(), additionTables.end());
    rc = SchemaSyncHelper::SyncData(m_conn, tables, fromAlias, toAlias);
    if (rc != BE_SQLITE_OK) {
        m_conn.AbandonChanges();
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return Status::ERROR;
    }

    rc = m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
    if (rc != BE_SQLITE_OK) {
        return Status::ERROR;
    }

    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSync::PullSqlSchema(DbR conn) {

    Utf8String facetsThatDoesNotExitsSql = SqlPrintfString(
        "SELECT [s].[sql] FROM [%s].[" TABLE_SQLSCHEMA "] [s] WHERE NOT EXISTS (SELECT 1 FROM [%s].[sqlite_master] m WHERE [m].[type]=[s].[type] AND [m].[name]=[s].[name]) ORDER BY [s].[id]",
        SchemaSyncHelper::ALIAS_SYNC_DB,
        SchemaSyncHelper::ALIAS_MAIN_DB
    ).GetUtf8CP();

    Statement stmt;
    auto rc = stmt.Prepare(conn, facetsThatDoesNotExitsSql.c_str());
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("PullSqlSchema() unable to prepare statement (%s): %s", facetsThatDoesNotExitsSql.c_str(), conn.GetLastError().c_str());
        return rc;
    }

    while ((rc=stmt.Step()) == BE_SQLITE_ROW) {
        auto sql = stmt.GetValueText(0);
        rc = conn.ExecuteDdl(sql);
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("PullSqlSchema() fail to execute ddl (%s): %s", sql, conn.GetLastError().c_str());
            return rc;
        }
    }

    return rc == BE_SQLITE_DONE ? BE_SQLITE_OK : rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSync::PushSqlSchema(DbR conn) {
    Utf8String truncatSql = SqlPrintfString("DELETE FROM [%s].[" TABLE_SQLSCHEMA "]", SchemaSyncHelper::ALIAS_SYNC_DB).GetUtf8CP();
    auto rc = conn.TryExecuteSql(truncatSql.c_str());
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("PushSqlSchema() unable to prepare statement (%s): %s", truncatSql.c_str(), conn.GetLastError().c_str());
        return rc;
    }
    Utf8String bulkInsertSql = SqlPrintfString(
        "INSERT INTO [%s].[" TABLE_SQLSCHEMA "](Type,Name,TableName,Sql) "
        "SELECT [type], [name], [tbl_name], [sql] FROM [%s].[sqlite_master] WHERE [name] NOT LIKE 'sqlite%%'",
        SchemaSyncHelper::ALIAS_SYNC_DB,
        SchemaSyncHelper::ALIAS_MAIN_DB).GetUtf8CP();

    rc = conn.TryExecuteSql(bulkInsertSql.c_str());
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("PushSqlSchema() fail to update sql schema (%s): %s", bulkInsertSql.c_str(), conn.GetLastError().c_str());
        return rc;
    }
    return BE_SQLITE_OK;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::PushInternal(SyncDbUri const& syncDbUri, TableList additionTables) {
    const auto vrc = VerifySyncDb(syncDbUri, false);
    if  (vrc != Status::OK) {
        return vrc;
    }

    const auto syncDbInfo = syncDbUri.GetInfo();
    const auto localDbInfo = GetInfo();
    if (syncDbInfo.GetDataVersion() != localDbInfo.GetDataVersion()) {
        return Status::ERROR;
    }

    if (SchemaSyncHelper::VerifyAlias(m_conn) != BE_SQLITE_OK) {
        return Status::ERROR;
    }

    auto rc = m_conn.AttachDb(syncDbUri.GetDbAttachUri().c_str(), SchemaSyncHelper::ALIAS_SYNC_DB);
    if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0630,
            "Unable to attach sync db '%s' as '%s' to primary connection: %s",
            syncDbUri.GetUri().c_str(),
            SchemaSyncHelper::ALIAS_SYNC_DB,
            m_conn.GetLastError().c_str());
        return Status::ERROR_UNABLE_TO_ATTACH;
    }

    // pull changes ================================================
    const auto fromAlias = SchemaSyncHelper::ALIAS_MAIN_DB;
    const auto toAlias = SchemaSyncHelper::ALIAS_SYNC_DB;

    TableList tables;
    rc = SchemaSyncHelper::GetMetaTables(m_conn, tables, fromAlias);
    if (rc != BE_SQLITE_OK) {
        m_conn.AbandonChanges();
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return Status::ERROR;
    }

    tables.insert(tables.end(), additionTables.begin(), additionTables.end());
    rc = SchemaSyncHelper::SyncData(m_conn, tables, fromAlias, toAlias);
    if (rc != BE_SQLITE_OK) {
        m_conn.AbandonChanges();
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return Status::ERROR;
    }

    rc = PushSqlSchema(m_conn);
    if (rc != BE_SQLITE_OK) {
        m_conn.AbandonChanges();
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return Status::ERROR;
    }

    rc = m_conn.SaveChanges();
    if (rc != BE_SQLITE_OK) {
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return Status::ERROR;
    }

    rc = m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
    if (rc != BE_SQLITE_OK) {
        return Status::ERROR;
    }

    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::Init(SyncDbUri const& syncDbUri, Utf8StringCR containerId, bool overrideContainer) {
    ECDB_PERF_LOG_SCOPE("Initializing schema sync db");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SchemaSync::Init");
    BeMutexHolder holder(m_conn.GetImpl().GetMutex());
    BeginModifiedRowCount();
    const auto rc = Init(syncDbUri, containerId, overrideContainer, { SchemaSyncHelper::TABLE_BE_PROP });
    EndModifiedRowCount();
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SchemaSync::Init");
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::Pull(SyncDbUri const& syncDbUri, SchemaImportToken const* schemaImportToken) {
    ECDB_PERF_LOG_SCOPE("Pulling from schema sync db");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SchemaSync::Pull");

    BeMutexHolder holder(m_conn.GetImpl().GetMutex());
    auto& mainDisp = m_conn.Schemas().Main();

    mainDisp.OnBeforeSchemaChanges().RaiseEvent(m_conn, SchemaChangeType::SchemaImport);
    m_conn.ClearECDbCache();
    const auto effectiveSyncDbUri = syncDbUri.IsEmpty() ? GetDefaultSyncDbUri() : syncDbUri;
    BeginModifiedRowCount();
    auto rc = PullInternal(effectiveSyncDbUri, {});
    EndModifiedRowCount();
    if (rc != Status::OK) {
        return rc;
    }

    rc = UpdateDbSchema();
    if (rc != Status::OK) {
        return rc;
    }

    auto localDb = GetInfo();
    auto syncDb = SyncDbInfo::From(syncDbUri);
    localDb.m_dataVer = syncDb.GetDataVersion();
    rc = SaveLocalDbInfo(m_conn, localDb);
    if (rc != Status::OK) {
        m_conn.AbandonChanges();
        return rc;
    }

    mainDisp.OnAfterSchemaChanges().RaiseEvent(m_conn, SchemaChangeType::SchemaImport);
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SchemaSync::Pull");
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::UpdateDbSchema() {
    ECDB_PERF_LOG_SCOPE("Updating sqlite schema");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SchemaSync::UpdateDbSchema");

    BeMutexHolder holder(m_conn.GetImpl().GetMutex());
    SchemaImportContext ctx(m_conn, SchemaManager::SchemaImportOptions(), /* synchronizeSchemas = */true);
    auto& mainDisp = m_conn.Schemas().Main();
    m_conn.ClearECDbCache();

    if (SUCCESS != mainDisp.GetDbSchema().ForceReloadTableAndIndexesFromDisk()) {
        return Status::ERROR;
    }

    if (SUCCESS != mainDisp.CreateOrUpdateRequiredTables()) {
        return Status::ERROR;
    }

    if (SUCCESS != mainDisp.CreateOrUpdateIndexesInDb(ctx)) {
        return Status::ERROR;
    }

    if (SUCCESS != mainDisp.PurgeOrphanTables(ctx)) {
        return Status::ERROR;
    }

    if (SUCCESS != DbMapValidator(ctx).Validate()) {
        return Status::ERROR;
    }

    m_conn.ClearECDbCache();

    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SchemaSync::UpdateDbSchema");
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::Push(SyncDbUri const& syncDbUri) {
    ECDB_PERF_LOG_SCOPE("Pushing tp schema sync db");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SchemaSync::Push");
    BeMutexHolder holder(m_conn.GetImpl().GetMutex());
    BeginModifiedRowCount();
    auto rc = PushInternal(syncDbUri, {});
    EndModifiedRowCount();
    if (rc == Status::OK && GetModifiedRowCount() > 0) {
        auto syncDbInfo = SyncDbInfo::From(syncDbUri);
        auto localDbInfo = GetInfo();
        syncDbInfo.m_dataVer += 1;
        localDbInfo.m_dataVer = syncDbInfo.m_dataVer;

        rc = SaveSyncDbInfo(syncDbUri, syncDbInfo);
        if (rc != Status::OK) {
            return rc;
        }

        rc = SaveLocalDbInfo(m_conn, localDbInfo);
        if (rc != Status::OK) {
            return rc;
        }
    }
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SchemaSync::Push");
    return rc;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaSync::ContainsChangeToLocalDbInfo(ChangeStream& stream)  {
    // Check if be_Prop has change for NameSpace=JsonNames::JNamespace && Name=JsonNames::JLocalDbInfo
    // every time schema sync is used it will increment data ver which should change JLocalDbInfo
    for(auto& change : stream.GetChanges()) {
        Utf8CP tableName = nullptr;
        if (BE_SQLITE_OK != change.GetOperation(&tableName, nullptr, nullptr, nullptr))
            continue;

        if (tableName && BeStringUtilities::StricmpAscii(tableName, "be_Prop") == 0) {
            auto namespaceVal = change.GetOldValue(0);
            auto nameVal = change.GetOldValue(1);
            const auto ns = namespaceVal.IsValid() && namespaceVal.GetValueType() == DbValueType::TextVal ? namespaceVal.GetValueText() : nullptr;
            const auto name = nameVal.IsValid() && nameVal.GetValueType() == DbValueType::TextVal ? nameVal.GetValueText() : nullptr;
            if (ns && name &&
                0 == BeStringUtilities::StricmpAscii(ns, JsonNames::JNamespace) &&
                0 == BeStringUtilities::StricmpAscii(name, JsonNames::JLocalDbInfo)) {
                return true;
            }
        }
    }
    return false;
}
//=======================================================================================
//     SchemaSync::LocalDbInfo
//+===============+===============+===============+===============+===============+======
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::LocalDbInfo SchemaSync::GetInfo() const {
    return LocalDbInfo::From(m_conn);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::SaveSyncDbInfo(DbR syncDb, SyncDbInfo const& info) {
    const auto propSpec = PropertySpec(JsonNames::JSyncDbInfo, JsonNames::JNamespace);
    BeJsDocument jsonDoc;
    info.To(BeJsValue(jsonDoc));
    auto rc = syncDb.SavePropertyString(propSpec, jsonDoc.Stringify());
    if (rc != BE_SQLITE_OK) {
        return SchemaSync::Status::ERROR;
    }
    return SchemaSync::Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::SaveLocalDbInfo(DbR db, LocalDbInfo const& info) {
    const auto propSpec = PropertySpec(JsonNames::JLocalDbInfo, JsonNames::JNamespace);
    BeJsDocument jsonDoc;
    info.To(BeJsValue(jsonDoc));
    auto rc = db.SavePropertyString(propSpec, jsonDoc.Stringify());
    if (rc != BE_SQLITE_OK) {
        return SchemaSync::Status::ERROR;
    }
    return SchemaSync::Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::SaveSyncDbInfo(SyncDbUri syncDbUri, SyncDbInfo const& info) {
    Db conn;
    Db::OpenParams openParams(Db::OpenMode::ReadWrite);
    ParseQueryParams(openParams, syncDbUri);
    auto rc = conn.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams);
    if (rc != BE_SQLITE_OK) {
        return SchemaSync::Status::ERROR;
    }
    auto kc = SaveSyncDbInfo(conn, info);
    if (kc != Status::OK) {
        conn.AbandonChanges();
        return kc;
    }
    conn.SaveChanges();
    return SchemaSync::Status::OK;
}

//=======================================================================================
//     SchemaSync::SyncDbInfo
//+===============+===============+===============+===============+===============+======
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaSync::SyncDbInfo::To(BeJsValue val) const {
    val.SetEmptyObject();
    val[JsonNames::SyncDataVer] = BeInt64Id(m_dataVer).ToHexStr();
    val[JsonNames::SyncId] = m_syncId;
}

//SchemaSyncHelper::SyncDbUri==================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SchemaSync::SyncDbUri::GetDbAttachUri() const {
    if (m_uri.StartsWith("file:") || m_uri.find("?") == Utf8String::npos)
        return m_uri;

    Utf8String uri = "file:" + m_uri;
    uri.ReplaceAll("\\", "/");
    return uri;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::SyncDbInfo SchemaSync::SyncDbUri::GetInfo() const{
    if (IsEmpty()) {
        return SyncDbInfo();
    }

    Db conn;
    Db::OpenParams openParams(Db::OpenMode::Readonly);
    ParseQueryParams(openParams, *this);
    if (conn.OpenBeSQLiteDb(m_uri.c_str(), openParams) != BE_SQLITE_OK) {
        return SyncDbInfo();
    }
    return SyncDbInfo::From(conn);
}

//SchemaSyncHelper::SyncDbInfo===========================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::SyncDbInfo SchemaSync::SyncDbInfo::From(DbCR conn){
    Utf8String strData;
    const auto propSpec = PropertySpec(JsonNames::JSyncDbInfo, JsonNames::JNamespace);
    auto rc = conn.QueryProperty(strData, propSpec);
    if (rc != BE_SQLITE_ROW) {
        return SyncDbInfo();
    }
    BeJsDocument jsonDoc;
    jsonDoc.Parse(strData);
    return SyncDbInfo::From(BeJsConst(jsonDoc));
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::SyncDbInfo SchemaSync::SyncDbInfo::From(SyncDbUri syncDbUri){
    Db conn;
    Db::OpenParams openParams(Db::OpenMode::Readonly);
    ParseQueryParams(openParams, syncDbUri);
    auto rc = conn.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams);
    if (rc != BE_SQLITE_OK) {
        return SyncDbInfo();
    }
    return From(conn);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::SyncDbInfo SchemaSync::SyncDbInfo::From(BeJsConst val){
    static SyncDbInfo s_empty;
    if (!val.isObject()){
        return s_empty;
    }

    SyncDbInfo info;
    if (val.isStringMember(JsonNames::SyncDataVer) && val.isStringMember(JsonNames::SyncId)) {
        BentleyStatus status;
        info.m_dataVer = BeInt64Id::FromString(val[JsonNames::SyncDataVer].asCString(), &status).GetValueUnchecked();
        if (status == ERROR) {
            return s_empty;
        }

        info.m_syncId = val[JsonNames::SyncId].asString();
        if (info.m_syncId.empty()) {
            return s_empty;
        }
        return info;
    }
    return s_empty;
}

//=======================================================================================
//     SchemaSync::LocalDbInfo
//+===============+===============+===============+===============+===============+======
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaSync::LocalDbInfo::To(BeJsValue val) const {
    val.SetEmptyObject();
    val[JsonNames::SyncDataVer] = BeInt64Id(m_dataVer).ToHexStr();
    val[JsonNames::SyncId] = m_syncId;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::LocalDbInfo SchemaSync::LocalDbInfo::From(DbCR conn){
    Utf8String strData;
    const auto propSpec = PropertySpec(JsonNames::JLocalDbInfo, JsonNames::JNamespace);
    auto rc = conn.QueryProperty(strData, propSpec);
    if (rc != BE_SQLITE_ROW) {
        return LocalDbInfo();
    }
    BeJsDocument jsonDoc;
    jsonDoc.Parse(strData);
    return LocalDbInfo::From(BeJsConst(jsonDoc));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::LocalDbInfo SchemaSync::LocalDbInfo::From(BeJsConst val){
    static LocalDbInfo s_empty;
    if (!val.isObject()){
        return s_empty;
    }

    LocalDbInfo info;
    if (val.isStringMember(JsonNames::SyncDataVer) && val.isStringMember(JsonNames::SyncId)) {
        BentleyStatus status;
        info.m_dataVer = BeInt64Id::FromString(val[JsonNames::SyncDataVer].asCString(), &status).GetValueUnchecked();
        if (status == ERROR) {
            return s_empty;
        }

        info.m_syncId = val[JsonNames::SyncId].asString();
        if (info.m_syncId.empty()) {
            return s_empty;
        }
        return info;
    }
    return s_empty;
}

END_BENTLEY_SQLITE_EC_NAMESPACE


