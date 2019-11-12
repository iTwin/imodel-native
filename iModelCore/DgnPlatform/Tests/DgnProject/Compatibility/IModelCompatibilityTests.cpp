/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <DgnPlatform/DgnCoreAPI.h>
#include <DgnPlatform/FunctionalDomain.h>
#include "Profiles.h"
#include "TestIModelCreators.h"
#include "TestDb.h"
#include "TestDomain.h"

USING_NAMESPACE_BENTLEY_EC

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct IModelCompatibilityTestFixture : CompatibilityTestFixture
    {
    protected:
        ScopedDgnHost m_host;
        void SetUp() override 
            {
            CompatibilityTestFixture::SetUp();
            ASSERT_EQ(SUCCESS, TestIModelCreation::Run());
            }
    };

//---------------------------------------------------------------------------------------
// Runs basic tests on all available test files. This is a basic test to cover tests and test files
// which are added in the future, and to which existing test runners cannot be adjusted to.
// @bsimethod                                  Krischan.Eberle                      07/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, BasicTestsOnAllPulledFiles)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfAllPulledTestFiles())
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
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
void Assert_BuiltinSchemaVersions_2_0_0_0 (TestIModel& testDb)
    {
    EXPECT_EQ(8, testDb.GetSchemaCount()) << testDb.GetDescription();
    //iModel built-in schema versions
    if (testDb.GetSchemaUpgradeOptions().AreDomainUpgradesAllowed())
        {
        EXPECT_LE(SchemaVersion(1, 0, 4), testDb.GetSchemaVersion("BisCore")) << testDb.GetDescription();
        EXPECT_LE(SchemaVersion(1, 0, 1), testDb.GetSchemaVersion("Generic")) << testDb.GetDescription();
        }

    EXPECT_LE(BeVersion(), testDb.GetOriginalECXmlVersion("BisCore")) << testDb.GetDescription();
    EXPECT_LE(BeVersion(), testDb.GetOriginalECXmlVersion("Generic")) << testDb.GetDescription();

    //ECDb built-in schema versions
    EXPECT_EQ(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), testDb.GetSchemaItemCounts("ECDbFileInfo")) << testDb.GetDescription();

    EXPECT_LE(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
    EXPECT_GE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbMap")) << testDb.GetDescription();
    EXPECT_LE(9, testDb.GetSchemaItemCounts("ECDbMap").Value()["classcount"].asInt()) << testDb.GetDescription();

    EXPECT_EQ(SchemaVersion(4, 0, 0), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbMeta")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":24, "enumcount": 8})js"), testDb.GetSchemaItemCounts("ECDbMeta")) << testDb.GetDescription();

    EXPECT_EQ(SchemaVersion(5, 0, 0), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbSystem")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), testDb.GetSchemaItemCounts("ECDbSystem")) << testDb.GetDescription();

    EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("ECDbSchemaPolicies")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbSchemaPolicies")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":3})js"), testDb.GetSchemaItemCounts("ECDbSchemaPolicies")) << testDb.GetDescription();

    //Standard schema versions (can get upgraded through domain schema upgrades, so we cannot test for a specific version)
    EXPECT_LE(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Eimantas.Morkunas                    07/19
//+---------------+---------------+---------------+---------------+---------------+------
void Assert_BuiltinSchemaVersions_2_0_0_1(TestIModel& testDb)
    {
    EXPECT_EQ(8, testDb.GetSchemaCount()) << testDb.GetDescription();
    //iModel built-in schema versions
    // Note: don't assert on original ecxml version for schemas that don't get upgraded automatically. That is to error-prone to test
    if (testDb.GetSchemaUpgradeOptions().AreDomainUpgradesAllowed())
        {
        EXPECT_LE(SchemaVersion(1, 0, 4), testDb.GetSchemaVersion("BisCore")) << testDb.GetDescription();
        EXPECT_LE(SchemaVersion(1, 0, 1), testDb.GetSchemaVersion("Generic")) << testDb.GetDescription();
        }

    //ECDb built-in schema versions
    EXPECT_EQ(SchemaVersion(2, 0, 1), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), testDb.GetSchemaItemCounts("ECDbFileInfo")) << testDb.GetDescription();

    EXPECT_LE(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbMap")) << testDb.GetDescription();
    EXPECT_LE(9, testDb.GetSchemaItemCounts("ECDbMap").Value()["classcount"].asInt()) << testDb.GetDescription();

    EXPECT_EQ(SchemaVersion(4, 0, 1), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbMeta")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":38, "enumcount": 8})js"), testDb.GetSchemaItemCounts("ECDbMeta")) << testDb.GetDescription();

    EXPECT_EQ(SchemaVersion(5, 0, 1), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbSystem")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), testDb.GetSchemaItemCounts("ECDbSystem")) << testDb.GetDescription();

    EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("ECDbSchemaPolicies")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":3})js"), testDb.GetSchemaItemCounts("ECDbSchemaPolicies")) << testDb.GetDescription();

    //Standard schema versions (can get upgraded without a profile change)
    EXPECT_LE(SchemaVersion(1, 0, 1), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("CoreCustomAttributes")) << testDb.GetDescription();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                      07/19
