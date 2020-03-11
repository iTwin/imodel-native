/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    static BriefcasePtr s_briefcase;

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        iModelTestsBase::SetUpTestCase();
        s_briefcase = iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info, 3);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        if (s_briefcase.IsValid())
            {
            s_briefcase = nullptr;
            }
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

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             10/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    BridgePropertiesPtr CreateBridgeProperties(int changedFilesCount, int usersCount)
        {
        bvector<Utf8String> changedFiles;
        for(int i = 0; i < changedFilesCount; i++)
            changedFiles.push_back(Utf8PrintfString("file%d.bim", i));

        bvector<Utf8String> users;
        for (int i = 0; i < usersCount; i++)
            users.push_back(Utf8PrintfString("user%d", i));

        return BridgeProperties::Create(BeSQLite::BeGuid(true), changedFiles, users);
        }
    
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             10/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AssertStringVectors(bvector<Utf8String> expected, bvector<Utf8String> actual)
        {
        EXPECT_EQ(expected.size(), actual.size());
        for (int i = 0; i < expected.size(); i++)
            EXPECT_EQ(expected.at(i), actual.at(i));
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Algirdas.Mikoliunas             10/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AssertBridgeProperties(BridgePropertiesPtr expected, BridgePropertiesPtr actual)
        {
        EXPECT_EQ(expected->GetJobId(), actual->GetJobId());
        AssertStringVectors(expected->GetUsers(), actual->GetUsers());
        AssertStringVectors(expected->GetChangedFiles(), actual->GetChangedFiles());
        }
    };
BriefcasePtr ChangeSetsTests::s_briefcase = nullptr;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetsTests, PushToEmptyiModel)
    {
    m_imodelName = GetTestiModelName();
    ASSERT_SUCCESS(iModelHubHelpers::DeleteiModelByName(s_client, m_imodelName));
    iModelResult createResult = CreateEmptyiModel(m_imodelName);
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
    m_briefcase->GetiModelConnectionPtr()->SetChangeSetsPageSize(2);

    BoolResult upToDateResult = m_briefcase->IsBriefcaseUpToDate()->GetResult();
    ASSERT_SUCCESS(upToDateResult);
    EXPECT_FALSE(upToDateResult.GetValue());

    Utf8String lastChangeSetId = m_briefcase->GetDgnDb().Revisions().GetParentRevisionId();
    auto missingChangeSetsResult = m_briefcase->GetiModelConnectionPtr()->GetChangeSetsAfterId(lastChangeSetId)->GetResult();
    ASSERT_SUCCESS(missingChangeSetsResult);
    int missingChangeSetsCount = missingChangeSetsResult.GetValue().size();

    ChangeSetsResult result = iModelHubHelpers::PullMergeAndPush(m_briefcase, false, true);
    ASSERT_SUCCESS(result);
    EXPECT_EQ(missingChangeSetsCount, result.GetValue().size());

    lastChangeSetId = m_briefcase->GetDgnDb().Revisions().GetParentRevisionId();
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
    m_briefcase->GetiModelConnectionPtr()->SetChangeSetsPageSize(1);
    Utf8String lastChangeSetId = m_briefcase->GetDgnDb().Revisions().GetParentRevisionId();

    ChangeSetsInfoResult changeSetsResult = m_briefcase->GetiModelConnection().GetAllChangeSets()->GetResult();
    ASSERT_SUCCESS(changeSetsResult);
    EXPECT_EQ(3, changeSetsResult.GetValue().size());

    ChangeSetsResult downloadedChangeSets = m_briefcase->GetiModelConnection().DownloadChangeSetsBetween("", lastChangeSetId)->GetResult();
    ASSERT_SUCCESS(downloadedChangeSets);
    EXPECT_EQ(3, downloadedChangeSets.GetValue().size());

    auto firstChangeSet = *changeSetsResult.GetValue().begin();
    downloadedChangeSets = m_briefcase->GetiModelConnection().DownloadChangeSetsBetween(firstChangeSet->GetId(), lastChangeSetId)->GetResult();
    ASSERT_SUCCESS(downloadedChangeSets);
    EXPECT_EQ(2, downloadedChangeSets.GetValue().size());

    ChangeSetInfoPtr lastChangeSet = changeSetsResult.GetValue().back();
    EXPECT_EQ(lastChangeSetId, lastChangeSet->GetId());
    EXPECT_NE("", lastChangeSet->GetDbGuid());
    EXPECT_GT(lastChangeSet->GetIndex(), 0);
    EXPECT_EQ("", lastChangeSet->GetDescription());
    EXPECT_GT(lastChangeSet->GetFileSize(), 0);
    EXPECT_NE("", lastChangeSet->GetUserCreated());
    EXPECT_GE(lastChangeSet->GetPushDate().GetYear(), 2017);
    EXPECT_GT(lastChangeSet->GetBriefcaseId().GetValue(), 0u);
    EXPECT_EQ(ChangeSetKind::Regular, lastChangeSet->GetContainingChanges());
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
* @bsimethod                                              Algirdas.Mikoliunas   10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetsTests, PushWithBridgeProperties)
    {
    m_briefcase = AcquireAndOpenBriefcase(true);
    auto model = CreateTestModel(*m_briefcase);
    CreateElement(*model, DgnCode(), false);

    ChangeSetKind containingChangesValue = ChangeSetKind::Definition | ChangeSetKind::ViewsAndModels;
    BridgePropertiesPtr expectedBridgeProperties = CreateBridgeProperties(2, 3);
    
    // Push changeSet with bridge properties
    PullChangeSetsArgumentsPtr pullArguments = PullChangeSetsArguments::Create();
    PushChangeSetArgumentsPtr pushArguments = PushChangeSetArguments::Create("ChangeSet with bridge properties", containingChangesValue, expectedBridgeProperties);
    ChangeSetsResult pushResult = iModelHubHelpers::PullMergeAndPush(*m_briefcase, pullArguments, pushArguments);
    ASSERT_SUCCESS(pushResult);

    // Query changeSets with bridge properties
    Utf8String lastChangeSetId = m_briefcase->GetDgnDb().Revisions().GetParentRevisionId();
    ChangeSetQuery changeSetQuery;
    changeSetQuery.SelectBridgeProperties();
    ChangeSetsInfoResult changeSetsResult = m_briefcase->GetiModelConnection().GetChangeSets(changeSetQuery)->GetResult();

    for (auto changeSet : changeSetsResult.GetValue())
        {
        BridgePropertiesPtr actualBridgeProperties = changeSet->GetBridgeProperties();
        if (changeSet->GetId().Equals(lastChangeSetId))
            {
            EXPECT_EQ(containingChangesValue, changeSet->GetContainingChanges());
            AssertBridgeProperties(expectedBridgeProperties, actualBridgeProperties);
            }
        else
            {
            EXPECT_TRUE(actualBridgeProperties.IsNull());
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetsTests, TwoPulls)
    {
    m_briefcase = AcquireAndOpenBriefcase(false);
    m_briefcase->GetiModelConnectionPtr()->SetChangeSetsPageSize(2);
    iModelManager* manager = (iModelManager*)_GetRepositoryManager(m_briefcase->GetDgnDb());
    manager->GetiModelConnectionPtr()->SetChangeSetsPageSize(2);

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
        briefcase->GetiModelConnectionPtr()->SetChangeSetsPageSize(i + 1);
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
    ChangeSetsTaskPtr changeSetsTask = s_connection->DownloadChangeSetsAfterId("", s_briefcase->GetDgnDb().GetDbGuid(), callback.Get(), cancellationToken);
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
