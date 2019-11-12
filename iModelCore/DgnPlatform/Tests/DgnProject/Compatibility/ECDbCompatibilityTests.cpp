/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"
#include "Profiles.h"
#include "TestECDbCreators.h"
#include "TestDb.h"

USING_NAMESPACE_BENTLEY_EC

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct ECDbCompatibilityTestFixture : CompatibilityTestFixture
    {
    protected:
        void SetUp() override 
            { 
            CompatibilityTestFixture::SetUp();
            ASSERT_EQ(SUCCESS, TestECDbCreation::Run()); 
            }
    };

//---------------------------------------------------------------------------------------
// Runs basic tests on all available test files. This is a basic test to cover tests and test files
// which are added in the future, and to which existing test runners cannot be adjusted to.
// @bsimethod                                  Krischan.Eberle                      07/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, BasicTestsOnAllPulledFiles)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfAllPulledTestFiles())
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            // Run SELECT statements against all classes
            for (ECSchemaCP schema : testDb.GetDb().Schemas().GetSchemas())
                {
                if (schema->GetName().Equals("ECDbSystem"))
                    continue; //doesn't have mapped classes

                for (ECClassCP cl : schema->GetClasses())
                    {
                    if (!cl->IsEntityClass() && !cl->IsRelationshipClass())
                        continue;

                    Utf8String ecsql("SELECT ECInstanceId,ECClassId");
                    for (ECPropertyCP prop : cl->GetProperties())
                        {
                        ecsql.append(",[").append(prop->GetName()).append("]");
                        }
                    ecsql.append(" FROM ").append(cl->GetECSqlName());

                    ECSqlStatement stmt;
                    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), ecsql.c_str())) << ecsql << " | " << testDb.GetDescription();
                    const DbResult stepStat = stmt.Step();
                    ASSERT_TRUE(BE_SQLITE_DONE == stepStat || BE_SQLITE_ROW == stepStat) << ecsql << " | " << testDb.GetDescription();
                    }
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                      07/19
//+---------------+---------------+---------------+---------------+---------------+------
void Assert_BuiltinSchemaVersions_4_0_0_1 (TestECDb& testDb)
    {
    EXPECT_EQ(5, testDb.GetSchemaCount()) << testDb.GetDescription();
    //ECDb built-in schema versions
    EXPECT_EQ(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), testDb.GetSchemaItemCounts("ECDbFileInfo")) << testDb.GetDescription();

    EXPECT_EQ(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbMap")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), testDb.GetSchemaItemCounts("ECDbMap")) << testDb.GetDescription();

    EXPECT_EQ(SchemaVersion(4, 0, 0), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbMeta")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":24, "enumcount": 8})js"), testDb.GetSchemaItemCounts("ECDbMeta")) << testDb.GetDescription();

    EXPECT_EQ(SchemaVersion(5, 0, 0), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbSystem")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), testDb.GetSchemaItemCounts("ECDbSystem")) << testDb.GetDescription();

    //Standard schema versions
    EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("CoreCustomAttributes")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), testDb.GetSchemaItemCounts("CoreCustomAttributes")) << testDb.GetDescription();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                      07/19
