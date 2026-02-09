/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once

#include <sstream>

#include "ECDbTests.h"
#include "TestHelper.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
//! Disables "Fail On Assertion" for the lifetime of this object.
//! Use this helper classes instead of BeTest::SetFailOnAssert as it automatically resets
//! the FailOnAssert state when the ScopedDisableFailOnAssertion object goes out of scope
// @bsiclass
//=======================================================================================
struct ScopedDisableFailOnAssertion final {
   private:
    bool m_isNoop = false;

   public:
    explicit ScopedDisableFailOnAssertion(bool disable = true) : m_isNoop(!BeTest::GetFailOnAssert() || !disable) {
        if (!m_isNoop)
            BeTest::SetFailOnAssert(false);
    }

    ~ScopedDisableFailOnAssertion() {
        if (!m_isNoop)
            BeTest::SetFailOnAssert(true);
    }
};

//=======================================================================================
//! ECDb that requires a schema import token. For testing the schema import token feature
// @bsiclass
//=======================================================================================
struct RestrictedSchemaImportECDb : ECDb {
   public:
    explicit RestrictedSchemaImportECDb(bool requiresSchemaImportToken = true) : ECDb() {
        ApplyECDbSettings(false, requiresSchemaImportToken);
    }

    ~RestrictedSchemaImportECDb() {}

    SchemaImportToken const* GetSchemaImportToken() const { return GetECDbSettingsManager().GetSchemaImportToken(); }
};

//=======================================================================================
//! All non-static methods operate on ECDb held by the test fixture. The test fixture's ECDb
//! is created by using SetupECDb.
// @bsiclass
//=======================================================================================
struct ECDbTestFixture : public ::testing::Test {
   public:
   private:
    struct FixtureECDb final : ECDb {
       private:
        //! The test fixture's ECDb file's test helper. Any test assertions that operate on m_ecdb can be executed
        //! using this object
        TestHelper m_testHelper;

       public:
        FixtureECDb() : ECDb(), m_testHelper(*this) {}
        ~FixtureECDb() {}
        void UsingSavepointWithCommit(std::function<void()> func) {
            Savepoint sp(*this, "");
            func();
            sp.Commit();
        }

        void UsingSavepointWithCancel(std::function<void()> func) {
            Savepoint sp(*this, "");
            func();
            sp.Cancel();
        }
        TestHelper const& GetTestHelper() const { return m_testHelper; }
    };

    struct SeedECDbManager final {
       private:
        bmap<BeFileName, BeFileName> m_seedFilePathsBySchemaFileName;

        // not copyable
        SeedECDbManager(SeedECDbManager const&) = delete;
        SeedECDbManager& operator=(SeedECDbManager const&) = delete;

       public:
        SeedECDbManager() {}
        ~SeedECDbManager() {}

        bool TryGet(BeFileName& seedPath, BeFileNameCR schemaFileName) const {
            auto it = m_seedFilePathsBySchemaFileName.find(schemaFileName);
            if (it == m_seedFilePathsBySchemaFileName.end())
                return false;

            seedPath = it->second;
            return true;
        }

        BeFileNameCR Add(BeFileNameCR schemaFileName, BeFileNameCR seedPath) {
            BeAssert(m_seedFilePathsBySchemaFileName.find(schemaFileName) == m_seedFilePathsBySchemaFileName.end());
            auto ret = m_seedFilePathsBySchemaFileName.insert(bpair<BeFileName, BeFileName>(schemaFileName, seedPath));
            // return the inserted seed path
            return ret.first->second;
        }
    };

    static bool s_isInitialized;
    static SeedECDbManager* s_seedECDbManager;

    static SeedECDbManager& SeedECDbs();

