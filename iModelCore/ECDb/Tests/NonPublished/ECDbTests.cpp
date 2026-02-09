/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
#include <ECDb/ConcurrentQueryManager.h>
#include <future>
#include <chrono>
#include <queue>
#include <thread>
#include <memory>
BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECInstanceId, Conversion) {
    // ToString
    ECInstanceId ecInstanceId(UINT64_C(123456789));
    Utf8CP expectedInstanceId = "123456789";
    Utf8Char actualInstanceId[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    ecInstanceId.ToString(actualInstanceId);
    EXPECT_STREQ(expectedInstanceId, actualInstanceId) << "Unexpected InstanceId generated from ECInstanceId " << ecInstanceId.GetValue();

    ecInstanceId = ECInstanceId(UINT64_C(0));
    expectedInstanceId = "0";
    actualInstanceId[0] = '\0';
    ecInstanceId.ToString(actualInstanceId);
    EXPECT_STREQ(expectedInstanceId, actualInstanceId) << "Unexpected InstanceId generated from ECInstanceId " << ecInstanceId.GetValueUnchecked();

    // FromString
    Utf8CP instanceId = "123456789";
    ECInstanceId expectedECInstanceId(UINT64_C(123456789));
    ECInstanceId actualECInstanceId;
    EXPECT_EQ(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId));
    EXPECT_EQ(expectedECInstanceId.GetValue(), actualECInstanceId.GetValue()) << "Unexpected ECInstanceId parsed from InstanceId " << instanceId;

    instanceId = "0";
    expectedECInstanceId = ECInstanceId(UINT64_C(0));
    actualECInstanceId = ECInstanceId();
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId));

    instanceId = "0000";
    expectedECInstanceId = ECInstanceId(UINT64_C(0));
    actualECInstanceId = ECInstanceId();
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId));

    instanceId = "-123456";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "InstanceId with negative number '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";

    instanceId = "-12345678901234";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "InstanceId with negative number '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";

    // now test with invalid instance ids
    ScopedDisableFailOnAssertion disableFailOnAssertion;

    instanceId = "0x75BCD15";
    expectedECInstanceId = ECInstanceId(UINT64_C(123456789));
    actualECInstanceId = ECInstanceId();
    EXPECT_EQ(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "InstanceId with hex formatted number '" << instanceId << "' is expected to be supported by ECInstanceId::FromString";
    EXPECT_TRUE(actualECInstanceId == ECInstanceId(UINT64_C(0x75BCD15)));

    instanceId = "i-12345";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "InstanceId starting with i- '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";

    instanceId = "1234a123";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "Non-numeric InstanceId '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";

    instanceId = "blabla";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "Non-numeric InstanceId '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST(SQLiteRegression, Test) {
    BeFileName testFilePath(L"D:\\temp\\sqliteselectissue.db");
    if (!testFilePath.DoesPathExist())
        return;

    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);

    Db db;
    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(testFilePath, Db::OpenParams(Db::OpenMode::Readonly)));

    Utf8CP problematicSql = R"sql(
    SELECT 1 FROM test_ClassA, (SELECT Id,ECClassId,SourceId,TargetId FROM test_Rel2) rel, test_ClassB
       WHERE test_ClassA.Id=? AND test_ClassA.Id=rel.SourceId AND test_ClassB.Id=rel.TargetId
                )sql";

    printf("Query plan: %s\r\n", db.ExplainQuery(problematicSql, true).c_str());
    printf("Query: %s\r\n", db.ExplainQuery(problematicSql, false).c_str());

    Statement verifyStmt;
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.Prepare(db, problematicSql));

    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.BindInt64(1, 1));
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step());
    verifyStmt.Reset();
    verifyStmt.ClearBindings();
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.BindText(1, "1", Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step()) << "This currently returns BE_SQLITE_DONE, possible due to a SQLite regression";
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbInit, Initialize) {
    BeFileName applicationSchemaDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(applicationSchemaDir);
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    ECDb::Initialize(temporaryDir, &applicationSchemaDir);
    ASSERT_EQ(true, ECDb::IsInitialized());
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, Settings) {
    EXPECT_FALSE(ECDb().GetECDbSettings().RequiresECCrudWriteToken());
    EXPECT_FALSE(ECDb().GetECDbSettings().RequiresECSchemaImportToken());

    struct RestrictableECDb final : ECDb {
        RestrictableECDb(bool requireCrudToken, bool requireSchemaImportToken) : ECDb() {
            ApplyECDbSettings(requireCrudToken, requireSchemaImportToken);
        }

        ~RestrictableECDb() {}

        ECCrudWriteToken const* GetCrudToken() const { return GetECDbSettingsManager().GetCrudWriteToken(); }
        SchemaImportToken const* GetImportToken() const { return GetECDbSettingsManager().GetSchemaImportToken(); }
    };

    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("settings.ecdb"));
    BeFileName testFilePath(m_ecdb.GetDbFileName());
    CloseECDb();

    auto createSchemaV1 = []() {
        ECSchemaPtr schema = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "Test", "t", 1, 0, 0));
        ECEntityClassP testClass = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(testClass, "Foo1"));
        PrimitiveECPropertyP testProp = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, testClass->CreatePrimitiveProperty(testProp, "Name", PRIMITIVETYPE_String));
        testClass = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(testClass, "Foo2"));
        EXPECT_EQ(ECObjectsStatus::Success, testClass->CreatePrimitiveProperty(testProp, "Name", PRIMITIVETYPE_String));
        return schema;
    };

    // creates V2 of the test schema that contains changes (a class is deleted) that are not supported by changeset merging
    auto createSchemaV2 = []() {
        ECSchemaPtr schema = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "Test", "t", 2, 0, 0));
        ECEntityClassP testClass = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(testClass, "Foo2"));
        PrimitiveECPropertyP testProp = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, testClass->CreatePrimitiveProperty(testProp, "Name", PRIMITIVETYPE_String));
        return schema;
    };

    for (bool requiresECCrudWriteToken : {false, true}) {
        for (bool requiresSchemaImportToken : {false, true}) {
            RestrictableECDb ecdb(requiresECCrudWriteToken, requiresSchemaImportToken);
            EXPECT_EQ(requiresECCrudWriteToken, ecdb.GetECDbSettings().RequiresECCrudWriteToken());
            EXPECT_EQ(requiresSchemaImportToken, ecdb.GetECDbSettings().RequiresECSchemaImportToken());

            ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));
            Utf8CP ecsql = "INSERT INTO ecdbf.ExternalFileInfo(Name) VALUES('foofile.txt')";
            if (requiresECCrudWriteToken) {
                ECSqlStatement stmt;
                ASSERT_TRUE(ecdb.GetCrudToken() != nullptr) << "RequiresECCrudWriteToken == true";
                ASSERT_EQ(ECSqlStatus::Error, stmt.Prepare(ecdb, ecsql, nullptr)) << "RequiresECCrudWriteToken == true";
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql, ecdb.GetCrudToken())) << "RequiresECCrudWriteToken == true";
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "RequiresECCrudWriteToken == true";
            } else {
                ECSqlStatement stmt;
                ASSERT_TRUE(ecdb.GetCrudToken() == nullptr) << "RequiresECCrudWriteToken == false";
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql, nullptr)) << "RequiresECCrudWriteToken == false";
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "RequiresECCrudWriteToken == false";
            }

            if (requiresSchemaImportToken) {
                ASSERT_TRUE(ecdb.GetImportToken() != nullptr) << "RequiresSchemaImportToken == true";
                ECSchemaPtr schema = createSchemaV1();
                ASSERT_EQ(ERROR, ecdb.Schemas().ImportSchemas({schema.get()}, nullptr)) << "RequiresSchemaImportToken == true";
                ecdb.AbandonChanges();
                schema = createSchemaV1();
                ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas({schema.get()}, ecdb.GetImportToken())) << "RequiresSchemaImportToken == true";
                ASSERT_TRUE(ecdb.Schemas().GetClass("Test", "Foo1") != nullptr) << "RequiresSchemaImportToken == true";
                ASSERT_TRUE(ecdb.Schemas().GetClass("Test", "Foo2") != nullptr) << "RequiresSchemaImportToken == true";
            } else {
                ECSchemaPtr schema = createSchemaV1();
                ASSERT_TRUE(ecdb.GetImportToken() == nullptr) << "RequiresSchemaImportToken == false";
                ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas({schema.get()}, nullptr)) << "RequiresSchemaImportToken == false";
                ASSERT_TRUE(ecdb.Schemas().GetClass("Test", "Foo1") != nullptr) << "RequiresSchemaImportToken == false";
                ASSERT_TRUE(ecdb.Schemas().GetClass("Test", "Foo2") != nullptr) << "RequiresSchemaImportToken == false";
            }

            ecdb.AbandonChanges();
        }
    }
}

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TestOtherConnectionECDb : ECDb {
    int m_changeCount;
    TestOtherConnectionECDb()
        : m_changeCount(0) {}

    void _OnDbChangedByOtherConnection() override {
        m_changeCount++;
        ECDb::_OnDbChangedByOtherConnection();
    }
};