//+---------------+---------------+---------------+---------------+---------------+------
void Assert_BuiltinSchemaVersions_2_0_0_4(TestIModel& testDb)
    {
    EXPECT_EQ(8, testDb.GetSchemaCount()) << testDb.GetDescription();
    //iModel built-in schema versions
    // Note: don't assert on original ecxml version for schemas that don't get upgraded automatically. That is to error-prone to test
    if (testDb.GetSchemaUpgradeOptions().AreDomainUpgradesAllowed())
        {
        EXPECT_LE(SchemaVersion(1, 0, 4), testDb.GetSchemaVersion("BisCore")) << testDb.GetDescription();
        EXPECT_LE(SchemaVersion(1, 0, 1), testDb.GetSchemaVersion("Generic")) << testDb.GetDescription();
        }

    //ECDb built-in schema versions
    EXPECT_EQ(SchemaVersion(2, 0, 1), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), testDb.GetSchemaItemCounts("ECDbFileInfo")) << testDb.GetDescription();
    
    EXPECT_LE(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbMap")) << testDb.GetDescription();
    EXPECT_LE(9, testDb.GetSchemaItemCounts("ECDbMap").Value()["classcount"].asInt()) << testDb.GetDescription();
    
    EXPECT_EQ(SchemaVersion(4, 0, 1), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbMeta")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":38, "enumcount": 8})js"), testDb.GetSchemaItemCounts("ECDbMeta")) << testDb.GetDescription();
    
    EXPECT_EQ(SchemaVersion(5, 0, 1), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
    EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbSystem")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), testDb.GetSchemaItemCounts("ECDbSystem")) << testDb.GetDescription();
    
    EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("ECDbSchemaPolicies")) << testDb.GetDescription();
    EXPECT_EQ(JsonValue(R"js({"classcount":3})js"), testDb.GetSchemaItemCounts("ECDbSchemaPolicies")) << testDb.GetDescription();

    //Standard schema versions (can get upgraded without a profile change)
    EXPECT_LE(SchemaVersion(1, 0, 3), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("CoreCustomAttributes")) << testDb.GetDescription();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                      07/19
