/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// TEMPORARY - tracing for the "upstream" schema sync flow, to be deleted before this ships.
//
// Prints one line per phase, never per row, so a whole test run stays readable. Only the upstream
// flow traces; the pull/push path does not. To remove: set the switch to 0 to silence it, or
// delete this block and every SS_TRACE( line in the file - they are all one-liners with no other
// side effects, so nothing else has to change.
//+===============+===============+===============+===============+===============+======
#define SCHEMA_SYNC_UPSTREAM_TRACE 1
#if SCHEMA_SYNC_UPSTREAM_TRACE
    #define SS_TRACE(...) do { printf("[schemasync] "); printf(__VA_ARGS__); printf("\n"); fflush(stdout); } while (false)

//=======================================================================================
// TEMPORARY - goes with the block above. An import into the sync db reports its reason through the
// issue reporter and returns a bare BE_SQLITE_ERROR, so without this a failure says nothing at all.
//+===============+===============+===============+===============+===============+======
struct SchemaSyncTraceIssueListener final : ECN::IIssueListener {
    void _OnIssueReported(ECN::IssueSeverity, ECN::IssueCategory, ECN::IssueType, ECN::IssueId, Utf8CP message) const override {
        SS_TRACE("  issue: %s", message);
    }
};
#else
    #define SS_TRACE(...) do { } while (false)
#endif

//=======================================================================================
//     JsonNames
//+===============+===============+===============+===============+===============+======
//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct JsonNames {
    constexpr static char SyncId[] = "id";
    constexpr static char SyncDataVer[] = "dataVer";
    constexpr static char JNamespaceEC[] = "ec_Db";
    constexpr static char JNamespaceBE[] = "be_Db";
    constexpr static char JNamespaceDGN[] = "dgn_Db";
    constexpr static char JSyncDbInfo[] = "syncDbInfo";
    constexpr static char JLocalDbInfo[] = "localDbInfo";
    constexpr static char JSchemaVersion[] = "SchemaVersion";
};

