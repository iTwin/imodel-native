/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ECDb/SchemaImportAdapter.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct SchemaImportHistoryTests : ECDbTestFixture, testing::WithParamInterface<bool> {
    struct ExportedSchema {
        std::filesystem::path file;
        Utf8String digest;
        SchemaKey key;
    };

    // Resolve references against this snapshot only, never a future export or an older database cache.
    struct SnapshotLocater : IECSchemaLocater {
        bmap<Utf8String, ExportedSchema, CompareIUtf8Ascii> schemas;

        Utf8String GetDescription() const override { return "Exported changeset schema snapshot"; }

        ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType match, ECSchemaReadContextR context) override {
            auto found = schemas.find(key.GetName());
            if (found == schemas.end() || !found->second.key.Matches(key, match)) {
                ADD_FAILURE() << "Snapshot cannot resolve " << key.GetFullSchemaName().c_str();
                return nullptr;
            }
            BeFileName file(found->second.file.wstring().c_str());
            ECSchemaPtr schema;
            auto status = ECSchema::ReadFromXmlFile(schema, file.GetName(), context);
            if (status != SchemaReadStatus::Success) {
                ADD_FAILURE() << "Cannot read " << file.GetNameUtf8().c_str() << "; SchemaReadStatus=" << static_cast<int>(status);
                return nullptr;
            }
            if (!schema->GetSchemaKey().Matches(found->second.key, SchemaMatchType::Exact)) {
                ADD_FAILURE() << "Schema identity differs from manifest: " << found->second.file.string();
                return nullptr;
            }
            return schema;
        }
    };

    static Utf8String GetEnvironment(char const* name) {
        Utf8String result;
#if defined(BENTLEY_WIN32)
        char* value = nullptr;
        size_t length = 0;
        if (0 == _dupenv_s(&value, &length, name) && value != nullptr) {
            result.assign(value);
            free(value);
        }
#else
        if (auto value = std::getenv(name))
            result.assign(value);
#endif
        return result;
    }

    static void ReadChangesetDirectories(std::map<int64_t, std::filesystem::path>& changesets) {
        auto directory = GetEnvironment("ECDB_SCHEMA_CHANGESET_DIR");
        ASSERT_FALSE(directory.empty()) << "Set ECDB_SCHEMA_CHANGESET_DIR to the exported schema history directory.";
        std::filesystem::path root(directory.c_str());
        ASSERT_TRUE(std::filesystem::is_directory(root)) << directory.c_str();
        for (auto const& entry : std::filesystem::directory_iterator(root)) {
            if (!entry.is_directory())
                continue;
            auto name = entry.path().filename().string();
            ASSERT_FALSE(name.empty());
            ASSERT_TRUE(std::all_of(name.begin(), name.end(), [](char c) { return c >= '0' && c <= '9'; })) << name;
            ASSERT_TRUE(changesets.emplace(std::stoll(name), entry.path()).second) << "Duplicate index: " << name;
        }
        ASSERT_FALSE(changesets.empty());
        ASSERT_EQ(0, changesets.begin()->first) << "The seed schema export at index 0 is required.";
    }

    static void ReadManifest(int64_t index, std::filesystem::path const& directory, int64_t previousIndex, SnapshotLocater const& previous, SnapshotLocater& snapshot) {
        auto manifestPath = directory / "manifest.json";
        std::ifstream stream(manifestPath, std::ios::binary);
        ASSERT_TRUE(stream.is_open()) << manifestPath.string();
        std::string text((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        ASSERT_FALSE(stream.bad()) << manifestPath.string();
        BeJsDocument manifest(text.c_str());
        ASSERT_FALSE(manifest.hasParseError()) << manifestPath.string();
        ASSERT_TRUE(manifest.isObject());
        ASSERT_EQ(1, manifest["manifestVersion"].asInt());
        ASSERT_EQ(index, manifest["changesetIndex"].asInt64());
        if (previousIndex < 0)
            ASSERT_TRUE(manifest["previousSchemaChangesetIndex"].isNull());
        else
            ASSERT_EQ(previousIndex, manifest["previousSchemaChangesetIndex"].asInt64()) << "A preceding schema snapshot is missing.";
        ASSERT_TRUE(manifest["removedSchemas"].isArray());
        ASSERT_EQ(0, manifest["removedSchemas"].size()) << "Schema removals cannot be replayed by ImportSchemas.";
        ASSERT_TRUE(manifest["schemas"].isArray());
        ASSERT_GT(manifest["schemas"].size(), 0);

        for (uint32_t i = 0; i < manifest["schemas"].size(); ++i) {
            auto item = manifest["schemas"][i];
            ASSERT_TRUE(item["schemaName"].isString());
            ASSERT_TRUE(item["version"].isString());
            ASSERT_TRUE(item["fileName"].isString());
            ASSERT_TRUE(item["sha256"].isString());
            Utf8String name(item["schemaName"].asCString());
            auto fileName = std::filesystem::path(item["fileName"].asCString());
            ASSERT_EQ(fileName, fileName.filename()) << "Expected an unqualified schema file name.";
            ExportedSchema exported;
            exported.file = directory / "files" / fileName;
            exported.digest = item["sha256"].asCString();
            ASSERT_EQ(ECObjectsStatus::Success, SchemaKey::ParseSchemaFullName(exported.key, Utf8PrintfString("%s.%s", name.c_str(), item["version"].asCString()).c_str()));
            if (!std::filesystem::is_regular_file(exported.file)) {
                auto found = previous.schemas.find(name);
                ASSERT_TRUE(found != previous.schemas.end()) << "Missing retained schema file: " << exported.file.string();
                ASSERT_EQ(exported.digest, found->second.digest) << "Missing changed schema file: " << exported.file.string();
                ASSERT_TRUE(exported.key.Matches(found->second.key, SchemaMatchType::Exact));
                exported.file = found->second.file;
            }
            ASSERT_EQ(item["byteLength"].asUInt64(), std::filesystem::file_size(exported.file)) << exported.file.string();
            ASSERT_TRUE(snapshot.schemas.emplace(name, std::move(exported)).second) << "Duplicate schema: " << name.c_str();
        }
    }

    static void PrintPlanHistogram(SchemaImportPlan const& plan) {
        std::map<Utf8String, std::pair<size_t, size_t>> histogram; // command [facet] -> (standalone, grouped)
        std::map<Utf8String, Utf8String> examples;
        for (auto const& step : plan.GetSteps()) {
            BeJsDocument commands;
            commands.Parse(step.m_commands);
            commands.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst command) {
                Utf8String key = command["command"].asString();
                key.append(" [").append(step.m_facet).append("]");
                // Type, enumeration, KoQ and navigation changes cannot join class-creation groups.
                for (auto member : {"type", "enum", "koq"})
                    if (key.StartsWith("AlterProperty") && command["args"].isMember(member))
                        key.append(" +").append(member);
                if (command["args"]["kind"].asString().EqualsIAscii("navigation"))
                    key.append(" +navigation");
                auto& counts = histogram[key];
                ++(step.m_group ? counts.second : counts.first);
                if (!step.m_group && examples.count(key) == 0)
                    examples[key] = command["args"].Stringify().substr(0, 160);
                return false;
            });
        }
        printf("Plan: %zu steps\n", plan.GetSteps().size());
        for (auto const& entry : histogram)
            printf("  %-48s standalone=%zu grouped=%zu\n", entry.first.c_str(), entry.second.first, entry.second.second);
        for (auto const& entry : examples)
            printf("  e.g. %s: %s\n", entry.first.c_str(), entry.second.c_str());
        fflush(stdout);
    }
};

