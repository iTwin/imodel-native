/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// PragmaChecksum
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaChecksum::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val, PragmaManager::OptionsMap const& options) {
	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	result->AppendProperty("sha3_256", PRIMITIVETYPE_String);
	result->FreezeSchemaChanges();

	if (!val.IsName() && !val.IsString()) {
		ecdb.GetImpl().Issues().Report(
			IssueSeverity::Error,
			IssueCategory::BusinessProperties,
			IssueType::ECSQL,
			ECDbIssueId::ECDb_0592,
			"unsupported val for checksum pragma.");

		rowSet = std::move(result);
		return BE_SQLITE_ERROR;
	}

    const Utf8String kECDbSchema = "ecdb_schema";
	const Utf8String kECDbMap = "ecdb_map";
	const Utf8String kSQLiteSchema = "sqlite_schema";

    if (kECDbSchema.EqualsIAscii(val.GetString())) {
        Utf8String sha3;
		if (SHA3Helper::ComputeHash(sha3, ecdb, SHA3Helper::SourceType::ECDB_SCHEMA, "main", SHA3Helper::HashSize::SHA3_256) != BE_SQLITE_OK) {
			ecdb.GetImpl().Issues().Report(
				IssueSeverity::Error,
				IssueCategory::BusinessProperties,
				IssueType::ECSQL,
				ECDbIssueId::ECDb_0593,
				"Unable to compute sha3/256 checksum for ec schemas.");

			rowSet = std::move(result);
            return BE_SQLITE_ERROR;
        }
		auto row = result->AppendRow();
        row.appendValue() = sha3;
        rowSet = std::move(result);
		return BE_SQLITE_OK;
    }
    if (kECDbMap.EqualsIAscii(val.GetString())) {
        Utf8String sha3;
		if (SHA3Helper::ComputeHash(sha3, ecdb, SHA3Helper::SourceType::ECDB_MAP, "main", SHA3Helper::HashSize::SHA3_256) != BE_SQLITE_OK) {
			ecdb.GetImpl().Issues().Report(
				IssueSeverity::Error,
				IssueCategory::BusinessProperties,
				IssueType::ECSQL,
				ECDbIssueId::ECDb_0594,
				"Unable to compute sha3/256 checksum for ec map.");

			rowSet = std::move(result);
            return BE_SQLITE_ERROR;
        }
		auto row = result->AppendRow();
        row.appendValue() = sha3;
		rowSet = std::move(result);
		return BE_SQLITE_OK;
    }
    if (kSQLiteSchema.EqualsIAscii(val.GetString())) {
        Utf8String sha3;
		if (SHA3Helper::ComputeHash(sha3, ecdb, SHA3Helper::SourceType::SQLITE_SCHEMA, "main", SHA3Helper::HashSize::SHA3_256) != BE_SQLITE_OK) {
			ecdb.GetImpl().Issues().Report(
				IssueSeverity::Error,
				IssueCategory::BusinessProperties,
				IssueType::ECSQL,
				ECDbIssueId::ECDb_0595,
				"Unable to compute sha3/256 checksum for db schema.");

			rowSet = std::move(result);
            return BE_SQLITE_ERROR;
        }
		auto row = result->AppendRow();
        row.appendValue() = sha3;
		rowSet = std::move(result);
		return BE_SQLITE_OK;
    }

	ecdb.GetImpl().Issues().ReportV(
		IssueSeverity::Error,
		IssueCategory::BusinessProperties,
		IssueType::ECSQL,
		ECDbIssueId::ECDb_0596,
		"Unable checksum val '%s'. Valid values are ecdb_schema|ecdb_map|sqlite_schema", val.GetString().c_str());

	rowSet = std::move(result);
	return BE_SQLITE_ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaChecksum::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const& options) {
	ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0552, "PRAGMA %s is readonly.", GetName().c_str());
	rowSet = std::make_unique<StaticPragmaResult>(ecdb);
	rowSet->FreezeSchemaChanges();
	return BE_SQLITE_READONLY;
}