   protected:
    //! The test's ECDb file. Any non-static methods of the ECDbTestFixture operate on it.
    FixtureECDb m_ecdb;
    DbResult SetupECDbForCurrentTest();
    BentleyStatus SetupECDbForCurrentTest(SchemaItem const&, ECDb::OpenParams const& openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    DbResult SetupECDb(Utf8CP ecdbFileName);
    BentleyStatus SetupECDb(Utf8CP ecdbFileName, SchemaItem const&, ECDb::OpenParams const& openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    BentleyStatus SetupECDb(Utf8CP ecdbFileName, void* fileData, uint32_t fileSize, ECDb::OpenParams const& openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    DbResult OpenECDb(BeFileNameCR filePath, ECDb::OpenParams const& openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    void CloseECDb();
    DbResult ReopenECDb();
    DbResult ReopenECDb(ECDb::OpenParams const&);
    DbResult CloneECDb(Utf8CP cloneFileName, BeFileNameCR seedFilePath, ECDb::OpenParams const& openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite)) { return CloneECDb(m_ecdb, cloneFileName, seedFilePath, openParams); }

    BentleyStatus PopulateECDb(ECN::ECSchemaCR, int instanceCountPerClass);
    BentleyStatus PopulateECDb(int instanceCountPerClass);

    BentleyStatus ImportSchema(SchemaItem const& schema) {
        EXPECT_TRUE(m_ecdb.IsDbOpen());
        return GetHelper().ImportSchema(schema);
    }
    BentleyStatus ImportSchema(SchemaItem const& schema, SchemaManager::SchemaImportOptions const& options) {
        EXPECT_TRUE(m_ecdb.IsDbOpen());
        return GetHelper().ImportSchema(schema, options);
    }
    BentleyStatus ImportSchemas(std::vector<SchemaItem> const& schemas) {
        EXPECT_TRUE(m_ecdb.IsDbOpen());
        return GetHelper().ImportSchemas(schemas);
    }
    BentleyStatus ImportSchemas(std::vector<SchemaItem> const& schemas, SchemaManager::SchemaImportOptions const& options) {
        EXPECT_TRUE(m_ecdb.IsDbOpen());
        return GetHelper().ImportSchemas(schemas, options);
    }
    BentleyStatus GetInstances(bvector<ECN::IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className);
    ECSqlStatus PrepareECSql(Utf8CP ecsql) {
        ECSqlStatement stmt;
        return stmt.Prepare(m_ecdb, ecsql);
    }
    TestHelper const& GetHelper() const { return m_ecdb.GetTestHelper(); }
    DbResult OpenECDbTestDataFile(Utf8CP name);

   private:
    static ECN::ECSchemaPtr s_unitsSchema;
    static ECN::ECSchemaPtr s_formatsSchema;

   public:
    ECDbTestFixture() : ::testing::Test() {}
    virtual ~ECDbTestFixture() {}
    void SetUp() override { Initialize(); }
    void TearDown() override { CloseECDb(); }

    //! Initializes the test environment by setting up the schema read context and search dirs etc.
    //! Gets implicitly called when calling SetupECDb, too. Tests that don't use
    //! that method can call this method statically.
    static void Initialize();

    static BeFileName BuildECDbPath(Utf8CP ecdbFileName);
    static DbResult CreateECDb(ECDbR, Utf8CP ecdbFileName = nullptr);
    static BentleyStatus ReadECSchema(ECN::ECSchemaReadContextPtr&, ECDbCR, SchemaItem const&);
    static ECN::ECSchemaPtr GetUnitsSchema(bool recreate = false);
    static ECN::ECSchemaPtr GetFormatsSchema(bool recreate = false);
    static bool EnableECSqlExperimentalFeatures(ECDbCR, bool);
    static bool IsECSqlExperimentalFeaturesEnabled(ECDbCR);
    static DbResult CloneECDb(ECDbR clone, Utf8CP cloneFileName, BeFileNameCR seedFilePath, ECDb::OpenParams const& openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    static DbResult CloneECDb(ECDbR clone, BeFileNameCR cloneFilePath, BeFileNameCR seedFilePath, ECDb::OpenParams const& openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
};

SchemaItem operator"" _schema(const char* s, size_t n);
Json::Value operator"" _json(const char* s, size_t n);
Json::Value GetPropertyMap(ECDbCR ecdb, Utf8CP className);
ECInstanceKey InsertInstance(ECDbCR ecdb, Json::Value const& v);
Json::Value ReadInstance(ECDbCR ecdb, ECInstanceKey ik, Utf8CP prop);
void UpdateInstance(ECDbCR ecdb, ECInstanceKey key, Json::Value const& v);
void DeleteInstance(ECDbCR ecdb, ECInstanceKey key);

//=======================================================================================
//! Used in combination with LogCatcher to capture log messages
// @bsiclass
//=======================================================================================
struct TestLogger : NativeLogging::Logger {
    std::vector<std::pair<NativeLogging::SEVERITY, Utf8String>> m_messages;

    void LogMessage(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg) override { m_messages.emplace_back(sev, msg); }
    bool IsSeverityEnabled(Utf8CP category, NativeLogging::SEVERITY sev) override { return true; }
    void Clear() { m_messages.clear(); }

    bool ValidateMessageAtIndex(size_t index, NativeLogging::SEVERITY expectedSeverity, const Utf8String& expectedMessage) const {
        if (index < m_messages.size()) {
            const auto& [severity, message] = m_messages[index];
            return severity == expectedSeverity && message.Equals(expectedMessage);
        }
        return false;  // Return false on index out of bounds
    }

    const std::pair<NativeLogging::SEVERITY, Utf8String>* GetLastMessage() const {
        if (!m_messages.empty()) {
            return &m_messages.back();
        }
        return nullptr;  // Return nullptr if there are no messages
    }

    const std::pair<NativeLogging::SEVERITY, Utf8String>* GetLastMessage(NativeLogging::SEVERITY severity) const {
        for (auto it = m_messages.rbegin(); it != m_messages.rend(); ++it) {
            if (it->first == severity) {
                return &(*it);
            }
        }
        return nullptr;  // Return nullptr if no messages with the specified severity are found
    }
};

//=======================================================================================
//! Until destruction, captures log messages and redirects them to the TestLogger
// @bsiclass
//=======================================================================================
struct LogCatcher {
    NativeLogging::Logger& m_previousLogger;
    TestLogger& m_testLogger;

    LogCatcher(TestLogger& testLogger) : m_testLogger(testLogger), m_previousLogger(NativeLogging::Logging::GetLogger()) {
        NativeLogging::Logging::SetLogger(&m_testLogger);
    }

    ~LogCatcher() { NativeLogging::Logging::SetLogger(&m_previousLogger); }
};

//=======================================================================================
//! Until destruction, captures log messages and redirects them to the TestLogger
// @bsiclass
//=======================================================================================
struct ReportedIssue {
    ECN::IssueSeverity severity;
    ECN::IssueCategory category;
    ECN::IssueType type;
    ECN::IssueId id;
    Utf8String message;

    ReportedIssue(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, ECN::IssueId id, Utf8CP message)
        : severity(severity), category(category), type(type), id(id), message(message) {}
};

//=======================================================================================
//! Until destruction, captures log messages and redirects them to the TestLogger
// @bsiclass
//=======================================================================================
struct TestIssueListener : ECN::IIssueListener {
    mutable std::vector<ReportedIssue> m_issues;

    void _OnIssueReported(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, ECN::IssueId id, Utf8CP message) const override {
        m_issues.emplace_back(severity, category, type, id, message);
    }

    void CompareIssues(bvector<Utf8String> const& expectedIssues);
    void CompareIssues(const std::vector<ReportedIssue>& expectedIssues);
    Utf8String GetLastMessage() const { return IsEmpty() ? Utf8String() : m_issues.back().message; }
    void ClearIssues() { m_issues.clear(); }
    bool IsEmpty() const { return m_issues.empty(); }
};

END_ECDBUNITTESTS_NAMESPACE
