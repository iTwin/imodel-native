/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
// flatbuffers is consumed as a headers-only VendorAPI (util.cpp is never compiled/linked).
// On MSVC, FLATBUFFERS_LOCALE_INDEPENDENT defaults to 1, which makes flexbuffers.h reference
// flatbuffers::ClassicLocale::instance_ (defined only in util.cpp), causing an unresolved
// external symbol at link time. Forcing it to 0 keeps flexbuffers.h fully header-only.
#define FLATBUFFERS_LOCALE_INDEPENDENT 0
#include <flatbuffers/flexbuffers.h>

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
            "Sync db (%p) already initialized. %s", (void *)this, doc.Stringify().c_str());
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

    // Seed the reservation stores from the local db baseline into the sync db.
    // This captures the container baseline exactly once at Init time so that
    // ReserveSchemaImport never needs to re-seed counters from a briefcase's
    // divergent local db.
    {
        const auto seedResult = SeedReservationStoreInternal(syncDbUri);
        if (seedResult != Status::OK)
            return seedResult;
    } // Ultimately only this part will remain and all the rest of the stuff before will be removed once we have this new technique in place.

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
SchemaSync::Status SchemaSync::SeedReservationStoreInternal(SyncDbUri const& syncDbUri) {
    Db::OpenParams openParams(Db::OpenMode::ReadWrite);
    ParseQueryParams(openParams, syncDbUri);
    if (BE_SQLITE_OK != m_pendingReservationDb.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams)) {
        LOG.errorv("SchemaSync::SeedReservationStoreInternal: Failed to open sync db at '%s'.", syncDbUri.GetUri().c_str());
        return Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB;
    }

    if (BE_SQLITE_OK != m_pendingReservationDb.ExecuteSql(SchemaReservationHelper::RESERVATION_TABLE_DDL)) {
        LOG.error("SchemaSync::SeedReservationStoreInternal: Failed to create reservation id table.");
        AbandonPendingReservation();
        return Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB;
    }
    if (BE_SQLITE_OK != m_pendingReservationDb.ExecuteSql(SchemaReservationHelper::RESERVATION_COLUMNS_TABLE_DDL)) {
        LOG.error("SchemaSync::SeedReservationStoreInternal: Failed to create reservation columns table.");
        AbandonPendingReservation();
        return Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB;
    }

    SchemaReservationStore resStore;
    SchemaReservationColumnStore colStore;
    if (SUCCESS != SchemaReservationHelper::SeedReservationStoreFromLocalDb(m_conn, resStore)) {
        LOG.error("SchemaSync::SeedReservationStoreInternal: Failed to seed reservation store from local db.");
        AbandonPendingReservation();
        return Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB;
    }
    if (SUCCESS != SchemaReservationHelper::SeedColumnStoreFromLocalDb(m_conn, resStore, colStore)) {
        LOG.error("SchemaSync::SeedReservationStoreInternal: Failed to seed column reservation store from local db.");
        AbandonPendingReservation();
        return Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB;
    }
    if (SUCCESS != SchemaReservationHelper::WriteReservationStoreToSyncDb(m_pendingReservationDb, resStore)) {
        LOG.error("SchemaSync::SeedReservationStoreInternal: Failed to write reservation store to sync db.");
        AbandonPendingReservation();
        return Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB;
    }
    if (SUCCESS != SchemaReservationHelper::WriteColumnStoreToSyncDb(m_pendingReservationDb, colStore)) {
        LOG.error("SchemaSync::SeedReservationStoreInternal: Failed to write column reservation store to sync db.");
        AbandonPendingReservation();
        return Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB;
    }
    if (Status::OK != CommitPendingReservation()) {
        LOG.error("SchemaSync::SeedReservationStoreInternal: Failed to commit reservation to sync db.");
        return Status::ERROR_FAIL_TO_INIT_SCHEMA_SYNC_DB;
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

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::ReadTableStore(Db& syncDb, Utf8CP tableName, SchemaReservationTableStore& store) {
    store.Clear();

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(syncDb,
            "SELECT [LastReservedId],[KeyMap] FROM [schema_reservation_ids] WHERE [TableName]=?"))
        return ERROR;
    if (BE_SQLITE_OK != stmt.BindText(1, tableName, Statement::MakeCopy::No))
        return ERROR;
    if (stmt.Step() != BE_SQLITE_ROW)
        return SUCCESS;

    store.SetLastReservedId((uint64_t) stmt.GetValueInt64(0));

    const void* blobData = stmt.GetValueBlob(1);
    int blobSize = stmt.GetColumnBytes(1);
    if (blobData == nullptr || blobSize <= 0)
        return SUCCESS;

    auto root = flexbuffers::GetRoot(reinterpret_cast<const uint8_t*>(blobData), (size_t) blobSize);
    if (!root.IsMap()) {
        LOG.errorv("SchemaReservationHelper::ReadTableStore(): KeyMap blob for table '%s' is not a map.", tableName);
        return ERROR;
    }

    auto map  = root.AsMap();
    auto keys = map.Keys();
    for (size_t i = 0; i < keys.size(); ++i) {
        Utf8CP key = keys[i].AsKey();
        if (key == nullptr)
            continue;
        store.AddEntry(key, (uint64_t) map[key].AsUInt64());
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::WriteTableStore(Db& syncDb, Utf8CP tableName, SchemaReservationTableStore const& store) {
    flexbuffers::Builder fbb;
    fbb.Map([&]() {
        for (auto const& kv : store.GetKeyMap())
            fbb.UInt(kv.first.c_str(), kv.second);
    });
    fbb.Finish();
    auto const& buf = fbb.GetBuffer();

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(syncDb,
            "INSERT OR REPLACE INTO [schema_reservation_ids] "
            "([TableName],[LastReservedId],[KeyMap]) VALUES(?,?,?)"))
        return ERROR;
    if (BE_SQLITE_OK != stmt.BindText(1, tableName, Statement::MakeCopy::No))
        return ERROR;
    if (BE_SQLITE_OK != stmt.BindInt64(2, (int64_t) store.GetLastReservedId()))
        return ERROR;
    if (BE_SQLITE_OK != stmt.BindBlob(3, buf.data(), (int) buf.size(), Statement::MakeCopy::No))
        return ERROR;
    return stmt.Step() == BE_SQLITE_DONE ? SUCCESS : ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::SeedLastReservedIdsFromLocalDb(ECDbCR localDb, SchemaReservationStore& store) {
    auto seedOne = [&localDb](SchemaReservationTableStore& ts, Utf8CP tableName) -> bool {
        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(localDb,
                SqlPrintfString("SELECT COALESCE(MAX(Id),0) FROM [main].[%s]", tableName).GetUtf8CP()))
            return false;
        if (stmt.Step() == BE_SQLITE_ROW)
            ts.SeedLastReservedId((uint64_t) stmt.GetValueInt64(0));
        return true;
    };

    if (!seedOne(store.schema,                     RES_TABLE_SCHEMA))          return ERROR;
    if (!seedOne(store.schemaReference,            RES_TABLE_SCHEMAREF))       return ERROR;
    if (!seedOne(store.ecClass,                    RES_TABLE_CLASS))           return ERROR;
    if (!seedOne(store.classHasBaseClasses,        RES_TABLE_CLASSBASES))      return ERROR;
    if (!seedOne(store.property,                   RES_TABLE_PROPERTY))        return ERROR;
    if (!seedOne(store.enumeration,                RES_TABLE_ENUM))            return ERROR;
    if (!seedOne(store.kindOfQuantity,             RES_TABLE_KOQ))             return ERROR;
    if (!seedOne(store.unitSystem,                 RES_TABLE_UNITSYSTEM))      return ERROR;
    if (!seedOne(store.phenomenon,                 RES_TABLE_PHENOMENON))      return ERROR;
    if (!seedOne(store.unit,                       RES_TABLE_UNIT))            return ERROR;
    if (!seedOne(store.format,                     RES_TABLE_FORMAT))          return ERROR;
    if (!seedOne(store.formatCompositeUnit,        RES_TABLE_FORMATUNIT))      return ERROR;
    if (!seedOne(store.propertyCategory,           RES_TABLE_PROPCAT))         return ERROR;
    if (!seedOne(store.relationshipConstraint,     RES_TABLE_RELCONSTRAINT))   return ERROR;
    if (!seedOne(store.relationshipConstraintClass,RES_TABLE_RELCONSTRCLASS))  return ERROR;
    if (!seedOne(store.customAttribute,            RES_TABLE_CA))              return ERROR;
    if (!seedOne(store.ecTable,                    RES_TABLE_TABLE))           return ERROR;
    if (!seedOne(store.column,                     RES_TABLE_COLUMN))          return ERROR;
    if (!seedOne(store.propertyMap,                RES_TABLE_PROPMAP))         return ERROR;
    if (!seedOne(store.propertyPath,               RES_TABLE_PROPPATH))        return ERROR;
    if (!seedOne(store.ecIndex,                    RES_TABLE_INDEX))           return ERROR;
    if (!seedOne(store.indexColumn,                RES_TABLE_INDEXCOL))        return ERROR;
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::LoadReservationStoreFromSyncDb(Db& syncDb, SchemaReservationStore& store) {
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_SCHEMA,         store.schema))          return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_SCHEMAREF,      store.schemaReference)) return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_CLASS,          store.ecClass))         return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_CLASSBASES,     store.classHasBaseClasses)) return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_PROPERTY,       store.property))        return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_ENUM,           store.enumeration))     return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_KOQ,            store.kindOfQuantity))  return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_UNITSYSTEM,     store.unitSystem))      return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_PHENOMENON,     store.phenomenon))      return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_UNIT,           store.unit))            return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_FORMAT,         store.format))          return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_FORMATUNIT,     store.formatCompositeUnit)) return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_PROPCAT,        store.propertyCategory)) return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_RELCONSTRAINT,  store.relationshipConstraint)) return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_RELCONSTRCLASS, store.relationshipConstraintClass)) return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_CA,             store.customAttribute)) return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_TABLE,          store.ecTable))         return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_COLUMN,         store.column))          return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_PROPMAP,        store.propertyMap))     return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_PROPPATH,       store.propertyPath))    return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_INDEX,          store.ecIndex))         return ERROR;
    if (SUCCESS != ReadTableStore(syncDb, RES_TABLE_INDEXCOL,       store.indexColumn))     return ERROR;
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::WriteReservationStoreToSyncDb(Db& syncDb, SchemaReservationStore const& store) {
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_SCHEMA,         store.schema))          return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_SCHEMAREF,      store.schemaReference)) return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_CLASS,          store.ecClass))         return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_CLASSBASES,     store.classHasBaseClasses)) return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_PROPERTY,       store.property))        return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_ENUM,           store.enumeration))     return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_KOQ,            store.kindOfQuantity))  return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_UNITSYSTEM,     store.unitSystem))      return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_PHENOMENON,     store.phenomenon))      return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_UNIT,           store.unit))            return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_FORMAT,         store.format))          return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_FORMATUNIT,     store.formatCompositeUnit)) return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_PROPCAT,        store.propertyCategory)) return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_RELCONSTRAINT,  store.relationshipConstraint)) return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_RELCONSTRCLASS, store.relationshipConstraintClass)) return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_CA,             store.customAttribute)) return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_TABLE,          store.ecTable))         return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_COLUMN,         store.column))          return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_PROPMAP,        store.propertyMap))     return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_PROPPATH,       store.propertyPath))    return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_INDEX,          store.ecIndex))         return ERROR;
    if (SUCCESS != WriteTableStore(syncDb, RES_TABLE_INDEXCOL,       store.indexColumn))     return ERROR;
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t SchemaReservationHelper::LookupSchemaReferenceId(ECDbCR localDb, Utf8StringCR schemaName, Utf8StringCR refSchemaName) {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(localDb,
            "SELECT sr.[Id] FROM [main].[ec_SchemaReference] sr "
            "JOIN [main].[ec_Schema] s1 ON s1.[Id] = sr.[SchemaId] "
            "JOIN [main].[ec_Schema] s2 ON s2.[Id] = sr.[ReferencedSchemaId] "
            "WHERE s1.[Name] = ? AND s2.[Name] = ?"))
        return 0;
    stmt.BindText(1, schemaName.c_str(), Statement::MakeCopy::No);
    stmt.BindText(2, refSchemaName.c_str(), Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW ? (uint64_t)stmt.GetValueInt64(0) : 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t SchemaReservationHelper::LookupClassHasBaseClassesId(ECDbCR localDb, ECN::ECClassCR ecClass, ECN::ECClassCR baseClass) {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(localDb,
            "SELECT chbc.[Id] FROM [main].[ec_ClassHasBaseClasses] chbc "
            "JOIN [main].[ec_Class] c1 ON c1.[Id] = chbc.[ClassId] "
            "JOIN [main].[ec_Schema] s1 ON s1.[Id] = c1.[SchemaId] "
            "JOIN [main].[ec_Class] c2 ON c2.[Id] = chbc.[BaseClassId] "
            "JOIN [main].[ec_Schema] s2 ON s2.[Id] = c2.[SchemaId] "
            "WHERE s1.[Name] = ? AND c1.[Name] = ? AND s2.[Name] = ? AND c2.[Name] = ?"))
        return 0;
    stmt.BindText(1, ecClass.GetSchema().GetName().c_str(), Statement::MakeCopy::No);
    stmt.BindText(2, ecClass.GetName().c_str(), Statement::MakeCopy::No);
    stmt.BindText(3, baseClass.GetSchema().GetName().c_str(), Statement::MakeCopy::No);
    stmt.BindText(4, baseClass.GetName().c_str(), Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW ? (uint64_t)stmt.GetValueInt64(0) : 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t SchemaReservationHelper::LookupFormatCompositeUnitId(ECDbCR localDb, ECN::ECFormatCR fmt, int ordinal) {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(localDb,
            "SELECT fcu.[Id] FROM [main].[ec_FormatCompositeUnit] fcu "
            "JOIN [main].[ec_Format] f ON f.[Id] = fcu.[FormatId] "
            "JOIN [main].[ec_Schema] s ON s.[Id] = f.[SchemaId] "
            "WHERE s.[Name] = ? AND f.[Name] = ? AND fcu.[Ordinal] = ?"))
        return 0;
    stmt.BindText(1, fmt.GetSchema().GetName().c_str(), Statement::MakeCopy::No);
    stmt.BindText(2, fmt.GetName().c_str(), Statement::MakeCopy::No);
    stmt.BindInt(3, ordinal);
    return stmt.Step() == BE_SQLITE_ROW ? (uint64_t)stmt.GetValueInt64(0) : 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t SchemaReservationHelper::LookupRelConstraintId(ECDbCR localDb, ECN::ECRelationshipClassCR relClass, ECN::ECRelationshipEnd end) {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(localDb,
            "SELECT rc.[Id] FROM [main].[ec_RelationshipConstraint] rc "
            "JOIN [main].[ec_Class] c ON c.[Id] = rc.[RelationshipClassId] "
            "JOIN [main].[ec_Schema] s ON s.[Id] = c.[SchemaId] "
            "WHERE s.[Name] = ? AND c.[Name] = ? AND rc.[RelationshipEnd] = ?"))
        return 0;
    stmt.BindText(1, relClass.GetSchema().GetName().c_str(), Statement::MakeCopy::No);
    stmt.BindText(2, relClass.GetName().c_str(), Statement::MakeCopy::No);
    stmt.BindInt(3, (int)end);
    return stmt.Step() == BE_SQLITE_ROW ? (uint64_t)stmt.GetValueInt64(0) : 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t SchemaReservationHelper::LookupRelConstraintClassId(ECDbCR localDb, ECN::ECRelationshipClassCR relClass, ECN::ECRelationshipEnd end, ECN::ECClassCR constraintClass) {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(localDb,
            "SELECT rcc.[Id] FROM [main].[ec_RelationshipConstraintClass] rcc "
            "JOIN [main].[ec_RelationshipConstraint] rc ON rc.[Id] = rcc.[ConstraintId] "
            "JOIN [main].[ec_Class] relc ON relc.[Id] = rc.[RelationshipClassId] "
            "JOIN [main].[ec_Schema] rels ON rels.[Id] = relc.[SchemaId] "
            "JOIN [main].[ec_Class] cc ON cc.[Id] = rcc.[ClassId] "
            "JOIN [main].[ec_Schema] ccs ON ccs.[Id] = cc.[SchemaId] "
            "WHERE rels.[Name] = ? AND relc.[Name] = ? AND rc.[RelationshipEnd] = ? AND ccs.[Name] = ? AND cc.[Name] = ?"))
        return 0;
    stmt.BindText(1, relClass.GetSchema().GetName().c_str(), Statement::MakeCopy::No);
    stmt.BindText(2, relClass.GetName().c_str(), Statement::MakeCopy::No);
    stmt.BindInt(3, (int)end);
    stmt.BindText(4, constraintClass.GetSchema().GetName().c_str(), Statement::MakeCopy::No);
    stmt.BindText(5, constraintClass.GetName().c_str(), Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW ? (uint64_t)stmt.GetValueInt64(0) : 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t SchemaReservationHelper::LookupCustomAttributeId(ECDbCR localDb, uint64_t containerId, int containerType, ECN::ECClassCR caClass) {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(localDb,
            "SELECT ca.[Id] FROM [main].[ec_CustomAttribute] ca "
            "JOIN [main].[ec_Class] cc ON cc.[Id] = ca.[ClassId] "
            "JOIN [main].[ec_Schema] cs ON cs.[Id] = cc.[SchemaId] "
            "WHERE ca.[ContainerId] = ? AND ca.[ContainerType] = ? AND cs.[Name] = ? AND cc.[Name] = ?"))
        return 0;
    stmt.BindInt64(1, (int64_t)containerId);
    stmt.BindInt(2, containerType);
    stmt.BindText(3, caClass.GetSchema().GetName().c_str(), Statement::MakeCopy::No);
    stmt.BindText(4, caClass.GetName().c_str(), Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW ? (uint64_t)stmt.GetValueInt64(0) : 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::SeedSchemaFromLocalDb(ECDbCR localDb, ECN::ECSchemaCR schema,
                                                              SchemaReservationStore& store,
                                                              bset<Utf8String, CompareIUtf8Ascii>& visited) {
    if (visited.find(schema.GetName()) != visited.end())
        return SUCCESS;
    visited.insert(schema.GetName());

    // ec_Schema
    if (schema.HasId())
        store.schema.AddEntry(SchemaWriter::DeriveSchemaKey(schema), schema.GetId().GetValue());

    // ec_SchemaReference — recurse into referenced schemas
    for (auto const& refPair : schema.GetReferencedSchemas()) {
        ECN::ECSchemaCP ref = refPair.second.get();
        if (ref == nullptr) continue;
        uint64_t srId = LookupSchemaReferenceId(localDb, schema.GetName(), ref->GetName());
        if (srId != 0)
            store.schemaReference.AddEntry(SchemaWriter::DeriveSchemaReferenceKey(schema, *ref), srId);
        if (SUCCESS != SeedSchemaFromLocalDb(localDb, *ref, store, visited))
            return ERROR;
    }

    // Classes
    for (ECN::ECClassCP ecClass : schema.GetClasses()) {
        if (ecClass == nullptr) continue;

        // ec_Class
        if (ecClass->HasId())
            store.ecClass.AddEntry(SchemaWriter::DeriveClassKey(*ecClass), ecClass->GetId().GetValue());

        // ec_ClassHasBaseClasses
        for (ECN::ECClassCP base : ecClass->GetBaseClasses()) {
            if (base == nullptr) continue;
            uint64_t chbcId = LookupClassHasBaseClassesId(localDb, *ecClass, *base);
            if (chbcId != 0)
                store.classHasBaseClasses.AddEntry(SchemaWriter::DeriveClassHasBaseClassesKey(*ecClass, *base), chbcId);
        }

        // ec_Property (owned properties only)
        for (ECN::ECPropertyCP prop : ecClass->GetProperties(false)) {
            if (prop == nullptr || !prop->HasId()) continue;
            store.property.AddEntry(SchemaWriter::DerivePropertyKey(*prop), prop->GetId().GetValue());
        }

        // ec_RelationshipConstraint + ec_RelationshipConstraintClass + constraint CAs
        ECN::ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();
        if (relClass != nullptr) {
            for (auto end : { ECRelationshipEnd_Source, ECRelationshipEnd_Target }) {
                ECN::ECRelationshipConstraintCR constraint = (end == ECRelationshipEnd_Source)
                    ? relClass->GetSource() : relClass->GetTarget();

                uint64_t rcId = LookupRelConstraintId(localDb, *relClass, end);
                if (rcId != 0)
                    store.relationshipConstraint.AddEntry(SchemaWriter::DeriveRelationshipConstraintKey(*relClass, end), rcId);

                for (ECN::ECClassCP cc : constraint.GetConstraintClasses()) {
                    if (cc == nullptr) continue;
                    uint64_t rccId = LookupRelConstraintClassId(localDb, *relClass, end, *cc);
                    if (rccId != 0)
                        store.relationshipConstraintClass.AddEntry(
                            SchemaWriter::DeriveRelationshipConstraintClassKey(*relClass, end, *cc), rccId);
                }

                if (rcId != 0) {
                    int containerType = (end == ECRelationshipEnd_Source)
                        ? (int)SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint
                        : (int)SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint;
                    Utf8String ck = SchemaWriter::DeriveRelationshipConstraintKey(*relClass, end);
                    for (IECInstancePtr ca : constraint.GetCustomAttributes(false)) {
                        if (!ca.IsValid()) continue;
                        uint64_t caId = LookupCustomAttributeId(localDb, rcId, containerType, ca->GetClass());
                        if (caId != 0)
                            store.customAttribute.AddEntry(SchemaWriter::DeriveCustomAttributeKey(ck, ca->GetClass()), caId);
                    }
                }
            }
        }

        // Class-level CAs
        if (ecClass->HasId()) {
            int containerType = (int)SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class;
            Utf8String classKey = SchemaWriter::DeriveClassKey(*ecClass);
            for (IECInstancePtr ca : ecClass->GetCustomAttributes(false)) {
                if (!ca.IsValid()) continue;
                uint64_t caId = LookupCustomAttributeId(localDb, ecClass->GetId().GetValue(), containerType, ca->GetClass());
                if (caId != 0)
                    store.customAttribute.AddEntry(SchemaWriter::DeriveCustomAttributeKey(classKey, ca->GetClass()), caId);
            }
        }

        // Property-level CAs
        for (ECN::ECPropertyCP prop : ecClass->GetProperties(false)) {
            if (prop == nullptr || !prop->HasId()) continue;
            int containerType = (int)SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property;
            Utf8String pk = SchemaWriter::DerivePropertyKey(*prop);
            for (IECInstancePtr ca : prop->GetCustomAttributes(false)) {
                if (!ca.IsValid()) continue;
                uint64_t caId = LookupCustomAttributeId(localDb, prop->GetId().GetValue(), containerType, ca->GetClass());
                if (caId != 0)
                    store.customAttribute.AddEntry(SchemaWriter::DeriveCustomAttributeKey(pk, ca->GetClass()), caId);
            }
        }
    }

    // ec_Enumeration
    for (ECN::ECEnumerationCP e : schema.GetEnumerations())
        if (e != nullptr && e->HasId())
            store.enumeration.AddEntry(SchemaWriter::DeriveEnumerationKey(*e), e->GetId().GetValue());

    // ec_KindOfQuantity
    for (ECN::KindOfQuantityCP k : schema.GetKindOfQuantities())
        if (k != nullptr && k->HasId())
            store.kindOfQuantity.AddEntry(SchemaWriter::DeriveKindOfQuantityKey(*k), k->GetId().GetValue());

    // ec_UnitSystem
    for (ECN::UnitSystemCP us : schema.GetUnitSystems())
        if (us != nullptr && us->HasId())
            store.unitSystem.AddEntry(SchemaWriter::DeriveUnitSystemKey(*us), us->GetId().GetValue());

    // ec_Phenomenon
    for (ECN::PhenomenonCP ph : schema.GetPhenomena())
        if (ph != nullptr && ph->HasId())
            store.phenomenon.AddEntry(SchemaWriter::DerivePhenomenonKey(*ph), ph->GetId().GetValue());

    // ec_Unit
    for (ECN::ECUnitCP u : schema.GetUnits())
        if (u != nullptr && u->HasId())
            store.unit.AddEntry(SchemaWriter::DeriveUnitKey(*u), u->GetId().GetValue());

    // ec_Format + ec_FormatCompositeUnit
    for (ECN::ECFormatCP fmt : schema.GetFormats()) {
        if (fmt == nullptr) continue;
        if (fmt->HasId())
            store.format.AddEntry(SchemaWriter::DeriveFormatKey(*fmt), fmt->GetId().GetValue());
        if (fmt->HasComposite()) {
            Formatting::CompositeValueSpecCR spec = *fmt->GetCompositeSpec();
            int ord = 0;
            if (spec.HasMajorUnit()) {
                uint64_t fcuId = LookupFormatCompositeUnitId(localDb, *fmt, ord);
                if (fcuId != 0) store.formatCompositeUnit.AddEntry(SchemaWriter::DeriveFormatCompositeUnitKey(*fmt, ord), fcuId);
                ord++;
            }
            if (spec.HasMiddleUnit()) {
                uint64_t fcuId = LookupFormatCompositeUnitId(localDb, *fmt, ord);
                if (fcuId != 0) store.formatCompositeUnit.AddEntry(SchemaWriter::DeriveFormatCompositeUnitKey(*fmt, ord), fcuId);
                ord++;
            }
            if (spec.HasMinorUnit()) {
                uint64_t fcuId = LookupFormatCompositeUnitId(localDb, *fmt, ord);
                if (fcuId != 0) store.formatCompositeUnit.AddEntry(SchemaWriter::DeriveFormatCompositeUnitKey(*fmt, ord), fcuId);
                ord++;
            }
            if (spec.HasSubUnit()) {
                uint64_t fcuId = LookupFormatCompositeUnitId(localDb, *fmt, ord);
                if (fcuId != 0) store.formatCompositeUnit.AddEntry(SchemaWriter::DeriveFormatCompositeUnitKey(*fmt, ord), fcuId);
            }
        }
    }

    // ec_PropertyCategory
    for (ECN::PropertyCategoryCP cat : schema.GetPropertyCategories())
        if (cat != nullptr && cat->HasId())
            store.propertyCategory.AddEntry(SchemaWriter::DerivePropertyCategoryKey(*cat), cat->GetId().GetValue());

    // Schema-level CAs
    if (schema.HasId()) {
        int containerType = (int)SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema;
        Utf8String sk = SchemaWriter::DeriveSchemaKey(schema);
        for (ECN::IECInstancePtr ca : schema.GetCustomAttributes(false)) {
            if (!ca.IsValid()) continue;
            uint64_t caId = LookupCustomAttributeId(localDb, schema.GetId().GetValue(), containerType, ca->GetClass());
            if (caId != 0)
                store.customAttribute.AddEntry(SchemaWriter::DeriveCustomAttributeKey(sk, ca->GetClass()), caId);
        }
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::SeedReservationStoreFromLocalDb(ECDbCR localDb, SchemaReservationStore& store) {
    bvector<ECN::ECSchemaCP> allSchemas = localDb.Schemas().GetSchemas(true);
    bset<Utf8String, CompareIUtf8Ascii> visited;
    for (ECN::ECSchemaCP schema : allSchemas) {
        if (schema != nullptr) {
            if (SUCCESS != SeedSchemaFromLocalDb(localDb, *schema, store, visited))
                return ERROR;
        }
    }
    return SeedLastReservedIdsFromLocalDb(localDb, store);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::SeedColumnKeyMapsFromLocalDb(ECDbCR localDb,
                                                                     SchemaReservationStore& idStore,
                                                                     SchemaReservationColumnStore& colStore) {
    const Utf8CP sql =
        "SELECT s.[Name], cls.[Name], pp.[AccessString], t.[Name], col.[Ordinal], col.[Id] "
        "FROM [main].[ec_PropertyMap] pm "
        "JOIN [main].[ec_PropertyPath] pp ON pp.[Id] = pm.[PropertyPathId] "
        "JOIN [main].[ec_Property] root_prop ON root_prop.[Id] = pp.[RootPropertyId] "
        "JOIN [main].[ec_Class] cls ON cls.[Id] = root_prop.[ClassId] "
        "JOIN [main].[ec_Schema] s ON s.[Id] = cls.[SchemaId] "
        "JOIN [main].[ec_Column] col ON col.[Id] = pm.[ColumnId] "
        "JOIN [main].[ec_Table] t ON t.[Id] = col.[TableId] "
        "WHERE pm.[ClassId] = root_prop.[ClassId] "
        "  AND t.[Type] IN (" SQLVAL_DbTable_Type_Primary "," SQLVAL_DbTable_Type_Overflow ") "
        "  AND root_prop.[Kind] != 4";

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(localDb, sql))
        return ERROR;

    DbResult rc;
    while ((rc = stmt.Step()) == BE_SQLITE_ROW) {
        Utf8CP schemaName   = stmt.GetValueText(0);
        Utf8CP className    = stmt.GetValueText(1);
        Utf8CP accessString = stmt.GetValueText(2);
        Utf8CP tableName    = stmt.GetValueText(3);
        if (schemaName == nullptr || className == nullptr || accessString == nullptr || tableName == nullptr)
            continue;

        uint64_t columnOrd = (uint64_t)stmt.GetValueInt64(4);
        uint64_t columnId  = (uint64_t)stmt.GetValueInt64(5);

        Utf8String columnKey = Utf8PrintfString("%s:%s:%s", schemaName, className, accessString);

        SchemaReservationColumnEntry entry;
        entry.columnOrd = columnOrd;
        entry.columnId  = columnId;
        colStore.GetOrCreate(tableName).AddEntry(columnKey, entry);

        // Also mirror the column id into the id store so both stores stay consistent.
        idStore.column.AddEntry(columnKey, columnId);
    }
    return rc == BE_SQLITE_DONE ? SUCCESS : ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::SeedColumnStoreFromLocalDb(ECDbCR localDb,
                                                                   SchemaReservationStore& idStore,
                                                                   SchemaReservationColumnStore& colStore) {
    // Seed the per-table high-water ordinal baseline (MAX(Ordinal) per physical table).
    if (SUCCESS != SeedLastUsedColumnOrdsFromLocalDb(localDb, colStore))
        return ERROR;
    // Seed the propertyKey → (columnOrd, columnId) key maps.
    return SeedColumnKeyMapsFromLocalDb(localDb, idStore, colStore);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaReservationHelper::WalkSchemaForReservation(ECN::ECSchemaCR schema, SchemaReservationStore& store,
                                                        bset<Utf8String, CompareIUtf8Ascii>& visited) {
if (visited.find(schema.GetName()) != visited.end())
            return;
        visited.insert(schema.GetName());

        store.schema.GetOrAllocate(SchemaWriter::DeriveSchemaKey(schema));

        for (auto const& refPair : schema.GetReferencedSchemas()) {
            ECN::ECSchemaCP ref = refPair.second.get();
            if (ref == nullptr) continue;
            store.schemaReference.GetOrAllocate(SchemaWriter::DeriveSchemaReferenceKey(schema, *ref));
            WalkSchemaForReservation(*ref, store, visited);
        }

        for (ECClassCP ecClass : schema.GetClasses()) {
            if (ecClass == nullptr) continue;
            store.ecClass.GetOrAllocate(SchemaWriter::DeriveClassKey(*ecClass));
            for (ECClassCP base : ecClass->GetBaseClasses())
                if (base != nullptr)
                    store.classHasBaseClasses.GetOrAllocate(SchemaWriter::DeriveClassHasBaseClassesKey(*ecClass, *base));
            for (ECPropertyCP prop : ecClass->GetProperties(false))
            if (prop != nullptr)
                store.property.GetOrAllocate(SchemaWriter::DerivePropertyKey(*prop));

        ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();
        if (relClass != nullptr) {
            for (auto end : { ECRelationshipEnd_Source, ECRelationshipEnd_Target }) {
                ECRelationshipConstraintCR constraint = (end == ECRelationshipEnd_Source)
                    ? relClass->GetSource() : relClass->GetTarget();
                store.relationshipConstraint.GetOrAllocate(SchemaWriter::DeriveRelationshipConstraintKey(*relClass, end));
                for (ECClassCP cc : constraint.GetConstraintClasses()) {
                    if (cc != nullptr)
                        store.relationshipConstraintClass.GetOrAllocate(
                            SchemaWriter::DeriveRelationshipConstraintClassKey(*relClass, end, *cc));
                } 
                Utf8String ck = SchemaWriter::DeriveRelationshipConstraintKey(*relClass, end);
                for (IECInstancePtr ca : constraint.GetCustomAttributes(false))
                    store.customAttribute.GetOrAllocate(SchemaWriter::DeriveCustomAttributeKey(ck, ca->GetClass()));
            }
        }

        Utf8String classKey = SchemaWriter::DeriveClassKey(*ecClass);
        for (IECInstancePtr ca : ecClass->GetCustomAttributes(false))
            store.customAttribute.GetOrAllocate(SchemaWriter::DeriveCustomAttributeKey(classKey, ca->GetClass()));

        for (ECPropertyCP prop : ecClass->GetProperties(false)) {
            if (prop == nullptr) continue;
            Utf8String pk = SchemaWriter::DerivePropertyKey(*prop);
            for (IECInstancePtr ca : prop->GetCustomAttributes(false))
                store.customAttribute.GetOrAllocate(SchemaWriter::DeriveCustomAttributeKey(pk, ca->GetClass()));
        }
    }

    for (ECEnumerationCP e : schema.GetEnumerations())
        if (e != nullptr) store.enumeration.GetOrAllocate(SchemaWriter::DeriveEnumerationKey(*e));
    for (KindOfQuantityCP k : schema.GetKindOfQuantities())
        if (k != nullptr) store.kindOfQuantity.GetOrAllocate(SchemaWriter::DeriveKindOfQuantityKey(*k));
    for (UnitSystemCP us : schema.GetUnitSystems())
        if (us != nullptr) store.unitSystem.GetOrAllocate(SchemaWriter::DeriveUnitSystemKey(*us));
    for (PhenomenonCP ph : schema.GetPhenomena())
        if (ph != nullptr) store.phenomenon.GetOrAllocate(SchemaWriter::DerivePhenomenonKey(*ph));
    for (ECUnitCP u : schema.GetUnits())
        if (u != nullptr) store.unit.GetOrAllocate(SchemaWriter::DeriveUnitKey(*u));

    for (ECFormatCP fmt : schema.GetFormats()) {
        if (fmt == nullptr) continue;
        store.format.GetOrAllocate(SchemaWriter::DeriveFormatKey(*fmt));
        if (fmt->HasComposite()) {
            Formatting::CompositeValueSpecCR spec = *fmt->GetCompositeSpec();
            int ord = 0;
            if (spec.HasMajorUnit())  { store.formatCompositeUnit.GetOrAllocate(SchemaWriter::DeriveFormatCompositeUnitKey(*fmt, ord)); ord++; }
            if (spec.HasMiddleUnit()) { store.formatCompositeUnit.GetOrAllocate(SchemaWriter::DeriveFormatCompositeUnitKey(*fmt, ord)); ord++; }
            if (spec.HasMinorUnit())  { store.formatCompositeUnit.GetOrAllocate(SchemaWriter::DeriveFormatCompositeUnitKey(*fmt, ord)); ord++; }
            if (spec.HasSubUnit())    { store.formatCompositeUnit.GetOrAllocate(SchemaWriter::DeriveFormatCompositeUnitKey(*fmt, ord)); }
        }
    }

    for (PropertyCategoryCP cat : schema.GetPropertyCategories()) {
        if (cat != nullptr) 
            store.propertyCategory.GetOrAllocate(SchemaWriter::DerivePropertyCategoryKey(*cat));
    }
        
    Utf8String sk = SchemaWriter::DeriveSchemaKey(schema);
    for (ECN::IECInstancePtr ca : schema.GetCustomAttributes(false))
        store.customAttribute.GetOrAllocate(SchemaWriter::DeriveCustomAttributeKey(sk, ca->GetClass()));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaSync::LoadReservationStore(SyncDbUri const& syncDbUri, SchemaReservationStore& store) const {
    // Use pending connection if open so uncommitted reservations are visible.
    if (m_pendingReservationDb.IsDbOpen()) {
        if (!m_pendingReservationDb.TableExists("schema_reservation_ids"))
            return ERROR;
        return SchemaReservationHelper::LoadReservationStoreFromSyncDb(const_cast<Db&>(m_pendingReservationDb), store);
    }
    Db syncDb;
    Db::OpenParams openParams(Db::OpenMode::Readonly);
    SchemaSync::ParseQueryParams(openParams, syncDbUri);
    if (BE_SQLITE_OK != syncDb.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams))
        return ERROR;
    if (!syncDb.TableExists("schema_reservation_ids"))
        return ERROR;
    return SchemaReservationHelper::LoadReservationStoreFromSyncDb(syncDb, store);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::ReserveSchemaImport(bvector<ECN::ECSchemaCP> const& schemas, SyncDbUri const& syncDbUri) {
    if (schemas.empty())
        return Status::OK;
    if (syncDbUri.IsEmpty()) {
        LOG.error("ReserveSchemaImport: syncDbUri must not be empty.");
        return Status::ERROR;
    }

    Db::OpenParams openParams(Db::OpenMode::ReadWrite);
    SchemaSync::ParseQueryParams(openParams, syncDbUri);
    if (BE_SQLITE_OK != m_pendingReservationDb.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams)) {
        LOG.errorv("ReserveSchemaImport: Failed to open sync db at '%s'.", syncDbUri.GetUri().c_str());
        return Status::ERROR;
    }

    if (BE_SQLITE_OK != m_pendingReservationDb.ExecuteSql(SchemaReservationHelper::RESERVATION_TABLE_DDL)) {
        LOG.error("ReserveSchemaImport: Failed to create reservation table.");
        AbandonPendingReservation();
        return Status::ERROR;
    }

    SchemaReservationStore store;
    if (SUCCESS != SchemaReservationHelper::LoadReservationStoreFromSyncDb(m_pendingReservationDb, store)) {
        LOG.error("ReserveSchemaImport: Failed to read reservation store.");
        AbandonPendingReservation();
        return Status::ERROR;
    }

    bset<Utf8String, CompareIUtf8Ascii> visited;
    for (ECN::ECSchemaCP schema : schemas)
        if (schema != nullptr)
            SchemaReservationHelper::WalkSchemaForReservation(*schema, store, visited);

    if (SUCCESS != SchemaReservationHelper::WriteReservationStoreToSyncDb(m_pendingReservationDb, store)) {
        LOG.error("ReserveSchemaImport: Failed to write reservation store.");
        AbandonPendingReservation();
        return Status::ERROR;
    }

    if (BE_SQLITE_OK != m_pendingReservationDb.ExecuteSql(SchemaReservationHelper::RESERVATION_COLUMNS_TABLE_DDL)) {
        LOG.error("ReserveSchemaImport: Failed to create column reservation table.");
        AbandonPendingReservation();
        return Status::ERROR;
    }

    SchemaReservationColumnStore colStore;
    if (SUCCESS != SchemaReservationHelper::LoadColumnStoreFromSyncDb(m_pendingReservationDb, colStore)) {
        LOG.error("ReserveSchemaImport: Failed to read column reservation store.");
        AbandonPendingReservation();
        return Status::ERROR;
    }

    // Build a classKey → ECClass index for slot-occupant relatedness checks.
    bmap<Utf8String, ECN::ECClassCP, CompareIUtf8Ascii> classIndex;
    {
        bset<Utf8String, CompareIUtf8Ascii> indexVisited;
        for (ECN::ECSchemaCP schema : schemas)
            if (schema != nullptr)
                SchemaReservationHelper::CollectClassIndex(*schema, classIndex, indexVisited);
    }

    bset<Utf8String, CompareIUtf8Ascii> colVisited;
    for (ECN::ECSchemaCP schema : schemas)
        if (schema != nullptr)
            SchemaReservationHelper::WalkSchemaForColumnReservation(*schema, store, colStore, classIndex, colVisited);

    // Reserve mapping-table ids (ec_Table, ec_PropertyPath, ec_PropertyMap). Runs AFTER the column
    // walk because it reads primary-vs-overflow placement from the column store as the single source
    // of truth. Reserve ec_Table for every physical table the column walk produced (incl. overflow).
    for (auto const& storePair : colStore.GetStores())
        store.ecTable.GetOrAllocate(Utf8String(TABLESPACE_Main) + ":" + storePair.first);

    bset<Utf8String, CompareIUtf8Ascii> mapVisited;
    for (ECN::ECSchemaCP schema : schemas)
        if (schema != nullptr)
            SchemaReservationHelper::WalkSchemaForMappingReservation(*schema, store, colStore, mapVisited);

    if (SUCCESS != SchemaReservationHelper::WriteColumnStoreToSyncDb(m_pendingReservationDb, colStore)) {
        LOG.error("ReserveSchemaImport: Failed to write column reservation store.");
        AbandonPendingReservation();
        return Status::ERROR;
    }

    // Re-persist the id store: the mapping walk populated ecTable/propertyMap/propertyPath after the
    // earlier WriteReservationStoreToSyncDb call, so those rows must be written again.
    if (SUCCESS != SchemaReservationHelper::WriteReservationStoreToSyncDb(m_pendingReservationDb, store)) {
        LOG.error("ReserveSchemaImport: Failed to write mapping reservation store.");
        AbandonPendingReservation();
        return Status::ERROR;
    }

    // NOTE: SaveChanges() is intentionally NOT called here.
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::CommitPendingReservation() {
    if (!m_pendingReservationDb.IsDbOpen())
        return Status::OK;
    const auto rc = m_pendingReservationDb.SaveChanges();
    if (BE_SQLITE_OK != rc) {
        LOG.error("SchemaSync::CommitPendingReservation: Failed to commit reservation transaction.");
        return Status::ERROR;
    }
    m_pendingReservationDb.CloseDb();
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::AbandonPendingReservation() {
    if (!m_pendingReservationDb.IsDbOpen())
        return Status::OK;
    const auto rc = m_pendingReservationDb.AbandonChanges();
    if (BE_SQLITE_OK != rc) {
        LOG.error("SchemaSync::AbandonPendingReservation: Failed to roll back reservation transaction.");
        return Status::ERROR;
    }
    m_pendingReservationDb.CloseDb();
    return Status::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::KeyedModeGuard::KeyedModeGuard(IdFactory& f) : m_factory(&f), m_active(false) {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::KeyedModeGuard::~KeyedModeGuard() {
    if (m_active)
        m_factory->ClearKeyedMode();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::ReservationTxGuard::ReservationTxGuard(SchemaSync& s) : m_sync(s), m_committed(false) {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::ReservationTxGuard::~ReservationTxGuard() {
    if (!m_committed) {
        if (SchemaSync::Status::OK != m_sync.AbandonPendingReservation())
            LOG.error("ReservationTxGuard: Failed to roll back reservation transaction on import failure.");
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSync::Status SchemaSync::ReservationTxGuard::Commit() {
    const auto rc = m_sync.CommitPendingReservation();
    if (SchemaSync::Status::OK == rc)
        m_committed = true;
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::ReadColumnTableStore(Db& syncDb, Utf8CP physicalTableName, SchemaReservationColumnTableStore& store) {
    store.Clear();

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(syncDb,
            "SELECT [KeyMap] FROM [schema_reservation_columns] WHERE [PhysicalTableName]=?"))
        return ERROR;
    if (BE_SQLITE_OK != stmt.BindText(1, physicalTableName, Statement::MakeCopy::No))
        return ERROR;
    if (stmt.Step() != BE_SQLITE_ROW)
        return SUCCESS;

    const void* blobData = stmt.GetValueBlob(0);
    int blobSize = stmt.GetColumnBytes(0);
    if (blobData == nullptr || blobSize <= 0)
        return SUCCESS;

    auto root = flexbuffers::GetRoot(reinterpret_cast<const uint8_t*>(blobData), (size_t) blobSize);
    if (!root.IsMap())
        return SUCCESS;

    auto map  = root.AsMap();
    auto keys = map.Keys();
    for (size_t i = 0; i < keys.size(); ++i) {
        Utf8CP key = keys[i].AsKey();
        if (key == nullptr) continue;
        auto vec = map[key].AsVector();
        if (vec.size() < 2) continue;
        SchemaReservationColumnEntry entry;
        entry.columnOrd = (uint64_t) vec[0].AsUInt64();
        entry.columnId  = (uint64_t) vec[1].AsUInt64();
        store.AddEntry(key, entry);
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::WriteColumnTableStore(Db& syncDb, Utf8CP physicalTableName, SchemaReservationColumnTableStore const& store) {
    flexbuffers::Builder fbb;
    fbb.Map([&]() {
        for (auto const& kv : store.GetKeyMap()) {
            fbb.Vector(kv.first.c_str(), [&]() {
                fbb.UInt(kv.second.columnOrd);
                fbb.UInt(kv.second.columnId);
            });
        }
    });
    fbb.Finish();
    auto const& buf = fbb.GetBuffer();

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(syncDb,
            "INSERT OR REPLACE INTO [schema_reservation_columns] "
            "([PhysicalTableName],[KeyMap]) VALUES(?,?)"))
        return ERROR;
    if (BE_SQLITE_OK != stmt.BindText(1, physicalTableName, Statement::MakeCopy::No))
        return ERROR;
    if (BE_SQLITE_OK != stmt.BindBlob(2, buf.data(), (int) buf.size(), Statement::MakeCopy::No))
        return ERROR;
    return stmt.Step() == BE_SQLITE_DONE ? SUCCESS : ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::SeedLastUsedColumnOrdsFromLocalDb(ECDbCR localDb, SchemaReservationColumnStore& store) {
    Statement stmt;
    const Utf8CP sql =
        "SELECT t.[Name], COALESCE(MAX(c.[Ordinal]), 0) "
        "FROM [main].[ec_Table] t "
        "JOIN [main].[ec_Column] c ON c.[TableId] = t.[Id] "
        "WHERE t.[Type] IN (" SQLVAL_DbTable_Type_Primary "," SQLVAL_DbTable_Type_Overflow ") "
        "  AND c.[ColumnKind] = " SQLVAL_DbColumn_Kind_SharedData " "
        "GROUP BY t.[Name]";
    if (BE_SQLITE_OK != stmt.Prepare(localDb, sql))
        return ERROR;

    DbResult rc;
    while ((rc = stmt.Step()) == BE_SQLITE_ROW) {
        Utf8CP tableName = stmt.GetValueText(0);
        if (tableName == nullptr || tableName[0] == '\0') continue;
        uint64_t maxOrd = (uint64_t) stmt.GetValueInt64(1);
        store.GetOrCreate(tableName).SeedHighWaterOrd(maxOrd);
    }
    return rc == BE_SQLITE_DONE ? SUCCESS : ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::LoadColumnStoreFromSyncDb(Db& syncDb, SchemaReservationColumnStore& store) {
    store.Clear();
    if (!syncDb.TableExists("schema_reservation_columns"))
        return SUCCESS; // table not yet created — nothing to load

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(syncDb,
            "SELECT [PhysicalTableName] FROM [schema_reservation_columns]"))
        return ERROR;

    // Collect table names first, then read each store.
    bvector<Utf8String> physTableNames;
    DbResult rc;
    while ((rc = stmt.Step()) == BE_SQLITE_ROW) {
        Utf8CP name = stmt.GetValueText(0);
        if (name != nullptr && name[0] != '\0')
            physTableNames.push_back(name);
    }
    if (rc != BE_SQLITE_DONE)
        return ERROR;

    for (auto const& name : physTableNames) {
        SchemaReservationColumnTableStore& ts = store.GetOrCreate(name);
        if (SUCCESS != ReadColumnTableStore(syncDb, name.c_str(), ts))
            return ERROR;
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::WriteColumnStoreToSyncDb(Db& syncDb, SchemaReservationColumnStore const& store) {
    for (auto const& kv : store.GetStores()) {
        if (SUCCESS != WriteColumnTableStore(syncDb, kv.first.c_str(), kv.second))
            return ERROR;
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECClassCP SchemaReservationHelper::FindTphAncestor(ECN::ECClassCR ecClass) {
    for (ECN::ECClassCP base : ecClass.GetBaseClasses()) {
        if (base == nullptr) continue;

        ClassMapCustomAttribute ca;
        ECDbMapCustomAttributeHelper::TryGetClassMap(ca, *base);
        if (ca.IsValid()) {
            Nullable<Utf8String> stratStr;
            if (SUCCESS == ca.TryGetMapStrategy(stratStr) && !stratStr.IsNull()) {
                MapStrategy strat;
                if (SUCCESS == MapStrategyExtendedInfo::ParseMapStrategy(strat, stratStr.Value())) {
                    if (strat == MapStrategy::TablePerHierarchy)
                        return base;  // This base IS the TPH root.
                    // OwnTable / NotMapped / ExistingTable: the base defines an independent table;
                    // don't recurse further through this branch.
                    continue;
                }
            }
        }

        // Base has no explicit (or unrecognised) map strategy — it may itself be a TPH subclass.
        // Recurse upward to find a TPH root in the ancestor chain.
        ECN::ECClassCP higher = FindTphAncestor(*base);
        if (higher != nullptr)
            return higher;
    }
    return nullptr;  // No TPH ancestor; the class is its own table root.
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TablePerHierarchyInfo::ShareColumnsMode SchemaReservationHelper::ComputePropagatedShareMode(
    ECN::ECClassCR ecClass, Nullable<uint32_t>& maxBeforeOverflow) {
    using SCMode = TablePerHierarchyInfo::ShareColumnsMode;

    for (ECN::ECClassCP base : ecClass.GetBaseClasses()) {
        if (base == nullptr) continue;
        // Skip bases that define an independent table (OwnTable / NotMapped / ExistingTable).
        ClassMapCustomAttribute baseMapCA;
        ECDbMapCustomAttributeHelper::TryGetClassMap(baseMapCA, *base);
        if (baseMapCA.IsValid()) {
            Nullable<Utf8String> stratStr;
            if (SUCCESS == baseMapCA.TryGetMapStrategy(stratStr) && !stratStr.IsNull()) {
                MapStrategy strat;
                if (SUCCESS == MapStrategyExtendedInfo::ParseMapStrategy(strat, stratStr.Value()))
                    if (strat == MapStrategy::OwnTable || strat == MapStrategy::NotMapped ||
                        strat == MapStrategy::ExistingTable)
                        continue;
            }
        }
        if (ComputePropagatedShareMode(*base, maxBeforeOverflow) != SCMode::No)
            return SCMode::Yes;  // Any non-No from a TPH-chain base → inherited Yes.
    }

    ShareColumnsCustomAttribute shareCA;
    if (!ECDbMapCustomAttributeHelper::TryGetShareColumns(shareCA, ecClass) || !shareCA.IsValid())
        return SCMode::No;

    Nullable<bool> applyToSubclassesOnly;
    shareCA.TryGetApplyToSubclassesOnly(applyToSubclassesOnly);
    shareCA.TryGetMaxSharedColumnsBeforeOverflow(maxBeforeOverflow);
    const bool subOnly = !applyToSubclassesOnly.IsNull() && applyToSubclassesOnly.Value();
    return subOnly ? SCMode::ApplyToSubclassesOnly : SCMode::Yes;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaReservationHelper::ClassUsesSharedColumns(ECN::ECClassCR ecClass, Nullable<uint32_t>& maxBeforeOverflow) {
    return ComputePropagatedShareMode(ecClass, maxBeforeOverflow) == TablePerHierarchyInfo::ShareColumnsMode::Yes;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaReservationHelper::PropertyHasExplicitColumnName(ECN::ECPropertyCR prop) {
    ECN::PrimitiveECPropertyCP primProp = prop.GetAsPrimitiveProperty();
    if (primProp == nullptr) return false;  // Only primitive properties can carry PropertyMap.ColumnName.
    PropertyMapCustomAttribute propMapCA;
    if (!ECDbMapCustomAttributeHelper::TryGetPropertyMap(propMapCA, *primProp))
        return false;
    Nullable<Utf8String> colName;
    if (SUCCESS != propMapCA.TryGetColumnName(colName))
        return false;
    return !colName.IsNull() && !colName.Value().empty();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaReservationHelper::CollectClassIndex(
    ECN::ECSchemaCR schema,
    bmap<Utf8String, ECN::ECClassCP, CompareIUtf8Ascii>& index,
    bset<Utf8String, CompareIUtf8Ascii>& visited)
{
    if (visited.find(schema.GetName()) != visited.end())
        return;
    visited.insert(schema.GetName());

    for (auto const& refPair : schema.GetReferencedSchemas()) {
        ECN::ECSchemaCP ref = refPair.second.get();
        if (ref != nullptr)
            CollectClassIndex(*ref, index, visited);
    }

    for (ECN::ECClassCP ecClass : schema.GetClasses()) {
        if (ecClass == nullptr) continue;
        index[SchemaWriter::DeriveClassKey(*ecClass)] = ecClass;
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaReservationHelper::IsSlotReusableByClass(
    SchemaReservationColumnSlot const& slot, ECN::ECClassCR ecClass,
    bmap<Utf8String, ECN::ECClassCP, CompareIUtf8Ascii> const& classIndex)
{
    for (Utf8StringCR occupantKey : slot.occupants) {
        auto it = classIndex.find(occupantKey);
        if (it == classIndex.end() || it->second == nullptr)
            return false;  // unresolved occupant → conservatively block reuse
        ECN::ECClassCP occ = it->second;
        // occ == ecClass, occ is an ancestor of ecClass, or occ is a descendant of ecClass.
        if (ecClass.Is(occ) || occ->Is(&ecClass))
            return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaReservationHelper::WalkSchemaForColumnReservation(
    ECN::ECSchemaCR schema,
    SchemaReservationStore& idStore,
    SchemaReservationColumnStore& colStore,
    bmap<Utf8String, ECN::ECClassCP, CompareIUtf8Ascii> const& classIndex,
    bset<Utf8String, CompareIUtf8Ascii>& visited)
{
    if (visited.find(schema.GetName()) != visited.end())
        return;
    visited.insert(schema.GetName());

    // Recurse into referenced schemas so dependencies are processed first.
    for (auto const& refPair : schema.GetReferencedSchemas()) {
        ECN::ECSchemaCP ref = refPair.second.get();
        if (ref != nullptr)
            WalkSchemaForColumnReservation(*ref, idStore, colStore, classIndex, visited);
    }

    for (ECN::ECClassCP ecClass : schema.GetClasses()) {
        if (ecClass == nullptr) continue;

        // Skip classes that don't use shared columns.
        if (ecClass->IsRelationshipClass()) continue;
        if (ecClass->IsCustomAttributeClass() || ecClass->IsStructClass()) continue;

        // Skip classes explicitly opted out of mapping.
        {
            ClassMapCustomAttribute classMapCA;
            ECDbMapCustomAttributeHelper::TryGetClassMap(classMapCA, *ecClass);
            if (classMapCA.IsValid()) {
                Nullable<Utf8String> stratStr;
                if (SUCCESS == classMapCA.TryGetMapStrategy(stratStr) && !stratStr.IsNull()) {
                    MapStrategy strat;
                    if (SUCCESS == MapStrategyExtendedInfo::ParseMapStrategy(strat, stratStr.Value())) {
                        if (strat == MapStrategy::NotMapped || strat == MapStrategy::ExistingTable)
                            continue;
                    }
                }
            }
        }

        // Only classes that introduce at least one owned property can require new column slots.
        auto const& ownedProps = ecClass->GetProperties(false);
        if (ownedProps.empty()) continue;

        // Only classes using the shared-column strategy need reservation.
        Nullable<uint32_t> maxBeforeOverflow;
        if (!ClassUsesSharedColumns(*ecClass, maxBeforeOverflow))
            continue;

        // Primary table name is derived from the TPH root (or the class itself if no root).
        ECN::ECClassCP tphAncestor = FindTphAncestor(*ecClass);
        ECN::ECClassCR rootClass = (tphAncestor != nullptr) ? *tphAncestor : *ecClass;
        Utf8String primaryTableName;
        if (SUCCESS != DbMappingManager::Tables::DetermineTableName(primaryTableName, rootClass)) {
            LOG.warningv(
                "WalkSchemaForColumnReservation: could not derive table name for '%s' — skipping.",
                ecClass->GetFullName());
            continue;
        }

        // Overflow table name follows the naming convention; created lazily.
        Utf8String overflowTableName = primaryTableName + "_Overflow";
        SchemaReservationColumnTableStore& primaryStore = colStore.GetOrCreate(primaryTableName);
        SchemaReservationColumnTableStore* overflowStore = nullptr;  // created on first use

        for (ECN::ECPropertyCP prop : ownedProps) {
            if (prop == nullptr) continue;

            // Navigation and explicitly-named properties are deterministic; skip.
            if (prop->GetIsNavigation()) continue;
            if (PropertyHasExplicitColumnName(*prop)) continue;

            // Collect leaf access strings (one per physical column for multi-column properties).
            bvector<Utf8String> leafAccessStrings;
            ClassMapColumnFactory::CollectColumnAccessStrings(*prop, prop->GetName(), leafAccessStrings);
            if (leafAccessStrings.empty()) continue;

            bvector<Utf8String> leafKeys;
            leafKeys.reserve(leafAccessStrings.size());
            for (Utf8StringCR accessString : leafAccessStrings)
                leafKeys.push_back(SchemaWriter::DerivePropertyColumnKey(*ecClass, accessString));

            // Already reserved by a prior walk — skip.
            bool alreadyReserved = false;
            for (Utf8StringCR leafKey : leafKeys) {
                if (primaryStore.Lookup(leafKey) != nullptr) { alreadyReserved = true; break; }
                if (const SchemaReservationColumnTableStore* existOvf = colStore.TryGet(overflowTableName))
                    if (existOvf->Lookup(leafKey) != nullptr) { alreadyReserved = true; break; }
            }
            if (alreadyReserved) continue;

            const size_t columnsRequired = leafKeys.size();

            // Choose primary or overflow using the same overflow-budget logic as the allocator.
            const uint64_t highWater = primaryStore.GetHighWaterOrd();
            const uint32_t availablePhysicalColumns =
                ((highWater + 1) < (uint64_t)ClassMapColumnFactory::kMaxPhysicalColumnsPerTable)
                    ? (uint32_t)((uint64_t)ClassMapColumnFactory::kMaxPhysicalColumnsPerTable - (highWater + 1))
                    : 0;
            const uint32_t sharedColumnCount = (uint32_t)primaryStore.GetSlots().size();
            uint32_t reusableSharedColumnCount = 0;
            for (auto const& slotPair : primaryStore.GetSlots()) {
                if (IsSlotReusableByClass(slotPair.second, *ecClass, classIndex))
                    reusableSharedColumnCount++;
            }

            SchemaReservationColumnTableStore* targetStore = &primaryStore;
            if (ClassMapColumnFactory::EvaluateOverflowFromBudget(
                    (uint32_t)columnsRequired, availablePhysicalColumns, sharedColumnCount,
                    reusableSharedColumnCount, maxBeforeOverflow)) {
                if (overflowStore == nullptr)
                    overflowStore = &colStore.GetOrCreate(overflowTableName);
                targetStore = overflowStore;
            }

            // Reuse an available slot or allocate a new one for each leaf.
            for (Utf8StringCR leafKey : leafKeys) {
                bool reusedSlot = false;
                for (auto const& slotPair : targetStore->GetSlots()) {
                    if (IsSlotReusableByClass(slotPair.second, *ecClass, classIndex)) {
                        SchemaReservationColumnEntry entry;
                        entry.columnOrd = slotPair.second.columnOrd;
                        entry.columnId  = slotPair.second.columnId; // share the existing column id
                        targetStore->AddEntry(leafKey, entry);
                        reusedSlot = true;
                        break;
                    }
                }
                if (reusedSlot)
                    continue;

                SchemaReservationColumnEntry entry;
                entry.columnOrd = targetStore->GetHighWaterOrd() + 1;
                entry.columnId  = idStore.column.GetOrAllocate(leafKey); // new column id
                targetStore->AddEntry(leafKey, entry);
            }
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaReservationHelper::IsClassMappedForReservation(ECN::ECClassCR ecClass)
{
    // Relationship classes (link-table / FK maps) and non-mapped class kinds are out of scope
    // for this walk; their mapping rows are covered elsewhere / listed as residual risk.
    if (ecClass.IsRelationshipClass())
        return false;
    if (ecClass.IsCustomAttributeClass() || ecClass.IsStructClass())
        return false;

    ClassMapCustomAttribute classMapCA;
    ECDbMapCustomAttributeHelper::TryGetClassMap(classMapCA, ecClass);
    if (classMapCA.IsValid()) {
        Nullable<Utf8String> stratStr;
        if (SUCCESS == classMapCA.TryGetMapStrategy(stratStr) && !stratStr.IsNull()) {
            MapStrategy strat;
            if (SUCCESS == MapStrategyExtendedInfo::ParseMapStrategy(strat, stratStr.Value())) {
                if (strat == MapStrategy::NotMapped || strat == MapStrategy::ExistingTable)
                    return false;
            }
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaReservationHelper::DerivePrimaryTableName(ECN::ECClassCR ecClass, Utf8StringR tableName)
{
    // Primary table is the TPH root's table (shared by the whole hierarchy) or the class's own.
    ECN::ECClassCP tphAncestor = FindTphAncestor(ecClass);
    ECN::ECClassCR rootClass = (tphAncestor != nullptr) ? *tphAncestor : ecClass;
    return DbMappingManager::Tables::DetermineTableName(tableName, rootClass);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SchemaReservationHelper::ResolveLeafColumnTableName(
    SchemaReservationColumnStore const& colStore,
    Utf8StringCR leafColumnKey,
    Utf8StringCR primaryTableName)
{
    // The column-reservation walk is the single source of truth for primary-vs-overflow
    // placement of shared columns. If the leaf was reserved there, use the physical table
    // that holds it; otherwise the column is non-shared and lands in the primary table.
    for (auto const& kv : colStore.GetStores()) {
        if (kv.second.Lookup(leafColumnKey) != nullptr)
            return kv.first;
    }
    return primaryTableName;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaReservationHelper::ReserveLeafPropertyReservation(
    SchemaReservationStore& idStore,
    SchemaReservationColumnStore const& colStore,
    ECN::ECClassCR mappedClass,
    ECN::ECClassCR declaringClass,
    ECN::ECPropertyCR rootProperty,
    Utf8StringCR accessString,
    Utf8StringCR primaryTableName)
{
    // ec_PropertyPath is keyed by the DECLARING class of the root property (matches the
    // p.ClassId join on the consume side), so inherited paths reserve a single shared id.
    Utf8String pathKey = declaringClass.GetSchema().GetName();
    pathKey += ":";
    pathKey += declaringClass.GetName();
    pathKey += ":";
    pathKey += rootProperty.GetName();
    pathKey += ":";
    pathKey += accessString;
    idStore.propertyPath.GetOrAllocate(pathKey);

    // Resolve the physical table this leaf lands in from the column store (shared columns) or
    // the class's primary table (non-shared / deterministic columns).
    Utf8String leafColumnKey = SchemaWriter::DerivePropertyColumnKey(declaringClass, accessString);
    Utf8String targetTable = ResolveLeafColumnTableName(colStore, leafColumnKey, primaryTableName);

    // ec_PropertyMap is keyed by the CONCRETE mapped class + access string + placement, so each
    // concrete class (including subclasses reusing inherited/shared columns) reserves its own id.
    Utf8String mapKey = mappedClass.GetSchema().GetName();
    mapKey += ":";
    mapKey += mappedClass.GetName();
    mapKey += ":";
    mapKey += accessString;
    mapKey += ":";
    mapKey += TABLESPACE_Main;
    mapKey += ":";
    mapKey += targetTable;
    idStore.propertyMap.GetOrAllocate(mapKey);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaReservationHelper::WalkSchemaForMappingReservation(
    ECN::ECSchemaCR schema,
    SchemaReservationStore& idStore,
    SchemaReservationColumnStore const& colStore,
    bset<Utf8String, CompareIUtf8Ascii>& visited)
{
    if (visited.find(schema.GetName()) != visited.end())
        return;
    visited.insert(schema.GetName());

    // Recurse into referenced schemas so dependencies are processed first.
    for (auto const& refPair : schema.GetReferencedSchemas()) {
        ECN::ECSchemaCP ref = refPair.second.get();
        if (ref != nullptr)
            WalkSchemaForMappingReservation(*ref, idStore, colStore, visited);
    }

    for (ECN::ECClassCP ecClass : schema.GetClasses()) {
        if (ecClass == nullptr)
            continue;
        if (!IsClassMappedForReservation(*ecClass))
            continue;

        Utf8String primaryTableName;
        if (SUCCESS != DerivePrimaryTableName(*ecClass, primaryTableName)) {
            LOG.warningv(
                "WalkSchemaForMappingReservation: could not derive table name for '%s' — skipping.",
                ecClass->GetFullName());
            continue;
        }

        // Reserve ec_Table for the class's primary table (idempotent across the hierarchy).
        idStore.ecTable.GetOrAllocate(Utf8String(TABLESPACE_Main) + ":" + primaryTableName);

        // Reserve ec_PropertyPath + ec_PropertyMap for the full property set (owned + inherited).
        // Each concrete class gets its own ec_PropertyMap rows; the placement (which physical
        // table each leaf lands in) is read from the column store, never recomputed here.
        for (ECN::ECPropertyCP prop : ecClass->GetProperties(true)) {
            if (prop == nullptr)
                continue;
            ECN::ECClassCR declaringClass = prop->GetClass();

            bvector<Utf8String> leafAccessStrings;
            ClassMapColumnFactory::CollectColumnAccessStrings(*prop, prop->GetName(), leafAccessStrings);
            for (Utf8StringCR accessString : leafAccessStrings)
                ReserveLeafPropertyReservation(idStore, colStore, *ecClass, declaringClass, *prop,
                                               accessString, primaryTableName);
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaSync::LoadColumnStore(SyncDbUri const& syncDbUri, SchemaReservationColumnStore& store) const {
    // Use pending connection if open so uncommitted column reservations are visible.
    if (m_pendingReservationDb.IsDbOpen())
        return SchemaReservationHelper::LoadColumnStoreFromSyncDb(const_cast<Db&>(m_pendingReservationDb), store);
    Db syncDb;
    Db::OpenParams openParams(Db::OpenMode::Readonly);
    SchemaSync::ParseQueryParams(openParams, syncDbUri);
    if (BE_SQLITE_OK != syncDb.OpenBeSQLiteDb(syncDbUri.GetUri().c_str(), openParams))
        return ERROR;
    return SchemaReservationHelper::LoadColumnStoreFromSyncDb(syncDb, store);
}

END_BENTLEY_SQLITE_EC_NAMESPACE