//=======================================================================================
// PragmaParseTree
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaParseTree::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& ecsql, PragmaManager::OptionsMap const& options) {
    if (!isExperimentalFeatureAllowed(ecdb, options)) {
		ecdb.GetImpl().Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0597, "'PRAGMA parse_tree' is experimental feature and disabled by default.");
		return BE_SQLITE_ERROR;
	}

	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	result->AppendProperty("val", PRIMITIVETYPE_String);
	result->FreezeSchemaChanges();

    BeJsDocument out;
    if (SUCCESS != ECSqlParseTreeFormatter::ECSqlToJson(out, ecdb, ecsql.GetString().c_str())) {
		ecdb.GetImpl().Issues().ReportV(
			IssueSeverity::Error,
			IssueCategory::BusinessProperties,
			IssueType::ECSQL,
			ECDbIssueId::ECDb_0598,
			"Unable to parse ecsql '%s'. ", ecsql.GetString().c_str());
	}

	auto row = result->AppendRow();
	row.appendValue() = out.Stringify();
	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaParseTree::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const& options) {
	ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0552, "PRAGMA %s is readonly.", GetName().c_str());
	rowSet = std::make_unique<StaticPragmaResult>(ecdb);
	rowSet->FreezeSchemaChanges();
	return BE_SQLITE_READONLY;
}
//=======================================================================================
// DisqualifyTypeIndex
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaECDbVersion::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const& options) {
	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	result->AppendProperty("current", PRIMITIVETYPE_String);
	result->AppendProperty("file", PRIMITIVETYPE_String);
	result->FreezeSchemaChanges();
	auto row = result->AppendRow();
	row.appendValue() = ECDb::CurrentECDbProfileVersion().ToString();
	row.appendValue() =  ecdb.GetECDbProfileVersion().ToString();
	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaECDbVersion::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const& options) {
	ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0552, "PRAGMA %s is readonly.", GetName().c_str());
	rowSet = std::make_unique<StaticPragmaResult>(ecdb);
	rowSet->FreezeSchemaChanges();
	return BE_SQLITE_READONLY;
}