//+---------------+---------------+---------------+---------------+---------------+------
void Assert_BuiltinSchemaVersions_4_0_0_2 (TestECDb& testDb)
    {
    EXPECT_EQ(5, testDb.GetSchemaCount()) << testDb.GetDescription();
    //ECDb built-in schema versions
    EXPECT_EQ(SchemaVersion(2, 0, 1), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion (3, 2), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), testDb.GetSchemaItemCounts("ECDbFileInfo")) << testDb.GetDescription();

    EXPECT_EQ(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion (3, 1), testDb.GetOriginalECXmlVersion("ECDbMap")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), testDb.GetSchemaItemCounts("ECDbMap")) << testDb.GetDescription();

    EXPECT_EQ(SchemaVersion(4, 0, 1), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion (3, 2), testDb.GetOriginalECXmlVersion("ECDbMeta")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":38, "enumcount": 8})js"), testDb.GetSchemaItemCounts("ECDbMeta")) << testDb.GetDescription();

    EXPECT_EQ(SchemaVersion(5, 0, 1), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion (3, 2), testDb.GetOriginalECXmlVersion("ECDbSystem")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), testDb.GetSchemaItemCounts("ECDbSystem")) << testDb.GetDescription();

    //Standard schema versions
    EXPECT_LE(SchemaVersion(1, 0, 1), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
    EXPECT_LE(BeVersion (3, 1), testDb.GetOriginalECXmlVersion("CoreCustomAttributes")) << testDb.GetDescription();
    EXPECT_LE(15, testDb.GetSchemaItemCounts("CoreCustomAttributes").m_value["classcount"].asInt());
    EXPECT_LE(2, testDb.GetSchemaItemCounts("CoreCustomAttributes").m_value["enumcount"].asInt());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                      07/19
//+---------------+---------------+---------------+---------------+---------------+------
void Assert_BuiltinSchemaVersions_4_X_X_X(TestECDb& testDb)
    {
    EXPECT_LE(5, testDb.GetSchemaCount()) << testDb.GetDescription();

    //ECDb built-in schema versions
    EXPECT_LE(SchemaVersion(2, 0, 1), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
    EXPECT_LE(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << testDb.GetDescription();
    
    EXPECT_LE(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbMap")) << testDb.GetDescription();
    
    EXPECT_LE(SchemaVersion(4, 0, 1), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
    EXPECT_LE(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbMeta")) << testDb.GetDescription();
    
    EXPECT_LE(SchemaVersion(5, 0, 1), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
    EXPECT_LE(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbSystem")) << testDb.GetDescription();

    //Standard schema versions
    EXPECT_LE(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("CoreCustomAttributes")) << testDb.GetDescription();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, BuiltinSchemaVersions)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EMPTY))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            switch (testDb.GetAge())
                {
                case ProfileState::Age::Older:
                    {
                    if (testDb.GetECDbProfileVersion() == ProfileVersion(4, 0, 0, 1))
                        Assert_BuiltinSchemaVersions_4_0_0_1(testDb);
                    else 
                        FAIL() << "*ERROR* case not handled | " << testDb.GetDescription();
                    break;
                    }
                case ProfileState::Age::UpToDate:
                    {
                    if (testDb.GetECDbProfileVersion() == ProfileVersion(4, 0, 0, 2))
                        Assert_BuiltinSchemaVersions_4_0_0_2(testDb);
                    else 
                        FAIL() << "*ERROR* case not handled | " << testDb.GetDescription();
                    break;
                    }

                case ProfileState::Age::Newer:
                    {
                    Assert_BuiltinSchemaVersions_4_X_X_X(testDb);
                    break;
                    }
                default:
                    FAIL() << "Unhandled ProfileState::Age enum value | " << testDb.GetDescription();
                    break;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, ECSqlColumnInfoForAliases)
    {
    auto assertColInfo = [] (TestDb const& testDb, ECSqlColumnInfo const& colInfo, bool hasAlias)
        {
        ASSERT_TRUE(colInfo.GetProperty() != nullptr) << testDb.GetDescription();
        Utf8StringCR selectClauseItem = colInfo.GetProperty()->GetDisplayLabel();

        EXPECT_EQ(hasAlias, colInfo.IsGeneratedProperty()) << selectClauseItem << " | " << testDb.GetDescription();
        EXPECT_EQ(!hasAlias, colInfo.IsSystemProperty()) << selectClauseItem << " | " << testDb.GetDescription();
        EXPECT_EQ(PRIMITIVETYPE_Long, colInfo.GetDataType().GetPrimitiveType()) << selectClauseItem << " | " << testDb.GetDescription();

        ASSERT_TRUE(colInfo.GetProperty()->GetIsPrimitive()) << selectClauseItem << " | " << testDb.GetDescription();
        // Since 4.0.0.2 the Id system properties in the ECDbSystem schema have the extended type "Id"
        // However, when using aliases, the generated prop gets that extended type regardless of the profile version.
        // The test asserts that ruleset.
        if (hasAlias || testDb.SupportsFeature(ECDbFeature::SystemPropertiesHaveIdExtendedType))
            EXPECT_STREQ("Id", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << selectClauseItem << " | " << testDb.GetDescription();
        else
            EXPECT_FALSE(colInfo.GetProperty()->HasExtendedType()) << selectClauseItem << " | " << testDb.GetDescription();
        };

    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EMPTY))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "SELECT ECInstanceId, ECClassId, ECInstanceId id, ECClassId classId FROM ecdbf.FileInfo")) << testDb.GetDescription();
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << testDb.GetDescription();
            assertColInfo(testDb, stmt.GetColumnInfo(0), false);
            assertColInfo(testDb, stmt.GetColumnInfo(1), false);
            assertColInfo(testDb, stmt.GetColumnInfo(2), true);
            assertColInfo(testDb, stmt.GetColumnInfo(3), true);
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, ECInstanceId id, ECClassId classId, SourceECInstanceId sourceId, SourceECClassId sourceClassId, TargetECInstanceId targetId, TargetECClassId targetClassId FROM meta.ClassHasAllBaseClasses LIMIT 1")) << testDb.GetDescription();
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << testDb.GetDescription();
            assertColInfo(testDb, stmt.GetColumnInfo(0), false);
            assertColInfo(testDb, stmt.GetColumnInfo(1), false);
            assertColInfo(testDb, stmt.GetColumnInfo(2), false);
            assertColInfo(testDb, stmt.GetColumnInfo(3), false);
            assertColInfo(testDb, stmt.GetColumnInfo(4), false);
            assertColInfo(testDb, stmt.GetColumnInfo(5), false);
            assertColInfo(testDb, stmt.GetColumnInfo(6), true);
            assertColInfo(testDb, stmt.GetColumnInfo(7), true);
            assertColInfo(testDb, stmt.GetColumnInfo(8), true);
            assertColInfo(testDb, stmt.GetColumnInfo(9), true);
            assertColInfo(testDb, stmt.GetColumnInfo(10), true);
            assertColInfo(testDb, stmt.GetColumnInfo(11), true);
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "SELECT Schema.Id, Schema.RelECClassId, Schema.Id schemaId, Schema.RelECClassId schemaRelClassId FROM meta.ECClassDef LIMIT 1")) << testDb.GetDescription();
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << testDb.GetDescription();
            assertColInfo(testDb, stmt.GetColumnInfo(0), false);
            assertColInfo(testDb, stmt.GetColumnInfo(1), false);
            assertColInfo(testDb, stmt.GetColumnInfo(2), true);
            assertColInfo(testDb, stmt.GetColumnInfo(3), true);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC31Enums)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC31ENUMS))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            if (!testDb.SupportsFeature(ECDbFeature::NamedEnumerators))
                {
                testDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                    {{"Unspecified", ECValue("Unspecified"), nullptr},
                    {"Utc", ECValue("Utc"), nullptr},
                    {"Local", ECValue("Local"), nullptr}});

                testDb.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                    {{"ECClassModifier0", ECValue(0), "None"},
                    {"ECClassModifier1", ECValue(1), "Abstract"},
                    {"ECClassModifier2", ECValue(2), "Sealed"}});

                testDb.AssertEnum("TestSchema", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                    {{"IntEnum_EnumeratorsWithoutDisplayLabel0", ECValue(0), nullptr},
                    {"IntEnum_EnumeratorsWithoutDisplayLabel1", ECValue(1), nullptr},
                    {"IntEnum_EnumeratorsWithoutDisplayLabel2", ECValue(2), nullptr}});

                testDb.AssertEnum("TestSchema", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                    {{"On", ECValue("On"), "Turned On"},
                    {"Off", ECValue("Off"), "Turned Off"}});
                }
            else
                {
                testDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                    {{"Unspecified", ECValue("Unspecified"), nullptr},
                    {"Utc", ECValue("Utc"), nullptr},
                    {"Local", ECValue("Local"), nullptr}});

                testDb.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                    {{"None", ECValue(0), "None"},
                    {"Abstract", ECValue(1), "Abstract"},
                    {"Sealed", ECValue(2), "Sealed"}});

                testDb.AssertEnum("TestSchema", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                    {{"IntEnum_EnumeratorsWithoutDisplayLabel0", ECValue(0), nullptr},
                    {"IntEnum_EnumeratorsWithoutDisplayLabel1", ECValue(1), nullptr},
                    {"IntEnum_EnumeratorsWithoutDisplayLabel2", ECValue(2), nullptr}});

                testDb.AssertEnum("TestSchema", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                    {{"On", ECValue("On"), "Turned On"},
                    {"Off", ECValue("Off"), "Turned Off"}});
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      07/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, UpgradingEC31EnumsToEC32AfterProfileUpgrade)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32ENUMS_PROFILEUPGRADED))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            // older files for which the schema upgrade wasn't run must have the auto-generated enumerator names
            if (testDb.GetOriginalECXmlVersion("TestSchema") <= BeVersion(3, 1))
                {
                testDb.AssertEnum("TestSchema", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                    {{"IntEnum_EnumeratorsWithoutDisplayLabel0", ECValue(0), nullptr},
                    {"IntEnum_EnumeratorsWithoutDisplayLabel1", ECValue(1), nullptr},
                    {"IntEnum_EnumeratorsWithoutDisplayLabel2", ECValue(2), nullptr}});

                testDb.AssertEnum("TestSchema", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                    {{"On", ECValue("On"), "Turned On"},
                    {"Off", ECValue("Off"), "Turned Off"}});
                }
            else
                {
                testDb.AssertEnum("TestSchema", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                    {{"Unknown", ECValue(0), nullptr},
                    {"On", ECValue(1), nullptr},
                    {"Off", ECValue(2), nullptr}});

                testDb.AssertEnum("TestSchema", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                    {{"An", ECValue("On"), "Turned On"},
                    {"Aus", ECValue("Off"), "Turned Off"}});
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32Enums)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32ENUMS))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            if (testDb.VersionSupportsFeature(testDb.GetECDbInitialVersion(), ECDbFeature::NamedEnumerators))
                {
                EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();

                testDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                    {{"Unspecified", ECValue("Unspecified"), nullptr},
                    {"Utc", ECValue("Utc"), nullptr},
                    {"Local", ECValue("Local"), nullptr}});

                testDb.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                    {{"None", ECValue(0), "None"},
                    {"Abstract", ECValue(1), "Abstract"},
                    {"Sealed", ECValue(2), "Sealed"}});

                testDb.AssertEnum("TestSchema", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                    {{"Unknown", ECValue(0), nullptr},
                    {"On", ECValue(1), nullptr},
                    {"Off", ECValue(2), nullptr}});

                testDb.AssertEnum("TestSchema", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                    {{"On", ECValue("On"), "Turned On"},
                    {"Off", ECValue("Off"), "Turned Off"}});

                continue;
                }

            // original file is 4.0.0.1
            EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();

            if (!testDb.SupportsFeature(ECDbFeature::NamedEnumerators))
                {
                // file still is a 4.0.0.1 file -> wasn't upgraded
                testDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                    {{"Unspecified", ECValue("Unspecified"), nullptr},
                    {"Utc", ECValue("Utc"), nullptr},
                    {"Local", ECValue("Local"), nullptr}});

                testDb.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                    {{"ECClassModifier0", ECValue(0), "None"},
                    {"ECClassModifier1", ECValue(1), "Abstract"},
                    {"ECClassModifier2", ECValue(2), "Sealed"}});

                testDb.AssertEnum("TestSchema", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                    {{"IntEnum_EnumeratorsWithoutDisplayLabel0", ECValue(0), nullptr},
                    {"IntEnum_EnumeratorsWithoutDisplayLabel1", ECValue(1), nullptr},
                    {"IntEnum_EnumeratorsWithoutDisplayLabel2", ECValue(2), nullptr}});

                testDb.AssertEnum("TestSchema", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                    {{"On", ECValue("On"), "Turned On"},
                    {"Off", ECValue("Off"), "Turned Off"}});

                continue;
                }

            // file was upgraded to 4.0.0.2
            EXPECT_TRUE(testDb.GetTestFile().IsUpgraded() || testDb.IsUpgraded()) << testDb.GetDescription();

            testDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                {{"Unspecified", ECValue("Unspecified"), nullptr},
                {"Utc", ECValue("Utc"), nullptr},
                {"Local", ECValue("Local"), nullptr}});

            testDb.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{"None", ECValue(0), "None"},
                {"Abstract", ECValue(1), "Abstract"},
                {"Sealed", ECValue(2), "Sealed"}});

            testDb.AssertEnum("TestSchema", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{"IntEnum_EnumeratorsWithoutDisplayLabel0", ECValue(0), nullptr},
                {"IntEnum_EnumeratorsWithoutDisplayLabel1", ECValue(1), nullptr},
                {"IntEnum_EnumeratorsWithoutDisplayLabel2", ECValue(2), nullptr}});

            testDb.AssertEnum("TestSchema", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                {{"On", ECValue("On"), "Turned On"},
                {"Off", ECValue("Off"), "Turned Off"}});
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC31KindOfQuantities)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC31KOQS))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            testDb.AssertKindOfQuantity("TestSchema", "ANGLE", "Angle", nullptr, "u:RAD", JsonValue(R"json(["f:DefaultRealU(2)[u:ARC_DEG]", "f:AngleDMS"])json"), 0.0001);
            testDb.AssertKindOfQuantity("TestSchema", "POWER", "Power", nullptr, "u:W", JsonValue(R"json(["f:DefaultRealU(4)[u:W]", "f:DefaultRealU(4)[u:KW]", "f:DefaultRealU(4)[u:MEGAW]", "f:DefaultRealU(4)[u:BTU_PER_HR]", "f:DefaultRealU(4)[u:KILOBTU_PER_HR]", "f:DefaultRealU(4)[u:HP]"])json"), 0.001);
            testDb.AssertKindOfQuantity("TestSchema", "LIQUID_VOLUME", "Liquid Volume", nullptr, "u:CUB_M", JsonValue(R"json(["f:DefaultRealU(4)[u:LITRE]", "f:DefaultRealU(4)[u:GALLON]"])json"), 0.0001);

            if (TestDb::VersionSupportsFeature(testDb.GetECDbInitialVersion(), ECDbFeature::UnitsAndFormats))
                {
                testDb.AssertKindOfQuantity("TestSchema", "TestKoq_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(), 0.4);

                // KOQs which are actually invalid in EC3.1 and could not be deserialized in pre EC3.2 code. In EC3.2 code this is tolerated
                // but invalid pres formats are dropped.
                // So these tests may only be run if the file was created with code that supports EC3.2 or later
                testDb.AssertKindOfQuantity("TestSchema", "TestKoq_LUX_M", nullptr, nullptr, "u:LUX", JsonValue(), 1.1);
                testDb.AssertKindOfQuantity("TestSchema", "TestKoq_M_LUX", nullptr, nullptr, "u:M", JsonValue(), 1.2);
                testDb.AssertKindOfQuantity("TestSchema", "TestKoq_M_SQFTreal4u", nullptr, nullptr, "u:M", JsonValue(), 1.3);
                testDb.AssertKindOfQuantity("TestSchema", "TestKoq_M_CM_LUX", nullptr, nullptr, "u:M", JsonValue(R"json(["f:DefaultReal[u:CM]"])json"), 1.4);
                testDb.AssertKindOfQuantity("TestSchema", "TestKoq_LUX_CM_MM", nullptr, nullptr, "u:LUX", JsonValue(), 1.5);
                testDb.AssertKindOfQuantity("TestSchema", "TestKoq_LUXreal4u_CM_MM", nullptr, nullptr, "u:LUX", JsonValue(R"json(["f:DefaultRealU(4)[u:LUX]"])json"), 1.6);
                }
            else
                {
                //The original KOQ was serialized to disk in bim02dev in a wrong way, where it did persist the format along with the unit,
                //although it shouldn't have one. This will not be fixed, as EC32 will make this obsolete anyways.
                testDb.AssertKindOfQuantity("TestSchema", "TestKoq_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(R"json(["f:DefaultReal[u:W_PER_M_K]"])json"), 0.4);
                }
            
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(R"json(["f:DefaultReal[u:W_PER_M_K]"])json"), 0.5);
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_PersUnitWithFormatWithUnit_NoPresUnit", nullptr, nullptr, "u:FT", JsonValue(R"json(["f:AmerFI"])json"), 0.6);
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_M_Mfi8", nullptr, nullptr, "u:M", JsonValue(R"json(["f:AmerFI"])json"), 0.7);
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_Mfi8", nullptr, nullptr, "u:M", JsonValue(R"json(["f:AmerFI"])json"), 0.8);
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_SQFTfi8", nullptr, nullptr, "u:SQ_FT", JsonValue(), 0.9);
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_SQFTfi8_SQFTreal4u", nullptr, nullptr, "u:SQ_FT", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_FT]"])json"), 1.0);

            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Units", false) != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Formats", false) != nullptr) << testDb.GetDescription();

            testDb.AssertUnit("Units", "COULOMB", "C", nullptr, "A*S", nullptr, nullptr, nullptr, QualifiedName("Units", "SI"), QualifiedName("Units", "ELECTRIC_CHARGE"), false, QualifiedName());
            //specifically test for thread pitch units which were added later to the unit schema.
            testDb.AssertUnit("Units", "IN_PER_DEGREE", "in/degree", nullptr, "IN*ARC_DEG(-1)", nullptr, nullptr, nullptr, QualifiedName("Units", "USCUSTOM"), QualifiedName("Units", "THREAD_PITCH"), false, QualifiedName());
            testDb.AssertUnit("Units", "M_PER_REVOLUTION", "m/r", nullptr, "M*REVOLUTION(-1)", nullptr, nullptr, nullptr, QualifiedName("Units", "INTERNATIONAL"), QualifiedName("Units", "THREAD_PITCH"), false, QualifiedName());

            testDb.AssertUnitSystem("Units", "INTERNATIONAL", nullptr, nullptr);
            testDb.AssertUnitSystem("Units", "SI", nullptr, nullptr);
            testDb.AssertPhenomenon("Units", "LUMINOSITY", "Luminosity", nullptr, "LUMINOSITY");
            testDb.AssertPhenomenon("Units", "THREAD_PITCH", "Thread Pitch", nullptr, "LENGTH*ANGLE(-1)");
            testDb.AssertFormat("Formats", "AmerFI", "FeetInches", nullptr, JsonValue(R"json({"type": "Fractional", "formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"], "precision": 8, "uomSeparator":""})json"),
                                JsonValue(R"json({"includeZero":true, "spacer":"", "units": [{"name":"FT", "label":"'"}, {"name":"IN", "label":"\""}]})json"));

            if (!testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                {
                ECSqlStatement stmt;
                EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(testDb.GetDb(), "SELECT * FROM meta.UnitDef")) << testDb.GetDescription();
                stmt.Finalize();
                EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(testDb.GetDb(), "SELECT * FROM meta.UnitSystemDef")) << testDb.GetDescription();
                stmt.Finalize();
                EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(testDb.GetDb(), "SELECT * FROM meta.PhenomenonDef")) << testDb.GetDescription();
                stmt.Finalize();
                EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(testDb.GetDb(), "SELECT * FROM meta.FormatDef")) << testDb.GetDescription();
                stmt.Finalize();
                EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(testDb.GetDb(), "SELECT * FROM meta.FormatCompositeUnitDef")) << testDb.GetDescription();
                stmt.Finalize();
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC31ThreadPitchKindOfQuantities)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC31THREADPITCHKOQS))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_IN_DEGREE", nullptr, nullptr, "u:IN_PER_DEGREE", JsonValue(), 1.0);
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_IN_DEGREE_DEFAULTREALU", nullptr, nullptr, "u:IN_PER_DEGREE", JsonValue(R"json(["f:DefaultRealU[u:IN_PER_DEGREE]"])json"), 1.1);
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_M_REVOLUTION", nullptr, nullptr, "u:M_PER_REVOLUTION", JsonValue(), 1.2);
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_M_REVOLUTION_DEFAULTREALU", nullptr, nullptr, "u:M_PER_REVOLUTION", JsonValue(R"json(["f:DefaultRealU[u:M_PER_REVOLUTION]"])json"), 1.3);

            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Units", false) != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Formats", false) != nullptr) << testDb.GetDescription();

            testDb.AssertUnit("Units", "IN_PER_DEGREE", "in/degree", nullptr, "IN*ARC_DEG(-1)", nullptr, nullptr, nullptr, QualifiedName("Units", "USCUSTOM"), QualifiedName("Units", "THREAD_PITCH"), false, QualifiedName());
            testDb.AssertUnit("Units", "M_PER_REVOLUTION", "m/r", nullptr, "M*REVOLUTION(-1)", nullptr, nullptr, nullptr, QualifiedName("Units", "INTERNATIONAL"), QualifiedName("Units", "THREAD_PITCH"), false, QualifiedName());
            testDb.AssertUnitSystem("Units", "INTERNATIONAL", nullptr, nullptr);
            testDb.AssertPhenomenon("Units", "THREAD_PITCH", "Thread Pitch", nullptr, "LENGTH*ANGLE(-1)");
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, SchemaManager_EC31KindOfQuantities)
    {
    Utf8CP koq1Name = "TestKoq_M_Mfi8";
    Utf8CP koq2Name = "TestKoq_SQFTfi8_SQFTreal4u";

    auto assertReferencedUnitsAndFormatsSchema = [] (TestECDb const& testDb, ECSchemaCR koqSchema)
        {
        auto it = koqSchema.GetReferencedSchemas().Find(SchemaKey("Units", 1, 0), ECN::SchemaMatchType::LatestReadCompatible);
        ASSERT_FALSE(it == koqSchema.GetReferencedSchemas().end()) << testDb.GetDescription();
        ASSERT_EQ(testDb.SupportsFeature(ECDbFeature::UnitsAndFormats), it->second->HasId()) << testDb.GetDescription();

        it = koqSchema.GetReferencedSchemas().Find(SchemaKey("Formats", 1, 0), ECN::SchemaMatchType::LatestReadCompatible);
        ASSERT_FALSE(it == koqSchema.GetReferencedSchemas().end()) << testDb.GetDescription();
        ASSERT_EQ(testDb.SupportsFeature(ECDbFeature::UnitsAndFormats), it->second->HasId()) << testDb.GetDescription();
        };

    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC31KOQS))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            testDb.GetDb().ClearECDbCache();

            //load KOQs with empty cache
            KindOfQuantityCP koq1 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq1Name);
            ASSERT_TRUE(koq1 != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq1, "TestSchema", koq1Name, nullptr, nullptr, "u:M", JsonValue(R"json(["f:AmerFI"])json"), 0.7);

            KindOfQuantityCP koq2 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq2Name);
            ASSERT_TRUE(koq2 != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq2, "TestSchema", koq2Name, nullptr, nullptr, "u:SQ_FT", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_FT]"])json"), 1.0);
            testDb.GetDb().ClearECDbCache();

            //load KOQs after schema stub was loaded (but without elements)
            ASSERT_TRUE(testDb.GetDb().Schemas().GetSchema("TestSchema", false) != nullptr) << testDb.GetDescription();

            koq1 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq1Name);
            ASSERT_TRUE(koq1 != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq1, "TestSchema", koq1Name, nullptr, nullptr, "u:M", JsonValue(R"json(["f:AmerFI"])json"), 0.7);

            koq2 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq2Name);
            ASSERT_TRUE(koq2 != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq2, "TestSchema", koq2Name, nullptr, nullptr, "u:SQ_FT", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_FT]"])json"), 1.0);
            testDb.GetDb().ClearECDbCache();

            //load KOQs after schema was fully loaded
            ASSERT_TRUE(testDb.GetDb().Schemas().GetSchema("TestSchema", true) != nullptr) << testDb.GetDescription();

            koq1 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq1Name);
            ASSERT_TRUE(koq1 != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq1, "TestSchema", koq1Name, nullptr, nullptr, "u:M", JsonValue(R"json(["f:AmerFI"])json"), 0.7);

            koq2 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq2Name);
            ASSERT_TRUE(koq2 != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq2, "TestSchema", koq2Name, nullptr, nullptr, "u:SQ_FT", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_FT]"])json"), 1.0);
            testDb.GetDb().ClearECDbCache();

            //load KOQs after all schema stubs were loaded (no elements)
            testDb.GetDb().Schemas().GetSchemas(false);
            koq1 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq1Name);
            ASSERT_TRUE(koq1 != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq1, "TestSchema", koq1Name, nullptr, nullptr, "u:M", JsonValue(R"json(["f:AmerFI"])json"), 0.7);

            koq2 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq2Name);
            ASSERT_TRUE(koq2 != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq2, "TestSchema", koq2Name, nullptr, nullptr, "u:SQ_FT", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_FT]"])json"), 1.0);
            testDb.GetDb().ClearECDbCache();

            //load KOQs after all schema were fully loaded
            testDb.GetDb().Schemas().GetSchemas(true);
            koq1 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq1Name);
            ASSERT_TRUE(koq1 != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq1, "TestSchema", koq1Name, nullptr, nullptr, "u:M", JsonValue(R"json(["f:AmerFI"])json"), 0.7);

            koq2 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq2Name);
            ASSERT_TRUE(koq2 != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq2, "TestSchema", koq2Name, nullptr, nullptr, "u:SQ_FT", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_FT]"])json"), 1.0);
            testDb.GetDb().ClearECDbCache();

            //Load schema elements after a KOQ was loaded (and the temporary schemas were deserialized)
            //This must ignore the temporary schema references as they don't have a schema id
            koq1 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq1Name);
            ASSERT_TRUE(koq1 != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq1, "TestSchema", koq1Name, nullptr, nullptr, "u:M", JsonValue(R"json(["f:AmerFI"])json"), 0.7);
            assertReferencedUnitsAndFormatsSchema(testDb, koq1->GetSchema());

            koq2 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq2Name);
            ASSERT_TRUE(koq2 != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq2, "TestSchema", koq2Name, nullptr, nullptr, "u:SQ_FT", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_FT]"])json"), 1.0);
            assertReferencedUnitsAndFormatsSchema(testDb, koq2->GetSchema());

            bvector<ECSchemaCP> schemas = testDb.GetDb().Schemas().GetSchemas(true);
            ASSERT_EQ(8, schemas.size()) << testDb.GetDescription();
            bool containsUnitsSchema = false, containsFormatsSchema = false;
            for (ECSchemaCP schema : schemas)
                {
                if (schema->GetName().Equals("Units"))
                    containsUnitsSchema = true;
                if (schema->GetName().Equals("Formats"))
                    containsFormatsSchema = true;
                }
            ASSERT_TRUE(containsUnitsSchema) << testDb.GetDescription();
            ASSERT_TRUE(containsFormatsSchema) << testDb.GetDescription();

            testDb.GetDb().ClearECDbCache();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32KindOfQuantities)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32KOQS))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            ASSERT_TRUE(testDb.SupportsFeature(ECDbFeature::UnitsAndFormats)) << testDb.GetDescription();

            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_PresFormatWithMandatoryComposite", "My first test KOQ", nullptr, "u:CM", JsonValue(R"js(["f:DefaultRealU(4)[u:M]"])js"), 0.1);
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_PresFormatWithOptionalComposite", nullptr, "My second test KOQ", "u:CM", JsonValue(R"js(["f:AmerFI[u:FT|feet][u:IN|inches]"])js"), 0.2);
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_PresFormatWithoutComposite", nullptr, nullptr, "u:CM", JsonValue(R"js(["f:AmerFI"])js"), 0.3);
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_NoPresFormat", nullptr, nullptr, "u:KG", JsonValue(), 0.4);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC31Units)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EMPTY))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            //this test only tests files which do not support EC32 units yet
            if (testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                continue;

            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            testDb.GetDb().ClearECDbCache();

            EXPECT_EQ(5, testDb.GetDb().Schemas().GetSchemas(false).size()) << testDb.GetDescription();

            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Units") == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("u", false, SchemaLookupMode::ByAlias) == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Formats") == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("f", false, SchemaLookupMode::ByAlias) == nullptr) << testDb.GetDescription();

            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnit("Units", "CM") == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnit("u", "CM", SchemaLookupMode::ByAlias) == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnitSystem("Units", "SI") == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnitSystem("u", "SI", SchemaLookupMode::ByAlias) == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetPhenomenon("Units", "AREA") == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetPhenomenon("u", "AREA", SchemaLookupMode::ByAlias) == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetFormat("Formats", "DefaultReal") == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetFormat("f", "DefaultReal", SchemaLookupMode::ByAlias) == nullptr) << testDb.GetDescription();

            if (testDb.GetOpenParams().IsReadonly())
                continue;

            // now import a schema with a KOQ. This should trigger deserializing the units/formats schema from disk
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <KindOfQuantity typeName="ANGLE" displayLabel="Angle" persistenceUnit="RAD(DefaultReal)" presentationUnits="ARC_DEG(real2u);ARC_DEG(dms)" relativeError="0.0001"/>
                     </ECSchema>)xml"));
            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            ASSERT_EQ(SUCCESS, testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas())) << testDb.GetDescription();

            //3 more schemas: the imported test schema and the in-memory units/formats schemas
            EXPECT_EQ(8, testDb.GetDb().Schemas().GetSchemas(false).size()) << testDb.GetDescription();

            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Units") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("u", false, SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Formats") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("f", false, SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();

            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnit("Units", "CM") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnit("u", "CM", SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnitSystem("Units", "SI") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnitSystem("u", "SI", SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetPhenomenon("Units", "AREA") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetPhenomenon("u", "AREA", SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetFormat("Formats", "DefaultReal") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetFormat("f", "DefaultReal", SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();
            }
        }

    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC31KOQS))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            //this test only tests files which do not support EC32 units yet
            if (testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                continue;

            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            testDb.GetDb().ClearECDbCache();

            EXPECT_EQ(8, testDb.GetDb().Schemas().GetSchemas(false).size()) << testDb.GetDescription();

            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Units") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("u", false, SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Formats") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("f", false, SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();

            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnit("Units", "CM") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnit("u", "CM", SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnitSystem("Units", "SI") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnitSystem("u", "SI", SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetPhenomenon("Units", "AREA") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetPhenomenon("u", "AREA", SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetFormat("Formats", "DefaultReal") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetFormat("f", "DefaultReal", SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();

            if (testDb.GetOpenParams().IsReadonly())
                continue;

            // now import another schema. This should work fine even though the previous code has triggered to deserialize the units and format schema into memory.
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="NewSchema" alias="ns" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <KindOfQuantity typeName="ANGLE" displayLabel="Angle" persistenceUnit="RAD(DefaultReal)" presentationUnits="ARC_DEG(real2u);ARC_DEG(dms)" relativeError="0.0001"/>
                     </ECSchema>)xml"));
            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            ASSERT_EQ(SUCCESS, testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas())) << testDb.GetDescription();

            EXPECT_EQ(9, testDb.GetDb().Schemas().GetSchemas(false).size()) << testDb.GetDescription();

            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Units") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("u", false, SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Formats") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("f", false, SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();

            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnit("Units", "CM") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnit("u", "CM", SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnitSystem("Units", "SI") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetUnitSystem("u", "SI", SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetPhenomenon("Units", "AREA") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetPhenomenon("u", "AREA", SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetFormat("Formats", "DefaultReal") != nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetFormat("f", "DefaultReal", SchemaLookupMode::ByAlias) != nullptr) << testDb.GetDescription();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32Units)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32UNITS))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            ASSERT_TRUE(testDb.SupportsFeature(ECDbFeature::UnitsAndFormats)) << testDb.GetDescription();

            testDb.AssertKindOfQuantity("TestSchema", "KoqWithCustomFormat", nullptr, nullptr, "u:M", JsonValue(R"js(["MyFormat[u:M]"])js"), 0.1);
            testDb.AssertKindOfQuantity("TestSchema", "KoqWithCustomUnit", nullptr, nullptr, "MySquareM", JsonValue(R"js(["f:DefaultRealU(4)[MySquareM]"])js"), 0.2);
            testDb.AssertKindOfQuantity("TestSchema", "KoqWithCustomUnitAndFormat", nullptr, nullptr, "MySquareFt", JsonValue(R"js(["MyFormat[MySquareFt]"])js"), 0.3);

            testDb.AssertUnitSystem("TestSchema", "MyMetric", "Metric", "Metric Units of measure");
            testDb.AssertUnitSystem("TestSchema", "MyImperial", "Imperial", "Units of measure from the British Empire");
            testDb.AssertUnitSystem("Units", "SI", nullptr, nullptr);
            testDb.AssertUnitSystem("Units", "CONSTANT", nullptr, nullptr);

            testDb.AssertPhenomenon("TestSchema", "MyArea", "Area", nullptr, "LENGTH*LENGTH");
            testDb.AssertPhenomenon("Units", "AREA", "Area", nullptr, "LENGTH(2)");
            testDb.AssertPhenomenon("Units", "TORQUE", "Torque", nullptr, "FORCE*LENGTH*ANGLE(-1)");
            testDb.AssertPhenomenon("Units", "LUMINOSITY", "Luminosity", nullptr, "LUMINOSITY");

            testDb.AssertUnit("TestSchema", "MySquareM", "Square Meter", nullptr, "M*M", 1.0, nullptr, nullptr, QualifiedName("TestSchema", "MyMetric"), QualifiedName("TestSchema", "MyArea"), false, QualifiedName());
            testDb.AssertUnit("TestSchema", "MySquareFt", "Square Feet", nullptr, "Ft*Ft", 10.0, nullptr, 0.4, QualifiedName("TestSchema", "MyImperial"), QualifiedName("TestSchema", "MyArea"), false, QualifiedName());
            testDb.AssertUnit("Units", "COULOMB", "C", nullptr, "A*S", nullptr, nullptr, nullptr, QualifiedName("Units", "SI"), QualifiedName("Units", "ELECTRIC_CHARGE"), false, QualifiedName());
            testDb.AssertUnit("Units", "PI", "Pi", nullptr, "ONE", 3.1415926535897932384626433832795, nullptr, nullptr, QualifiedName(), QualifiedName("Units", "LENGTH_RATIO"), true, QualifiedName());
            testDb.AssertUnit("Units", "QUARTER_PI", "Pi/4", nullptr, "PI", 1.0, 4.0, nullptr, QualifiedName(), QualifiedName("Units", "LENGTH_RATIO"), true, QualifiedName());
            testDb.AssertUnit("Units", "MILLI", "milli", nullptr, "ONE", .001, nullptr, nullptr, QualifiedName(), QualifiedName("Units", "NUMBER"), true, QualifiedName());
            testDb.AssertUnit("Units", "HORIZONTAL_PER_VERTICAL", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, QualifiedName("Units", "INTERNATIONAL"), QualifiedName("Units", "SLOPE"), false, QualifiedName("Units", "VERTICAL_PER_HORIZONTAL"));

            testDb.AssertFormat("TestSchema", "MyFormat", "My Format", nullptr, JsonValue(R"json({"roundFactor":0.3, "type": "Fractional", "showSignOption": "OnlyNegative", "formatTraits": ["trailZeroes", "keepSingleZero"], "precision": 4, "decimalSeparator": ".", "thousandSeparator": ",", "uomSeparator": " "})json"), JsonValue());
            testDb.AssertFormat("TestSchema", "MyFormatWithComposite", "My Format with composite", nullptr, JsonValue(R"json({"type": "Decimal", "formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"], "precision": 2})json"),
                                JsonValue(R"json({"includeZero":true, "spacer":"-", "units": [{"name":"HR", "label":"hour"}, {"name":"MIN", "label":"min"}]})json"));
            testDb.AssertFormat("Formats", "DefaultReal", "real", nullptr, JsonValue(R"json({"type": "Decimal", "formatTraits": ["keepSingleZero", "keepDecimalPoint"], "precision": 6})json"), JsonValue());
            testDb.AssertFormat("Formats", "AmerFI", "FeetInches", nullptr, JsonValue(R"json({"type": "Fractional", "formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"], "precision": 8, "uomSeparator":""})json"),
                                JsonValue(R"json({"includeZero":true, "spacer":"", "units": [{"name":"FT", "label":"'"}, {"name":"IN", "label":"\""}]})json"));
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC31SchemaImport)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EMPTY))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                        <ECSchemaReference name="ECDbFileInfo" version="02.00.00" alias="ecdbf" />
                        <ECEntityClass typeName="Foo">
                                <ECCustomAttributes>
                                    <ClassMap xmlns="ECDbMap.02.00.00">
                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                </ECCustomAttributes>
                                <ECProperty propertyName="Code" typeName="int" />
                                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                                <ECProperty propertyName="Status" typeName="StatusEnum" />
                        </ECEntityClass>
                        <ECEntityClass typeName="MyFileInfo">
                            <BaseClass>ecdbf:FileInfo</BaseClass>
                            <ECProperty propertyName="Path" typeName="string" />
                        </ECEntityClass>
                        <KindOfQuantity typeName="ANGLE" displayLabel="Angle" persistenceUnit="RAD(DefaultReal)" presentationUnits="ARC_DEG(real2u);ARC_DEG(dms)" relativeError="0.0001"/>
                        <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="SQ.M(DefaultReal)" presentationUnits="SQ.M(real4u);SQ.FT(real4u)" relativeError="0.0001"/>
                        <KindOfQuantity typeName="TEMPERATURE" displayLabel="Temperature" persistenceUnit="K(DefaultReal)" presentationUnits="CELSIUS(real4u);FAHRENHEIT(real4u);K(real4u)" relativeError="0.01"/>
                        <ECEnumeration typeName="StatusEnum" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                            <ECEnumerator value="0"/>
                            <ECEnumerator value="1"/>
                            <ECEnumerator value="2"/>
                        </ECEnumeration>
                     </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const BentleyStatus schemaImportStat = testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetAge())
                {
                EXPECT_EQ(ERROR, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            ASSERT_EQ(SUCCESS, schemaImportStat) << testDb.GetDescription();

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "INSERT INTO ts.Foo(Code,Size,Status) VALUES(1,3.0,2)")) << testDb.GetDescription();

            ECInstanceKey fooKey;
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooKey)) << testDb.GetDescription();
            EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id": "%s", "Code": 1, "Size": 3.0, "Status": 2}])json", fooKey.GetInstanceId().ToHexStr().c_str())), testDb.ExecuteECSqlSelect("SELECT ECInstanceId, Code, Size, Status FROM ts.Foo")) << testDb.GetDescription();

            ECClassCP fooClass = testDb.GetDb().Schemas().GetClass("TestSchema", "Foo");
            ASSERT_TRUE(fooClass != nullptr && fooClass->IsEntityClass()) << testDb.GetDescription();

            ECPropertyCP sizeProp = fooClass->GetPropertyP("Size");
            ASSERT_TRUE(sizeProp != nullptr && sizeProp->GetIsPrimitive()) << testDb.GetDescription();
            KindOfQuantityCP koq = sizeProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
            ASSERT_TRUE(koq != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]"])json"), 0.0001);

            ECPropertyCP statusProp = fooClass->GetPropertyP("Status");
            ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << testDb.GetDescription();
            ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
            ASSERT_TRUE(ecenum != nullptr) << testDb.GetDescription();

            testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{"StatusEnum0", ECValue(0), nullptr},
                {"StatusEnum1", ECValue(1), nullptr},
                {"StatusEnum2", ECValue(2), nullptr}});

            testDb.AssertKindOfQuantity("TestSchema", "ANGLE", "Angle", nullptr, "u:RAD", JsonValue(R"json(["f:DefaultRealU(2)[u:ARC_DEG]", "f:AngleDMS"])json"), 0.0001);
            testDb.AssertKindOfQuantity("TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]"])json"), 0.0001);
            testDb.AssertKindOfQuantity("TestSchema", "TEMPERATURE", "Temperature", nullptr, "u:K", JsonValue(R"json(["f:DefaultRealU(4)[u:CELSIUS]","f:DefaultRealU(4)[u:FAHRENHEIT]","f:DefaultRealU(4)[u:K]"])json"), 0.01);

            testDb.AssertEnum("TestSchema", "StatusEnum", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{"StatusEnum0", ECValue(0), nullptr},
                {"StatusEnum1", ECValue(1), nullptr},
                {"StatusEnum2", ECValue(2), nullptr}});

            if (testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                {
                EXPECT_EQ(JsonValue("[{\"cnt\": 2}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef WHERE Name IN ('Units','Formats')")) << "When importing into 4.0.0.2 or newer file, units and formats schema must be persisted. | " << testDb.GetDescription();
                EXPECT_EQ(JsonValue("[{\"cnt\": 4}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef s JOIN meta.SchemaHasSchemaReferences ref ON s.ECInstanceId=ref.SourceECInstanceId WHERE s.Name='TestSchema'")) << "When importing into 4.0.0.2 or newer file, units and formats schema must be persisted. | " << testDb.GetDescription();
                }
            else
                {
                EXPECT_EQ(JsonValue("[{\"cnt\": 0}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef WHERE Name IN ('Units','Formats')")) << "When importing into 4.0.0.1 file, units and formats schema must not be persisted. | " << testDb.GetDescription();
                EXPECT_EQ(JsonValue("[{\"cnt\": 2}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef s JOIN meta.SchemaHasSchemaReferences ref ON s.ECInstanceId=ref.SourceECInstanceId WHERE s.Name='TestSchema'")) << "When importing into 4.0.0.1 file, units and formats schema must not be persisted. | " << testDb.GetDescription();
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC31SchemaImportWithEC32Reference)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EMPTY))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            // Schema uses a newer version of the ECDbFileInfo schema -> add ecdb schema search path
            BeFileName ecdbStandardSchemasFolder(ECSchemaReadContext::GetHostAssetsDirectory());
            ecdbStandardSchemasFolder.AppendToPath(L"ECSchemas");
            ecdbStandardSchemasFolder.AppendToPath(L"ECDb");
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchemas(testDb.GetDb(), {
                    SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="BaseSchema" alias="bs" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                        <ECEntityClass typeName="MyBaseClass">
                            <ECCustomAttributes>
                                <ClassMap xmlns="ECDbMap.02.00.00">
                                    <MapStrategy>TablePerHierarchy</MapStrategy>
                                </ClassMap>
                            </ECCustomAttributes>
                            <ECProperty propertyName="Code" typeName="int" />
                        </ECEntityClass>
                     </ECSchema>)xml"),
                    SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <ECSchemaReference name="BaseSchema" version="01.00.00" alias="bs" />
                        <ECEntityClass typeName="Foo">
                            <BaseClass>bs:MyBaseClass</BaseClass>
                                <ECProperty propertyName="Code" typeName="int" />
                                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                                <ECProperty propertyName="Status" typeName="StatusEnum" />
                        </ECEntityClass>
                        <KindOfQuantity typeName="ANGLE" displayLabel="Angle" persistenceUnit="RAD(DefaultReal)" presentationUnits="ARC_DEG(real2u);ARC_DEG(dms)" relativeError="0.0001"/>
                        <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="SQ.M(DefaultReal)" presentationUnits="SQ.M(real4u);SQ.FT(real4u)" relativeError="0.0001"/>
                        <KindOfQuantity typeName="TEMPERATURE" displayLabel="Temperature" persistenceUnit="K(DefaultReal)" presentationUnits="CELSIUS(real4u);FAHRENHEIT(real4u);K(real4u)" relativeError="0.01"/>
                        <ECEnumeration typeName="StatusEnum" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                            <ECEnumerator value="0"/>
                            <ECEnumerator value="1"/>
                            <ECEnumerator value="2"/>
                        </ECEnumeration>
                     </ECSchema>)xml")}, {ecdbStandardSchemasFolder});

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const BentleyStatus schemaImportStat = testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetAge() || !testDb.SupportsFeature(ECDbFeature::EC32))
                {
                EXPECT_EQ(ERROR, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            ASSERT_EQ(SUCCESS, schemaImportStat) << testDb.GetDescription();

            EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();
            EXPECT_EQ(JsonValue("[{\"cnt\": 2}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef WHERE Name IN ('Units','Formats')")) << "When importing into 4.0.0.1 file, units and formats schema must not be persisted. | " << testDb.GetDescription();
            EXPECT_EQ(JsonValue("[{\"cnt\": 3}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef s JOIN meta.SchemaHasSchemaReferences ref ON s.ECInstanceId=ref.SourceECInstanceId WHERE s.Name='TestSchema'")) << "When importing into 4.0.0.1 file, units and formats schema must not be persisted. | " << testDb.GetDescription();

            ECInstanceKey fooKey;
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "INSERT INTO ts.Foo(Code,Size,Status) VALUES(1,3.0,2)")) << testDb.GetDescription();
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooKey)) << testDb.GetDescription();
            stmt.Finalize();
            EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id": "%s", "Code": 1, "Size": 3.0, "Status": 2}])json", fooKey.GetInstanceId().ToHexStr().c_str())), testDb.ExecuteECSqlSelect("SELECT ECInstanceId, Code, Size, Status FROM ts.Foo")) << testDb.GetDescription();

            ECClassCP fooClass = testDb.GetDb().Schemas().GetClass("TestSchema", "Foo");
            ASSERT_TRUE(fooClass != nullptr && fooClass->IsEntityClass()) << testDb.GetDescription();

            ECPropertyCP sizeProp = fooClass->GetPropertyP("Size");
            ASSERT_TRUE(sizeProp != nullptr && sizeProp->GetIsPrimitive()) << testDb.GetDescription();
            KindOfQuantityCP koq = sizeProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
            ASSERT_TRUE(koq != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]"])json"), 0.0001);

            ECPropertyCP statusProp = fooClass->GetPropertyP("Status");
            ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << testDb.GetDescription();
            ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
            ASSERT_TRUE(ecenum != nullptr) << testDb.GetDescription();
            testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{"StatusEnum0", ECValue(0), nullptr},
                {"StatusEnum1", ECValue(1), nullptr},
                {"StatusEnum2", ECValue(2), nullptr}});

            testDb.AssertKindOfQuantity("TestSchema", "ANGLE", "Angle", nullptr, "u:RAD", JsonValue(R"json(["f:DefaultRealU(2)[u:ARC_DEG]", "f:AngleDMS"])json"), 0.0001);
            testDb.AssertKindOfQuantity("TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]"])json"), 0.0001);
            testDb.AssertKindOfQuantity("TestSchema", "TEMPERATURE", "Temperature", nullptr, "u:K", JsonValue(R"json(["f:DefaultRealU(4)[u:CELSIUS]","f:DefaultRealU(4)[u:FAHRENHEIT]","f:DefaultRealU(4)[u:K]"])json"), 0.01);

            testDb.AssertEnum("TestSchema", "StatusEnum", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{"StatusEnum0", ECValue(0), nullptr},
                {"StatusEnum1", ECValue(1), nullptr},
                {"StatusEnum2", ECValue(2), nullptr}});
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC31SchemaImportWithReadContextVariations)
    {
    BeFileName ecdbSchemaAssetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(ecdbSchemaAssetsDir);
    ecdbSchemaAssetsDir.AppendToPath(L"ECSchemas").AppendToPath(L"ECDb");

    auto deserializeSchema = [] (ECN::ECSchemaReadContext& ctx)
        {
        ScopedDisableFailOnAssertion disableFailOnAssert;
        ECSchemaPtr schema = nullptr;
        return SchemaReadStatus::Success == ECSchema::ReadFromXmlString(schema, R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <ECSchemaReference name="CoreCustomAttributes" version="01.00.02" alias="cca" />
                        <ECEntityClass typeName="Foo">
                                <ECCustomAttributes>
                                    <HiddenClass xmlns="CoreCustomAttributes.01.00.02" />
                                </ECCustomAttributes>
                                <ECProperty propertyName="Code" typeName="int" />
                                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                                <ECProperty propertyName="Status" typeName="StatusEnum" />
                        </ECEntityClass>
                        <KindOfQuantity typeName="ANGLE" displayLabel="Angle" persistenceUnit="RAD(DefaultReal)" presentationUnits="ARC_DEG(real2u);ARC_DEG(dms)" relativeError="0.0001"/>
                        <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="SQ.M(DefaultReal)" presentationUnits="SQ.M(real4u);SQ.FT(real4u)" relativeError="0.0001"/>
                        <KindOfQuantity typeName="TEMPERATURE" displayLabel="Temperature" persistenceUnit="K(DefaultReal)" presentationUnits="CELSIUS(real4u);FAHRENHEIT(real4u);K(real4u)" relativeError="0.01"/>
                        <ECEnumeration typeName="StatusEnum" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                            <ECEnumerator value="0"/>
                            <ECEnumerator value="1"/>
                            <ECEnumerator value="2"/>
                        </ECEnumeration>
                     </ECSchema>)xml", ctx) ? SUCCESS : ERROR;
        };

    auto assertSchemaImport = [] (TestECDb& testDb, ECSchemaReadContext& ctx, Utf8CP scenario)
        {
        ScopedDisableFailOnAssertion disableFailOnAssert;
        ECSchemaPtr schema = nullptr;
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <ECSchemaReference name="CoreCustomAttributes" version="01.00.02" alias="cca" />
                        <ECEntityClass typeName="Foo">
                                <ECCustomAttributes>
                                    <HiddenClass xmlns="CoreCustomAttributes.01.00.02" />
                                </ECCustomAttributes>
                                <ECProperty propertyName="Code" typeName="int" />
                                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                                <ECProperty propertyName="Status" typeName="StatusEnum" />
                        </ECEntityClass>
                        <KindOfQuantity typeName="ANGLE" displayLabel="Angle" persistenceUnit="RAD(DefaultReal)" presentationUnits="ARC_DEG(real2u);ARC_DEG(dms)" relativeError="0.0001"/>
                        <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="SQ.M(DefaultReal)" presentationUnits="SQ.M(real4u);SQ.FT(real4u)" relativeError="0.0001"/>
                        <KindOfQuantity typeName="TEMPERATURE" displayLabel="Temperature" persistenceUnit="K(DefaultReal)" presentationUnits="CELSIUS(real4u);FAHRENHEIT(real4u);K(real4u)" relativeError="0.01"/>
                        <ECEnumeration typeName="StatusEnum" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                            <ECEnumerator value="0"/>
                            <ECEnumerator value="1"/>
                            <ECEnumerator value="2"/>
                        </ECEnumeration>
                     </ECSchema>)xml", ctx)) << scenario << " | " << testDb.GetDescription();

        BentleyStatus schemaImportStat = testDb.GetDb().Schemas().ImportSchemas(ctx.GetCache().GetSchemas());
        switch (testDb.GetAge())
            {
            case ProfileState::Age::Older:
                if (testDb.GetECDbProfileVersion() == ProfileVersion(4, 0, 0, 1))
                    {
                    ASSERT_EQ(SUCCESS, schemaImportStat) << scenario << " | " << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("TestSchema")) << scenario << " | " << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << scenario << " | " << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbMap")) << scenario << " | " << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbMeta")) << scenario << " | " << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbSystem")) << scenario << " | " << testDb.GetDescription();
                    }
                else
                    FAIL() << "*ERROR* case not handled | " << testDb.GetDescription();
                break;
            case ProfileState::Age::UpToDate:
                if (testDb.GetECDbProfileVersion() == ProfileVersion(4, 0, 0, 2))
                    {
                    ASSERT_EQ(SUCCESS, schemaImportStat) << scenario << " | " << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("TestSchema")) << scenario << " | " << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << scenario << " | " << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbMap")) << scenario << " | " << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbMeta")) << scenario << " | " << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbSystem")) << scenario << " | " << testDb.GetDescription();
                    }
                else
                    FAIL() << "*ERROR* case not handled | " << testDb.GetDescription();
                break;
            case ProfileState::Age::Newer:
                EXPECT_EQ(ERROR, schemaImportStat) << scenario << " | " << testDb.GetDescription();
                break;
            default:
                FAIL() << "Unhandled ProfileState::Age enum value | " << testDb.GetDescription();
                break;
            }
        };

    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EMPTY))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            Utf8CP scenario = "Read context with ECDb schema locater";
            ECN::ECSchemaReadContextPtr ctx = ECN::ECSchemaReadContext::CreateContext();
            ctx->AddSchemaLocater(testDb.GetDb().GetSchemaLocater());
            assertSchemaImport(testDb, *ctx, scenario);

            scenario = "Read context with ECDb schema locater and ECDb schema assets folder";
            ctx = ECN::ECSchemaReadContext::CreateContext();
            ctx->AddSchemaLocater(testDb.GetDb().GetSchemaLocater());
            ctx->AddSchemaPath(ecdbSchemaAssetsDir);

            assertSchemaImport(testDb, *ctx, scenario);

            scenario = "Read context with ECDb schema assets folder and ECDb as final schema locater";
            ctx = ECN::ECSchemaReadContext::CreateContext();
            ctx->AddSchemaPath(ecdbSchemaAssetsDir);
            ctx->SetFinalSchemaLocater(testDb.GetDb().GetSchemaLocater());

            assertSchemaImport(testDb, *ctx, scenario);

            scenario = "Read context with only ECDb schema assets folder";
            ctx = ECN::ECSchemaReadContext::CreateContext();
            ctx->AddSchemaPath(ecdbSchemaAssetsDir);
            assertSchemaImport(testDb, *ctx, scenario);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32SchemaImport_Enums)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EMPTY))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECEntityClass typeName="Foo">
                            <ECProperty propertyName="Code" typeName="int" />
                            <ECProperty propertyName="Status" typeName="StatusEnum" />
                        </ECEntityClass>
                    <ECEnumeration typeName="StatusEnum" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                        <ECEnumerator name="On" value="0"/>
                        <ECEnumerator name="Off" value="1"/>
                        <ECEnumerator name="Unknown" value="2"/>
                    </ECEnumeration>
                    </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const BentleyStatus schemaImportStat = testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetAge() || !testDb.SupportsFeature(ECDbFeature::NamedEnumerators))
                {
                EXPECT_EQ(ERROR, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SUCCESS, schemaImportStat) << testDb.GetDescription();
            ECInstanceKey fooKey;
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "INSERT INTO ts.Foo(Code,Status) VALUES(1,2)")) << testDb.GetDescription();
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooKey)) << testDb.GetDescription();
            stmt.Finalize();
            EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id": "%s", "Code": 1, "Status": 2}])json", fooKey.GetInstanceId().ToHexStr().c_str())), testDb.ExecuteECSqlSelect("SELECT ECInstanceId, Code, Status FROM ts.Foo")) << testDb.GetDescription();

            ECClassCP fooClass = testDb.GetDb().Schemas().GetClass("TestSchema", "Foo");
            ASSERT_TRUE(fooClass != nullptr && fooClass->IsEntityClass()) << testDb.GetDescription();

            ECPropertyCP statusProp = fooClass->GetPropertyP("Status");
            ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << testDb.GetDescription();
            ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
            ASSERT_TRUE(ecenum != nullptr) << testDb.GetDescription();
            testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{"On", ECValue(0), nullptr},
                {"Off", ECValue(1), nullptr},
                {"Unknown", ECValue(2), nullptr}});

            testDb.AssertEnum("TestSchema", "StatusEnum", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{"On", ECValue(0), nullptr},
                {"Off", ECValue(1), nullptr},
                {"Unknown", ECValue(2), nullptr}});
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32SchemaImport_Koqs)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EMPTY))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                   <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                        <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                        <ECSchemaReference name="Formats" version="01.00.00" alias="f" />
                        <ECEntityClass typeName="Foo">
                                <ECProperty propertyName="Code" typeName="int" />
                                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                            </ECEntityClass>
                        <KindOfQuantity typeName="ANGLE" displayLabel="Angle" persistenceUnit="u:RAD" presentationUnits="f:DefaultRealU(2)[u:ARC_DEG]" relativeError="0.0001"/>
                        <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="u:SQ_M" presentationUnits="f:DefaultRealU(4)[u:SQ_M];f:DefaultRealU(4)[u:SQ_FT]" relativeError="0.0001"/>
                        <KindOfQuantity typeName="TEMPERATURE" displayLabel="Temperature" persistenceUnit="u:K" presentationUnits="f:DefaultRealU(4)[u:CELSIUS];f:DefaultRealU(4)[u:FAHRENHEIT];f:DefaultRealU(4)[u:K]" relativeError="0.01"/>
                     </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const BentleyStatus schemaImportStat = testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetAge() || !testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                {
                EXPECT_EQ(ERROR, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SUCCESS, schemaImportStat) << testDb.GetDescription();

            ECInstanceKey fooKey;
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "INSERT INTO ts.Foo(Code,Size) VALUES(1,3.0)")) << testDb.GetDescription();
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooKey)) << testDb.GetDescription();
            EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id": "%s", "Code": 1, "Size": 3.0}])json", fooKey.GetInstanceId().ToHexStr().c_str())), testDb.ExecuteECSqlSelect("SELECT ECInstanceId, Code, Size FROM ts.Foo")) << testDb.GetDescription();

            ECClassCP fooClass = testDb.GetDb().Schemas().GetClass("TestSchema", "Foo");
            ASSERT_TRUE(fooClass != nullptr && fooClass->IsEntityClass()) << testDb.GetDescription();

            ECPropertyCP sizeProp = fooClass->GetPropertyP("Size");
            ASSERT_TRUE(sizeProp != nullptr && sizeProp->GetIsPrimitive()) << testDb.GetDescription();
            KindOfQuantityCP koq = sizeProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
            ASSERT_TRUE(koq != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]"])json"), 0.0001);

            testDb.AssertKindOfQuantity("TestSchema", "ANGLE", "Angle", nullptr, "u:RAD", JsonValue(R"json(["f:DefaultRealU(2)[u:ARC_DEG]"])json"), 0.0001);
            testDb.AssertKindOfQuantity("TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]"])json"), 0.0001);
            testDb.AssertKindOfQuantity("TestSchema", "TEMPERATURE", "Temperature", nullptr, "u:K", JsonValue(R"json(["f:DefaultRealU(4)[u:CELSIUS]","f:DefaultRealU(4)[u:FAHRENHEIT]","f:DefaultRealU(4)[u:K]"])json"), 0.01);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      10/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC31SchemaImport_Formats_API)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EMPTY))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            ECSchemaPtr schema;
            ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0)) << testDb.GetDescription();
            schema->SetOriginalECXmlVersion(3, 1);
            ECFormatP format = nullptr;
            Formatting::NumericFormatSpec spec;
            spec.SetPresentationType(Formatting::PresentationType::Decimal);
            spec.SetPrecision(Formatting::DecimalPrecision::Precision6);
            spec.SetFormatTraits((Formatting::FormatTraits) ((int) Formatting::FormatTraits::KeepSingleZero | (int) Formatting::FormatTraits::KeepDecimalPoint));
            ASSERT_EQ(ECObjectsStatus::Success, schema->CreateFormat(format, "DefaultReal", "real", nullptr, &spec)) << testDb.GetDescription();

            ASSERT_EQ(ERROR, testDb.GetDb().Schemas().ImportSchemas({schema.get()})) << testDb.GetDescription();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      10/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC31SchemaUpgrade_Formats_API)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EMPTY))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            // Import base line of a schema, which is then upgraded in the next step
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                   <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <ECEntityClass typeName="Foo">
                            <ECProperty propertyName="Code" typeName="int" />
                        </ECEntityClass>
                     </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            BentleyStatus schemaImportStat = testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas());
            if (testDb.GetAge() == ProfileState::Age::Newer)
                {
                EXPECT_EQ(ERROR, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            ASSERT_EQ(SUCCESS, schemaImportStat) << testDb.GetDescription();
            ASSERT_EQ(BE_SQLITE_OK, testDb.GetDb().SaveChanges()) << testDb.GetDescription();
            testDb.GetDb().CloseDb();
            ASSERT_EQ(BE_SQLITE_OK, testDb.GetDb().OpenBeSQLiteDb(testDb.GetTestFile().GetPath(), testDb.GetOpenParams())) << testDb.GetDescription();

            // now build new version with API and add a format -> this should fail as ECDb does not support
            // schemas that are 3.1 but have EC3.2 features
            ECSchemaPtr schema;
            ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 1)) << testDb.GetDescription();
            schema->SetOriginalECXmlVersion(3, 1);
            ECEntityClassP fooClass = nullptr;
            ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(fooClass, "Foo")) << testDb.GetDescription();
            PrimitiveECPropertyP prop = nullptr;
            ASSERT_EQ(ECObjectsStatus::Success, fooClass->CreatePrimitiveProperty(prop, "Code", PRIMITIVETYPE_Integer)) << testDb.GetDescription();
            ECFormatP format = nullptr;
            Formatting::NumericFormatSpec spec;
            spec.SetPresentationType(Formatting::PresentationType::Decimal);
            spec.SetPrecision(Formatting::DecimalPrecision::Precision6);
            spec.SetFormatTraits((Formatting::FormatTraits) ((int) Formatting::FormatTraits::KeepSingleZero | (int) Formatting::FormatTraits::KeepDecimalPoint));
            ASSERT_EQ(ECObjectsStatus::Success, schema->CreateFormat(format, "DefaultReal", "real", nullptr, &spec)) << testDb.GetDescription();

            ASSERT_EQ(ERROR, testDb.GetDb().Schemas().ImportSchemas({schema.get()})) << testDb.GetDescription();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC31Enums_SchemaUpgrade)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC31ENUMS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //Schema changes:
            //- bumped up version to 1.0.1
            // - Enum StatusEnum
            //    - Changed display label to "Status"
            //    - Added enumerator 3
            // - Added subclasses SubA, SubB, SubC
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEnumeration typeName="StatusEnum" displayLabel="Status" backingTypeName="int" isStrict="true">
                        <ECEnumerator value="0"/>
                        <ECEnumerator value="1"/>
                        <ECEnumerator value="2"/>
                        <ECEnumerator value="3"/>
                    </ECEnumeration>
                    <ECEntityClass typeName="BaseA">
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Status" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubA">
                        <BaseClass>BaseA</BaseClass>
                        <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BaseB">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Status" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubB">
                        <BaseClass>BaseB</BaseClass>
                        <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BaseC">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>6</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Status" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubC">
                        <BaseClass>BaseC</BaseClass>
                        <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
                    </ECEntityClass>
                </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const BentleyStatus schemaImportStat = testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetAge())
                {
                EXPECT_EQ(ERROR, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SUCCESS, schemaImportStat) << testDb.GetDescription();

            if (testDb.SupportsFeature(ECDbFeature::PersistedECVersions))
                EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();
            else
                EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();

            for (Utf8CP className : {"SubA", "SubB", "SubC"})
                {
                ECInstanceKey key;
                ECSqlStatement stmt;
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), Utf8PrintfString("INSERT INTO ts.%s(Code,Status,Statuses) VALUES(?,?,?)", className).c_str())) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 1)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, 3)) << className << " | " << testDb.GetDescription();
                IECSqlBinder& statusBinder = stmt.GetBinder(3);
                ASSERT_EQ(ECSqlStatus::Success, statusBinder.AddArrayElement().BindInt(0)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, statusBinder.AddArrayElement().BindInt(1)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, statusBinder.AddArrayElement().BindInt(2)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << className << " | " << testDb.GetDescription();
                stmt.Finalize();
                EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id": "%s", "Code": 1, "Status": 3, "Statuses": [0,1,2]}])json", key.GetInstanceId().ToHexStr().c_str())),
                            testDb.ExecuteECSqlSelect(Utf8PrintfString("SELECT ECInstanceId,Code,Status,Statuses FROM ts.%s", className).c_str())) << className << " | " << testDb.GetDescription();

                ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", className);
                ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << className << " | " << testDb.GetDescription();

                ECPropertyCP statusProp = cl->GetPropertyP("Status");
                ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << className << " | " << testDb.GetDescription();
                ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
                ASSERT_TRUE(ecenum != nullptr) << className << " | " << testDb.GetDescription();
                testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                    {{"StatusEnum0", ECValue(0), nullptr},
                    {"StatusEnum1", ECValue(1), nullptr},
                    {"StatusEnum2", ECValue(2), nullptr},
                    {"StatusEnum3", ECValue(3), nullptr}});

                ECPropertyCP statusesProp = cl->GetPropertyP("Statuses");
                ASSERT_TRUE(statusesProp != nullptr && statusesProp->GetIsPrimitiveArray()) << className << " | " << testDb.GetDescription();
                ECEnumerationCP statusesEnum = statusesProp->GetAsPrimitiveArrayProperty()->GetEnumeration();
                ASSERT_TRUE(statusesEnum != nullptr) << className << " | " << testDb.GetDescription();
                testDb.AssertEnum(*statusesEnum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                    {{"StatusEnum0", ECValue(0), nullptr},
                    {"StatusEnum1", ECValue(1), nullptr},
                    {"StatusEnum2", ECValue(2), nullptr},
                    {"StatusEnum3", ECValue(3), nullptr}});
                }

            testDb.AssertEnum("TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                {{"StatusEnum0", ECValue(0), nullptr},
                {"StatusEnum1", ECValue(1), nullptr},
                {"StatusEnum2", ECValue(2), nullptr},
                {"StatusEnum3", ECValue(3), nullptr}});
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC31Koqs_SchemaUpgrade)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC31KOQS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //Schema changes:
            //- bumped up version to 1.0.1
            //- KOQ AREA: 
            //    - Added presentation unit SQ.CM
            //    - Changed relativeError to 0.001
            // - Added subclasses SubA, SubB, SubC
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />

                    <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="SQ.M(DefaultReal)" presentationUnits="SQ.M(real4u);SQ.FT(real4u);SQ.CM(real4u)" relativeError="0.001"/>
                    <ECEntityClass typeName="BaseA">
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubA">
                        <BaseClass>BaseA</BaseClass>
                        <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BaseB">
                        <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubB">
                        <BaseClass>BaseB</BaseClass>
                        <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BaseC">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>6</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubC">
                        <BaseClass>BaseC</BaseClass>
                        <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const BentleyStatus schemaImportStat = testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetAge())
                {
                EXPECT_EQ(ERROR, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SUCCESS, schemaImportStat) << testDb.GetDescription();

            if (testDb.SupportsFeature(ECDbFeature::PersistedECVersions))
                EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();
            else
                EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();

            if (testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                {
                EXPECT_EQ(JsonValue("[{\"cnt\": 2}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef WHERE Name IN ('Units','Formats')")) << "When importing into 4.0.0.2 or newer file, units and formats schema must not be persisted. | " << testDb.GetDescription();
                EXPECT_EQ(JsonValue("[{\"cnt\": 3}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef s JOIN meta.SchemaHasSchemaReferences ref ON s.ECInstanceId=ref.SourceECInstanceId WHERE s.Name='TestSchema'")) << "When importing into 4.0.0.2 or newer file, units and formats schema must not be persisted. | " << testDb.GetDescription();
                }
            else
                {
                EXPECT_EQ(JsonValue("[{\"cnt\": 0}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef WHERE Name IN ('Units','Formats')")) << "When importing into 4.0.0.1 file, units and formats schema must not be persisted. | " << testDb.GetDescription();
                EXPECT_EQ(JsonValue("[{\"cnt\": 1}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef s JOIN meta.SchemaHasSchemaReferences ref ON s.ECInstanceId=ref.SourceECInstanceId WHERE s.Name='TestSchema'")) << "When importing into 4.0.0.1 file, units and formats schema must not be persisted. | " << testDb.GetDescription();
                }

            for (Utf8CP className : {"SubA", "SubB", "SubC"})
                {
                ECInstanceKey key;
                ECSqlStatement stmt;
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), Utf8PrintfString("INSERT INTO ts.%s(Code,Size,Sizes) VALUES(?,?,?)", className).c_str())) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 1)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(2, 2.2)) << className << " | " << testDb.GetDescription();

                IECSqlBinder& sizesBinder = stmt.GetBinder(3);
                ASSERT_EQ(ECSqlStatus::Success, sizesBinder.AddArrayElement().BindDouble(3.3)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, sizesBinder.AddArrayElement().BindDouble(33.3)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, sizesBinder.AddArrayElement().BindDouble(333.3)) << className << " | " << testDb.GetDescription();

                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << className << " | " << testDb.GetDescription();
                stmt.Finalize();
                EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id": "%s", "Code": 1, "Size": 2.2, "Sizes" : [3.3,33.3,333.3]}])json", key.GetInstanceId().ToHexStr().c_str())),
                            testDb.ExecuteECSqlSelect(Utf8PrintfString("SELECT ECInstanceId,Code,Size,Sizes FROM ts.%s", className).c_str())) << className << " | " << testDb.GetDescription();

                ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", className);
                ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << className << " | " << testDb.GetDescription();

                ECPropertyCP sizeProp = cl->GetPropertyP("Size");
                ASSERT_TRUE(sizeProp != nullptr && sizeProp->GetIsPrimitive()) << className << " | " << testDb.GetDescription();
                KindOfQuantityCP koq = sizeProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
                ASSERT_TRUE(koq != nullptr) << className << " | " << testDb.GetDescription();
                testDb.AssertKindOfQuantity(*koq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);

                ECPropertyCP sizesProp = cl->GetPropertyP("Sizes");
                ASSERT_TRUE(sizesProp != nullptr && sizesProp->GetIsPrimitiveArray()) << className << " | " << testDb.GetDescription();
                KindOfQuantityCP sizesKoq = sizesProp->GetAsPrimitiveArrayProperty()->GetKindOfQuantity();
                ASSERT_TRUE(sizesKoq != nullptr) << className << " | " << testDb.GetDescription();
                testDb.AssertKindOfQuantity(*sizesKoq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);
                }
            testDb.AssertKindOfQuantity("TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC31ToEC32SchemaUpgrade_Enums)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC31ENUMS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //Schema changes:
            //- bumped up version to 1.0.1
            // - Enum StatusEnum
            //    - Changed display label to "Status"
            //    - add meaningful names to enumerators
            //    - Added enumerator 3
            // - Added subclasses SubA, SubB, SubC
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />

                    <ECEnumeration typeName="StatusEnum" displayLabel="Status" backingTypeName="int" isStrict="true">
                        <ECEnumerator name="On" value="0"/>
                        <ECEnumerator name="Off" value="1"/>
                        <ECEnumerator name="Unknown" value="2"/>
                        <ECEnumerator name="Halfhalf" value="3"/>
                    </ECEnumeration>
                    <ECEntityClass typeName="BaseA">
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Status" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubA">
                        <BaseClass>BaseA</BaseClass>
                        <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BaseB">
                        <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Status" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubB">
                        <BaseClass>BaseB</BaseClass>
                        <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BaseC">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>6</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Status" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubC">
                        <BaseClass>BaseC</BaseClass>
                        <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
                    </ECEntityClass>
                </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const BentleyStatus schemaImportStat = testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetAge() || !testDb.SupportsFeature(ECDbFeature::NamedEnumerators))
                {
                EXPECT_EQ(ERROR, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SUCCESS, schemaImportStat) << testDb.GetDescription();
            EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();

            for (Utf8CP className : {"SubA", "SubB", "SubC"})
                {
                ECInstanceKey key;
                ECSqlStatement stmt;
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), Utf8PrintfString("INSERT INTO ts.%s(Code,Status,Statuses) VALUES(?,?,?)", className).c_str())) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 1)) << className << " | " << testDb.GetDescription();

                ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, 3)) << className << " | " << testDb.GetDescription();

                IECSqlBinder& statusBinder = stmt.GetBinder(3);
                ASSERT_EQ(ECSqlStatus::Success, statusBinder.AddArrayElement().BindInt(0)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, statusBinder.AddArrayElement().BindInt(1)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, statusBinder.AddArrayElement().BindInt(2)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << className << " | " << testDb.GetDescription();
                stmt.Finalize();
                EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id": "%s", "Code": 1, "Status": 3, "Statuses": [0,1,2]}])json", key.GetInstanceId().ToHexStr().c_str())),
                            testDb.ExecuteECSqlSelect(Utf8PrintfString("SELECT ECInstanceId,Code,Status,Statuses FROM ts.%s", className).c_str())) << className << " | " << testDb.GetDescription();

                ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", className);
                ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << className << " | " << testDb.GetDescription();

                ECPropertyCP statusProp = cl->GetPropertyP("Status");
                ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << className << " | " << testDb.GetDescription();
                ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
                ASSERT_TRUE(ecenum != nullptr) << className << " | " << testDb.GetDescription();
                testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                    {{"On", ECValue(0), nullptr},
                    {"Off", ECValue(1), nullptr},
                    {"Unknown", ECValue(2), nullptr},
                    {"Halfhalf", ECValue(3), nullptr}});

                ECPropertyCP statusesProp = cl->GetPropertyP("Statuses");
                ASSERT_TRUE(statusesProp != nullptr && statusesProp->GetIsPrimitiveArray()) << className << " | " << testDb.GetDescription();
                ECEnumerationCP statusesEnum = statusesProp->GetAsPrimitiveArrayProperty()->GetEnumeration();
                ASSERT_TRUE(statusesEnum != nullptr) << className << " | " << testDb.GetDescription();
                testDb.AssertEnum(*statusesEnum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                    {{"On", ECValue(0), nullptr},
                    {"Off", ECValue(1), nullptr},
                    {"Unknown", ECValue(2), nullptr},
                    {"Halfhalf", ECValue(3), nullptr}});
                }

            testDb.AssertEnum("TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                {{"On", ECValue(0), nullptr},
                {"Off", ECValue(1), nullptr},
                {"Unknown", ECValue(2), nullptr},
                {"Halfhalf", ECValue(3), nullptr}});
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC31ToEC32SchemaUpgrade_Koqs)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC31KOQS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //Schema changes:
            //- bumped up version to 1.0.1
            //- KOQ AREA: 
            //    - Added presentation unit f:DefaultRealU(4)[u:SQ_CM]
            //    - Changed relativeError to 0.001
            // - Added subclasses SubA, SubB, SubC
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <ECSchemaReference name="Formats" version="01.00.00" alias="f" />

                    <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="u:SQ_M" presentationUnits="f:DefaultRealU(4)[u:SQ_M];f:DefaultRealU(4)[u:SQ_FT];f:DefaultRealU(4)[u:SQ_CM]" relativeError="0.001"/>
                    <ECEntityClass typeName="BaseA">
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubA">
                        <BaseClass>BaseA</BaseClass>
                        <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BaseB">
                        <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubB">
                        <BaseClass>BaseB</BaseClass>
                        <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BaseC">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>6</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubC">
                        <BaseClass>BaseC</BaseClass>
                        <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const BentleyStatus schemaImportStat = testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetAge() || !testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                {
                EXPECT_EQ(ERROR, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SUCCESS, schemaImportStat) << testDb.GetDescription();
            EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();

            for (Utf8CP className : {"SubA", "SubB", "SubC"})
                {
                ECInstanceKey key;
                ECSqlStatement stmt;
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), Utf8PrintfString("INSERT INTO ts.%s(Code,Size,Sizes) VALUES(?,?,?)", className).c_str())) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 1)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(2, 2.2)) << className << " | " << testDb.GetDescription();

                IECSqlBinder& sizesBinder = stmt.GetBinder(3);
                ASSERT_EQ(ECSqlStatus::Success, sizesBinder.AddArrayElement().BindDouble(3.3)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, sizesBinder.AddArrayElement().BindDouble(33.3)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, sizesBinder.AddArrayElement().BindDouble(333.3)) << className << " | " << testDb.GetDescription();

                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << className << " | " << testDb.GetDescription();
                stmt.Finalize();
                EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id": "%s", "Code": 1, "Size": 2.2, "Sizes" : [3.3,33.3,333.3]}])json", key.GetInstanceId().ToHexStr().c_str())),
                            testDb.ExecuteECSqlSelect(Utf8PrintfString("SELECT ECInstanceId,Code,Size,Sizes FROM ts.%s", className).c_str())) << className << " | " << testDb.GetDescription();

                ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", className);
                ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << className << " | " << testDb.GetDescription();

                ECPropertyCP sizeProp = cl->GetPropertyP("Size");
                ASSERT_TRUE(sizeProp != nullptr && sizeProp->GetIsPrimitive()) << className << " | " << testDb.GetDescription();
                KindOfQuantityCP koq = sizeProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
                ASSERT_TRUE(koq != nullptr) << className << " | " << testDb.GetDescription();
                testDb.AssertKindOfQuantity(*koq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);

                ECPropertyCP sizesProp = cl->GetPropertyP("Sizes");
                ASSERT_TRUE(sizesProp != nullptr && sizesProp->GetIsPrimitiveArray()) << className << " | " << testDb.GetDescription();
                KindOfQuantityCP sizesKoq = sizesProp->GetAsPrimitiveArrayProperty()->GetKindOfQuantity();
                ASSERT_TRUE(sizesKoq != nullptr) << className << " | " << testDb.GetDescription();
                testDb.AssertKindOfQuantity(*sizesKoq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);
                }

            testDb.AssertKindOfQuantity("TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32SchemaUpgrade_Enums)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32ENUMS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //Schema changes:
            //- bumped up version to 1.0.1
            // - Enum StatusEnum
            //    - Changed display label to "Status"
            //    - Added enumerator 3
            // - Added subclasses SubA, SubB, SubC
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />

                    <ECEnumeration typeName="StatusEnum" displayLabel="Status" backingTypeName="int" isStrict="true">
                        <ECEnumerator name="On" value="0"/>
                        <ECEnumerator name="Off" value="1"/>
                        <ECEnumerator name="Unknown" value="2"/>
                        <ECEnumerator name="Halfhalf" value="3"/>
                    </ECEnumeration>
                    <ECEntityClass typeName="BaseA">
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Status" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubA">
                        <BaseClass>BaseA</BaseClass>
                        <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BaseB">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Status" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubB">
                        <BaseClass>BaseB</BaseClass>
                        <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BaseC">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>6</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Status" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubC">
                        <BaseClass>BaseC</BaseClass>
                        <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
                    </ECEntityClass>
                </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const BentleyStatus schemaImportStat = testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetAge() || !testDb.SupportsFeature(ECDbFeature::NamedEnumerators))
                {
                EXPECT_EQ(ERROR, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SUCCESS, schemaImportStat) << testDb.GetDescription();
            EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();

            for (Utf8CP className : {"SubA", "SubB", "SubC"})
                {
                ECSqlStatement stmt;
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), Utf8PrintfString("INSERT INTO ts.%s(Code,Status,Statuses) VALUES(?,?,?)", className).c_str())) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 1)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, 3)) << className << " | " << testDb.GetDescription();

                IECSqlBinder& statusBinder = stmt.GetBinder(3);
                ASSERT_EQ(ECSqlStatus::Success, statusBinder.AddArrayElement().BindInt(0)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, statusBinder.AddArrayElement().BindInt(1)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, statusBinder.AddArrayElement().BindInt(2)) << className << " | " << testDb.GetDescription();

                ECInstanceKey key;
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << className << " | " << testDb.GetDescription();
                EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id": "%s", "Code": 1, "Status": 3, "Statuses": [0,1,2]}])json", key.GetInstanceId().ToHexStr().c_str())),
                            testDb.ExecuteECSqlSelect(Utf8PrintfString("SELECT ECInstanceId,Code,Status,Statuses FROM ts.%s", className).c_str())) << className << " | " << testDb.GetDescription();

                ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", className);
                ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << className << " | " << testDb.GetDescription();

                ECPropertyCP statusProp = cl->GetPropertyP("Status");
                ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << className << " | " << testDb.GetDescription();
                ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
                ASSERT_TRUE(ecenum != nullptr) << className << " | " << testDb.GetDescription();
                testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                    {{"On", ECValue(0), nullptr},
                    {"Off", ECValue(1), nullptr},
                    {"Unknown", ECValue(2), nullptr},
                    {"Halfhalf", ECValue(3), nullptr}});

                ECPropertyCP statusesProp = cl->GetPropertyP("Statuses");
                ASSERT_TRUE(statusesProp != nullptr && statusesProp->GetIsPrimitiveArray()) << className << " | " << testDb.GetDescription();
                ECEnumerationCP statusesEnum = statusesProp->GetAsPrimitiveArrayProperty()->GetEnumeration();
                ASSERT_TRUE(statusesEnum != nullptr) << className << " | " << testDb.GetDescription();
                testDb.AssertEnum(*statusesEnum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                    {{"On", ECValue(0), nullptr},
                    {"Off", ECValue(1), nullptr},
                    {"Unknown", ECValue(2), nullptr},
                    {"Halfhalf", ECValue(3), nullptr}});
                }

            testDb.AssertEnum("TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                {{"On", ECValue(0), nullptr},
                {"Off", ECValue(1), nullptr},
                {"Unknown", ECValue(2), nullptr},
                {"Halfhalf", ECValue(3), nullptr}});
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32SchemaUpgrade_Koqs)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32KOQS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //Schema changes:
            //- bumped up version to 1.0.1
            //- KOQ AREA: 
            //    - Added presentation unit f:DefaultRealU(4)[u:SQ_CM]
            //    - Changed relativeError to 0.001
            // - Added subclasses SubA, SubB, SubC
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <ECSchemaReference name="Formats" version="01.00.00" alias="f" />

                    <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="u:SQ_M" presentationUnits="f:DefaultRealU(4)[u:SQ_M];f:DefaultRealU(4)[u:SQ_FT];f:DefaultRealU(4)[u:SQ_CM]" relativeError="0.001"/>
                    <ECEntityClass typeName="BaseA">
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubA">
                        <BaseClass>BaseA</BaseClass>
                        <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BaseB">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubB">
                        <BaseClass>BaseB</BaseClass>
                        <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BaseC">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>6</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubC">
                        <BaseClass>BaseC</BaseClass>
                        <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const BentleyStatus schemaImportStat = testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetAge() || !testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                {
                EXPECT_EQ(ERROR, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SUCCESS, schemaImportStat) << testDb.GetDescription();
            EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();

            for (Utf8CP className : {"SubA", "SubB", "SubC"})
                {
                ECInstanceKey key;
                ECSqlStatement stmt;
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), Utf8PrintfString("INSERT INTO ts.%s(Code,Size,Sizes) VALUES(?,?,?)", className).c_str())) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 1)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(2, 2.2)) << className << " | " << testDb.GetDescription();

                IECSqlBinder& sizesBinder = stmt.GetBinder(3);
                ASSERT_EQ(ECSqlStatus::Success, sizesBinder.AddArrayElement().BindDouble(3.3)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, sizesBinder.AddArrayElement().BindDouble(33.3)) << className << " | " << testDb.GetDescription();
                ASSERT_EQ(ECSqlStatus::Success, sizesBinder.AddArrayElement().BindDouble(333.3)) << className << " | " << testDb.GetDescription();

                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << className << " | " << testDb.GetDescription();
                stmt.Finalize();
                EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id": "%s", "Code": 1, "Size": 2.2, "Sizes" : [3.3,33.3,333.3]}]])json", key.GetInstanceId().ToHexStr().c_str())),
                            testDb.ExecuteECSqlSelect(Utf8PrintfString("SELECT ECInstanceId,Code,Size,Sizes FROM ts.%s", className).c_str())) << className << " | " << testDb.GetDescription();

                ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", className);
                ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << className << " | " << testDb.GetDescription();

                ECPropertyCP sizeProp = cl->GetPropertyP("Size");
                ASSERT_TRUE(sizeProp != nullptr && sizeProp->GetIsPrimitive()) << className << " | " << testDb.GetDescription();
                KindOfQuantityCP koq = sizeProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
                ASSERT_TRUE(koq != nullptr) << className << " | " << testDb.GetDescription();
                testDb.AssertKindOfQuantity(*koq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);

                ECPropertyCP sizesProp = cl->GetPropertyP("Sizes");
                ASSERT_TRUE(sizesProp != nullptr && sizesProp->GetIsPrimitiveArray()) << className << " | " << testDb.GetDescription();
                KindOfQuantityCP sizesKoq = sizesProp->GetAsPrimitiveArrayProperty()->GetKindOfQuantity();
                ASSERT_TRUE(sizesKoq != nullptr) << className << " | " << testDb.GetDescription();
                testDb.AssertKindOfQuantity(*sizesKoq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);
                }

            testDb.AssertKindOfQuantity("TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);
            }
        }
    }