//---------------------------------------------------------------------------------------
// Test to ensure that PRAGMA data_version changes when another connection changes a database
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, TwoConnections) {
    BeFileName testECDbPath;
    {
        ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("twoConnections.ecdb"));
        testECDbPath = BeFileName(m_ecdb.GetDbFileName());
        m_ecdb.CloseDb();
    }

    TestOtherConnectionECDb ecdb1, ecdb2;
    ASSERT_EQ(BE_SQLITE_OK, ecdb1.OpenBeSQLiteDb(testECDbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, DefaultTxn::No)));
    ASSERT_EQ(BE_SQLITE_OK, ecdb2.OpenBeSQLiteDb(testECDbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, DefaultTxn::No)));

    {  // make a change to the database from the first connection
        Savepoint t1(ecdb1, "tx1");
        // the first transaction should not call _OnDbChangedByOtherConnection
        ASSERT_EQ(0, ecdb1.m_changeCount);
        ecdb1.CreateTable("TEST", "Col1 INTEGER");
    }

    {  // make a change to the database from the second connection
        Savepoint t2(ecdb2, "tx2");
        // the first transaction on the second connection should not call _OnDbChangedByOtherConnection
        ASSERT_EQ(0, ecdb2.m_changeCount);
        ecdb2.ExecuteSql("INSERT INTO TEST(Col1) VALUES(3)");
    }

    {  // start another transaction on the first connection. This should notice that the second connection changed the db.
        Savepoint t3(ecdb1, "tx1");
        ASSERT_EQ(1, ecdb1.m_changeCount);
        ecdb1.ExecuteSql("INSERT INTO TEST(Col1) VALUES(4)");
    }

    {  // additional changes from the same connection should not trigger additional calls to _OnDbChangedByOtherConnection
        Savepoint t3(ecdb1, "tx1");
        ASSERT_EQ(1, ecdb1.m_changeCount);
        ecdb1.ExecuteSql("INSERT INTO TEST(Col1) VALUES(5)");
    }
}

TEST_F(ECDbTestFixture, TestDropSchemasWithInstances) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("TestDropSchemasWithInstances.ecdb"));

    // Import schemas
    auto context = ECSchemaReadContext::CreateContext();
    ASSERT_TRUE(context.IsValid());
    context->AddSchemaLocater(m_ecdb.GetSchemaLocater());

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, R"xml(<?xml version='1.0' encoding='utf-8'?>
        <ECSchema schemaName='TestSchema1' alias='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
            <ECEntityClass typeName='A'>
                <ECProperty propertyName='Prop_A' typeName='int' />
            </ECEntityClass>
        </ECSchema>)xml",
                                                                     *context));
    ASSERT_TRUE(schema.IsValid());

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, R"xml(<?xml version='1.0' encoding='utf-8'?>
        <ECSchema schemaName='TestSchema2' alias='ts2' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
            <ECEntityClass typeName='B'>
                <ECProperty propertyName='Prop_B' typeName='int' />
            </ECEntityClass>
        </ECSchema>)xml",
                                                                     *context));
    ASSERT_TRUE(schema.IsValid());

    m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas());

    ASSERT_TRUE(m_ecdb.Schemas().ContainsSchema("TestSchema1"));
    ASSERT_TRUE(m_ecdb.Schemas().ContainsSchema("TestSchema2"));

    // Add instances
    ECSqlStatement stmt;
    ECInstanceKey instanceKey;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts1.A(Prop_A) VALUES(100)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(instanceKey));

    // Schema drop should fail
    TestIssueListener listener;
    m_ecdb.AddIssueListener(listener);
    auto status = m_ecdb.Schemas().DropSchemas({"TestSchema1", "TestSchema2"});
    EXPECT_TRUE(status.IsError());
    EXPECT_TRUE(status.HasInstances());
    EXPECT_STREQ(listener.GetLastMessage().c_str(), "Drop ECSchema failed. One or more schemas have instances present. Make sure to delete them before dropping the schemas.");

    // Delete the instances
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ts1.A WHERE ECInstanceId=?"));
    stmt.BindId(1, instanceKey.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    // Schema drop should now pass
    listener.ClearIssues();
    EXPECT_TRUE(m_ecdb.Schemas().DropSchemas({"TestSchema1", "TestSchema2"}).IsSuccess());
    EXPECT_TRUE(listener.IsEmpty());
}

