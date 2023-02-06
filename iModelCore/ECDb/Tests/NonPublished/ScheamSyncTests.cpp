/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"
#include "ECDbPublishedTests.h"
#include <numeric>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
BEGIN_ECDBUNITTESTS_NAMESPACE


struct SchemaSyncTestFixture : public ECDbTestFixture {


	static std::unique_ptr<ECDb> CreateECDb(Utf8CP asFileName) {
        auto fileName = BuildECDbPath(asFileName);
        if (fileName.DoesPathExist()) {
			if (fileName.BeDeleteFile() != BeFileNameStatus::Success) {
                throw std::runtime_error("unable to delete file");
            }
        }
        auto ecdb = std::make_unique<ECDb>();
		if (BE_SQLITE_OK != ecdb->CreateNewDb(fileName)) {
            return nullptr;
        }
        ecdb->SaveChanges();
        return std::move(ecdb);
    }

	static std::unique_ptr<ECDb> CopyAs(ECDbR fromDb, Utf8CP asFileName) {
        fromDb.SaveChanges();
        auto fileName = BuildECDbPath(asFileName);
        if (fileName.DoesPathExist()) {
			if (fileName.BeDeleteFile() != BeFileNameStatus::Success) {
                throw std::runtime_error("unable to delete file");
            }
        }
        BeFileName::BeCopyFile( BeFileName(fromDb.GetDbFileName(), true), fileName);
        auto ecdb = std::make_unique<ECDb>();
        if (BE_SQLITE_OK != ecdb->OpenBeSQLiteDb(fileName, Db::OpenParams(Db::OpenMode::ReadWrite))) {
            return nullptr;
        }
        ecdb->SaveChanges();
        return std::move(ecdb);
    }
    static SchemaImportResult ImportSchemas(ECDbR ecdb, std::vector<SchemaItem> items, SchemaManager::SchemaImportOptions opts = SchemaManager::SchemaImportOptions::None) {
        auto schemaReadContext = ECSchemaReadContext::CreateContext();
		schemaReadContext->AddSchemaLocater(ecdb.GetSchemaLocater());
		bvector<ECSchemaCP> importSchemas;
		for(auto& item: items) {
			ECSchemaPtr schema;
			ECSchema::ReadFromXmlString(schema, item.GetXmlString().c_str(), *schemaReadContext);
			if (!schema.IsValid()) {
                return SchemaImportResult::ERROR;
            }
            importSchemas.push_back(schema.get());
        }
        return ecdb.Schemas().ImportSchemas(importSchemas, opts);
    }
	static SchemaImportResult ImportSchema(ECDbR ecdb, SchemaItem item, SchemaManager::SchemaImportOptions opts = SchemaManager::SchemaImportOptions::None) {
        return ImportSchemas(ecdb, std::vector<SchemaItem>{item}, opts);
    }
	static std::unique_ptr<ECDb> OpenECDb(Utf8CP asFileNam) {
        auto ecdb = std::make_unique<ECDb>();
        if (BE_SQLITE_OK != ecdb->OpenBeSQLiteDb(asFileNam, Db::OpenParams(Db::OpenMode::ReadWrite))) {
            return nullptr;
        }
        return std::move(ecdb);
    }
};

struct Sync {
    enum class SyncFlags {
        PullFromSyncDb,
        PushToSyncDb
    };

private:
    static std::string Join(std::vector<std::string> const& list, std::string delimiter = ",", std::string prefix = "", std::string postfix = "");
    static std::string ToLower(std::string const& val);
    static DbResult GetColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, std::vector<std::string>& columnNames);
    static DbResult GetPrimaryKeyColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, std::vector<std::string>& columnNames);
    static DbResult InsertOrUpdate(DbR conn, std::vector<std::string> const& tables, Utf8CP sourceDbAlias, Utf8CP targetDbAlias = "main");
    static DbResult InsertOrUpdate(DbR conn, Utf8CP tableName, Utf8CP sourceDbAlias, Utf8CP targetDbAlias = "main");
public:
    static DbResult InsertOrUpdate(ECDbR conn, Utf8CP syncDbPath, SyncFlags flag, std::function<bool(const Utf8String&)> tableFilter, bool useNestedSafePoint = false);
    static DbResult InsertOrUpdate(Utf8CP sourceDb, Utf8CP targetDb, std::function<bool(const Utf8String&)> tableFilter, bool useNestedSafePoint = false);
};


struct AttachDbScope {
	private:
        std::string m_alias;
        ECDbR m_ecdb;
	public:
		AttachDbScope(ECDbR ecdb, Utf8CP fileName, Utf8CP alias): m_alias(alias), m_ecdb(ecdb) {
			ecdb.SaveChanges();
			if (ecdb.AttachDb(fileName, alias) != BE_SQLITE_OK) {
				throw std::runtime_error("unable to attach db");
			}
        }
		~AttachDbScope() {
			m_ecdb.SaveChanges();
            m_ecdb.DetachDb(m_alias.c_str());
        }
};

