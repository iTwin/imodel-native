/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/IModelCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include "ProfileManager.h"
#include "TestIModelCreators.h"
#include "TestHelper.h"

USING_NAMESPACE_BENTLEY_EC

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct IModelCompatibilityTestFixture : CompatibilityTestFixture
    {
    protected:
        ScopedDgnHost m_host;

        Profile& Profile() const { return ProfileManager::Get().GetProfile(ProfileType::DgnDb); }

        DgnDbPtr OpenTestFile(DbResult* stat, BeFileNameCR path) { return DgnDb::OpenDgnDb(stat, path, DgnDb::OpenParams(DgnDb::OpenMode::Readonly)); }

        void SetUp() override { ASSERT_EQ(SUCCESS, TestIModelCreation::Run()); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, BuiltinSchemaVersions)
    {
    for (TestFile const& testFile : Profile().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        DbResult stat = BE_SQLITE_ERROR;
        DgnDbPtr bim = OpenTestFile(&stat, testFile.GetPath());
        ASSERT_EQ(BE_SQLITE_OK, stat) << testFile.ToString();
        ASSERT_TRUE(bim != nullptr) << testFile.ToString();

        TestHelper helper(testFile, *bim);
        helper.AssertLoadSchemas();
        const int schemaCount = helper.GetSchemaCount();

        switch (testFile.GetProfileState())
            {
                case ProfileState::Current:
                case ProfileState::Older:
                {
                EXPECT_EQ(8, schemaCount) << testFile.ToString();

                for (ECSchemaCP schema : bim->Schemas().GetSchemas(false))
                    {
                    //ECVersion not read by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ((int) ECVersion::V3_1, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    //OriginalECXML version not persisted in ECDb pre 4.0.0.2, so ECObjects defaults to 3.1
                    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    EXPECT_EQ(1, schema->GetOriginalECXmlVersionMinor()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    }

                //DgnDb built-in schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("BisCore")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":163, "enumcount": 2})js"), helper.GetSchemaItemCounts("BisCore")) << testFile.ToString();

                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("Generic")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":17})js"), helper.GetSchemaItemCounts("Generic")) << testFile.ToString();

                //ECDb built-in schema versions
                EXPECT_EQ(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), helper.GetSchemaItemCounts("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), helper.GetSchemaItemCounts("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(4, 0, 0), helper.GetSchemaVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":24, "enumcount": 8})js"), helper.GetSchemaItemCounts("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(5, 0, 0), helper.GetSchemaVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), helper.GetSchemaItemCounts("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("ECDbSchemaPolicies")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":3})js"), helper.GetSchemaItemCounts("ECDbSchemaPolicies")) << testFile.ToString();

                //standard schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), helper.GetSchemaItemCounts("CoreCustomAttributes")) << testFile.ToString();
                break;
                }

                case ProfileState::Newer:
                {
                EXPECT_EQ(8, schemaCount) << testFile.ToString();

                for (ECSchemaCP schema : bim->Schemas().GetSchemas(false))
                    {
                    //ECVersion not read by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ((int) ECVersion::V3_1, (int) schema->GetECVersion()) << schema->GetFullSchemaName();
                    //OriginalECXML version not read by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor()) << schema->GetFullSchemaName();
                    EXPECT_EQ(1, schema->GetOriginalECXmlVersionMinor()) << schema->GetFullSchemaName();
                    }

                //DgnDb built-in schema versions
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("BisCore")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("Generic")) << testFile.ToString();

                //ECDb built-in schema versions
                //ECDbFileInfo version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap")) << testFile.ToString();
                //ECDbMeta version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(4, 0, 0), helper.GetSchemaVersion("ECDbMeta")) << testFile.ToString();
                //ECDbSystem version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(5, 0, 0), helper.GetSchemaVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("ECDbSchemaPolicies")) << testFile.ToString();

                //standard schema versions
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes")) << testFile.ToString();

                break;
                }
            }
        }
    }