TEST_F(ECDbTestFixture, TestDropSchemasBeingReferenced) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("TestDropSchemasBeingReferenced.ecdb"));

    // Import schemas
    auto context = ECSchemaReadContext::CreateContext();
    ASSERT_TRUE(context.IsValid());
    context->AddSchemaLocater(m_ecdb.GetSchemaLocater());

    const auto schemaTemplate = R"xml(<?xml version='1.0' encoding='utf-8'?>
        <ECSchema schemaName='TestSchema%d' alias='ts%d' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
            %s
        </ECSchema>)xml";

    for (const auto& [schemaNum, schemaRefs] : std::vector<std::pair<unsigned int, Utf8CP>>{
             {std::make_pair(1, "")},
             {std::make_pair(2, "")},
             {std::make_pair(3, "<ECSchemaReference name='TestSchema1' version='1.0.0' alias='ts1'/>")},
             {std::make_pair(4, "<ECSchemaReference name='TestSchema2' version='1.0.0' alias='ts2'/>")},
             {std::make_pair(5, R"xml(<ECSchemaReference name='TestSchema1' version='1.0.0' alias='ts1'/>
            <ECSchemaReference name='TestSchema2' version='1.0.0' alias='ts2'/>)xml")},
             {std::make_pair(6, R"xml(<ECSchemaReference name='TestSchema3' version='1.0.0' alias='ts3'/>
            <ECSchemaReference name='TestSchema5' version='1.0.0' alias='ts5'/>)xml")},
             {std::make_pair(7, "<ECSchemaReference name='TestSchema4' version='1.0.0' alias='ts4'/>")},
         }) {
        ECSchemaPtr schema;
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, Utf8PrintfString(schemaTemplate, schemaNum, schemaNum, schemaRefs).c_str(), *context));
        ASSERT_TRUE(schema.IsValid());
    }

    m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas());

    for (auto i = 1; i <= 7; ++i)
        ASSERT_TRUE(m_ecdb.Schemas().ContainsSchema(Utf8PrintfString("TestSchema%d", i)));

    m_ecdb.SaveChanges();

    TestIssueListener listener;
    m_ecdb.AddIssueListener(listener);

    /* The schema references have been set up as:

                       TestSchema1        TestSchema2
                        /       \        /      |
                TestSchema3   TestSchema5   TestSchema4
                        \      /                |
                      TestSchema6           TestSchema7
    */

    for (const auto& [testCaseNumber, schemasToDrop, expectedStatus, expectedStatusCode, expectedErrorMessage] : bvector<std::tuple<const unsigned int, const std::vector<Utf8String>, const bool, const DropSchemaResult::Status, Utf8CP>>{
             {__LINE__, {"TestSchema1", "TestSchemaMissing1", "TestSchema2", "TestSchemaMissing2"}, false, DropSchemaResult::ErrorSchemaNotFound, "Drop ECSchemas failed. ECSchema: Schemas TestSchemaMissing1,TestSchemaMissing2 not found."},

             {__LINE__, {"TestSchema5", "TestSchema6", "TestSchema2", "TestSchema4", "TestSchema3", "TestSchema7", "TestSchema1"}, true, DropSchemaResult::Success, ""},
             {__LINE__, {"TestSchema6", "TestSchema7"}, true, DropSchemaResult::Success, ""},
             {__LINE__, {"TestSchema5", "TestSchema6"}, true, DropSchemaResult::Success, ""},
             {__LINE__, {"TestSchema3", "TestSchema5", "TestSchema6"}, true, DropSchemaResult::Success, ""},
             {__LINE__, {"TestSchema1", "TestSchema3", "TestSchema5", "TestSchema6"}, true, DropSchemaResult::Success, ""},
             {__LINE__, {"TestSchema2", "TestSchema4", "TestSchema5", "TestSchema6", "TestSchema7"}, true, DropSchemaResult::Success, ""},
             {__LINE__, {"TestSchema7", "TestSchema4"}, true, DropSchemaResult::Success, ""},

             {__LINE__, {"TestSchema1", "TestSchema2"}, false, DropSchemaResult::ErrorDeletedSchemaIsReferencedByAnotherSchema, "Drop ECSchemas failed. Schema(s) TestSchema1,TestSchema2 are being referenced by other schemas."},
             {__LINE__, {"TestSchema3", "TestSchema5", "TestSchema4"}, false, DropSchemaResult::ErrorDeletedSchemaIsReferencedByAnotherSchema, "Drop ECSchemas failed. Schema(s) TestSchema3,TestSchema4,TestSchema5 are being referenced by other schemas."},
             {__LINE__, {"TestSchema1", "TestSchema3", "TestSchema6"}, false, DropSchemaResult::ErrorDeletedSchemaIsReferencedByAnotherSchema, "Drop ECSchemas failed. Schema(s) TestSchema1 are being referenced by other schemas."},
             {__LINE__, {"TestSchema1", "TestSchema5", "TestSchema6"}, false, DropSchemaResult::ErrorDeletedSchemaIsReferencedByAnotherSchema, "Drop ECSchemas failed. Schema(s) TestSchema1 are being referenced by other schemas."},
             {__LINE__, {"TestSchema7", "TestSchema2", "TestSchema4"}, false, DropSchemaResult::ErrorDeletedSchemaIsReferencedByAnotherSchema, "Drop ECSchemas failed. Schema(s) TestSchema2 are being referenced by other schemas."},
         }) {
        listener.ClearIssues();
        StopWatch timer(true);
        auto status = m_ecdb.Schemas().DropSchemas(schemasToDrop);
        timer.Stop();
        // std::cout << "Elapsed seconds: " << timer.GetElapsedSeconds() << std::endl;
        EXPECT_EQ(status.IsSuccess(), expectedStatus) << "Failed test case " << testCaseNumber;
        EXPECT_EQ(status.GetStatus(), expectedStatusCode) << "Failed test case " << testCaseNumber;

        if (Utf8String::IsNullOrEmpty(expectedErrorMessage)) {
            EXPECT_TRUE(listener.IsEmpty()) << "Failed test case " << testCaseNumber;
            for (const auto& schema : schemasToDrop)
                EXPECT_FALSE(m_ecdb.Schemas().ContainsSchema(schema)) << "Failed to drop schema " << schema << " in test case " << testCaseNumber;
        } else {
            EXPECT_STREQ(listener.GetLastMessage().c_str(), expectedErrorMessage) << "Failed test case " << testCaseNumber;
        }

        m_ecdb.AbandonChanges();
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct TestBusyRetry : BusyRetry {
    int m_retryCount;
    mutable int m_actualRetryCount;

    Savepoint* m_savepointToCommitDuringRetry;
    int m_retryCountBeforeCommit;

    explicit TestBusyRetry(int retryCount)
        : m_retryCount(retryCount), m_actualRetryCount(0), m_savepointToCommitDuringRetry(nullptr), m_retryCountBeforeCommit(-1) {}

    int _OnBusy(int count) const override {
        // count is 0-based
        if (count >= m_retryCount)
            return 0;

        m_actualRetryCount = count + 1;

        if (m_savepointToCommitDuringRetry != nullptr && m_retryCountBeforeCommit == count) {
            if (BE_SQLITE_OK != m_savepointToCommitDuringRetry->Commit()) {
                BeAssert(false && "Cannot commit write transaction in retry loop.");
                return 1;
            }
        }

        return 1;
    }

    void Reset() { m_actualRetryCount = 0; }
    void SetSavepointToCommitDuringRetry(Savepoint& savepoint, int retryCountBeforeCommit) {
        m_savepointToCommitDuringRetry = &savepoint;
        m_retryCountBeforeCommit = retryCountBeforeCommit;
    }
};

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, TwoConnectionsWithBusyRetryHandler) {
    BeFileName testECDbPath;
    {
        ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("twoConnectionsWithBusyRetryHandler.ecdb"));
        testECDbPath = BeFileName(m_ecdb.GetDbFileName());
        m_ecdb.CloseDb();
    }

    ECDb::OpenParams openParams(ECDb::OpenMode::ReadWrite, DefaultTxn::No);
    ECDb ecdb1;
    DbResult result = ecdb1.OpenBeSQLiteDb(testECDbPath, openParams);
    ASSERT_EQ(BE_SQLITE_OK, result);
    TestBusyRetry retry(3);
    retry.AddRef();
    ECDb ecdb2;
    openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite, DefaultTxn::No);
    openParams.SetBusyRetry(&retry);
    result = ecdb2.OpenBeSQLiteDb(testECDbPath, openParams);
    ASSERT_EQ(BE_SQLITE_OK, result);

    {
        // make a change to the database from the first connection
        Savepoint s1(ecdb1, "ecdb1");
        ecdb1.CreateTable("TEST", "Col1 INTEGER");

        // try to make a change to the database from the second connection which should fail

        Savepoint s2(ecdb2, "ecdb2", false, BeSQLiteTxnMode::Immediate);
        result = s2.Begin();
        ASSERT_EQ(BE_SQLITE_BUSY, result);
        ASSERT_EQ(retry.m_retryCount, retry.m_actualRetryCount);
    }

    // at this point all locks should be cleared again.

    // now try to make a change to the database if the retry handler commits the open transaction on the first connection
    {
        // make a change to the database from the first connection
        Savepoint s1(ecdb1, "ecdb1");
        ecdb1.CreateTable("TEST2", "Col1 INTEGER");

        retry.Reset();
        Savepoint s2(ecdb2, "ecdb2", false, BeSQLiteTxnMode::Immediate);
        int retryAttemptsBeforeCommitting = 2;
        retry.SetSavepointToCommitDuringRetry(s1, 2);
        result = s2.Begin();
        ASSERT_EQ(BE_SQLITE_OK, result) << "Change on first conn gets committed during retry, so Savepoint.Begin should succeed";
        // after commit one more retry will be done (therefore + 1)
        ASSERT_EQ(retryAttemptsBeforeCommitting + 1, retry.m_actualRetryCount);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, ResetInstanceIdSequence) {
    struct TestECDb final : ECDb {
        TestECDb() : ECDb() {}
        ~TestECDb() {}

        BentleyStatus CallResetInstanceIdSequence(BeBriefcaseId newBriefcaseId, IdSet<ECClassId> const* ecClassIgnoreList) { return ResetInstanceIdSequence(newBriefcaseId, ecClassIgnoreList); }

        BeBriefcaseBasedId GetInstanceIdSequenceValue() {
            BriefcaseLocalValueCache& cache = GetBLVCache();
            size_t ix = 0;
            if (!cache.TryGetIndex(ix, "ec_instanceidsequence"))
                return BeBriefcaseBasedId();

            uint64_t val = 0;
            if (BE_SQLITE_OK != cache.QueryValue(val, ix))
                return BeBriefcaseBasedId();

            return BeBriefcaseBasedId(val);
        }
    };

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ResetInstanceIdSequence.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                                            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                                                                            <ECEntityClass typeName="A" >
                                                                                <ECCustomAttributes>
                                                                                   <ClassMap xmlns="ECDbMap.02.00">
                                                                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                                                                    </ClassMap>
                                                                                 </ECCustomAttributes>
                                                                                <ECProperty propertyName="Prop1" typeName="int" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="A1" >
                                                                                <BaseClass>A</BaseClass>
                                                                                <ECProperty propertyName="Prop2" typeName="int" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="B" >
                                                                                <ECCustomAttributes>
                                                                                   <ClassMap xmlns="ECDbMap.02.00">
                                                                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                                                                    </ClassMap>
                                                                                 </ECCustomAttributes>
                                                                                <ECProperty propertyName="Prop1" typeName="int" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="B1" >
                                                                                <BaseClass>B</BaseClass>
                                                                                <ECProperty propertyName="Prop2" typeName="int" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="C" >
                                                                                <ECCustomAttributes>
                                                                                   <ClassMap xmlns="ECDbMap.02.00">
                                                                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                                                                    </ClassMap>
                                                                                 </ECCustomAttributes>
                                                                                <ECProperty propertyName="Prop1" typeName="int" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="C1" >
                                                                                <BaseClass>C</BaseClass>
                                                                                <ECProperty propertyName="Prop2" typeName="int" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="C2" >
                                                                                <BaseClass>C</BaseClass>
                                                                                <ECProperty propertyName="Prop3" typeName="int" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="C21" >
                                                                                <BaseClass>C2</BaseClass>
                                                                                <ECProperty propertyName="Prop4" typeName="int" />
                                                                            </ECEntityClass>
                                                                        </ECSchema>)xml")));

    BeFileName filePath(m_ecdb.GetDbFileName());

    BeBriefcaseId masterBriefcaseId(BeBriefcaseId::Standalone());
    BeBriefcaseId briefcaseAId(3);
    BeBriefcaseId briefcaseBId(111);

    std::map<uint32_t, uint64_t> sequenceValuesPerBriefcase{{masterBriefcaseId.GetValue(), 0},
                                                            {briefcaseAId.GetValue(), 0},
                                                            {briefcaseBId.GetValue(), 0}};

    ASSERT_EQ(BentleyStatus::SUCCESS, PopulateECDb(5));
    sequenceValuesPerBriefcase[masterBriefcaseId.GetValue()] = UINT64_C(40);

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ResetBriefcaseId(briefcaseAId));
    ASSERT_EQ(BentleyStatus::SUCCESS, PopulateECDb(5));
    sequenceValuesPerBriefcase[briefcaseAId.GetValue()] = UINT64_C(40);

    m_ecdb.CloseDb();

    TestECDb testDb;
    ASSERT_EQ(BE_SQLITE_OK, testDb.OpenBeSQLiteDb(filePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));

    ASSERT_EQ(sequenceValuesPerBriefcase[testDb.GetBriefcaseId().GetValue()], testDb.GetInstanceIdSequenceValue().GetLocalId()) << "Briefcase Id: " << testDb.GetBriefcaseId().GetValue();

    for (std::pair<uint32_t, uint64_t> const& kvPair : sequenceValuesPerBriefcase) {
        BeBriefcaseId briefcaseId(kvPair.first);
        uint64_t expectedMaxId = kvPair.second;
        BeBriefcaseBasedId expectedId(briefcaseId, expectedMaxId);
        ASSERT_EQ(SUCCESS, testDb.CallResetInstanceIdSequence(briefcaseId, nullptr)) << briefcaseId.GetValue();
        ASSERT_EQ(expectedId, testDb.GetInstanceIdSequenceValue()) << "Briefcase Id: " << briefcaseId.GetValue() << " Expected sequence value (lower 40 bits): " << expectedMaxId;
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, GetAndAssignBriefcaseIdForDb) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ecdbbriefcaseIdtest.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));

    BeBriefcaseId id = m_ecdb.GetBriefcaseId();
    ASSERT_TRUE(id.IsValid());
    BeBriefcaseId newId = BeBriefcaseId(BeBriefcaseId::FirstValidBriefcaseId());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ResetBriefcaseId(newId));
    int32_t changedBriefcaseId = m_ecdb.GetBriefcaseId().GetValue();
    ASSERT_EQ(newId.GetValue(), changedBriefcaseId);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, GetAndChangeGUIDForDb) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("getAndChangeGUIDForDb.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));

    BeGuid guid = m_ecdb.GetDbGuid();
    ASSERT_TRUE(guid.IsValid());

    BeGuid oldGuid = guid;
    guid.Create();
    if (guid.IsValid())
        m_ecdb.ChangeDbGuid(guid);
    BeGuid newGuid = m_ecdb.GetDbGuid();
    ASSERT_TRUE(newGuid.IsValid());
    ASSERT_TRUE(oldGuid != newGuid);

    guid.Invalidate();
    ASSERT_FALSE(guid.IsValid());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, CurrentECSqlVersion) {
    BeVersion expectedVersion(2, 0, 2, 0);
    ASSERT_EQ(ECDb::GetECSqlVersion(), expectedVersion);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, CurrentECDbProfileVersion) {
    ProfileVersion expectedVersion(4, 0, 0, 5);
    ASSERT_EQ(ECDb::CurrentECDbProfileVersion(), expectedVersion);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, NewFileECDbProfileVersion) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("newFile.ecdb"));
    ProfileVersion expectedVersion(4, 0, 0, 5);
    ASSERT_EQ(m_ecdb.GetECDbProfileVersion(), expectedVersion);
}

