/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

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
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SqlIndexDef final {
	private:
        std::string m_createSql;
        std::string m_name;
	public:
        SqlIndexDef(SqlIndexDef&&) = default;
		SqlIndexDef(SqlIndexDef const&) = default;
		SqlIndexDef& operator = (SqlIndexDef&&) = default;
		SqlIndexDef& operator = (SqlIndexDef const&) = default;
        SqlIndexDef(std::string const& indexName, std::string const& createSql) : m_name(indexName), m_createSql(createSql){}
		DbResult Drop(DbCR db) const {
            return db.ExecuteSql(SqlPrintfString("DROP INDEX %s", m_name.c_str()).GetUtf8CP());
        }
		DbResult Create(DbCR db) const {
			return db.ExecuteSql(m_createSql.c_str());
		}
		static DbResult GetSystemUniqueIndexes(std::vector<SqlIndexDef>& indexes, DbCR db , std::string const& dbAlias = "main") {
            const auto sql = std::string {SqlPrintfString(R"z(
				SELECT
					[name],
					[sql]
				FROM   [%s].[sqlite_master]
				WHERE  [tbl_name] LIKE 'ec\_%%' ESCAPE '\'
						AND [type] = 'index'
						AND [sql] LIKE 'CREATE UNIQUE %%';
			)z", dbAlias.c_str()).GetUtf8CP()};

			Statement stmt;
            indexes.clear();
            auto rc = stmt.Prepare(db, sql.c_str());
			if (rc != BE_SQLITE_OK) {
                printf("%s\n", sql.c_str());
                return rc;
            }
			while(stmt.Step() == BE_SQLITE_ROW) {
                indexes.emplace_back(stmt.GetValueText(0), stmt.GetValueText(1));
            }
            return rc == BE_SQLITE_DONE ? BE_SQLITE_OK : rc;
        }
};

//=======================================================================================
// 	SchemaSynchronizer
//+===============+===============+===============+===============+===============+======
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
	std::vector<std::string> delClauseExprs;
	for(auto& col : targetCols) {
		setClauseExprs.push_back(SqlPrintfString("%s=excluded.%s", col.c_str(), col.c_str()).GetUtf8CP());
    }
	for(auto& col : sourcePkCols) {
  		delClauseExprs.push_back(SqlPrintfString("%s=[source].%s", col.c_str(), col.c_str()).GetUtf8CP());
    }

	const auto updateColsSql = Join(setClauseExprs);
    const auto targetPkColsSql = Join(targetPkCols);
	const auto deleteColsSql = Join(delClauseExprs, " AND ");

    const auto allowDelete = true;
    if (allowDelete) {
		const auto deleteTargetSql = std::string {
			SqlPrintfString("DELETE FROM %s WHERE NOT EXISTS (SELECT 1 FROM %s [source] WHERE %s)",
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
			printf("SQL (%s): %s \n", conn.GetLastError().c_str(), deleteTargetSql.c_str());
            return rc;
        }
        // printf("deleted rows (%s): %d\n", targetTableSql.c_str(), conn.GetModifiedRowCount());
    }

    const auto sql = std::string{SqlPrintfString(
		"insert into %s(%s) select %s from %s where 1 on conflict do update set %s",
		targetTableSql.c_str(),
		targetColsSql.c_str(),
		sourceColsSql.c_str(),
		sourceTableSql.c_str(),
		//targetPkColsSql.c_str(),
		updateColsSql.c_str()
	).GetUtf8CP()};

	Statement stmt;
	rc = stmt.Prepare(conn, sql.c_str());
	if (rc != BE_SQLITE_OK) {
		return rc;
	}
    rc = stmt.Step();
	if (rc != BE_SQLITE_DONE) {
        printf("SQL (%s): %s \n", conn.GetLastError().c_str(), sql.c_str());
    }
	// printf("upsert rows  (%s): %d\n",targetTableSql.c_str(), conn.GetModifiedRowCount());
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
DbResult SchemaSynchronizer::GetECTables(DbR conn, std::vector<std::string>& tables, Utf8CP dbAlias) {
    const auto queryECTableSql = std::string {
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
DbResult SchemaSynchronizer::SyncData(ECDbR conn, Utf8CP syncDbPath, SyncAction action, bool verifySynDb, std::vector<std::string> const& additionTables) {
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
	DbResult rc;
	if (verifySynDb) {
        const auto syncInfo = SchemaSyncInfo::From(syncDb);
		if (syncInfo.IsEmpty()) {
			conn.GetImpl().Issues().ReportV(
				IssueSeverity::Error,
				IssueCategory::SchemaSynchronization,
				IssueType::ECDbIssue,
				"SyncInfo not found. Not valid SyncDb %s. ", syncDbPath);
            return BE_SQLITE_ERROR;
        }
		if (syncInfo.GetSourceProjectGuid() != conn.QueryProjectGuid()) {
			conn.GetImpl().Issues().ReportV(
				IssueSeverity::Error,
				IssueCategory::SchemaSynchronization,
				IssueType::ECDbIssue,
				"SyncInfo project guid does not match. %s <> %s",
				 syncInfo.GetSourceProjectGuid().ToString().c_str(),
				 conn.QueryProjectGuid().ToString().c_str());
			return BE_SQLITE_ERROR;
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
		rc = stmt.Step();
		if (rc != BE_SQLITE_ROW && rc != BE_SQLITE_DONE) {
			conn.GetImpl().Issues().ReportV(
				IssueSeverity::Error,
				IssueCategory::SchemaSynchronization,
				IssueType::ECDbIssue,
				"Unable to read from SyncDb '%s': %s", syncDbPath, conn.GetLastError().c_str());
			return rc;
		}
		stmt.Finalize();
	}

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

    const auto fromAlias = action == SyncAction::Pull ? kSyncDbAlias : kMainDbAlias;
	const auto toAlias = action == SyncAction::Pull ?  kMainDbAlias: kSyncDbAlias;
    auto detachAndReturn = [&](DbResult result) {
		if (result != BE_SQLITE_OK) {
			// rollback before detach
        	conn.AbandonChanges();
        }
        conn.DetachDb(kSyncDbAlias);
        return result;
    };

    std::vector<std::string> tables;
    rc = GetECTables(conn, tables, fromAlias);
    if (rc != BE_SQLITE_OK) {
		return detachAndReturn(rc);
	}

    tables.insert(tables.end(), additionTables.begin(), additionTables.end());
    return detachAndReturn(SyncData(conn, tables, fromAlias, toAlias));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSynchronizer::InitSynDb(ECDbR conn, Utf8CP syncDbUri) {
    Db syncDb;
    auto rc = syncDb.OpenBeSQLiteDb(syncDbUri, Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes));
	if (rc != BE_SQLITE_OK) {
        return rc;
	}
	syncDb.GetStatementCache().Empty();

    auto dropDataTables = [](DbR db) {
		Statement stmt;
        std::vector<std::string> tables;
        auto rc = stmt.Prepare(db, "SELECT [Name] FROM [ec_Table] WHERE [Type] IN (" SQLVAL_DbTable_Type_Primary "," SQLVAL_DbTable_Type_Joined "," SQLVAL_DbTable_Type_Overflow R"x() AND Name NOT LIKE 'ecdbf\_%' ESCAPE '\' ORDER BY [Type] DESC)x");
		if (rc != BE_SQLITE_OK) {
			return rc;
		}
		while(stmt.Step() == BE_SQLITE_ROW) {
            tables.push_back(stmt.GetValueText(0));
        }

        stmt.Finalize();
        for(auto& table : tables) {
			rc = db.ExecuteSql(SqlPrintfString("DROP TABLE IF EXISTS [main].[%s];", table.c_str()).GetUtf8CP());
			if (rc != BE_SQLITE_OK) {
				return rc;
			}
		}
        return BE_SQLITE_OK;
    };
    auto dropMetaTables = [&](DbR db) {
        std::vector<std::string> tables;
        auto rc = GetECTables(db, tables, "main");
		if (rc != BE_SQLITE_OK) {
            return rc;
        }
        std::reverse(tables.begin(), tables.end());
        for(auto& table: tables) {
			rc = db.ExecuteSql(SqlPrintfString("DROP TABLE IF EXISTS [main].[%s];", table.c_str()).GetUtf8CP());
			if (rc != BE_SQLITE_OK) {
				return rc;
			}
		}
        return BE_SQLITE_OK;
    };
    auto createMetaTables = [](ECDbR fromDb, DbR syncDb) {
		const auto sql = std::string {R"z(
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
    };

    rc = dropDataTables(syncDb);
	if (rc != BE_SQLITE_OK) {
		return rc;
	}

    rc = dropMetaTables(syncDb);
	if (rc != BE_SQLITE_OK) {
		return rc;
	}

    rc = createMetaTables(conn, syncDb);
	if (rc != BE_SQLITE_OK) {
		return rc;
	}

	auto syncInfo = SchemaSyncInfo::New();
    syncInfo.SetSource(conn);
    rc = syncInfo.SaveTo(syncDb);
	if (rc != BE_SQLITE_OK) {
		return rc;
	}

    rc = syncDb.SaveChanges();
	if (rc != BE_SQLITE_OK) {
		return rc;
	}

    syncDb.CloseDb();
    return SyncData(conn, syncDbUri, SchemaManager::SyncAction::Push, false, {"be_Prop"});
}

//=======================================================================================
// 	SyncInfo
//+===============+===============+===============+===============+===============+======
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSyncInfo SchemaSyncInfo::New() {
	SchemaSyncInfo info;
	info.m_syncId.Create();
	info.m_created = DateTime::GetCurrentTimeUtc();
	info.m_modified = DateTime::GetCurrentTimeUtc();
	info.m_version = BeInt64Id(1u);
	return info;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaSyncInfo::SetSource(DbCR db) {
	m_sourceProjectGuid = db.QueryProjectGuid();
	m_sourceDbGuid = db.GetDbGuid();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaSyncInfo::SaveTo(DbR db) const {
	BeJsDocument doc;
    const auto propSpec = PropertySpec(JPropertyName, JPropertyNamespace);
    doc[JSyncId] = m_syncId.ToString();
	doc[JCreatedOnUtc] = m_created.ToTimestampString();
	doc[JModifiedOnUtc] = DateTime::GetCurrentTimeUtc().ToTimestampString();
	doc[JVersion] = m_version.ToHexStr();
	doc[JSourceProjectGuid] = m_sourceProjectGuid.ToString();
	doc[JSourceDbGuid] = m_sourceDbGuid.ToString();
	return db.SavePropertyString(propSpec, doc.Stringify());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSyncInfo const& SchemaSyncInfo::Empty() {
    static SchemaSyncInfo s_empty;
    return s_empty;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaSyncInfo SchemaSyncInfo::From(DbCR db) {
	Utf8String strData;
    SchemaSyncInfo info;

    const auto propSpec = PropertySpec(JPropertyName, JPropertyNamespace);
	auto rc = db.QueryProperty(strData, propSpec);
	if (rc != BE_SQLITE_ROW) {
		return SchemaSyncInfo::Empty();
	}

	BeJsDocument doc;
	doc.Parse(strData);
	if (!doc.isObject()){
		return SchemaSyncInfo::Empty();
	}

	if (doc.hasMember(JSyncId)) {
		info.m_syncId.FromString(doc[JSyncId].asCString());
	} else {
		return SchemaSyncInfo::Empty();
	}

	if (doc.hasMember(JCreatedOnUtc)){
		info.m_created.FromString(doc[JCreatedOnUtc].asCString());
	} else {
		return SchemaSyncInfo::Empty();
	}

	if (doc.hasMember(JModifiedOnUtc)){
		info.m_modified.FromString(doc[JModifiedOnUtc].asCString());
	} else {
		return SchemaSyncInfo::Empty();
	}

	if (doc.hasMember(JSourceProjectGuid)){
		info.m_sourceProjectGuid.FromString(doc[JSourceProjectGuid].asCString());
	} else {
		return SchemaSyncInfo::Empty();
	}

	if (doc.hasMember(JSourceDbGuid)){
		info.m_sourceDbGuid.FromString(doc[JSourceDbGuid].asCString());
	} else {
		return SchemaSyncInfo::Empty();
	}

	return info;
};

END_BENTLEY_SQLITE_EC_NAMESPACE