//SchemaSyncHelper==============================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ProfileVersion SchemaSyncHelper::QueryProfileVersion(SchemaSync::SyncDbUri syncDbUri, ProfileKind kind) {
    Db conn;
    Db::OpenParams openParams(Db::OpenMode::Readonly);
    SchemaSync::ParseQueryParams(openParams, syncDbUri);
    const auto rc = conn.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams);
    if (rc != BE_SQLITE_OK) {
        ProfileVersion(0, 0, 0, 0);
    }
    return QueryProfileVersion(conn, kind);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PropertySpec SchemaSyncHelper::GetPropertySpec(ProfileKind kind) {
    if (kind == ProfileKind::BE){
        return PropertySpec(JsonNames::JSchemaVersion, JsonNames::JNamespaceBE);
    } else if (kind == ProfileKind::EC){
        return PropertySpec(JsonNames::JSchemaVersion, JsonNames::JNamespaceEC);
    } else if (kind == ProfileKind::DGN){
        return PropertySpec(JsonNames::JSchemaVersion, JsonNames::JNamespaceDGN);
    }
    BeAssert(false && "unrecognized value");
    return PropertySpec("", "");
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ProfileVersion SchemaSyncHelper::QueryProfileVersion(DbR db, ProfileKind kind) {
    const auto profileSpec = SchemaSyncHelper::GetPropertySpec(kind);
    Utf8String versionJson;
    if (BE_SQLITE_ROW != db.QueryProperty(versionJson, profileSpec)) {
        return ProfileVersion(0, 0, 0, 0);
    }

    ProfileVersion ver(0, 0, 0, 0);
    if (ver.FromJson(versionJson.c_str()) != BentleyStatus::SUCCESS) {
        return ProfileVersion(0, 0, 0, 0);
    }
    return ver;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::SaveProfileVersion(SchemaSync::SyncDbUri syncDbUri, ProfileKind kind, ProfileVersion const& ver) {
    Db conn;
    Db::OpenParams openParams(Db::OpenMode::ReadWrite);
    SchemaSync::ParseQueryParams(openParams, syncDbUri);
    auto rc = conn.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams);
    if (rc != BE_SQLITE_OK) {
        LOG.error("SchemaSyncHelper::SaveProfileVersion(): Failed to save profile version. Unable to open sync db.");
        return rc;
    }

    return SaveProfileVersion(conn, kind, ver);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::SaveProfileVersion(DbR conn, ProfileKind kind, ProfileVersion const& ver) {
    const auto profileSpec = SchemaSyncHelper::GetPropertySpec(kind);
    if (conn.IsReadonly()) {
        LOG.error("SchemaSyncHelper::SaveProfileVersion(): Failed to save profile version. Database is readonly.");
        return BE_SQLITE_ERROR;
    }
    Utf8String profileVersionStr = ver.ToJson();
    auto rc = conn.SavePropertyString(profileSpec, profileVersionStr);
    if (rc != BE_SQLITE_OK) {
        LOG.error("SchemaSyncHelper::SaveProfileVersion(): Failed to save profile version.");
        return rc;
    }

    return conn.SaveChanges();
}

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
            LOG.errorv("%s\n",
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
        LOG.errorv("SchemaSyncHelper::GetMetaTables(): Failed to prepare statement. %s", BeSQLiteLib::GetErrorString(rc));
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
        LOG.errorv("SchemaSyncHelper::DropDataTables(): Failed to prepared statement to query meta tables to be dropped. %s", BeSQLiteLib::GetErrorString(rc));
        return rc;
    }
    while(stmt.Step() == BE_SQLITE_ROW) {
        tables.push_back(stmt.GetValueText(0));
    }

    stmt.Finalize();
    for(auto& table : tables) {
        rc = conn.ExecuteSql(SqlPrintfString("DROP TABLE IF EXISTS [main].[%s];", table.c_str()).GetUtf8CP());
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("SchemaSyncHelper::DropDataTables(): Failed to drop table %s. %s", table.c_str(), BeSQLiteLib::GetErrorString(rc));
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
            LOG.errorv("SchemaSyncHelper::DropMetaTables(): Failed to drop table %s. %s", table.c_str(), BeSQLiteLib::GetErrorString(rc));
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
    if (BE_SQLITE_OK != rc){
        LOG.errorv("SchemaSyncHelper::GetPrimaryKeyColumnNames(): Failed to prepare statement to query primary key columns. %s", BeSQLiteLib::GetErrorString(rc));
        return rc;
    }

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
        LOG.error("SchemaSyncHelper::SyncData(): Failed to set defer_foreign_keys=1");
        return rc;
    }
    for (auto& tbl : tables) {
        rc = SyncData(conn, tbl.c_str(), sourceDbAlias, targetDbAlias);
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("SchemaSyncHelper::SyncData(): Failed to sync data for table %s. %s", tbl.c_str(), BeSQLiteLib::GetErrorString(rc));
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
    if (BE_SQLITE_OK != rc){
        LOG.errorv("SchemaSyncHelper::SyncData(): Failed to get column names for table %s. %s", tableName, BeSQLiteLib::GetErrorString(rc));
        return rc;
    }

    auto sourcePkCols = StringList{};
    rc = GetPrimaryKeyColumnNames(conn, sourceDbAlias, tableName, sourcePkCols);
    if (BE_SQLITE_OK != rc){
        LOG.errorv("SchemaSyncHelper::SyncData(): Failed to get primary key column names for table %s. %s", tableName, BeSQLiteLib::GetErrorString(rc));
        return rc;
    }

    auto targetCols = StringList{};
    rc = GetColumnNames(conn, targetDbAlias, tableName, targetCols);
    if (BE_SQLITE_OK != rc) {
        LOG.errorv("SchemaSyncHelper::SyncData(): Failed to get column names for table %s. %s", tableName, BeSQLiteLib::GetErrorString(rc));
        return rc;
    }

    auto targetPkCols = StringList{};
    rc = GetPrimaryKeyColumnNames(conn, targetDbAlias, tableName, targetPkCols);
    if (BE_SQLITE_OK != rc){
        LOG.errorv("SchemaSyncHelper::SyncData(): Failed to get primary key column names for table %s. %s", tableName, BeSQLiteLib::GetErrorString(rc));
        return rc;
    }

    std::sort(std::begin(sourceCols), std::end(sourceCols));
    std::sort(std::begin(sourcePkCols), std::end(sourcePkCols));
    std::sort(std::begin(targetCols), std::end(targetCols));
    std::sort(std::begin(targetPkCols), std::end(targetPkCols));

    const auto sourceColCount = sourceCols.size();
    const auto sourcePkColCount = sourcePkCols.size();

    if(sourceColCount != targetCols.size()) {
        LOG.errorv("SchemaSyncHelper::SyncData(): Column count mismatch for table %s", tableName);
        return BE_SQLITE_SCHEMA;
    }
    if(sourcePkColCount != targetPkCols.size()) {
        LOG.errorv("SchemaSyncHelper::SyncData(): Primary key column count mismatch for table %s", tableName);
        return BE_SQLITE_SCHEMA;
    }
    for (auto i = 0; i < sourceColCount; ++i) {
        if (ToLower(sourceCols[i]) != ToLower(targetCols[i])){
            LOG.errorv("SchemaSyncHelper::SyncData(): Column name mismatch for table %s", tableName);
            return BE_SQLITE_SCHEMA;
        }
    }
    for (auto i = 0; i < sourcePkColCount; ++i) {
        if (ToLower(sourcePkCols[i]) != ToLower(targetPkCols[i])) {
            LOG.errorv("SchemaSyncHelper::SyncData(): Primary key column name mismatch for table %s", tableName);
            return BE_SQLITE_SCHEMA;
        }
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
            LOG.errorv("SchemaSyncHelper::SyncData(): Failed to prepare statement to delete data from target table %s. %s", tableName, BeSQLiteLib::GetErrorString(rc));
            return rc;
        }
        rc = stmt.Step();
        if (rc != BE_SQLITE_DONE) {
            LOG.errorv("SchemaSyncHelper::SyncData(): Failed to delete data from target table %s. %s", tableName, BeSQLiteLib::GetErrorString(rc));
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
        LOG.errorv("SchemaSyncHelper::SyncData(): Failed to prepare statement to sync data for table %s. %s", tableName, BeSQLiteLib::GetErrorString(rc));
        return rc;
    }
    rc = stmt.Step();
    if (rc != BE_SQLITE_DONE) {
        LOG.errorv("SchemaSyncHelper::SyncData(): Failed to sync data for table %s. %s", tableName, BeSQLiteLib::GetErrorString(rc));
        return rc;
    }
    return BE_SQLITE_OK;
}

//SchemaSync===================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::SetDefaultSyncDbUri(SyncDbUri syncDbUri) {
    auto rc = VerifySyncDb(syncDbUri, false, false);
    if (rc != SchemaSync::Status::OK) {
        LOG.error("SchemaSync::SetDefaultSyncDbUri(): Failed to verify sync db.");
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

    rc = SchemaSyncHelper::SyncProfileTablesSchema(m_conn, sharedDb);
    if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0620,
            "Fail to re-create meta table(s) in schema sync db (%s). %s", syncDbUri.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
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
    const auto pullResult = PushInternal(syncDbUri, additionTables, true);
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
        const auto propSpec = PropertySpec(JsonNames::JLocalDbInfo, JsonNames::JNamespaceEC);
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
Utf8String SchemaSync::GetStatusAsString(Status status) {
    switch (status) {
        case Status::OK:
            return "OK";
        case Status::ERROR:
            return "ERROR";
        case Status::ERROR_READONLY:
            return "ERROR_READONLY";
        case Status::ERROR_OPENING_SCHEMA_SYNC_DB:
            return "ERROR_OPENING_SCHEMA_SYNC_DB";
        case Status::ERROR_INVALID_SCHEMA_SYNC_DB:
            return "ERROR_INVALID_SCHEMA_SYNC_DB";
        case Status::ERROR_INVALID_LOCAL_SYNC_DB:
            return "ERROR_INVALID_LOCAL_SYNC_DB";
        case Status::ERROR_SCHEMA_SYNC_DB_ALREADY_INITIALIZED:
            return "ERROR_SCHEMA_SYNC_DB_ALREADY_INITIALIZED";
        case Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB:
            return "ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB";
        case Status::ERROR_SCHEMA_SYNC_INFO_DONOT_MATCH:
            return "ERROR_SCHEMA_SYNC_INFO_DONOT_MATCH";
        case Status::ERROR_UNABLE_TO_ATTACH:
            return "ERROR_UNABLE_TO_ATTACH";
        case Status::ERROR_SYNC_SQL_SCHEMA:
            return "ERROR_SYNC_SQL_SCHEMA";
        case Status::ERROR_DATA_TRANSFORM_REQUIRED:
            return "ERROR_DATA_TRANSFORM_REQUIRED";
        case Status::ERROR_SYNC_DB_CHANGED:
            return "ERROR_SYNC_DB_CHANGED";
        case Status::ERROR_PROFILE_VERSION_MISMATCH:
            return "ERROR_PROFILE_VERSION_MISMATCH";
        default:
            return "SCHEMA_SYNC_FAIL";
    }
}

//---------------------------------------------------------------------------------------
// Pull and push each tolerate profile skew in one direction. Deciding the mapping in one file and
// adopting it in the other cannot: a difference in profile version means the two could map the same
// schema differently. Nothing may move until they are aligned, which is a maintenance-mode job.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::VerifyProfileVersionsMatch(SyncDbUri const& syncDbUri) const {
    const auto syncDbVersion = SchemaSyncHelper::QueryProfileVersion(syncDbUri, SchemaSyncHelper::ProfileKind::EC);
    const auto localVersion = SchemaSyncHelper::QueryProfileVersion(m_conn, SchemaSyncHelper::ProfileKind::EC);

    if (syncDbVersion.IsEmpty() || localVersion.IsEmpty()) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0681,
            "Failed to read the EC profile version of the sync db (%s) or of this briefcase.", syncDbUri.GetUri().c_str());
        return Status::ERROR;
    }

    if (syncDbVersion != localVersion) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0683,
            "Schema sync requires the sync db (%s) and this briefcase (%s) to be on the same EC profile version.",
            syncDbVersion.ToString().c_str(), localVersion.ToString().c_str());
        return Status::ERROR_PROFILE_VERSION_MISMATCH;
    }

    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::VerifySyncDb(SyncDbUri const& syncDbUri, bool isPull, bool isInit) const{
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
    if (!isInit) {
        const auto shareDbProfileVersion = SchemaSyncHelper::QueryProfileVersion(sharedDb, SchemaSyncHelper::ProfileKind::EC);
        const auto currentDbProfileVersion = SchemaSyncHelper::QueryProfileVersion(m_conn, SchemaSyncHelper::ProfileKind::EC);
        if (shareDbProfileVersion.IsEmpty()) {
            m_conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0681,
                "Failed to read ecdb profile version from sync db(%s)", syncDbUri.GetUri().c_str());
            return Status::ERROR;
        }

        if (currentDbProfileVersion.IsEmpty()) {
            m_conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0682,
                "Failed to read ecdb profile version for current connection(%s)", m_conn.GetDbFileName());
            return Status::ERROR;
        }

        if (isPull) {
            if (currentDbProfileVersion > shareDbProfileVersion) {
                m_conn.GetImpl().Issues().ReportV(
                    IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0683,
                    "Pull failed: ECDb Profile version of sync db (%s) should be greater or equal to current db (%s).",
                        shareDbProfileVersion.ToString().c_str(),
                        currentDbProfileVersion.ToString().c_str());
                return Status::ERROR;
            }
        } else {
            if (currentDbProfileVersion < shareDbProfileVersion) {
                m_conn.GetImpl().Issues().ReportV(
                    IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0683,
                    "Push failed: ECDb Profile version of sync db (%s) should be less or equal to current db (%s).",
                        shareDbProfileVersion.ToString().c_str(),
                        currentDbProfileVersion.ToString().c_str());
                return Status::ERROR;
            }
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
    const auto vrc = VerifySyncDb(syncDbUri, true, false);
    if  (vrc != Status::OK) {
        LOG.error("SchemaSync::PullInternal(): Failed to verify sync db.");
        return vrc;
    }

    const auto syncDbInfo = syncDbUri.GetInfo();
    auto localDbInfo = GetInfo();
    if (syncDbInfo.GetDataVersion() == localDbInfo.GetDataVersion()) {
        return Status::OK;
    }

    if (syncDbInfo.GetDataVersion() < localDbInfo.GetDataVersion()) {
        LOG.error("SchemaSync::PullInternal(): Sync db data version is less than local db data version.");
        return Status::ERROR;
    }

    if (SchemaSyncHelper::VerifyAlias(m_conn) != BE_SQLITE_OK) {
        LOG.error("SchemaSync::PullInternal(): Failed to verify alias.");
        return Status::ERROR;
    }

    // patch thisDb with on from container
    auto rc = SchemaSyncHelper::SyncProfileTablesSchema(m_conn, syncDbUri, false);
    if (rc != BE_SQLITE_OK) {
        LOG.error("SchemaSync::PullInternal(): Failed to sync profile tables schema.");
        m_conn.AbandonChanges();
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return Status::ERROR_SYNC_SQL_SCHEMA;
    }

    rc = m_conn.AttachDb(syncDbUri.GetDbAttachUri().c_str(), SchemaSyncHelper::ALIAS_SYNC_DB);
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
    const auto fromAlias = SchemaSyncHelper::ALIAS_SYNC_DB;
    const auto toAlias = SchemaSyncHelper::ALIAS_MAIN_DB;

    TableList tables;
    rc = SchemaSyncHelper::GetMetaTables(m_conn, tables, fromAlias);
     if (rc != BE_SQLITE_OK) {
        LOG.error("SchemaSync::PullInternal(): Failed to get meta tables.");
        m_conn.AbandonChanges();
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return Status::ERROR;
    }

    tables.insert(tables.end(), additionTables.begin(), additionTables.end());
    rc = SchemaSyncHelper::SyncData(m_conn, tables, fromAlias, toAlias);
    if (rc != BE_SQLITE_OK) {
        LOG.error("SchemaSync::PullInternal(): Failed to sync data.");
        m_conn.AbandonChanges();
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return Status::ERROR;
    }

    rc = m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
    if (rc != BE_SQLITE_OK) {
        LOG.error("SchemaSync::PullInternal(): Failed to detach db.");
        return Status::ERROR;
    }

    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::PushInternal(SyncDbUri const& syncDbUri, TableList additionTables, bool isInit) {
    const auto vrc = VerifySyncDb(syncDbUri, false, isInit);
    if  (vrc != Status::OK) {
        LOG.error("SchemaSync::PushInternal(): Failed to verify sync db.");
        return vrc;
    }

    const auto syncDbInfo = syncDbUri.GetInfo();
    const auto localDbInfo = GetInfo();
    if (syncDbInfo.GetDataVersion() != localDbInfo.GetDataVersion()) {
        LOG.error("SchemaSync::PushInternal(): Sync db data version is not equal to local db data version.");
        return Status::ERROR;
    }

    if (SchemaSyncHelper::VerifyAlias(m_conn) != BE_SQLITE_OK) {
        LOG.error("SchemaSync::PushInternal(): Failed to verify alias.");
        return Status::ERROR;
    }

    // patch container with thisDb schema changes if any
    auto rc = SchemaSyncHelper::SyncProfileTablesSchema(m_conn, syncDbUri, true);
    if (rc != BE_SQLITE_OK) {
        LOG.error("SchemaSync::PushInternal(): Failed to sync profile tables schema.");
        m_conn.AbandonChanges();
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return Status::ERROR;
    }

    rc = m_conn.AttachDb(syncDbUri.GetDbAttachUri().c_str(), SchemaSyncHelper::ALIAS_SYNC_DB);
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
        LOG.error("SchemaSync::Pull(): Failed to pull from schema sync db");
        return rc;
    }

    auto sqliteRc = SchemaSyncHelper::UpdateProfileVersion(m_conn, effectiveSyncDbUri, false);
    if (sqliteRc != BE_SQLITE_OK) {
        LOG.error("SchemaSync::Pull(): Failed to update profile version in schema sync db");
        return Status::ERROR;
    }

    rc = UpdateDbSchema();
    if (rc != Status::OK) {
        LOG.error("SchemaSync::Pull(): Failed to update schema in schema sync db");
        return rc;
    }

    auto localDb = GetInfo();
    auto syncDb = SyncDbInfo::From(effectiveSyncDbUri);
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
    const auto kDoNotTrackDdlChanges = true;
    const auto rc = m_conn.Schemas().Main().UpdateDbSchema(kDoNotTrackDdlChanges);
    if (rc != SUCCESS) {
        LOG.error("SchemaSync::UpdateDbSchema(): Failed to update db schema.");
        return Status::ERROR;
    }
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::UpdateDataVersion(SyncDbUri const& syncDbUri) {
    auto syncDbInfo = SyncDbInfo::From(syncDbUri);

    auto localDbInfo = GetInfo();
    syncDbInfo.m_dataVer += 1;
    localDbInfo.m_dataVer = syncDbInfo.m_dataVer;

    auto rc = SaveSyncDbInfo(syncDbUri, syncDbInfo);
    if (rc != Status::OK) {
        LOG.error("SchemaSync::UpdateDataVersion() Failed to save sync db info");
        return rc;
    }

    rc = SaveLocalDbInfo(m_conn, localDbInfo);
    if (rc != Status::OK) {
        LOG.error("SchemaSync::UpdateDataVersion() Failed to save local db info");
        return rc;
    }
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::Push(SyncDbUri const& syncDbUri) {
    ECDB_PERF_LOG_SCOPE("Pushing tp schema sync db");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SchemaSync::Push");
    BeMutexHolder holder(m_conn.GetImpl().GetMutex());
    const auto effectiveSyncDbUri = syncDbUri.IsEmpty() ? GetDefaultSyncDbUri() : syncDbUri;
    BeginModifiedRowCount();
    auto rc = PushInternal(effectiveSyncDbUri, {}, false);
    EndModifiedRowCount();
    if (rc == Status::OK && GetModifiedRowCount() > 0) {
        DbResult sqliteStatus = SchemaSyncHelper::UpdateProfileVersion(m_conn, effectiveSyncDbUri, true);
        if (sqliteStatus != BE_SQLITE_OK) {
            LOG.error("SchemaSync::Push() Failed to update profile version in schema sync db");
            return Status::ERROR;
        }

        rc = UpdateDataVersion(effectiveSyncDbUri);
        if (rc != Status::OK) {
            LOG.error("SchemaSync::Push() Failed to update data version in schema sync db");
            return rc;
        }
    }
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SchemaSync::Push");
    return rc;
}

//SchemaSyncUpstreamHelper======================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncUpstreamHelper::DropClosure(ECDbR conn) {
    for (auto tempTable : { TEMP_SCHEMA_IDS, TEMP_CLASS_IDS, TEMP_TABLE_IDS, TEMP_COLUMN_IDS, TEMP_SEED_IDS }) {
        const auto rc = conn.ExecuteSql(SqlPrintfString("DROP TABLE IF EXISTS %s", tempTable).GetUtf8CP());
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("SchemaSyncUpstreamHelper::DropClosure(): Failed to drop %s. %s", tempTable, BeSQLiteLib::GetErrorString(rc));
            return rc;
        }
    }
    return BE_SQLITE_OK;
}

