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

        JsonValue schemaCountJson = TestHelper::ExecuteECSqlSelect(*bim, "SELECT count(*) schemaCount FROM meta.ECSchemaDef");
        ASSERT_TRUE(schemaCountJson.m_value.isArray() && schemaCountJson.m_value.size() == 1 && schemaCountJson.m_value[0].isMember("schemaCount")) << schemaCountJson.ToString();
        const int schemaCount = schemaCountJson.m_value[0]["schemaCount"].asInt();

        switch (testFile.GetProfileState())
            {
                case ProfileState::Current:
                case ProfileState::Older:
                {
                EXPECT_EQ(8, schemaCount) << testFile.GetPath().GetNameUtf8();

                for (ECSchemaCP schema : bim->Schemas().GetSchemas(false))
                    {
                    EXPECT_EQ((int) ECVersion::Latest, (int) schema->GetECVersion()) << schema->GetFullSchemaName();
                    //OriginalECXML version not persisted in ECDb pre 4.0.0.2, so ECObjects defaults to 3.1
                    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor()) << schema->GetFullSchemaName();
                    EXPECT_EQ(1, schema->GetOriginalECXmlVersionMinor()) << schema->GetFullSchemaName();
                    }

                //DgnDb built-in schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 1), TestHelper::GetSchemaVersion(*bim, "BisCore"));
                EXPECT_EQ(SchemaVersion(1, 0, 0), TestHelper::GetSchemaVersion(*bim, "Generic"));

                //ECDb built-in schema versions
                EXPECT_EQ(SchemaVersion(2, 0, 0), TestHelper::GetSchemaVersion(*bim, "ECDbFileInfo"));
                EXPECT_EQ(SchemaVersion(2, 0, 0), TestHelper::GetSchemaVersion(*bim, "ECDbMap"));
                EXPECT_EQ(SchemaVersion(4, 0, 0), TestHelper::GetSchemaVersion(*bim, "ECDbMeta"));
                EXPECT_EQ(SchemaVersion(5, 0, 0), TestHelper::GetSchemaVersion(*bim, "ECDbSystem"));
                EXPECT_EQ(SchemaVersion(1, 0, 0), TestHelper::GetSchemaVersion(*bim, "ECDbSchemaPolicies"));

                //standard schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), TestHelper::GetSchemaVersion(*bim, "CoreCustomAttributes"));
                break;
                }

                case ProfileState::Newer:
                {
                EXPECT_EQ(8, schemaCount) << testFile.GetPath().GetNameUtf8();

                for (ECSchemaCP schema : bim->Schemas().GetSchemas(false))
                    {
                    EXPECT_EQ((int) ECVersion::Latest, (int) schema->GetECVersion()) << schema->GetFullSchemaName();
                    //OriginalECXML version not read by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor()) << schema->GetFullSchemaName();
                    EXPECT_EQ(1, schema->GetOriginalECXmlVersionMinor()) << schema->GetFullSchemaName();
                    }

                //DgnDb built-in schema versions
                EXPECT_LE(SchemaVersion(1, 0, 1), TestHelper::GetSchemaVersion(*bim, "BisCore"));
                EXPECT_LE(SchemaVersion(1, 0, 0), TestHelper::GetSchemaVersion(*bim, "Generic"));

                //ECDb built-in schema versions
                //ECDbFileInfo version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(2, 0, 0), TestHelper::GetSchemaVersion(*bim, "ECDbFileInfo"));
                EXPECT_LE(SchemaVersion(2, 0, 0), TestHelper::GetSchemaVersion(*bim, "ECDbMap"));
                //ECDbMeta version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(4, 0, 0), TestHelper::GetSchemaVersion(*bim, "ECDbMeta"));
                //ECDbSystem version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(5, 0, 0), TestHelper::GetSchemaVersion(*bim, "ECDbSystem"));
                EXPECT_LE(SchemaVersion(1, 0, 0), TestHelper::GetSchemaVersion(*bim, "ECDbSchemaPolicies"));

                //standard schema versions
                EXPECT_LE(SchemaVersion(1, 0, 0), TestHelper::GetSchemaVersion(*bim, "CoreCustomAttributes"));

                break;
                }
            }
        }
    }

