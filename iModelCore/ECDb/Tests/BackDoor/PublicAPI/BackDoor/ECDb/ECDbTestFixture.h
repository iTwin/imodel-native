/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ECDbTests.h"
#include "TestHelper.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
//! Disables "Fail On Assertion" for the lifetime of this object.
//! Use this helper classes instead of BeTest::SetFailOnAssert as it automatically resets
//! the FailOnAssert state when the ScopedDisableFailOnAssertion object goes out of scope
// @bsiclass
//=======================================================================================    
struct ScopedDisableFailOnAssertion final
    {
private:
    bool m_isNoop = false;

public:
    explicit ScopedDisableFailOnAssertion(bool disable = true) : m_isNoop(!BeTest::GetFailOnAssert() || !disable)
        {
        if (!m_isNoop)
            BeTest::SetFailOnAssert(false);
        }

    ~ScopedDisableFailOnAssertion()
        {
        if (!m_isNoop)
            BeTest::SetFailOnAssert(true);
        }
    };

//=======================================================================================
//! ECDb that requires a schema import token. For testing the schema import token feature
// @bsiclass
//=======================================================================================    
struct RestrictedSchemaImportECDb : ECDb
    {
    public:
        explicit RestrictedSchemaImportECDb(bool requiresSchemaImportToken = true) : ECDb()
            {
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
struct ECDbTestFixture : public ::testing::Test
    {
public:

private:
    struct FixtureECDb final : ECDb
        {
        private:
            //! The test fixture's ECDb file's test helper. Any test assertions that operate on m_ecdb can be executed
            //! using this object
            TestHelper m_testHelper;

        public:
            FixtureECDb() : ECDb(), m_testHelper(*this) {}
            ~FixtureECDb() {}

            TestHelper const& GetTestHelper() const { return m_testHelper; }
        };

    struct SeedECDbManager final
        {
    private:
        bmap<BeFileName, BeFileName> m_seedFilePathsBySchemaFileName;

        //not copyable
        SeedECDbManager(SeedECDbManager const&) = delete;
        SeedECDbManager& operator=(SeedECDbManager const&) = delete;

    public:
        SeedECDbManager() {}
        ~SeedECDbManager() {}

        bool TryGet(BeFileName& seedPath, BeFileNameCR schemaFileName) const
            {
            auto it = m_seedFilePathsBySchemaFileName.find(schemaFileName);
            if (it == m_seedFilePathsBySchemaFileName.end())
                return false;

            seedPath = it->second;
            return true;
            }

        BeFileNameCR Add(BeFileNameCR schemaFileName, BeFileNameCR seedPath) 
            { 
            BeAssert(m_seedFilePathsBySchemaFileName.find(schemaFileName) == m_seedFilePathsBySchemaFileName.end());
            auto ret = m_seedFilePathsBySchemaFileName.insert(bpair<BeFileName, BeFileName>(schemaFileName, seedPath));
            //return the inserted seed path
            return ret.first->second;
            }

        };

    static bool s_isInitialized;
    static SeedECDbManager* s_seedECDbManager;

    static SeedECDbManager& SeedECDbs();

protected:
    //! The test's ECDb file. Any non-static methods of the ECDbTestFixture operate on it.
    FixtureECDb m_ecdb;
    DbResult SetupECDb(Utf8CP ecdbFileName);
    BentleyStatus SetupECDb(Utf8CP ecdbFileName, SchemaItem const&, ECDb::OpenParams const& openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    BentleyStatus SetupECDb(Utf8CP ecdbFileName, void *fileData, uint32_t fileSize, ECDb::OpenParams const& openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    DbResult OpenECDb(BeFileNameCR filePath, ECDb::OpenParams const& openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    void CloseECDb();
    DbResult ReopenECDb();
    DbResult ReopenECDb(ECDb::OpenParams const&);
    DbResult CloneECDb(Utf8CP cloneFileName, BeFileNameCR seedFilePath, ECDb::OpenParams const& openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite)) { return CloneECDb(m_ecdb, cloneFileName, seedFilePath, openParams); }
    static DbResult CloneECDb(ECDbR clone, Utf8CP cloneFileName, BeFileNameCR seedFilePath, ECDb::OpenParams const& openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    BentleyStatus PopulateECDb(ECN::ECSchemaCR, int instanceCountPerClass);
    BentleyStatus PopulateECDb(int instanceCountPerClass);

    BentleyStatus ImportSchema(SchemaItem const& schema) { EXPECT_TRUE(m_ecdb.IsDbOpen());  return GetHelper().ImportSchema(schema); }
    BentleyStatus ImportSchemas(std::vector<SchemaItem> const& schemas) { EXPECT_TRUE(m_ecdb.IsDbOpen());  return GetHelper().ImportSchemas(schemas); }

    BentleyStatus GetInstances(bvector<ECN::IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className);
    ECSqlStatus PrepareECSql(Utf8CP ecsql) { ECSqlStatement stmt; return stmt.Prepare(m_ecdb, ecsql); }
    TestHelper const& GetHelper() const { return m_ecdb.GetTestHelper(); }

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
    };

END_ECDBUNITTESTS_NAMESPACE
