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
DbResult PragmaChecksum::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val) {
	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	result->AppendProperty("sha3_256", PRIMITIVETYPE_String);
	result->FreezeSchemaChanges();

	if (!val.IsName() && !val.IsString()) {
		ecdb.GetImpl().Issues().Report(
			IssueSeverity::Error,
			IssueCategory::BusinessProperties,
			IssueType::ECSQL,
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
		"Unable checksum val '%s'. Valid values are ecdb_schema|ecdb_map|sqlite_schema", val.GetString().c_str());

	rowSet = std::move(result);
	return BE_SQLITE_ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaChecksum::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) {
	ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "PRAGMA %s is readonly.", GetName().c_str());
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
DbResult PragmaECDbVersion::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) {
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
DbResult PragmaECDbVersion::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) {
	ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "PRAGMA %s is readonly.", GetName().c_str());
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
DbResult DisqualifyTypeIndex::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, ECClassCR cls) {
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
DbResult DisqualifyTypeIndex::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val, ECClassCR cls) {
	if (!val.IsBool() && !val.IsInteger()) {
		ecdb.GetImpl().Issues().ReportV(
			IssueSeverity::Error,
			IssueCategory::BusinessProperties,
			IssueType::ECSQL, "PRAGMA %s, expect bool or integer value", GetName().c_str());
		return BE_SQLITE_ERROR;
	}
	auto mapInfo = ecdb.Schemas().GetClassMapStrategy(cls.GetSchema().GetName(), cls.GetName());
	if (mapInfo.IsNotMapped() || mapInfo.IsEmpty()) {
		ecdb.GetImpl().Issues().ReportV(
			IssueSeverity::Error,
			IssueCategory::BusinessProperties,
			IssueType::ECSQL, "PRAGMA %s, is not invoked on class %s which is not mapped or exist.", GetName().c_str(), cls.GetFullName());
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
DbResult PragmaExplainQuery::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val) {
	if (!val.IsString()) {
		ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "PRAGMA %s expect a ECSQL query as string argument.", GetName().c_str());
		return BE_SQLITE_ERROR;
	}
	ECSqlStatement stmt;
	if (ECSqlStatus::Success != stmt.Prepare(ecdb, val.GetString().c_str())) {
		ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "PRAGMA %s failed to prepare ECSQL query.", GetName().c_str());
		return BE_SQLITE_ERROR;
	}
	Statement sqlStmt;
	if (BE_SQLITE_OK != sqlStmt.Prepare(ecdb, SqlPrintfString("explain query plan %s", stmt.GetNativeSql()))){
		ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "PRAGMA %s failed to explain native sql.", GetName().c_str());
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
DbResult PragmaExplainQuery::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) {
	ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "PRAGMA %s is readonly.", GetName().c_str());
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
// PragmaECDbValidation
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaECDbValidation::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) {
	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	result->AppendProperty("check", PRIMITIVETYPE_String);
	result->AppendProperty("result", PRIMITIVETYPE_String);
	result->FreezeSchemaChanges();

	auto checkResults = ECDbValidationChecks::PerformAllChecks(ecdb);

	for(auto& checkResult: checkResults) {
		auto row = result->AppendRow();
		row.appendValue() = checkResult.checkName;
		row.appendValue() = checkResult.status;
	}

	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaECDbValidation::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) {
	return BE_SQLITE_READONLY;
}

//=======================================================================================
// PragmaECDbClassIdValidation
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaECDbClassIdValidation::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) {
	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	result->AppendProperty("result", PRIMITIVETYPE_String);
	result->AppendProperty("details", PRIMITIVETYPE_String);
	result->FreezeSchemaChanges();

	auto checkResults = ECDbValidationChecks::ClassIdCheck(ecdb);

	for(auto& checkResult: checkResults) {
		auto row = result->AppendRow();
		row.appendValue() = checkResult.status;
		row.appendValue() = checkResult.details;
	}

	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaECDbClassIdValidation::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) {
	return BE_SQLITE_READONLY;
}


//=======================================================================================
// PragmaECDbClassIdValidation
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaECDbNavPropIdValidation::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) {
	auto result = std::make_unique<StaticPragmaResult>(ecdb);
	result->AppendProperty("result", PRIMITIVETYPE_String);
	result->AppendProperty("details", PRIMITIVETYPE_String);
	result->FreezeSchemaChanges();

	auto checkResults = ECDbValidationChecks::NavigationPropertyIdCheck(ecdb);

	for(auto& checkResult: checkResults) {
		auto row = result->AppendRow();
		row.appendValue() = checkResult.status;
		row.appendValue() = checkResult.details;
	}

	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaECDbNavPropIdValidation::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) {
	return BE_SQLITE_READONLY;
}



END_BENTLEY_SQLITE_EC_NAMESPACE

