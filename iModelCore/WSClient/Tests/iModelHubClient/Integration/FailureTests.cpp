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

static const wchar_t* s_briefcasePathFileName = L"BreakTestPath.txt";

//---------------------------------------------------------------------------------------
//@bsiclass                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
struct FailureTests : public IntegrationTestsBase
    {
    iModelInfoPtr m_info = nullptr;
    iModelConnectionPtr m_connection = nullptr;

    /*--------------------------------------------------------------------------------------+
    //@bsimethod                                   Algirdas.Mikoliunas             02/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        BeFileName customOutputDir = GetCustomOutputDirectory();
        iModelHubHost& host = iModelHubHost::Instance();
        host.SetCustomOutputDir(customOutputDir);

        IntegrationTestsBase::SetUpTestCase();
        }
    
    /*--------------------------------------------------------------------------------------+
    //@bsimethod                                   Algirdas.Mikoliunas             02/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        iModelTestsBase::TearDownTestCase();

        iModelHubHost& host = iModelHubHost::Instance();
        host.SetCustomOutputDir(BeFileName());
        }
    
    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    virtual void SetUp() override
        {
        m_imodelName = GetFailureTestiModelName();
        IntegrationTestsBase::SetUp();
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             02/2018
    //---------------------------------------------------------------------------------------
    static BeFileName GetCustomOutputDirectory()
        {
        BeFileName outputDir;
        BeTest::GetHost().GetOutputRoot(outputDir);
        outputDir.AppendToPath(L"../FailureTests");

        if (!BeFileName::DoesPathExist(outputDir))
            BeFileName::CreateNewDirectory(outputDir);

        return outputDir;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static Utf8String GetFailureTestiModelName()
        {
        return Utf8PrintfString("Failure-%s", GetTestInfo().name());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void AcquireAndOpenBriefcase(BriefcasePtr& briefcase, iModelInfoPtr info, bool pull)
        {
        BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, info, pull);
        ASSERT_SUCCESS(briefcaseResult);
        briefcase = briefcaseResult.GetValue();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    BriefcasePtr AcquireAndOpenBriefcase()
        {
        BriefcasePtr briefcase;
        AcquireAndOpenBriefcase(briefcase, m_info, true);
        return briefcase;
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void InitializeContinueTestCase()
        {
        iModelHubHelpers::DeleteiModelByName(s_client, m_imodelName);
        
        DgnDbPtr db = CreateTestDb(m_imodelName);
        iModelResult createResult = CreateiModel(db);
        ASSERT_SUCCESS(createResult);
        m_info = createResult.GetValue();

        m_connection = CreateiModelConnection(m_info);
        SaveString(m_info->GetName());
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
        command.Sprintf("%s\\iModelHubNativeTests.exe --gtest_catch_exceptions=0 --gtest_filter=CrashTests.%s", path.c_str(), crashTestCaseName.c_str());
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
    static void SaveBriefcasePath(BeFileName briefcasePath)
        {
        SaveString(briefcasePath.GetNameUtf8());
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    Utf8String ReadString()
        {
        BeFileName path = OutputDir().AppendToPath(s_briefcasePathFileName);

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
    static void SaveString(Utf8String savedString)
        {
        BeFileName path = OutputDir().AppendToPath(s_briefcasePathFileName);

        BeFile file;
        if (BeFileStatus::Success != file.Create(path.c_str(), true))
            return;
        file.Close();

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
    void ContinueAfteriModelCrash(Utf8String crashTestCaseName)
        {
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

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void ContinueAfterRecoverBriefcaseCrash(Utf8String crashTestCaseName)
        {
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
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, AfterDownloadChangeSets)
    {
    ContinueAfterCrashPullMergeAndPush("AfterDownloadChangeSets");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, BeforeStartCreateChangeSet)
    {
    ContinueAfterCrashPullMergeAndPush("BeforeStartCreateChangeSet");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, AfterStartCreateChangeSet)
    {
    ContinueAfterCrashPullMergeAndPush("AfterStartCreateChangeSet");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, BeforePushChangeSetToServer)
    {
    ContinueAfterCrashPullMergeAndPush("BeforePushChangeSetToServer");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, AfterPushChangeSetToServer)
    {
    ContinueAfterCrashPullMergeAndPush("AfterPushChangeSetToServer");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, AfterFinishCreateChangeSet)
    {
    ContinueAfterCrashPullMergeAndPush("AfterFinishCreateChangeSet");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, AfterCreateRequest)
    {
    ContinueAfteriModelCrash("AfterCreateRequest");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, AfterDownloadBriefcaseFile)
    {
    ContinueAfterRecoverBriefcaseCrash("AfterDownloadBriefcaseFile");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, AfterDeleteBriefcase)
    {
    ContinueAfterRecoverBriefcaseCrash("AfterDeleteBriefcase");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, AfterOpenBriefcaseForMerge)
    {
    ContinueAfterAcquireBriefcaseCrash("AfterOpenBriefcaseForMerge");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, AfterMergeChangeSets)
    {
    ContinueAfterAcquireBriefcaseCrash("AfterMergeChangeSets");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, AfterCreateBriefcaseInstance)
    {
    ContinueAfterAcquireBriefcaseCrash("AfterCreateBriefcaseInstance");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, AfterWriteiModelInfo)
    {
    ContinueAfterRecoverBriefcaseCrash("AfterWriteiModelInfo");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, AfterCreateChangeSetRequest)
    {
    ContinueAfterCrashPullMergeAndPush("AfterCreateChangeSetRequest");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(FailureTests, AfterUploadChangeSetFile)
    {
    ContinueAfterCrashPullMergeAndPush("AfterUploadChangeSetFile");
    }

//---------------------------------------------------------------------------------------
//@bsiclass                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
struct CrashTests : public IntegrationTestsBase
    {
    iModelInfoPtr m_info = nullptr;

    /*--------------------------------------------------------------------------------------+
    //@bsimethod                                   Algirdas.Mikoliunas             02/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        BeFileName customOutputDir = FailureTests::GetCustomOutputDirectory();
        iModelHubHost& host = iModelHubHost::Instance();
        host.SetCustomOutputDir(customOutputDir);

        IntegrationTestsBase::SetUpTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    //@bsimethod                                   Algirdas.Mikoliunas             02/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        iModelTestsBase::TearDownTestCase();

        iModelHubHost& host = iModelHubHost::Instance();
        host.SetCustomOutputDir(BeFileName());
        }
    
    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void InitializeCrashTestCase()
        {
        auto iModelName = FailureTests::GetFailureTestiModelName();
        auto iModelInfoResult = s_client->GetiModelByName(s_projectId, iModelName)->GetResult();
        if (iModelInfoResult.IsSuccess())
            m_info = iModelInfoResult.GetValue();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    BriefcasePtr AcquireAndOpenBriefcase()
        {
        BriefcasePtr briefcase;
        FailureTests::AcquireAndOpenBriefcase(briefcase, m_info, true);
        return briefcase;
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
        FailureTests::SaveBriefcasePath(briefcase1->GetDgnDb().GetFileName());

        // Break PullMergeAndPush
        BreakHelper::SetBreakpoint(breakpoint);
        auto pushResult = briefcase1->PullMergeAndPush(nullptr, false)->GetResult();
        }
    
    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void CrashiModelCreateTestCase(Breakpoints breakpoint)
        {
        InitializeCrashTestCase();

        //Create the seed file
        auto imodelName = FailureTests::GetFailureTestiModelName();
        iModelHubHelpers::DeleteiModelByName(s_client, imodelName);
        DgnDbPtr db = CreateTestDb(imodelName);
        EXPECT_TRUE(db.IsValid());

        FailureTests::SaveBriefcasePath(db->GetFileName());
        BreakHelper::SetBreakpoint(breakpoint);

        //Upload the seed file to the server
        auto createResult = s_client->CreateNewiModel(s_projectId, *db)->GetResult();
        }
    
    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    void CrashRecoverBriefcase(Breakpoints breakpoint)
        {
        InitializeCrashTestCase();

        // Create imodel and acquire a briefcase
        auto imodelName = FailureTests::GetFailureTestiModelName();
        iModelHubHelpers::DeleteiModelByName(s_client, imodelName);

        auto db = CreateTestDb(imodelName);
        iModelResult createResult = CreateiModel(db);
        ASSERT_SUCCESS(createResult);
        m_info = createResult.GetValue();

        auto imodelConnection = CreateiModelConnection(m_info);
        auto briefcase = AcquireAndOpenBriefcase();

        // Open briefcase should fail without refresh
        auto briefcaseName = briefcase->GetDgnDb().GetFileName();
        briefcase->GetDgnDb().CloseDb();
        auto db2 = DgnDb::OpenDgnDb(nullptr, briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));

        FailureTests::SaveBriefcasePath(db2->GetFileName());
        BreakHelper::SetBreakpoint(breakpoint);

        // Refresh briefcase
        auto refreshResult = s_client->RecoverBriefcase(db2, false)->GetResult();
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
    };

//---------------------------------------------------------------------------------------
// Following test cases are used by other test cases to imitate crashed application
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, AfterDownloadChangeSets)
    {
    CrashPullMergeAndPush(Breakpoints::AfterDownloadChangeSets);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, BeforeStartCreateChangeSet)
    {
    CrashPullMergeAndPush(Breakpoints::BeforeStartCreateChangeSet);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, AfterStartCreateChangeSet)
    {
    CrashPullMergeAndPush(Breakpoints::AfterStartCreateChangeSet);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, BeforePushChangeSetToServer)
    {
    CrashPullMergeAndPush(Breakpoints::BeforePushChangeSetToServer);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, AfterPushChangeSetToServer)
    {
    CrashPullMergeAndPush(Breakpoints::AfterPushChangeSetToServer);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, AfterFinishCreateChangeSet)
    {
    CrashPullMergeAndPush(Breakpoints::AfterFinishCreateChangeSet);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, AfterCreateRequest)
    {
    CrashiModelCreateTestCase(Breakpoints::Client_AfterCreateRequest);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, AfterDownloadBriefcaseFile)
    {
    CrashRecoverBriefcase(Breakpoints::Client_AfterDownloadBriefcaseFile);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, AfterDeleteBriefcase)
    {
    CrashRecoverBriefcase(Breakpoints::Client_AfterDeleteBriefcase);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, AfterOpenBriefcaseForMerge)
    {
    CrashAcquireBriefcase(Breakpoints::Client_AfterOpenBriefcaseForMerge);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, AfterMergeChangeSets)
    {
    CrashAcquireBriefcase(Breakpoints::Client_AfterMergeChangeSets);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, AfterCreateBriefcaseInstance)
    {
    CrashAcquireBriefcase(Breakpoints::Client_AfterCreateBriefcaseInstance);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, AfterWriteiModelInfo)
    {
    CrashRecoverBriefcase(Breakpoints::iModelConnection_AfterWriteiModelInfo);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, AfterCreateChangeSetRequest)
    {
    CrashPullMergeAndPush(Breakpoints::iModelConnection_AfterCreateChangeSetRequest);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(CrashTests, AfterUploadChangeSetFile)
    {
    CrashPullMergeAndPush(Breakpoints::iModelConnection_AfterUploadChangeSetFile);
    }

#endif
