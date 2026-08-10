#include "MockHubApi.h"
#include <numeric>
#include <iostream>

USING_NAMESPACE_BENTLEY_SQLITE_EC;
//***************************************************************************************
// SchemaSyncDb
//***************************************************************************************
// Default SHA  hashes
const char* SchemaSyncTestFixture::DEFAULT_SHA3_256_ECDB_SCHEMA = "44c5d675cdab562b732a90b8c0128149daaa7a2beefbcbddb576f7bf059cec33";
const char* SchemaSyncTestFixture::DEFAULT_SHA3_256_ECDB_MAP = "9c7834d13177336f0fa57105b9c1175b912b2e12e62ca2224482c0ffd9dfd337";
const char* SchemaSyncTestFixture::DEFAULT_SHA3_256_SQLITE_SCHEMA = "c4ca1cdd07de041e71f3e8d4b1942d29da89653c85276025d786688b6f576443";
const char* SchemaSyncTestFixture::DEFAULT_SHA3_256_CHANNEL_SQLITE_SCHEMA = "c4ca1cdd07de041e71f3e8d4b1942d29da89653c85276025d786688b6f576443";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::Test(Utf8CP name, std::function<void()> test){
    std::cout << "  o " << " " << name << std::endl;
    test();
}

//=======================================================================================
// Guardrails for things the schema sync design relies on everywhere, so a change that quietly
// breaks one is caught by whichever test happens to run next. All non-fatal, so a violation names
// itself without hiding what the test was asserting.
//=======================================================================================
namespace {

constexpr Utf8CP CHECK_SYNC_ALIAS = "check_sync_db";

// gtest's failure state is not declared in the non-gtest BeTest configuration, where the checks
// simply always run.
bool CurrentTestHasFailed() {
#if defined (USE_GTEST)
    return ::testing::Test::HasFailure();
#else
    return false;
#endif
}

// Everything a schema sync db is allowed to contain besides its ec_ tables. The two ecdbf_ tables
// are the only data tables SchemaSync::Init leaves behind, so they are named rather than
// prefix-matched: a third one appearing is exactly the kind of leak this check exists to find.
bool IsAllowedInSyncDb(Utf8StringCR tableName) {
    return tableName.StartsWithIAscii("ec_")
        || tableName.StartsWithIAscii("be_")
        || tableName.StartsWithIAscii("sqlite_")
        || tableName.EqualsIAscii("ecdbf_ExternalFileInfo")
        || tableName.EqualsIAscii("ecdbf_FileInfoOwnership");
}

// Read straight from pragma table_info, so the test helper carries no dependency on ECDb's internal
// schema sync helpers.
bool ReadColumns(DbCR db, Utf8CP alias, Utf8StringCR table, bvector<Utf8String>& all, bvector<Utf8String>& pk) {
    Statement stmt;
    if (stmt.Prepare(db, SqlPrintfString("pragma [%s].table_info([%s])", alias, table.c_str()).GetUtf8CP()) != BE_SQLITE_OK)
        return false;

    while (stmt.Step() == BE_SQLITE_ROW) {
        Utf8String name(stmt.GetValueText(1));
        if (stmt.GetValueInt(5) > 0)
            pk.push_back(name);

        all.push_back(name);
    }
    return !all.empty();
}

// ec_cache_* is left out: it is derived, every file rebuilds it locally, and neither the adopt nor
// the overwrite path copies it, so the two files legitimately disagree on every row.
bvector<Utf8String> ReadECTableNames(DbCR db) {
    bvector<Utf8String> tables;
    Statement stmt;
    if (stmt.Prepare(db, "SELECT name FROM main.sqlite_master WHERE type='table' AND name LIKE 'ec\\_%' ESCAPE '\\'"
                         " AND name NOT LIKE 'ec\\_cache\\_%' ESCAPE '\\' ORDER BY name") != BE_SQLITE_OK)
        return tables;

    while (stmt.Step() == BE_SQLITE_ROW)
        tables.push_back(stmt.GetValueText(0));

    return tables;
}

Utf8String JoinExprs(bvector<Utf8String> const& exprs, Utf8CP separator) {
    Utf8String joined;
    for (auto const& expr : exprs) {
        if (!joined.empty())
            joined.append(separator);

        joined.append(expr);
    }
    return joined;
}

// Reads one table as one quoted text line per row, keyed by rowid. quote() keeps NULLs and types
// visible, which matters because two values that print the same can still hash differently.
bmap<int64_t, Utf8String> ReadTableRows(ECDbR db, Utf8CP table) {
    bvector<Utf8String> columns;
    {
    Statement stmt;
    if (stmt.Prepare(db, SqlPrintfString("pragma main.table_info(%s)", table).GetUtf8CP()) != BE_SQLITE_OK)
        return {};
    while (stmt.Step() == BE_SQLITE_ROW)
        columns.push_back(stmt.GetValueText(1));
    }

    bvector<Utf8String> exprs;
    for (auto const& column : columns)
        exprs.push_back(SqlPrintfString("'%s='||quote([%s])", column.c_str(), column.c_str()).GetUtf8CP());

    bmap<int64_t, Utf8String> rows;
    Statement stmt;
    if (stmt.Prepare(db, SqlPrintfString("SELECT ROWID, %s FROM main.[%s]", JoinExprs(exprs, " || ' | ' || ").c_str(), table).GetUtf8CP()) != BE_SQLITE_OK)
        return {};
    while (stmt.Step() == BE_SQLITE_ROW)
        rows[stmt.GetValueInt64(0)] = stmt.GetValueText(1);
    return rows;
}

// Names the rows that differ, rather than only reporting that a hash did not match. A bare hash
// mismatch says nothing about which column is wrong, and the interesting ones - a foreign key nulled
// by a cascade, a type that changed - are invisible in a checksum.
void ReportRowDifferences(ECDbR actual, ECDbR expected, Utf8CP table, Utf8CP context) {
    const auto actualRows = ReadTableRows(actual, table);
    const auto expectedRows = ReadTableRows(expected, table);

    int reported = 0;
    constexpr int maxReported = 10;
    for (auto const& entry : expectedRows) {
        const auto found = actualRows.find(entry.first);
        if (found == actualRows.end()) {
            printf("[schemasync-check] %s: %s rowid=%lld only in expected: %s\n", context, table, (long long)entry.first, entry.second.c_str());
            ++reported;
        } else if (!entry.second.Equals(found->second)) {
            printf("[schemasync-check] %s: %s rowid=%lld\n    expected: %s\n      actual: %s\n", context, table, (long long)entry.first, entry.second.c_str(), found->second.c_str());
            ++reported;
        }
        if (reported >= maxReported)
            return;
    }
    for (auto const& entry : actualRows) {
        if (expectedRows.find(entry.first) != expectedRows.end())
            continue;
        printf("[schemasync-check] %s: %s rowid=%lld only in actual: %s\n", context, table, (long long)entry.first, entry.second.c_str());
        if (++reported >= maxReported)
            return;
    }
}

} // namespace

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::VerifySyncDbHoldsOnlyMetadata(ECDbR syncDb, Utf8CP context) {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(syncDb, "SELECT name FROM main.sqlite_master WHERE type='table' ORDER BY name"));
    while (stmt.Step() == BE_SQLITE_ROW) {
        Utf8String name(stmt.GetValueText(0));
        EXPECT_TRUE(IsAllowedInSyncDb(name)) << context << ": the sync db holds '" << name.c_str() << "', which is not metadata. File: " << syncDb.GetDbFileName();
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::VerifyBriefcaseRowsExistInSyncDb(ECDbR briefcase, SchemaSyncDb& syncDb, Utf8CP context) {
    if (!syncDb.GetFileName().DoesPathExist())
        return;

    if (BE_SQLITE_OK != briefcase.AttachDb(syncDb.GetFileName().GetNameUtf8().c_str(), CHECK_SYNC_ALIAS)) {
        ADD_FAILURE() << context << ": could not attach the sync db to check row containment. File: " << briefcase.GetDbFileName();
        return;
    }

    for (auto const& table : ReadECTableNames(briefcase)) {
        bvector<Utf8String> localCols, localPk, syncCols, syncPk;
        if (!ReadColumns(briefcase, "main", table, localCols, localPk)) {
            ADD_FAILURE() << context << ": could not read the columns of " << table.c_str();
            continue;
        }
        if (!ReadColumns(briefcase, CHECK_SYNC_ALIAS, table, syncCols, syncPk)) {
            ADD_FAILURE() << context << ": the sync db has no " << table.c_str() << ", so the two files are not on the same profile";
            continue;
        }
        if (localCols.size() != syncCols.size()) {
            ADD_FAILURE() << context << ": " << table.c_str() << " has a different shape in the two files";
            continue;
        }

        bvector<Utf8String> matchExprs;
        for (auto const& col : localCols) {
            // = on the primary key so the lookup uses its index; IS elsewhere so two NULLs match.
            const bool isKey = std::find(localPk.begin(), localPk.end(), col) != localPk.end();
            matchExprs.push_back(SqlPrintfString("[s].[%s] %s [t].[%s]", col.c_str(), isKey ? "=" : "IS", col.c_str()).GetUtf8CP());
        }
        if (matchExprs.empty())
            continue;

        const auto missing = Utf8String{SqlPrintfString(
            "FROM main.[%s] AS [t] WHERE NOT EXISTS (SELECT 1 FROM [%s].[%s] AS [s] WHERE %s)",
            table.c_str(), CHECK_SYNC_ALIAS, table.c_str(), JoinExprs(matchExprs, " AND ").c_str()).GetUtf8CP()};

        Statement countStmt;
        if (countStmt.Prepare(briefcase, SqlPrintfString("SELECT COUNT(*) %s", missing.c_str()).GetUtf8CP()) != BE_SQLITE_OK) {
            ADD_FAILURE() << context << ": could not check containment for " << table.c_str();
            continue;
        }
        if (countStmt.Step() != BE_SQLITE_ROW || countStmt.GetValueInt64(0) == 0)
            continue;

        ADD_FAILURE() << context << ": " << countStmt.GetValueInt64(0) << " row(s) in " << table.c_str()
                      << " do not exist in the sync db. File: " << briefcase.GetDbFileName();

        bvector<Utf8String> quoted;
        for (auto const& col : localCols)
            quoted.push_back(SqlPrintfString("'%s='||quote([t].[%s])", col.c_str(), col.c_str()).GetUtf8CP());

        Statement rowStmt;
        if (rowStmt.Prepare(briefcase, SqlPrintfString("SELECT %s %s LIMIT 5", JoinExprs(quoted, "||' | '||").c_str(), missing.c_str()).GetUtf8CP()) != BE_SQLITE_OK)
            continue;

        while (rowStmt.Step() == BE_SQLITE_ROW)
            printf("[schemasync-check] %s: %s not in sync db: %s\n", context, table.c_str(), rowStmt.GetValueText(0));
    }

    briefcase.DetachDb(CHECK_SYNC_ALIAS);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::VerifyFileIsSound(ECDbCR db, Utf8CP context) {
    Statement stmt;
    if (stmt.Prepare(db, "PRAGMA integrity_check") == BE_SQLITE_OK) {
        while (stmt.Step() == BE_SQLITE_ROW) {
            Utf8CP result = stmt.GetValueText(0);
            EXPECT_STREQ("ok", result) << context << ": integrity_check failed. File: " << db.GetDbFileName();
        }
    }

    EXPECT_TRUE(ForeignkeyCheck(db)) << context << ": foreign key violations. File: " << db.GetDbFileName();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::VerifySchemaSyncRules(SchemaSyncDb& syncDb, std::vector<ECDb*> const& briefcases, Utf8CP context) {
    if (!syncDb.GetFileName().DoesPathExist())
        return;

    syncDb.WithReadOnly([&](ECDbR sync) {
        VerifySyncDbHoldsOnlyMetadata(sync, context);
        VerifyFileIsSound(sync, context);
    });

    const auto syncDataVer = SchemaSync::SyncDbInfo::From(syncDb.GetSyncDbUri()).GetDataVersion();
    for (auto* briefcase : briefcases) {
        if (briefcase == nullptr || !briefcase->IsDbOpen())
            continue;

        VerifyFileIsSound(*briefcase, context);

        // Containment only holds while the briefcase is level with the sync db. Once somebody else
        // has imported - a delete in particular - the sync db no longer has everything this file
        // still holds, and it will not until this file pulls.
        auto const& schemaSync = briefcase->Schemas().GetSchemaSync();
        if (schemaSync.IsEnabled() && schemaSync.GetInfo().GetDataVersion() == syncDataVer)
            VerifyBriefcaseRowsExistInSyncDb(*briefcase, syncDb, context);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::ExpectECTablesIdentical(ECDbR actual, ECDbR expected, Utf8CP context) {
    const auto tables = ReadECTableNames(expected);
    ASSERT_FALSE(tables.empty());

    auto rowCount = [](ECDbR db, Utf8CP table) -> int64_t {
        Statement stmt;
        if (stmt.Prepare(db, SqlPrintfString("SELECT COUNT(*) FROM main.[%s]", table).GetUtf8CP()) != BE_SQLITE_OK)
            return -1;
        return stmt.Step() == BE_SQLITE_ROW ? stmt.GetValueInt64(0) : -1;
    };
    auto contentHash = [](ECDbR db, Utf8CP table) -> Utf8String {
        Statement stmt;
        if (stmt.Prepare(db, SqlPrintfString("SELECT hex(sha3_query('SELECT * FROM [%s] ORDER BY ROWID'))", table).GetUtf8CP()) != BE_SQLITE_OK)
            return "";
        return stmt.Step() == BE_SQLITE_ROW ? Utf8String(stmt.GetValueText(0)) : Utf8String("");
    };

    for (auto const& table : tables) {
        const auto actualCount = rowCount(actual, table.c_str());
        const auto expectedCount = rowCount(expected, table.c_str());
        EXPECT_EQ(expectedCount, actualCount) << context << ": row count differs in " << table.c_str();
        if (actualCount != expectedCount) {
            ReportRowDifferences(actual, expected, table.c_str(), context);
            continue;
        }
        if (!contentHash(expected, table.c_str()).Equals(contentHash(actual, table.c_str()))) {
            ADD_FAILURE() << context << ": row CONTENT differs in " << table.c_str() << " (same count, different values)";
            ReportRowDifferences(actual, expected, table.c_str(), context);
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::ExpectPhysicalSchemaIdentical(ECDbR actual, ECDbR expected, Utf8CP context) {
    auto read = [](ECDbR db) {
        bmap<Utf8String, Utf8String> objects;
        Statement stmt;
        if (stmt.Prepare(db, "SELECT type, name, sql FROM main.sqlite_master WHERE sql IS NOT NULL") != BE_SQLITE_OK)
            return objects;
        while (stmt.Step() == BE_SQLITE_ROW)
            objects[Utf8String(stmt.GetValueText(1))] = Utf8PrintfString("%s: %s", stmt.GetValueText(0), stmt.GetValueText(2));
        return objects;
    };

    const auto actualObjects = read(actual);
    const auto expectedObjects = read(expected);
    ASSERT_FALSE(expectedObjects.empty());

    for (auto const& entry : expectedObjects) {
        const auto found = actualObjects.find(entry.first);
        if (found == actualObjects.end())
            ADD_FAILURE() << context << ": " << entry.first.c_str() << " is missing\n    expected: " << entry.second.c_str();
        else if (!entry.second.Equals(found->second))
            ADD_FAILURE() << context << ": " << entry.first.c_str() << " differs\n    expected: " << entry.second.c_str() << "\n      actual: " << found->second.c_str();
    }
    for (auto const& entry : actualObjects) {
        if (expectedObjects.find(entry.first) == expectedObjects.end())
            ADD_FAILURE() << context << ": " << entry.first.c_str() << " exists only in the file under test\n    " << entry.second.c_str();
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::ExpectNoForeignKeyViolations(ECDbR db, Utf8CP context) {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "PRAGMA main.foreign_key_check"));
    int violations = 0;
    while (stmt.Step() == BE_SQLITE_ROW) {
        ++violations;
        printf("[schemasync-check] FK violation (%s): table=%s rowid=%lld parent=%s fkid=%d\n",
            context, stmt.GetValueText(0), stmt.GetValueInt64(1), stmt.GetValueText(2), stmt.GetValueInt(3));
    }
    EXPECT_EQ(0, violations) << context << ": rows were copied whose parents are missing";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::TearDown() {
    // Only on a test that got where it meant to - a failed one has arbitrary state and would report
    // violations that say nothing.
    if (!CurrentTestHasFailed() && m_schemaChannel != nullptr && m_briefcase != nullptr)
        VerifySchemaSyncRules(*m_schemaChannel, { m_briefcase.get() }, "teardown");

    ECDbTestFixture::TearDown();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaImportResult SchemaSyncTestFixture::ImportSchemas(ECDbR ecdb, std::vector<SchemaItem> items, SchemaManager::SchemaImportOptions opts, SchemaSync::SyncDbUri uri) {
    auto schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(ecdb.GetSchemaLocater());
    bvector<ECSchemaCP> importSchemas;
    for(auto& item: items) {
        ECSchemaPtr schema;
        if (item.GetType() == SchemaItem::Type::File)
            {
            // Construct the path to the sample schema
            BeFileName ecSchemaFilePath;
            BeTest::GetHost().GetDocumentsRoot(ecSchemaFilePath);
            ecSchemaFilePath.AppendToPath(L"ECDb");
            ecSchemaFilePath.AppendToPath(L"Schemas");
            ecSchemaFilePath.AppendToPath(item.GetFileName());

            if (!ecSchemaFilePath.DoesPathExist())
                return SchemaImportResult::ERROR;

            schemaReadContext->AddSchemaPath(ecSchemaFilePath.GetName());
            ECSchema::ReadFromXmlFile(schema, ecSchemaFilePath, *schemaReadContext);
            }
        else
            {
            BeAssert(item.GetType() == SchemaItem::Type::String);
            ECSchema::ReadFromXmlString(schema, item.GetXmlString().c_str(), *schemaReadContext);
            }
        if (!schema.IsValid()) {
            return SchemaImportResult::ERROR;
        }
        importSchemas.push_back(schema.get());
    }
    return ecdb.Schemas().ImportSchemas(importSchemas, opts,nullptr, uri);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaImportResult SchemaSyncTestFixture::ImportSchema(ECDbR ecdb, SchemaItem item, SchemaManager::SchemaImportOptions opts, SchemaSync::SyncDbUri uri) {
    return ImportSchemas(ecdb, std::vector<SchemaItem>{item}, opts, uri);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaImportResult SchemaSyncTestFixture::ImportSchema(SchemaItem item, SchemaManager::SchemaImportOptions opts)
    {
    return ImportSchemas(*m_briefcase, std::vector<SchemaItem> {item}, opts, GetSyncDbUri());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<TrackedECDb> SchemaSyncTestFixture::OpenECDb(Utf8CP asFileNam) {
    auto ecdb = std::make_unique<TrackedECDb>();
    if (BE_SQLITE_OK != ecdb->OpenBeSQLiteDb(asFileNam, Db::OpenParams(Db::OpenMode::ReadWrite))) {
        return nullptr;
    }
    return std::move(ecdb);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SchemaSyncTestFixture::ReopenECDb()
    {
    if (!m_briefcase->IsDbOpen())
        return BE_SQLITE_ERROR;

    auto saveStatus = m_briefcase->SaveChanges();
    if (saveStatus != BE_SQLITE_OK)
        {
        printf("Failed to save changes");
        return saveStatus;
        }

    Utf8String filename = m_briefcase->GetDbFileName();

    m_briefcase->CloseDb();
    m_briefcase = OpenECDb(filename.c_str());
    if (m_briefcase == nullptr)
        return BE_SQLITE_ERROR;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::CloseECDb()
    {
    if (m_briefcase->IsDbOpen())
        m_briefcase->SaveChanges();
    m_briefcase->CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaImportResult SchemaSyncTestFixture::SetupECDb(Utf8CP ecdbName)
    {
    m_hub = std::make_unique<ECDbHub>(ECDbHub());
    m_briefcase = m_hub->CreateBriefcase();
    m_schemaChannel = std::make_unique<SchemaSyncDb>(SchemaSyncDb(ecdbName));
    if (SchemaSync::Status::OK != m_briefcase->Schemas().GetSchemaSync().Init(GetSyncDbUri(), BeGuid(true).ToString(), false))
        return SchemaImportResult::ERROR;

    EXPECT_EQ(BE_SQLITE_OK, m_briefcase->PullMergePush("init"));
    EXPECT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

    m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckSyncHashes(syncDb); });
    CheckHashes(*m_briefcase);

    return SchemaImportResult::OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaImportResult SchemaSyncTestFixture::SetupECDb(Utf8CP ecdbName, SchemaItem const& schema, SchemaManager::SchemaImportOptions opts)
    {
    m_hub = std::make_unique<ECDbHub>(ECDbHub());
    m_briefcase = m_hub->CreateBriefcase();
    m_schemaChannel = std::make_unique<SchemaSyncDb>(SchemaSyncDb(ecdbName));
    if (SchemaSync::Status::OK != m_briefcase->Schemas().GetSchemaSync().Init(GetSyncDbUri(), BeGuid(true).ToString(), false))
        return SchemaImportResult::ERROR;

    EXPECT_EQ(BE_SQLITE_OK, m_briefcase->PullMergePush("init"));
    EXPECT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

    m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckSyncHashes(syncDb); });
    CheckHashes(*m_briefcase);

    if (SchemaImportResult::OK != SchemaSyncTestFixture::ImportSchema(*m_briefcase, schema, opts, GetSyncDbUri()))
        {
        EXPECT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
        return SchemaImportResult::ERROR;
        }

    EXPECT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

    return SchemaImportResult::OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DropSchemaResult SchemaSyncTestFixture::DropSchema(Utf8CP schemaName)
    {
    auto dropSuccess = m_briefcase->Schemas().DropSchema(schemaName);

    if (dropSuccess.IsSuccess())
        EXPECT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
    else
        EXPECT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());

    return dropSuccess;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SchemaSyncTestFixture::GetSchemaHash(ECDbCR db) {
    ECSqlStatement stmt;
    if (stmt.Prepare(db, "PRAGMA checksum(ecdb_schema)") != ECSqlStatus::Success) {
        return "";
    }
    if (stmt.Step() == BE_SQLITE_ROW) {
        return stmt.GetValueText(0);
    }
    return "";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SchemaSyncTestFixture::GetMapHash(ECDbCR db) {
    ECSqlStatement stmt;
    if (stmt.Prepare(db, "PRAGMA checksum(ecdb_map)") != ECSqlStatus::Success) {
        return "";
    }
    if (stmt.Step() == BE_SQLITE_ROW) {
        return stmt.GetValueText(0);
    }
    return "";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SchemaSyncTestFixture::GetDbSchemaHash(ECDbCR db) {
    ECSqlStatement stmt;
    if (stmt.Prepare(db, "PRAGMA checksum(sqlite_schema)") != ECSqlStatus::Success) {
        return "";
    }
    if (stmt.Step() == BE_SQLITE_ROW) {
        return stmt.GetValueText(0);
    }
    return "";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaSyncTestFixture::ForeignkeyCheck(ECDbCR db) {
    Statement stmt;
    EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "PRAGMA foreign_key_check"));
    auto rc = stmt.Step();
    if (rc == BE_SQLITE_DONE) {
        return true;
    }
    while(rc == BE_SQLITE_ROW) {
        printf("%s\n",
                SqlPrintfString("[table=%s], [rowid=%lld], [parent=%s], [fkid=%d]",
                                stmt.GetValueText(0),
                                stmt.GetValueInt64(1),
                                stmt.GetValueText(2),
                                stmt.GetValueInt(3))
                    .GetUtf8CP());

        rc = stmt.Step();
    }
    return false;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::PrintHash(ECDbR ecdb, Utf8CP desc) {
    printf("=====%s======\n", desc);
    printf("\tSchema: SHA3-%s\n", GetSchemaHash(ecdb).c_str());
    printf("\t   Map: SHA3-%s\n", GetMapHash(ecdb).c_str());
    printf("\t    Db: SHA3-%s\n", GetDbSchemaHash(ecdb).c_str());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::CheckHashes(ECDbR ecdb, Utf8CP schemaHash, Utf8CP mapHash, Utf8CP dbSchemaHash, bool strictCheck, int lineNo)
    {
    if (strictCheck)
        {
        ASSERT_STREQ(schemaHash, GetSchemaHash(ecdb).c_str())       << "File: " << ecdb.GetDbFileName() << " Line: " << lineNo;
        ASSERT_STREQ(mapHash, GetMapHash(ecdb).c_str())             << "File: " << ecdb.GetDbFileName() << " Line: " << lineNo;;
        ASSERT_STREQ(dbSchemaHash, GetDbSchemaHash(ecdb).c_str())   << "File: " << ecdb.GetDbFileName() << " Line: " << lineNo;;
        }
    else
        {
        EXPECT_STREQ(schemaHash, GetSchemaHash(ecdb).c_str())       << "File: " << ecdb.GetDbFileName() << " Line: " << lineNo;;
        EXPECT_STREQ(mapHash, GetMapHash(ecdb).c_str())             << "File: " << ecdb.GetDbFileName() << " Line: " << lineNo;;
        EXPECT_STREQ(dbSchemaHash, GetDbSchemaHash(ecdb).c_str())   << "File: " << ecdb.GetDbFileName() << " Line: " << lineNo;;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string SchemaSyncTestFixture::GetIndexDDL(ECDbCR ecdb, Utf8CP indexName) {
    Statement stmt;
    EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "select sql from sqlite_master where name=?"));
    stmt.BindText(1, indexName, Statement::MakeCopy::Yes);
    if (stmt.Step() == BE_SQLITE_ROW) {
        return stmt.GetValueText(0);
    }
    return "";
};

//***************************************************************************************
// SchemaSyncDb
//***************************************************************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSyncDb::SchemaSyncDb(Utf8CP name){
    BeFileName outPath;
    BeTest::GetHost().GetOutputRoot(outPath);
    Utf8String fileName = name;
    fileName.append(".ecdb");
    outPath.AppendToPath(WString(fileName.c_str(), true).GetWCharCP());
    if (outPath.DoesPathExist()) {
        if (outPath.BeDeleteFile() != BeFileNameStatus::Success) {
            throw std::runtime_error("unable to delete file");
        }
    }
    m_fileName = outPath;
    auto ecdb = std::make_unique<ECDb>();
    if (BE_SQLITE_OK != ecdb->CreateNewDb(m_fileName)) {
        throw std::runtime_error("unable to create file");
    }
    ecdb->SaveChanges();
    ecdb->CloseDb();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName SchemaSyncDb::GetFileName() const { return m_fileName;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<ECDb> SchemaSyncDb::OpenReadOnly(DefaultTxn mode) {
    auto ecdb = std::make_unique<ECDb>();
    if (ecdb->OpenBeSQLiteDb(m_fileName, Db::OpenParams(Db::OpenMode::Readonly, mode)) != BE_SQLITE_OK) {
        return nullptr;
    }
    return std::move(ecdb);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<ECDb> SchemaSyncDb::OpenReadWrite(DefaultTxn mode) {
    auto ecdb = std::make_unique<ECDb>();
    if (ecdb->OpenBeSQLiteDb(m_fileName, Db::OpenParams(Db::OpenMode::ReadWrite, mode)) != BE_SQLITE_OK) {
        return nullptr;
    }
    return std::move(ecdb);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncDb::WithReadOnly(std::function<void(ECDbR)> cb, DefaultTxn mode) {
    auto ecdb = OpenReadOnly(mode);
    if (ecdb == nullptr) {
        throw std::runtime_error("unable to open file");
    }
    cb(*ecdb);
    ecdb->CloseDb();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncDb::WithReadWrite(std::function<void(ECDbR)> cb, DefaultTxn mode) {
    auto ecdb = OpenReadWrite(mode);
    if (ecdb == nullptr) {
        throw std::runtime_error("unable to open file");
    }
    cb(*ecdb);
    ecdb->CloseDb();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// DbResult SchemaSyncDb::Push(ECDbR ecdb, std::function<void()> cb) {
//     auto rc = ecdb.Schemas().SyncSchemas(GetFileName().GetNameUtf8(), SchemaManager::SyncAction::Push);
//     if (rc == BE_SQLITE_OK && cb != nullptr) {
//         cb();
//     }
//     return rc;
// }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSync::Status SchemaSyncDb::Pull(ECDbR ecdb, std::function<void()> cb) {
    auto rc = ecdb.Schemas().GetSchemaSync().Pull(GetSyncDbUri());
    if (rc == SchemaSync::Status::OK && cb != nullptr) {
        cb();
    }
    return rc;
}

//***************************************************************************************
// InMemoryECDb
//***************************************************************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InMemoryECDb::WriteToDisk(Utf8CP fileName, const char *zSchema, bool overrideFile) const {
    BeFileName filePath(fileName);
    if (filePath.DoesPathExist()) {
        if (overrideFile) {
            if (filePath.BeDeleteFile() != BeFileNameStatus::Success) {
                return false;
            }
        } else {
            return false;
        }
    }
    DbBuffer buf = Serialize(zSchema);
    if (buf.Empty()) {
        return false;
    }
    BeFile outFile;
    if (BeFileStatus::Success != outFile.Create(filePath, true)) {
        return false;
    }
    if (BeFileStatus::Success != outFile.Write(nullptr, buf.Data(), (uint32_t)buf.Size())) {
        return false;
    }
    if (BeFileStatus::Success != outFile.Flush() ){
        return false;
    }
    return BeFileStatus::Success == outFile.Close();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InMemoryECDb::Ptr InMemoryECDb::CreateSnapshot(DbResult* outRc) {
    DbResult ALLOW_NULL_OUTPUT(rc, outRc);
    //SaveChanges("create snapshot");
    auto buff = Serialize();
    auto dbPtr = InMemoryECDb::Create();
    dbPtr->CloseDb();
    rc = Db::Deserialize(buff, *dbPtr, DbDeserializeOptions::FreeOnClose | DbDeserializeOptions::Resizable, nullptr, [&](DbR db) {
        db.ResetBriefcaseId(BeBriefcaseId(0));
    });
    if (rc == BE_SQLITE_OK) {

        dbPtr->ChangeDbGuid(GetDbGuid());
        return std::move(dbPtr);
    }
    return nullptr;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InMemoryECDb::Ptr InMemoryECDb::Create() {
	return Ptr(new InMemoryECDb());

}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaImportResult InMemoryECDb::ImportSchema(SchemaItem const& si) {
    auto ctx = ECSchemaReadContextPtr();
    if (ECDbTestFixture::ReadECSchema(ctx, *this, si) != SUCCESS)
        return SchemaImportResult::ERROR;

    bvector<ECN::ECSchemaP> schemas;
    ctx->GetCache().GetSchemas(schemas);
    bvector<ECN::ECSchemaCP> schemasIn(schemas.begin(), schemas.end());
    return Schemas().ImportSchemas(schemasIn, nullptr);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InMemoryECDb::InMemoryECDb() {
    if (CreateNewDb(BEDB_MemoryDb) != BE_SQLITE_OK) {
        throw std::runtime_error("unable to created in memory ecdb");
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InMemoryECDb::~InMemoryECDb() {
    if (IsDbOpen())
        CloseDb();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InMemoryECDb::_OnDbClose() {
    SaveChanges();
    ECDb::_OnDbClose();
}

//***************************************************************************************
// TrackedECDb
//***************************************************************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TrackedECDb::_OnDbCreated(CreateParams const& params) {
	auto rc = ECDb::_OnDbCreated(params);
	SetupTracker();
	return rc;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TrackedECDb::_OnDbOpening() {
	auto rc = ECDb::_OnDbOpening();
	if (!IsReadonly()) {
		SetupTracker();

	}
	return rc;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TrackedECDb::SetupTracker(std::unique_ptr<ECDbChangeTracker> tracker) {
    if (m_tracker != nullptr) {
        m_tracker->EndTracking();
        this->SetChangeTracker(nullptr);
        m_tracker = nullptr;
    }
    m_tracker = tracker != nullptr ? std::move(tracker) : std::make_unique<ECDbChangeTracker>(*this);
    this->SetChangeTracker(m_tracker.get());
    m_tracker->EnableTracking(true);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TrackedECDb::~TrackedECDb() {
    if (IsDbOpen())
        CloseDb();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TrackedECDb::_OnDbClose() {
    SaveChanges();
    this->SetChangeTracker(nullptr);
    if (m_tracker != nullptr) {
        m_tracker->EndTracking();
        m_tracker = nullptr;
    }
    ECDb::_OnDbClose();
}

//***************************************************************************************
// ECDbChangeTracker
//***************************************************************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<ECDbChangeSet const*> ECDbChangeTracker::GetLocalChangesets() const {
    std::vector<ECDbChangeSet const*>  list;
    for(auto& r : m_localChangesets) {
        list.push_back(r.get());
    }
    return list;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbChangeSet::Ptr ECDbChangeTracker::MakeChangeset(bool deleteLocalChangesets, Utf8CP op) {
    bvector<Utf8String> ddlChanges;
    ChangeGroup group;
    bool hasSchemaChanges = false;

    m_mdb.SaveChanges(op);

    for (auto& changeset : m_localChangesets) {
        const auto rc = changeset->AddToChangeGroup(group);
        if (rc != BE_SQLITE_OK) {
            return nullptr;
        }
        for (auto& ddl : changeset->GetDDLs()) {
            ddlChanges.push_back(ddl);
        }
        if (!hasSchemaChanges) {
            hasSchemaChanges = changeset->HasSchemaChanges();
        }
    }

    auto changeset = ECDbChangeSet::Create((int)(m_localChangesets.size() + 1), op, BeStringUtilities::Join(ddlChanges, ";").c_str(), hasSchemaChanges);
    if (BE_SQLITE_OK != changeset->FromChangeGroup(group)) {
        return nullptr;
    }
    if (deleteLocalChangesets) {
        m_localChangesets.clear();
    }
    return std::move(changeset);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeTracker::OnCommitStatus ECDbChangeTracker::_OnCommit(bool isCommit, Utf8CP operation) {
    if (isCommit) {
        auto changeset = ECDbChangeSet::From(*this, operation);
        if (changeset != nullptr) {
            m_localChangesets.push_back(std::move(changeset));
        }
    }
    EndTracking();
    CreateSession();
    return OnCommitStatus::Commit;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbChangeTracker::Ptr ECDbChangeTracker::Clone(ECDb& db) const {
    auto tracker= Create(db);
    for (auto& changeset : m_localChangesets) {
        tracker->m_localChangesets.push_back(changeset->Clone());
    }
    return std::move(tracker);

}

//***************************************************************************************
// ECDbChangeSet
//***************************************************************************************

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbChangeSet::ToSQL(DbCR db, std::function<void(bool isDDL, std::string const&)> cb) const{
	for (auto& ddl : GetDDLs()) {
		cb(true,  ddl + ";");
	}
	bmap<Utf8String,bvector<Utf8String>> tableColMap;
	auto toString = [](DbValue const& val) -> std::string {
		if (!val.IsValid()) {
			return "NULL";
		}
		switch (val.GetValueType()) {
		case DbValueType::IntegerVal:
			return Utf8PrintfString("%" PRId64, val.GetValueInt64());
		case DbValueType::FloatVal:
			return Utf8PrintfString("%0.17lf", val.GetValueDouble());
		case DbValueType::TextVal:
			return Utf8PrintfString("'%s'", val.GetValueText());
		case DbValueType::BlobVal: {
			Utf8String hexStr;
			std::string valStr = "X'";
			const auto ptr = (uint8_t const*)(val.GetValueBlob());
			const auto len = val.GetValueBytes();
			for (auto i = 0; i < len; ++i) {
				hexStr.clear();
				hexStr.Sprintf("%02X", ptr[i]);
				valStr.append(hexStr);
			}
			valStr.append("'");
			return std::move(valStr);
		}
		case DbValueType::NullVal:
			return "NULL";
		}
		return "<unknown>";
	};

	std::string sql;
	for (auto const& change : const_cast<ECDbChangeSet*>(this)->GetChanges()) {
		Byte* pkCols;
		int nPkCols;
		Utf8CP tableName;
		DbOpcode opCode;
		int indirect;
		int nCols;
		change.GetOperation(&tableName, &nCols, &opCode, &indirect);
		change.GetPrimaryKeyColumns(&pkCols, &nPkCols);
		auto it = tableColMap.find(tableName);
		if (it == tableColMap.end()) {
			auto newIt = tableColMap.emplace(tableName, bvector<Utf8String>());
			it = newIt.first;
			db.GetColumns(it->second, tableName);
		}
		auto const& columns = it->second;
		if (opCode == DbOpcode::Delete) {
			sql = "DELETE FROM ";
			sql.append(tableName).append(" WHERE ");
			bool first = true;
			for (int i = 0; i < nCols; ++i) {
				if (pkCols[i]) {
					auto& columnName = columns[i];
					auto dbVal = change.GetOldValue(i);
					if (first) {
						first = false;
					} else {
						sql.append(" AND ");
					}
					sql.append("(").append(columnName).append(" IS ").append(toString(dbVal).append(")"));
				}
			}
			sql.append(";");
			cb(false, sql);
		} else if (opCode == DbOpcode::Insert) {
			sql = "INSERT INTO ";
			sql.append(tableName).append("(");
			std::string val = " VALUES(";
			bool first = true;
			for (int i = 0; i < nCols; ++i) {
				auto& columnName = columns[i];
				auto dbVal = change.GetNewValue(i);
				if (dbVal.IsNull())
					continue;

				if (first) {
					first = false;
				} else {
					sql.append(",");
					val.append(",");
				}
				sql.append(columnName);
				val.append(toString(dbVal));
			}
			sql.append(")").append(val).append(");");
			cb(false, sql);
		} else if (opCode == DbOpcode::Update) {
			sql = "UPDATE ";
			sql.append(tableName).append(" SET ");
			std::string where = " WHERE ";
			bool firstPk = true;
			bool firstData = true;
			for (int i = 0; i < nCols; ++i) {
				auto& columnName = columns[i];
				if (pkCols[i]) {
					auto dbVal = change.GetOldValue(i);
					if (firstPk) {
						firstPk = false;
					} else {
						sql.append(" AND ");
					}
					where.append("(").append(columnName).append(" IS ").append(toString(dbVal).append(")"));
				} else {
					auto oldVal = change.GetNewValue(i);
					if (firstData) {
						firstData = false;
					} else {
						sql.append(",");
					}
					sql.append(columnName).append("=").append(toString(oldVal));
				}
			}
			sql.append(where).append(";");
			cb(false, sql);
		}
	}
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> ECDbChangeSet::GetDDLs() const {
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(m_ddl.c_str(), ";", tokens);
    return tokens;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbChangeSet::Ptr ECDbChangeSet::Clone() const {
    auto changeset = std::make_unique<ECDbChangeSet>(0, m_operation.c_str(), m_ddl.c_str(), m_hasSchemaChanges);
    for (auto& chunk : this->m_data.m_chunks) {
        changeset->m_data.Append((Byte const*)&chunk[0], (int)chunk.size());
    }
    return std::move(changeset);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbChangeSet::Ptr ECDbChangeSet::From(ECDbChangeTracker& tracker, Utf8CP comment) {
    if (!tracker.HasChanges() && !tracker.HasDdlChanges()) {
        return nullptr;
    }
    auto changeset = std::make_unique<ECDbChangeSet>( (int)(tracker.GetLocalChangesets().size() + 1), comment, tracker.GetDDL().c_str(), false);
    if (tracker.HasChanges()) {
        auto rc = changeset->FromChangeTrack(tracker);
        if (rc != BE_SQLITE_OK) {
            return nullptr;
        }
        bool hasECChanges = false;
        for (auto& change : changeset->GetChanges()) {
            if (change.GetTableName().StartsWithIAscii("ec_")) {
                hasECChanges = true;
                break;
            }
        }
        changeset->m_hasSchemaChanges = hasECChanges;
    }
    return std::move(changeset);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbChangeSet::Ptr ECDbChangeSet::Create(int index, Utf8CP op, Utf8CP ddl, bool isSchemaChangeset) {
    return std::make_unique<ECDbChangeSet>(index, op, ddl, isSchemaChangeset);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeStream::ConflictResolution ECDbChangeSet::_OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) {

    Utf8CP tableName = nullptr;
    int nCols, indirect;
    DbOpcode opcode;
    DbResult result = iter.GetOperation(&tableName, &nCols, &opcode, &indirect);
    BeAssert(result == BE_SQLITE_OK);

    if (cause == ChangeSet::ConflictCause::Conflict) {
        if (0 == ::strncmp(tableName, "ec_", 3) && m_ecdb != nullptr && m_ecdb->Schemas().GetSchemaSync().IsEnabled()) {
            // Replace would DELETE the existing row before re-inserting it, and every ec_ child table
            // is ON DELETE CASCADE - so the delete takes the row's children with it and the re-insert
            // restores only the parent. This harness never passes fkNoAction, so actions are always
            // live here; production turns them off for a schema changeset that updates or deletes
            // ec_cache_ rows, which an additive import does not produce.
            // Under schema sync the rows arriving here are normally ones this briefcase already holds,
            // because every briefcase gets its ids from the same authority, so skipping keeps what is
            // already correct instead of destroying dependents.
            // Note this does not distinguish an identical row from a genuinely differing one; a
            // differing row is a real conflict and needs a real decision. Mirrors the rule in
            // ChangesetFileReader::_OnConflict.
            return ChangeSet::ConflictResolution::Skip;
        }
        return ChangeSet::ConflictResolution::Replace;
    }
    if (cause == ChangeSet::ConflictCause::Data) {
        // A briefcase holding local changes keeps its own ec_ rows: it got them from the sync db,
        // which decides them, so an incoming changeset carrying a different "before" value has
        // nothing to say about them. Mirrors ChangesetFileReader::_OnConflict, which does this in
        // the HasPendingTxns() branch. Without local changes the conflict falls through to Replace
        // below, exactly as it does there.
        auto const* briefcase = dynamic_cast<TrackedECDb const*>(m_ecdb);
        if (0 == ::strncmp(tableName, "ec_", 3) && briefcase != nullptr && briefcase->HasLocalChangesets())
            return ChangeSet::ConflictResolution::Skip;
    }
    if (cause == ChangeSet::ConflictCause::ForeignKey) {
        // Note: No current or conflicting row information is provided if it's a FKey conflict
        // Since we abort on FKey conflicts, always try and provide details about the error
        int nConflicts = 0;
        result = iter.GetFKeyConflicts(&nConflicts);
        BeAssert(result == BE_SQLITE_OK);
        LOG.errorv("Detected %d foreign key conflicts in ChangeSet. Aborting merge.", nConflicts);
        return ChangeSet::ConflictResolution::Abort ;
    }
    if(cause == ChangeSet::ConflictCause::NotFound) {
        if (opcode == DbOpcode::Delete) {
            // Caused by CASCADE DELETE on a foreign key, and is usually not a problem.
            return ChangeSet::ConflictResolution::Skip;
        }
  if (opcode == DbOpcode::Update && 0 == ::strncmp(tableName, "ec_", 3)) {
            // Caused by a ON DELETE SET NULL constraint on a foreign key - this is known to happen with "ec_" tables, but needs investigation if it happens otherwise
            return ChangeSet::ConflictResolution::Skip;
        }
        // Refer to comment below
        return opcode == DbOpcode::Update ? ChangeSet::ConflictResolution::Skip : ChangeSet::ConflictResolution::Replace;
    }
    if (ChangeSet::ConflictCause::Constraint == cause) {
        return ChangeSet::ConflictResolution::Skip;
    }
    return ConflictResolution::Replace;
}

//***************************************************************************************
// ECDbHub
//***************************************************************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ECDbHub::BuildECDbPath(Utf8CP name) const {
    BeFileName outPath = m_basePath;
    Utf8String fileName = name;
    fileName.append(".ecdb");
    outPath.AppendToPath(WString(fileName.c_str(), true).c_str());
    return outPath;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbHub::CreateSeedFile() {
    m_seedFile = BuildECDbPath("seed");
    if (m_seedFile.DoesPathExist()) {
        if (m_seedFile.BeDeleteFile() != BeFileNameStatus::Success) {
            throw std::runtime_error("unable to delete file");
        }
    }
/* Use this instead of the block below to create a new DB. Currently this will change the Checksums used in all tests...
    auto ecdb = std::make_unique<TrackedECDb>();
    if (BE_SQLITE_OK != ecdb->CreateNewDb(m_seedFile)) {
        return BE_SQLITE_ERROR;
    }
    ecdb->SaveChanges();
    ecdb->CloseDb();*/

    BeFileName seed4003FileName;
    BeTest::GetHost().GetDocumentsRoot(seed4003FileName);
    seed4003FileName.AppendToPath(L"ECDb").AppendToPath(L"profileseeds").AppendToPath(L"4003-sync-seed.ecdb");
    ECDb ecdb;
    if (ECDbTestFixture::CloneECDb(ecdb, m_seedFile, seed4003FileName) != DbResult::BE_SQLITE_OK)
        return BE_SQLITE_ERROR;

    ecdb.SaveChanges();
    ecdb.CloseDb();
    return BE_SQLITE_OK;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbHub::ECDbHub():m_id(true), m_briefcaseid(10) {
    BeFileName outPath;
    BeTest::GetHost().GetOutputRoot(outPath);
    outPath.AppendToPath(WString(m_id.ToString().c_str(), true).c_str());
    if (!outPath.DoesPathExist()) {
        BeFileName::CreateNewDirectory(outPath.GetName());
    }
    m_basePath = outPath;
    CreateSeedFile();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<ECDbChangeSet*> ECDbHub::Query(int afterChangesetId) {
	std::vector<ECDbChangeSet*> results;
	if (afterChangesetId < 0 || afterChangesetId >= (int)m_changesets.size()) {
		return results;
	}
	for (auto i = afterChangesetId; i < m_changesets.size(); ++i) {
		results.push_back(m_changesets[i].get());
	}
	return results;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int ECDbHub::PushNewChangeset(ECDbChangeSet::Ptr changeset) {
	m_changesets.push_back(std::move(changeset));
	return (int)(m_changesets.size()) - 1;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<TrackedECDb> ECDbHub::CreateBriefcase() {
    const auto briefcaseId = GetNextBriefcaseId();
    const auto fileName = BuildECDbPath(SqlPrintfString("%d", briefcaseId.GetValue()).GetUtf8CP());
    BeFileName::BeCopyFile(m_seedFile, fileName);
    auto ecdb = std::make_unique<TrackedECDb>();
    if (BE_SQLITE_OK != ecdb->OpenBeSQLiteDb(fileName, Db::OpenParams(Db::OpenMode::ReadWrite))) {
        fileName.BeDeleteFile();
        return nullptr;
    }
    ecdb->ResetBriefcaseId(briefcaseId);
    ecdb->SaveChanges();
    ecdb->SetHub(*this);
    ecdb->PullMergePush("");
    ecdb->SaveChanges();
    return std::move(ecdb);
}

struct PrintChangeSet {
    public:
        static void Print(ECDbCR conn, ECDbChangeSet const& cs) {
            std::vector<std::string> ddlList;
            std::vector<std::string> dmlList;
            cs.ToSQL(conn, [&](bool isDDL, const std::string& sql) {
                if (isDDL) {
                    ddlList.push_back(sql);
                } else {
                    dmlList.push_back(sql);
                }
            });
            int i = 0;
            std::sort(dmlList.begin(), dmlList.end());
            printf("====================================================\n");
            for(auto& v: ddlList) {
                printf("%2d. %s\n", ++i, v.c_str());
            }
            for(auto& v: dmlList) {
                printf("%2d. %s\n", ++i, v.c_str());
            }
            printf("====================================================\n");
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TrackedECDb::PullMergePush(Utf8CP comment) {
    if (m_hub == nullptr || m_tracker == nullptr) {
        return BE_SQLITE_ERROR;
    }
    auto changesetsToApply = m_hub->Query(m_changesetId + 1);

#ifdef TRACE_CS
    auto cancelTrace = GetTraceStmtEvent().AddListener([](TraceContext const& ctx, Utf8CP sql) {
        printf("[STMT] %s\n", ctx.GetExpandedSql().c_str());

    });
#endif
    m_tracker->EnableTracking(false);
    for (auto& changesetToApply : changesetsToApply) {
        changesetToApply->SetECDb(*this);
#ifdef TRACE_CS
        PrintChangeSet::Print(*this, *changesetToApply);
#endif
        // DDL first, so a widened table can take the row changes that come with it - the same order
        // and the same ignore-failures rule as TxnManager::ApplyDdlChanges.
        for (auto& ddl : changesetToApply->GetDDLs())
            TryExecuteSql(ddl.c_str());

        auto rc = changesetToApply->ApplyChanges(*this, false, true);
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("PullAndMergeChangesFrom(): %s", GetLastError().c_str());
#ifdef TRACE_CS
            cancelTrace();
#endif
            return rc;
        }
    }
    if (!changesetsToApply.empty()) {
        auto rc = AfterSchemaChangeSetApplied();
        if (rc != BE_SQLITE_OK) {
#ifdef TRACE_CS
            cancelTrace();
#endif
            return rc;
        }
    }
    m_tracker->EnableTracking(true);
    if (!m_tracker->GetLocalChangesets().empty()){
        auto changeset = m_tracker->MakeChangeset(true, comment);
        if (changeset == nullptr) {
            m_tracker->EnableTracking(true);
#ifdef TRACE_CS
            cancelTrace();
#endif
            return BE_SQLITE_ERROR;
        }
        m_changesetId = m_hub->PushNewChangeset(std::move(changeset));
    }
#ifdef TRACE_CS
    cancelTrace();
#endif
    return BE_SQLITE_OK;

}