// ECDB_SCHEMA_CHANGESET_DIR=/path/to/test-sc ./ECDbTest --gtest_also_run_disabled_tests --gtest_filter='AdHoc/SchemaImportHistoryTests.*'
TEST_P(SchemaImportHistoryTests, DISABLED_ReplayExportedChangesets) {
    auto scanOption = GetEnvironment("ECDB_SCHEMA_CONTINUE_AFTER_FAILURE");
    ASSERT_TRUE(scanOption.empty() || scanOption == "1");
    bool scan = scanOption == "1";
    ASSERT_TRUE(!scan || GetParam()) << "Failure scanning requires the SchemaEditor test instance.";
    std::map<int64_t, std::filesystem::path> changesets;
    ReadChangesetDirectories(changesets);
    ASSERT_FALSE(HasFailure());
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb(GetParam() ? "SchemaHistoryAdapter.ecdb" : "SchemaHistoryNative.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("CREATE VIRTUAL TABLE dgn_SpatialIndex USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    printf("Schema history: %s, %zu snapshots, database %s\n", GetParam() ? "SchemaImportAdapter" : "Native", changesets.size(), m_ecdb.GetDbFileName());
    fflush(stdout);

    SnapshotLocater previous;
    int64_t previousIndex = -1;
    size_t completed = 0;
    size_t processed = 0, nativeRecoveries = 0, freshBaselines = 0;
    bvector<Utf8String> scanFailures;
    double totalImportSeconds = 0.0;
    StopWatch replayTimer(true);
    auto options = SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade | SchemaImportOptions::AllowMajorSchemaUpgradeForDynamicSchemas;
    for (auto const& changeset : changesets) {
        StopWatch snapshotTimer(true);
        SCOPED_TRACE(testing::Message() << "Changeset index " << changeset.first << ", last successful index " << previousIndex);
        SnapshotLocater snapshot;
        ReadManifest(changeset.first, changeset.second, previousIndex, previous, snapshot);
        ASSERT_FALSE(HasFailure());

        auto context = ECSchemaReadContext::CreateContext();
        context->AddSchemaLocater(snapshot);
        bvector<ECSchemaCP> schemas;
        for (auto const& item : snapshot.schemas) {
            auto key = item.second.key;
            auto schema = context->LocateSchema(key, SchemaMatchType::Exact);
            ASSERT_TRUE(schema.IsValid()) << item.second.file.string();
            schemas.push_back(schema.get());
        }
        ASSERT_FALSE(HasFailure()) << "Schema reading failed; no import attempted.";
        printf("Changeset %lld: importing %zu schemas\n", static_cast<long long>(changeset.first), schemas.size());
        fflush(stdout);
        StopWatch importTimer(true);
        bool adapterSucceeded = true;
        if (GetParam()) {
            SchemaImportCompletion completion;
            auto result = SchemaImportAdapter::ImportSchemas(m_ecdb, schemas, &completion, options);
            BeJsDocument diagnostic;
            result.ToJson(diagnostic);
            if (!result.IsOk()) {
                EXPECT_FALSE(completion.IsPending());
                RecordProperty("FailedChangesetIndex", std::to_string(changeset.first));
                RecordProperty("SchemaImportAdapterResult", diagnostic.Stringify().c_str());
            }
            if (!result.IsOk() && scan) {
                adapterSucceeded = false;
                importTimer.Stop();
                scanFailures.push_back(Utf8PrintfString("Changeset %lld: %s", static_cast<long long>(changeset.first), diagnostic.Stringify().c_str()));
                printf("SCAN adapter FAILED %lld after %.3f s: %s\n", static_cast<long long>(changeset.first),
                       importTimer.GetElapsedSeconds(), diagnostic.Stringify().c_str());
                fflush(stdout);
                ASSERT_FALSE(completion.IsPending());
                ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AbandonChanges());
                ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

                StopWatch recoveryTimer(true);
                auto recover = [&]() {
                    TestIssueListener listener;
                    m_ecdb.AddIssueListener(listener);
                    auto status = m_ecdb.Schemas().ImportSchemas(schemas, options);
                    m_ecdb.RemoveIssueListener();
                    if (status != SUCCESS) {
                        printf("SCAN native recovery FAILED %lld\n", static_cast<long long>(changeset.first));
                        for (auto const& issue : listener.m_issues)
                            printf("  %s\n", issue.message.c_str());
                        fflush(stdout);
                    }
                    return status;
                };
                if (recover() != SUCCESS) {
                    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.AbandonChanges());
                    auto fileName = Utf8PrintfString("SchemaHistoryRecovery_%lld.ecdb", static_cast<long long>(changeset.first));
                    ASSERT_EQ(BE_SQLITE_OK, SetupECDb(fileName.c_str()));
                    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("CREATE VIRTUAL TABLE dgn_SpatialIndex USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"));
                    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
                    printf("SCAN fresh native baseline %lld; prior physical mapping history is not preserved\n", static_cast<long long>(changeset.first));
                    fflush(stdout);
                    ASSERT_EQ(SUCCESS, recover()) << "Cannot establish the next snapshot's baseline.";
                    ++freshBaselines;
                }
                ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
                ++nativeRecoveries;
                printf("SCAN native recovery committed %lld in %.3f s\n", static_cast<long long>(changeset.first), recoveryTimer.GetCurrentSeconds());
                fflush(stdout);
            } else {
                ASSERT_TRUE(result.IsOk()) << diagnostic.Stringify().c_str();
            }
            if (adapterSucceeded && completion.IsPending())
                ASSERT_EQ(BE_SQLITE_OK, completion.Commit());
        } else {
            TestIssueListener listener;
            m_ecdb.AddIssueListener(listener);
            auto status = m_ecdb.Schemas().ImportSchemas(schemas, options);
            m_ecdb.RemoveIssueListener();
            if (status != SUCCESS) {
                printf("Changeset %lld: import FAILED after %.3f s; %zu/%zu snapshots completed, replay elapsed %.3f s\n",
                       static_cast<long long>(changeset.first), importTimer.GetCurrentSeconds(), completed, changesets.size(), replayTimer.GetCurrentSeconds());
                fflush(stdout);
                RecordProperty("FailedChangesetIndex", std::to_string(changeset.first));
                for (auto const& issue : listener.m_issues)
                    ADD_FAILURE() << issue.message.c_str();
                EXPECT_EQ(BE_SQLITE_OK, m_ecdb.AbandonChanges());
            }
            ASSERT_EQ(SUCCESS, status);
            ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
        }
        if (adapterSucceeded) {
            importTimer.Stop();
            totalImportSeconds += importTimer.GetElapsedSeconds();
            ++completed;
            RecordProperty("CompletedImportAndCommitSeconds", Utf8PrintfString("%.3f", totalImportSeconds).c_str());
            printf("Changeset %lld: import and commit %.3f s, cumulative %.3f s\n",
                   static_cast<long long>(changeset.first), importTimer.GetElapsedSeconds(), totalImportSeconds);
            fflush(stdout);
        }
        ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
        for (auto const& item : snapshot.schemas) {
            auto schema = m_ecdb.Schemas().GetSchema(item.first);
            ASSERT_NE(nullptr, schema) << item.first.c_str();
            EXPECT_GE(schema->GetSchemaKey().CompareByVersion(item.second.key), 0) << item.first.c_str();
        }
        for (auto query : {"SELECT ECInstanceId FROM bis.Element", "SELECT ECInstanceId FROM bis.Model", "SELECT ECInstanceId FROM bis.SpatialIndex"}) {
            ECSqlStatement statement;
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, query)) << query;
            ASSERT_EQ(BE_SQLITE_DONE, statement.Step()) << query;
        }
        ASSERT_FALSE(HasFailure());
        previous.schemas = std::move(snapshot.schemas);
        previousIndex = changeset.first;
        ++processed;
        if (adapterSucceeded)
            RecordProperty("LastSuccessfulChangesetIndex", std::to_string(previousIndex));
        RecordProperty("CompletedSnapshots", std::to_string(completed));
        RecordProperty("ProcessedSnapshots", std::to_string(processed));
        RecordProperty("FailedSnapshots", std::to_string(scanFailures.size()));
        RecordProperty("NativeRecoveries", std::to_string(nativeRecoveries));
        RecordProperty("FreshBaselines", std::to_string(freshBaselines));
        printf("Changeset %lld: committed and reopened (%zu/%zu snapshots, %s), snapshot %.3f s, replay elapsed %.3f s\n",
               static_cast<long long>(previousIndex), processed, changesets.size(), adapterSucceeded ? (GetParam() ? "adapter" : "native") : "native recovery",
               snapshotTimer.GetCurrentSeconds(), replayTimer.GetCurrentSeconds());
        fflush(stdout);
    }
    if (scan) {
        printf("SCAN complete: %zu/%zu attempted, %zu adapter successes, %zu adapter failures, %zu native recoveries, %zu fresh baselines\n",
               processed, changesets.size(), completed, scanFailures.size(), nativeRecoveries, freshBaselines);
        fflush(stdout);
        for (auto const& failure : scanFailures)
            ADD_FAILURE() << failure.c_str();
    }
}