TEST_F(SchemaSyncTestFixture, test) {
    auto syncDb = CreateECDb("sync.db");
    const auto synDbFile = std::string{syncDb->GetDbFileName()};
    syncDb = nullptr;

	auto masterDb = CreateECDb("master.db");
    auto clientDb = CreateECDb("client.db");

    auto tableFilterCallback = [](Utf8StringCR tableName) {
        return tableName.StartsWith("ec_");
    };

    auto pushChangesToSyncDb = [&]( ECDbR ecdb) {
        printf("Push To SyncDb\r\n");
        ASSERT_EQ(BE_SQLITE_OK, Sync::InsertOrUpdate(ecdb, synDbFile.c_str(), Sync::SyncFlags::PushToSyncDb, tableFilterCallback, /* useNestedSafePoint = */ true));
    };
    auto pullChangesFromSyncDb = [&](ECDbR ecdb) {
		printf("PullFrom SyncDb\r\n");
        ASSERT_EQ(BE_SQLITE_OK, Sync::InsertOrUpdate(ecdb, synDbFile.c_str(), Sync::SyncFlags::PullFromSyncDb, tableFilterCallback, /* useNestedSafePoint = */ true));
        ecdb.ClearECDbCache();
    };

    pushChangesToSyncDb(*masterDb);

    auto schema1 = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
		<ECSchema schemaName="TestSchema1" alias="ts1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
			<ECEntityClass typeName="Pipe1">
				<ECProperty propertyName="p1" typeName="int" />
				<ECProperty propertyName="p2" typeName="int" />
			</ECEntityClass>
		</ECSchema>)xml");


    ASSERT_EQ (SchemaImportResult::OK, ImportSchema(*masterDb, schema1));
    masterDb->SaveChanges();
    pushChangesToSyncDb(*masterDb);

	// check if sync db has schema changes
	syncDb = OpenECDb(synDbFile.c_str());
    auto pipe1 = syncDb->Schemas().GetClass("TestSchema1", "Pipe1");
    ASSERT_NE(pipe1, nullptr);
	ASSERT_EQ(pipe1->GetPropertyCount(), 2);
    syncDb = nullptr;

	// pull changes from sync db into client db and check if schema changes was there and valid
    pullChangesFromSyncDb(*clientDb);
    clientDb->ClearECDbCache();
    pipe1 = clientDb->Schemas().GetClass("TestSchema1", "Pipe1");
    ASSERT_NE(pipe1, nullptr);
	ASSERT_EQ(pipe1->GetPropertyCount(), 2);

	// add two more properties from client db and push it to sync db.
    auto schema2 = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
		<ECSchema schemaName="TestSchema1" alias="ts1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
			<ECEntityClass typeName="Pipe1">
				<ECProperty propertyName="p1" typeName="int" />
				<ECProperty propertyName="p2" typeName="int" />
				<ECProperty propertyName="p3" typeName="int" />
				<ECProperty propertyName="p4" typeName="int" />
			</ECEntityClass>
		</ECSchema>)xml");
    ASSERT_EQ (SchemaImportResult::OK, ImportSchema(*clientDb, schema2));
    clientDb->SaveChanges();
	pushChangesToSyncDb(*clientDb);

	// check if sync db has schema changes
	syncDb = OpenECDb(synDbFile.c_str());
    pipe1 = syncDb->Schemas().GetClass("TestSchema1", "Pipe1");
    ASSERT_NE(pipe1, nullptr);
	ASSERT_EQ(pipe1->GetPropertyCount(), 4);
    syncDb = nullptr;


	// pull changes from sync db into master db and check if schema changes was there and valid
    pullChangesFromSyncDb(*masterDb);
    masterDb->ClearECDbCache();
    pipe1 = masterDb->Schemas().GetClass("TestSchema1", "Pipe1");
    ASSERT_NE(pipe1, nullptr);
	ASSERT_EQ(pipe1->GetPropertyCount(), 4);

}