//=======================================================================================
// Serves an explicit set of schemas by key. Sits ahead of the sync db's locater while schemas are
// being re-pointed, so that a schema referencing one of its siblings picks up that sibling's copy -
// which already points at the sync db - rather than the original, which still points at the
// briefcase.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaSetLocater final : ECN::IECSchemaLocater {
    private:
        bvector<ECSchemaPtr> m_schemas;

        ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR) override {
            for (auto const& schema : m_schemas) {
                if (schema->GetSchemaKey().Matches(key, matchType))
                    return schema;
            }
            return nullptr;
        }

    public:
        void Add(ECSchemaPtr schema) { m_schemas.push_back(schema); }
        Utf8String GetDescription() const override { return Utf8PrintfString("SchemaSetLocater holding %d schema(s)", (int)m_schemas.size()); }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaSyncUpstreamHelper::ReloadAgainstSyncDb(bvector<ECN::ECSchemaPtr>& reloaded, bvector<ECN::ECSchemaCP> const& schemas, ECDbR syncConn) {
    bvector<ECSchemaCP> inDependencyOrder(schemas);
    ECSchema::SortSchemasInDependencyOrder(inDependencyOrder);

    SchemaSetLocater copies;
    auto readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(copies);
    readContext->AddSchemaLocater(syncConn.GetSchemaLocater());

    for (auto schema : inDependencyOrder) {
        // CopySchema reports a missing reference as a bare SchemaNotFound, so resolve them here
        // first - otherwise a failure cannot say which reference of which schema was the problem.
        for (auto const& reference : schema->GetReferencedSchemas()) {
            SchemaKey key(reference.first);
            if (readContext->LocateSchema(key, SchemaMatchType::Latest).IsNull()) {
                syncConn.GetImpl().Issues().ReportV(
                    IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0585,
                    "Cannot import '%s' through the schema sync db: it references '%s', which the sync db does not have.",
                    schema->GetFullSchemaName().c_str(), key.GetFullSchemaName().c_str());
                return ERROR;
            }
        }

        ECSchemaPtr copy;
        const auto status = schema->CopySchema(copy, readContext.get());
        if (status != ECObjectsStatus::Success || copy.IsNull()) {
            syncConn.GetImpl().Issues().ReportV(
                IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0585,
                "Failed to re-point '%s' at the schema sync db (ECObjectsStatus %d). Context: %s",
                schema->GetFullSchemaName().c_str(), (int)status, readContext->GetDescription().c_str());
            return ERROR;
        }

        // CopySchema deliberately does not carry this over, and SchemaWriter refuses to decrease it.
        copy->SetOriginalECXmlVersion(schema->GetOriginalECXmlVersionMajor(), schema->GetOriginalECXmlVersionMinor());

        copies.Add(copy);
        reloaded.push_back(copy);
    }

    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int64_t SchemaSyncUpstreamHelper::CountClosureRows(ECDbR conn, Utf8CP tempTableName) {
    Statement stmt;
    if (stmt.Prepare(conn, SqlPrintfString("SELECT COUNT(*) FROM %s", tempTableName).GetUtf8CP()) != BE_SQLITE_OK)
        return -1;
    if (stmt.Step() != BE_SQLITE_ROW)
        return -1;
    return stmt.GetValueInt64(0);
}

//---------------------------------------------------------------------------------------
// Builds the id sets that define what "belongs to" the requested schemas.
//
// The order below matters, because each set is derived from the previous one:
//   schemas -> classes -> columns (from property maps) -> tables -> remaining structural columns
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncUpstreamHelper::BuildClosure(ECDbR conn, Utf8CP syncAlias, StringList const& schemaNames) {
    auto rc = DropClosure(conn);
    if (rc != BE_SQLITE_OK)
        return rc;

    for (auto tempTable : { TEMP_SCHEMA_IDS, TEMP_CLASS_IDS, TEMP_TABLE_IDS, TEMP_COLUMN_IDS, TEMP_SEED_IDS }) {
        rc = conn.ExecuteSql(SqlPrintfString("CREATE TABLE %s(Id INTEGER PRIMARY KEY)", tempTable).GetUtf8CP());
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("SchemaSyncUpstreamHelper::BuildClosure(): Failed to create %s. %s", tempTable, BeSQLiteLib::GetErrorString(rc));
            return rc;
        }
    }

    // Seed: the schemas the caller named. A name that does not exist in the sync db is an error
    // rather than an empty result - silently adopting nothing would look like success.
    {
        Statement stmt;
        rc = stmt.Prepare(conn, SqlPrintfString("INSERT OR IGNORE INTO %s(Id) SELECT Id FROM [%s].ec_Schema WHERE Name=?", TEMP_SEED_IDS, syncAlias).GetUtf8CP());
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("SchemaSyncUpstreamHelper::BuildClosure(): Failed to prepare schema seed statement. %s", BeSQLiteLib::GetErrorString(rc));
            return rc;
        }
        for (auto const& name : schemaNames) {
            stmt.Reset();
            stmt.ClearBindings();
            if ((rc = stmt.BindText(1, name.c_str(), Statement::MakeCopy::No)) != BE_SQLITE_OK)
                return rc;
            if ((rc = stmt.Step()) != BE_SQLITE_DONE) {
                LOG.errorv("SchemaSyncUpstreamHelper::BuildClosure(): Failed to seed schema '%s'. %s", name.c_str(), BeSQLiteLib::GetErrorString(rc));
                return rc;
            }
            if (conn.GetModifiedRowCount() == 0) {
                // Either the schema is missing from the sync db, or a previously named schema already
                // pulled it in. Only the former is a problem.
                Statement exists;
                if (exists.Prepare(conn, SqlPrintfString("SELECT 1 FROM [%s].ec_Schema WHERE Name=?", syncAlias).GetUtf8CP()) != BE_SQLITE_OK)
                    return BE_SQLITE_ERROR;
                exists.BindText(1, name.c_str(), Statement::MakeCopy::No);
                if (exists.Step() != BE_SQLITE_ROW) {
                    LOG.errorv("SchemaSyncUpstreamHelper::BuildClosure(): Schema '%s' does not exist in the sync db.", name.c_str());
                    return BE_SQLITE_NOTFOUND;
                }
            }
        }
    }

    // Expand over references. A schema's classes may derive from, or refer to, anything in a schema
    // it references, so the referenced schemas have to come along - at the sync db's version, which
    // is the point of doing this at all.
    rc = conn.ExecuteSql(SqlPrintfString(
        "INSERT OR IGNORE INTO %s(Id) "
        "WITH RECURSIVE closure(Id) AS ("
        "  SELECT Id FROM %s"
        "  UNION"
        "  SELECT r.ReferencedSchemaId FROM [%s].ec_SchemaReference r JOIN closure c ON r.SchemaId = c.Id"
        "    WHERE r.ReferencedSchemaId IS NOT NULL"
        ") SELECT Id FROM closure",
        TEMP_SCHEMA_IDS, TEMP_SEED_IDS, syncAlias).GetUtf8CP());
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("SchemaSyncUpstreamHelper::BuildClosure(): Failed to expand schema references. %s", BeSQLiteLib::GetErrorString(rc));
        return rc;
    }

    rc = conn.ExecuteSql(SqlPrintfString(
        "INSERT OR IGNORE INTO %s(Id) SELECT Id FROM [%s].ec_Class WHERE SchemaId IN (SELECT Id FROM %s)",
        TEMP_CLASS_IDS, syncAlias, TEMP_SCHEMA_IDS).GetUtf8CP());
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("SchemaSyncUpstreamHelper::BuildClosure(): Failed to collect class ids. %s", BeSQLiteLib::GetErrorString(rc));
        return rc;
    }

    // Columns the closure's classes actually map to. This is the precise notion of "a column that
    // belongs to us": a shared column allocated by somebody else's un-pushed import is not in here,
    // and that is exactly the clutter we are keeping out.
    rc = conn.ExecuteSql(SqlPrintfString(
        "INSERT OR IGNORE INTO %s(Id) SELECT DISTINCT pm.ColumnId FROM [%s].ec_PropertyMap pm WHERE pm.ClassId IN (SELECT Id FROM %s)",
        TEMP_COLUMN_IDS, syncAlias, TEMP_CLASS_IDS).GetUtf8CP());
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("SchemaSyncUpstreamHelper::BuildClosure(): Failed to collect mapped column ids. %s", BeSQLiteLib::GetErrorString(rc));
        return rc;
    }

    // Tables reached through those columns, plus tables the closure owns outright. Collected into
    // the seed set first, because the parent walk below has to read from a different table than it
    // writes to.
    rc = conn.ExecuteSql(SqlPrintfString("DELETE FROM %s", TEMP_SEED_IDS).GetUtf8CP());
    if (rc != BE_SQLITE_OK)
        return rc;

    rc = conn.ExecuteSql(SqlPrintfString(
        "INSERT OR IGNORE INTO %s(Id) SELECT DISTINCT c.TableId FROM [%s].ec_Column c WHERE c.Id IN (SELECT Id FROM %s)",
        TEMP_SEED_IDS, syncAlias, TEMP_COLUMN_IDS).GetUtf8CP());
    if (rc != BE_SQLITE_OK)
        return rc;

    rc = conn.ExecuteSql(SqlPrintfString(
        "INSERT OR IGNORE INTO %s(Id) SELECT t.Id FROM [%s].ec_Table t WHERE t.ExclusiveRootClassId IN (SELECT Id FROM %s)",
        TEMP_SEED_IDS, syncAlias, TEMP_CLASS_IDS).GetUtf8CP());
    if (rc != BE_SQLITE_OK)
        return rc;

    // Overflow and joined tables point at their parent, and the parent has to exist locally before
    // the child can be described.
    rc = conn.ExecuteSql(SqlPrintfString(
        "INSERT OR IGNORE INTO %s(Id) "
        "WITH RECURSIVE parents(Id) AS ("
        "  SELECT Id FROM %s"
        "  UNION"
        "  SELECT t.ParentTableId FROM [%s].ec_Table t JOIN parents p ON t.Id = p.Id WHERE t.ParentTableId IS NOT NULL"
        ") SELECT Id FROM parents",
        TEMP_TABLE_IDS, TEMP_SEED_IDS, syncAlias).GetUtf8CP());
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("SchemaSyncUpstreamHelper::BuildClosure(): Failed to collect parent tables. %s", BeSQLiteLib::GetErrorString(rc));
        return rc;
    }

    // Structural columns of those tables that no property maps to at all - anything ECDb needs to
    // describe the table itself. Shared columns are deliberately excluded here: an unallocated slot
    // in the pool is not ours until something in the closure maps to it.
    rc = conn.ExecuteSql(SqlPrintfString(
        "INSERT OR IGNORE INTO %s(Id) SELECT c.Id FROM [%s].ec_Column c "
        "WHERE c.TableId IN (SELECT Id FROM %s) AND c.ColumnKind <> %d "
        "  AND NOT EXISTS (SELECT 1 FROM [%s].ec_PropertyMap pm WHERE pm.ColumnId = c.Id)",
        TEMP_COLUMN_IDS, syncAlias, TEMP_TABLE_IDS, (int)DbColumn::Kind::SharedData, syncAlias).GetUtf8CP());
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("SchemaSyncUpstreamHelper::BuildClosure(): Failed to collect structural column ids. %s", BeSQLiteLib::GetErrorString(rc));
        return rc;
    }

    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncUpstreamHelper::UpsertFiltered(ECDbR conn, Utf8CP tableName, Utf8CP sourceAlias, Utf8CP targetAlias, Utf8CP whereClause) {
    SchemaSyncHelper::StringList cols;
    auto rc = SchemaSyncHelper::GetColumnNames(conn, sourceAlias, tableName, cols);
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("SchemaSyncUpstreamHelper::UpsertFiltered(): Failed to get column names for %s. %s", tableName, BeSQLiteLib::GetErrorString(rc));
        return rc;
    }

    SchemaSyncHelper::StringList targetCols;
    rc = SchemaSyncHelper::GetColumnNames(conn, targetAlias, tableName, targetCols);
    if (rc != BE_SQLITE_OK)
        return rc;

    if (cols.size() != targetCols.size()) {
        LOG.errorv("SchemaSyncUpstreamHelper::UpsertFiltered(): Column count mismatch for table %s.", tableName);
        return BE_SQLITE_SCHEMA;
    }

    SchemaSyncHelper::StringList setClauseExprs;
    for (auto const& col : cols)
        setClauseExprs.push_back(SqlPrintfString("%s=excluded.%s", col.c_str(), col.c_str()).GetUtf8CP());

    const auto colsSql = SchemaSyncHelper::Join(cols);
    const auto sql = Utf8String{SqlPrintfString(
        "INSERT INTO [%s].[%s](%s) SELECT %s FROM [%s].[%s] WHERE %s ON CONFLICT DO UPDATE SET %s",
        targetAlias, tableName, colsSql.c_str(),
        colsSql.c_str(), sourceAlias, tableName,
        whereClause,
        SchemaSyncHelper::Join(setClauseExprs).c_str()).GetUtf8CP()};

    Statement stmt;
    rc = stmt.Prepare(conn, sql.c_str());
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("SchemaSyncUpstreamHelper::UpsertFiltered(): Failed to prepare copy for %s. %s (sql: %s)", tableName, BeSQLiteLib::GetErrorString(rc), sql.c_str());
        return rc;
    }
    rc = stmt.Step();
    if (rc != BE_SQLITE_DONE) {
        LOG.errorv("SchemaSyncUpstreamHelper::UpsertFiltered(): Failed to copy %s. %s", tableName, BeSQLiteLib::GetErrorString(rc));
        return rc;
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncUpstreamHelper::DeleteMissing(ECDbR conn, Utf8CP tableName, Utf8CP sourceAlias, Utf8CP targetAlias, Utf8CP scopeClause) {
    SchemaSyncHelper::StringList pkCols;
    auto rc = SchemaSyncHelper::GetPrimaryKeyColumnNames(conn, targetAlias, tableName, pkCols);
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("SchemaSyncUpstreamHelper::DeleteMissing(): Failed to get primary key columns for %s. %s", tableName, BeSQLiteLib::GetErrorString(rc));
        return rc;
    }
    if (pkCols.empty()) {
        LOG.errorv("SchemaSyncUpstreamHelper::DeleteMissing(): %s has no primary key, so rows cannot be matched up.", tableName);
        return BE_SQLITE_SCHEMA;
    }

    SchemaSyncHelper::StringList keyMatchExprs;
    for (auto const& col : pkCols)
        keyMatchExprs.push_back(SqlPrintfString("[S].[%s] IS [T].[%s]", col.c_str(), col.c_str()).GetUtf8CP());

    const auto sql = Utf8String{SqlPrintfString(
        "DELETE FROM [%s].[%s] AS [T] WHERE (%s) AND NOT EXISTS (SELECT 1 FROM [%s].[%s] [S] WHERE %s)",
        targetAlias, tableName, scopeClause,
        sourceAlias, tableName,
        SchemaSyncHelper::Join(keyMatchExprs, " AND ").c_str()).GetUtf8CP()};

    rc = conn.ExecuteSql(sql.c_str());
    if (rc != BE_SQLITE_OK)
        LOG.errorv("SchemaSyncUpstreamHelper::DeleteMissing(): Failed to delete stale rows from %s. %s (sql: %s)", tableName, BeSQLiteLib::GetErrorString(rc), sql.c_str());

    return rc;
}

