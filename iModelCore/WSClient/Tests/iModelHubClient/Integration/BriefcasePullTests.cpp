/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/BriefcasePullTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"
#include "LRPJobBackdoorAPI.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE

static const Utf8CP s_iModelName = "BriefcasePullTests";

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct BriefcasePullTests : public iModelTestsBase
    {
    static VersionInfoPtr       s_version1;
    static VersionInfoPtr       s_version2;
    static VersionInfoPtr       s_version3;
    DgnDbPtr                    m_db;

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              12/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        iModelTestsBase::SetUpTestCase();
        iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info, 7);
        iModelHubHelpers::CreateNamedVersion(s_version1, s_connection, "Version1", 2);
        iModelHubHelpers::CreateNamedVersion(s_version2, s_connection, "Version2", 4);
        iModelHubHelpers::CreateNamedVersion(s_version3, s_connection, "Version3", 6);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              12/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        iModelTestsBase::TearDownTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              12/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void TearDown() override
        {
        if (m_db.IsValid())
            {
            if (m_db->IsDbOpen())
                m_db->CloseDb();
            m_db = nullptr;
            }
        iModelTestsBase::TearDown();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              12/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExpectFileIsReadOnly(BeFileNameCR path, Utf8String changesetId)
        {
        DgnDbPtr db;
        OpenReadOnlyDgnDb(db, path);

        BriefcaseResult briefcaseResult = s_client->OpenBriefcase(db, false)->GetResult();
        ASSERT_FAILURE(briefcaseResult);
        EXPECT_EQ(Error::Id::FileIsNotBriefcase, briefcaseResult.GetError().GetId());
        EXPECT_EQ(changesetId, db->Revisions().GetParentRevisionId());

        PhysicalPartitionPtr partition = CreateModeledElement(TestCodeName().c_str(), *db);
        EXPECT_FALSE(partition->Insert().IsValid());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              12/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AcquireAndOpenDgnDb(iModelInfoPtr info, bool pull = false)
        {
        BriefcaseInfoResult acquireResult = iModelHubHelpers::AcquireBriefcase(s_client, info, pull);
        ASSERT_SUCCESS(acquireResult);

        m_db = nullptr;
        OpenDgnDb(m_db, acquireResult.GetValue()->GetLocalPath());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              12/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExpectDbIsUpToDate(DgnDbCR db)
        {
        EXPECT_EQ(iModelHubHelpers::GetLastChangeSet(s_connection)->GetId(), db.Revisions().GetParentRevisionId());
        }
    };
VersionInfoPtr BriefcasePullTests::s_version1 = nullptr;
VersionInfoPtr BriefcasePullTests::s_version2 = nullptr;
VersionInfoPtr BriefcasePullTests::s_version3 = nullptr;


/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcasePullTests, SuccessfulAcquireBriefcaseWithoutMerge)
    {
    AcquireAndOpenDgnDb(s_info);
    iModelHubHelpers::OpenBriefcase(s_client, m_db);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcasePullTests, SuccessfulAcquireAndMergeBriefcase)
    {
    AcquireAndOpenDgnDb(s_info, true);
    ExpectDbIsUpToDate(*m_db);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcasePullTests, SuccessfulOpenAndMergeBriefcase)
    {
    AcquireAndOpenDgnDb(s_info);
    iModelHubHelpers::OpenBriefcase(s_client, m_db, true);
    ExpectDbIsUpToDate(*m_db);
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcasePullTests, DownloadStandaloneBriefcaseUpdatedToVersion)
    {
    TestsProgressCallback callback;
    BeFileNameResult acquireResult = s_client->DownloadStandaloneBriefcaseUpdatedToVersion(*s_info, s_version2->GetId(), [=](iModelInfo imodelInfo, FileInfo fileInfo)
        {
        BeFileName filePath = OutputDir();
        filePath.AppendToPath(BeFileName(imodelInfo.GetId()));
        filePath.AppendToPath(BeFileName(s_version2->GetId()));
        filePath.AppendToPath(BeFileName(fileInfo.GetFileName()));
        return filePath;
        }, callback.Get())->GetResult();
    ASSERT_SUCCESS(acquireResult);
    callback.Verify();
    ExpectFileIsReadOnly(acquireResult.GetValue(), s_version2->GetChangeSetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Gintare.Grazulyte    12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcasePullTests, DownloadStandaloneBriefcaseUpdatedToVersionInvalidVersion)
    {
    BeFileNameResult acquireResult = s_client->DownloadStandaloneBriefcaseUpdatedToVersion(*s_info, "11111111-1111-4111-8111-111111111111", [=](iModelInfo imodelInfo, FileInfo fileInfo)
        {
        BeFileName filePath = OutputDir();
        filePath.AppendToPath(BeFileName(imodelInfo.GetId()));
        filePath.AppendToPath(BeFileName("11111111-1111-4111-8111-111111111111"));
        filePath.AppendToPath(BeFileName(fileInfo.GetFileName()));
        return filePath;
        })->GetResult();
    ASSERT_FAILURE(acquireResult);
    EXPECT_EQ(Error::Id::InvalidVersion, acquireResult.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcasePullTests, DownloadStandaloneBriefcaseUpdatedToChangeSet)
    {
    Utf8String briefcaseChangeSet = iModelHubHelpers::GetChangeSetByIndex(s_connection, 2)->GetId();
    TestsProgressCallback callback;
    BeFileNameResult acquireResult = s_client->DownloadStandaloneBriefcaseUpdatedToChangeSet(*s_info, briefcaseChangeSet, [=](iModelInfo imodelInfo, FileInfo fileInfo)
        {
        BeFileName filePath = OutputDir();
        filePath.AppendToPath(BeFileName(imodelInfo.GetId()));
        filePath.AppendToPath(BeFileName(briefcaseChangeSet));
        filePath.AppendToPath(BeFileName(fileInfo.GetFileName()));
        return filePath;
        }, callback.Get())->GetResult();
    ASSERT_SUCCESS(acquireResult);
    callback.Verify();
    ExpectFileIsReadOnly(acquireResult.GetValue(), briefcaseChangeSet);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcasePullTests, DownloadStandaloneBriefcase)
    {
    TestsProgressCallback callback;
    BeFileNameResult acquireResult = s_client->DownloadStandaloneBriefcase(*s_info, [=](iModelInfo imodelInfo, FileInfo fileInfo)
        {
        BeFileName filePath = OutputDir();
        filePath.AppendToPath(BeFileName(imodelInfo.GetId()));
        filePath.AppendToPath(BeFileName(fileInfo.GetFileName()));
        return filePath;
        }, callback.Get())->GetResult();
    ASSERT_SUCCESS(acquireResult);
    callback.Verify();
    Utf8String lastChangeSet = iModelHubHelpers::GetChangeSetByIndex(s_connection, 7)->GetId();
    ExpectFileIsReadOnly(acquireResult.GetValue(), lastChangeSet);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcasePullTests, UpdateBriefcaseToVersion)
    {
    //aquire briefcase without changeSets
    BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, s_info, false);
    ASSERT_SUCCESS(briefcaseResult);
    BriefcasePtr briefcase = briefcaseResult.GetValue();

    //add changeSets
    ASSERT_SUCCESS(iModelHubHelpers::UpdateToVersion(briefcase, s_version2));

    //remove changeSets
    ASSERT_SUCCESS(iModelHubHelpers::UpdateToVersion(briefcase, s_version1));

    //add changeSets after removal
    ASSERT_SUCCESS(iModelHubHelpers::UpdateToVersion(briefcase, s_version3));

    //pull
    EXPECT_SUCCESS(briefcase->PullAndMerge()->GetResult());
    ExpectDbIsUpToDate(briefcase->GetDgnDb());

    ASSERT_TRUE(CreateModel(TestCodeName().c_str(), briefcase->GetDgnDb()).IsValid());
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true));

    ASSERT_SUCCESS(iModelHubHelpers::UpdateToVersion(briefcase, s_version1));

    //pull with reverted and new changeSets
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, false, true));
    ExpectDbIsUpToDate(briefcase->GetDgnDb());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Robertas.Maleckas      11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BriefcasePullTests, SuccessfulAcquireBriefcaseAfterMergeJob)
    {
    //This test will merge the imodel, making it no longer usable for this test fixture
    IWSRepositoryClientPtr projectClient = nullptr;
    iModelHubHelpers::CreateProjectWSClient(projectClient, *s_client, s_projectId);
    ASSERT_TRUE(nullptr != projectClient);
    Utf8StringCR mergeJobId = LRPJobBackdoorAPI::ScheduleLRPJob(projectClient, "MergeJob", s_info->GetId());

    bool mergeJobSuccessful = LRPJobBackdoorAPI::WaitForLRPJobToFinish(projectClient, mergeJobId);
    EXPECT_TRUE(mergeJobSuccessful);

    BriefcaseInfoResult acquireBriefcaseResult = iModelHubHelpers::AcquireBriefcase(s_client, s_info);
    ASSERT_SUCCESS(acquireBriefcaseResult);
    auto dbPath = acquireBriefcaseResult.GetValue()->GetLocalPath();
    EXPECT_TRUE(dbPath.DoesPathExist());
    }