// Seeds the database natively with the focus schema's transitive references from the final snapshot, then imports only
// the focus schema and reports the import time. Compare the Native and SchemaEditor instances.
// ECDB_SCHEMA_CHANGESET_DIR=/path/to/test-sc ECDB_SCHEMA_FOCUS=BuildingDataGroup ./ECDbTest --gtest_also_run_disabled_tests --gtest_filter='AdHoc/SchemaImportHistoryTests.DISABLED_ImportFocusSchema/*'
TEST_P(SchemaImportHistoryTests, DISABLED_ImportFocusSchema) {
    auto focusName = GetEnvironment("ECDB_SCHEMA_FOCUS");
    ASSERT_FALSE(focusName.empty()) << "Set ECDB_SCHEMA_FOCUS to the schema name to import.";
    std::map<int64_t, std::filesystem::path> changesets;
    ReadChangesetDirectories(changesets);
    ASSERT_FALSE(HasFailure());
    SnapshotLocater snapshot;
    int64_t previousIndex = -1;
    for (auto const& changeset : changesets) {
        SnapshotLocater current;
        ReadManifest(changeset.first, changeset.second, previousIndex, snapshot, current);
        ASSERT_FALSE(HasFailure()) << "Changeset index " << changeset.first;
        snapshot.schemas = std::move(current.schemas);
        previousIndex = changeset.first;
    }
    auto found = snapshot.schemas.find(focusName);
    ASSERT_TRUE(found != snapshot.schemas.end()) << focusName.c_str() << " is not in the final snapshot " << previousIndex;

    auto context = ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(snapshot);
    auto focusKey = found->second.key;
    auto focus = context->LocateSchema(focusKey, SchemaMatchType::Exact);
    ASSERT_TRUE(focus.IsValid());
    bvector<ECSchemaCP> references;
    bset<ECSchemaCP> visited;
    std::function<void(ECSchemaCR)> collect = [&](ECSchemaCR schema) {
        for (auto const& reference : schema.GetReferencedSchemas()) {
            if (visited.insert(reference.second.get()).second) {
                collect(*reference.second);
                references.push_back(reference.second.get());
            }
        }
    };
    collect(*focus);
    ASSERT_EQ(visited.end(), visited.find(focus.get())) << "Focus schema must not reference itself.";

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb(GetParam() ? "FocusAdapter.ecdb" : "FocusNative.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("CREATE VIRTUAL TABLE dgn_SpatialIndex USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"));
    auto options = SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade | SchemaImportOptions::AllowMajorSchemaUpgradeForDynamicSchemas;
    {
        TestIssueListener listener;
        m_ecdb.AddIssueListener(listener);
        auto status = m_ecdb.Schemas().ImportSchemas(references, options);
        m_ecdb.RemoveIssueListener();
        for (auto const& issue : listener.m_issues)
            EXPECT_EQ(SUCCESS, status) << issue.message.c_str();
        ASSERT_EQ(SUCCESS, status);
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    size_t classCount = 0, propertyCount = 0;
    for (auto ecClass : focus->GetClasses()) {
        ++classCount;
        propertyCount += ecClass->GetPropertyCount(false);
    }
    printf("Focus %s: %zu classes, %zu local properties, %zu seeded references; importing with %s\n", focus->GetFullSchemaName().c_str(),
           classCount, propertyCount, references.size(), GetParam() ? "SchemaImportAdapter" : "Native");
    fflush(stdout);

    bvector<ECSchemaCP> schemas{focus.get()};
    StopWatch timer(true);
    if (GetParam()) {
        SchemaImportCompletion completion;
        SchemaImportPlan plan;
        auto result = SchemaImportAdapter::BuildPlan(m_ecdb, schemas, plan, options);
        if (result.IsOk()) {
            PrintPlanHistogram(plan);
            result = SchemaImportAdapter::ExecutePlan(m_ecdb, plan, &completion);
        }
        BeJsDocument diagnostic;
        result.ToJson(diagnostic);
        ASSERT_TRUE(result.IsOk()) << diagnostic.Stringify().c_str();
        if (completion.IsPending())
            ASSERT_EQ(BE_SQLITE_OK, completion.Commit());
    } else {
        TestIssueListener listener;
        m_ecdb.AddIssueListener(listener);
        auto status = m_ecdb.Schemas().ImportSchemas(schemas, options);
        m_ecdb.RemoveIssueListener();
        for (auto const& issue : listener.m_issues)
            EXPECT_EQ(SUCCESS, status) << issue.message.c_str();
        ASSERT_EQ(SUCCESS, status);
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    }
    timer.Stop();
    printf("Focus %s imported in %.3f s\n", focusName.c_str(), timer.GetElapsedSeconds());
    fflush(stdout);
    RecordProperty("ImportSeconds", Utf8PrintfString("%.3f", timer.GetElapsedSeconds()).c_str());

    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    auto imported = m_ecdb.Schemas().GetSchema(focusName);
    ASSERT_NE(nullptr, imported);
    EXPECT_EQ(0, imported->GetSchemaKey().CompareByVersion(focusKey));
    size_t prepared = 0;
    for (auto ecClass : imported->GetClasses()) {
        if (!ecClass->IsEntityClass() || ecClass->GetClassModifier() == ECClassModifier::Abstract)
            continue;
        ECSqlStatement statement;
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, Utf8PrintfString("SELECT * FROM %s", ecClass->GetECSqlName().c_str()).c_str())) << ecClass->GetFullName();
        ASSERT_EQ(BE_SQLITE_DONE, statement.Step()) << ecClass->GetFullName();
        ++prepared;
    }
    printf("Focus %s: prepared SELECT * for %zu entity classes\n", focusName.c_str(), prepared);
}