//---------------------------------------------------------------------------------------
// Make the target's copy of each table equal the source's, touching as few rows as possible.
//
// Two properties are in tension here and both matter.
//
// Correctness rules out SchemaSyncHelper::SyncData, which upserts with ON CONFLICT DO UPDATE and no
// conflict target. An upgrade can give the same logical row a different id, so an incoming row can
// collide with a surviving target row on one of the ec_ tables' unique indexes rather than on the
// primary key - ec_PropertyMap has UNIQUE(ClassId, PropertyPathId, ColumnId) - and the update then
// rewrites that surviving row instead of inserting a new one. Pull and push never meet this because
// both sides evolve in lockstep.
//
// Efficiency rules out the blunt fix of emptying every table and refilling it. The sync db lives in
// a CloudSqlite container with 64 KiB blocks, and every other client caches those blocks; rewriting
// rows that did not change would give them all a new block id and force a re-download of the whole
// file. So instead:
//
//   pass 1 - delete every target row that is not byte-identical to a source row with the same key,
//            which covers both "no longer exists" and "changed";
//   pass 2 - insert every source row whose key is now absent from the target.
//
// Rows that did not change are never written, so their pages - and their blocks - stay as they are.
// After pass 1 every surviving row equals its source row, so pass 2 cannot hit a unique-index
// conflict, and inserts trigger no foreign key actions.
//
// One thing to watch if this ever comes out wrong: not every ec_ foreign key cascades.
// ec_Table.ExclusiveRootClassId and ec_RelationshipConstraint.AbstractConstraintClassId are ON
// DELETE SET NULL, so deleting a stale class rewrites a surviving row in another table - possibly
// one this pass has already checked and kept. Turning foreign keys off for the duration is not
// available: SQLite ignores that pragma inside a transaction, and defer_foreign_keys only defers
// checking, not the actions.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncUpstreamHelper::MirrorTables(ECDbR conn, StringList const& tables, Utf8CP sourceAlias, Utf8CP targetAlias) {
    auto rc = conn.ExecuteSql("PRAGMA defer_foreign_keys=1");
    if (rc != BE_SQLITE_OK) {
        LOG.error("SchemaSyncUpstreamHelper::MirrorTables(): Failed to set defer_foreign_keys=1");
        return rc;
    }

    StringList deleteStatements;
    StringList insertStatements;
    for (auto const& tableName : tables) {
        Utf8CP table = tableName.c_str();
        SchemaSyncHelper::StringList cols, pkCols;
        rc = SchemaSyncHelper::GetColumnNames(conn, targetAlias, table, cols);
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("SchemaSyncUpstreamHelper::MirrorTables(): Failed to get column names for %s. %s", table, BeSQLiteLib::GetErrorString(rc));
            return rc;
        }
        rc = SchemaSyncHelper::GetPrimaryKeyColumnNames(conn, targetAlias, table, pkCols);
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("SchemaSyncUpstreamHelper::MirrorTables(): Failed to get primary key columns for %s. %s", table, BeSQLiteLib::GetErrorString(rc));
            return rc;
        }
        if (pkCols.empty()) {
            LOG.errorv("SchemaSyncUpstreamHelper::MirrorTables(): %s has no primary key, so rows cannot be matched up.", table);
            return BE_SQLITE_SCHEMA;
        }

        // IS rather than = so that two NULLs count as equal.
        SchemaSyncHelper::StringList sameRowExprs;
        for (auto const& col : cols)
            sameRowExprs.push_back(SqlPrintfString("[S].[%s] IS [T].[%s]", col.c_str(), col.c_str()).GetUtf8CP());

        SchemaSyncHelper::StringList keyMatchExprs;
        for (auto const& col : pkCols)
            keyMatchExprs.push_back(SqlPrintfString("[T].[%s] IS [S].[%s]", col.c_str(), col.c_str()).GetUtf8CP());

        deleteStatements.push_back(SqlPrintfString(
            "DELETE FROM [%s].[%s] AS [T] WHERE NOT EXISTS (SELECT 1 FROM [%s].[%s] [S] WHERE %s)",
            targetAlias, table, sourceAlias, table,
            SchemaSyncHelper::Join(sameRowExprs, " AND ").c_str()).GetUtf8CP());

        const auto colsSql = SchemaSyncHelper::Join(cols);
        insertStatements.push_back(SqlPrintfString(
            "INSERT INTO [%s].[%s](%s) SELECT %s FROM [%s].[%s] [S] WHERE NOT EXISTS (SELECT 1 FROM [%s].[%s] [T] WHERE %s)",
            targetAlias, table, colsSql.c_str(),
            colsSql.c_str(), sourceAlias, table,
            targetAlias, table,
            SchemaSyncHelper::Join(keyMatchExprs, " AND ").c_str()).GetUtf8CP());
    }

    int64_t deleted = 0;
    Utf8String deletedDetail;
    for (size_t i = 0; i < deleteStatements.size(); ++i) {
        rc = conn.ExecuteSql(deleteStatements[i].c_str());
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("SchemaSyncUpstreamHelper::MirrorTables(): Failed to drop stale rows. %s (sql: %s)", BeSQLiteLib::GetErrorString(rc), deleteStatements[i].c_str());
            return rc;
        }
        const auto affected = conn.GetModifiedRowCount();
        if (affected > 0) {
            deleted += affected;
            deletedDetail.append(SqlPrintfString(" %s=%d", tables[i].c_str(), affected).GetUtf8CP());
        }
    }

    int64_t inserted = 0;
    Utf8String insertedDetail;
    for (size_t i = 0; i < insertStatements.size(); ++i) {
        rc = conn.ExecuteSql(insertStatements[i].c_str());
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("SchemaSyncUpstreamHelper::MirrorTables(): Failed to insert missing rows. %s (sql: %s)", BeSQLiteLib::GetErrorString(rc), insertStatements[i].c_str());
            return rc;
        }
        const auto affected = conn.GetModifiedRowCount();
        if (affected > 0) {
            inserted += affected;
            insertedDetail.append(SqlPrintfString(" %s=%d", tables[i].c_str(), affected).GetUtf8CP());
        }
    }

    // Counts, not rows: this is what says whether the mirror wrote a handful of rows or churned the
    // whole file, which is the difference between a cheap upload and every client re-downloading it.
    SS_TRACE("mirror %s -> %s: deleted %lld, inserted %lld (of %d tables)", sourceAlias, targetAlias, (long long)deleted, (long long)inserted, (int)tables.size());
    if (deleted > 0)
        SS_TRACE("  deleted:%s", deletedDetail.c_str());
    if (inserted > 0)
        SS_TRACE("  inserted:%s", insertedDetail.c_str());

    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// Brings the closure's rows into line with the sync db, table by table.
