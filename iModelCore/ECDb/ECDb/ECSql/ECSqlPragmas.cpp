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
	result->AppendProperty("sha1", PRIMITIVETYPE_String);
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

    const Utf8String kEcSchema = "ec_schema";
	const Utf8String kEcMap = "ec_map";
	const Utf8String kDbSchema = "db_schema";

    if (kEcSchema.EqualsIAscii(val.GetString())) {
        SHA1 sha1;
		if (DataSHA1::ComputeSchemaHash(sha1, ecdb) != BE_SQLITE_OK) {
			ecdb.GetImpl().Issues().Report(
				IssueSeverity::Error,
				IssueCategory::BusinessProperties,
				IssueType::ECSQL,
				"Unable to compute sha1 checksum for ec schemas.");

			rowSet = std::move(result);
            return BE_SQLITE_ERROR;
        }
		auto row = result->AppendRow();
        row.appendValue() = sha1.GetHashString();
		rowSet = std::move(result);
		return BE_SQLITE_OK;
    }
    if (kEcMap.EqualsIAscii(val.GetString())) {
        SHA1 sha1;
		if (DataSHA1::ComputeMapHash(sha1, ecdb) != BE_SQLITE_OK) {
			ecdb.GetImpl().Issues().Report(
				IssueSeverity::Error,
				IssueCategory::BusinessProperties,
				IssueType::ECSQL,
				"Unable to compute sha1 checksum for ec map.");

			rowSet = std::move(result);
            return BE_SQLITE_ERROR;
        }
		auto row = result->AppendRow();
        row.appendValue() = sha1.GetHashString();
		rowSet = std::move(result);
		return BE_SQLITE_OK;
    }
    if (kDbSchema.EqualsIAscii(val.GetString())) {
        SHA1 sha1;
		if (DataSHA1::ComputeDbSchemaHash(sha1, ecdb) != BE_SQLITE_OK) {
			ecdb.GetImpl().Issues().Report(
				IssueSeverity::Error,
				IssueCategory::BusinessProperties,
				IssueType::ECSQL,
				"Unable to compute sha1 checksum for db schema.");

			rowSet = std::move(result);
            return BE_SQLITE_ERROR;
        }
		auto row = result->AppendRow();
        row.appendValue() = sha1.GetHashString();
		rowSet = std::move(result);
		return BE_SQLITE_OK;
    }

	ecdb.GetImpl().Issues().ReportV(
		IssueSeverity::Error,
		IssueCategory::BusinessProperties,
		IssueType::ECSQL,
		"Unable checksum val '%s'. Valid values are ec_schema|ec_map|db_schema", val.GetString().c_str());

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
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void DataSHA1::AppendRow (SHA1& hash, Statement& stmt, bool includeNulls) {
	const uint32_t kNullVal = 0x7fffffff;
	double valFloat;
	int64_t valInt;
	const auto columnCount = stmt.GetColumnCount();
	for(auto i= 0; i< columnCount; ++i ) {
		switch (stmt.GetColumnType(i)) {
			case DbValueType::BlobVal: hash.Add(stmt.GetValueBlob(i), (size_t)stmt.GetColumnBytes(i)); break;
			case DbValueType::FloatVal: hash.Add(static_cast<void const*>(&(valFloat = stmt.GetValueDouble(i))), sizeof(valFloat)); break;
			case DbValueType::IntegerVal: hash.Add(static_cast<void const*>(&(valInt = stmt.GetValueInt64(i))), sizeof(valInt)); break;
			case DbValueType::TextVal: hash.Add(static_cast<void const*>(stmt.GetValueText(i)), (size_t)stmt.GetColumnBytes(i)); break;
			case DbValueType::NullVal:
				if (includeNulls) {
					hash.Add(static_cast<void const*>(&kNullVal), sizeof(kNullVal));
				}
				break;
			}
	}
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult DataSHA1::AppendRows(SHA1& hash, Statement& stmt, bool includeNulls) {
	DbResult rc;
	while((rc=stmt.Step()) == BE_SQLITE_ROW) {
		AppendRow(hash, stmt, includeNulls);
	}
    return rc == BE_SQLITE_DONE ? BE_SQLITE_OK : rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult DataSHA1::ComputeSchemaHash(SHA1& hash, DbCR db, Utf8CP dbAlias) {
	auto tables = std::vector<std::string> {
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

	};
	for (auto& table: tables) {
		if (TableExists(db, table.c_str(), dbAlias)) {
			auto rc = ComputeTableHash(hash, db, table.c_str(), dbAlias, false, false);
			if (rc != BE_SQLITE_OK) {
				return rc;
			}
		}
	}
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult DataSHA1::ComputeMapHash(SHA1& hash, DbCR db, Utf8CP dbAlias) {
	auto tables = std::vector<std::string> {
		"ec_PropertyPath",
		"ec_ClassMap",
		"ec_Table",
		"ec_Column",
		"ec_Index",
		"ec_IndexColumn",
		"ec_PropertyMap",
	};
	for (auto& table: tables) {
		if (TableExists(db, table.c_str(), dbAlias)) {
			auto rc = ComputeTableHash(hash, db, table.c_str(), dbAlias, false, false);
			if (rc != BE_SQLITE_OK) {
				return rc;
			}
		}
	}
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult DataSHA1::ComputeDbSchemaHash(SHA1& hash, DbCR db, Utf8CP dbAlias) {
	Statement stmt;
	auto rc = stmt.Prepare(db, SqlPrintfString(
		"select name||':'||iif(sql is null,'',sql) er from [%s].sqlite_master order by er", dbAlias));

	if (rc != BE_SQLITE_OK) {
		return rc;
	}
	return AppendRows(hash,stmt, false);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------

bool DataSHA1::TableExists(DbCR db, Utf8CP tableName, Utf8CP dbAlias) {
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
DbResult DataSHA1::ComputeTableHash(SHA1& hash, DbCR db, Utf8CP tableName, Utf8CP dbAlias, bool includeNulls, bool sortColumns) {
	if (!db.IsDbOpen()) {
		return BE_SQLITE_ERROR_NOTOPEN;
	}

	if (!db.TableExists(tableName)) {
		return BE_SQLITE_NOTFOUND;
	}
	auto join = [&](std::vector<std::string> const& list, std::string delimiter=",") -> std::string {
		return std::accumulate(
			std::next(list.begin()),
			std::end(list),
			std::string{list.front()},
			[&](std::string const& acc, const std::string& piece) {
				return acc + delimiter + piece;
			}
		);
	};
	auto getColumns = [&]() -> std::vector<std::string> {
		Statement stmt;
		std::vector<std::string> columnNames;
		const auto sql = std::string{SqlPrintfString("pragma %s.table_info(%s)", dbAlias, tableName).GetUtf8CP()};
		if (BE_SQLITE_OK == stmt.Prepare(db, sql.c_str())) {
			while(stmt.Step() == BE_SQLITE_ROW) {
				columnNames.push_back(stmt.GetValueText(1));
			}
		}
		if (sortColumns) {
			std::sort(columnNames.begin(), columnNames.end());
		}
		return std::move(columnNames);
	};

	const auto sql = std::string {
		SqlPrintfString(
			"select %s from [%s].[%s] order by [rowid]",
			join(getColumns()).c_str(),
			dbAlias,
			tableName).GetUtf8CP()
	};

	Statement stmt;
	auto rc = stmt.Prepare(db, sql.c_str());
	if (rc != BE_SQLITE_OK) {
		return rc;
	}
	return AppendRows(hash, stmt, includeNulls);
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

