/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

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
        case Status::ERROR_IMPORT_LOG:
            return "ERROR_IMPORT_LOG";
        default:
            return "SCHEMA_SYNC_FAIL";
    }
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

//SchemaSyncImportLog===========================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncImportLog::EnsureTables(DbR syncDb) {
    if (syncDb.TableExists(TABLE_IMPORT) && syncDb.TableExists(TABLE_IMPORT_SCHEMA))
        return BE_SQLITE_OK;

    auto rc = syncDb.ExecuteDdl(R"sql(
        CREATE TABLE IF NOT EXISTS [schema_sync_import](
            [Id] INTEGER PRIMARY KEY,
            [Guid] TEXT NOT NULL UNIQUE,
            [UserName] TEXT,
            [Timestamp] INTEGER NOT NULL,
            [State] INTEGER NOT NULL DEFAULT 0,
            [Description] TEXT,
            [RejectedBy] TEXT,
            [RejectReason] TEXT))sql");
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("SchemaSyncImportLog::EnsureTables(): Failed to create %s. %s", TABLE_IMPORT, syncDb.GetLastError().c_str());
        return rc;
    }

    rc = syncDb.ExecuteDdl(R"sql(
        CREATE TABLE IF NOT EXISTS [schema_sync_import_schema](
            [ImportId] INTEGER NOT NULL REFERENCES [schema_sync_import]([Id]) ON DELETE CASCADE,
            [Ordinal] INTEGER NOT NULL,
            [Name] TEXT NOT NULL,
            [VersionRead] INTEGER NOT NULL,
            [VersionWrite] INTEGER NOT NULL,
            [VersionMinor] INTEGER NOT NULL,
            [IsDynamic] INTEGER NOT NULL DEFAULT 0,
            [XmlSize] INTEGER NOT NULL,
            [Xml] BLOB NOT NULL,
            PRIMARY KEY([ImportId],[Ordinal])))sql");
    if (rc != BE_SQLITE_OK) {
        LOG.errorv("SchemaSyncImportLog::EnsureTables(): Failed to create %s. %s", TABLE_IMPORT_SCHEMA, syncDb.GetLastError().c_str());
        return rc;
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaSyncImportLog::Compress(Utf8StringCR xml, ByteStreamR compressed) {
    SnappyToBlob writer;
    writer.Init();
    writer.Write((Byte const*)xml.c_str(), (uint32_t)xml.size());
    writer.Finish();
    writer.SaveTo(compressed);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaSyncImportLog::Decompress(void const* data, uint32_t size, uint32_t uncompressedSize, Utf8StringR xml) {
    xml.clear();
    if (data == nullptr || size == 0 || uncompressedSize == 0)
        return ERROR;

    ByteStream buffer;
    buffer.Resize(uncompressedSize);

    SnappyFromMemory reader;
    reader.Init(const_cast<void*>(data), size);
    uint32_t actuallyRead = 0;
    if (ZIP_SUCCESS != reader._Read(buffer.GetDataP(), uncompressedSize, actuallyRead) || actuallyRead != uncompressedSize) {
        LOG.error("SchemaSyncImportLog::Decompress(): Failed to decompress schema xml.");
        return ERROR;
    }

    xml.assign((Utf8CP)buffer.GetData(), uncompressedSize);
    return SUCCESS;
}

//SchemaSync (orchestration poc)================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::OrchestrationScope::OrchestrationScope(SchemaSync& sync, Utf8StringCR user, int64_t replayOfImportId)
    : m_sync(sync), m_prevUser(sync.m_importUser), m_prevReplayOfImportId(sync.m_replayOfImportId) {
    m_sync.m_importUser = user;
    m_sync.m_replayOfImportId = replayOfImportId;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::OrchestrationScope::~OrchestrationScope() {
    m_sync.m_importUser = m_prevUser;
    m_sync.m_replayOfImportId = m_prevReplayOfImportId;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaSync::ImportRecord::To(BeJsValue val) const {
    val.SetEmptyObject();
    val["id"] = (double)m_id;
    val["guid"] = m_guid;
    val["user"] = m_user;
    val["timestamp"] = (double)m_timestamp;
    val["state"] = m_state == ImportState::Rejected ? "rejected" : "pending";
    val["description"] = m_description;
    val["hasDynamicSchema"] = m_hasDynamicSchema;
    auto schemas = val["schemas"];
    schemas.toArray();
    for (auto const& name : m_schemaNames)
        schemas.appendValue() = name;
}

//---------------------------------------------------------------------------------------
// Opens the sync db outside of the briefcase connection. The import log is independent of
// the ec_ tables, so it does not need the attach/detach dance Pull and Push use.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static DbResult OpenSyncDbForImportLog(Db& syncDb, SchemaSync::SyncDbUri const& syncDbUri, bool writable) {
    Db::OpenParams openParams(writable ? Db::OpenMode::ReadWrite : Db::OpenMode::Readonly, DefaultTxn::Yes);
    SchemaSync::ParseQueryParams(openParams, syncDbUri);
    const auto rc = syncDb.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams);
    if (rc != BE_SQLITE_OK)
        LOG.errorv("SchemaSync: Failed to open sync db (%s). %s", syncDbUri.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::EnsureImportLog(SyncDbUri const& syncDbUri) {
    // Replaying a recorded import writes nothing to the sync db, so do not ask for write access unless
    // the tables really have to be created. That keeps a read only catch up from needing the container lock.
    {
        Db syncDb;
        if (OpenSyncDbForImportLog(syncDb, syncDbUri, false) != BE_SQLITE_OK)
            return Status::ERROR_OPENING_SCHEMA_SYNC_DB;

        if (syncDb.TableExists(SchemaSyncImportLog::TABLE_IMPORT) && syncDb.TableExists(SchemaSyncImportLog::TABLE_IMPORT_SCHEMA))
            return Status::OK;
    }

    Db syncDb;
    if (OpenSyncDbForImportLog(syncDb, syncDbUri, true) != BE_SQLITE_OK)
        return Status::ERROR_OPENING_SCHEMA_SYNC_DB;

    if (SchemaSyncImportLog::EnsureTables(syncDb) != BE_SQLITE_OK)
        return Status::ERROR_IMPORT_LOG;

    if (syncDb.SaveChanges() != BE_SQLITE_OK)
        return Status::ERROR_IMPORT_LOG;

    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static SchemaSync::Status ReadImportRecords(Db& syncDb, Utf8CP whereClause, bool hasWhereArg, int64_t whereArg, bvector<SchemaSync::ImportRecord>& records) {
    Statement stmt;
    const auto sql = Utf8PrintfString(
        "SELECT [Id],[Guid],[UserName],[Timestamp],[State],[Description] FROM [%s] %s ORDER BY [Id]",
        SchemaSyncImportLog::TABLE_IMPORT, whereClause);
    if (stmt.Prepare(syncDb, sql.c_str()) != BE_SQLITE_OK) {
        LOG.errorv("SchemaSync: Failed to prepare import log query. %s", syncDb.GetLastError().c_str());
        return SchemaSync::Status::ERROR_IMPORT_LOG;
    }
    if (hasWhereArg && stmt.BindInt64(1, whereArg) != BE_SQLITE_OK)
        return SchemaSync::Status::ERROR_IMPORT_LOG;

    while (stmt.Step() == BE_SQLITE_ROW) {
        SchemaSync::ImportRecord record;
        record.m_id = stmt.GetValueInt64(0);
        record.m_guid = stmt.GetValueText(1);
        record.m_user = stmt.IsColumnNull(2) ? "" : stmt.GetValueText(2);
        record.m_timestamp = stmt.GetValueInt64(3);
        record.m_state = (SchemaSync::ImportState)stmt.GetValueInt(4);
        record.m_description = stmt.IsColumnNull(5) ? "" : stmt.GetValueText(5);
        records.push_back(record);
    }
    stmt.Finalize();

    Statement schemaStmt;
    const auto schemaSql = Utf8PrintfString(
        "SELECT [Name],[VersionRead],[VersionWrite],[VersionMinor],[IsDynamic] FROM [%s] WHERE [ImportId]=? ORDER BY [Ordinal]",
        SchemaSyncImportLog::TABLE_IMPORT_SCHEMA);
    if (schemaStmt.Prepare(syncDb, schemaSql.c_str()) != BE_SQLITE_OK) {
        LOG.errorv("SchemaSync: Failed to prepare import log schema query. %s", syncDb.GetLastError().c_str());
        return SchemaSync::Status::ERROR_IMPORT_LOG;
    }
    for (auto& record : records) {
        schemaStmt.Reset();
        schemaStmt.ClearBindings();
        schemaStmt.BindInt64(1, record.m_id);
        while (schemaStmt.Step() == BE_SQLITE_ROW) {
            record.m_schemaNames.push_back(ECN::SchemaKey(
                schemaStmt.GetValueText(0),
                schemaStmt.GetValueInt(1),
                schemaStmt.GetValueInt(2),
                schemaStmt.GetValueInt(3)).GetFullSchemaName());
            if (schemaStmt.GetValueBoolean(4))
                record.m_hasDynamicSchema = true;
        }
    }
    return SchemaSync::Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::QueryPendingImports(SyncDbUri const& syncDbUri, bvector<ImportRecord>& records) const {
    records.clear();
    Db syncDb;
    if (OpenSyncDbForImportLog(syncDb, syncDbUri, false) != BE_SQLITE_OK)
        return Status::ERROR_OPENING_SCHEMA_SYNC_DB;

    if (!syncDb.TableExists(SchemaSyncImportLog::TABLE_IMPORT))
        return Status::OK; // nothing recorded yet

    const auto whereClause = Utf8PrintfString("WHERE [Id]>? AND [State]=%d", (int)ImportState::Pending);
    const auto lastSeen = GetLastSeenImportId();
    const auto rc = ReadImportRecords(syncDb, whereClause.c_str(), true, lastSeen, records);
    LOG.infov("SchemaSync orchestration: %d import(s) pending after %" PRId64 ".", (int)records.size(), lastSeen);
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::QueryImports(SyncDbUri const& syncDbUri, bvector<ImportRecord>& records) const {
    records.clear();
    Db syncDb;
    if (OpenSyncDbForImportLog(syncDb, syncDbUri, false) != BE_SQLITE_OK)
        return Status::ERROR_OPENING_SCHEMA_SYNC_DB;

    if (!syncDb.TableExists(SchemaSyncImportLog::TABLE_IMPORT))
        return Status::OK;

    return ReadImportRecords(syncDb, "", false, 0, records);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::QueryImportSchemaXml(SyncDbUri const& syncDbUri, int64_t importId, bvector<Utf8String>& schemaXml) const {
    schemaXml.clear();
    Db syncDb;
    if (OpenSyncDbForImportLog(syncDb, syncDbUri, false) != BE_SQLITE_OK)
        return Status::ERROR_OPENING_SCHEMA_SYNC_DB;

    if (!syncDb.TableExists(SchemaSyncImportLog::TABLE_IMPORT_SCHEMA))
        return Status::ERROR_IMPORT_LOG;

    Statement stmt;
    const auto sql = Utf8PrintfString(
        "SELECT [XmlSize],[Xml] FROM [%s] WHERE [ImportId]=? ORDER BY [Ordinal]", SchemaSyncImportLog::TABLE_IMPORT_SCHEMA);
    if (stmt.Prepare(syncDb, sql.c_str()) != BE_SQLITE_OK)
        return Status::ERROR_IMPORT_LOG;

    stmt.BindInt64(1, importId);
    while (stmt.Step() == BE_SQLITE_ROW) {
        void const* blob = stmt.GetValueBlob(1);
        const auto blobSize = stmt.GetColumnBytes(1);
        Utf8String xml;
        if (SUCCESS != SchemaSyncImportLog::Decompress(blob, (uint32_t)blobSize, (uint32_t)stmt.GetValueInt64(0), xml)) {
            m_conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0742,
                "Failed to read the schema xml of import %" PRId64 " from the schema sync db.", importId);
            return Status::ERROR_IMPORT_LOG;
        }
        schemaXml.push_back(xml);
    }

    if (schemaXml.empty()) {
        m_conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0743,
            "Import %" PRId64 " does not exist in the schema sync db or holds no schemas.", importId);
        return Status::ERROR_IMPORT_LOG;
    }
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::RecordImport(SyncDbUri const& syncDbUri, bvector<ECN::ECSchemaCP> const& changedSchemas, Utf8StringCR description, int64_t& importId) {
    importId = 0;
    if (changedSchemas.empty())
        return Status::OK;

    Db syncDb;
    if (OpenSyncDbForImportLog(syncDb, syncDbUri, true) != BE_SQLITE_OK)
        return Status::ERROR_OPENING_SCHEMA_SYNC_DB;

    if (SchemaSyncImportLog::EnsureTables(syncDb) != BE_SQLITE_OK)
        return Status::ERROR_IMPORT_LOG;

    // Serialize first, so a schema we cannot write out does not leave a half written record behind.
    // Write each schema in the EC xml version it came from: ec_Schema stores OriginalECXmlVersion, and
    // an import may not decrease it. Serializing everything as Latest would give a replaying briefcase
    // a different OriginalECXmlVersion than the briefcase that recorded the import. Same reasoning as
    // ECSchema::ComputeCheckSum.
    bvector<Utf8String> xmls;
    for (auto schema : changedSchemas) {
        ECN::ECVersion xmlVersion;
        if (ECN::ECObjectsStatus::Success != ECN::ECSchema::CreateECVersion(xmlVersion, schema->GetOriginalECXmlVersionMajor(), schema->GetOriginalECXmlVersionMinor()))
            xmlVersion = ECN::ECVersion::V3_1;

        Utf8String xml;
        if (SchemaWriteStatus::Success != schema->WriteToXmlString(xml, xmlVersion)) {
            m_conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0744,
                "Failed to serialize ECSchema '%s' for the schema sync import log.", schema->GetFullSchemaName().c_str());
            return Status::ERROR_IMPORT_LOG;
        }

        // A schema with classes that serializes without any of them means we were handed an ECDb backed
        // object whose content had already been released. Replaying that would quietly produce an empty
        // schema, which is far worse than refusing the import here.
        if (schema->GetClassCount() > 0 && xml.find("Class typeName=") == Utf8String::npos) {
            m_conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0744,
                "ECSchema '%s' has %d classes but serialized without any of them for the schema sync import log.",
                schema->GetFullSchemaName().c_str(), (int)schema->GetClassCount());
            return Status::ERROR_IMPORT_LOG;
        }
        xmls.push_back(xml);
    }

    Statement importStmt;
    const auto importSql = Utf8PrintfString(
        "INSERT INTO [%s]([Guid],[UserName],[Timestamp],[State],[Description]) VALUES(?,?,?,?,?)", SchemaSyncImportLog::TABLE_IMPORT);
    if (importStmt.Prepare(syncDb, importSql.c_str()) != BE_SQLITE_OK)
        return Status::ERROR_IMPORT_LOG;

    const auto guid = BeGuid(true).ToString();
    importStmt.BindText(1, guid, Statement::MakeCopy::Yes);
    importStmt.BindText(2, m_importUser, Statement::MakeCopy::Yes);
    importStmt.BindInt64(3, (int64_t)BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    importStmt.BindInt(4, (int)ImportState::Pending);
    importStmt.BindText(5, description, Statement::MakeCopy::Yes);
    if (importStmt.Step() != BE_SQLITE_DONE) {
        LOG.errorv("SchemaSync::RecordImport(): Failed to insert import record. %s", syncDb.GetLastError().c_str());
        syncDb.AbandonChanges();
        return Status::ERROR_IMPORT_LOG;
    }
    importStmt.Finalize();
    importId = syncDb.GetLastInsertRowId();

    Statement schemaStmt;
    const auto schemaSql = Utf8PrintfString(
        "INSERT INTO [%s]([ImportId],[Ordinal],[Name],[VersionRead],[VersionWrite],[VersionMinor],[IsDynamic],[XmlSize],[Xml]) VALUES(?,?,?,?,?,?,?,?,?)",
        SchemaSyncImportLog::TABLE_IMPORT_SCHEMA);
    if (schemaStmt.Prepare(syncDb, schemaSql.c_str()) != BE_SQLITE_OK) {
        syncDb.AbandonChanges();
        return Status::ERROR_IMPORT_LOG;
    }

    for (size_t i = 0; i < changedSchemas.size(); ++i) {
        auto schema = changedSchemas[i];
        ByteStream compressed;
        SchemaSyncImportLog::Compress(xmls[i], compressed);

        schemaStmt.Reset();
        schemaStmt.ClearBindings();
        schemaStmt.BindInt64(1, importId);
        schemaStmt.BindInt(2, (int)i);
        schemaStmt.BindText(3, schema->GetName(), Statement::MakeCopy::Yes);
        schemaStmt.BindInt(4, (int)schema->GetVersionRead());
        schemaStmt.BindInt(5, (int)schema->GetVersionWrite());
        schemaStmt.BindInt(6, (int)schema->GetVersionMinor());
        schemaStmt.BindBoolean(7, schema->IsDynamicSchema());
        schemaStmt.BindInt64(8, (int64_t)xmls[i].size());
        schemaStmt.BindBlob(9, compressed.GetData(), (int)compressed.GetSize(), Statement::MakeCopy::Yes);
        if (schemaStmt.Step() != BE_SQLITE_DONE) {
            LOG.errorv("SchemaSync::RecordImport(): Failed to insert schema xml. %s", syncDb.GetLastError().c_str());
            syncDb.AbandonChanges();
            importId = 0;
            return Status::ERROR_IMPORT_LOG;
        }

        LOG.infov("SchemaSync orchestration: import %" PRId64 " schema %d/%d '%s'%s, %d bytes of xml (%d compressed).",
            importId, (int)i + 1, (int)changedSchemas.size(), schema->GetFullSchemaName().c_str(),
            schema->IsDynamicSchema() ? " (dynamic)" : "", (int)xmls[i].size(), (int)compressed.GetSize());
    }
    schemaStmt.Finalize();

    if (syncDb.SaveChanges() != BE_SQLITE_OK) {
        importId = 0;
        return Status::ERROR_IMPORT_LOG;
    }

    LOG.infov("SchemaSync orchestration: recorded import %" PRId64 " by '%s' with %d schema(s).",
        importId, m_importUser.c_str(), (int)changedSchemas.size());
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::RejectImports(SyncDbUri const& syncDbUri, bvector<int64_t> const& importIds, Utf8StringCR rejectedBy, Utf8StringCR reason) {
    if (importIds.empty())
        return Status::OK;

    Db syncDb;
    if (OpenSyncDbForImportLog(syncDb, syncDbUri, true) != BE_SQLITE_OK)
        return Status::ERROR_OPENING_SCHEMA_SYNC_DB;

    if (!syncDb.TableExists(SchemaSyncImportLog::TABLE_IMPORT))
        return Status::ERROR_IMPORT_LOG;

    Statement stmt;
    const auto sql = Utf8PrintfString(
        "UPDATE [%s] SET [State]=?,[RejectedBy]=?,[RejectReason]=? WHERE [Id]=? AND [State]=?", SchemaSyncImportLog::TABLE_IMPORT);
    if (stmt.Prepare(syncDb, sql.c_str()) != BE_SQLITE_OK)
        return Status::ERROR_IMPORT_LOG;

    for (auto id : importIds) {
        stmt.Reset();
        stmt.ClearBindings();
        stmt.BindInt(1, (int)ImportState::Rejected);
        stmt.BindText(2, rejectedBy, Statement::MakeCopy::Yes);
        stmt.BindText(3, reason, Statement::MakeCopy::Yes);
        stmt.BindInt64(4, id);
        stmt.BindInt(5, (int)ImportState::Pending);
        if (stmt.Step() != BE_SQLITE_DONE) {
            LOG.errorv("SchemaSync::RejectImports(): Failed to reject import %" PRId64 ". %s", id, syncDb.GetLastError().c_str());
            syncDb.AbandonChanges();
            return Status::ERROR_IMPORT_LOG;
        }
    }
    stmt.Finalize();

    if (syncDb.SaveChanges() != BE_SQLITE_OK)
        return Status::ERROR_IMPORT_LOG;

    LOG.infov("SchemaSync orchestration: '%s' rejected %d import(s). Reason: %s", rejectedBy.c_str(), (int)importIds.size(), reason.c_str());
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int64_t SchemaSync::GetLastSeenImportId() const {
    uint64_t value = 0;
    if (BE_SQLITE_ROW != m_conn.QueryBriefcaseLocalValue(value, SchemaSyncImportLog::BLV_LAST_SEEN_IMPORT_ID))
        return 0;
    return (int64_t)value;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::SetLastSeenImportId(int64_t importId) {
    if (importId <= GetLastSeenImportId())
        return Status::OK;

    // SaveBriefcaseLocalValue steps an insert, so success is BE_SQLITE_DONE.
    const auto rc = m_conn.SaveBriefcaseLocalValue(SchemaSyncImportLog::BLV_LAST_SEEN_IMPORT_ID, (uint64_t)importId);
    if (rc != BE_SQLITE_DONE && rc != BE_SQLITE_OK) {
        LOG.errorv("SchemaSync::SetLastSeenImportId(): Failed to save the last seen import id. %s", BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_IMPORT_LOG;
    }
    return Status::OK;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
