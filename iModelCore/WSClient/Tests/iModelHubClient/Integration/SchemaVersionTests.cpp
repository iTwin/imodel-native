/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/SchemaVersionTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE

static const Utf8CP s_iModelName = "SchemaVersionTests";

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaVersionTests : public iModelTestsBase
    {
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        iModelTestsBase::SetUpTestCase();
        iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info, 2);
        SchemaVersionTestDomain::Register("02.02.02", DgnDomain::Required::Yes, DgnDomain::Readonly::No);
        SchemaVersionTestDomain::GetDomain().RegisterHandler(TestElementHandler::GetHandler());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        SchemaVersionTestDomain::GetDomain().SetRequired(DgnDomain::Required::No);
        iModelTestsBase::TearDownTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AcquireBriefcase(bool pull = true)
        {
        BriefcaseInfoResult result = iModelHubHelpers::AcquireBriefcase(s_client, s_info, pull);
        ASSERT_SUCCESS(result);
        auto dbPath = result.GetValue()->GetLocalPath();
        EXPECT_TRUE(dbPath.DoesPathExist());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AcquireAndOpenBriefcase(BriefcasePtr& briefcase, iModelInfoPtr info)
        {
        DgnDb::OpenParams openParams(DgnDb::OpenMode::ReadWrite, DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade));
        BriefcaseInfoResult acquireResult = iModelHubHelpers::AcquireBriefcase(s_client, info);
        ASSERT_SUCCESS(acquireResult);

        DgnDbPtr db1 = DgnDb::OpenDgnDb(nullptr, acquireResult.GetValue()->GetLocalPath(), openParams);
        BriefcaseResult openResult = iModelHubHelpers::OpenBriefcase(s_client, db1);
        ASSERT_SUCCESS(openResult);

        briefcase = openResult.GetValue();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    BriefcasePtr AcquireAndOpenBriefcase()
        {
        BriefcasePtr briefcase;
        AcquireAndOpenBriefcase(briefcase, s_info);
        return briefcase;
        }
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaVersionTests, AcquireBriefcaseWithoutSync)
    {
    AcquireBriefcase(false);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaVersionTests, AcquireBriefcaseChangeSetsInServer)
    {
    AcquireBriefcase(true);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaVersionTests, AcquireBriefcaseWithSchemaChanges)
    {
    // First briefcase makes schema changes
    BriefcasePtr briefcase1 = AcquireAndOpenBriefcase();

    briefcase1->GetDgnDb().CreateTable("TestTable1", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    ASSERT_TRUE(briefcase1->GetDgnDb().Txns().HasDbSchemaChanges());
    briefcase1->GetDgnDb().SaveChanges("ChangeSet 1");
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false));

    AcquireBriefcase(true);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaVersionTests, ImportSchemasAndMakeChanges)
    {
    SchemaVersionTestDomain::GetDomain().SetRequired(DgnDomain::Required::No);
    iModelResult createResult = CreateEmptyiModel(GetTestiModelName());
    ASSERT_SUCCESS(createResult);
    iModelInfoPtr info = createResult.GetValue();
    SchemaVersionTestDomain::GetDomain().SetRequired(DgnDomain::Required::Yes);

    // Acquire briefcase
    BriefcasePtr briefcase1;
    AcquireAndOpenBriefcase(briefcase1, info);
    DgnDbR db1 = briefcase1->GetDgnDb();
    EXPECT_TRUE(db1.Schemas().ContainsSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));

    // Push changes
    db1.SaveChanges("ChangeSet 1");
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase1, true, false));

    // Try to acquire other briefcase
    BriefcasePtr briefcase2;
    AcquireAndOpenBriefcase(briefcase2, info);
    DgnDbR db2 = briefcase2->GetDgnDb();
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("Used", db2));
    EXPECT_TRUE(db1.Schemas().ContainsSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));

    db2.SaveChanges("ChangeSet 2");
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase2, true, false));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2017
//---------------------------------------------------------------------------------------
TEST_F(SchemaVersionTests, DownloadStandaloneBriefcase)
    {
    BeFileName outputDir = OutputDir();
    auto result = s_client->DownloadStandaloneBriefcase(*s_info, [=] (iModelInfo imodelInfo, FileInfo fileInfo)
        {
        BeFileName filePath = outputDir;
        filePath.AppendToPath(BeFileName(imodelInfo.GetId()));
        filePath.AppendToPath(BeFileName(fileInfo.GetFileName()));
        return filePath;
        })->GetResult();
    ASSERT_SUCCESS(result);
    EXPECT_TRUE(result.GetValue().DoesPathExist());
    }