//+---------------+---------------+---------------+---------------+---------------+------
void Assert_BuiltinSchemaVersions_2_X_X_X(TestIModel& testDb)
    {
    EXPECT_LE(8, testDb.GetSchemaCount()) << testDb.GetDescription();

    //iModel built-in schema versions
    // Note: don't assert on original ecxml version for schemas that don't get upgraded automatically. That is to error-prone to test
    EXPECT_LE(SchemaVersion(1, 0, 4), testDb.GetSchemaVersion("BisCore")) << testDb.GetDescription();
    EXPECT_LE(SchemaVersion(1, 0, 1), testDb.GetSchemaVersion("Generic")) << testDb.GetDescription();

    //ECDb built-in schema versions
    EXPECT_LE(SchemaVersion(2, 0, 1), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
    EXPECT_LE(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << testDb.GetDescription();

    EXPECT_LE(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbMap")) << testDb.GetDescription();

    EXPECT_LE(SchemaVersion(4, 0, 1), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
    EXPECT_LE(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbMeta")) << testDb.GetDescription();

    EXPECT_LE(SchemaVersion(5, 0, 1), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
    EXPECT_LE(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbSystem")) << testDb.GetDescription();

    EXPECT_LE(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("ECDbSchemaPolicies")) << testDb.GetDescription();

    //Standard schema versions
    EXPECT_LE(SchemaVersion(1, 0, 3), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("CoreCustomAttributes")) << testDb.GetDescription();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, BuiltinSchemaVersions)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            switch (testDb.GetAge())
               {
                case ProfileState::Age::Older:
                    {
                    if (testDb.GetDgnDbProfileVersion() == ProfileVersion(2, 0, 0, 0))
                        Assert_BuiltinSchemaVersions_2_0_0_0(testDb);
                    else if (testDb.GetDgnDbProfileVersion() == ProfileVersion(2, 0, 0, 1) ||
                            testDb.GetDgnDbProfileVersion() == ProfileVersion(2, 0, 0, 2) ||
                            testDb.GetDgnDbProfileVersion() == ProfileVersion(2, 0, 0, 3))
                        Assert_BuiltinSchemaVersions_2_0_0_1(testDb);
                    else 
                        FAIL() << "*ERROR* case not handled | " << testDb.GetDescription();
                    break;
                    }
                case ProfileState::Age::UpToDate:
                    {
                    if (testDb.GetDgnDbProfileVersion() == ProfileVersion(2, 0, 0, 4))
                        Assert_BuiltinSchemaVersions_2_0_0_4(testDb);
                    else 
                        FAIL() << "*ERROR* case not handled | " << testDb.GetDescription();
                    break;
                    }

                case ProfileState::Age::Newer:
                    {
                    Assert_BuiltinSchemaVersions_2_X_X_X(testDb);
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
TEST_F(IModelCompatibilityTestFixture, ECSqlColumnInfoForAliases)
    {
    auto assertColInfo = [] (TestDb const& testDb, ECSqlColumnInfo const& colInfo, bool hasAlias)
        {
        ASSERT_TRUE(colInfo.GetProperty() != nullptr) << testDb.GetDescription();
        Utf8StringCR selectClauseItem = colInfo.GetProperty()->GetDisplayLabel();

        EXPECT_EQ(hasAlias, colInfo.IsGeneratedProperty()) << selectClauseItem << " | " << testDb.GetDescription();
        EXPECT_EQ(!hasAlias, colInfo.IsSystemProperty()) << selectClauseItem << " | " << testDb.GetDescription();
        EXPECT_EQ(PRIMITIVETYPE_Long, colInfo.GetDataType().GetPrimitiveType()) << selectClauseItem << " | " << testDb.GetDescription();

        ASSERT_TRUE(colInfo.GetProperty()->GetIsPrimitive()) << selectClauseItem << " | " << testDb.GetDescription();
        // Since ECDb 4.0.0.2 the Id system properties in the ECDbSystem schema have the extended type "Id"
        // However, when using aliases, the generated prop gets that extended type regardless of the profile version.
        // The test asserts that ruleset.
        if (hasAlias || testDb.SupportsFeature(ECDbFeature::SystemPropertiesHaveIdExtendedType))
            EXPECT_STREQ("Id", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << selectClauseItem << " | " << testDb.GetDescription();
        else
            EXPECT_FALSE(colInfo.GetProperty()->HasExtendedType()) << selectClauseItem << " | " << testDb.GetDescription();
        };

    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
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
TEST_F(IModelCompatibilityTestFixture, EC31Enums)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31ENUMS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            if (!testDb.SupportsFeature(ECDbFeature::NamedEnumerators))
                {
                testDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                    {{"Unspecified", ECValue("Unspecified"), nullptr},
                    {"Utc", ECValue("Utc"), nullptr},
                    {"Local", ECValue("Local"), nullptr} });

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
TEST_F(IModelCompatibilityTestFixture, UpgradingEC31EnumsToEC32AfterProfileUpgrade)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32ENUMS_PROFILEUPGRADED))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
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
TEST_F(IModelCompatibilityTestFixture, EC32Enums)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32ENUMS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
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
TEST_F(IModelCompatibilityTestFixture, EC31KindOfQuantities)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31KOQS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
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
TEST_F(IModelCompatibilityTestFixture, EC31ThreadPitchKindOfQuantities)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31THREADPITCHKOQS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
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
TEST_F(IModelCompatibilityTestFixture, SchemaManager_EC31KindOfQuantities)
    {
    Utf8CP koq1Name = "TestKoq_M_Mfi8";
    Utf8CP koq2Name = "TestKoq_SQFTfi8_SQFTreal4u";

    auto assertReferencedUnitsAndFormatsSchema = [] (TestIModel const& testDb, ECSchemaCR koqSchema)
        {
        auto it = koqSchema.GetReferencedSchemas().Find(SchemaKey("Units", 1, 0), ECN::SchemaMatchType::LatestReadCompatible);
        ASSERT_FALSE(it == koqSchema.GetReferencedSchemas().end()) << testDb.GetDescription();
        ASSERT_EQ(testDb.SupportsFeature(ECDbFeature::UnitsAndFormats), it->second->HasId()) << testDb.GetDescription();

        it = koqSchema.GetReferencedSchemas().Find(SchemaKey("Formats", 1, 0), ECN::SchemaMatchType::LatestReadCompatible);
        ASSERT_FALSE(it == koqSchema.GetReferencedSchemas().end()) << testDb.GetDescription();
        ASSERT_EQ(testDb.SupportsFeature(ECDbFeature::UnitsAndFormats), it->second->HasId()) << testDb.GetDescription();
        };

    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31KOQS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
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
            ASSERT_EQ(11, schemas.size()) << testDb.GetDescription();
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
TEST_F(IModelCompatibilityTestFixture, EC32KindOfQuantities)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32KOQS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
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
TEST_F(IModelCompatibilityTestFixture, EC31Units)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            //this test only tests files which do not support EC32 units yet
            if (testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                continue;

            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            testDb.GetDb().ClearECDbCache();

            EXPECT_EQ(8, testDb.GetDb().Schemas().GetSchemas(false).size()) << testDb.GetDescription();

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
            ASSERT_EQ(SchemaStatus::Success, testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas())) << testDb.GetDescription();

            //3 more schemas: the imported test schema and the in-memory units/formats schemas
            EXPECT_EQ(11, testDb.GetDb().Schemas().GetSchemas(false).size()) << testDb.GetDescription();

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

    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31KOQS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            //this test only tests files which do not support EC32 units yet
            if (testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                continue;

            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            testDb.GetDb().ClearECDbCache();

            EXPECT_EQ(11, testDb.GetDb().Schemas().GetSchemas(false).size()) << testDb.GetDescription();

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
            ASSERT_EQ(SchemaStatus::Success, testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas())) << testDb.GetDescription();

            EXPECT_EQ(12, testDb.GetDb().Schemas().GetSchemas(false).size()) << testDb.GetDescription();

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
TEST_F(IModelCompatibilityTestFixture, EC32Units)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32UNITS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
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
TEST_F(IModelCompatibilityTestFixture, EC31SchemaImport)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>

                        <ECEntityClass typeName="MyDomainClass">
                                <BaseClass>bis:PhysicalElement</BaseClass>
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
                     </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetECDbAge())
                {
                EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SchemaStatus::Success, schemaImportStat) << testDb.GetDescription();

            if (testDb.SupportsFeature(ECDbFeature::PersistedECVersions))
                EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();
            else
                EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();

            if (testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                {
                EXPECT_EQ(JsonValue("[{\"cnt\": 2}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef WHERE Name IN ('Units','Formats')")) << "When importing into 4.0.0.2 or newer file, units and formats schema must be persisted. | " << testDb.GetDescription();
                EXPECT_EQ(JsonValue("[{\"cnt\": 3}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef s JOIN meta.SchemaHasSchemaReferences ref ON s.ECInstanceId=ref.SourceECInstanceId WHERE s.Name='TestSchema'")) << "When importing into 4.0.0.2 or newer file, units and formats schema must be persisted. | " << testDb.GetDescription();
                }
            else
                {
                EXPECT_EQ(JsonValue("[{\"cnt\": 0}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef WHERE Name IN ('Units','Formats')")) << "When importing into 4.0.0.1 file, units and formats schema must not be persisted. | " << testDb.GetDescription();
                EXPECT_EQ(JsonValue("[{\"cnt\": 1}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef s JOIN meta.SchemaHasSchemaReferences ref ON s.ECInstanceId=ref.SourceECInstanceId WHERE s.Name='TestSchema'")) << "When importing into 4.0.0.1 file, units and formats schema must not be persisted. | " << testDb.GetDescription();
                }

            EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId, Size, Status FROM ts.MyDomainClass")) << testDb.GetDescription();

            ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", "MyDomainClass");
            ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << testDb.GetDescription();

            ECPropertyCP sizeProp = cl->GetPropertyP("Size");
            ASSERT_TRUE(sizeProp != nullptr && sizeProp->GetIsPrimitive()) << testDb.GetDescription();
            KindOfQuantityCP koq = sizeProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
            ASSERT_TRUE(koq != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]"])json"), 0.0001);

            ECPropertyCP statusProp = cl->GetPropertyP("Status");
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
TEST_F(IModelCompatibilityTestFixture, EC32SchemaImport_Enums)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>

                        <ECEntityClass typeName="MyDomainClass">
                                <BaseClass>bis:PhysicalElement</BaseClass>
                                <ECProperty propertyName="Status" typeName="StatusEnum" />
                        </ECEntityClass>
                        <ECEnumeration typeName="StatusEnum" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                            <ECEnumerator name="On" value="0"/>
                            <ECEnumerator name="Off" value="1"/>
                            <ECEnumerator name="Unknown" value="2"/>
                        </ECEnumeration>
                     </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetECDbAge() || !testDb.SupportsFeature(ECDbFeature::NamedEnumerators))
                {
                EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId, Status FROM ts.MyDomainClass")) << testDb.GetDescription();

            ECClassCP myDomainClass = testDb.GetDb().Schemas().GetClass("TestSchema", "MyDomainClass");
            ASSERT_TRUE(myDomainClass != nullptr && myDomainClass->IsEntityClass()) << testDb.GetDescription();

            ECPropertyCP statusProp = myDomainClass->GetPropertyP("Status");
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
TEST_F(IModelCompatibilityTestFixture, EC32SchemaImport_Koqs)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
                        <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                        <ECSchemaReference name="Formats" version="01.00.00" alias="f" />

                        <ECEntityClass typeName="MyDomainClass">
                            <BaseClass>bis:PhysicalElement</BaseClass>
                            <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                        </ECEntityClass>
                        <KindOfQuantity typeName="ANGLE" displayLabel="Angle" persistenceUnit="u:RAD" presentationUnits="f:DefaultRealU(2)[u:ARC_DEG]" relativeError="0.0001"/>
                        <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="u:SQ_M" presentationUnits="f:DefaultRealU(4)[u:SQ_M];f:DefaultRealU(4)[u:SQ_FT]" relativeError="0.0001"/>
                        <KindOfQuantity typeName="TEMPERATURE" displayLabel="Temperature" persistenceUnit="u:K" presentationUnits="f:DefaultRealU(4)[u:CELSIUS];f:DefaultRealU(4)[u:FAHRENHEIT];f:DefaultRealU(4)[u:K]" relativeError="0.01"/>
                     </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetECDbAge() || !testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                {
                EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SchemaStatus::Success, schemaImportStat) << testDb.GetDescription();

            EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId, Size FROM ts.MyDomainClass")) << testDb.GetDescription();

            ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", "MyDomainClass");
            ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << testDb.GetDescription();

            ECPropertyCP sizeProp = cl->GetPropertyP("Size");
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
TEST_F(IModelCompatibilityTestFixture, EC31SchemaImport_Formats_API)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            ECSchemaPtr schema;
            ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0)) << testDb.GetDescription();
            schema->SetOriginalECXmlVersion(3, 1);

            Formatting::NumericFormatSpec spec;
            spec.SetPresentationType(Formatting::PresentationType::Decimal);
            spec.SetPrecision(Formatting::DecimalPrecision::Precision6);
            spec.SetFormatTraits((Formatting::FormatTraits) ((int) Formatting::FormatTraits::KeepSingleZero | (int) Formatting::FormatTraits::KeepDecimalPoint));

            ECFormatP format = nullptr;
            ASSERT_EQ(ECObjectsStatus::Success, schema->CreateFormat(format, "DefaultReal", "real", nullptr, &spec)) << testDb.GetDescription();

            ASSERT_EQ(SchemaStatus::SchemaImportFailed, testDb.GetDgnDb().ImportSchemas({schema.get()}));
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      10/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC31SchemaUpgrade_Formats_API)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            // Import base line of a schema, which is then upgraded in the next step
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                   <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                      <ECStructClass typeName="Foo">
                            <ECProperty propertyName="Size" typeName="double" />
                       </ECStructClass>
                    </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());
            if (testDb.GetECDbAge() == ProfileState::Age::Newer)
                {
                EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            ASSERT_EQ(SchemaStatus::Success, schemaImportStat) << testDb.GetDescription();
            ASSERT_EQ(BE_SQLITE_OK, testDb.GetDb().SaveChanges()) << testDb.GetDescription();
            testDb.GetDb().CloseDb();
            ASSERT_EQ(BE_SQLITE_OK, testDb.GetDb().OpenBeSQLiteDb(testDb.GetTestFile().GetPath(), testDb.GetOpenParams())) << testDb.GetDescription();

            // now build new version with API and add a format -> this should fail as ECDb does not support
            // schemas that are 3.1 but have EC3.2 features
            ECSchemaPtr schema;
            ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 1)) << testDb.GetDescription();
            schema->SetOriginalECXmlVersion(3, 1);

            ECStructClassP fooClass = nullptr;
            ASSERT_EQ(ECObjectsStatus::Success, schema->CreateStructClass(fooClass, "Foo")) << testDb.GetDescription();
            PrimitiveECPropertyP prop = nullptr;
            ASSERT_EQ(ECObjectsStatus::Success, fooClass->CreatePrimitiveProperty(prop, "Code", PRIMITIVETYPE_Integer)) << testDb.GetDescription();

            Formatting::NumericFormatSpec spec;
            spec.SetPresentationType(Formatting::PresentationType::Decimal);
            spec.SetPrecision(Formatting::DecimalPrecision::Precision6);
            spec.SetFormatTraits((Formatting::FormatTraits) ((int) Formatting::FormatTraits::KeepSingleZero | (int) Formatting::FormatTraits::KeepDecimalPoint));

            ECFormatP format = nullptr;
            ASSERT_EQ(ECObjectsStatus::Success, schema->CreateFormat(format, "DefaultReal", "real", nullptr, &spec)) << testDb.GetDescription();

            ASSERT_EQ(SchemaStatus::SchemaImportFailed, testDb.GetDgnDb().ImportSchemas({schema.get()})) << testDb.GetDescription();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC31Enum_SchemaUpgrade)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31ENUMS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
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
            // - Added subclass SubDomainClass
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>

                <ECEnumeration typeName="StatusEnum" displayLabel="Status" backingTypeName="int" isStrict="true">
                    <ECEnumerator value="0"/>
                    <ECEnumerator value="1"/>
                    <ECEnumerator value="2"/>
                    <ECEnumerator value="3"/>
                </ECEnumeration>
                <ECEntityClass typeName="MyDomainClass">
                    <BaseClass>bis:PhysicalElement</BaseClass>
                    <ECProperty propertyName="Status" typeName="StatusEnum" />
                </ECEntityClass>
                <ECEntityClass typeName="SubDomainClass">
                    <BaseClass>MyDomainClass</BaseClass>
                    <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
                </ECEntityClass>
                </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetECDbAge())
                {
                EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SchemaStatus::Success, schemaImportStat) << testDb.GetDescription();

            if (testDb.SupportsFeature(ECDbFeature::PersistedECVersions))
                EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();
            else
                EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();

            EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId,Status,Statuses FROM ts.SubDomainClass")) << testDb.GetDescription();

            ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", "SubDomainClass");
            ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << "SubDomainClass | " << testDb.GetDescription();

            ECPropertyCP statusProp = cl->GetPropertyP("Status");
            ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << "SubDomainClass | " << testDb.GetDescription();
            ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
            ASSERT_TRUE(ecenum != nullptr) << "SubDomainClass | " << testDb.GetDescription();
            testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                {{"StatusEnum0", ECValue(0), nullptr},
                {"StatusEnum1", ECValue(1), nullptr},
                {"StatusEnum2", ECValue(2), nullptr},
                {"StatusEnum3", ECValue(3), nullptr}});

            ECPropertyCP statusesProp = cl->GetPropertyP("Statuses");
            ASSERT_TRUE(statusesProp != nullptr && statusesProp->GetIsPrimitiveArray()) << "SubDomainClass | " << testDb.GetDescription();
            ECEnumerationCP statusEnum = statusesProp->GetAsPrimitiveArrayProperty()->GetEnumeration();
            ASSERT_TRUE(statusEnum != nullptr) << "SubDomainClass | " << testDb.GetDescription();
            testDb.AssertEnum(*statusEnum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                {{"StatusEnum0", ECValue(0), nullptr},
                {"StatusEnum1", ECValue(1), nullptr},
                {"StatusEnum2", ECValue(2), nullptr},
                {"StatusEnum3", ECValue(3), nullptr}});

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
TEST_F(IModelCompatibilityTestFixture, EC31Koqs_SchemaUpgrade)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31KOQS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //Schema changes:
            //- bumped up version to 1.0.1
            //- KOQ AREA: 
            //    - Added presentation unit SQ.CM(real4u)
            //    - Changed relativeError to 0.001
            // - Added subclass SubDomainClass
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>

                    <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="SQ.M(DefaultReal)" presentationUnits="SQ.M(real4u);SQ.FT(real4u);SQ.CM(real4u)" relativeError="0.001"/>
                    <ECEntityClass typeName="MyDomainClass">
                        <BaseClass>bis:PhysicalElement</BaseClass>
                        <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubDomainClass">
                        <BaseClass>MyDomainClass</BaseClass>
                        <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetECDbAge())
                {
                EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            if (testDb.SupportsFeature(ECDbFeature::PersistedECVersions))
                EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();
            else
                EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();


            if (testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                {
                EXPECT_EQ(JsonValue("[{\"cnt\": 2}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef WHERE Name IN ('Units','Formats')")) << "When importing into 4.0.0.2 or newer file, units and formats schema must be persisted. | " << testDb.GetDescription();
                EXPECT_EQ(JsonValue("[{\"cnt\": 3}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef s JOIN meta.SchemaHasSchemaReferences ref ON s.ECInstanceId=ref.SourceECInstanceId WHERE s.Name='TestSchema'")) << "When importing into 4.0.0.2 or newer file, units and formats schema must be persisted. | " << testDb.GetDescription();
                }
            else
                {
                EXPECT_EQ(JsonValue("[{\"cnt\": 0}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef WHERE Name IN ('Units','Formats')")) << "When importing into 4.0.0.1 file, units and formats schema must not be persisted. | " << testDb.GetDescription();
                EXPECT_EQ(JsonValue("[{\"cnt\": 1}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef s JOIN meta.SchemaHasSchemaReferences ref ON s.ECInstanceId=ref.SourceECInstanceId WHERE s.Name='TestSchema'")) << "When importing into 4.0.0.1 file, units and formats schema must not be persisted. | " << testDb.GetDescription();
                }

            EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId,Size,Sizes FROM ts.SubDomainClass")) << testDb.GetDescription();

            ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", "SubDomainClass");
            ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << "SubDomainClass | " << testDb.GetDescription();

            ECPropertyCP sizeProp = cl->GetPropertyP("Size");
            ASSERT_TRUE(sizeProp != nullptr && sizeProp->GetIsPrimitive()) << testDb.GetDescription();
            KindOfQuantityCP koq = sizeProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
            ASSERT_TRUE(koq != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);

            ECPropertyCP sizesProp = cl->GetPropertyP("Sizes");
            ASSERT_TRUE(sizesProp != nullptr && sizesProp->GetIsPrimitiveArray()) << testDb.GetDescription();
            KindOfQuantityCP sizesKoq = sizesProp->GetAsPrimitiveArrayProperty()->GetKindOfQuantity();
            ASSERT_TRUE(sizesKoq != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*sizesKoq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);

            testDb.AssertKindOfQuantity("TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC31ToEC32SchemaUpgrade_Enums)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31ENUMS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
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
            // - Added subclass SubDomainClass
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="BisCore" version="01.00.00" alias="bis" />

                    <ECEnumeration typeName="StatusEnum" displayLabel="Status" backingTypeName="int" isStrict="true">
                        <ECEnumerator name="On" value="0"/>
                        <ECEnumerator name="Off" value="1"/>
                        <ECEnumerator name="Unknown" value="2"/>
                        <ECEnumerator name="Halfhalf" value="3"/>
                    </ECEnumeration>
                    <ECEntityClass typeName="MyDomainClass">
                        <BaseClass>bis:PhysicalElement</BaseClass>
                        <ECProperty propertyName="Status" typeName="StatusEnum" />
                        </ECEntityClass>
                        <ECEntityClass typeName="SubDomainClass">
                        <BaseClass>MyDomainClass</BaseClass>
                        <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
                    </ECEntityClass>
                </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetECDbAge() || !testDb.SupportsFeature(ECDbFeature::NamedEnumerators))
                {
                EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SchemaStatus::Success, schemaImportStat) << testDb.GetDescription();
            EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();

            EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId,Status,Statuses FROM ts.SubDomainClass")) << testDb.GetDescription();

            ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", "SubDomainClass");
            ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << "SubDomainClass | " << testDb.GetDescription();

            ECPropertyCP statusProp = cl->GetPropertyP("Status");
            ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << "SubDomainClass | " << testDb.GetDescription();
            ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
            ASSERT_TRUE(ecenum != nullptr) << "SubDomainClass | " << testDb.GetDescription();
            testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                {{"On", ECValue(0), nullptr},
                {"Off", ECValue(1), nullptr},
                {"Unknown", ECValue(2), nullptr},
                {"Halfhalf", ECValue(3), nullptr}});

            ECPropertyCP statusesProp = cl->GetPropertyP("Statuses");
            ASSERT_TRUE(statusesProp != nullptr && statusesProp->GetIsPrimitiveArray()) << "SubDomainClass | " << testDb.GetDescription();
            ECEnumerationCP statusesEnum = statusesProp->GetAsPrimitiveArrayProperty()->GetEnumeration();
            ASSERT_TRUE(statusesEnum != nullptr) << "SubDomainClass | " << testDb.GetDescription();
            testDb.AssertEnum(*statusesEnum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                {{"On", ECValue(0), nullptr},
                {"Off", ECValue(1), nullptr},
                {"Unknown", ECValue(2), nullptr},
                {"Halfhalf", ECValue(3), nullptr}});

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
TEST_F(IModelCompatibilityTestFixture, EC31ToEC32SchemaUpgrade_Koqs)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31KOQS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
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
            // - Added subclass SubDomainClass
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <ECSchemaReference name="Formats" version="01.00.00" alias="f" />

                    <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="u:SQ_M" presentationUnits="f:DefaultRealU(4)[u:SQ_M];f:DefaultRealU(4)[u:SQ_FT];f:DefaultRealU(4)[u:SQ_CM]" relativeError="0.001"/>
                    <ECEntityClass typeName="MyDomainClass">
                        <BaseClass>bis:PhysicalElement</BaseClass>
                        <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubDomainClass">
                        <BaseClass>MyDomainClass</BaseClass>
                        <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
                    </ECEntityClass>
                </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            // If the ECDb version is newer or 
            if (ProfileState::Age::Newer == testDb.GetECDbAge() || !testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                {
                EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SchemaStatus::Success, schemaImportStat) << testDb.GetDescription();
            EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();

            EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId,Size,Sizes FROM ts.SubDomainClass")) << testDb.GetDescription();

            ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", "SubDomainClass");
            ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << "SubDomainClass | " << testDb.GetDescription();

            ECPropertyCP sizeProp = cl->GetPropertyP("Size");
            ASSERT_TRUE(sizeProp != nullptr && sizeProp->GetIsPrimitive()) << testDb.GetDescription();
            KindOfQuantityCP koq = sizeProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
            ASSERT_TRUE(koq != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);

            ECPropertyCP sizesProp = cl->GetPropertyP("Sizes");
            ASSERT_TRUE(sizesProp != nullptr && sizesProp->GetIsPrimitiveArray()) << testDb.GetDescription();
            KindOfQuantityCP sizesKoq = sizesProp->GetAsPrimitiveArrayProperty()->GetKindOfQuantity();
            ASSERT_TRUE(sizesKoq != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*sizesKoq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);

            testDb.AssertKindOfQuantity("TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC32SchemaUpgrade_Enums)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32ENUMS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
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
                    <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
                    <ECEnumeration typeName="StatusEnum" displayLabel="Status" backingTypeName="int" isStrict="true">
                        <ECEnumerator name="On" value="0"/>
                        <ECEnumerator name="Off" value="1"/>
                        <ECEnumerator name="Unknown" value="2"/>
                        <ECEnumerator name="Halfhalf" value="3"/>
                    </ECEnumeration>
                    <ECEntityClass typeName="MyDomainClass">
                        <BaseClass>bis:PhysicalElement</BaseClass>
                        <ECProperty propertyName="Status" typeName="StatusEnum" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubDomainClass">
                        <BaseClass>MyDomainClass</BaseClass>
                        <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
                    </ECEntityClass>
                </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetECDbAge() || !testDb.SupportsFeature(ECDbFeature::NamedEnumerators))
                {
                EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SchemaStatus::Success, schemaImportStat) << testDb.GetDescription();

            EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId,Status,Statuses FROM ts.SubDomainClass")) << testDb.GetDescription();

            ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", "SubDomainClass");
            ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << "SubDomainClass | " << testDb.GetDescription();

            ECPropertyCP statusProp = cl->GetPropertyP("Status");
            ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << "SubDomainClass | " << testDb.GetDescription();
            ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
            ASSERT_TRUE(ecenum != nullptr) << "SubDomainClass | " << testDb.GetDescription();
            testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                {{"On", ECValue(0), nullptr},
                {"Off", ECValue(1), nullptr},
                {"Unknown", ECValue(2), nullptr},
                {"Halfhalf", ECValue(3), nullptr}});

            ECPropertyCP statusesProp = cl->GetPropertyP("Statuses");
            ASSERT_TRUE(statusesProp != nullptr && statusesProp->GetIsPrimitiveArray()) << "SubDomainClass | " << testDb.GetDescription();
            ECEnumerationCP statusesEnum = statusesProp->GetAsPrimitiveArrayProperty()->GetEnumeration();
            ASSERT_TRUE(statusesEnum != nullptr) << "SubDomainClass | " << testDb.GetDescription();
            testDb.AssertEnum(*statusesEnum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                {{"On", ECValue(0), nullptr},
                {"Off", ECValue(1), nullptr},
                {"Unknown", ECValue(2), nullptr},
                {"Halfhalf", ECValue(3), nullptr}});

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
TEST_F(IModelCompatibilityTestFixture, EC32SchemaUpgrade_Koqs)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32KOQS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
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
            // - Added subclass MyDomainClass
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
            <ECSchemaReference name="Units" version="01.00.00" alias="u" />
            <ECSchemaReference name="Formats" version="01.00.00" alias="f" />

            <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="u:SQ_M" presentationUnits="f:DefaultRealU(4)[u:SQ_M];f:DefaultRealU(4)[u:SQ_FT];f:DefaultRealU(4)[u:SQ_CM]" relativeError="0.001"/>
            <ECEntityClass typeName="MyDomainClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
             </ECEntityClass>
             <ECEntityClass typeName="SubDomainClass">
                <BaseClass>MyDomainClass</BaseClass>
                <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
             </ECEntityClass>
            </ECSchema>)xml"));

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

            if (ProfileState::Age::Newer == testDb.GetECDbAge() || !testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                {
                EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                continue;
                }

            EXPECT_EQ(SchemaStatus::Success, schemaImportStat) << testDb.GetDescription();
            EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();
            EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId,Size,Sizes FROM ts.SubDomainClass")) << testDb.GetDescription();

            ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", "SubDomainClass");
            ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << "SubDomainClass | " << testDb.GetDescription();

            ECPropertyCP sizeProp = cl->GetPropertyP("Size");
            ASSERT_TRUE(sizeProp != nullptr && sizeProp->GetIsPrimitive()) << testDb.GetDescription();
            KindOfQuantityCP koq = sizeProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
            ASSERT_TRUE(koq != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*koq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);

            ECPropertyCP sizesProp = cl->GetPropertyP("Sizes");
            ASSERT_TRUE(sizesProp != nullptr && sizesProp->GetIsPrimitiveArray()) << testDb.GetDescription();
            KindOfQuantityCP sizesKoq = sizesProp->GetAsPrimitiveArrayProperty()->GetKindOfQuantity();
            ASSERT_TRUE(sizesKoq != nullptr) << testDb.GetDescription();
            testDb.AssertKindOfQuantity(*sizesKoq, "TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);

            testDb.AssertKindOfQuantity("TestSchema", "AREA", "Area", nullptr, "u:SQ_M", JsonValue(R"json(["f:DefaultRealU(4)[u:SQ_M]", "f:DefaultRealU(4)[u:SQ_FT]", "f:DefaultRealU(4)[u:SQ_CM]"])json"), 0.001);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, AddDomain)
    {
    ASSERT_EQ(SUCCESS, DgnDomains::RegisterDomain(FunctionalDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No));
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            const DbResult openStat = testDb.Open();
            DgnDb::OpenParams const& params = static_cast<DgnDb::OpenParams const&> (testDb.GetOpenParams());

            if (params.IsReadonly() || params.GetSchemaUpgradeOptions().GetDomainUpgradeOptions() != SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)
                {
                ASSERT_EQ(BE_SQLITE_ERROR_SchemaUpgradeRequired, openStat) << testDb.GetDescription();
                continue;
                }

            if (testFile.GetECDbAge() == ProfileState::Age::Newer)
                {
                // schema import not possible to newer ECDb profile files
                ASSERT_EQ(BE_SQLITE_ERROR_SchemaUpgradeFailed, openStat) << testDb.GetDescription();
                continue;
                }

            ASSERT_EQ(BE_SQLITE_OK, openStat) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            EXPECT_TRUE(testDb.GetDb().Schemas().ContainsSchema("Functional")) << testDb.GetDescription();
            SchemaVersion expectedSchemaVersion(1, 0, 0);
            SchemaVersion testDbSchemaVersion = testDb.GetSchemaVersion("Functional");
            EXPECT_EQ(expectedSchemaVersion.GetMajor(), testDbSchemaVersion.GetMajor()) << testDb.GetDescription();
            EXPECT_EQ(expectedSchemaVersion.GetMinor(), testDbSchemaVersion.GetMinor()) << testDb.GetDescription();
            // The Functional schema may have updates, so only Major and Minor are checked
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, OpenDomainIModel)
    {
    ASSERT_EQ(SUCCESS, IModelEvolutionTestsDomain::Register(SchemaVersion(1, 0, 0), DgnDomain::Required::Yes, DgnDomain::Readonly::No));
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_TESTDOMAIN))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            EXPECT_TRUE(testDb.GetDb().Schemas().ContainsSchema(TESTDOMAIN_NAME)) << testDb.GetDescription();
            EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion(TESTDOMAIN_NAME)) << testDb.GetDescription();
            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "SELECT Name,Material FROM me.Toy")) << testDb.GetDescription();
            stmt.Finalize();
            testDb.AssertEnum(TESTDOMAIN_NAME, "Material", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{"Material0", ECValue(0), "Wood"},
                 {"Material1", ECValue(1), "Plastic"},
                 {"Material2", ECValue(2), "Metal"}});
            }
        }
    IModelEvolutionTestsDomain::GetDomain().SetRequired(DgnDomain::Required::No);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, UpgradeDomainIModel)
    {
    // bump up domain schema version to trigger upgrade
    ASSERT_EQ(SUCCESS, IModelEvolutionTestsDomain::Register(SchemaVersion(1, 0, 1), DgnDomain::Required::Yes, DgnDomain::Readonly::No));
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_TESTDOMAIN))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            const DbResult openStat = testDb.Open();
            DgnDb::OpenParams const& params = static_cast<DgnDb::OpenParams const&> (testDb.GetOpenParams());

            if (params.GetSchemaUpgradeOptions().GetDomainUpgradeOptions() != SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)
                {
                //opens but schema is not upgraded
                ASSERT_EQ(BE_SQLITE_OK, openStat) << testDb.GetDescription();
                testDb.AssertProfileVersion();
                testDb.AssertLoadSchemas();

                EXPECT_TRUE(testDb.GetDb().Schemas().ContainsSchema(TESTDOMAIN_NAME)) << testDb.GetDescription();
                EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion(TESTDOMAIN_NAME)) << testDb.GetDescription();
                ECSqlStatement stmt;
                EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "SELECT Name,Material FROM me.Toy")) << testDb.GetDescription();
                stmt.Finalize();
                testDb.AssertEnum(TESTDOMAIN_NAME, "Material", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                    {{"Material0", ECValue(0), "Wood"},
                    {"Material1", ECValue(1), "Plastic"},
                    {"Material2", ECValue(2), "Metal"}});

                continue;
                }

            if (testFile.GetECDbAge() == ProfileState::Age::Newer)
                {
                // schema import not possible to newer ECDb profile files
                ASSERT_EQ(BE_SQLITE_ERROR_SchemaUpgradeFailed, openStat) << testDb.GetDescription();
                continue;
                }

            // opened and upgraded the schema
            ASSERT_EQ(BE_SQLITE_OK, openStat) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            // As the schema was upgraded, it should have set the original ECXML version, even if it originally
            // was a 4.0.0.1 file.
            if (testDb.SupportsFeature(ECDbFeature::PersistedECVersions))
                EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion(TESTDOMAIN_NAME)) << testDb.GetDescription();

            EXPECT_EQ(SchemaVersion(1, 0, 1), testDb.GetSchemaVersion(TESTDOMAIN_NAME)) << testDb.GetDescription();
            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "SELECT Name,NickName,Material FROM me.Toy")) << testDb.GetDescription();
            testDb.AssertEnum(TESTDOMAIN_NAME, "Material", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{"Material0", ECValue(0), "Wood"},
                {"Material1", ECValue(1), "Plastic"},
                {"Material2", ECValue(2), "Metal"},
                {"Material3", ECValue(3), "Mix"}});
            }
        }

    IModelEvolutionTestsDomain::GetDomain().SetRequired(DgnDomain::Required::No);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, UpgradeDomainIModelToEC32)
    {
    // bump up domain schema version to trigger upgrade
    // Schema 1.0.2 includes the conversion to EC3.2
    ASSERT_EQ(SUCCESS, IModelEvolutionTestsDomain::Register(SchemaVersion(1, 0, 2), DgnDomain::Required::Yes, DgnDomain::Readonly::No));
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_TESTDOMAIN))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            const DbResult openStat = testDb.Open();
            DgnDb::OpenParams const& params = static_cast<DgnDb::OpenParams const&> (testDb.GetOpenParams());

            if (params.GetSchemaUpgradeOptions().GetDomainUpgradeOptions() != SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)
                {
                ASSERT_EQ(BE_SQLITE_OK, openStat) << testDb.GetDescription();
                testDb.AssertProfileVersion();
                testDb.AssertLoadSchemas();

                EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion(TESTDOMAIN_NAME)) << testDb.GetDescription();
                ECSqlStatement stmt;
                EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "SELECT Name,Material FROM me.Toy")) << testDb.GetDescription();
                testDb.AssertEnum(TESTDOMAIN_NAME, "Material", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                    {{"Material0", ECValue(0), "Wood"},
                    {"Material1", ECValue(1), "Plastic"},
                    {"Material2", ECValue(2), "Metal"}});
                continue;
                }

            if (ProfileState::Age::Newer == testFile.GetECDbAge()
                || !testDb.VersionSupportsFeature(testFile.GetECDbVersion(), ECDbFeature::EC32)
                && !testDb.IsUpgraded())
                {
                //schema import not possible to newer ECDb profile files or files that don't support EC3.2
                ASSERT_EQ(BE_SQLITE_ERROR_SchemaUpgradeFailed, openStat) << testDb.GetDescription();
                continue;
                }

            ASSERT_EQ(BE_SQLITE_OK, openStat) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            EXPECT_TRUE(testDb.GetDb().Schemas().ContainsSchema(TESTDOMAIN_NAME)) << testDb.GetDescription();
            EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion(TESTDOMAIN_NAME)) << testDb.GetDescription();
            EXPECT_EQ(SchemaVersion(1, 0, 2), testDb.GetSchemaVersion(TESTDOMAIN_NAME)) << testDb.GetDescription();

            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "SELECT Name,NickName,Material FROM me.Toy")) << testDb.GetDescription();

            testDb.AssertEnum(TESTDOMAIN_NAME, "Material", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{"Wood", ECValue(0), "Wood"},
                {"Plastic", ECValue(1), "Plastic"},
                {"Metal", ECValue(2), "Metal"},
                {"Mix", ECValue(3), "Mix"}});
            }
        }

    IModelEvolutionTestsDomain::GetDomain().SetRequired(DgnDomain::Required::No);
    }
