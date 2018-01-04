/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/FailureTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"
#include "Helpers.h"
#include <WebServices/iModelHub/Client/Client.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTest.h>
#include "Windows.h"

#if defined (ENABLE_IMODELHUB_CRASH_TESTS)
#include <WebServices/iModelHub/Client/BreakHelper.h>
#endif

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

#if defined (ENABLE_IMODELHUB_CRASH_TESTS)
//---------------------------------------------------------------------------------------
//@bsiclass                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
struct FailureTests : public iModelTestsBase
    {
    iModelInfoPtr m_info = nullptr;
    iModelConnectionPtr m_connection = nullptr;
    const wchar_t*  m_briefcasePathFileName = L"BreakTestPath.txt";
    
    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    virtual void SetUp() override
        {
        IntegrationTestsBase::SetUp();
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    virtual void TearDown() override
        {
        if (m_info.IsValid())
            {
            m_info = nullptr;
            }
        if (m_connection.IsValid())
            {
            m_connection = nullptr;
            }
        IntegrationTestsBase::TearDown();
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void InitializeContinueTestCase()
        {
        DgnDbPtr db = CreateTestDb(TestCaseiModelName());
        iModelResult createResult = CreateiModel(db);
        ASSERT_SUCCESS(createResult);
        m_info = createResult.GetValue();

        m_connection = CreateiModelConnection(m_info);
        SaveString(m_info->GetName());
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void InitializeCrashTestCase()
        {
        auto imodelsIterator = s_client->GetiModels(s_projectId)->GetResult().GetValue();
        auto imodelName = ReadString();
        for (auto imodelIterator = imodelsIterator.begin(); imodelIterator != imodelsIterator.end(); ++imodelIterator)
            {
            if ((*imodelIterator)->GetName().Equals(imodelName))
                m_info = *imodelIterator;
            }
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    Utf8String GetExePath() {
        wchar_t buffer[MAX_PATH];
        GetModuleFileName(NULL, buffer, MAX_PATH);
        std::string::size_type pos = Utf8String(buffer).find_last_of("\\/");
        return Utf8String(buffer).substr(0, pos);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void RunCrashTestCase(Utf8String crashTestCaseName)
        {
        auto path = GetExePath();
        Utf8String command;
        command.Sprintf("%s\\iModelHubNativeTests.exe --gtest_catch_exceptions=0 --gtest_filter=FailureTests.%s", path.c_str(), crashTestCaseName.c_str());
        int result = system(command.c_str());
        EXPECT_TRUE(1 == result || 3 == result);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    BeFileName ReadBriefcaseName()
        {
        return BeFileName(ReadString());
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void SaveBriefcasePath(BeFileName briefcasePath)
        {
        SaveString(briefcasePath.GetNameUtf8());
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    Utf8String ReadString()
        {
        BeFileName path = OutputDir().AppendToPath(m_briefcasePathFileName);

        BeFile file;
        if (BeFileStatus::Success != file.Open(path.c_str(), BeFileAccess::Read))
            return "";

        ByteStream byteStream;
        if (BeFileStatus::Success != file.ReadEntireFile(byteStream))
            return "";

        Utf8String contents((Utf8CP)byteStream.GetData(), byteStream.GetSize());
        file.Close();

        return contents;
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void SaveString(Utf8String savedString)
        {
        BeFileName path = OutputDir().AppendToPath(m_briefcasePathFileName);

        BeFile file;
        if (BeFileStatus::Success != file.Create(path.c_str(), true))
            return;

        if (BeFileStatus::Success != file.Open(path.c_str(), BeFileAccess::Write))
            return;

        uint32_t nWritten;
        if (BeFileStatus::Success != file.Write(&nWritten, savedString.c_str(), (uint32_t)savedString.SizeInBytes()))
            return;

        file.Close();
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void CrashPullMergeAndPush(Breakpoints breakpoint)
        {
        InitializeCrashTestCase();

        //Prapare imodel and acquire briefcases
        auto briefcase1 = AcquireAndOpenBriefcase();

        //Create model in briefcase 1. This should also acquire locks automatically.
        auto model1 = CreateModel("Model1", briefcase1->GetDgnDb());
        briefcase1->GetDgnDb().SaveChanges();
        SaveBriefcasePath(briefcase1->GetDgnDb().GetFileName());

        // Break PullMergeAndPush
        BreakHelper::SetBreakpoint(breakpoint);
        auto pushResult = briefcase1->PullMergeAndPush(nullptr, false)->GetResult();
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void ContinueAfterCrashPullMergeAndPush(Utf8String crashTestCaseName)
        {
        InitializeContinueTestCase();

        // Run test that crashes
        RunCrashTestCase(crashTestCaseName);

        // Open crashed briefcase
        BeFileName filename = ReadBriefcaseName();
        auto db2 = DgnDb::OpenDgnDb(nullptr, filename, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        auto reloadedBriefcase = s_client->OpenBriefcase(db2)->GetResult().GetValue();
        DgnDbR reloadedDb = reloadedBriefcase->GetDgnDb();
        auto reloadedChangeSetId = reloadedBriefcase->GetLastChangeSetPulled();

        // Create one more model
        CreateModel("Model3", reloadedDb);
        reloadedDb.SaveChanges();

        // Push changes
        auto pushResult = reloadedBriefcase->PullMergeAndPush(nullptr, false)->GetResult();
        EXPECT_SUCCESS(pushResult);

        DgnDbR db = reloadedBriefcase->GetDgnDb();
        ExpectCodeState(CreateCodeUsed(MakeModelCode("Model1", db), reloadedChangeSetId), _GetRepositoryManager(db));
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void CrashiModelCreateTestCase(Breakpoints breakpoint)
        {
        InitializeCrashTestCase();

        //Create the seed file
        DgnDbPtr db = CreateTestDb("");
        EXPECT_TRUE(db.IsValid());

        SaveBriefcasePath(db->GetFileName());
        BreakHelper::SetBreakpoint(breakpoint);

        //Upload the seed file to the server
        auto createResult = s_client->CreateNewiModel(s_projectId, *db)->GetResult();
        }
    
    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void ContinueAfteriModelCrash(Utf8String crashTestCaseName)
        {
        InitializeContinueTestCase();

        // Run test that crashes
        RunCrashTestCase(crashTestCaseName);

        // Open dgndb
        BeFileName filename = ReadBriefcaseName();
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, filename, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        EXPECT_TRUE(db.IsValid());

        auto createResult = s_client->CreateNewiModel(s_projectId, *db)->GetResult();
        EXPECT_SUCCESS(createResult);
        }
    
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static FileInfoPtr CreateFileInfo(DgnDbR db)
        {
        return FileInfo::Create(db, ::testing::UnitTest::GetInstance()->current_test_info()->name());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ReplaceSeedFile(iModelConnectionCR connection)
        {
        DgnDbPtr db = CreateTestDb(GetTestiModelName().append("2"));
        FileInfoPtr fileInfo = CreateFileInfo(db);
        FileResult uploadResult = connection.UploadNewSeedFile(db->GetFileName(), *fileInfo, true)->GetResult();
        ASSERT_SUCCESS(uploadResult);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void CrashRecoverBriefcase(Breakpoints breakpoint)
        {
        InitializeCrashTestCase();

        // Create imodel and acquire a briefcase
        m_imodelName = GetTestiModelName().append("2");
        auto db = CreateTestDb(m_imodelName);
        iModelResult createResult = CreateiModel(db);
        ASSERT_SUCCESS(createResult);
        iModelInfoPtr imodelInfo = createResult.GetValue();

        auto imodelConnection = CreateiModelConnection(imodelInfo);
        auto briefcase = AcquireAndOpenBriefcase();

        // ReplaceSeedFile
        ReplaceSeedFile(imodelConnection);

        // Open briefcase should fail without refresh
        auto briefcaseName = briefcase->GetDgnDb().GetFileName();
        briefcase->GetDgnDb().CloseDb();
        auto db2 = DgnDb::OpenDgnDb(nullptr, briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));

        SaveBriefcasePath(db2->GetFileName());
        BreakHelper::SetBreakpoint(breakpoint);

        // Refresh briefcase
        auto refreshResult = s_client->RecoverBriefcase(db2, false)->GetResult();
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void ContinueAfterRecoverBriefcaseCrash(Utf8String crashTestCaseName)
        {
        InitializeContinueTestCase();

        // Run test that crashes
        RunCrashTestCase(crashTestCaseName);

        // Open dgndb
        BeFileName filename = ReadBriefcaseName();
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, filename, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        EXPECT_TRUE(db.IsValid());

        // Recover briefcase
        auto refreshResult = s_client->RecoverBriefcase(db, false)->GetResult();
        EXPECT_SUCCESS(refreshResult);

        // Open briefcase should now succeed
        auto db3 = DgnDb::OpenDgnDb(nullptr, filename, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        auto openResult = s_client->OpenBriefcase(db3)->GetResult();
        EXPECT_SUCCESS(openResult);

        // Create one more model
        CreateModel("Model3", *db3);
        db3->SaveChanges();

        // Push changes
        auto pushResult = openResult.GetValue()->PullMergeAndPush(nullptr, false)->GetResult();
        EXPECT_SUCCESS(pushResult);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void CrashAcquireBriefcase(Breakpoints breakpoint)
        {
        InitializeCrashTestCase();

        BreakHelper::SetBreakpoint(breakpoint);
        auto briefcase1 = AcquireAndOpenBriefcase();
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void ContinueAfterAcquireBriefcaseCrash(Utf8String crashTestCaseName)
        {
        InitializeContinueTestCase();

        // Run test that crashes
        RunCrashTestCase(crashTestCaseName);

        auto briefcase1 = AcquireAndOpenBriefcase();
        DgnDbR db = briefcase1->GetDgnDb();

        // Create one more model
        CreateModel("Model3", db);
        db.SaveChanges();

        // Push changes
        auto pushResult = briefcase1->PullMergeAndPush(nullptr, false)->GetResult();
        EXPECT_SUCCESS(pushResult);
        }
    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void CrashSeedFileReplace(Breakpoints breakpoint)
        {
        InitializeCrashTestCase();

        auto imodelConnection = CreateiModelConnection(m_info);
        auto briefcase = AcquireAndOpenBriefcase();
        DgnDbR db = briefcase->GetDgnDb();

        SaveBriefcasePath(db.GetFileName());
        BreakHelper::SetBreakpoint(breakpoint);

        // ReplaceSeedFile
        ReplaceSeedFile(imodelConnection);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void ContinueAfterCrashSeedFileReplace(Utf8String crashTestCaseName)
        {
        InitializeContinueTestCase();

        // Run test that crashes
        RunCrashTestCase(crashTestCaseName);

        // Open dgndb
        BeFileName filename = ReadBriefcaseName();
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, filename, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        EXPECT_TRUE(db.IsValid());
        
        // ReplaceSeedFile
        auto connectionResult = s_client->ConnectToiModel(s_projectId, m_info->GetId())->GetResult();
        EXPECT_SUCCESS(connectionResult);
        auto imodelConnection = connectionResult.GetValue();
        ReplaceSeedFile(imodelConnection);
        }
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueExceptionAfterDownloadChangeSets)
    {
    ContinueAfterCrashPullMergeAndPush("CrashAfterDownloadChangeSets");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueExceptionBeforeStartCreateChangeSet)
    {
    ContinueAfterCrashPullMergeAndPush("CrashBeforeStartCreateChangeSet");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueExceptionAfterStartCreateChangeSet)
    {
    ContinueAfterCrashPullMergeAndPush("CrashAfterStartCreateChangeSet");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueExceptionBeforePushChangeSetToServer)
    {
    ContinueAfterCrashPullMergeAndPush("CrashBeforePushChangeSetToServer");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueExceptionAfterPushChangeSetToServer)
    {
    ContinueAfterCrashPullMergeAndPush("CrashAfterPushChangeSetToServer");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueExceptionAfterFinishCreateChangeSet)
    {
    ContinueAfterCrashPullMergeAndPush("CrashAfterFinishCreateChangeSet");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueClient_AfterCreateRequest)
    {
    ContinueAfteriModelCrash("CrashClient_AfterCreateRequest");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueClient_AfterDownloadBriefcaseFile)
    {
    ContinueAfterRecoverBriefcaseCrash("CrashClient_AfterDownloadBriefcaseFile");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueClient_AfterDeleteBriefcase)
    {
    ContinueAfterRecoverBriefcaseCrash("CrashClient_AfterDeleteBriefcase");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueClient_AfterOpenBriefcaseForMerge)
    {
    ContinueAfterAcquireBriefcaseCrash("CrashClient_AfterOpenBriefcaseForMerge");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueClient_AfterMergeChangeSets)
    {
    ContinueAfterAcquireBriefcaseCrash("CrashClient_AfterMergeChangeSets");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueClient_AfterCreateBriefcaseInstance)
    {
    ContinueAfterAcquireBriefcaseCrash("CrashClient_AfterCreateBriefcaseInstance");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueiModelConnection_AfterCreateNewServerFile)
    {
    ContinueAfterCrashSeedFileReplace("CrashiModelConnection_AfterCreateNewServerFile");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueiModelConnection_AfterUploadServerFile)
    {
    ContinueAfterCrashSeedFileReplace("CrashiModelConnection_AfterUploadServerFile");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueiModelConnection_AfterWriteiModelInfo)
    {
    ContinueAfterRecoverBriefcaseCrash("CrashiModelConnection_AfterWriteiModelInfo");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueiModelConnection_AfterCreateChangeSetRequest)
    {
    ContinueAfterCrashPullMergeAndPush("CrashiModelConnection_AfterCreateChangeSetRequest");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, ContinueiModelConnection_AfterUploadChangeSetFile)
    {
    ContinueAfterCrashPullMergeAndPush("CrashiModelConnection_AfterUploadChangeSetFile");
    }

//---------------------------------------------------------------------------------------
// Following test cases are used by other test cases to imitate crashed application
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashAfterDownloadChangeSets)
    {
    CrashPullMergeAndPush(Breakpoints::AfterDownloadChangeSets);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashBeforeStartCreateChangeSet)
    {
    CrashPullMergeAndPush(Breakpoints::BeforeStartCreateChangeSet);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashAfterStartCreateChangeSet)
    {
    CrashPullMergeAndPush(Breakpoints::AfterStartCreateChangeSet);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashBeforePushChangeSetToServer)
    {
    CrashPullMergeAndPush(Breakpoints::BeforePushChangeSetToServer);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashAfterPushChangeSetToServer)
    {
    CrashPullMergeAndPush(Breakpoints::AfterPushChangeSetToServer);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashAfterFinishCreateChangeSet)
    {
    CrashPullMergeAndPush(Breakpoints::AfterFinishCreateChangeSet);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashClient_AfterCreateRequest)
    {
    CrashiModelCreateTestCase(Breakpoints::Client_AfterCreateRequest);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashClient_AfterDownloadBriefcaseFile)
    {
    CrashRecoverBriefcase(Breakpoints::Client_AfterDownloadBriefcaseFile);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashClient_AfterDeleteBriefcase)
    {
    CrashRecoverBriefcase(Breakpoints::Client_AfterDeleteBriefcase);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashClient_AfterOpenBriefcaseForMerge)
    {
    CrashAcquireBriefcase(Breakpoints::Client_AfterOpenBriefcaseForMerge);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashClient_AfterMergeChangeSets)
    {
    CrashAcquireBriefcase(Breakpoints::Client_AfterMergeChangeSets);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashClient_AfterCreateBriefcaseInstance)
    {
    CrashAcquireBriefcase(Breakpoints::Client_AfterCreateBriefcaseInstance);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashiModelConnection_AfterCreateNewServerFile)
    {
    CrashSeedFileReplace(Breakpoints::iModelConnection_AfterCreateNewServerFile);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashiModelConnection_AfterUploadServerFile)
    {
    CrashSeedFileReplace(Breakpoints::iModelConnection_AfterUploadServerFile);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashiModelConnection_AfterWriteiModelInfo)
    {
    CrashRecoverBriefcase(Breakpoints::iModelConnection_AfterWriteiModelInfo);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashiModelConnection_AfterCreateChangeSetRequest)
    {
    CrashPullMergeAndPush(Breakpoints::iModelConnection_AfterCreateChangeSetRequest);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, CrashiModelConnection_AfterUploadChangeSetFile)
    {
    CrashPullMergeAndPush(Breakpoints::iModelConnection_AfterUploadChangeSetFile);
    }

#endif
