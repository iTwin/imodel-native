/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSynchronizer::GetColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, std::vector<std::string>& columnNames) {
	Statement stmt;
    const auto sql = std::string{SqlPrintfString("pragma %s.table_info(%s)", dbAlias, tableName).GetUtf8CP()};
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
DbResult SchemaSynchronizer::GetPrimaryKeyColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, std::vector<std::string>& columnNames) {
	Statement stmt;
    const auto sql = std::string{SqlPrintfString("pragma %s.table_info(%s)", dbAlias, tableName).GetUtf8CP()};
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
std::string SchemaSynchronizer::Join(std::vector<std::string> const& list, std::string delimiter, std::string prefix, std::string postfix) {
	return prefix + std::accumulate(
		std::next(list.begin()),
		std::end(list),
		std::string{list.front()},
		[&](std::string const& acc, const std::string& piece) {
			return acc + delimiter + piece;
		}
	) + postfix;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::string SchemaSynchronizer::ToLower(std::string const& val) {
	std::string out;
	std::for_each(val.begin(), val.end(), [&](char const& ch) {
		out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
	});
	return out;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSynchronizer::SyncData(ECDbR conn, std::vector<std::string> const& tables, Utf8CP sourceDbAlias, Utf8CP targetDbAlias) {
	auto rc = conn.ExecuteSql("PRAGMA defer_foreign_keys=1");
	if (rc != BE_SQLITE_OK) {
		return rc;
	}
	for (auto& tbl : tables) {
		rc = SyncData(conn, tbl.c_str(), sourceDbAlias, targetDbAlias);
		if (rc != BE_SQLITE_OK) {
			conn.ExecuteSql("PRAGMA defer_foreign_keys=0");
			return rc;
		}
	}
	return conn.ExecuteSql("PRAGMA defer_foreign_keys=0");
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSynchronizer::SyncData(ECDbR conn, Utf8CP tableName, Utf8CP sourceDbAlias, Utf8CP targetDbAlias) {
	DbResult rc;
	auto sourceCols = std::vector<std::string>{};
	rc = GetColumnNames(conn, sourceDbAlias, tableName, sourceCols);
	if (BE_SQLITE_OK != rc)
		return rc;

	auto sourcePkCols = std::vector<std::string>{};
	rc = GetPrimaryKeyColumnNames(conn, sourceDbAlias, tableName, sourcePkCols);
	if (BE_SQLITE_OK != rc)
		return rc;

	auto targetCols = std::vector<std::string>{};
	rc = GetColumnNames(conn, targetDbAlias, tableName, targetCols);
	if (BE_SQLITE_OK != rc)
		return rc;

	auto targetPkCols = std::vector<std::string>{};
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

	const auto sourceTableSql = std::string {SqlPrintfString("[%s].[%s]", sourceDbAlias, tableName).GetUtf8CP()};
	const auto targetTableSql = std::string {SqlPrintfString("[%s].[%s]", targetDbAlias, tableName).GetUtf8CP()};
	const auto sourceColsSql = Join(sourceCols);
	const auto targetColsSql = Join(targetCols);

	std::vector<std::string> setClauseExprs;
	for(auto& col : targetCols) {
        setClauseExprs.push_back(SqlPrintfString("%s=excluded.%s", col.c_str(), col.c_str()).GetUtf8CP());
    }
	const auto updateColsSql = Join(setClauseExprs);
    const auto targetPkColsSql = Join(targetPkCols);

	if (sourcePkColCount == 1) {
		const auto deleteTargetSql = std::string {
			SqlPrintfString("DELETE FROM %s WHERE [%s] NOT IN (SELECT [%s] FROM %s)",
				targetTableSql.c_str(),
				targetPkCols.front().c_str(),
				sourcePkCols.front().c_str(),
				sourceTableSql.c_str()
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
	} else {
		conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error,
			IssueCategory::SchemaSynchronization,
			IssueType::ECDbIssue,
			"Sync does not support tables with composite primary keys. %s table has composite primary key.", sourceTableSql.c_str());
        return BE_SQLITE_SCHEMA;
    }

    const auto sql = std::string{SqlPrintfString(
		"insert into %s(%s) select %s from %s where 1 on conflict(%s) do update set %s",
		targetTableSql.c_str(),
		targetColsSql.c_str(),
		sourceColsSql.c_str(),
		sourceTableSql.c_str(),
		targetPkColsSql.c_str(),
		updateColsSql.c_str()
	).GetUtf8CP()};

	Statement stmt;
	rc = stmt.Prepare(conn, sql.c_str());
	if (rc != BE_SQLITE_OK) {
		return rc;
	}
    rc = stmt.Step();
    return rc == BE_SQLITE_DONE ? BE_SQLITE_OK : rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSynchronizer::TryGetAttachDbs(AliasMap& aliasMap, ECDbR conn) {
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
DbResult SchemaSynchronizer::SyncData(ECDbR conn, Utf8CP syncDbPath, SyncAction action, std::function<bool(const Utf8String&)> tableFilter, bool useNestedSafePoint) {
	const auto kSyncDbAlias = "sync_db";
	const auto kMainDbAlias = "main";


    BeFileName syncDbFile(syncDbPath);
	if (!syncDbFile.DoesPathExist()) {
		return BE_SQLITE_ERROR;
	}

	if (conn.IsReadonly()) {
		conn.GetImpl().Issues().Report(
			IssueSeverity::Error,
			IssueCategory::SchemaSynchronization,
			IssueType::ECDbIssue,
			"Primary connection is readonly. It must be in read/write mode.");
		return BE_SQLITE_READONLY;
	}

	ECDb syncDb;
	if (action == SyncAction::Pull) {
		// check if sync db can be open in readonly mode and run a query to make sure its valid
        auto rc = syncDb.OpenBeSQLiteDb(syncDbFile, Db::OpenParams(Db::OpenMode::Readonly));
		if (rc != BE_SQLITE_OK) {
	            conn.GetImpl().Issues().ReportV(
				IssueSeverity::Error,
				IssueCategory::SchemaSynchronization,
				IssueType::ECDbIssue,
				"Fail to to open sync-db in readonly mode: %s", syncDbPath);
			return rc;
		}
	} else {
		// check if sync db can be open in read/write mode and run a query to make sure its valid
        auto rc = syncDb.OpenBeSQLiteDb(syncDbFile, Db::OpenParams(Db::OpenMode::ReadWrite));
		if (rc != BE_SQLITE_OK) {
	            conn.GetImpl().Issues().ReportV(
				IssueSeverity::Error,
				IssueCategory::SchemaSynchronization,
				IssueType::ECDbIssue,
				"Fail to to open sync-db in readonly mode: %s", syncDbPath);
			return rc;
		}
	}

	ECSqlStatement stmt;
	auto ecsqlRc = stmt.Prepare(syncDb, "SELECT * FROM ECDbMeta.ECClassDef");
	if (!ecsqlRc.IsSuccess()) {
		conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error,
			IssueCategory::SchemaSynchronization,
			IssueType::ECDbIssue,
			"SyncDb does not seem to be a valid iModel '%s': %s", syncDbPath, conn.GetLastError().c_str());
		return BE_SQLITE_ERROR;
	}
    auto rc = stmt.Step();
	if (rc != BE_SQLITE_ROW && rc != BE_SQLITE_DONE) {
		conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error,
			IssueCategory::SchemaSynchronization,
			IssueType::ECDbIssue,
			"Unable to read from SyncDb '%s': %s", syncDbPath, conn.GetLastError().c_str());
        return rc;
    }
    stmt.Finalize();
    syncDb.CloseDb();

    AliasMap aliasMap;
	rc = TryGetAttachDbs(aliasMap, conn);
	if (rc != BE_SQLITE_OK) {
		conn.GetImpl().Issues().Report(
			IssueSeverity::Error,
			IssueCategory::SchemaSynchronization,
			IssueType::ECDbIssue,
			"Unable to query attach db from primary connection");
        return rc;
	}

	if (aliasMap.find(kMainDbAlias) == aliasMap.end()) {
		conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error,
			IssueCategory::SchemaSynchronization,
			IssueType::ECDbIssue,
			"Expecting '%s' attach db on primary connection", kMainDbAlias);
        return rc;
	}

	if (aliasMap.find(kSyncDbAlias) != aliasMap.end()) {
		conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error,
			IssueCategory::SchemaSynchronization,
			IssueType::ECDbIssue,
			"Db alias '%s' use by sync db is already in use", kSyncDbAlias);
        return rc;
	}

    rc = conn.AttachDb(syncDbPath, kSyncDbAlias);
	if (rc != BE_SQLITE_OK) {
		conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error,
			IssueCategory::SchemaSynchronization,
			IssueType::ECDbIssue,
			"Unable to attach sync db '%s' as '%s' to primary connection: %s", syncDbPath, kSyncDbAlias, conn.GetLastError().c_str());
        return rc;
	}
    auto detachAndReturn = [&](DbResult result) {
        conn.DetachDb(kSyncDbAlias);
		if (result == BE_SQLITE_OK) {
            conn.SaveChanges();
        }
        return result;
    };

    std::unique_ptr<Savepoint> savePoint;
	if (useNestedSafePoint) {
		savePoint = std::make_unique<Savepoint>(conn, "sync-db-conn");
	}

    const auto fromAlias = action == SyncAction::Pull ? kSyncDbAlias : kMainDbAlias;
	const auto toAlias = action == SyncAction::Pull ?  kMainDbAlias: kSyncDbAlias;

    const auto enumTblSql = std::string {
		SqlPrintfString(R"sql(
			SELECT [target].[tbl_name]
			FROM   [%s].[sqlite_master] [target], [%s].[sqlite_master] [source]
			WHERE  [target].[type] = 'table'
					AND [target].[rootpage] != 0
					AND [target].[tbl_name] NOT LIKE 'sqlite_%%'
					AND [source].[type] = 'table'
					AND [source].[rootpage] != 0
					AND [source].[tbl_name] NOT LIKE 'sqlite_%%'
					AND [source].[tbl_name] = [target].[tbl_name]
			ORDER  BY [target].[tbl_name];
	)sql", toAlias , fromAlias).GetUtf8CP()};

    std::vector<std::string> tables;
    Statement iuStmt;
	rc = iuStmt.Prepare(conn, enumTblSql.c_str());
	if (rc != BE_SQLITE_OK) {
		return detachAndReturn(rc);
	}

	while(iuStmt.Step() == BE_SQLITE_ROW) {
		Utf8String tbl = iuStmt.GetValueText(0);
		if (tableFilter(tbl)) {
			tables.push_back(tbl);
		}
	}
	return detachAndReturn(SyncData(conn, tables, fromAlias, toAlias));
}


END_BENTLEY_SQLITE_EC_NAMESPACE