// Seeds the database natively with the snapshot preceding ECDB_SCHEMA_UPGRADE_TO, then imports that changeset's snapshot
// and reports the upgrade time. Compare the Native and SchemaEditor instances.
// ECDB_SCHEMA_CHANGESET_DIR=/path/to/test-sc ECDB_SCHEMA_UPGRADE_TO=149 ./ECDbTest --gtest_also_run_disabled_tests --gtest_filter='AdHoc/SchemaImportHistoryTests.DISABLED_ImportChangesetUpgrade/*'
TEST_P(SchemaImportHistoryTests, DISABLED_ImportChangesetUpgrade) {
    auto target = GetEnvironment("ECDB_SCHEMA_UPGRADE_TO");
    ASSERT_FALSE(target.empty()) << "Set ECDB_SCHEMA_UPGRADE_TO to the changeset index to import.";
    auto targetIndex = std::stoll(target.c_str());
    std::map<int64_t, std::filesystem::path> changesets;
    ReadChangesetDirectories(changesets);
    ASSERT_FALSE(HasFailure());
    ASSERT_TRUE(changesets.count(targetIndex) != 0 && targetIndex != 0) << "No non-seed changeset " << target.c_str();
    SnapshotLocater previous, current;
    int64_t previousIndex = -1;
    for (auto const& changeset : changesets) {
        SnapshotLocater snapshot;
        ReadManifest(changeset.first, changeset.second, previousIndex, current, snapshot);
        ASSERT_FALSE(HasFailure()) << "Changeset index " << changeset.first;
        previous.schemas = std::move(current.schemas);
        current.schemas = std::move(snapshot.schemas);
        if (changeset.first == targetIndex)
            break;
        previousIndex = changeset.first;
    }
    auto locate = [](SnapshotLocater& snapshot, ECSchemaReadContextPtr& context, bvector<ECSchemaCP>& schemas) {
        context = ECSchemaReadContext::CreateContext();
        context->AddSchemaLocater(snapshot);
        for (auto const& item : snapshot.schemas) {
            auto key = item.second.key;
            auto schema = context->LocateSchema(key, SchemaMatchType::Exact);
            ASSERT_TRUE(schema.IsValid()) << item.second.file.string();
            schemas.push_back(schema.get());
        }
    };

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb(GetParam() ? "UpgradeAdapter.ecdb" : "UpgradeNative.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("CREATE VIRTUAL TABLE dgn_SpatialIndex USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"));
    auto options = SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade | SchemaImportOptions::AllowMajorSchemaUpgradeForDynamicSchemas;
    {
        ECSchemaReadContextPtr context;
        bvector<ECSchemaCP> schemas;
        locate(previous, context, schemas);
        ASSERT_FALSE(HasFailure());
        TestIssueListener listener;
        m_ecdb.AddIssueListener(listener);
        auto status = m_ecdb.Schemas().ImportSchemas(schemas, options);
        m_ecdb.RemoveIssueListener();
        for (auto const& issue : listener.m_issues)
            EXPECT_EQ(SUCCESS, status) << issue.message.c_str();
        ASSERT_EQ(SUCCESS, status);
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ECSchemaReadContextPtr context;
    bvector<ECSchemaCP> schemas;
    locate(current, context, schemas);
    ASSERT_FALSE(HasFailure());
    printf("Upgrade %lld -> %lld with %s\n", static_cast<long long>(previousIndex), static_cast<long long>(targetIndex), GetParam() ? "SchemaImportAdapter" : "Native");
    fflush(stdout);
    StopWatch timer(true);
    if (GetParam()) {
        SchemaImportCompletion completion;
        SchemaImportPlan plan;
        auto result = SchemaImportAdapter::BuildPlan(m_ecdb, schemas, plan, options);
        if (result.IsOk()) {
            printf("Plan built in %.3f s\n", timer.GetCurrentSeconds());
            PrintPlanHistogram(plan);
            result = SchemaImportAdapter::ExecutePlan(m_ecdb, plan, &completion);
        }
        BeJsDocument diagnostic;
        result.ToJson(diagnostic);
        ASSERT_TRUE(result.IsOk()) << diagnostic.Stringify().c_str();
        if (completion.IsPending())
            ASSERT_EQ(BE_SQLITE_OK, completion.Commit());
    } else {
        TestIssueListener listener;
        m_ecdb.AddIssueListener(listener);
        auto status = m_ecdb.Schemas().ImportSchemas(schemas, options);
        m_ecdb.RemoveIssueListener();
        for (auto const& issue : listener.m_issues)
            EXPECT_EQ(SUCCESS, status) << issue.message.c_str();
        ASSERT_EQ(SUCCESS, status);
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    }
    timer.Stop();
    printf("Upgrade to %lld imported in %.3f s\n", static_cast<long long>(targetIndex), timer.GetElapsedSeconds());
    fflush(stdout);
    RecordProperty("ImportSeconds", Utf8PrintfString("%.3f", timer.GetElapsedSeconds()).c_str());
}

INSTANTIATE_TEST_SUITE_P(AdHoc, SchemaImportHistoryTests, testing::Values(false, true),
                         [](testing::TestParamInfo<bool> const& info) { return info.param ? "SchemaEditor" : "Native"; });

END_ECDBUNITTESTS_NAMESPACE
