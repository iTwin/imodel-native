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
// 	JsonNames
//+===============+===============+===============+===============+===============+======
//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct JsonNames {
	constexpr static char ChannelId[] = "id";
	constexpr static char ChannelDataVer[] = "data_ver";
	constexpr static char ChannelProjectId[] = "project_id";
	constexpr static char ChannelFileId[] = "file_id";
	constexpr static char ChannelLastModUtc[] = "last_mod_utc";
	constexpr static char ChannelChangeSetId[] = "parent_changeset_id";
	constexpr static char ChannelChangeSetIndex[] = "parent_changeset_index";
	constexpr static char JNamespace[] = "ec_Db";
    constexpr static char JSharedChannelInfo[] = "shared_channel_info";
	constexpr static char JLocalChannelInfo[] = "local_channel_info";
};


struct SharedSchemaChannelHelper final {
    using AliasMap = bmap<Utf8String, Utf8String, CompareIUtf8Ascii>;
    using StringList = bvector<Utf8String>;
    static constexpr auto ALIAS_SHARED_DB = "channel_db";
	static constexpr auto ALIAS_MAIN_DB = "main";
    static constexpr auto TABLE_BE_PROP = "be_Prop";

	//---------------------------------------------------------------------------------------
	// @bsimethod
	//+---------------+---------------+---------------+---------------+---------------+------
	int ForeignKeyCheck(DbCR conn, std::vector<std::string>const& tables, Utf8CP dbAlias) {
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
    static DbResult GetMetaTables(DbR conn, StringList& tables, Utf8CP dbAlias) {
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
	static DbResult DropDataTables(DbR conn) {
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
	static DbResult DropMetaTables(DbR conn) {
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
	static DbResult CreateMetaTablesFrom(ECDbR fromDb, DbR syncDb) {
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
	static DbResult TryGetAttachDbs(AliasMap& aliasMap, ECDbR conn) {
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
	static DbResult VerifyAlias(ECDbR conn) {
		AliasMap aliasMap;
		auto rc = TryGetAttachDbs(aliasMap, conn);
		if (rc != BE_SQLITE_OK) {
			conn.GetImpl().Issues().Report(
				IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
				"Unable to query attach db from primary connection");
			return rc;
		}
		if (aliasMap.find(ALIAS_MAIN_DB) == aliasMap.end()) {
			conn.GetImpl().Issues().ReportV(
				IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
				"Expecting '%s' attach db on primary connection", ALIAS_MAIN_DB);
			return rc;
		}

		if (aliasMap.find(ALIAS_SHARED_DB) != aliasMap.end()) {
			conn.GetImpl().Issues().ReportV(
				IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
				"Db alias '%s' use by channel db is already in use", ALIAS_SHARED_DB);
			return rc;
		}
        return BE_SQLITE_OK;
    }

	//---------------------------------------------------------------------------------------
	// @bsimethod
	//+---------------+---------------+---------------+---------------+---------------+------
	static DbResult GetColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, StringList& columnNames) {
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
	static Utf8String Join(StringList const& list, Utf8String delimiter = ",", Utf8String prefix = "", Utf8String postfix = "") {
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
	static Utf8String ToLower(Utf8String const& val) {
		Utf8String out;
		std::for_each(val.begin(), val.end(), [&](char const& ch) {
			out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
		});
		return out;
	}

	//---------------------------------------------------------------------------------------
	// @bsimethod
	//+---------------+---------------+---------------+---------------+---------------+------
	static DbResult GetPrimaryKeyColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, StringList& columnNames) {
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
	static DbResult SyncData(ECDbR conn, StringList const& tables, Utf8CP sourceDbAlias, Utf8CP targetDbAlias) {
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
	static DbResult SyncData(ECDbR conn, Utf8CP tableName, Utf8CP sourceDbAlias, Utf8CP targetDbAlias) {
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
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SharedSchemaChannel::Status SharedSchemaChannel::Init(ChannelUri const& channelURI, TableList additionTables) {
    auto const info = channelURI.GetInfo();
	if (!info.IsEmpty()) {
        BeJsDocument doc;
        info.To(BeJsValue(doc));
        m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"Shared schema channel (%a) already initalized. %s", doc.Stringify().c_str());
        return Status::ERROR_SHARED_CHANNEL_ALREADY_INITIALIZED;
    }

    Db sharedDb;
    auto rc = sharedDb.OpenBeSQLiteDb(channelURI.GetUri().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes));
	if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"Fail to open shared db %s. %s", channelURI.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_SHARED_CHANNEL_ALREADY_INITIALIZED;
	}

	sharedDb.GetStatementCache().Empty();

    rc = SharedSchemaChannelHelper::DropDataTables(sharedDb);
	if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"Fail to drop data table(s) from shared schema channel (%s). %s", channelURI.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_FAIL_TO_INIT_SHARED_DB;
	}

    rc = SharedSchemaChannelHelper::DropMetaTables(sharedDb);
	if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"Fail to drop meta table(s) from shared schema channel (%s). %s", channelURI.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_FAIL_TO_INIT_SHARED_DB;
	}

    rc = SharedSchemaChannelHelper::CreateMetaTablesFrom(m_conn, sharedDb);
	if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"Fail to re-create meta table(s) in shared schema channel (%s). %s", channelURI.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_FAIL_TO_INIT_SHARED_DB;
	}

    rc = UpdateOrCreateSharedChannelInfo(sharedDb);
    if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"Fail to save shared channel info in (%s). %s", channelURI.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_FAIL_TO_INIT_SHARED_DB;
	}

    const auto sharedInfo = SharedChannelInfo::From(sharedDb);
    rc = sharedDb.SaveChanges();
	if (rc != BE_SQLITE_OK || sharedInfo.IsEmpty()) {
        m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"Fail to save changes to shared schema channel (%s). %s", channelURI.GetUri().c_str(), BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR_FAIL_TO_INIT_SHARED_DB;
	}

    rc = UpdateOrCreateLocalChannelInfo(sharedInfo);
    if (rc != BE_SQLITE_OK) {
        m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"Fail to save shared channel info to local db. %s", BeSQLiteLib::GetErrorString(rc));
        return Status::ERROR;
	}

    sharedDb.CloseDb();
    return PushInternal(channelURI, additionTables);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SharedSchemaChannel::Status SharedSchemaChannel::VerifyChannel(ChannelUri const& channelURI, bool isPull) {
	if (m_conn.IsReadonly()) {
		m_conn.GetImpl().Issues().Report(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"Primary connection is readonly. It must be in read/write mode.");
		return Status::ERROR_READONLY;
	}

	ECDb sharedDb;
    DbResult rc = BE_SQLITE_OK;
    if (isPull) {
		rc = sharedDb.OpenBeSQLiteDb(channelURI.GetUri().c_str(), Db::OpenParams(Db::OpenMode::Readonly));
		if (rc != BE_SQLITE_OK) {
				m_conn.GetImpl().Issues().ReportV(
					IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
					"Fail to to open shared schema channel db in readonly mode: (%s)", channelURI.GetUri().c_str());
			return Status::ERROR_OPENING_SHARED_DB;
		}
	} else {
		rc = sharedDb.OpenBeSQLiteDb(channelURI.GetUri().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
		if (rc != BE_SQLITE_OK) {
				m_conn.GetImpl().Issues().ReportV(
					IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
					"Fail to to open shared schema channel db in readonly mode: (%s)", channelURI.GetUri().c_str());
			return Status::ERROR_OPENING_SHARED_DB;
		}

	}
    const auto sharedChannelInfo = SharedChannelInfo::From(sharedDb);
	if (sharedChannelInfo.IsEmpty()) {
		m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"Invalid shared channel db (%s). Shared channel info not found.", channelURI.GetUri().c_str());
		return Status::ERROR_INVALID_SHARED_DB;
	}

    const auto localChannelInfo = LocalChannelInfo::From(m_conn);
	if (localChannelInfo.IsEmpty()) {
		m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"Local db is not set to use shared schema channel (%s).", channelURI.GetUri().c_str());
		return Status::ERROR_INVALID_LOCAL_DB;
	}

    if (sharedChannelInfo.GetChannelId() != localChannelInfo.GetChannelId()) {
		m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"channel id does not match (local) %s <> (shared) %s.",
				localChannelInfo.GetChannelId().ToString().c_str(),
				sharedChannelInfo.GetChannelId().ToString().c_str());
		return Status::ERROR_INVALID_SHARED_CHANNEL;
	}

    const auto projectGUID = m_conn.QueryProjectGuid();
    if (sharedChannelInfo.GetProjectId() != projectGUID) {
		m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"project id does not match (local) %s <> (shared) %s.",
				projectGUID.ToString().c_str(),
				sharedChannelInfo.GetProjectId().ToString().c_str());
		return Status::ERROR_INVALID_SHARED_CHANNEL;
	}

