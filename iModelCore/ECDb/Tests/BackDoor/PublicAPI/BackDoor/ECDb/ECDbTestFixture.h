/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ECDbTests.h"
#include "TestHelper.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
//! ECDb that requires a schema import token. For testing the schema import token feature
// @bsiclass                                               Krischan.Eberle     04/2017
//=======================================================================================    
struct RestrictedSchemaImportECDb : ECDb
    {
    public:
        RestrictedSchemaImportECDb(bool requiresSchemaImportToken, bool allowChangesetMergingIncompatibleECSchemaImport) : ECDb()
            {
            ApplyECDbSettings(false, requiresSchemaImportToken, allowChangesetMergingIncompatibleECSchemaImport);
            }

        ~RestrictedSchemaImportECDb() {}

        SchemaImportToken const* GetSchemaImportToken() const { return GetECDbSettings().GetSchemaImportToken(); }
        bool AllowChangesetMergingIncompatibleSchemaImport() const { return GetECDbSettings().AllowChangesetMergingIncompatibleSchemaImport(); }
    };

//=======================================================================================
//! All non-static methods operate on ECDb held by the test fixture. The test fixture's ECDb
//! is created by using SetupECDb.
// @bsiclass                                                 Carole.MacDonald     09/2015
//=======================================================================================    
struct ECDbTestFixture : public ::testing::Test
    {
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

    struct SeedECDbManager final : NonCopyableClass
        {
    private:
        bmap<BeFileName, BeFileName> m_seedFilePathsBySchemaFileName;

    public:
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
    BentleyStatus SetupECDb(Utf8CP ecdbFileName, SchemaItem const&, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    DbResult OpenECDb(BeFileNameCR filePath, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    void CloseECDb();
    DbResult ReopenECDb();
    DbResult CloneECDb(Utf8CP cloneFileName, BeFileNameCR seedFilePath, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite)) { return CloneECDb(m_ecdb, cloneFileName, seedFilePath, openParams); }
    static DbResult CloneECDb(ECDbR clone, Utf8CP cloneFileName, BeFileNameCR seedFilePath, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    BentleyStatus PopulateECDb(ECN::ECSchemaCR, int instanceCountPerClass);
    BentleyStatus PopulateECDb(int instanceCountPerClass);

    BentleyStatus ImportSchema(SchemaItem const& schema) { EXPECT_TRUE(m_ecdb.IsDbOpen());  return GetHelper().ImportSchema(schema); }
    BentleyStatus ImportSchemas(std::vector<SchemaItem> const& schemas) { EXPECT_TRUE(m_ecdb.IsDbOpen());  return GetHelper().ImportSchemas(schemas); }

    BentleyStatus GetInstances(bvector<ECN::IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className);

    TestHelper const& GetHelper() const { return m_ecdb.GetTestHelper(); }

public:
    ECDbTestFixture() : ::testing::Test() {}
    virtual ~ECDbTestFixture() {}
    void SetUp() override { Initialize(); }
    void TearDown() override {}

    //! Initializes the test environment by setting up the schema read context and search dirs etc.
    //! Gets implicitly called when calling SetupECDb, too. Tests that don't use
    //! that method can call this method statically.
    static void Initialize();

    static BeFileName BuildECDbPath(Utf8CP ecdbFileName);
    static DbResult CreateECDb(ECDbR, Utf8CP ecdbFileName = nullptr);
    static BentleyStatus ReadECSchema(ECN::ECSchemaReadContextPtr&, ECDbCR, SchemaItem const&);
    };

END_ECDBUNITTESTS_NAMESPACE