TEST_F(ECDbTestFixture, TestGreatestAndLeastFunctionsWithLiterals) {
    TestIssueListener listener;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("newFile.ecdb"));
    m_ecdb.AddIssueListener(listener);

    // Test DQL statements
    for (const auto& [lineNumber, sqlStatement, expectedStatus, expectedResult, errorMsg] : bvector<std::tuple<const int, const Utf8String, const ECSqlStatus, const std::vector<Utf8String>, const Utf8String>>{
             {__LINE__, "SELECT GREATEST()", ECSqlStatus::InvalidECSql, {"0"}, "Preparing the ECSQL 'SELECT GREATEST()' failed. Underlying SQLite statement failed to prepare: BE_SQLITE_ERROR wrong number of arguments to function MAX() (BE_SQLITE_ERROR) [SQL: SELECT MAX()]"},
             {__LINE__, "SELECT GREATEST(4)", ECSqlStatus::Success, {"4"}, ""},
             {__LINE__, "SELECT GREATEST(4, 2, 1, 5, 3)", ECSqlStatus::Success, {"5"}, ""},

             {__LINE__, "SELECT LEAST()", ECSqlStatus::InvalidECSql, {"0"}, "Preparing the ECSQL 'SELECT LEAST()' failed. Underlying SQLite statement failed to prepare: BE_SQLITE_ERROR wrong number of arguments to function MIN() (BE_SQLITE_ERROR) [SQL: SELECT MIN()]"},
             {__LINE__, "SELECT LEAST(4)", ECSqlStatus::Success, {"4"}, ""},
             {__LINE__, "SELECT LEAST(4, 2, 1, 5, 3)", ECSqlStatus::Success, {"1"}, ""},

             {__LINE__, "SELECT MAX()", ECSqlStatus::InvalidECSql, {"0"}, "Failed to parse ECSQL 'SELECT MAX()': syntax error"},
             {__LINE__, "SELECT MAX(  )", ECSqlStatus::InvalidECSql, {"0"}, "Failed to parse ECSQL 'SELECT MAX(  )': syntax error"},
             {__LINE__, "SELECT MAX( \n )", ECSqlStatus::InvalidECSql, {"0"}, "Failed to parse ECSQL 'SELECT MAX( \n )': syntax error"},
             {__LINE__, "SELECT MAX(4)", ECSqlStatus::Success, {"4"}, ""},
             {__LINE__, "SELECT MAX(4, 2, 1, 5, 3)", ECSqlStatus::InvalidECSql, {"0"}, "Failed to parse ECSQL 'SELECT MAX(4, 2, 1, 5, 3)': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},

             {__LINE__, "SELECT MIN()", ECSqlStatus::InvalidECSql, {"0"}, "Failed to parse ECSQL 'SELECT MIN()': syntax error"},
             {__LINE__, "SELECT MIN(  )", ECSqlStatus::InvalidECSql, {"0"}, "Failed to parse ECSQL 'SELECT MIN(  )': syntax error"},
             {__LINE__, "SELECT MIN( \n )", ECSqlStatus::InvalidECSql, {"0"}, "Failed to parse ECSQL 'SELECT MIN( \n )': syntax error"},
             {__LINE__, "SELECT MIN(4)", ECSqlStatus::Success, {"4"}, ""},
             {__LINE__, "SELECT MIN(4, 2, 1, 5, 3)", ECSqlStatus::InvalidECSql, {"0"}, "Failed to parse ECSQL 'SELECT MIN(4, 2, 1, 5, 3)': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},

             {__LINE__, "SELECT GREATEST(4, 5, 1), GREATEST(6, 7, 9)", ECSqlStatus::Success, {"5", "9"}, ""},
             {__LINE__, "SELECT GREATEST(4, 5, 1), LEAST(4, 5, 1)", ECSqlStatus::Success, {"5", "1"}, ""},
             {__LINE__, "SELECT LEAST(4, 5, 1), GREATEST(4, 5, 1)", ECSqlStatus::Success, {"1", "5"}, ""},
             {__LINE__, "SELECT LEAST(4, 5, 1), LEAST(6, 7, 9)", ECSqlStatus::Success, {"1", "6"}, ""},
             {__LINE__, "SELECT MAX(4, 5, 1), MIN(4, 5, 1)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MAX(4, 5, 1), MIN(4, 5, 1)': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT MIN(4, 5, 1), MAX(4, 5, 1)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MIN(4, 5, 1), MAX(4, 5, 1)': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},

             {__LINE__, "SELECT 'GREATEST(2,3)' AS str", ECSqlStatus::Success, {"GREATEST(2,3)"}, ""},
             {__LINE__, "SELECT 'LEAST(2,3)' AS str", ECSqlStatus::Success, {"LEAST(2,3)"}, ""},
             {__LINE__, "SELECT 'MAX(2,3)' AS str", ECSqlStatus::Success, {"MAX(2,3)"}, ""},
             {__LINE__, "SELECT 'MIN(2,3)' AS str", ECSqlStatus::Success, {"MIN(2,3)"}, ""},

             {__LINE__, "SELECT '  GREATEST( 3, 4)'", ECSqlStatus::Success, {"  GREATEST( 3, 4)"}, ""},
             {__LINE__, "SELECT '  LEAST( 3, 4)'", ECSqlStatus::Success, {"  LEAST( 3, 4)"}, ""},
             {__LINE__, "SELECT '  MAX( 3, 4)'", ECSqlStatus::Success, {"  MAX( 3, 4)"}, ""},
             {__LINE__, "SELECT '  MIN( 3, 4)'", ECSqlStatus::Success, {"  MIN( 3, 4)"}, ""},

             {__LINE__, "-- GREATEST( 3, 4)  comment", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL '-- GREATEST( 3, 4)  comment': syntax error"},
             {__LINE__, "-- LEAST( 3, 4)  comment", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL '-- LEAST( 3, 4)  comment': syntax error"},
             {__LINE__, "-- MAX( 3, 4)  comment", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL '-- MAX( 3, 4)  comment': syntax error"},
             {__LINE__, "-- MIN( 3, 4)  comment", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL '-- MIN( 3, 4)  comment': syntax error"},

             {__LINE__, "SELECT  GREATEST( 3, 4)", ECSqlStatus::Success, {"4"}, ""},
             {__LINE__, "SELECT  LEAST( 3, 4)", ECSqlStatus::Success, {"3"}, ""},
             {__LINE__, "SELECT  MAX( 3, 4)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT  MAX( 3, 4)': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT  MIN( 3, 4)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT  MIN( 3, 4)': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},

             {__LINE__, "SELECT GREATEST('a', 'b', 'c')", ECSqlStatus::Success, {"c"}, ""},
             {__LINE__, "SELECT LEAST('a', 'b', 'c')", ECSqlStatus::Success, {"a"}, ""},
             {__LINE__, "SELECT MAX('a', 'b', 'c')", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MAX('a', 'b', 'c')': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT MIN('a', 'b', 'c')", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MIN('a', 'b', 'c')': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},

             {__LINE__, "SELECT GREATEST(1.5, 1.4, 1.1)", ECSqlStatus::Success, {"1.5"}, ""},
             {__LINE__, "SELECT LEAST(1.5, 1.4, 1.1)", ECSqlStatus::Success, {"1.1"}, ""},
             {__LINE__, "SELECT MAX(1.5, 1.4, 1.1)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MAX(1.5, 1.4, 1.1)': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT MIN(1.5, 1.4, 1.1)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MIN(1.5, 1.4, 1.1)': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},

             {__LINE__, "SELECT GREATEST(GREATEST(3, 4), GREATEST(5, 9))", ECSqlStatus::Success, {"9"}, ""},
             {__LINE__, "SELECT GREATEST(LEAST(3, 4), GREATEST(5, 9))", ECSqlStatus::Success, {"9"}, ""},
             {__LINE__, "SELECT GREATEST(GREATEST(3, 4), LEAST(5, 9))", ECSqlStatus::Success, {"5"}, ""},
             {__LINE__, "SELECT GREATEST(LEAST(3, 4), LEAST(5, 9))", ECSqlStatus::Success, {"5"}, ""},
             {__LINE__, "SELECT LEAST(LEAST(3, 4), LEAST(5, 9))", ECSqlStatus::Success, {"3"}, ""},
             {__LINE__, "SELECT LEAST(LEAST(3, 4), GREATEST(5, 9))", ECSqlStatus::Success, {"3"}, ""},
             {__LINE__, "SELECT LEAST(GREATEST(3, 4), LEAST(5, 9))", ECSqlStatus::Success, {"4"}, ""},
             {__LINE__, "SELECT LEAST(GREATEST(3, 4), GREATEST(5, 9))", ECSqlStatus::Success, {"4"}, ""},

             {__LINE__, "SELECT MAX(GREATEST(3, 4), LEAST(5, 9))", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MAX(GREATEST(3, 4), LEAST(5, 9))': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT MAX(GREATEST(3, 4))", ECSqlStatus::Success, {"4"}, ""},
             {__LINE__, "SELECT MIN(GREATEST(3, 4), LEAST(5, 9))", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MIN(GREATEST(3, 4), LEAST(5, 9))': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT MIN(GREATEST(3, 4))", ECSqlStatus::Success, {"4"}, ""},

             {__LINE__, "SELECT LEAST(MAX(3, 4), LEAST(5, 9))", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT LEAST(MAX(3, 4), LEAST(5, 9))': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT LEAST(GREATEST(3, 4), MAX(5, 9))", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT LEAST(GREATEST(3, 4), MAX(5, 9))': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT LEAST(MIN(3, 4), LEAST(5, 9))", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT LEAST(MIN(3, 4), LEAST(5, 9))': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT LEAST(GREATEST(3, 4), MIN(5, 9))", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT LEAST(GREATEST(3, 4), MIN(5, 9))': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},

             {__LINE__, "SELECT MAX(MAX(3, 4), 9)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MAX(MAX(3, 4), 9)': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT MAX(MIN(3, 4), 9)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MAX(MIN(3, 4), 9)': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT MIN(MAX(3, 4), 9)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MIN(MAX(3, 4), 9)': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT MIN(MIN(3, 4), 9)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MIN(MIN(3, 4), 9)': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},

             {__LINE__, "SELECT MAX(MAX(3, 4))", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MAX(MAX(3, 4))': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT MAX(MIN(3, 4))", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MAX(MIN(3, 4))': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT MIN(MAX(3, 4))", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MIN(MAX(3, 4))': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, "SELECT MIN(MIN(3, 4))", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MIN(MIN(3, 4))': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},
         }) {
        listener.ClearIssues();
        ECSqlStatement statement;
        EXPECT_EQ(expectedStatus, statement.Prepare(m_ecdb, sqlStatement.c_str())) << "Test case at line " << lineNumber << " failed.\n";
        if (expectedStatus == ECSqlStatus::Success) {
            EXPECT_EQ(BE_SQLITE_ROW, statement.Step()) << "Test case at line " << lineNumber << " failed.\n";
            for (auto i = 0; i < expectedResult.size(); ++i)
                EXPECT_EQ(expectedResult[i], statement.GetValueText(i)) << "Test case at line " << lineNumber << " failed.\n";
        } else {
            EXPECT_STREQ(listener.GetLastMessage().c_str(), errorMsg.c_str()) << "Test case at line " << lineNumber << " failed.\n";
        }
        statement.Finalize();
    }
}

TEST_F(ECDbTestFixture, TestGreatestAndLeastFunctionsDQLAndDML) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("newFile.ecdb", SchemaItem(
                                                                    "<?xml version='1.0' encoding='utf-8'?> "
                                                                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
                                                                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                                    "    <ECEntityClass typeName='TestClass' modifier='None'>"
                                                                    "        <ECProperty propertyName='TestCol1' typeName='int' />"
                                                                    "        <ECProperty propertyName='TestCol2' typeName='int' />"
                                                                    "    </ECEntityClass>"
                                                                    "</ECSchema>")));

    TestIssueListener listener;
    m_ecdb.AddIssueListener(listener);

    for (const auto& [lineNumber, isSelect, sqlStatement, expectedStatus, expectedResult, errorMsg] : bvector<std::tuple<const int, const bool, const Utf8String, const ECSqlStatus, const std::vector<std::pair<Utf8String, Utf8String>>, const Utf8String>>{
             // Inserts with literals
             {__LINE__, false, "INSERT INTO ts.TestClass(TestCol1, TestCol2) VALUES(GREATEST(5, 2, 1, 4), 0)", ECSqlStatus::Success, {{"5", "0"}}, ""},
             {__LINE__, false, "INSERT INTO ts.TestClass(TestCol1, TestCol2) VALUES(LEAST(5, 2, 1, 4), 0)", ECSqlStatus::Success, {{"5", "0"}, {"1", "0"}}, ""},
             {__LINE__, false, "INSERT INTO ts.TestClass(TestCol1, TestCol2) VALUES(GREATEST(5, 2, 1, 4), GREATEST(6, 9, 4))", ECSqlStatus::Success, {{"5", "0"}, {"1", "0"}, {"5", "9"}}, ""},
             {__LINE__, false, "INSERT INTO ts.TestClass(TestCol1, TestCol2) VALUES(GREATEST(2, 1, 4), LEAST(6, 1, 4))", ECSqlStatus::Success, {{"5", "0"}, {"1", "0"}, {"5", "9"}, {"4", "1"}}, ""},
             {__LINE__, false, "INSERT INTO ts.TestClass(TestCol1, TestCol2) VALUES(LEAST(2, 1, 4), GREATEST(6, 1, 4))", ECSqlStatus::Success, {{"5", "0"}, {"1", "0"}, {"5", "9"}, {"4", "1"}, {"1", "6"}}, ""},
             {__LINE__, false, "INSERT INTO ts.TestClass(TestCol1, TestCol2) VALUES(LEAST(8, 7, 9), LEAST(6, 1, 4))", ECSqlStatus::Success, {{"5", "0"}, {"1", "0"}, {"5", "9"}, {"4", "1"}, {"1", "6"}, {"7", "1"}}, ""},

             // Select with column values
             {__LINE__, true, "SELECT COUNT(*) FROM ts.TestClass WHERE TestCol2=GREATEST(1, 2, 6)", ECSqlStatus::Success, {{"1", ""}}, ""},
             {__LINE__, true, "SELECT COUNT(*) FROM ts.TestClass WHERE TestCol1=least(9, 7, 5)", ECSqlStatus::Success, {{"2", ""}}, ""},

             {__LINE__, true, "SELECT GREATEST(TestCol1, TestCol2) FROM ts.TestClass", ECSqlStatus::Success, {{"5", ""}, {"1", ""}, {"9", ""}, {"4", ""}, {"6", ""}, {"7", ""}}, ""},
             {__LINE__, true, "SELECT GREATEST(GREATEST(TestCol1, TestCol2)) FROM ts.TestClass", ECSqlStatus::Success, {{"9", ""}}, ""},
             {__LINE__, true, "SELECT GREATEST(LEAST(TestCol1, TestCol2)) FROM ts.TestClass", ECSqlStatus::Success, {{"5", ""}}, ""},
             {__LINE__, true, "SELECT LEAST(TestCol1, TestCol2) FROM ts.TestClass", ECSqlStatus::Success, {{"0", ""}, {"0", ""}, {"5", ""}, {"1", ""}, {"1", ""}, {"1", ""}}, ""},
             {__LINE__, true, "SELECT LEAST(GREATEST(TestCol1, TestCol2)) FROM ts.TestClass", ECSqlStatus::Success, {{"1", ""}}, ""},
             {__LINE__, true, "SELECT LEAST(LEAST(TestCol1, TestCol2)) FROM ts.TestClass", ECSqlStatus::Success, {{"0", ""}}, ""},

             {__LINE__, true, "SELECT GREATEST(TestCol1, TestCol2) FROM ts.TestClass ORDER BY TestCol1", ECSqlStatus::Success, {{"1", ""}, {"6", ""}, {"4", ""}, {"5", ""}, {"9", ""}, {"7", ""}}, ""},
             {__LINE__, true, "SELECT GREATEST(TestCol1, TestCol2) FROM ts.TestClass ORDER BY TestCol2 DESC", ECSqlStatus::Success, {{"9", ""}, {"6", ""}, {"4", ""}, {"7", ""}, {"5", ""}, {"1", ""}}, ""},
             {__LINE__, true, "SELECT GREATEST(TestCol1, (Select LEAST(TestCol1, TestCol2) from ts.TestClass)) from ts.TestClass ORDER BY TestCol1", ECSqlStatus::Success, {{"1", ""}, {"1", ""}, {"4", ""}, {"5", ""}, {"5", ""}, {"7", ""}}, ""},
             {__LINE__, true, "SELECT GREATEST(TestCol1, (Select LEAST(TestCol1, TestCol2) from ts.TestClass ORDER BY TestCol2)) from ts.TestClass", ECSqlStatus::Success, {{"5", ""}, {"1", ""}, {"5", ""}, {"4", ""}, {"1", ""}, {"7", ""}}, ""},
             {__LINE__, true, "SELECT * FROM (SELECT GREATEST(TestCol1, (Select LEAST(TestCol1, TestCol2) from ts.TestClass ORDER BY TestCol2)) AS TestCol from ts.TestClass) TEMP WHERE TestCol >= GREATEST(1, 4, 5)", ECSqlStatus::Success, {{"5", ""}, {"5", ""}, {"7", ""}}, ""},
             {__LINE__, true, "SELECT * FROM (SELECT GREATEST(TestCol1, (Select LEAST(TestCol1, TestCol2) from ts.TestClass ORDER BY TestCol2)) AS TestCol from ts.TestClass) TEMP WHERE TestCol <= LEAST(5, 7, 4)", ECSqlStatus::Success, {{"1", ""}, {"4", ""}, {"1", ""}}, ""},
             {__LINE__, true, "SELECT * FROM (SELECT LEAST(TestCol1, (Select GREATEST(TestCol1, TestCol2) from ts.TestClass ORDER BY TestCol2)) AS TestCol from ts.TestClass) TEMP WHERE TestCol = GREATEST(1, 5)", ECSqlStatus::Success, {{"5", ""}, {"5", ""}, {"5", ""}}, ""},
             {__LINE__, true, "SELECT LEAST(4, GREATEST(TestCol1, (Select LEAST(TestCol1, TestCol2) from ts.TestClass ORDER BY TestCol1))) from ts.TestClass", ECSqlStatus::Success, {{"4", ""}, {"1", ""}, {"4", ""}, {"4", ""}, {"1", ""}, {"4", ""}}, ""},

             {__LINE__, true, "SELECT MAX(GREATEST(TestCol1, TestCol2)) FROM ts.TestClass", ECSqlStatus::Success, {{"9", ""}}, ""},
             {__LINE__, true, "SELECT MAX(LEAST(TestCol1, TestCol2)) FROM ts.TestClass", ECSqlStatus::Success, {{"5", ""}}, ""},
             {__LINE__, true, "SELECT MIN(GREATEST(TestCol1, TestCol2)) FROM ts.TestClass", ECSqlStatus::Success, {{"1", ""}}, ""},
             {__LINE__, true, "SELECT MIN(LEAST(TestCol1, TestCol2)) FROM ts.TestClass", ECSqlStatus::Success, {{"0", ""}}, ""},

             // Select with column values (Negative tests)
             {__LINE__, true, "SELECT MAX(TestCol1, TestCol2) FROM ts.TestClass", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MAX(TestCol1, TestCol2) FROM ts.TestClass': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, true, "SELECT MAX(MAX(TestCol1, TestCol2)) FROM ts.TestClass", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MAX(MAX(TestCol1, TestCol2)) FROM ts.TestClass': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, true, "SELECT MAX(MIN(TestCol1, TestCol2)) FROM ts.TestClass", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MAX(MIN(TestCol1, TestCol2)) FROM ts.TestClass': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},
             {__LINE__, true, "SELECT MIN(TestCol1, TestCol2) FROM ts.TestClass", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MIN(TestCol1, TestCol2) FROM ts.TestClass': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},
             {__LINE__, true, "SELECT MIN(MAX(TestCol1, TestCol2)) FROM ts.TestClass", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MIN(MAX(TestCol1, TestCol2)) FROM ts.TestClass': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, true, "SELECT MIN(MIN(TestCol1, TestCol2)) FROM ts.TestClass", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT MIN(MIN(TestCol1, TestCol2)) FROM ts.TestClass': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},

             {__LINE__, true, "select GREATEST\n( 3,\n4)\n FROM ts.TestClass", ECSqlStatus::Success, {{"4", ""}, {"4", ""}, {"4", ""}, {"4", ""}, {"4", ""}, {"4", ""}}, ""},
             {__LINE__, true, "select LEAST\n( 3, \n4)\nFROM ts.TestClass", ECSqlStatus::Success, {{"3", ""}, {"3", ""}, {"3", ""}, {"3", ""}, {"3", ""}, {"3", ""}}, ""},
             {__LINE__, true, "select MAX\n( 3, \n4)\nFROM ts.TestClass", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'select MAX\n( 3, \n4)\nFROM ts.TestClass': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, true, "select MIN\n( 3, \n4)\nFROM ts.TestClass", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'select MIN\n( 3, \n4)\nFROM ts.TestClass': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},

             // Update using least function with multiple literals
             {__LINE__, false, "UPDATE ts.TestClass SET TestCol1=greatest(3, 1, 2)", ECSqlStatus::Success, {{"3", "0"}, {"3", "0"}, {"3", "9"}, {"3", "1"}, {"3", "6"}, {"3", "1"}}, ""},
             {__LINE__, false, "UPDATE ts.TestClass SET TestCol2 = LEAST(12, 45, 20, 15) WHERE TestCol2=0", ECSqlStatus::Success, {{"3", "12"}, {"3", "12"}, {"3", "9"}, {"3", "1"}, {"3", "6"}, {"3", "1"}}, ""},

             // Delete
             {__LINE__, false, "DELETE FROM ts.TestClass WHERE TestCol2 = least(3, 2, 1)", ECSqlStatus::Success, {{"3", "12"}, {"3", "12"}, {"3", "9"}, {"3", "6"}}, ""},
             {__LINE__, false, "DELETE FROM ts.TestClass WHERE TestCol1 = greatest(3, 2, 1)", ECSqlStatus::Success, {}, ""},

             // DML (Negative tests)
             {__LINE__, false, "INSERT INTO ts.TestClass(TestCol1, TestCol2) VALUES(MAX(3, 2, 1))", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'INSERT INTO ts.TestClass(TestCol1, TestCol2) VALUES(MAX(3, 2, 1))': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, false, "UPDATE ts.TestClass SET TestCol1=max(5, 0, 1)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'UPDATE ts.TestClass SET TestCol1=max(5, 0, 1)': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, true, "SELECT COUNT(*) FROM ts.TestClass WHERE TestCol1 = MAX(3, 0, 1)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT COUNT(*) FROM ts.TestClass WHERE TestCol1 = MAX(3, 0, 1)': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},
             {__LINE__, false, "DELETE FROM ts.TestClass WHERE TestCol1 = max(3, 0, 1)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'DELETE FROM ts.TestClass WHERE TestCol1 = max(3, 0, 1)': Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])"},

             {__LINE__, false, "INSERT INTO ts.TestClass(TestCol1, TestCol2) VALUES(MIN(3, 2, 1))", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'INSERT INTO ts.TestClass(TestCol1, TestCol2) VALUES(MIN(3, 2, 1))': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},
             {__LINE__, false, "UPDATE ts.TestClass SET TestCol1=MIN(5, 0, 1)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'UPDATE ts.TestClass SET TestCol1=MIN(5, 0, 1)': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},
             {__LINE__, true, "SELECT COUNT(*) FROM ts.TestClass WHERE TestCol1 = min(3, 0, 1)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'SELECT COUNT(*) FROM ts.TestClass WHERE TestCol1 = min(3, 0, 1)': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},
             {__LINE__, false, "DELETE FROM ts.TestClass WHERE TestCol1 = min(3, 0, 1)", ECSqlStatus::InvalidECSql, {}, "Failed to parse ECSQL 'DELETE FROM ts.TestClass WHERE TestCol1 = min(3, 0, 1)': Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])"},
         }) {
        // Reset listener queue for current test case
        listener.ClearIssues();

        ECSqlStatement testCaseSQL;
        // Prepare the SQL statement
        EXPECT_EQ(expectedStatus, testCaseSQL.Prepare(m_ecdb, sqlStatement.c_str(), true)) << "Test case at line " << lineNumber << " failed.\n";

        if (expectedStatus == ECSqlStatus::Success) {
            if (isSelect) {
                // Test the select statement test case results
                auto i = 0U;
                while (BE_SQLITE_ROW == testCaseSQL.Step()) {
                    EXPECT_EQ(expectedResult[i].first, testCaseSQL.GetValueText(0)) << "Test case at line " << lineNumber << " failed.\n";
                    if (!Utf8String::IsNullOrEmpty(expectedResult[i].second.c_str()))
                        EXPECT_EQ(expectedResult[i].second, testCaseSQL.GetValueText(1)) << "Test case at line " << lineNumber << " failed.\n";
                    ++i;
                }
                testCaseSQL.Finalize();
            } else {
                // Test the insert, update and delete test case results
                EXPECT_EQ(BE_SQLITE_DONE, testCaseSQL.Step()) << "Test case at line " << lineNumber << " failed.\n";
                testCaseSQL.Finalize();

                // Query the table to test whether insert/update/delete has been done
                ECSqlStatement selectStatement;
                if (selectStatement.Prepare(m_ecdb, "SELECT TestCol1, TestCol2 FROM ts.TestClass") == ECSqlStatus::Success) {
                    auto i = 0U;
                    while (BE_SQLITE_ROW == selectStatement.Step()) {
                        EXPECT_EQ(expectedResult[i].first, selectStatement.GetValueText(0)) << "Test case at line " << lineNumber << " failed.\n";
                        if (!Utf8String::IsNullOrEmpty(expectedResult[i].second.c_str()))
                            EXPECT_EQ(expectedResult[i].second, selectStatement.GetValueText(1)) << "Test case at line " << lineNumber << " failed.\n";
                        ++i;
                    }
                }
                selectStatement.Finalize();
            }
        } else {
            // Test if the parsing for MIN/MAX has failed with appropriate error message
            EXPECT_STREQ(listener.GetLastMessage().c_str(), errorMsg.c_str()) << "Test case at line " << lineNumber << " failed.\n";
        }
    }
}

TEST_F(ECDbTestFixture, NullOrEmptyStringUnitLabelsInCompositeFormats) {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("UnitLabelsCompositeFormat.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    Utf8CP schemaXml = R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u" />

            <Unit typeName="TestUnitMajor" displayLabel="TestUnitMajorLabel" definition="u:KM" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />
            <Unit typeName="TestUnitMiddle" displayLabel="TestUnitMiddleLabel" definition="u:M" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />
            <Unit typeName="TestUnitMinor" displayLabel="TestUnitMinorLabel" definition="u:CM" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />
            <Unit typeName="TestUnitSub" displayLabel="TestUnitSubLabel" definition="u:MM" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />

            <Format typeName="TestFormat" displayLabel="TestFormat" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero" precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" " >
                <Composite spacer="=" includeZero="False">
                    <Unit label="romeo">TestUnitMajor</Unit>
                    <Unit label="oscar">TestUnitMiddle</Unit>
                    <Unit label="hotel">TestUnitMinor</Unit>
                    <Unit label="tango">TestUnitSub</Unit>
                </Composite>
            </Format>

            <KindOfQuantity typeName="TestKOQ1" description="Test KOQ1" displayLabel="TestKOQ1" persistenceUnit="u:M" presentationUnits="TestFormat[TestUnitMajor][TestUnitMiddle|foxtrot][TestUnitMinor|][TestUnitSub|tango]" relativeError="10e-3" />

            <Format typeName="TestFormatWithSpecialLabels" displayLabel="TestFormatWithSpecialLabels" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero" precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" " >
                <Composite spacer="=" includeZero="False">
                    <Unit>TestUnitMajor</Unit>
                    <Unit label="">TestUnitMiddle</Unit>
                    <Unit label="&quot;">TestUnitMinor</Unit>
                    <Unit label="tango">TestUnitSub</Unit>
                </Composite>
            </Format>

            <KindOfQuantity typeName="TestKOQ2" description="Test KOQ2" displayLabel="TestKOQ2" persistenceUnit="u:M" presentationUnits="TestFormatWithSpecialLabels[TestUnitMajor|][TestUnitMiddle][TestUnitMinor][TestUnitSub|tango]" relativeError="10e-3" />
        </ECSchema>)xml";

    const std::vector<std::pair<bool, Utf8CP>> compositeSpecStringLabels = {{true, "romeo"}, {true, "oscar"}, {true, "hotel"}, {true, "tango"}};
    const std::vector<std::pair<bool, Utf8CP>> compositeSpecDifferentLabels = {{false, "TestUnitMajorLabel"}, {true, ""}, {true, "\""}, {true, "tango"}};
    const std::vector<std::pair<bool, Utf8CP>> overriddenFormatKOQ1 = {{false, "TestUnitMajorLabel"}, {true, "foxtrot"}, {true, ""}, {true, "tango"}};
    const std::vector<std::pair<bool, Utf8CP>> overriddenFormatKOQ2 = {{true, ""}, {false, "TestUnitMiddleLabel"}, {false, "TestUnitMinorLabel"}, {true, "tango"}};

    static const auto verifyCompositeFormatLabels = [](NamedFormatCR format, std::vector<std::pair<bool, Utf8CP>> expectedResults, Utf8CP errMsg) -> void {
        ASSERT_EQ(expectedResults.size(), 4) << "Composite Units cannot be more than 4";

        const auto spec = format.GetCompositeSpec();

        EXPECT_EQ(expectedResults[0].first, spec->HasMajorLabel()) << errMsg;
        EXPECT_EQ(expectedResults[1].first, spec->HasMiddleLabel()) << errMsg;
        EXPECT_EQ(expectedResults[2].first, spec->HasMinorLabel()) << errMsg;
        EXPECT_EQ(expectedResults[3].first, spec->HasSubLabel()) << errMsg;

        EXPECT_STREQ(expectedResults[0].second, spec->GetMajorLabel().c_str()) << errMsg;
        EXPECT_STREQ(expectedResults[1].second, spec->GetMiddleLabel().c_str()) << errMsg;
        EXPECT_STREQ(expectedResults[2].second, spec->GetMinorLabel().c_str()) << errMsg;
        EXPECT_STREQ(expectedResults[3].second, spec->GetSubLabel().c_str()) << errMsg;
    };

    static const auto verifyFormats = [&](ECSchemaCP schema, Utf8CP errMsg) -> void {
        ASSERT_TRUE(schema);

        // Test the composite units on the main format
        verifyCompositeFormatLabels(*schema->GetFormatCP("TestFormat"), compositeSpecStringLabels, Utf8PrintfString("%s for TestFormat.", errMsg).c_str());
        verifyCompositeFormatLabels(*schema->GetFormatCP("TestFormatWithSpecialLabels"), compositeSpecDifferentLabels, Utf8PrintfString("%s for TestFormatWithSpecialLabels.", errMsg).c_str());

        // Test the overriden format composite units on both the KoQ
        const auto koq1 = schema->GetKindOfQuantityCP("TestKOQ1");
        ASSERT_TRUE(koq1);
        verifyCompositeFormatLabels(koq1->GetPresentationFormats()[0], overriddenFormatKOQ1, Utf8PrintfString("%s for TestKOQ1.", errMsg).c_str());

        const auto koq2 = schema->GetKindOfQuantityCP("TestKOQ2");
        ASSERT_TRUE(koq2);
        verifyCompositeFormatLabels(koq2->GetPresentationFormats()[0], overriddenFormatKOQ2, Utf8PrintfString("%s for TestKOQ2.", errMsg).c_str());
    };

    // Test case 1: Import schema and check if composite formats are correct
    {
        ASSERT_EQ(SUCCESS, GetHelper().ImportSchemas({SchemaItem(schemaXml)}));
        verifyFormats(m_ecdb.Schemas().GetSchema("TestSchema"), "Failed when testing schema import");
    }

    // Test case 2: RoundTrip Test
    {
        Utf8String serializedSchemaXml;
        // Deserialize original XML
        ECSchemaPtr schema;
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
        verifyFormats(schema.get(), "Failed when testing initial schema deserialization");

        //  Serialize it to Xml
        ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml, ECVersion::Latest));

        // Deserialize it back and verify the formats in the schema
        ECSchemaPtr roundtrippedSchema;
        ECSchemaReadContextPtr newContext = ECSchemaReadContext::CreateContext();
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(roundtrippedSchema, serializedSchemaXml.c_str(), *newContext));
        verifyFormats(roundtrippedSchema.get(), "Failed when testing final schema deserialization");
    }
}

END_ECDBUNITTESTS_NAMESPACE