    const auto fileGUID = m_conn.GetDbGuid();
    if (sharedChannelInfo.GetFileId() != fileGUID) {
		m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"project id does not match (local) %s <> (shared) %s.",
				fileGUID.ToString().c_str(),
				sharedChannelInfo.GetFileId().ToString().c_str());
		return Status::ERROR_INVALID_SHARED_CHANNEL;
	}
	sharedDb.CloseDb();
    return Status::SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SharedSchemaChannel::Status SharedSchemaChannel::PullInternal(ChannelUri const& channelURI, TableList additionTables) {
    const auto vrc = VerifyChannel(channelURI, true);
	if  (vrc != Status::SUCCESS) {
        return vrc;
    }

    const auto sharedChannelInfo = channelURI.GetInfo();
    const auto localChannelInfo = GetInfo();
    if (sharedChannelInfo.GetDataVersion() == localChannelInfo.GetDataVersion()) {
        return Status::SUCCESS;
    }

    if (sharedChannelInfo.GetDataVersion() < localChannelInfo.GetDataVersion()) {
		// this should never happen.
        return Status::ERROR;
	}

	if (SharedSchemaChannelHelper::VerifyAlias(m_conn) != BE_SQLITE_OK) {
        return Status::ERROR;
    }

    auto rc = m_conn.AttachDb(channelURI.GetUri().c_str(), SharedSchemaChannelHelper::ALIAS_SHARED_DB);
	if (rc != BE_SQLITE_OK) {
		m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"Unable to attach sync db '%s' as '%s' to primary connection: %s",
			channelURI.GetUri().c_str(),
			SharedSchemaChannelHelper::ALIAS_SHARED_DB,
			m_conn.GetLastError().c_str());
        return Status::ERROR_UNABLE_TO_ATTACH;
	}

	// pull changes ================================================
    const auto fromAlias = SharedSchemaChannelHelper::ALIAS_SHARED_DB;
	const auto toAlias = SharedSchemaChannelHelper::ALIAS_MAIN_DB;

    TableList tables;
    rc = SharedSchemaChannelHelper::GetMetaTables(m_conn, tables, fromAlias);
    if (rc != BE_SQLITE_OK) {
		m_conn.AbandonChanges();
		m_conn.DetachDb(SharedSchemaChannelHelper::ALIAS_SHARED_DB);
        return Status::ERROR;
    }

    tables.insert(tables.end(), additionTables.begin(), additionTables.end());
    rc = SharedSchemaChannelHelper::SyncData(m_conn, tables, fromAlias, toAlias);
    if (rc != BE_SQLITE_OK) {
		m_conn.AbandonChanges();
		m_conn.DetachDb(SharedSchemaChannelHelper::ALIAS_SHARED_DB);
        return Status::ERROR;
    }

    rc = UpdateOrCreateLocalChannelInfo(sharedChannelInfo);
    if (rc != BE_SQLITE_OK) {
		m_conn.AbandonChanges();
		m_conn.DetachDb(SharedSchemaChannelHelper::ALIAS_SHARED_DB);
        return Status::ERROR;
    }

	rc = m_conn.DetachDb(SharedSchemaChannelHelper::ALIAS_SHARED_DB);
    if (rc != BE_SQLITE_OK) {
		return Status::ERROR;
	}

    return Status::SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SharedSchemaChannel::Status SharedSchemaChannel::PushInternal(ChannelUri const& channelURI, TableList additionTables) {
    const auto vrc = VerifyChannel(channelURI, false);
	if  (vrc != Status::SUCCESS) {
        return vrc;
    }

    const auto sharedChannelInfo = channelURI.GetInfo();
    const auto localChannelInfo = GetInfo();
    if (sharedChannelInfo.GetDataVersion() != localChannelInfo.GetDataVersion()) {
        return Status::ERROR;
    }

	if (SharedSchemaChannelHelper::VerifyAlias(m_conn) != BE_SQLITE_OK) {
        return Status::ERROR;
    }

    auto rc = m_conn.AttachDb(channelURI.GetUri().c_str(), SharedSchemaChannelHelper::ALIAS_SHARED_DB);
	if (rc != BE_SQLITE_OK) {
		m_conn.GetImpl().Issues().ReportV(
			IssueSeverity::Error, IssueCategory::SchemaSynchronization, IssueType::ECDbIssue,
			"Unable to attach sync db '%s' as '%s' to primary connection: %s",
			channelURI.GetUri().c_str(),
			SharedSchemaChannelHelper::ALIAS_SHARED_DB,
			m_conn.GetLastError().c_str());
        return Status::ERROR_UNABLE_TO_ATTACH;
	}

	// pull changes ================================================
    const auto fromAlias = SharedSchemaChannelHelper::ALIAS_MAIN_DB;
	const auto toAlias = SharedSchemaChannelHelper::ALIAS_SHARED_DB;

    TableList tables;
    rc = SharedSchemaChannelHelper::GetMetaTables(m_conn, tables, fromAlias);
    if (rc != BE_SQLITE_OK) {
		m_conn.AbandonChanges();
		m_conn.DetachDb(SharedSchemaChannelHelper::ALIAS_SHARED_DB);
        return Status::ERROR;
    }

    tables.insert(tables.end(), additionTables.begin(), additionTables.end());
    rc = SharedSchemaChannelHelper::SyncData(m_conn, tables, fromAlias, toAlias);
    if (rc != BE_SQLITE_OK) {
		m_conn.AbandonChanges();
		m_conn.DetachDb(SharedSchemaChannelHelper::ALIAS_SHARED_DB);
        return Status::ERROR;
    }

	rc = m_conn.SaveChanges();
    if (rc != BE_SQLITE_OK) {
		m_conn.DetachDb(SharedSchemaChannelHelper::ALIAS_SHARED_DB);
        return Status::ERROR;
    }

	rc = m_conn.DetachDb(SharedSchemaChannelHelper::ALIAS_SHARED_DB);
    if (rc != BE_SQLITE_OK) {
        return Status::ERROR;
	}

    rc = UpdateOrCreateSharedChannelInfo(channelURI);
    if (rc != BE_SQLITE_OK) {
        return Status::ERROR;
    }

    rc = UpdateOrCreateLocalChannelInfo(channelURI.GetInfo());
    if (rc != BE_SQLITE_OK) {
        return Status::ERROR;
    }
    return Status::SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SharedSchemaChannel::Status SharedSchemaChannel::Init(ChannelUri const& channelURI) {
	ECDB_PERF_LOG_SCOPE("Initializing shared schema channel");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SharedSchemaChannel::Init");
	BeMutexHolder holder(m_conn.GetImpl().GetMutex());
    const auto rc = Init(channelURI, { SharedSchemaChannelHelper::TABLE_BE_PROP });
	STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SharedSchemaChannel::Init");
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SharedSchemaChannel::Status SharedSchemaChannel::Pull(ChannelUri const& channelURI, SchemaImportToken const* schemaImportToken) {
    ECDB_PERF_LOG_SCOPE("Pulling from shared schema channel");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SharedSchemaChannel::Pull");

	BeMutexHolder holder(m_conn.GetImpl().GetMutex());
    auto& mainDisp = m_conn.Schemas().Main();
    Policy policy = PolicyManager::GetPolicy(SchemaImportPermissionPolicyAssertion(m_conn, schemaImportToken));
    if (!policy.IsSupported()) {
        LOG.error("Failed to drop ECSchema: Caller has not provided a SchemaImportToken.");
        return Status::ERROR;
    }

    mainDisp.OnBeforeSchemaChanges().RaiseEvent(m_conn, SchemaChangeType::SchemaImport);
    SchemaImportContext ctx(m_conn, SchemaManager::SchemaImportOptions(), /* synchronizeSchemas = */true);
    m_conn.ClearECDbCache();

    const auto rc = PullInternal(channelURI, {});
	if (rc != Status::SUCCESS) {
        return rc;
    }

    if (SUCCESS != mainDisp.GetDbSchema().ForceReloadTableAndIndexesFromDisk()) {
        return Status::ERROR;
    }
    // pull changes local schema
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
    mainDisp.OnAfterSchemaChanges().RaiseEvent(m_conn, SchemaChangeType::SchemaImport);

	STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SharedSchemaChannel::Pull");
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SharedSchemaChannel::Status SharedSchemaChannel::Push(ChannelUri const& channelURI) {
    ECDB_PERF_LOG_SCOPE("Pushing tp shared schema channel");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SharedSchemaChannel::Push");
    BeMutexHolder holder(m_conn.GetImpl().GetMutex());
    const auto rc = PushInternal(channelURI, {});

    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SharedSchemaChannel::Push");
    return rc;
}

//=======================================================================================
// 	SharedSchemaChannel::LocalChannelInfo
//+===============+===============+===============+===============+===============+======
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SharedSchemaChannel::LocalChannelInfo SharedSchemaChannel::GetInfo() const {
    return LocalChannelInfo::From(m_conn);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SharedSchemaChannel::GetParentRevisionId() const {
    const auto PARENT_CS_ID = "ParentChangeSetId";
	Utf8String revisionId;
    DbResult result = m_conn.QueryBriefcaseLocalValue(revisionId, PARENT_CS_ID);
    return (BE_SQLITE_ROW == result) ? revisionId : "";
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SharedSchemaChannel::GetParentRevision(int32_t& index, Utf8StringR id) const {
    const auto PARENT_CHANGESET = "parentChangeset";
    id = GetParentRevisionId();
    index = id.empty() ? 0 : -1;
    Utf8String json;

    if (BE_SQLITE_ROW != m_conn.QueryBriefcaseLocalValue(json, PARENT_CHANGESET))
        return;

    BeJsDocument jsonObj(json);
    if (jsonObj.isStringMember("id") && jsonObj.isNumericMember("index") && id.Equals(jsonObj["id"].asString()))
        index = jsonObj["index"].GetInt();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SharedSchemaChannel::UpdateOrCreateSharedChannelInfo(DbR channelDb) {
    auto info = SharedChannelInfo::From(channelDb);
	if (info.IsEmpty()) {
        info.m_channelId.Create();
        info.m_projectId = m_conn.QueryProjectGuid();
        info.m_fileId = m_conn.GetDbGuid();
    }

	int32_t csIndex;
	Utf8String csId;
	GetParentRevision(csIndex, csId);
	info.m_changesetId = csId;
	info.m_changesetIndex = csIndex;
	info.m_lastModUtc = DateTime::GetCurrentTimeUtc();
    info.m_dataVer += 1;

    const auto propSpec = PropertySpec(JsonNames::JSharedChannelInfo, JsonNames::JNamespace);
	BeJsDocument jsonDoc;
    info.To(BeJsValue(jsonDoc));
    auto rc = channelDb.SavePropertyString(propSpec, jsonDoc.Stringify());
	if (rc != BE_SQLITE_OK) {
        return rc;
	}
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SharedSchemaChannel::UpdateOrCreateSharedChannelInfo(ChannelUri channelUri) {
    Db conn;
    auto rc = conn.OpenBeSQLiteDb(channelUri.GetUri().c_str(), Db::OpenMode(Db::OpenMode::ReadWrite));
    if (rc != BE_SQLITE_OK) {
        return BE_SQLITE_ERROR;
    }

    rc = UpdateOrCreateSharedChannelInfo(conn);
    if (rc != BE_SQLITE_OK) {
        conn.AbandonChanges();
        return rc;
    }
    return conn.SaveChanges();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SharedSchemaChannel::UpdateOrCreateLocalChannelInfo(SharedChannelInfo const& from) {
    auto info = GetInfo();
	info.m_dataVer = from.m_dataVer;
	info.m_lastModUtc = from.GetLastModUtc();
	info.m_channelId = from.m_channelId;

	// Save property
    const auto propSpec = PropertySpec(JsonNames::JLocalChannelInfo, JsonNames::JNamespace);
	BeJsDocument jsonDoc;
    info.To(BeJsValue(jsonDoc));
    auto rc = m_conn.SavePropertyString(propSpec, jsonDoc.Stringify());
	if (rc != BE_SQLITE_OK) {
        return rc;
    }
    return BE_SQLITE_OK;
}
//=======================================================================================
// 	SharedSchemaChannel::SharedChannelInfo
//+===============+===============+===============+===============+===============+======
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SharedSchemaChannel::SharedChannelInfo::To(BeJsValue val) const {
    val.SetEmptyObject();
    val[JsonNames::ChannelDataVer] = BeInt64Id(m_dataVer).ToHexStr();
    val[JsonNames::ChannelId] = m_channelId.ToString();
	val[JsonNames::ChannelProjectId] = m_projectId.ToString();
	val[JsonNames::ChannelFileId] = m_fileId.ToString();
	val[JsonNames::ChannelLastModUtc] = m_lastModUtc.ToTimestampString();
	val[JsonNames::ChannelChangeSetId] = m_changesetId;
	val[JsonNames::ChannelChangeSetIndex] = m_changesetIndex;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SharedSchemaChannel::SharedChannelInfo SharedSchemaChannel::ChannelUri::GetInfo() const{
	if (IsEmpty()) {
        return SharedChannelInfo();
    }

	Db conn;
	if (conn.OpenBeSQLiteDb(m_uri.c_str(), Db::OpenParams(Db::OpenMode::Readonly)) != BE_SQLITE_OK) {
		return SharedChannelInfo();
	}
    return SharedChannelInfo::From(conn);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SharedSchemaChannel::SharedChannelInfo SharedSchemaChannel::SharedChannelInfo::From(DbCR conn){
    Utf8String strData;
    const auto propSpec = PropertySpec(JsonNames::JSharedChannelInfo, JsonNames::JNamespace);
	auto rc = conn.QueryProperty(strData, propSpec);
	if (rc != BE_SQLITE_ROW) {
		return SharedChannelInfo();
	}
    BeJsDocument jsonDoc;
    jsonDoc.Parse(strData);
    return SharedChannelInfo::From(BeJsConst(jsonDoc));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SharedSchemaChannel::SharedChannelInfo SharedSchemaChannel::SharedChannelInfo::From(BeJsConst val){
    static SharedChannelInfo s_empty;
    if (!val.isObject()){
		return s_empty;
	}

    SharedChannelInfo info;
    if (val.isStringMember(JsonNames::ChannelDataVer)
		 && val.isStringMember(JsonNames::ChannelId)
		 && val.isStringMember(JsonNames::ChannelProjectId)
		 && val.isStringMember(JsonNames::ChannelFileId)
		 && val.isStringMember(JsonNames::ChannelLastModUtc)
		 && val.isStringMember(JsonNames::ChannelChangeSetId)
		 && val.isNumericMember(JsonNames::ChannelChangeSetIndex)
	) {
        BentleyStatus status;
        info.m_dataVer = BeInt64Id::FromString(val[JsonNames::ChannelDataVer].asCString(), &status).GetValueUnchecked();
		if (status == ERROR) {
            return s_empty;
        }

        info.m_channelId.FromString(val[JsonNames::ChannelId].asCString());
		if (!info.m_channelId.IsValid()) {
			return s_empty;
		}

		info.m_projectId.FromString(val[JsonNames::ChannelProjectId].asCString());
		info.m_fileId.FromString(val[JsonNames::ChannelFileId].asCString());
		info.m_lastModUtc.FromString(val[JsonNames::ChannelLastModUtc].asCString());
        info.m_changesetId = val[JsonNames::ChannelChangeSetId].asCString();
        info.m_changesetIndex = (int32_t)val[JsonNames::ChannelChangeSetIndex].asInt();
        return info;
    }
    return s_empty;
}

//=======================================================================================
// 	SharedSchemaChannel::LocalChannelInfo
//+===============+===============+===============+===============+===============+======
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SharedSchemaChannel::LocalChannelInfo::To(BeJsValue val) const {
    val.SetEmptyObject();
    val[JsonNames::ChannelDataVer] = BeInt64Id(m_dataVer).ToHexStr();
    val[JsonNames::ChannelId] = m_channelId.ToString();
	val[JsonNames::ChannelLastModUtc] = m_lastModUtc.ToTimestampString();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SharedSchemaChannel::LocalChannelInfo SharedSchemaChannel::LocalChannelInfo::From(DbCR conn){
    Utf8String strData;
    const auto propSpec = PropertySpec(JsonNames::JLocalChannelInfo, JsonNames::JNamespace);
	auto rc = conn.QueryProperty(strData, propSpec);
	if (rc != BE_SQLITE_ROW) {
		return LocalChannelInfo();
	}
    BeJsDocument jsonDoc;
    jsonDoc.Parse(strData);
    return LocalChannelInfo::From(BeJsConst(jsonDoc));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SharedSchemaChannel::LocalChannelInfo SharedSchemaChannel::LocalChannelInfo::From(BeJsConst val){
    static LocalChannelInfo s_empty;
    if (!val.isObject()){
		return s_empty;
	}

    LocalChannelInfo info;
    if (val.isStringMember(JsonNames::ChannelDataVer)
		 && val.isStringMember(JsonNames::ChannelId)
		 && val.isStringMember(JsonNames::ChannelLastModUtc)
	) {
        BentleyStatus status;
        info.m_dataVer = BeInt64Id::FromString(val[JsonNames::ChannelDataVer].asCString(), &status).GetValueUnchecked();
		if (status == ERROR) {
            return s_empty;
        }

        info.m_channelId.FromString(val[JsonNames::ChannelId].asCString());
		if (!info.m_channelId.IsValid()) {
			return s_empty;
		}

		info.m_lastModUtc.FromString(val[JsonNames::ChannelLastModUtc].asCString());
        return info;
    }
    return s_empty;
}

END_BENTLEY_SQLITE_EC_NAMESPACE


