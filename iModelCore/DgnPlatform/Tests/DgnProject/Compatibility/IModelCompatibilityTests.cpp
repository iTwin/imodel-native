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
                {
                EXPECT_EQ(8, schemaCount) << testFile.ToString();

                for (ECSchemaCP schema : bim->Schemas().GetSchemas(false))
                    {
                    EXPECT_EQ((int) ECVersion::V3_2, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    }

                //DgnDb built-in schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("BisCore")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 1), helper.GetOriginalECXmlVersion("BisCore")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":163, "enumcount": 2})js"), helper.GetSchemaItemCounts("BisCore")) << testFile.ToString();

                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("Generic")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 1), helper.GetOriginalECXmlVersion("Generic")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":17})js"), helper.GetSchemaItemCounts("Generic")) << testFile.ToString();

                //ECDb built-in schema versions
                EXPECT_EQ(SchemaVersion(2, 0, 1), helper.GetSchemaVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbFileInfo")) << "Schema has enums which were upgraded to EC32 format, therefore original XML version was set to 3.2 | " << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), helper.GetSchemaItemCounts("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMap")) << "Schema has no enums, so was not upgraded to EC32, so no original ECXML persisted yet | " << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), helper.GetSchemaItemCounts("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(4, 0, 1), helper.GetSchemaVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMeta")) << "Schema has enums which were upgraded to EC32 format, therefore original XML version was set to 3.2 | " << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":38, "enumcount": 9})js"), helper.GetSchemaItemCounts("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(5, 0, 1), helper.GetSchemaVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSystem")) << "Schema has no enums, so was not upgraded to EC32, so no original ECXML persisted yet | " << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), helper.GetSchemaItemCounts("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("ECDbSchemaPolicies")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSchemaPolicies")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":3})js"), helper.GetSchemaItemCounts("ECDbSchemaPolicies")) << testFile.ToString();

                //standard schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 1), helper.GetOriginalECXmlVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), helper.GetSchemaItemCounts("CoreCustomAttributes")) << testFile.ToString();
                break;
                }
                case ProfileState::Older:
                {
                EXPECT_EQ(8, schemaCount) << testFile.ToString();

                for (ECSchemaCP schema : bim->Schemas().GetSchemas(false))
                    {
                    EXPECT_EQ((int) ECVersion::V3_2, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    }

                //DgnDb built-in schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("BisCore")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 1), helper.GetOriginalECXmlVersion("BisCore")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":163, "enumcount": 2})js"), helper.GetSchemaItemCounts("BisCore")) << testFile.ToString();

                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("Generic")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 1), helper.GetOriginalECXmlVersion("Generic")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":17})js"), helper.GetSchemaItemCounts("Generic")) << testFile.ToString();

                //ECDb built-in schema versions
                EXPECT_EQ(SchemaVersion(2, 0, 1), helper.GetSchemaVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), helper.GetSchemaItemCounts("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(BeVersion(), helper.GetOriginalECXmlVersion("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), helper.GetSchemaItemCounts("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(4, 0, 1), helper.GetSchemaVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":24, "enumcount": 8})js"), helper.GetSchemaItemCounts("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(5, 0, 1), helper.GetSchemaVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(BeVersion(), helper.GetOriginalECXmlVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), helper.GetSchemaItemCounts("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("ECDbSchemaPolicies")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSchemaPolicies")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":3})js"), helper.GetSchemaItemCounts("ECDbSchemaPolicies")) << testFile.ToString();

                //standard schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_EQ(BeVersion(), helper.GetOriginalECXmlVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), helper.GetSchemaItemCounts("CoreCustomAttributes"));
                break;
                }

                case ProfileState::Newer:
                {
                EXPECT_EQ(8, schemaCount) << testFile.ToString();

                for (ECSchemaCP schema : bim->Schemas().GetSchemas(false))
                    {
                    EXPECT_LE((int) ECVersion::V3_2, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    }

                //DgnDb built-in schema versions
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("BisCore")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("BisCore")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("Generic")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("Generic")) << testFile.ToString();

                //ECDb built-in schema versions
                EXPECT_LE(SchemaVersion(2, 0, 1), helper.GetSchemaVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMap")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(4, 0, 1), helper.GetSchemaVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(5, 0, 1), helper.GetSchemaVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("ECDbSchemaPolicies")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSchemaPolicies")) << testFile.ToString();

                //standard schema versions
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("CoreCustomAttributes")) << testFile.ToString();

                break;
                }
            }
        }
    }

