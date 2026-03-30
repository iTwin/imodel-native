/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// Phase 2 vtab module cursor implementations
//=======================================================================================

//=======================================================================================
// PragmaECDbVersionModule
//=======================================================================================
DbResult PragmaECDbVersionModule::VTab::Cursor::Filter(int, const char*, int, BeSQLite::DbValue*) {
	SetColumnText(0, ECDb::CurrentECDbProfileVersion().ToString().c_str());
	SetColumnText(1, GetECDb().GetECDbProfileVersion().ToString().c_str());
	m_rowId = 1;
	m_eof = false;
	return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaECSqlVersionModule
//=======================================================================================
DbResult PragmaECSqlVersionModule::VTab::Cursor::Filter(int, const char*, int, BeSQLite::DbValue*) {
	SetColumnText(0, ECDb::GetECSqlVersion().ToString().c_str());
	m_rowId = 1;
	m_eof = false;
	return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaExperimentalFeaturesModule
//=======================================================================================
DbResult PragmaExperimentalFeaturesModule::VTab::BestIndex(IndexInfo& info) {
	int nArg = 0;
	for (int i = 0; i < info.GetConstraintCount(); i++) {
		auto* c = info.GetConstraint(i);
		if (!c->IsUsable()) continue;
		if (c->GetColumn() == 1 && c->GetOp() == IndexInfo::Operator::EQ) {
			info.GetConstraintUsage(i)->SetArgvIndex(++nArg);
			info.GetConstraintUsage(i)->SetOmit(true);
		}
	}
	info.SetEstimatedCost(1.0);
	return BE_SQLITE_OK;
}
DbResult PragmaExperimentalFeaturesModule::VTab::Cursor::Filter(int, const char*, int argc, BeSQLite::DbValue* argv) {
	if (argc > 0)
		GetECDb().GetImpl().GetECSqlConfig().SetExperimentalFeaturesEnabled(argv[0].GetValueInt64() != 0);
	SetColumnBool(0, GetECDb().GetImpl().GetECSqlConfig().GetExperimentalFeaturesEnabled());
	m_rowId = 1;
	m_eof = false;
	return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaCheckECSqlWriteValuesModule
//=======================================================================================
DbResult PragmaCheckECSqlWriteValuesModule::VTab::BestIndex(IndexInfo& info) {
	int nArg = 0;
	for (int i = 0; i < info.GetConstraintCount(); i++) {
		auto* c = info.GetConstraint(i);
		if (!c->IsUsable()) continue;
		if (c->GetColumn() == 1 && c->GetOp() == IndexInfo::Operator::EQ) {
			info.GetConstraintUsage(i)->SetArgvIndex(++nArg);
			info.GetConstraintUsage(i)->SetOmit(true);
		}
	}
	info.SetEstimatedCost(1.0);
	return BE_SQLITE_OK;
}
DbResult PragmaCheckECSqlWriteValuesModule::VTab::Cursor::Filter(int, const char*, int argc, BeSQLite::DbValue* argv) {
	if (argc > 0)
		GetECDb().GetImpl().GetECSqlConfig().SetWriteValueValidation(argv[0].GetValueInt64() != 0);
	SetColumnBool(0, GetECDb().GetImpl().GetECSqlConfig().IsWriteValueValidationEnabled());
	m_rowId = 1;
	m_eof = false;
	return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaChecksumModule
//=======================================================================================
DbResult PragmaChecksumModule::VTab::BestIndex(IndexInfo& info) {
	int nArg = 0;
	for (int i = 0; i < info.GetConstraintCount(); i++) {
		auto* c = info.GetConstraint(i);
		if (!c->IsUsable()) continue;
		if (c->GetColumn() == 1 && c->GetOp() == IndexInfo::Operator::EQ) {
			info.GetConstraintUsage(i)->SetArgvIndex(++nArg);
			info.GetConstraintUsage(i)->SetOmit(true);
		}
	}
	info.SetEstimatedCost(1.0);
	return BE_SQLITE_OK;
}
DbResult PragmaChecksumModule::VTab::Cursor::Filter(int, const char*, int argc, BeSQLite::DbValue* argv) {
	m_eof = true;
	if (argc == 0) return BE_SQLITE_ERROR;
	Utf8CP sourceStr = argv[0].GetValueText();
	if (sourceStr == nullptr) return BE_SQLITE_ERROR;
	Utf8String src(sourceStr);
	const Utf8String kECDbSchema = "ecdb_schema";
	const Utf8String kECDbMap = "ecdb_map";
	const Utf8String kSQLiteSchema = "sqlite_schema";
	SHA3Helper::SourceType sourceType;
	if (kECDbSchema.EqualsIAscii(src)) sourceType = SHA3Helper::SourceType::ECDB_SCHEMA;
	else if (kECDbMap.EqualsIAscii(src)) sourceType = SHA3Helper::SourceType::ECDB_MAP;
	else if (kSQLiteSchema.EqualsIAscii(src)) sourceType = SHA3Helper::SourceType::SQLITE_SCHEMA;
	else return BE_SQLITE_ERROR;
	Utf8String sha3;
	if (SHA3Helper::ComputeHash(sha3, GetECDb(), sourceType) != BE_SQLITE_OK) return BE_SQLITE_ERROR;
	SetColumnText(0, sha3.c_str());
	m_rowId = 1;
	m_eof = false;
	return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaParseTreeModule
//=======================================================================================
DbResult PragmaParseTreeModule::VTab::BestIndex(IndexInfo& info) {
	int nArg = 0;
	for (int i = 0; i < info.GetConstraintCount(); i++) {
		auto* c = info.GetConstraint(i);
		if (!c->IsUsable()) continue;
		if (c->GetColumn() == 1 && c->GetOp() == IndexInfo::Operator::EQ) {
			info.GetConstraintUsage(i)->SetArgvIndex(++nArg);
			info.GetConstraintUsage(i)->SetOmit(true);
		}
	}
	info.SetEstimatedCost(1.0);
	return BE_SQLITE_OK;
}
DbResult PragmaParseTreeModule::VTab::Cursor::Filter(int, const char*, int argc, BeSQLite::DbValue* argv) {
	m_eof = true;
	if (argc == 0) return BE_SQLITE_ERROR;
	Utf8CP ecsql = argv[0].GetValueText();
	if (ecsql == nullptr) return BE_SQLITE_ERROR;
	BeJsDocument out;
	ECSqlParseTreeFormatter::ECSqlToJson(out, GetECDb(), ecsql);
	SetColumnText(0, out.Stringify().c_str());
	m_rowId = 1;
	m_eof = false;
	return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaSqliteSqlModule
//=======================================================================================
DbResult PragmaSqliteSqlModule::VTab::BestIndex(IndexInfo& info) {
	int nArg = 0;
	for (int i = 0; i < info.GetConstraintCount(); i++) {
		auto* c = info.GetConstraint(i);
		if (!c->IsUsable()) continue;
		if (c->GetColumn() == 1 && c->GetOp() == IndexInfo::Operator::EQ) {
			info.GetConstraintUsage(i)->SetArgvIndex(++nArg);
			info.GetConstraintUsage(i)->SetOmit(true);
		}
	}
	info.SetEstimatedCost(1.0);
	return BE_SQLITE_OK;
}
DbResult PragmaSqliteSqlModule::VTab::Cursor::Filter(int, const char*, int argc, BeSQLite::DbValue* argv) {
	m_eof = true;
	if (argc == 0) return BE_SQLITE_ERROR;
	Utf8CP ecsql = argv[0].GetValueText();
	if (ecsql == nullptr || ecsql[0] == '\0') return BE_SQLITE_ERROR;
	ECSqlStatement stmt;
	if (ECSqlStatus::Success != stmt.Prepare(GetECDb(), ecsql)) return BE_SQLITE_ERROR;
	SetColumnText(0, stmt.GetNativeSql());
	m_rowId = 1;
	m_eof = false;
	return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaExplainQueryModule
//=======================================================================================
DbResult PragmaExplainQueryModule::VTab::BestIndex(IndexInfo& info) {
	int nArg = 0;
	for (int i = 0; i < info.GetConstraintCount(); i++) {
		auto* c = info.GetConstraint(i);
		if (!c->IsUsable()) continue;
		if (c->GetColumn() == 4 && c->GetOp() == IndexInfo::Operator::EQ) {
			info.GetConstraintUsage(i)->SetArgvIndex(++nArg);
			info.GetConstraintUsage(i)->SetOmit(true);
		}
	}
	info.SetEstimatedCost(1.0);
	return BE_SQLITE_OK;
}
DbResult PragmaExplainQueryModule::VTab::Cursor::Filter(int, const char*, int argc, BeSQLite::DbValue* argv) {
	m_rows.clear();
	m_rowIdx = 0;
	m_eof = true;
	m_currentRow.clear();
	if (argc == 0) return BE_SQLITE_OK;
	Utf8CP ecsql = argv[0].GetValueText();
	if (ecsql == nullptr) return BE_SQLITE_OK;
	ECSqlStatement stmt;
	if (ECSqlStatus::Success != stmt.Prepare(GetECDb(), ecsql)) return BE_SQLITE_ERROR;
	Statement sqlStmt;
	if (BE_SQLITE_OK != sqlStmt.Prepare(GetECDb(), SqlPrintfString("explain query plan %s", stmt.GetNativeSql())))
		return BE_SQLITE_ERROR;
	while (sqlStmt.Step() == BE_SQLITE_ROW) {
		std::vector<PragmaColumnValue> row(4);
		for (int i = 0; i < sqlStmt.GetColumnCount() && i < 4; ++i) {
			auto type = sqlStmt.GetColumnType(i);
			if (type == DbValueType::TextVal)
				row[i] = PragmaColumnValue(sqlStmt.GetValueText(i));
			else if (type == DbValueType::IntegerVal)
				row[i] = PragmaColumnValue(sqlStmt.GetValueInt64(i));
			else if (type == DbValueType::FloatVal)
				row[i] = PragmaColumnValue(sqlStmt.GetValueDouble(i));
		}
		m_rows.push_back(std::move(row));
	}
	if (!m_rows.empty()) {
		m_currentRow = m_rows[0];
		m_rowId = 1;
		m_eof = false;
	}
	return BE_SQLITE_OK;
}
DbResult PragmaExplainQueryModule::VTab::Cursor::Next() {
	++m_rowIdx;
	if (m_rowIdx >= m_rows.size()) {
		m_eof = true;
	} else {
		m_currentRow = m_rows[m_rowIdx];
		m_rowId = (int64_t)(m_rowIdx + 1);
	}
	return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaDbListModule
//=======================================================================================
DbResult PragmaDbListModule::VTab::Cursor::Filter(int, const char*, int, BeSQLite::DbValue*) {
	m_rows.clear();
	m_rowIdx = 0;
	m_eof = true;
	m_currentRow.clear();
	ECDbCR ecdb = GetECDb();
	const auto dbs = ecdb.GetAttachedDbs();
	int i = 0;
	for (auto& db : dbs) {
		std::vector<PragmaColumnValue> row(4);
		row[0] = PragmaColumnValue((int64_t)i++);
		row[1] = PragmaColumnValue(db.m_alias);
		row[2] = PragmaColumnValue(db.m_fileName);
		row[3] = PragmaColumnValue(ecdb.Schemas().GetDispatcher().ExistsManager(db.m_alias) ? "ECDb" : "SQLite");
		m_rows.push_back(std::move(row));
	}
	if (!m_rows.empty()) {
		m_currentRow = m_rows[0];
		m_rowId = 1;
		m_eof = false;
	}
	return BE_SQLITE_OK;
}
DbResult PragmaDbListModule::VTab::Cursor::Next() {
	++m_rowIdx;
	if (m_rowIdx >= m_rows.size()) {
		m_eof = true;
	} else {
		m_currentRow = m_rows[m_rowIdx];
		m_rowId = (int64_t)(m_rowIdx + 1);
	}
	return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaPurgeOrphanRelationshipsModule
//=======================================================================================
DbResult PragmaPurgeOrphanRelationshipsModule::VTab::Cursor::Filter(int, const char*, int, BeSQLite::DbValue*) {
	m_eof = true;
	ECDbCR ecdb = GetECDb();
	std::vector<ECClassId> rootRels;
	if (IntegrityChecker(ecdb).GetRootLinkTableRelationships(rootRels) != BE_SQLITE_OK)
		return BE_SQLITE_ERROR;
	for (const auto& relId : rootRels) {
		const auto classCP = ecdb.Schemas().GetClass(relId);
		if (classCP == nullptr) return BE_SQLITE_ERROR;
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
			return BE_SQLITE_ERROR;
		if (stmt.Step() == BE_SQLITE_ERROR)
			return BE_SQLITE_ERROR;
	}
	SetColumnText(0, "ok");
	m_rowId = 1;
	m_eof = false;
	return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaIntegrityCheckModule
//=======================================================================================
DbResult PragmaIntegrityCheckModule::VTab::BestIndex(IndexInfo& info) {
	int nArg = 0;
	for (int i = 0; i < info.GetConstraintCount(); i++) {
		auto* c = info.GetConstraint(i);
		if (!c->IsUsable()) continue;
		if (c->GetColumn() == 20 && c->GetOp() == IndexInfo::Operator::EQ) {
			info.GetConstraintUsage(i)->SetArgvIndex(++nArg);
			info.GetConstraintUsage(i)->SetOmit(true);
		}
	}
	info.SetEstimatedCost(1.0);
	return BE_SQLITE_OK;
}
DbResult PragmaIntegrityCheckModule::VTab::Cursor::Filter(int, const char*, int argc, BeSQLite::DbValue* argv) {
	m_rows.clear();
	m_rowIdx = 0;
	m_eof = true;
	m_currentRow.clear();
	ECDbCR ecdb = GetECDb();
	IntegrityChecker checker(ecdb);
	Utf8CP checkArg = (argc > 0) ? argv[0].GetValueText() : nullptr;
	auto checks = IntegrityChecker::Checks::All;
	if (checkArg != nullptr && *checkArg != '\0') {
		auto customCheck = IntegrityChecker::GetCheckId(checkArg);
		if (customCheck != IntegrityChecker::Checks::None)
			checks = customCheck;
	}
	int rowCount = 1;
	auto makeRow = [&]() -> std::vector<PragmaColumnValue> {
		return std::vector<PragmaColumnValue>(20);
	};
	DbResult rc = BE_SQLITE_OK;
	switch (checks) {
		case IntegrityChecker::Checks::CheckSchemaLoad:
			rc = checker.CheckSchemaLoad([&](Utf8CP schemaName) {
				auto row = makeRow();
				row[0] = PragmaColumnValue((int64_t)rowCount++);
				row[4] = PragmaColumnValue(schemaName);
				m_rows.push_back(std::move(row));
				return true;
			});
			break;
		case IntegrityChecker::Checks::CheckEcProfile:
			rc = checker.CheckEcProfile([&](std::string type, std::string name, std::string issue) {
				auto row = makeRow();
				row[0] = PragmaColumnValue((int64_t)rowCount++);
				row[5] = PragmaColumnValue(type.c_str());
				row[6] = PragmaColumnValue(name.c_str());
				row[7] = PragmaColumnValue(issue.c_str());
				m_rows.push_back(std::move(row));
				return true;
			});
			break;
		case IntegrityChecker::Checks::CheckDataSchema:
			rc = checker.CheckDataSchema([&](std::string name, std::string type) {
				auto row = makeRow();
				row[0] = PragmaColumnValue((int64_t)rowCount++);
				row[5] = PragmaColumnValue(type.c_str());
				row[6] = PragmaColumnValue(name.c_str());
				m_rows.push_back(std::move(row));
				return true;
			});
			break;
		case IntegrityChecker::Checks::CheckDataColumns:
			rc = checker.CheckDataColumns([&](std::string table, std::string column) {
				auto row = makeRow();
				row[0] = PragmaColumnValue((int64_t)rowCount++);
				row[8] = PragmaColumnValue(table.c_str());
				row[9] = PragmaColumnValue(column.c_str());
				m_rows.push_back(std::move(row));
				return true;
			});
			break;
		case IntegrityChecker::Checks::CheckNavClassIds:
			rc = checker.CheckNavClassIds([&](ECInstanceId id, Utf8CP className, Utf8CP propertyName, ECInstanceId navId, ECN::ECClassId navClassId) {
				auto row = makeRow();
				row[0] = PragmaColumnValue((int64_t)rowCount++);
				row[10] = PragmaColumnValue(id.ToHexStr().c_str());
				row[11] = PragmaColumnValue(className);
				row[12] = PragmaColumnValue(propertyName);
				row[13] = PragmaColumnValue(navId.ToHexStr().c_str());
				row[14] = PragmaColumnValue(navClassId.ToHexStr().c_str());
				m_rows.push_back(std::move(row));
				return true;
			});
			break;
		case IntegrityChecker::Checks::CheckNavIds:
			rc = checker.CheckNavIds([&](ECInstanceId id, Utf8CP className, Utf8CP propertyName, ECInstanceId navId, Utf8CP primaryClass) {
				auto row = makeRow();
				row[0] = PragmaColumnValue((int64_t)rowCount++);
				row[10] = PragmaColumnValue(id.ToHexStr().c_str());
				row[11] = PragmaColumnValue(className);
				row[12] = PragmaColumnValue(propertyName);
				row[13] = PragmaColumnValue(navId.ToHexStr().c_str());
				row[15] = PragmaColumnValue(primaryClass);
				m_rows.push_back(std::move(row));
				return true;
			});
			break;
		case IntegrityChecker::Checks::CheckLinkTableFkClassIds:
			rc = checker.CheckLinkTableFkClassIds([&](ECInstanceId id, Utf8CP relName, Utf8CP propertyName, ECInstanceId keyId, ECClassId keyClassId) {
				auto row = makeRow();
				row[0] = PragmaColumnValue((int64_t)rowCount++);
				row[10] = PragmaColumnValue(id.ToHexStr().c_str());
				row[11] = PragmaColumnValue(relName);
				row[12] = PragmaColumnValue(propertyName);
				row[16] = PragmaColumnValue(keyId.ToHexStr().c_str());
				row[17] = PragmaColumnValue(keyClassId.ToHexStr().c_str());
				m_rows.push_back(std::move(row));
				return true;
			});
			break;
		case IntegrityChecker::Checks::CheckLinkTableFkIds:
			rc = checker.CheckLinkTableFkIds([&](ECInstanceId id, Utf8CP relName, Utf8CP propertyName, ECInstanceId keyId, Utf8CP primaryClass) {
				auto row = makeRow();
				row[0] = PragmaColumnValue((int64_t)rowCount++);
				row[10] = PragmaColumnValue(id.ToHexStr().c_str());
				row[11] = PragmaColumnValue(relName);
				row[12] = PragmaColumnValue(propertyName);
				row[16] = PragmaColumnValue(keyId.ToHexStr().c_str());
				row[15] = PragmaColumnValue(primaryClass);
				m_rows.push_back(std::move(row));
				return true;
			});
			break;
		case IntegrityChecker::Checks::CheckClassIds:
			rc = checker.CheckClassIds([&](Utf8CP name, ECInstanceId id, ECN::ECClassId classId, Utf8CP type) {
				auto row = makeRow();
				row[0] = PragmaColumnValue((int64_t)rowCount++);
				row[11] = PragmaColumnValue(name);
				row[10] = PragmaColumnValue(id.ToHexStr().c_str());
				row[18] = PragmaColumnValue(classId.ToHexStr().c_str());
				row[5] = PragmaColumnValue(type);
				m_rows.push_back(std::move(row));
				return true;
			});
			break;
		case IntegrityChecker::Checks::CheckMissingChildRows:
			rc = checker.CheckMissingChildRows([&](Utf8CP name, ECInstanceId id, ECN::ECClassId classId, Utf8CP missingTables) {
				auto row = makeRow();
				row[0] = PragmaColumnValue((int64_t)rowCount++);
				row[11] = PragmaColumnValue(name);
				row[10] = PragmaColumnValue(id.ToHexStr().c_str());
				row[18] = PragmaColumnValue(classId.ToHexStr().c_str());
				row[19] = PragmaColumnValue(missingTables);
				m_rows.push_back(std::move(row));
				return true;
			});
			break;
		default:
			rc = checker.QuickCheck(IntegrityChecker::Checks::All, [&](Utf8CP checkName, bool passed, BeDuration dur) {
				auto row = makeRow();
				row[0] = PragmaColumnValue((int64_t)rowCount++);
				row[1] = PragmaColumnValue(checkName);
				row[2] = PragmaColumnValue(passed);
				row[3] = PragmaColumnValue(Utf8PrintfString("%.3f", dur.ToSeconds()).c_str());
				m_rows.push_back(std::move(row));
			});
			break;
	}
	if (rc != BE_SQLITE_OK) return rc;
	if (!m_rows.empty()) {
		m_currentRow = m_rows[0];
		m_rowId = 1;
		m_eof = false;
	}
	return BE_SQLITE_OK;
}
DbResult PragmaIntegrityCheckModule::VTab::Cursor::Next() {
	++m_rowIdx;
	if (m_rowIdx >= m_rows.size()) {
		m_eof = true;
	} else {
		m_currentRow = m_rows[m_rowIdx];
		m_rowId = (int64_t)(m_rowIdx + 1);
	}
	return BE_SQLITE_OK;
}

//=======================================================================================
// DisqualifyTypeIndex::Module
//=======================================================================================
DbResult DisqualifyTypeIndex::Module::VTab::BestIndex(IndexInfo& info) {
	int nArg = 0;
	for (int i = 0; i < info.GetConstraintCount(); i++) {
		auto* c = info.GetConstraint(i);
		if (!c->IsUsable()) continue;
		if ((c->GetColumn() == 1 || c->GetColumn() == 2) && c->GetOp() == IndexInfo::Operator::EQ) {
			info.GetConstraintUsage(i)->SetArgvIndex(++nArg);
			info.GetConstraintUsage(i)->SetOmit(true);
		}
	}
	info.SetEstimatedCost(1.0);
	return BE_SQLITE_OK;
}
DbResult DisqualifyTypeIndex::Module::VTab::Cursor::Filter(int, const char*, int argc, BeSQLite::DbValue* argv) {
	m_eof = true;
	if (argc < 1) return BE_SQLITE_ERROR;
	auto& mod = static_cast<Module&>(GetTable().GetModule());
	ECClassId classId((uint64_t)argv[0].GetValueInt64());
	if (argc >= 2) {
		bool newVal = argv[1].GetValueInt64() != 0;
		if (newVal)
			mod.m_set.insert(classId);
		else
			mod.m_set.erase(classId);
	}
	SetColumnBool(0, mod.m_set.find(classId) != mod.m_set.end());
	m_rowId = 1;
	m_eof = false;
	return BE_SQLITE_OK;
}



//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaChecksum::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val, PragmaManager::OptionsMap const& options) {
	if (!val.IsName() && !val.IsString()) {
		ecdb.GetImpl().Issues().Report(
			IssueSeverity::Error,
			IssueCategory::BusinessProperties,
			IssueType::ECSQL,
			ECDbIssueId::ECDb_0592,
			"unsupported val for checksum pragma.");
		return BE_SQLITE_ERROR;
	}

	const Utf8String kECDbSchema = "ecdb_schema";
	const Utf8String kECDbMap = "ecdb_map";
	const Utf8String kSQLiteSchema = "sqlite_schema";

	if (!kECDbSchema.EqualsIAscii(val.GetString()) &&
		!kECDbMap.EqualsIAscii(val.GetString()) &&
		!kSQLiteSchema.EqualsIAscii(val.GetString())) {
		ecdb.GetImpl().Issues().ReportV(
			IssueSeverity::Error,
			IssueCategory::BusinessProperties,
			IssueType::ECSQL,
			ECDbIssueId::ECDb_0596,
			"Unable checksum val '%s'. Valid values are ecdb_schema|ecdb_map|sqlite_schema", val.GetString().c_str());
		return BE_SQLITE_ERROR;
	}

	auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
	result->AppendProperty("sha3_256", PRIMITIVETYPE_String);
	result->FreezeSchemaChanges();
	result->PrepareQuery("SELECT sha3_256 FROM pragma_checksum(?1)");
	result->BindText(1, val.GetString().c_str());
	rowSet = std::move(result);
	return BE_SQLITE_OK;
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
    auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
    result->AppendProperty("val", PRIMITIVETYPE_String);
    result->FreezeSchemaChanges();
	result->PrepareQuery("SELECT val FROM pragma_parse_tree(?1)");
	result->BindText(1, ecsql.GetString().c_str());
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
// PragmaECDbVersion
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaECDbVersion::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const& options) {
    auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
    result->AppendProperty("current", PRIMITIVETYPE_String);
    result->AppendProperty("file", PRIMITIVETYPE_String);
    result->FreezeSchemaChanges();
	result->PrepareQuery("SELECT current,file FROM pragma_ecdb_ver()");
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
// PragmaECSqlVersion
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaECSqlVersion::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, const PragmaVal&, const PragmaManager::OptionsMap& options) {
    auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
    result->AppendProperty("ecsql_ver", PRIMITIVETYPE_String);
    result->FreezeSchemaChanges();
	result->PrepareQuery("SELECT ecsql_ver FROM pragma_ecsql_ver()");
	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaECSqlVersion::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, const PragmaVal&, const PragmaManager::OptionsMap& options) {
    ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0552, "PRAGMA %s is readonly.", GetName().c_str());
    rowSet = std::make_unique<StaticPragmaResult>(ecdb);
    rowSet->FreezeSchemaChanges();
    return BE_SQLITE_READONLY;
}

//=======================================================================================
// PragmaSqliteSql
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaSqliteSql::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, const PragmaVal& val, const PragmaManager::OptionsMap& options) {
    if (!val.IsString() || val.GetString().empty()) {
        ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0601, "PRAGMA %s expect a ECSQL query as string argument.", GetName().c_str());
        return BE_SQLITE_ERROR;
    }
    // Validate the ECSQL at prepare time so callers can detect invalid input from Prepare()
    ECSqlStatement validationStmt;
    if (ECSqlStatus::Success != validationStmt.Prepare(const_cast<ECDbR>(ecdb), val.GetString().c_str()))
        return BE_SQLITE_ERROR;
    auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
    result->AppendProperty("sqlite_sql", PRIMITIVETYPE_String);
    result->FreezeSchemaChanges();
	result->PrepareQuery("SELECT sqlite_sql FROM pragma_sqlite_sql(?1)");
	result->BindText(1, val.GetString().c_str());
	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaSqliteSql::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, const PragmaVal&, const PragmaManager::OptionsMap& options) {
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
	auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
	result->AppendProperty("disqualify_type_index", PRIMITIVETYPE_Boolean);
	result->FreezeSchemaChanges();
	result->PrepareQuery("SELECT disqualify_type_index FROM pragma_disqualify_type_index(?1)");
	result->BindInt64(1, (int64_t)cls.GetId().GetValue());
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
	auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
	result->AppendProperty("disqualify_type_index", PRIMITIVETYPE_Boolean);
	result->FreezeSchemaChanges();
	result->PrepareQuery("SELECT disqualify_type_index FROM pragma_disqualify_type_index(?1, ?2)");
	result->BindInt64(1, (int64_t)cls.GetId().GetValue());
	result->BindBool(2, val.GetBool());
	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaExplainQuery
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod — kept for legacy callers; logic moved to PragmaExplainQueryModule cursor
//---------------------------------------------------------------------------------------
DbResult PragmaExplainQuery::ToResultSet(Statement& from, StaticPragmaResult& to) {
    auto createSchema = [&]() {
        for (int i = 0; i < from.GetColumnCount(); ++i) {
            if (from.GetColumnType(i) == DbValueType::TextVal)
                to.AppendProperty(from.GetColumnName(i), PRIMITIVETYPE_String);
            else if (from.GetColumnType(i) == DbValueType::FloatVal)
                to.AppendProperty(from.GetColumnName(i), PRIMITIVETYPE_Double);
            else if (from.GetColumnType(i) == DbValueType::IntegerVal)
                to.AppendProperty(from.GetColumnName(i), PRIMITIVETYPE_Long);
            else
                return BE_SQLITE_ERROR;
        }
        to.FreezeSchemaChanges();
        return BE_SQLITE_OK;
    };
    while (from.Step() == BE_SQLITE_ROW) {
        if (to.GetColumnCount() == 0) {
            const auto rc = createSchema();
            if (rc != BE_SQLITE_OK)
                return rc;
        }
        auto row = to.AppendRow();
        for (int i = 0; i < from.GetColumnCount(); ++i) {
            if (from.GetColumnType(i) == DbValueType::TextVal)
                row.appendValue() = from.GetValueText(i);
            else if (from.GetColumnType(i) == DbValueType::FloatVal)
                row.appendValue() = from.GetValueDouble(i);
            else if (from.GetColumnType(i) == DbValueType::IntegerVal)
                row.appendValue() = from.GetValueInt64(i);
            else
                row.appendValue().SetNull();
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
	auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
	result->AppendProperty("id", PRIMITIVETYPE_Integer);
	result->AppendProperty("parent", PRIMITIVETYPE_Integer);
	result->AppendProperty("notused", PRIMITIVETYPE_Integer);
	result->AppendProperty("detail", PRIMITIVETYPE_String);
	result->FreezeSchemaChanges();
	result->PrepareQuery("SELECT id,parent,notused,detail FROM pragma_explain_query(?1)");
	result->BindText(1, val.GetString().c_str());
	rowSet = std::move(result);
	return BE_SQLITE_OK;
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
// PragmaExperimentalFeatures
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaExperimentalFeatures::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const& options) {
    auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
    result->AppendProperty("experimental_features_enabled", PRIMITIVETYPE_Boolean);
    result->FreezeSchemaChanges();
	result->PrepareQuery("SELECT experimental_features_enabled FROM pragma_experimental_features_enabled()");
	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaExperimentalFeatures::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val, PragmaManager::OptionsMap const& options) {
	auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
	result->AppendProperty("experimental_features_enabled", PRIMITIVETYPE_Boolean);
	result->FreezeSchemaChanges();
	if (val.IsBool()) {
		result->PrepareQuery("SELECT experimental_features_enabled FROM pragma_experimental_features_enabled(?1)");
		result->BindBool(1, val.GetBool());
	} else {
		result->PrepareQuery("SELECT experimental_features_enabled FROM pragma_experimental_features_enabled()");
	}
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
    auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
	result->AppendProperty("sno", PRIMITIVETYPE_Integer);
	result->AppendProperty("check_name", PRIMITIVETYPE_String);
	result->AppendProperty("result", PRIMITIVETYPE_Boolean);
	result->AppendProperty("elapsed_sec", PRIMITIVETYPE_String);
	result->AppendProperty("schema", PRIMITIVETYPE_String);
	result->AppendProperty("type", PRIMITIVETYPE_String);
	result->AppendProperty("name", PRIMITIVETYPE_String);
	result->AppendProperty("issue", PRIMITIVETYPE_String);
	result->AppendProperty("table_name", PRIMITIVETYPE_String);
	result->AppendProperty("col", PRIMITIVETYPE_String);
	result->AppendProperty("id", PRIMITIVETYPE_String);
	result->AppendProperty("class", PRIMITIVETYPE_String);
	result->AppendProperty("property", PRIMITIVETYPE_String);
	result->AppendProperty("nav_id", PRIMITIVETYPE_String);
	result->AppendProperty("nav_classId", PRIMITIVETYPE_String);
	result->AppendProperty("primary_class", PRIMITIVETYPE_String);
	result->AppendProperty("key_id", PRIMITIVETYPE_String);
	result->AppendProperty("key_classId", PRIMITIVETYPE_String);
	result->AppendProperty("class_id", PRIMITIVETYPE_String);
	result->AppendProperty("MissingRowInTables", PRIMITIVETYPE_String);
	result->FreezeSchemaChanges();
	if (v.IsString()) {
		result->PrepareQuery("SELECT sno,check_name,result,elapsed_sec,schema,type,name,issue,table_name,col,id,class,property,nav_id,nav_classId,primary_class,key_id,key_classId,class_id,MissingRowInTables FROM pragma_integrity_check(?1)");
		result->BindText(1, v.GetString().c_str());
	} else {
		result->PrepareQuery("SELECT sno,check_name,result,elapsed_sec,schema,type,name,issue,table_name,col,id,class,property,nav_id,nav_classId,primary_class,key_id,key_classId,class_id,MissingRowInTables FROM pragma_integrity_check()");
	}
	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod — stubs; logic moved to PragmaIntegrityCheckModule cursor
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaIntegrityCheck::CheckAll(IntegrityChecker&, StaticPragmaResult&, ECDbCR) { return BE_SQLITE_OK; }
DbResult PragmaIntegrityCheck::CheckSchemaLoad(IntegrityChecker&, StaticPragmaResult&, ECDbCR) { return BE_SQLITE_OK; }
DbResult PragmaIntegrityCheck::CheckEcProfile(IntegrityChecker&, StaticPragmaResult&, ECDbCR) { return BE_SQLITE_OK; }
DbResult PragmaIntegrityCheck::CheckDataSchema(IntegrityChecker&, StaticPragmaResult&, ECDbCR) { return BE_SQLITE_OK; }
DbResult PragmaIntegrityCheck::CheckDataColumns(IntegrityChecker&, StaticPragmaResult&, ECDbCR) { return BE_SQLITE_OK; }
DbResult PragmaIntegrityCheck::CheckNavClassIds(IntegrityChecker&, StaticPragmaResult&, ECDbCR) { return BE_SQLITE_OK; }
DbResult PragmaIntegrityCheck::CheckNavIds(IntegrityChecker&, StaticPragmaResult&, ECDbCR) { return BE_SQLITE_OK; }
DbResult PragmaIntegrityCheck::CheckLinkTableFkClassIds(IntegrityChecker&, StaticPragmaResult&, ECDbCR) { return BE_SQLITE_OK; }
DbResult PragmaIntegrityCheck::CheckLinkTableFkIds(IntegrityChecker&, StaticPragmaResult&, ECDbCR) { return BE_SQLITE_OK; }
DbResult PragmaIntegrityCheck::CheckClassIds(IntegrityChecker&, StaticPragmaResult&, ECDbCR) { return BE_SQLITE_OK; }
DbResult PragmaIntegrityCheck::CheckMissingChildRows(IntegrityChecker&, StaticPragmaResult&, ECDbCR) { return BE_SQLITE_OK; }

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
DbResult PragmaPurgeOrphanRelationships::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, const PragmaVal& val, PragmaManager::OptionsMap const& options) {
    if (!isExperimentalFeatureAllowed(ecdb, options)) {
        ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0729, "'PRAGMA %s' is an experimental feature and is disabled by default.", GetName().c_str());
        return BE_SQLITE_ERROR;
    }
    auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
    result->AppendProperty("status", PRIMITIVETYPE_String);
    result->FreezeSchemaChanges();
	result->PrepareQuery("SELECT status FROM pragma_purge_orphan_relationships()");
	rowSet = std::move(result);
	return BE_SQLITE_OK;
	}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaPurgeOrphanRelationships::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const& options) {
    if (!isExperimentalFeatureAllowed(ecdb, options)) {
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
    auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
	result->AppendProperty("sno", PRIMITIVETYPE_Integer);
	result->AppendProperty("alias", PRIMITIVETYPE_String);
	result->AppendProperty("fileName", PRIMITIVETYPE_String);
	result->AppendProperty("profile", PRIMITIVETYPE_String);
	result->FreezeSchemaChanges();
	result->PrepareQuery("SELECT sno,alias,fileName,profile FROM pragma_db_list()");
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
DbResult PragmaCheckECSqlWriteValues::Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const& options) {
    auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
    result->AppendProperty("validate_ecsql_writes", PRIMITIVETYPE_Boolean);
    result->FreezeSchemaChanges();
	result->PrepareQuery("SELECT validate_ecsql_writes FROM pragma_validate_ecsql_writes()");
	rowSet = std::move(result);
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PragmaCheckECSqlWriteValues::Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val, PragmaManager::OptionsMap const& options) {
	auto result = std::make_unique<PragmaVirtualTabResult>(ecdb);
	result->AppendProperty("validate_ecsql_writes", PRIMITIVETYPE_Boolean);
	result->FreezeSchemaChanges();
	if (val.IsBool()) {
		result->PrepareQuery("SELECT validate_ecsql_writes FROM pragma_validate_ecsql_writes(?1)");
		result->BindBool(1, val.GetBool());
	} else {
		result->PrepareQuery("SELECT validate_ecsql_writes FROM pragma_validate_ecsql_writes()");
	}
	rowSet = std::move(result);
	return BE_SQLITE_OK;
}


END_BENTLEY_SQLITE_EC_NAMESPACE