DbResult Sync::GetColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, std::vector<std::string>& columnNames) {
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
DbResult Sync::GetPrimaryKeyColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, std::vector<std::string>& columnNames) {
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
std::string Sync::Join(std::vector<std::string> const& list, std::string delimiter, std::string prefix, std::string postfix) {
	return prefix + std::accumulate(
		std::next(list.begin()),
		std::end(list),
		std::string{list.front()},
		[&](std::string const& acc, const std::string& piece) {
			return acc + delimiter + piece;
		}
	) + postfix;
}
std::string Sync::ToLower(std::string const& val) {
	std::string out;
	std::for_each(val.begin(), val.end(), [&](char const& ch) {
		out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
	});
	return out;
}
DbResult Sync::InsertOrUpdate(DbR conn, std::vector<std::string> const& tables, Utf8CP sourceDbAlias, Utf8CP targetDbAlias) {
	auto rc = conn.ExecuteSql("PRAGMA defer_foreign_keys=1");
	if (rc != BE_SQLITE_OK) {
		return rc;
	}
	for (auto& tbl : tables) {
		rc = InsertOrUpdate(conn, tbl.c_str(), sourceDbAlias, targetDbAlias);
		if (rc != BE_SQLITE_OK) {
			conn.ExecuteSql("PRAGMA defer_foreign_keys=0");
			return rc;
		}
	}
	return conn.ExecuteSql("PRAGMA defer_foreign_keys=0");
}
DbResult Sync::InsertOrUpdate(DbR conn, Utf8CP tableName, Utf8CP sourceDbAlias, Utf8CP targetDbAlias) {
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

	printf("%s: %d\n", tableName, conn.GetModifiedRowCount());
    return rc == BE_SQLITE_DONE ? BE_SQLITE_OK : rc;
}


DbResult Sync::InsertOrUpdate(ECDbR conn, Utf8CP syncDbPath, SyncFlags flag, std::function<bool(const Utf8String&)> tableFilter, bool useNestedSafePoint) {
	const auto kSyncDbAlias = "sync_db";
	const auto kMainDbAlias = "main";

	BeFileName syncDbFile(syncDbPath);
	if (!syncDbFile.DoesPathExist()) {
		return BE_SQLITE_ERROR;
	}

    AttachDbScope scope(conn, syncDbFile.GetNameUtf8().c_str(), kSyncDbAlias);
    std::unique_ptr<Savepoint> savePoint;
	if (useNestedSafePoint) {
		savePoint = std::make_unique<Savepoint>(conn, "sync-db-conn");
	}
    const auto fromAlias = flag == SyncFlags::PullFromSyncDb ? kSyncDbAlias : kMainDbAlias;
	const auto toAlias = flag == SyncFlags::PullFromSyncDb ?  kMainDbAlias: kSyncDbAlias;

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
    Statement stmt;
	const auto rc = stmt.Prepare(conn, enumTblSql.c_str());
	if (rc != BE_SQLITE_OK) {
		return rc;
	}

	while(stmt.Step() == BE_SQLITE_ROW) {
		Utf8String tbl = stmt.GetValueText(0);
		if (tableFilter(tbl)) {
			tables.push_back(tbl);
		}
	}
	return InsertOrUpdate(conn, tables, fromAlias, toAlias);
}

DbResult Sync::InsertOrUpdate(Utf8CP sourceDb, Utf8CP targetDb, std::function<bool(const Utf8String&)> tableFilter, bool useNestedSafePoint) {
	const auto kSourceDbAlias = "source_db";
	BeFileName sourceDbFile(sourceDb);
	BeFileName targetDbFile(targetDb);
	if (!sourceDbFile.DoesPathExist()) {
		return BE_SQLITE_ERROR;
	}
	if (!targetDbFile.DoesPathExist()) {
		return BE_SQLITE_ERROR;
	}
	if (sourceDbFile.EqualsI(targetDbFile)) {
		return BE_SQLITE_ERROR;
	}

	Db target;
	auto rc = target.OpenBeSQLiteDb(targetDbFile, Db::OpenParams(Db::OpenMode::ReadWrite));
	if (rc != BE_SQLITE_OK) {
		return rc;
	}

	rc = target.AttachDb(sourceDbFile.GetNameUtf8().c_str(), kSourceDbAlias);
	if (rc != BE_SQLITE_OK) {
		return rc;
	}
	std::unique_ptr<Savepoint> savePoint;
	if (useNestedSafePoint) {
		savePoint = std::make_unique<Savepoint>(target, "sync-db");
	}


	const auto enumTblSql = std::string{
		SqlPrintfString(R"sql(
			SELECT [target].[tbl_name]
			FROM   [%s].[sqlite_master] [target], [%s].[sqlite_master] [source]
			WHERE  [target].[type] = 'table'
					AND [target].[rootpage] != 0
					AND [target].[tbl_name] NOT LIKE 'sqlite_%'
					AND [source].[type] = 'table'
					AND [source].[rootpage] != 0
					AND [source].[tbl_name] NOT LIKE 'sqlite_%'
					AND [source].[tbl_name] = [target].[tbl_name]
			ORDER  BY [target].[tbl_name];
	)sql", "main" , kSourceDbAlias).GetUtf8CP()};

	std::vector<std::string> tables;
	Statement stmt;
	rc = stmt.Prepare(target, enumTblSql.c_str());
	if (rc != BE_SQLITE_OK) {
		return rc;
	}
	while(stmt.Step() != BE_SQLITE_ROW) {
		Utf8String tbl = stmt.GetValueText(0);
		if (tableFilter(tbl)) {
			tables.push_back(tbl);
		}
	}
	return InsertOrUpdate(target, tables, kSourceDbAlias, "main");
}

END_ECDBUNITTESTS_NAMESPACE