//
// Two passes, children first then parents, the same shape MirrorTables uses in the other direction:
// remove rows the sync db no longer has, then upsert everything it does.
//
// The delete pass rests on the sync db being a superset of the briefcase within the closure - every
// ec_ row a sync-enabled file holds came from there - so a row inside the closure that the sync db
// lacks was deleted there rather than created here. Its scope clause is evaluated against the
// briefcase, which is why the predicates that reach through another table need a second version.
//
// ec_cache_* is left out of both passes. Those are derived, and SchemaSync::UpdateDbSchema rebuilds
// them afterwards.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncUpstreamHelper::CopyClosure(ECDbR conn, Utf8CP syncAlias, Utf8CP targetAlias) {
    auto rc = conn.ExecuteSql("PRAGMA defer_foreign_keys=1");
    if (rc != BE_SQLITE_OK) {
        LOG.error("SchemaSyncUpstreamHelper::CopyClosure(): Failed to set defer_foreign_keys=1");
        return rc;
    }

    const auto inSchemas = Utf8String{SqlPrintfString("(SELECT Id FROM %s)", TEMP_SCHEMA_IDS).GetUtf8CP()};
    const auto inClasses = Utf8String{SqlPrintfString("(SELECT Id FROM %s)", TEMP_CLASS_IDS).GetUtf8CP()};
    const auto inTables = Utf8String{SqlPrintfString("(SELECT Id FROM %s)", TEMP_TABLE_IDS).GetUtf8CP()};
    const auto inColumns = Utf8String{SqlPrintfString("(SELECT Id FROM %s)", TEMP_COLUMN_IDS).GetUtf8CP()};

    // Custom attributes hang off a polymorphic container, so the predicate has to branch on
    // ContainerType. Note the container of a relationship-constraint CA is the constraint row, not
    // the relationship class.
    const auto caFilter = [&](Utf8CP alias) {
        return Utf8String{SqlPrintfString(
            "(ContainerType = %d AND ContainerId IN %s) OR "
            "(ContainerType = %d AND ContainerId IN %s) OR "
            "(ContainerType = %d AND ContainerId IN (SELECT Id FROM [%s].ec_Property WHERE ClassId IN %s)) OR "
            "(ContainerType IN (%d,%d) AND ContainerId IN (SELECT Id FROM [%s].ec_RelationshipConstraint WHERE RelationshipClassId IN %s))",
            (int)SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema, inSchemas.c_str(),
            (int)SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class, inClasses.c_str(),
            (int)SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property, alias, inClasses.c_str(),
            (int)SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint,
            (int)SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint,
            alias, inClasses.c_str()).GetUtf8CP()};
    };
    const auto compositeUnitFilter = [&](Utf8CP alias) {
        return Utf8String{SqlPrintfString("FormatId IN (SELECT Id FROM [%s].ec_Format WHERE SchemaId IN %s)", alias, inSchemas.c_str()).GetUtf8CP()};
    };
    const auto propertyPathFilter = [&](Utf8CP alias) {
        return Utf8String{SqlPrintfString("RootPropertyId IN (SELECT Id FROM [%s].ec_Property WHERE ClassId IN %s)", alias, inClasses.c_str()).GetUtf8CP()};
    };
    const auto constraintClassFilter = [&](Utf8CP alias) {
        return Utf8String{SqlPrintfString("ConstraintId IN (SELECT Id FROM [%s].ec_RelationshipConstraint WHERE RelationshipClassId IN %s)", alias, inClasses.c_str()).GetUtf8CP()};
    };
    const auto ownedIndexes = Utf8String{SqlPrintfString("TableId IN %s AND (ClassId IS NULL OR ClassId IN %s)", inTables.c_str(), inClasses.c_str()).GetUtf8CP()};
    const auto indexColumnFilter = [&](Utf8CP alias) {
        return Utf8String{SqlPrintfString("IndexId IN (SELECT Id FROM [%s].ec_Index WHERE %s)", alias, ownedIndexes.c_str()).GetUtf8CP()};
    };

    const auto bySchema = Utf8String{SqlPrintfString("SchemaId IN %s", inSchemas.c_str()).GetUtf8CP()};
    const auto byClass = Utf8String{SqlPrintfString("ClassId IN %s", inClasses.c_str()).GetUtf8CP()};

    // Ordered so that a row's parents land before it does. Foreign keys are deferred anyway, but
    // keeping the order honest makes failures easier to read; the delete pass walks it backwards.
    // An empty delete scope means the table cannot lose rows on this path.
    struct TablePlan { Utf8CP table; Utf8String copyWhere; Utf8String deleteScope; };
    const bvector<TablePlan> plan {
        { "ec_Schema",                    Utf8String{SqlPrintfString("Id IN %s", inSchemas.c_str()).GetUtf8CP()}, Utf8String{SqlPrintfString("Id IN %s", inSchemas.c_str()).GetUtf8CP()} },
        { "ec_SchemaReference",           bySchema, bySchema },
        { "ec_Class",                     bySchema, bySchema },
        { "ec_ClassHasBaseClasses",       byClass, byClass },
        { "ec_Enumeration",               bySchema, bySchema },
        { "ec_UnitSystem",                bySchema, bySchema },
        { "ec_Phenomenon",                bySchema, bySchema },
        { "ec_Unit",                      bySchema, bySchema },
        { "ec_Format",                    bySchema, bySchema },
        { "ec_FormatCompositeUnit",       compositeUnitFilter(syncAlias), compositeUnitFilter(targetAlias) },
        { "ec_KindOfQuantity",            bySchema, bySchema },
        { "ec_PropertyCategory",          bySchema, bySchema },
        { "ec_Property",                  byClass, byClass },
        { "ec_PropertyPath",              propertyPathFilter(syncAlias), propertyPathFilter(targetAlias) },
        { "ec_RelationshipConstraint",    Utf8String{SqlPrintfString("RelationshipClassId IN %s", inClasses.c_str()).GetUtf8CP()}, Utf8String{SqlPrintfString("RelationshipClassId IN %s", inClasses.c_str()).GetUtf8CP()} },
        { "ec_RelationshipConstraintClass", constraintClassFilter(syncAlias), constraintClassFilter(targetAlias) },
        { "ec_CustomAttribute",           caFilter(syncAlias), caFilter(targetAlias) },
        // A table only disappears when the class that owned it does, which the update path refuses,
        // and a table the sync db dropped is not in the closure to begin with.
        { "ec_Table",                     Utf8String{SqlPrintfString("Id IN %s", inTables.c_str()).GetUtf8CP()}, Utf8String{} },
        // Scoped by table rather than by the closure's column ids, because a column the sync db
        // deleted is not among them.
        { "ec_Column",                    Utf8String{SqlPrintfString("Id IN %s", inColumns.c_str()).GetUtf8CP()}, Utf8String{SqlPrintfString("TableId IN %s", inTables.c_str()).GetUtf8CP()} },
        { "ec_ClassMap",                  byClass, byClass },
        { "ec_PropertyMap",               Utf8String{SqlPrintfString("ClassId IN %s AND ColumnId IN %s", inClasses.c_str(), inColumns.c_str()).GetUtf8CP()}, byClass },
        { "ec_Index",                     ownedIndexes, ownedIndexes },
        { "ec_IndexColumn",               Utf8String{SqlPrintfString("ColumnId IN %s AND %s", inColumns.c_str(), indexColumnFilter(syncAlias).c_str()).GetUtf8CP()}, indexColumnFilter(targetAlias) },
    };

    int64_t deleted = 0;
    Utf8String deletedDetail;
    for (auto entry = plan.rbegin(); entry != plan.rend(); ++entry) {
        if (entry->deleteScope.empty())
            continue;

        rc = DeleteMissing(conn, entry->table, syncAlias, targetAlias, entry->deleteScope.c_str());
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("SchemaSyncUpstreamHelper::CopyClosure(): Failed to delete stale rows from %s.", entry->table);
            return rc;
        }
        const auto affected = conn.GetModifiedRowCount();
        if (affected > 0) {
            deleted += affected;
            deletedDetail.append(SqlPrintfString(" %s=%d", entry->table, affected).GetUtf8CP());
        }
    }

    int64_t copied = 0;
    Utf8String detail;
    for (auto const& entry : plan) {
        rc = UpsertFiltered(conn, entry.table, syncAlias, targetAlias, entry.copyWhere.c_str());
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("SchemaSyncUpstreamHelper::CopyClosure(): Failed on table %s.", entry.table);
            return rc;
        }
        const auto affected = conn.GetModifiedRowCount();
        if (affected > 0) {
            copied += affected;
            detail.append(SqlPrintfString(" %s=%d", entry.table, affected).GetUtf8CP());
        }
    }

    if (deleted > 0)
        SS_TRACE("adopt deleted %lld stale rows:%s", (long long)deleted, deletedDetail.c_str());
    SS_TRACE("adopt copied %lld rows from the sync db:%s", (long long)copied, detail.c_str());
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::AdoptSchemas(SyncDbUri const& syncDbUri, bvector<Utf8String> const& schemaNames) {
    ECDB_PERF_LOG_SCOPE("Adopting schemas from schema sync db");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SchemaSync::AdoptSchemas");

    if (schemaNames.empty()) {
        LOG.error("SchemaSync::AdoptSchemas(): No schema names given.");
        return Status::ERROR;
    }

    BeMutexHolder holder(m_conn.GetImpl().GetMutex());
    const auto effectiveSyncDbUri = syncDbUri.IsEmpty() ? GetDefaultSyncDbUri() : syncDbUri;

    const auto vrc = VerifySyncDb(effectiveSyncDbUri, true, false);
    if (vrc != Status::OK) {
        LOG.error("SchemaSync::AdoptSchemas(): Failed to verify sync db.");
        return vrc;
    }

    if (SchemaSyncHelper::VerifyAlias(m_conn) != BE_SQLITE_OK) {
        LOG.error("SchemaSync::AdoptSchemas(): Failed to verify alias.");
        return Status::ERROR;
    }

    auto& mainDisp = m_conn.Schemas().Main();
    mainDisp.OnBeforeSchemaChanges().RaiseEvent(m_conn, SchemaChangeType::SchemaImport);
    m_conn.ClearECDbCache();

    // Attaching and detaching both commit, so everything this call does has to happen between the
    // two - otherwise a failure in the second half leaves committed ec_ rows describing tables that
    // were never built, and AbandonChanges cannot reach them.
    auto rc = m_conn.AttachDb(effectiveSyncDbUri.GetDbAttachUri().c_str(), SchemaSyncHelper::ALIAS_SYNC_DB);
    if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0630,
            "Unable to attach sync db '%s' as '%s' to primary connection: %s",
            effectiveSyncDbUri.GetUri().c_str(), SchemaSyncHelper::ALIAS_SYNC_DB, m_conn.GetLastError().c_str());
        return Status::ERROR_UNABLE_TO_ATTACH;
    }

    const auto cleanup = [&](Status status) {
        SchemaSyncUpstreamHelper::DropClosure(m_conn);
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return status;
    };

    // The ordinary import path drops these too - they render the old mapping and are never
    // recreated automatically.
    if (SUCCESS != ViewGenerator::DropECClassViews(m_conn)) {
        LOG.error("SchemaSync::AdoptSchemas(): Failed to drop ECClass views.");
        m_conn.AbandonChanges();
        return cleanup(Status::ERROR);
    }

    BeginModifiedRowCount();
    rc = SchemaSyncUpstreamHelper::BuildClosure(m_conn, SchemaSyncHelper::ALIAS_SYNC_DB, schemaNames);
    if (rc != BE_SQLITE_OK) {
        LOG.error("SchemaSync::AdoptSchemas(): Failed to build schema closure.");
        m_conn.AbandonChanges();
        return cleanup(Status::ERROR);
    }

    SS_TRACE("adopt %s: closure is %lld schemas, %lld classes, %lld tables, %lld columns",
        SchemaSyncHelper::Join(schemaNames, ",").c_str(),
        (long long)SchemaSyncUpstreamHelper::CountClosureRows(m_conn, SchemaSyncUpstreamHelper::TEMP_SCHEMA_IDS),
        (long long)SchemaSyncUpstreamHelper::CountClosureRows(m_conn, SchemaSyncUpstreamHelper::TEMP_CLASS_IDS),
        (long long)SchemaSyncUpstreamHelper::CountClosureRows(m_conn, SchemaSyncUpstreamHelper::TEMP_TABLE_IDS),
        (long long)SchemaSyncUpstreamHelper::CountClosureRows(m_conn, SchemaSyncUpstreamHelper::TEMP_COLUMN_IDS));

    rc = SchemaSyncUpstreamHelper::CopyClosure(m_conn, SchemaSyncHelper::ALIAS_SYNC_DB, SchemaSyncHelper::ALIAS_MAIN_DB);
    if (rc != BE_SQLITE_OK) {
        LOG.error("SchemaSync::AdoptSchemas(): Failed to copy schema closure.");
        m_conn.AbandonChanges();
        return cleanup(Status::ERROR);
    }
    EndModifiedRowCount();

    // Materialise the physical tables and indexes the adopted rows imply. This is the same step a
    // pull ends with, and it is why no DDL has to travel between the two files.
    const auto updateRc = UpdateDbSchema();
    if (updateRc != Status::OK) {
        LOG.error("SchemaSync::AdoptSchemas(): Failed to update db schema.");
        m_conn.AbandonChanges();
        return cleanup(updateRc);
    }

    // Adopting can push a class that already holds data into an overflow table. An ordinary import
    // ends by giving every existing instance its matching overflow row; here that import ran in the
    // sync db, which holds no data, so it had nothing to do - and UpdateDbSchema only creates tables
    // and indexes. Without this, instances that predate the widening have no overflow row and every
    // write to a property that landed there is silently lost.
    if (BE_SQLITE_OK != m_conn.Schemas().Main().UpgradeECInstances()) {
        LOG.error("SchemaSync::AdoptSchemas(): Failed to give existing instances their overflow rows.");
        m_conn.AbandonChanges();
        return cleanup(Status::ERROR);
    }

    const auto detachRc = cleanup(Status::OK);
    UNUSED_VARIABLE(detachRc);

    SS_TRACE("adopt done: tables and indexes materialised, overflow rows caught up");

    mainDisp.OnAfterSchemaChanges().RaiseEvent(m_conn, SchemaChangeType::SchemaImport);
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SchemaSync::AdoptSchemas");
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::ImportSchemas(SyncDbUri const& syncDbUri, bvector<ECN::ECSchemaCP> const& schemas) {
    ECDB_PERF_LOG_SCOPE("Importing schemas through the schema sync db");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SchemaSync::ImportSchemas");

    if (schemas.empty()) {
        LOG.error("SchemaSync::ImportSchemas(): No schemas given.");
        return Status::ERROR;
    }

    BeMutexHolder holder(m_conn.GetImpl().GetMutex());
    const auto effectiveSyncDbUri = syncDbUri.IsEmpty() ? GetDefaultSyncDbUri() : syncDbUri;

    const auto vrc = VerifySyncDb(effectiveSyncDbUri, true, false);
    if (vrc != Status::OK) {
        LOG.error("SchemaSync::ImportSchemas(): Failed to verify sync db.");
        return vrc;
    }

    const auto prc = VerifyProfileVersionsMatch(effectiveSyncDbUri);
    if (prc != Status::OK)
        return prc;

    const auto dataVerBeforeImport = SyncDbInfo::From(effectiveSyncDbUri).GetDataVersion();
    SS_TRACE("import: %d schema(s), sync db dataVer %lld", (int)schemas.size(), (long long)dataVerBeforeImport);

    // Step 1. The import runs in the sync db, which is where ids and physical layout get decided.
    bvector<Utf8String> importedSchemaNames;
    auto status = ImportIntoSyncDb(effectiveSyncDbUri, schemas, importedSchemaNames, dataVerBeforeImport);
    if (status != Status::OK)
        return status;

    status = UpdateDataVersion(effectiveSyncDbUri);
    if (status != Status::OK) {
        LOG.error("SchemaSync::ImportSchemas(): Failed to update the data version.");
        return status;
    }

    // Step 2. Everything the sync db decided is now taken over verbatim; nothing is decided here.
    status = AdoptSchemas(effectiveSyncDbUri, importedSchemaNames);
    if (status != Status::OK) {
        LOG.error("SchemaSync::ImportSchemas(): Failed to adopt the imported schemas.");
        return status;
    }

    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SchemaSync::ImportSchemas");
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// Step 1 of SchemaSync::ImportSchemas, split out so the sync db connection is closed before the
// briefcase attaches the same file to adopt from it.
//
// The import is the ordinary one, run with DoNotCreateOrUpdateDataTables so it writes ec_ rows and
// nothing else. The sync db is the record of what was decided; building the tables that decision
// implies is the adopting briefcase's job.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::ImportIntoSyncDb(SyncDbUri const& syncDbUri, bvector<ECN::ECSchemaCP> const& schemas, bvector<Utf8String>& importedSchemaNames, DataVer dataVerBeforeImport) {
    ECDb syncConn;
    Db::OpenParams openParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes);
    ParseQueryParams(openParams, syncDbUri);
    auto rc = syncConn.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams);
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("SchemaSync::ImportSchemas(): Failed to open sync db '%s'. %s", syncDbUri.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_OPENING_SCHEMA_SYNC_DB;
    }

    // Init strips the sync db's local info exactly so that it is not itself a schema sync client.
    // If that ever stopped holding, the import below would try to sync to a sync db of its own.
    if (syncConn.Schemas().GetSchemaSync().IsEnabled()) {
        LOG.error("SchemaSync::ImportSchemas(): The sync db is itself set up to use schema sync, which is not valid.");
        return Status::ERROR_INVALID_SCHEMA_SYNC_DB;
    }

