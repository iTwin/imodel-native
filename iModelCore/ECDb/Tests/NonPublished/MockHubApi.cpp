#include "MockHubApi.h"
#include <ECDb/JsonAdapter.h>
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
size_t SchemaSyncTestFixture::InstanceCensus::GetInstanceCount() const {
    size_t count = 0;
    for (auto const& entry : m_rowsByClass)
        count += entry.second.size();
    return count;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSyncTestFixture::InstanceCensus SchemaSyncTestFixture::InstanceCensus::Take(ECDbCR db) {
    InstanceCensus census;
    for (ECSchemaCP schema : db.Schemas().GetSchemas(true)) {
        if (schema == nullptr)
            continue;

        // Only the test's own schemas. ECDbMeta in particular maps its classes onto the ec_ tables
        // themselves, so reading it would report every class in the file as an "instance".
        if (schema->IsStandardSchema() || schema->IsSystemSchema() ||
            schema->GetName().EqualsIAscii("ECDbMeta") || schema->GetName().EqualsIAscii("ECDbMap") ||
            schema->GetName().EqualsIAscii("ECDbChange") || schema->GetName().EqualsIAscii("ECDbFileInfo") ||
            schema->GetName().EqualsIAscii("ECDbSystem") || schema->GetName().EqualsIAscii("ECDbSchemaPolicies"))
            continue;

        for (ECClassCP ecClass : schema->GetClasses()) {
            if (ecClass == nullptr || (!ecClass->IsEntityClass() && !ecClass->IsRelationshipClass()))
                continue;

            // An abstract class with no table of its own, or one the mapper was told to skip, has
            // nothing to read. Asking anyway would fail to prepare and say nothing useful.
            // GetStrategy asserts on an empty result, so the empty check has to come first.
            // ExistingTable means the rows belong to something else and were never ours to lose.
            const auto mapStrategy = db.Schemas().GetClassMapStrategy(schema->GetName(), ecClass->GetName());
            if (mapStrategy.IsEmpty() ||
                mapStrategy.GetStrategy() == ClassMapStrategy::MapStrategy::NotMapped ||
                mapStrategy.GetStrategy() == ClassMapStrategy::MapStrategy::ExistingTable)
                continue;

            ECSqlStatement stmt;
            const Utf8PrintfString ecsql("SELECT * FROM ONLY [%s].[%s]", schema->GetName().c_str(), ecClass->GetName().c_str());
            if (stmt.Prepare(db, ecsql.c_str()) != ECSqlStatus::Success)
                continue;

            JsonECSqlSelectAdapter adapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
            bmap<Utf8String, Utf8String> rows;
            while (stmt.Step() == BE_SQLITE_ROW) {
                Json::Value row;
                if (SUCCESS != adapter.GetRow(row))
                    continue;

                // The id is the identity across the change - every other member is what we compare.
                const Utf8String id = row.isMember(ECJsonSystemNames::Id()) ? row[ECJsonSystemNames::Id()].asString() : Utf8String();
                if (id.empty())
                    continue;

                rows[id] = row.ToString();
            }

            if (!rows.empty())
                census.m_rowsByClass[ecClass->GetFullName()] = rows;
        }
    }
    return census;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::ExpectCensusPreserved(InstanceCensus const& before, InstanceCensus const& after, Utf8CP context,
                                                  std::vector<Utf8String> const& removedProperties) {
    auto isRemovalAllowed = [&removedProperties](Utf8StringCR className, Utf8StringCR property) {
        // The class name in the census is fully qualified; callers name the removal the way they
        // wrote it in the schema, so match on the unqualified end.
        for (auto const& allowed : removedProperties) {
            const auto dot = allowed.find_last_of('.');
            if (dot == Utf8String::npos)
                continue;
            if (!property.EqualsIAscii(allowed.substr(dot + 1).c_str()))
                continue;
            if (className.ContainsI(allowed.substr(0, dot).c_str()))
                return true;
        }
        return false;
    };

    for (auto const& classEntry : before.m_rowsByClass) {
        Utf8StringCR className = classEntry.first;
        const auto afterClass = after.m_rowsByClass.find(className);
        if (afterClass == after.m_rowsByClass.end()) {
            ADD_FAILURE() << context << ": every instance of " << className.c_str() << " is gone (" << (uint64_t)classEntry.second.size() << " of them)";
            continue;
        }

        for (auto const& rowEntry : classEntry.second) {
            const auto afterRow = afterClass->second.find(rowEntry.first);
            if (afterRow == afterClass->second.end()) {
                ADD_FAILURE() << context << ": " << className.c_str() << " instance " << rowEntry.first.c_str() << " is gone";
                continue;
            }

            if (rowEntry.second.Equals(afterRow->second))
                continue;

            Json::Value beforeRow, afterRowJson;
            if (!Json::Reader::Parse(rowEntry.second, beforeRow) || !Json::Reader::Parse(afterRow->second, afterRowJson)) {
                ADD_FAILURE() << context << ": " << className.c_str() << " instance " << rowEntry.first.c_str() << " changed and could not be parsed for a diff";
                continue;
            }

            for (auto const& member : beforeRow.getMemberNames()) {
                if (!afterRowJson.isMember(member)) {
                    if (!isRemovalAllowed(className, member))
                        ADD_FAILURE() << context << ": " << className.c_str() << "." << member.c_str() << " is gone from instance " << rowEntry.first.c_str();
                    continue;
                }
                if (beforeRow[member] != afterRowJson[member]) {
                    ADD_FAILURE() << context << ": " << className.c_str() << "." << member.c_str() << " changed on instance " << rowEntry.first.c_str()
                        << "\n    before: " << beforeRow[member].ToString().c_str()
                        << "\n     after: " << afterRowJson[member].ToString().c_str();
                }
            }
        }
    }
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
std::vector<ECDbChangeSet::Ptr> ECDbChangeTracker::TakeLocalChangesets() {
    auto taken = std::move(m_localChangesets);
    m_localChangesets.clear();
    return taken;
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
        if (!m_carriedDdl.empty()) {
            if (changeset == nullptr)
                changeset = ECDbChangeSet::Create((int)(m_localChangesets.size() + 1), operation, m_carriedDdl.c_str(), true);
            else
                changeset->PrependDDL(m_carriedDdl);
            m_carriedDdl.clear();
        }
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
void ECDbChangeSet::PrependDDL(Utf8StringCR ddl) {
    if (ddl.empty())
        return;
    Utf8String combined(ddl);
    if (!m_ddl.empty())
        combined.append(";").append(m_ddl);
    m_ddl = combined;
    m_hasSchemaChanges = true;
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
void ECDbChangeSet::DetermineSchemaSyncPrecedence() {
    m_ecChangesSupersedeBriefcase = false;
    if (m_ecdb == nullptr || !m_ecdb->Schemas().GetSchemaSync().IsEnabled())
        return;

    SchemaSync::DataVer txnDataVer = 0;
    for (auto& change : GetChanges()) {
        if (SchemaSync::IsLocalDbInfoChange(change) && SchemaSync::TryGetDataVersion(txnDataVer, change))
            break;
    }

    m_ecChangesSupersedeBriefcase = txnDataVer != 0 && txnDataVer > m_ecdb->Schemas().GetSchemaSync().GetInfo().GetDataVersion();
}

/*---------------------------------------------------------------------------------**//**
* Mirrors ConflictingRowDiffers in ChangesetTxns.cpp: the row being re-inserted under an id this
* briefcase already holds should be the row it already holds, since one authority produced both.
* Returns false when the comparison cannot be made.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ConflictingRowDiffers(DbR db, Changes::Change const& change) {
    const auto tableName = change.GetTableName();
    const auto columnCount = change.GetColumnCount();

    bvector<Utf8String> columns;
    if (!db.GetColumns(columns, tableName.c_str()) || (int) columns.size() != columnCount)
        return false;

    Utf8String sql("SELECT 1 FROM [");
    sql.append(tableName).append("] WHERE ");
    for (int i = 0; i < columnCount; ++i) {
        if (i > 0)
            sql.append(" AND ");

        sql.append("[").append(columns[i]).append("] IS ?"); // IS rather than = so that nulls compare
    }

    auto stmt = db.GetCachedStatement(sql.c_str());
    if (!stmt.IsValid())
        return false;

    for (int i = 0; i < columnCount; ++i) {
        const auto val = change.GetValue(i, Changes::Change::Stage::New);
        if (!val.IsValid() || BE_SQLITE_OK != stmt->BindDbValue(i + 1, val))
            return false;
    }
    return BE_SQLITE_ROW != stmt->Step();
}

/*---------------------------------------------------------------------------------**//**
* The rows an insert conflict could not write in place. FKNOACTION covers a whole apply rather than
* one change, so they go in as an apply of their own: it holds nothing but these rows, so the delete
* inside Replace has no children to cascade to and the row is back before the deferred check runs.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbChangeSet::ApplySupersedingRows(ECDbR db) {
    if (!m_hasSupersedingRows)
        return BE_SQLITE_OK;

    ChangeSet rows;
    auto rc = rows.FromChangeGroup(m_supersedingRows);
    if (rc != BE_SQLITE_OK)
        return rc;

    auto args = ApplyChangesArgs::Default()
        .SetFkNoAction(true)
        .SetConflictHandler([](ChangeStream::ConflictCause, Changes::Change) { return ChangeStream::ConflictResolution::Replace; });
    return rows.ApplyChanges(db, args);
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
    auto const* briefcase = dynamic_cast<TrackedECDb const*>(m_ecdb);

    if (cause == ChangeSet::ConflictCause::Conflict) {
        if (0 == ::strncmp(tableName, "ec_", 3) && m_ecdb != nullptr && m_ecdb->Schemas().GetSchemaSync().IsEnabled()) {
            // Replace would DELETE the existing row before re-inserting it, and every ec_ child table
            // is ON DELETE CASCADE - so the delete takes the row's children with it and the re-insert
            // restores only the parent. This harness never passes fkNoAction, so actions are always
            // live here; production turns them off for a schema changeset that updates or deletes
            // ec_cache_ rows, which an additive import does not produce.
            // Under schema sync the rows arriving here are normally ones this briefcase already holds,
            // because every briefcase gets its ids from the same authority. A differing row from a
            // superseding txn is written afterwards instead, by ApplySupersedingRows.
            if (briefcase != nullptr && briefcase->IsReplayingLocalChangesets() && m_ecChangesSupersedeBriefcase &&
                ConflictingRowDiffers(const_cast<ECDb&>(*m_ecdb), iter) && BE_SQLITE_OK == m_supersedingRows.AddChange(iter))
                m_hasSupersedingRows = true;

            return ChangeSet::ConflictResolution::Skip;
        }
        return ChangeSet::ConflictResolution::Replace;
    }
    if (cause == ChangeSet::ConflictCause::Data) {
        // Changesets reach the timeline in push order, which is not the order the sync db decided
        // things in. Both sides carry the sync db version they were produced against, so keep the
        // later one instead of whichever pushed first. Mirrors LocalChangeSet::_OnConflict, which
        // covers the same ground during rebase, and ChangesetFileReader::_OnConflict for the
        // be_Prop row itself.
        const auto isECRow = 0 == ::strncmp(tableName, "ec_", 3);
        if (briefcase != nullptr && briefcase->IsReplayingLocalChangesets() && (isECRow || SchemaSync::IsLocalDbInfoChange(iter)))
            return m_ecChangesSupersedeBriefcase ? ChangeSet::ConflictResolution::Replace : ChangeSet::ConflictResolution::Skip;

        if (SchemaSync::IsLocalDbInfoChange(iter) && m_ecdb != nullptr) {
            SchemaSync::DataVer incomingDataVer = 0;
            const auto heldDataVer = m_ecdb->Schemas().GetSchemaSync().GetInfo().GetDataVersion();
            return SchemaSync::TryGetDataVersion(incomingDataVer, iter) && incomingDataVer > heldDataVer
                ? ChangeSet::ConflictResolution::Replace
                : ChangeSet::ConflictResolution::Skip;
        }

        // A briefcase holding local changes keeps its own ec_ rows: it got them from the sync db,
        // which decides them, so an incoming changeset carrying a different "before" value has
        // nothing to say about them. Mirrors ChangesetFileReader::_OnConflict, which does this in
        // the HasPendingTxns() branch. Without local changes the conflict falls through to Replace
        // below, exactly as it does there.
        if (isECRow && briefcase != nullptr && briefcase->HasLocalChangesets())
            return ChangeSet::ConflictResolution::Skip;
    }
    if (cause == ChangeSet::ConflictCause::ForeignKey) {
        // Note: No current or conflicting row information is provided if it's a FKey conflict
        // Since we abort on FKey conflicts, always try and provide details about the error
        int nConflicts = 0;
        result = iter.GetFKeyConflicts(&nConflicts);
        BeAssert(result == BE_SQLITE_OK);
        printf("[mockhub] %d foreign key conflicts applying a changeset - aborting the merge\n", nConflicts);
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
* Applies the ec_ part of a changeset in effective opcode order - deletes, then updates, then
* inserts. A ChangeGroup scrambles operation order within a table, so on an inverted apply an
* insert can land ahead of the delete that frees its unique name and the conflict handler skips
* over the collision. Ported from TxnManager's ApplySchemaChangesInOrder; sqlite flips opcodes
* while inverting, so the raw order has to be the mirror of the wanted one.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult ApplySchemaChangesInOrder(ECDbChangeSet& changeset, ECDbR db, ApplyChangesArgs args) {
    const bool invert = args.GetInvert();

    ChangeGroup groups[3]; // [0]=Delete, [1]=Update, [2]=Insert
    for (auto change : Changes(changeset, false)) {
        if (!ApplyChangesArgs::IsSchemaChange(change))
            continue;
        auto rc = BE_SQLITE_OK;
        switch (change.GetOpcode()) {
            case DbOpcode::Delete: rc = groups[0].AddChange(change); break;
            case DbOpcode::Update: rc = groups[1].AddChange(change); break;
            case DbOpcode::Insert: rc = groups[2].AddChange(change); break;
            default: break;
        }
        if (rc != BE_SQLITE_OK)
            return rc;
    }

    const int order[] = { invert ? 2 : 0, 1, invert ? 0 : 2 };

    ChangeSet ordered;
    for (int idx : order) {
        ChangeSet part;
        auto rc = part.FromChangeGroup(groups[idx]);
        if (rc != BE_SQLITE_OK)
            return rc;
        if (part._IsEmpty())
            continue;
        // ReadFrom concatenates raw bytes; ConcatenateWith would run it through a ChangeGroup again.
        auto reader = part._GetReader();
        rc = ordered.ReadFrom(*reader);
        if (rc != BE_SQLITE_OK)
            return rc;
    }

    if (ordered._IsEmpty())
        return BE_SQLITE_OK;

    // ordered is a plain ChangeSet whose own _OnConflict aborts, so hand conflicts back to the
    // original stream and keep its ec_ rules.
    args.SetConflictHandler([&changeset](ChangeStream::ConflictCause cause, Changes::Change change) {
        return changeset.OnConflict(cause, change);
    });
    return ordered.ApplyChanges(db, args);
}

/*---------------------------------------------------------------------------------**//**
* Applies one changeset the way TxnManager::ApplyChanges does: schema rows first and in opcode
* order, then the post-schema hook, then the data rows. A briefcase rebuilds its tables from the
* ec_ rows a changeset carries, so the hook has to run between the two passes or the data rows
* arrive at a table that does not exist yet and are dropped. That holds even when the changeset
* also carries DDL, since applying it is best effort. Merge only, matching TxnManager gating it on
* TxnAction::Merge: the reverse pass leaves the tables alone, the reinstate pass does not.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult ApplyOneChangeset(ECDbChangeSet& changeset, ECDbR db, bool invert, bool isMerge) {
    auto args = [&]() { return ApplyChangesArgs::Default().SetInvert(invert).SetIgnoreNoop(true); };
    if (!changeset.HasSchemaChanges())
        return changeset.ApplyChanges(db, args());

    auto rc = ApplySchemaChangesInOrder(changeset, db, args().ApplyOnlySchemaChanges());
    if (rc != BE_SQLITE_OK)
        return rc;

    if (isMerge) {
        rc = db.AfterSchemaChangeSetApplied();
        if (rc != BE_SQLITE_OK) {
            printf("[mockhub] AfterSchemaChangeSetApplied failed on '%s': %s (%s)\n", changeset.GetOperation().c_str(),
                BeSQLiteLib::GetErrorName(rc), db.GetLastError().c_str());
            return rc;
        }
    }

    return changeset.ApplyChanges(db, args().ApplyOnlyDataChanges());
}

/*---------------------------------------------------------------------------------**//**
* A rebase failure carries no usable text - a deferred foreign key violation surfaces as a bare
* BE_SQLITE_CONSTRAINT from the apply - so name the phase and the dangling rows on the way out.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ReportRebaseFailure(ECDbR db, Utf8CP phase, ECDbChangeSet const& changeset, DbResult rc) {
    printf("[mockhub] rebase failed in %s on '%s': %s (%s)\n", phase, changeset.GetOperation().c_str(),
        BeSQLiteLib::GetErrorName(rc), db.GetLastError().c_str());

    Statement stmt;
    if (stmt.Prepare(db, "PRAGMA main.foreign_key_check") != BE_SQLITE_OK)
        return;
    while (stmt.Step() == BE_SQLITE_ROW)
        printf("[mockhub]   dangling: table=%s rowid=%lld parent=%s fkid=%d\n",
            stmt.GetValueText(0), stmt.GetValueInt64(1), stmt.GetValueText(2), stmt.GetValueInt(3));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TrackedECDb::RebaseOntoIncoming(std::vector<ECDbChangeSet*> const& incoming) {
    // Follows TxnManager: reverse the local txns, apply the incoming changesets against a briefcase with
    // nothing local, then reinstate each local txn and re-record what actually landed. A local changeset's
    // DDL is left alone throughout - it cannot be reversed, and PullMergeRebaseReinstateTxn skips it too.
    auto localChangesets = m_tracker->TakeLocalChangesets();
    m_tracker->EnableTracking(false);

    for (auto it = localChangesets.rbegin(); it != localChangesets.rend(); ++it) {
        (*it)->SetECDb(*this);
        auto rc = ApplyOneChangeset(**it, *this, true, false);
        if (rc != BE_SQLITE_OK) {
            ReportRebaseFailure(*this, "reverse", **it, rc);
            return rc;
        }
    }

    for (auto& changesetToApply : incoming) {
        changesetToApply->SetECDb(*this);
#ifdef TRACE_CS
        PrintChangeSet::Print(*this, *changesetToApply);
#endif
        // DDL first, so a widened table can take the row changes that come with it - the same order
        // and the same ignore-failures rule as TxnManager::ApplyDdlChanges.
        for (auto& ddl : changesetToApply->GetDDLs())
            TryExecuteSql(ddl.c_str());

        const auto kIsMerge = true;
        auto rc = ApplyOneChangeset(*changesetToApply, *this, false, kIsMerge);
        if (rc != BE_SQLITE_OK) {
            ReportRebaseFailure(*this, "apply incoming", *changesetToApply, rc);
            return rc;
        }
    }

    // TxnManager runs the data-side hook too. It refreshes the profile version and resets the instance
    // id sequence here; the overflow-row catch-up is deferred, because the rows the local changesets
    // bring back are still reversed at this point.
    bool incomingChangedSchema = false;
    for (auto& changeset : incoming)
        incomingChangedSchema = incomingChangedSchema || changeset->HasSchemaChanges();

    const auto kDeferInstanceUpgrade = true;
    auto rc = AfterDataChangeSetApplied(incomingChangedSchema, kDeferInstanceUpgrade);
    if (rc != BE_SQLITE_OK) {
        printf("[mockhub] rebase failed in AfterDataChangeSetApplied: %s (%s)\n", BeSQLiteLib::GetErrorName(rc), GetLastError().c_str());
        return rc;
    }

    m_tracker->EnableTracking(true);
    m_replayingLocalChangesets = true;
    for (auto& local : localChangesets) {
        local->SetECDb(*this);
        local->DetermineSchemaSyncPrecedence();
        // TxnManager::PullMergeRebaseReinstateTxn passes TxnAction::Merge here, so a reinstated local
        // schema txn gets the post-schema hook as well. The incoming pass repopulated the ec_cache_
        // tables while these rows were reversed out, so without it a class this briefcase imported
        // itself comes back with no ec_cache_ClassHasTables row - its storage description then has no
        // horizontal partition and the first ECSql against it dereferences an empty vector.
        const auto kReinstateIsMerge = true;
        rc = ApplyOneChangeset(*local, *this, false, kReinstateIsMerge);
        if (rc == BE_SQLITE_OK)
            rc = local->ApplySupersedingRows(*this);
        if (rc == BE_SQLITE_OK) {
            m_tracker->CarryDdlIntoNextChangeset(local->GetDDL());
            rc = SaveChanges(local->GetOperation().c_str());
        }
        if (rc != BE_SQLITE_OK) {
            ReportRebaseFailure(*this, "replay local", *local, rc);
            m_replayingLocalChangesets = false;
            return rc;
        }
    }
    m_replayingLocalChangesets = false;

    // Mirrors TxnManager::PullMergeRebaseEnd. The catch-up runs once, after the local changesets are
    // back, so rows that were sitting in an unpushed changeset get the overflow row a schema change
    // spilled their class into. It has to be tracked, or the rows never reach anybody else.
    if (incomingChangedSchema) {
        if (SUCCESS != Schemas().UpgradeECInstances()) {
            printf("[mockhub] rebase failed to upgrade instances after replaying local changesets\n");
            return BE_SQLITE_ERROR;
        }
        rc = SaveChanges("upgrade instances after rebase");
        if (rc != BE_SQLITE_OK) {
            printf("[mockhub] rebase failed to save the upgraded instances: %s\n", BeSQLiteLib::GetErrorName(rc));
            return rc;
        }
    }
    return BE_SQLITE_OK;
}

/*---------------------------------------------------------------------------------**//**
* The mock hub is in-process, so unlike DgnDb this can answer both halves - whether anything is
* held back, and whether anything pushed by others is missing.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool TrackedECDb::_IsLevelWithTimeline() {
    if (m_hub == nullptr)
        return true;

    return !HasLocalChangesets() && m_changesetId == m_hub->GetTipChangesetId();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TrackedECDb::PullMergePush(Utf8CP comment) {
    if (m_hub == nullptr || m_tracker == nullptr) {
        return BE_SQLITE_ERROR;
    }

#ifdef TRACE_CS
    auto cancelTrace = GetTraceStmtEvent().AddListener([](TraceContext const& ctx, Utf8CP sql) {
        printf("[STMT] %s\n", ctx.GetExpandedSql().c_str());

    });
#endif
    SaveChanges(); // a rebase reverses committed changesets, so nothing may be left uncommitted

    auto changesetsToApply = m_hub->Query(m_changesetId + 1);
    if (!changesetsToApply.empty()) {
        auto rc = RebaseOntoIncoming(changesetsToApply);
        if (rc != BE_SQLITE_OK) {
#ifdef TRACE_CS
            cancelTrace();
#endif
            return rc;
        }
        // Query() hands back everything from m_changesetId + 1 to the tip.
        m_changesetId += (int)changesetsToApply.size();
    }

    if (!m_tracker->GetLocalChangesets().empty()){
        auto changeset = m_tracker->MakeChangeset(true, comment);
        if (changeset == nullptr) {
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