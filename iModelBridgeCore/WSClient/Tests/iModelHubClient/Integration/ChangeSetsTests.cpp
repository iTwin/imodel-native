/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/ChangeSetsTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE

static const Utf8CP s_iModelName2 = "ChangeSetsTests2";

int GetDirSize(BeFileName dirToCheck)
    {
    BeFileListIterator fileIterator(dirToCheck.GetName(), false);
    BeFileName tempFileName;
    int size = 0;

    while (SUCCESS == fileIterator.GetNextFileName(tempFileName))
        {
        if (!tempFileName.GetExtension().Equals(L"lock"))
            size++;
        }

    return size;
    }

void GetPreDownloadDirectory(BeFileName& preDownloadPath)
    {
    BentleyStatus status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(preDownloadPath, L"DgnDbRev\\PreDownload");
    ASSERT_EQ(SUCCESS, status) << "Cannot get pre-download directory";
    preDownloadPath.AppendToPath(L"*");
    }

bool IsDirEmpty(BeFileName dirToCheck)
    {
    return 0 == GetDirSize(dirToCheck);
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChangeSetsTests : public iModelTestsBase
    {
    BriefcasePtr m_briefcase = nullptr;

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        iModelTestsBase::SetUpTestCase();
        iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info, 3);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        iModelTestsBase::TearDownTestCase();
        }
    

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void SetUp() override
        {
        iModelTestsBase::SetUp();
        BeFileName preDownloadPath;
        GetPreDownloadDirectory(preDownloadPath);
        if (!IsDirEmpty(preDownloadPath))
            {
            BeFileName::EmptyAndRemoveDirectory(preDownloadPath.c_str());
            }
        }
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void TearDown() override
        {
        BeFileName preDownloadPath;
        GetPreDownloadDirectory(preDownloadPath);
        if (!IsDirEmpty(preDownloadPath))
            {
            BeFileName::EmptyAndRemoveDirectory(preDownloadPath.c_str());
            }
        if (m_briefcase.IsValid())
            {
            m_briefcase = nullptr;
            }
        iModelTestsBase::TearDown();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    DgnModelPtr CreateTestModel(BriefcaseCR briefcase)
        {
        return CreateModel(GetTestInfo().name(), briefcase.GetDgnDb());
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetsTests, PushToEmptyiModel)
    {
    m_imodelName = GetTestiModelName();
    ASSERT_SUCCESS(iModelHubHelpers::DeleteiModelByName(s_client, m_imodelName));
    DgnDbPtr db = CreateTestDb();
    iModelResult createResult = CreateiModel(db);
    ASSERT_SUCCESS(createResult);

    BriefcaseResult acquireResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, createResult.GetValue(), false);
    ASSERT_SUCCESS(acquireResult);

    BriefcasePtr briefcase = acquireResult.GetValue();
    DgnModelPtr model = CreateTestModel(*briefcase);
    CreateElement(*model, DgnCode(), false);

    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true));

    Utf8String lastChangeSetId = briefcase->GetDgnDb().Revisions().GetParentRevisionId();
    EXPECT_FALSE(lastChangeSetId.empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetsTests, PullAndMerge)
    {
    m_briefcase = AcquireAndOpenBriefcase(false);

    BoolResult upToDateResult = m_briefcase->IsBriefcaseUpToDate()->GetResult();
    ASSERT_SUCCESS(upToDateResult);
    EXPECT_FALSE(upToDateResult.GetValue());

    ChangeSetsResult result = iModelHubHelpers::PullMergeAndPush(m_briefcase, false, true);
    ASSERT_SUCCESS(result);
    EXPECT_EQ(3, result.GetValue().size());

    Utf8String lastChangeSetId = m_briefcase->GetDgnDb().Revisions().GetParentRevisionId();
    EXPECT_FALSE(lastChangeSetId.empty());
    EXPECT_EQ(lastChangeSetId, result.GetValue().back()->GetId());

    upToDateResult = m_briefcase->IsBriefcaseUpToDate()->GetResult();
    ASSERT_SUCCESS(upToDateResult);
    EXPECT_TRUE(upToDateResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetsTests, ChangeSetsInfo)
    {
    m_briefcase = AcquireAndOpenBriefcase(true);
    Utf8String lastChangeSetId = m_briefcase->GetDgnDb().Revisions().GetParentRevisionId();

    ChangeSetsInfoResult changeSetsResult = m_briefcase->GetiModelConnection().GetAllChangeSets()->GetResult();
    ASSERT_SUCCESS(changeSetsResult);
    EXPECT_EQ(3, changeSetsResult.GetValue().size());

    ChangeSetInfoPtr lastChangeSet = changeSetsResult.GetValue().back();
    EXPECT_EQ(lastChangeSetId, lastChangeSet->GetId());
    EXPECT_NE("", lastChangeSet->GetDbGuid());
    EXPECT_GT(lastChangeSet->GetIndex(), 0);
    EXPECT_EQ("", lastChangeSet->GetDescription());
    EXPECT_GT(lastChangeSet->GetFileSize(), 0);
    EXPECT_NE("", lastChangeSet->GetUserCreated());
    EXPECT_GE(lastChangeSet->GetPushDate().GetYear(), 2017);
    EXPECT_GT(lastChangeSet->GetBriefcaseId().GetValue(), 0u);
    EXPECT_EQ(0, lastChangeSet->GetContainingChanges());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetsTests, PullMergeAndPush)
    {
    m_briefcase = AcquireAndOpenBriefcase(true);

    Utf8String originalChangeSetId = m_briefcase->GetDgnDb().Revisions().GetParentRevisionId();

    auto model = CreateTestModel(*m_briefcase);
    CreateElement(*model, DgnCode(), false);

    ChangeSetsResult result = iModelHubHelpers::PullMergeAndPush(m_briefcase, true);
    ASSERT_SUCCESS(result);

    Utf8String lastChangeSetId = m_briefcase->GetDgnDb().Revisions().GetParentRevisionId();
    EXPECT_FALSE(lastChangeSetId.empty());
    EXPECT_NE(originalChangeSetId, lastChangeSetId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetsTests, TwoPulls)
    {
    m_briefcase = AcquireAndOpenBriefcase(false);

    Utf8String originalChangeSetId = m_briefcase->GetDgnDb().Revisions().GetParentRevisionId();

    ChangeSetsResult result = iModelHubHelpers::PullMergeAndPush(m_briefcase, false, true);
    ASSERT_SUCCESS(result);

    Utf8String lastChangeSetId = m_briefcase->GetDgnDb().Revisions().GetParentRevisionId();
    EXPECT_FALSE(lastChangeSetId.empty());
    EXPECT_NE(originalChangeSetId, lastChangeSetId);

    iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info, 1);

    result = iModelHubHelpers::PullMergeAndPush(m_briefcase, false, true);
    ASSERT_SUCCESS(result);

    Utf8String lastChangeSetId2 = m_briefcase->GetDgnDb().Revisions().GetParentRevisionId();
    EXPECT_FALSE(lastChangeSetId2.empty());
    EXPECT_NE(lastChangeSetId2, lastChangeSetId);
    }

BeFileName PreparePreDownloadDir()
    {
    BeFileName preDownloadPath;
    GetPreDownloadDirectory(preDownloadPath);
    EXPECT_TRUE(IsDirEmpty(preDownloadPath));
    return preDownloadPath;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetsTests, PreDownload)
    {
    BeFileName preDownloadPath = PreparePreDownloadDir();
    m_briefcase = AcquireAndOpenBriefcase(false);

    ASSERT_SUCCESS(m_briefcase->GetiModelConnection().GetChangeSetCacheManager().EnableBackgroundDownload()->GetResult());

    iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info, 2);

    // Wait max 50 sec until changeSet files are preDownloaded
    int maxIterations = 100;
    while (GetDirSize(preDownloadPath) < 2 && maxIterations > 0)
        {
        BeThreadUtilities::BeSleep(500);
        maxIterations--;
        }
    EXPECT_FALSE(IsDirEmpty(preDownloadPath));

    // Check if PullAndMerge removes preDownloaded files
    auto result = m_briefcase->PullAndMerge()->GetResult();
    EXPECT_SUCCESS(result);
    EXPECT_TRUE(IsDirEmpty(preDownloadPath));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetsTests, PreDownloadSmallCacheSize)
    {
    BeFileName preDownloadPath = PreparePreDownloadDir();
    m_briefcase = AcquireAndOpenBriefcase(true);

    ASSERT_SUCCESS(m_briefcase->GetiModelConnection().GetChangeSetCacheManager().EnableBackgroundDownload()->GetResult());
    m_briefcase->GetiModelConnection().GetChangeSetCacheManager().SetMaxCacheSize(1);
    iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info, 2);

    // Wait max 20 sec until changeSet files are preDownloaded
    int maxIterations = 40;
    while (maxIterations > 0)
        {
        if (1 == GetDirSize(preDownloadPath))
            break;

        BeThreadUtilities::BeSleep(500);
        maxIterations--;
        }

    EXPECT_EQ(1, GetDirSize(preDownloadPath));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetsTests, PreDownloadTurnedOff)
    {
    BeFileName preDownloadPath = PreparePreDownloadDir();
    m_briefcase = AcquireAndOpenBriefcase(true);

    iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info, 2);

    BeThreadUtilities::BeSleep(1000);
    EXPECT_TRUE(IsDirEmpty(preDownloadPath));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetsTests, PreDownloadManyBriefcases)
    {
    // Acquire 4 briefcases
    bvector<BriefcasePtr> briefcases;
    for (int i = 0; i < 4; i++)
        {
        BriefcaseResult acquireResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, s_info, true);
        ASSERT_SUCCESS(acquireResult);
        BriefcasePtr briefcase = acquireResult.GetValue();
        briefcase->GetiModelConnection().GetChangeSetCacheManager().EnableBackgroundDownload()->GetResult();
        briefcases.push_back(briefcase);
        }

    // Initialize some changeSets
    iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info, 2);

    // Pull all briefcases & check all have the same changeSet
    Utf8String lastChangeSet;
    for (auto it = briefcases.begin(); it != briefcases.end(); ++it)
        {
        auto currentBriefcase = *it;
        EXPECT_SUCCESS(currentBriefcase->PullAndMerge()->GetResult());

        if (Utf8String::IsNullOrEmpty(lastChangeSet.c_str()))
            lastChangeSet = currentBriefcase->GetLastChangeSetPulled();
        else
            EXPECT_EQ(lastChangeSet, currentBriefcase->GetLastChangeSetPulled());
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             08/2017
//---------------------------------------------------------------------------------------
TEST_F(ChangeSetsTests, CancelDownloadChangeSets)
    {
    // Act
    SimpleCancellationTokenPtr cancellationToken = SimpleCancellationToken::Create();
    TestsProgressCallback callback;
    ChangeSetsTaskPtr changeSetsTask = s_connection->DownloadChangeSetsAfterId("", s_db->GetDbGuid(), callback.Get(), cancellationToken);
    changeSetsTask->Execute();
    cancellationToken->SetCanceled();
    BeThreadUtilities::BeSleep(5000);
    changeSetsTask->Wait();

    ChangeSetsResult result = changeSetsTask->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::Canceled, result.GetError().GetId());
    callback.Verify(false);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis           01/2018
//---------------------------------------------------------------------------------------
TEST_F(ChangeSetsTests, DownloadChangeSetFileExpectError)
    {
    m_briefcase = AcquireAndOpenBriefcase(false);
    
    //Try to download empty set of ChangeSets
    bvector<Utf8String> changeSets;
    auto result = m_briefcase->GetiModelConnection().DownloadChangeSets(changeSets)->GetResult();
    auto errId = result.GetError().GetId();
    EXPECT_EQ(Error::Id::QueryIdsNotSpecified, errId);
    }