#if SCHEMA_SYNC_UPSTREAM_TRACE
    SchemaSyncTraceIssueListener traceIssues;
    syncConn.AddIssueListener(traceIssues);
#endif

    bvector<ECSchemaPtr> reloaded;
    if (SUCCESS != SchemaSyncUpstreamHelper::ReloadAgainstSyncDb(reloaded, schemas, syncConn)) {
        LOG.error("SchemaSync::ImportSchemas(): Failed to re-point the schemas at the sync db.");
        return Status::ERROR;
    }

    bvector<ECSchemaCP> schemasToImport;
    for (auto const& schema : reloaded) {
        schemasToImport.push_back(schema.get());
        importedSchemaNames.push_back(schema->GetName());
    }

    const auto importRc = syncConn.Schemas().ImportSchemas(schemasToImport, SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables);
    SS_TRACE("import into sync db: %s -> %d", SchemaSyncHelper::Join(importedSchemaNames, ",").c_str(), (int)(SchemaImportResult::Status)importRc);
    if (importRc == SchemaImportResult::ERROR_DATA_TRANSFORM_REQUIRED) {
        LOG.info("SchemaSync::ImportSchemas(): The import would have to move data, which the additive path does not do.");
        syncConn.AbandonChanges();
        return Status::ERROR_DATA_TRANSFORM_REQUIRED;
    }

    if (!importRc.IsOk()) {
        LOG.errorv("SchemaSync::ImportSchemas(): The import into the sync db failed. %s", syncConn.GetLastError().c_str());
        SS_TRACE("  sqlite says: %s", syncConn.GetLastError().c_str());
        syncConn.AbandonChanges();
        return Status::ERROR;
    }

    // The caller holds the container write lock for this whole call, so the data version cannot have
    // moved. Checking it anyway backstops against somebody writing without the lock.
    if (SyncDbInfo::From(syncConn).GetDataVersion() != dataVerBeforeImport) {
        LOG.error("SchemaSync::ImportSchemas(): The sync db was written to during the import, which means it was written to without the lock.");
        syncConn.AbandonChanges();
        return Status::ERROR_SYNC_DB_CHANGED;
    }

    rc = syncConn.SaveChanges();
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("SchemaSync::ImportSchemas(): Failed to save the sync db. %s", BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR;
    }

    syncConn.CloseDb();
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::UpgradeSchemas(SyncDbUri const& syncDbUri, bvector<ECN::ECSchemaCP> const& schemas) {
    ECDB_PERF_LOG_SCOPE("Upgrading schemas and overwriting the schema sync db");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SchemaSync::UpgradeSchemas");

    if (schemas.empty()) {
        LOG.error("SchemaSync::UpgradeSchemas(): No schemas given.");
        return Status::ERROR;
    }

    BeMutexHolder holder(m_conn.GetImpl().GetMutex());
    const auto effectiveSyncDbUri = syncDbUri.IsEmpty() ? GetDefaultSyncDbUri() : syncDbUri;

    const auto vrc = VerifySyncDb(effectiveSyncDbUri, false, false);
    if (vrc != Status::OK) {
        LOG.error("SchemaSync::UpgradeSchemas(): Failed to verify sync db.");
        return vrc;
    }

    const auto prc = VerifyProfileVersionsMatch(effectiveSyncDbUri);
    if (prc != Status::OK)
        return prc;

    // No re-pointing here, which is the opposite of ImportSchemas: the schemas arrive resolved
    // against this briefcase and the briefcase is what decides. Whatever the sync db holds beyond it
    // is about to be thrown away anyway.
    //
    // The ordinary upgrade, run locally and unmodified. Schema sync is switched off for it so that
    // the pull/push hooks stay out of the way: a pull would drag in exactly the abandoned rows we
    // are about to delete, and a push would refuse on the data version.
    SchemaImportResult importRc = SchemaImportResult::ERROR;
        {
        DisableSchemaSync();
        importRc = m_conn.Schemas().ImportSchemas(schemas, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);
        ReEnableSchemaSync();
        }

    SS_TRACE("upgrade: local import of %d schema(s) with transforms allowed -> %d", (int)schemas.size(), (int)(SchemaImportResult::Status)importRc);

    if (!importRc.IsOk()) {
        LOG.error("SchemaSync::UpgradeSchemas(): The local import failed.");
        m_conn.AbandonChanges();
        return Status::ERROR;
    }

    auto status = OverwriteSyncDb(effectiveSyncDbUri);
    if (status != Status::OK) {
        LOG.error("SchemaSync::UpgradeSchemas(): Failed to overwrite the sync db from this briefcase.");
        m_conn.AbandonChanges();
        return status;
    }

    status = UpdateDataVersion(effectiveSyncDbUri);
    if (status != Status::OK) {
        LOG.error("SchemaSync::UpgradeSchemas(): Failed to update the data version.");
        m_conn.AbandonChanges();
        return status;
    }

    SS_TRACE("upgrade done: sync db now at dataVer %lld", (long long)GetInfo().GetDataVersion());
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SchemaSync::UpgradeSchemas");
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// Replace the sync db's ec_ rows with this briefcase's.
//
// This is a push without its "am I level with the sync db" precondition, which is the whole
// point: the sync db may hold rows from an import that was never pushed, and those are what we
// want gone. The copy itself is a differential mirror rather than an upsert - see MirrorTables.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::OverwriteSyncDb(SyncDbUri const& syncDbUri) {
    if (SchemaSyncHelper::VerifyAlias(m_conn) != BE_SQLITE_OK) {
        LOG.error("SchemaSync::OverwriteSyncDb(): Failed to verify alias.");
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

    TableList tables;
    rc = SchemaSyncHelper::GetMetaTables(m_conn, tables, SchemaSyncHelper::ALIAS_MAIN_DB);
    if (rc != BE_SQLITE_OK) {
        LOG.error("SchemaSync::OverwriteSyncDb(): Failed to read the list of meta tables.");
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return Status::ERROR;
    }

    // ec_cache_ is derived, holds nothing of its own, and every import regenerates it. Copying it
    // would also be the dominant cost, since its ids are handed out fresh on each rebuild and the
    // two files therefore disagree on nearly every row even when the class hierarchy is untouched.
    TableList tablesToMirror;
    for (auto const& table : tables) {
        if (!table.StartsWithIAscii("ec_cache_"))
            tablesToMirror.push_back(table);
    }

    rc = SchemaSyncUpstreamHelper::MirrorTables(m_conn, tablesToMirror, SchemaSyncHelper::ALIAS_MAIN_DB, SchemaSyncHelper::ALIAS_SYNC_DB);
    if (rc != BE_SQLITE_OK) {
        LOG.error("SchemaSync::OverwriteSyncDb(): Failed to bring the sync db's ec_ tables in line with this briefcase.");
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return Status::ERROR;
    }

#if SCHEMA_SYNC_UPSTREAM_TRACE
    {
    Statement fkStmt;
    if (fkStmt.Prepare(m_conn, SqlPrintfString("PRAGMA [%s].foreign_key_check", SchemaSyncHelper::ALIAS_SYNC_DB).GetUtf8CP()) == BE_SQLITE_OK) {
        int violations = 0;
        while (fkStmt.Step() == BE_SQLITE_ROW) {
            ++violations;
            SS_TRACE("  sync db FK violation: table=%s rowid=%lld parent=%s fkid=%d",
                fkStmt.GetValueText(0), (long long)fkStmt.GetValueInt64(1), fkStmt.GetValueText(2), fkStmt.GetValueInt(3));
        }
        if (violations == 0)
            SS_TRACE("overwrite: sync db has no dangling references");
    }
    }
#endif

    rc = m_conn.SaveChanges();
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("SchemaSync::OverwriteSyncDb(): Failed to save. %s", BeSQLiteLib::GetErrorString(rc));
        m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
        return Status::ERROR;
    }

    rc = m_conn.DetachDb(SchemaSyncHelper::ALIAS_SYNC_DB);
    if (rc != BE_SQLITE_OK) {
        LOG.error("SchemaSync::OverwriteSyncDb(): Failed to detach the sync db.");
        return Status::ERROR;
    }

    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSync::ScanForSchemaChanges(ChangeStream& stream, bool& isECMetaDataChanged, bool& isECDbProfileChanged, bool& isSchemaSyncInfoChanged) {
    // Check if be_Prop has change for NameSpace=JsonNames::JNamespace && Name=JsonNames::JLocalDbInfo
    // every time schema sync is used it will increment data ver which should change JLocalDbInfo
    isSchemaSyncInfoChanged = false;
    isECDbProfileChanged = false;
    isECMetaDataChanged = false;
    Utf8String tableName;
    for(auto& change : stream.GetChanges()) {
        Utf8CP tableNameP = nullptr;
        int nCols;
        DbOpcode opcode;
        int indirect;
        auto rc = change.GetOperation(&tableNameP, &nCols, &opcode, &indirect);
        if (BE_SQLITE_OK != rc)
            return rc;

        UNUSED_VARIABLE(nCols);
        UNUSED_VARIABLE(opcode);
        UNUSED_VARIABLE(indirect);

        tableName.AssignOrClear(tableNameP);
        if (!isECMetaDataChanged
            && (tableName.StartsWithIAscii("ec_"))
            && !tableName.StartsWithIAscii("ec_cache_")) {
            isECMetaDataChanged = true;
        }
        if (tableName.EqualsIAscii("be_Prop")) {
            if (!isSchemaSyncInfoChanged) {
                auto namespaceVal = change.GetOldValue(0);
                auto nameVal = change.GetOldValue(1);
                const auto ns = namespaceVal.IsValid() && namespaceVal.GetValueType() == DbValueType::TextVal ? namespaceVal.GetValueText() : nullptr;
                const auto name = nameVal.IsValid() && nameVal.GetValueType() == DbValueType::TextVal ? nameVal.GetValueText() : nullptr;
                if (ns && name &&
                    0 == BeStringUtilities::StricmpAscii(ns, JsonNames::JNamespaceEC) &&
                    0 == BeStringUtilities::StricmpAscii(name, JsonNames::JLocalDbInfo)) {
                    isSchemaSyncInfoChanged = true;
                }
            }
            if (!isECDbProfileChanged) {
                auto namespaceVal = change.GetOldValue(0);
                auto nameVal = change.GetOldValue(1);
                const auto ns = namespaceVal.IsValid() && namespaceVal.GetValueType() == DbValueType::TextVal ? namespaceVal.GetValueText() : nullptr;
                const auto name = nameVal.IsValid() && nameVal.GetValueType() == DbValueType::TextVal ? nameVal.GetValueText() : nullptr;
                if (ns && name &&
                    0 == BeStringUtilities::StricmpAscii(ns, ECDB_PROPSPEC_NAMESPACE) &&
                    0 == BeStringUtilities::StricmpAscii(name, "SchemaVersion")) {
                    isECDbProfileChanged = true;
                }
            }
        }
    }
    return BE_SQLITE_OK;
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
    const auto propSpec = PropertySpec(JsonNames::JSyncDbInfo, JsonNames::JNamespaceEC);
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
    const auto propSpec = PropertySpec(JsonNames::JLocalDbInfo, JsonNames::JNamespaceEC);
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
    const auto propSpec = PropertySpec(JsonNames::JSyncDbInfo, JsonNames::JNamespaceEC);
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
    const auto propSpec = PropertySpec(JsonNames::JLocalDbInfo, JsonNames::JNamespaceEC);
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

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::SyncProfileTablesSchema(DbR thisDb, SchemaSync::SyncDbUri const& syncDbUri, bool thisDbToSyncDb) {
    Db conn;
    Db::OpenParams openParams(thisDbToSyncDb ? Db::OpenMode::ReadWrite : Db::OpenMode::Readonly);
    SchemaSync::ParseQueryParams(openParams, syncDbUri);
    auto rc = conn.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams);
    if (rc != BE_SQLITE_OK) {
        return rc;
    }
    if (thisDbToSyncDb) {
        return SchemaSyncHelper::SyncProfileTablesSchema(thisDb, conn);
    }
    return SchemaSyncHelper::SyncProfileTablesSchema(conn, thisDb);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::SyncProfileTablesSchema(DbR fromDb, DbR toDb) {
    std::vector<Utf8String> patches;
    if (toDb.IsReadonly()) {
        LOG.error("SyncProfileTablesSchema() rhsDb is readonly");
        return BE_SQLITE_READONLY;
    }
    auto rc = MetaData::SchemaDiff(fromDb, toDb,
        [](MetaData::TableInfo const& tblInfo) -> bool {
            return !(tblInfo.schema.EqualsIAscii("main")
                && (tblInfo.name.StartsWithIAscii("ec_") || tblInfo.name.StartsWithIAscii("dgn_") || tblInfo.name.StartsWithIAscii("be_"))
                && tblInfo.type == "table");
        }, patches);

    if (rc != BE_SQLITE_OK) {
        LOG.errorv("SyncProfileTablesSchema() fail to get schema diff: %s", toDb.GetLastError().c_str());
        return rc;
    }

    for (auto& patch : patches) {
        rc = toDb.ExecuteDdl(patch.c_str());
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("SyncProfileTablesSchema() fail to execute patch (%s): %s", patch.c_str(), toDb.GetLastError().c_str());
            return rc;
        }
    }
    if (!patches.empty()) {
        toDb.SaveChanges();
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncHelper::UpdateProfileVersion(DbR conn, SchemaSync::SyncDbUri syncDbUri, bool thisDbToSyncDb) {

    Db sharedDb;
    Db::OpenParams openParams(thisDbToSyncDb ? Db::OpenMode::ReadWrite : Db::OpenMode::Readonly);
    SchemaSync::ParseQueryParams(openParams, syncDbUri);
    auto rc = sharedDb.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams);
    if (rc != BE_SQLITE_OK) {
        return rc;
    }
    if (thisDbToSyncDb) {
        const auto containerECVer = SchemaSyncHelper::QueryProfileVersion(sharedDb, SchemaSyncHelper::ProfileKind::EC);
        const auto thisECVer = SchemaSyncHelper::QueryProfileVersion(conn, SchemaSyncHelper::ProfileKind::EC);
        if (thisECVer > containerECVer){
            if (BE_SQLITE_OK != SchemaSyncHelper::SaveProfileVersion(sharedDb, SchemaSyncHelper::ProfileKind::EC, thisECVer)){
                return BE_SQLITE_ERROR;
            }
        }

        const auto containerBEVer = SchemaSyncHelper::QueryProfileVersion(sharedDb, SchemaSyncHelper::ProfileKind::BE);
        const auto thisBEVer = SchemaSyncHelper::QueryProfileVersion(conn, SchemaSyncHelper::ProfileKind::BE);
        if (thisBEVer > containerBEVer){
            if (BE_SQLITE_OK != SchemaSyncHelper::SaveProfileVersion(sharedDb, SchemaSyncHelper::ProfileKind::BE, thisBEVer)){
                return BE_SQLITE_ERROR;
            }
        }

        const auto containerDGNVer = SchemaSyncHelper::QueryProfileVersion(sharedDb, SchemaSyncHelper::ProfileKind::DGN);
        const auto thisDGNVer = SchemaSyncHelper::QueryProfileVersion(conn, SchemaSyncHelper::ProfileKind::DGN);
        if (thisDGNVer > containerDGNVer){
            if (BE_SQLITE_OK != SchemaSyncHelper::SaveProfileVersion(sharedDb, SchemaSyncHelper::ProfileKind::DGN, thisDGNVer)){
                return BE_SQLITE_ERROR;
            }
        }
    } else {
        if (conn.IsReadonly()) {
            LOG.error("UpdateProfileVersion() primary connection is readonly");
            return BE_SQLITE_READONLY;
        }
        const auto containerECVer = SchemaSyncHelper::QueryProfileVersion(sharedDb, SchemaSyncHelper::ProfileKind::EC);
        const auto thisECVer = SchemaSyncHelper::QueryProfileVersion(conn, SchemaSyncHelper::ProfileKind::EC);
        if (thisECVer < containerECVer){
            if (BE_SQLITE_OK != SchemaSyncHelper::SaveProfileVersion(conn, SchemaSyncHelper::ProfileKind::EC, containerECVer)){
                return BE_SQLITE_ERROR;
            }
        }

        const auto containerBEVer = SchemaSyncHelper::QueryProfileVersion(sharedDb, SchemaSyncHelper::ProfileKind::BE);
        const auto thisBEVer = SchemaSyncHelper::QueryProfileVersion(conn, SchemaSyncHelper::ProfileKind::BE);
        if (thisBEVer < containerBEVer){
            if (BE_SQLITE_OK != SchemaSyncHelper::SaveProfileVersion(conn, SchemaSyncHelper::ProfileKind::BE, containerBEVer)){
                return BE_SQLITE_ERROR;
            }
        }

        const auto containerDGNVer = SchemaSyncHelper::QueryProfileVersion(sharedDb, SchemaSyncHelper::ProfileKind::DGN);
        const auto thisDGNVer = SchemaSyncHelper::QueryProfileVersion(conn, SchemaSyncHelper::ProfileKind::DGN);
        if (thisDGNVer < containerDGNVer){
            if (BE_SQLITE_OK != SchemaSyncHelper::SaveProfileVersion(conn, SchemaSyncHelper::ProfileKind::DGN, containerDGNVer)){
                return BE_SQLITE_ERROR;
            }
        }
    }
    return BE_SQLITE_OK;
}


END_BENTLEY_SQLITE_EC_NAMESPACE