//=======================================================================================
// DisqualifyTypeIndex
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult DisqualifyTypeIndex::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, ECClassCR cls, PragmaManager::OptionsMap const& options) {
	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	result->AppendProperty(GetName(), PRIMITIVETYPE_Boolean);
	result->FreezeSchemaChanges();
	result->AppendRow().appendValue() = m_disqualifiedClassSet.find(cls.GetId()) != m_disqualifiedClassSet.end();
	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult DisqualifyTypeIndex::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val, ECClassCR cls, PragmaManager::OptionsMap const& options) {
	if (!val.IsBool() && !val.IsInteger()) {
		ecdb.GetImpl().Issues().ReportV(
			IssueSeverity::Error,
			IssueCategory::BusinessProperties,
			IssueType::ECSQL, ECDbIssueId::ECDb_0599, "PRAGMA %s, expect bool or integer value", GetName().c_str());
		return BE_SQLITE_ERROR;
	}
	auto mapInfo = ecdb.Schemas().GetClassMapStrategy(cls.GetSchema().GetName(), cls.GetName());
	if (mapInfo.IsNotMapped() || mapInfo.IsEmpty()) {
		ecdb.GetImpl().Issues().ReportV(
			IssueSeverity::Error,
			IssueCategory::BusinessProperties,
			IssueType::ECSQL, ECDbIssueId::ECDb_0600, "PRAGMA %s, is not invoked on class %s which is not mapped or exist.", GetName().c_str(), cls.GetFullName());
		return BE_SQLITE_ERROR;
	}
	if (val.GetBool()) {
		m_disqualifiedClassSet.insert(cls.GetId());
	} else {
		m_disqualifiedClassSet.erase(cls.GetId());
	}
	rowSet = std::make_unique<StaticPragmaResult>(ecdb);
	rowSet->FreezeSchemaChanges();
	return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaExplainQuery
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaExplainQuery::ToResultSet (Statement& from, StaticPragmaResult& to) {
	auto createSchema = [&]() {
		for(int i =0; i < from.GetColumnCount(); ++i) {
			if (from.GetColumnType(i) == DbValueType::TextVal) {
				to.AppendProperty(from.GetColumnName(i) , PRIMITIVETYPE_String);
			} else if (from.GetColumnType(i) == DbValueType::FloatVal) {
				to.AppendProperty(from.GetColumnName(i) , PRIMITIVETYPE_Double);
			} else if (from.GetColumnType(i) == DbValueType::IntegerVal) {
				to.AppendProperty(from.GetColumnName(i) , PRIMITIVETYPE_Long);
			} else {
				return BE_SQLITE_ERROR;
			}
		}
		to.FreezeSchemaChanges();
		return BE_SQLITE_OK;
	};
	while(from.Step() == BE_SQLITE_ROW) {
		if (to.GetColumnCount() == 0) {
			const auto rc = createSchema();
			if ( rc != BE_SQLITE_OK) {
				return rc;
			}
		}
		auto row = to.AppendRow();
		for(int i =0; i < from.GetColumnCount(); ++i) {
			if (from.GetColumnType(i) == DbValueType::TextVal) {
				row.appendValue() = from.GetValueText(i);
			} else if (from.GetColumnType(i) == DbValueType::FloatVal) {
				row.appendValue() = from.GetValueDouble(i);
			} else if (from.GetColumnType(i) == DbValueType::IntegerVal) {
				row.appendValue() = from.GetValueInt64(i);
			} else {
				row.appendValue().SetNull();
			}
		}
	}
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaExplainQuery::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val, PragmaManager::OptionsMap const& options) {
	if (!val.IsString()) {
		ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0601, "PRAGMA %s expect a ECSQL query as string argument.", GetName().c_str());
		return BE_SQLITE_ERROR;
	}
	ECSqlStatement stmt;
	if (ECSqlStatus::Success != stmt.Prepare(ecdb, val.GetString().c_str())) {
		ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0602, "PRAGMA %s failed to prepare ECSQL query.", GetName().c_str());
		return BE_SQLITE_ERROR;
	}
	Statement sqlStmt;
	if (BE_SQLITE_OK != sqlStmt.Prepare(ecdb, SqlPrintfString("explain query plan %s", stmt.GetNativeSql()))){
		ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0603, "PRAGMA %s failed to explain native sql.", GetName().c_str());
		return BE_SQLITE_ERROR;
	}

	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	const auto rc = ToResultSet(sqlStmt, *result);
	if (rc == BE_SQLITE_OK) {
		rowSet = std::move(result);
	}
	return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaExplainQuery::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const& options) {
	ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0552, "PRAGMA %s is readonly.", GetName().c_str());
	rowSet = std::make_unique<StaticPragmaResult>(ecdb);
	rowSet->FreezeSchemaChanges();
	return BE_SQLITE_READONLY;
}

