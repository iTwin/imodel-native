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
        ASSERT_EQ(BE_SQLITE_OK, stat) << testFile.GetPath().GetNameUtf8();
        ASSERT_TRUE(bim != nullptr) << testFile.GetPath().GetNameUtf8();

        TestHelper helper(testFile, *bim);
        helper.AssertLoadSchemas();
        const int schemaCount = helper.GetSchemaCount();

        switch (testFile.GetProfileState())
            {
                case ProfileState::Current:
                case ProfileState::Older:
                {
                EXPECT_EQ(8, schemaCount) << testFile.GetPath().GetNameUtf8();

                for (ECSchemaCP schema : bim->Schemas().GetSchemas(false))
                    {
                    EXPECT_EQ((int) ECVersion::V3_2, (int) schema->GetECVersion()) << schema->GetFullSchemaName();
                    }

                //DgnDb built-in schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("BisCore"));
                EXPECT_EQ(BeVersion(3, 1), helper.GetOriginalECXmlVersion("BisCore"));
                EXPECT_EQ(JsonValue(R"js({"classcount":163, "enumcount": 2})js"), helper.GetSchemaItemCounts("BisCore"));

                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("Generic"));
                EXPECT_EQ(BeVersion(3, 1), helper.GetOriginalECXmlVersion("Generic"));
                EXPECT_EQ(JsonValue(R"js({"classcount":17})js"), helper.GetSchemaItemCounts("Generic"));

                //ECDb built-in schema versions
                EXPECT_EQ(SchemaVersion(2, 0, 1), helper.GetSchemaVersion("ECDbFileInfo"));
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbFileInfo")) << "Schema has enums which were upgraded to EC32 format, therefore original XML version was set to 3.2";
                EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), helper.GetSchemaItemCounts("ECDbFileInfo"));
                EXPECT_EQ(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap"));
                EXPECT_EQ(BeVersion(), helper.GetOriginalECXmlVersion("ECDbMap")) << "Schema has no enums, so was not upgraded to EC32, so no original ECXML persisted yet";
                EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), helper.GetSchemaItemCounts("ECDbMap"));
                EXPECT_EQ(SchemaVersion(4, 0, 1), helper.GetSchemaVersion("ECDbMeta"));
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMeta")) << "Schema has enums which were upgraded to EC32 format, therefore original XML version was set to 3.2";
                EXPECT_EQ(JsonValue(R"js({"classcount":24, "enumcount": 8})js"), helper.GetSchemaItemCounts("ECDbMeta"));
                EXPECT_EQ(SchemaVersion(5, 0, 1), helper.GetSchemaVersion("ECDbSystem"));
                EXPECT_EQ(BeVersion(), helper.GetOriginalECXmlVersion("ECDbSystem")) << "Schema has no enums, so was not upgraded to EC32, so no original ECXML persisted yet";
                EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), helper.GetSchemaItemCounts("ECDbSystem"));
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("ECDbSchemaPolicies"));
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSchemaPolicies"));
                EXPECT_EQ(JsonValue(R"js({"classcount":3})js"), helper.GetSchemaItemCounts("ECDbSchemaPolicies"));

                //standard schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes"));
                EXPECT_EQ(BeVersion(3, 1), helper.GetOriginalECXmlVersion("CoreCustomAttributes"));
                EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), helper.GetSchemaItemCounts("CoreCustomAttributes"));
                break;
                }

                case ProfileState::Newer:
                {
                EXPECT_EQ(8, schemaCount) << testFile.GetPath().GetNameUtf8();

                for (ECSchemaCP schema : bim->Schemas().GetSchemas(false))
                    {
                    EXPECT_LE((int) ECVersion::V3_2, (int) schema->GetECVersion()) << schema->GetFullSchemaName();
                    }

                //DgnDb built-in schema versions
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("BisCore"));
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("BisCore"));
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("Generic"));
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("Generic"));

                //ECDb built-in schema versions
                EXPECT_LE(SchemaVersion(2, 0, 1), helper.GetSchemaVersion("ECDbFileInfo"));
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbFileInfo"));
                EXPECT_LE(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap"));
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMap"));
                EXPECT_LE(SchemaVersion(4, 0, 1), helper.GetSchemaVersion("ECDbMeta"));
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMeta"));
                EXPECT_LE(SchemaVersion(5, 0, 1), helper.GetSchemaVersion("ECDbSystem"));
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSystem"));
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("ECDbSchemaPolicies"));
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSchemaPolicies"));

                //standard schema versions
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes"));
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("CoreCustomAttributes"));

                break;
                }
            }
        }
    }