//=======================================================================================
// DisqualifyTypeIndex
//=======================================================================================
DbResult SHA3Helper::ComputeHash(Utf8StringR hash, DbCR db, SourceType type, Utf8CP dbAlias, HashSize hashSize) {
    DbResult rc = BE_SQLITE_OK;
    const auto skipTableThatDoesNotExists = true; // profile dependent tables
    if (type == SourceType::ECDB_SCHEMA) {
		rc = ComputeHash(hash, db, {
			"ec_Schema",
			"ec_SchemaReference",
			"ec_Class",
			"ec_ClassHasBaseClasses",
			"ec_Enumeration",
			"ec_KindOfQuantity",
			"ec_UnitSystem",
			"ec_Phenomenon",
			"ec_Unit",
			"ec_Format",
			"ec_FormatCompositeUnit",
			"ec_PropertyCategory",
			"ec_Property",
			"ec_RelationshipConstraint",
			"ec_RelationshipConstraintClass",
			"ec_CustomAttribute",
		}, dbAlias, hashSize, skipTableThatDoesNotExists);
	} else if (type == SourceType::ECDB_MAP) {
		rc = ComputeHash(hash, db, {
			"ec_PropertyPath",
			"ec_ClassMap",
			"ec_Table",
			"ec_Column",
			"ec_Index",
			"ec_IndexColumn",
			"ec_PropertyMap",
		}, dbAlias, hashSize, skipTableThatDoesNotExists);
	} else if (type == SourceType::SQLITE_SCHEMA) {
        rc = ComputeSQLiteSchemaHash(hash, db, dbAlias, hashSize);
    }
	if (rc != BE_SQLITE_OK) {
		return rc;
	}

    hash.ToLower();
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SHA3Helper::ComputeSQLiteSchemaHash(Utf8String& hash, DbCR db, Utf8CP dbAlias, HashSize hashSize) {
	Statement stmt;
	auto rc = stmt.Prepare(db, SqlPrintfString(
		"select hex(sha3_query(\"select name||':'||iif(sql is null,'',sql) er from [%s].sqlite_master order by er\", %d))", dbAlias, (int)hashSize));

	if (rc != BE_SQLITE_OK) {
		return rc;
	}

    rc = stmt.Step();
    if (rc != BE_SQLITE_ROW) {
        return rc;
    }

    hash = stmt.GetValueText(0);
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool SHA3Helper::TableExists(DbCR db, Utf8CP tableName, Utf8CP dbAlias) {
	Statement stmt;
    auto rc = stmt.Prepare(db, SqlPrintfString("PRAGMA [%s].table_info(%s)", dbAlias, tableName).GetUtf8CP());
	if (rc != BE_SQLITE_OK) {
        return false;
    }

    rc = stmt.Step();
    if (rc == BE_SQLITE_ROW) {
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SHA3Helper::ComputeHash(Utf8String& hash, DbCR db, std::vector<std::string> const & tables, Utf8CP dbAlias, HashSize hashSize, bool skipTableThatDoesNotExists) {
	if (!db.IsDbOpen()) {
		return BE_SQLITE_ERROR_NOTOPEN;
	}

	std::vector<std::string> tablesThatExits;
	for (auto& table: tables) {
		if (TableExists(db, table.c_str(), dbAlias)) {
            tablesThatExits.push_back(SqlPrintfString("select * from [%s].[%s] order by [rowid]", dbAlias, table.c_str()).GetUtf8CP());
        } else {
			if (!skipTableThatDoesNotExists) {
                return BE_SQLITE_NOTFOUND;
            }
		}
	}

	auto sqlList = std::accumulate(
		std::next(tablesThatExits.begin()),
		std::end(tablesThatExits),
		std::string{tablesThatExits.front()},
		[&](std::string const& acc, const std::string& piece) {
			return acc + ";" + piece;
		});

    const auto sql = std::string { SqlPrintfString("select hex(sha3_query(\"%s\", %d))", sqlList.c_str(), (int)hashSize).GetUtf8CP() };
	Statement stmt;
	auto rc = stmt.Prepare(db, sql.c_str());
	if (rc != BE_SQLITE_OK) {
		return rc;
	}

    rc = stmt.Step();
	if (rc != BE_SQLITE_ROW) {
		return rc;
	}

    hash = stmt.GetValueText(0);
    return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaExperimentalFeatures
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaExperimentalFeatures::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const& options)   {
	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	result->AppendProperty("experimental_features_enabled", PRIMITIVETYPE_Boolean);
	result->FreezeSchemaChanges();
	auto row = result->AppendRow();
	row.appendValue() = ecdb.GetImpl().GetECSqlConfig().GetExperimentalFeaturesEnabled();
	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaExperimentalFeatures::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val, PragmaManager::OptionsMap const& options) {
	if (val.IsBool()) {
		ecdb.GetImpl().GetECSqlConfig().SetExperimentalFeaturesEnabled(val.GetBool());
	}
	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	result->AppendProperty("experimental_features_enabled", PRIMITIVETYPE_Boolean);
	result->FreezeSchemaChanges();
	auto row = result->AppendRow();
	row.appendValue() = ecdb.GetImpl().GetECSqlConfig().GetExperimentalFeaturesEnabled();
	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaIntegrityCheck
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaIntegrityCheck::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& v, PragmaManager::OptionsMap const& options) {
    if (!isExperimentalFeatureAllowed(ecdb, options)) {
		ecdb.GetImpl().Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0604, "'PRAGMA integrity_check' is experimental feature and disabled by default.");
		return BE_SQLITE_ERROR;
	}

	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	IntegrityChecker checker(ecdb);
	DbResult rc = BE_SQLITE_OK;
	auto checks = IntegrityChecker::Checks::All;
	if (v.IsString()) {
		auto customCheck = IntegrityChecker::GetCheckId(v.GetString().c_str());
		if (customCheck != IntegrityChecker::Checks::None) {
			checks = customCheck;
		}
	}

	switch(checks) {
		case IntegrityChecker::Checks::CheckDataColumns:
			rc = CheckDataColumns(checker, *result, ecdb); break;
		case IntegrityChecker::Checks::CheckDataSchema:
			rc = CheckDataSchema(checker, *result, ecdb); break;
		case IntegrityChecker::Checks::CheckClassIds:
			rc = CheckClassIds(checker, *result, ecdb); break;
		case IntegrityChecker::Checks::CheckLinkTableFkClassIds:
			rc = CheckLinkTableFkClassIds(checker, *result, ecdb); break;
		case IntegrityChecker::Checks::CheckLinkTableFkIds:
			rc = CheckLinkTableFkIds(checker, *result, ecdb); break;
		case IntegrityChecker::Checks::CheckNavClassIds:
			rc = CheckNavClassIds(checker, *result, ecdb); break;
		case IntegrityChecker::Checks::CheckNavIds:
			rc = CheckNavIds(checker, *result, ecdb); break;
		case IntegrityChecker::Checks::CheckEcProfile:
			rc = CheckEcProfile(checker, *result, ecdb); break;
		case IntegrityChecker::Checks::CheckSchemaLoad:
			rc = CheckSchemaLoad(checker, *result, ecdb); break;
		case IntegrityChecker::Checks::CheckMissingChildRows:
			rc = CheckMissingChildRows(checker, *result, ecdb); break;
		default:
			rc = CheckAll(checker, *result, ecdb);
		};
	rowSet = std::move(result);
	return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaIntegrityCheck::CheckAll(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb) {
	result.AppendProperty("sno", PRIMITIVETYPE_Integer);
	result.AppendProperty("check", PRIMITIVETYPE_String);
	result.AppendProperty("result", PRIMITIVETYPE_Boolean);
	result.AppendProperty("elapsed_sec", PRIMITIVETYPE_String);
	result.FreezeSchemaChanges();

	int rowCount = 1;
	return checker.QuickCheck(IntegrityChecker::Checks::All, [&](Utf8CP checkName, bool passed, BeDuration dur) {
		auto row = result.AppendRow();
		row.appendValue() = rowCount++;
		row.appendValue() = checkName;
		row.appendValue() = passed;
		row.appendValue() = Utf8PrintfString("%.3f", dur.ToSeconds());
	});
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaIntegrityCheck::CheckSchemaLoad(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb) {
	result.AppendProperty("sno", PRIMITIVETYPE_Integer);
	result.AppendProperty("schema", PRIMITIVETYPE_String);
	result.FreezeSchemaChanges();
	int rowCount = 1;
	return checker.CheckSchemaLoad([&](Utf8CP schemaName) {
		auto row = result.AppendRow();
		row.appendValue() = rowCount++;
		row.appendValue() = schemaName;
		return true;
	});
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaIntegrityCheck::CheckEcProfile(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb) {
	result.AppendProperty("sno", PRIMITIVETYPE_Integer);
	result.AppendProperty("type", PRIMITIVETYPE_String);
	result.AppendProperty("name", PRIMITIVETYPE_String);
	result.AppendProperty("issue", PRIMITIVETYPE_String);
	result.FreezeSchemaChanges();
	int rowCount = 1;
	return checker.CheckEcProfile([&](std::string type, std::string name, std::string issue) {
		auto row = result.AppendRow();
		row.appendValue() = rowCount++;
		row.appendValue() = type;
		row.appendValue() = name;
		row.appendValue() = issue;
		return true;
	});
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaIntegrityCheck::CheckDataSchema(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb) {
	result.AppendProperty("sno", PRIMITIVETYPE_Integer);
	result.AppendProperty("type", PRIMITIVETYPE_String);
	result.AppendProperty("name", PRIMITIVETYPE_String);
	result.FreezeSchemaChanges();
	int rowCount = 1;
	return checker.CheckDataSchema([&](std::string name, std::string type) {
		auto row = result.AppendRow();
		row.appendValue() = rowCount++;
		row.appendValue() = type;
		row.appendValue() = name;
		return true;
	});
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaIntegrityCheck::CheckDataColumns(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb) {
	result.AppendProperty("sno", PRIMITIVETYPE_Integer);
	result.AppendProperty("table", PRIMITIVETYPE_String);
	result.AppendProperty("column", PRIMITIVETYPE_String);
	result.FreezeSchemaChanges();
	int rowCount = 1;
	return checker.CheckDataColumns([&](std::string table, std::string column) {
		auto row = result.AppendRow();
		row.appendValue() = rowCount++;
		row.appendValue() = table;
		row.appendValue() = column;
		return true;
	});
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaIntegrityCheck::CheckNavClassIds(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb) {
	result.AppendProperty("sno", PRIMITIVETYPE_Integer);
	result.AppendProperty("id", PRIMITIVETYPE_String);
	result.AppendProperty("class", PRIMITIVETYPE_String);
	result.AppendProperty("property", PRIMITIVETYPE_String);
	result.AppendProperty("nav_id", PRIMITIVETYPE_String);
	result.AppendProperty("nav_classId", PRIMITIVETYPE_String);
	result.FreezeSchemaChanges();
	int rowCount = 1;
	return checker.CheckNavClassIds([&](ECInstanceId id, Utf8CP className, Utf8CP propertyName, ECInstanceId navId, ECN::ECClassId navClassId) {
		auto row = result.AppendRow();
		row.appendValue() = rowCount++;
		row.appendValue() = id.ToHexStr();
		row.appendValue() = className;
		row.appendValue() = propertyName;
		row.appendValue() = navId.ToHexStr();
		row.appendValue() = navClassId.ToHexStr();
		return true;
	});
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaIntegrityCheck::CheckNavIds(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb) {
	result.AppendProperty("sno", PRIMITIVETYPE_Integer);
	result.AppendProperty("id", PRIMITIVETYPE_String);
	result.AppendProperty("class", PRIMITIVETYPE_String);
	result.AppendProperty("property", PRIMITIVETYPE_String);
	result.AppendProperty("nav_id", PRIMITIVETYPE_String);
	result.AppendProperty("primary_class", PRIMITIVETYPE_String);
	result.FreezeSchemaChanges();
	int rowCount = 1;
	return checker.CheckNavIds([&](ECInstanceId id , Utf8CP className, Utf8CP propertyName, ECInstanceId navId, Utf8CP primaryClass) {
		auto row = result.AppendRow();
		row.appendValue() = rowCount++;
		row.appendValue() = id.ToHexStr();
		row.appendValue() = className;
		row.appendValue() = propertyName;
		row.appendValue() = navId.ToHexStr();
		row.appendValue() = primaryClass;
		return true;
	});
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaIntegrityCheck::CheckLinkTableFkClassIds(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb) {
	result.AppendProperty("sno", PRIMITIVETYPE_Integer);
	result.AppendProperty("id", PRIMITIVETYPE_String);
	result.AppendProperty("relationship", PRIMITIVETYPE_String);
	result.AppendProperty("property", PRIMITIVETYPE_String);
	result.AppendProperty("key_id", PRIMITIVETYPE_String);
	result.AppendProperty("key_classId", PRIMITIVETYPE_String);
	result.FreezeSchemaChanges();
	int rowCount = 1;
	return checker.CheckLinkTableFkClassIds([&](ECInstanceId id , Utf8CP relName, Utf8CP propertyName, ECInstanceId keyId, ECClassId keyClassId) {
		auto row = result.AppendRow();
		row.appendValue() = rowCount++;
		row.appendValue() = id.ToHexStr();
		row.appendValue() = relName;
		row.appendValue() = propertyName;
		row.appendValue() = keyId.ToHexStr();
		row.appendValue() = keyClassId.ToHexStr();
		return true;
	});
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaIntegrityCheck::CheckLinkTableFkIds(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb) {
	result.AppendProperty("sno", PRIMITIVETYPE_Integer);
	result.AppendProperty("id", PRIMITIVETYPE_String);
	result.AppendProperty("relationship", PRIMITIVETYPE_String);
	result.AppendProperty("property", PRIMITIVETYPE_String);
	result.AppendProperty("key_id", PRIMITIVETYPE_String);
	result.AppendProperty("primary_class", PRIMITIVETYPE_String);
	result.FreezeSchemaChanges();
	int rowCount = 1;
	return checker.CheckLinkTableFkIds([&](ECInstanceId id , Utf8CP relName, Utf8CP propertyName, ECInstanceId keyId, Utf8CP primaryClass) {
		auto row = result.AppendRow();
		row.appendValue() = rowCount++;
		row.appendValue() = id.ToHexStr();
		row.appendValue() = relName;
		row.appendValue() = propertyName;
		row.appendValue() = keyId.ToHexStr();
		row.appendValue() = primaryClass;
		return true;
	});
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaIntegrityCheck::CheckClassIds(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb) {
	result.AppendProperty("sno", PRIMITIVETYPE_Integer);
	result.AppendProperty("class", PRIMITIVETYPE_String);
	result.AppendProperty("id", PRIMITIVETYPE_String);
	result.AppendProperty("class_id", PRIMITIVETYPE_String);
	result.AppendProperty("type", PRIMITIVETYPE_String);
	result.FreezeSchemaChanges();
	int rowCount = 1;
	return checker.CheckClassIds([&](Utf8CP name, ECInstanceId id, ECN::ECClassId classId, Utf8CP type) {
		auto row = result.AppendRow();
		row.appendValue() = rowCount++;
		row.appendValue() = name;
		row.appendValue() = id.ToHexStr();
		row.appendValue() = classId.ToHexStr();
		row.appendValue() = type;
		return true;
	});
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaIntegrityCheck::CheckMissingChildRows(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb) {
	result.AppendProperty("sno", PRIMITIVETYPE_Integer);
	result.AppendProperty("class", PRIMITIVETYPE_String);
	result.AppendProperty("id", PRIMITIVETYPE_String);
	result.AppendProperty("class_id", PRIMITIVETYPE_String);
	result.AppendProperty("MissingRowInTables", PRIMITIVETYPE_String);
	result.FreezeSchemaChanges();
	int rowCount = 1;
	return checker.CheckMissingChildRows([&](Utf8CP name, ECInstanceId id, ECN::ECClassId classId, Utf8CP type) {
		auto row = result.AppendRow();
		row.appendValue() = rowCount++;
		row.appendValue() = name;
		row.appendValue() = id.ToHexStr();
		row.appendValue() = classId.ToHexStr();
		row.appendValue() = type;
		return true;
	});
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaIntegrityCheck::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const& options) {
	return BE_SQLITE_READONLY;
}


//=======================================================================================
// PurgeOrphanedRelationships
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaPurgeOrphanRelationships::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, const PragmaVal& val, PragmaManager::OptionsMap const& options)
	{
	if (!isExperimentalFeatureAllowed(ecdb, options))
		{
		ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0729, "'PRAGMA %s' is an experimental feature and is disabled by default.", GetName().c_str());
		return BE_SQLITE_ERROR;
		}

	rowSet = std::make_unique<StaticPragmaResult>(ecdb);
	rowSet->FreezeSchemaChanges();

	std::vector<ECClassId> rootRels;
	if (const auto rc = IntegrityChecker(ecdb).GetRootLinkTableRelationships(rootRels); rc != BE_SQLITE_OK)
		return rc;

	for (const auto& relId : rootRels)
		{
		const auto classCP = ecdb.Schemas().GetClass(relId);
		if (classCP == nullptr)
			{
			ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0730, "Failed to find class with id '%s'.", relId.ToHexStr().c_str());
			return BE_SQLITE_ERROR;
			}

        const auto relCP = classCP->GetRelationshipClassCP();
		const auto relName = relCP->GetECSqlName().c_str();
		const auto sourceClassName = relCP->GetSource().GetConstraintClasses().front()->GetECSqlName().c_str();
		const auto targetClassName = relCP->GetTarget().GetConstraintClasses().front()->GetECSqlName().c_str();

		const auto ecSqlQuery = R"sql(
			delete from %s where ECInstanceId in
				(select r.ECInstanceId from %s r left join %s s on r.SourceECInstanceId = s.ECInstanceId where s.ECInstanceId is null
					union
				select r.ECInstanceId from %s r left join %s t on r.TargetECInstanceId = t.ECInstanceId where t.ECInstanceId is null)
			)sql";

		ECSqlStatement stmt;
		if (ECSqlStatus::Success != stmt.Prepare(ecdb, SqlPrintfString(ecSqlQuery, relName, relName, sourceClassName, relName, targetClassName).GetUtf8CP()))
			{
			ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0731, "failed to prepared ecsql for nav prop integrity check");
			return BE_SQLITE_ERROR;
			}
		if (stmt.Step() == BE_SQLITE_ERROR)
			return BE_SQLITE_ERROR;
		}
	return BE_SQLITE_OK;
	}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaPurgeOrphanRelationships::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const& options)
	{
	if (!isExperimentalFeatureAllowed(ecdb, options))
		{
		ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0732, "'PRAGMA %s' is an experimental feature and is disabled by default.", GetName().c_str());
		return BE_SQLITE_ERROR;
		}

	ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0733, "PRAGMA %s does not accept assignment arguments.", GetName().c_str());
	return BE_SQLITE_ERROR;
	}

//=======================================================================================
// PragmaDbList
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaDbList::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val, PragmaManager::OptionsMap const& options) {
	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	result->AppendProperty("sno", PRIMITIVETYPE_Integer);
	result->AppendProperty("alias", PRIMITIVETYPE_String);
	result->AppendProperty("fileName", PRIMITIVETYPE_String);
	result->AppendProperty("profile", PRIMITIVETYPE_String);
	result->FreezeSchemaChanges();
    const auto dbs = ecdb.GetAttachedDbs();
	int i = 0;
	for (auto& db : dbs) {
		auto row = result->AppendRow();
		row.appendValue() = i++;
		row.appendValue() = db.m_alias;
		row.appendValue() = db.m_fileName;
		row.appendValue() = ecdb.Schemas().GetDispatcher().ExistsManager(db.m_alias) ? "ECDb" : "SQLite";
	}

	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaDbList::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const& options) {
	ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0552, "PRAGMA %s is readonly.", GetName().c_str());
	rowSet = std::make_unique<StaticPragmaResult>(ecdb);
	rowSet->FreezeSchemaChanges();
	return BE_SQLITE_READONLY;
}


//=======================================================================================
// PragmaCheckECSqlWriteValues
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaCheckECSqlWriteValues::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const& options)   {
	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	result->AppendProperty("validate_ecsql_writes", PRIMITIVETYPE_Boolean);
	result->FreezeSchemaChanges();
	auto row = result->AppendRow();
	row.appendValue() = ecdb.GetImpl().GetECSqlConfig().IsWriteValueValidationEnabled();
	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaCheckECSqlWriteValues::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val, PragmaManager::OptionsMap const& options) {
	if (val.IsBool())
		ecdb.GetImpl().GetECSqlConfig().SetWriteValueValidation(val.GetBool());

	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	result->AppendProperty("validate_ecsql_writes", PRIMITIVETYPE_Boolean);
	result->FreezeSchemaChanges();
	auto row = result->AppendRow();
	row.appendValue() = ecdb.GetImpl().GetECSqlConfig().IsWriteValueValidationEnabled();
	rowSet = std::move(result);
	return BE_SQLITE_OK;
}


END_BENTLEY_SQLITE_EC_NAMESPACE

