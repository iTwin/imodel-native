/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "MstnBridgeTestsFixture.h"
#include <iModelBridge/iModelBridgeFwk.h>
#include <iModelBridge/iModelBridgeSyncInfoFile.h>
#include <iModelBridge/FakeRegistry.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#include <Bentley/Desktop/FileSystem.h>
#include <Bentley/BeDirectoryIterator.h>

PUSH_DISABLE_DEPRECATION_WARNINGS
extern "C"
    {
    EXPORT_ATTRIBUTE T_iModelBridge_getAffinity iModelBridge_getAffinity;
    }

USING_NAMESPACE_BENTLEY_DGN

#define MSTN_BRIDGE_REG_SUB_KEY L"iModelBridgeForMstn"
#define MSTN_BRIDGE_REG_SUB_KEY_A "iModelBridgeForMstn"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value readJsonFromFile(BentleyApi::BeFileNameCR fileName)
    {
    BentleyApi::BeFile errfile;
    if (BentleyApi::BeFileStatus::Success != errfile.Open(fileName.c_str(), BentleyApi::BeFileAccess::Read))
        return Json::nullValue;

    BentleyApi::bvector<Byte> bytes;
    if (BentleyApi::BeFileStatus::Success != errfile.ReadEntireFile(bytes))
        return Json::nullValue;

    if (bytes.empty())
        return Json::objectValue;

    bytes.push_back('\0'); // End of stream

    Utf8String contents;

    const Byte utf8BOM[] = {0xef, 0xbb, 0xbf};
    if (bytes[0] == utf8BOM[0] || bytes[1] == utf8BOM[1] || bytes[2] == utf8BOM[2])
        contents.assign((Utf8CP) (bytes.data() + 3));
    else
        contents.assign((Utf8CP) bytes.data());

    Json::Value json(Json::objectValue);
    Json::Reader::Parse(contents.c_str(), json);
    return json;
    }

//=======================================================================================
// @bsistruct
//=======================================================================================
struct MstnBridgeTests : public MstnBridgeTestsFixture
    {
    void SetupTwoRefs(bvector<WString>& args, BentleyApi::BeFileName& masterFile, BentleyApi::BeFileName& refFile1,
                      BentleyApi::BeFileName& refFile2, BentleyApi::BeFileName const& testDir, FakeRegistry& testRegistry);
    void VerifyCodeIsRegistered(DbFileInfo&, DgnCodeCR, Utf8CP codeValuePrefix, bool codeShouldBeRecorded);
    void VerifyElementHasCode(DgnElementCR, Utf8CP codeValuePrefix);
    void VerifyElementHasCode(BeFileNameCR bcName, DgnElementId eid, Utf8CP codeValuePrefix, bool codeShouldBeRecorded);
    void VerifyConvertedElementHasCode(int prevCount, BeFileNameCR bcName, int64_t srcId, Utf8CP codeValuePrefix, bool codeShouldBeRecorded);
    void RunBridgeAsUser(BeFileNameR briefcaseName, BeFileNameCR testDir, Utf8CP userName, BeFileNameCR inputFile, WCharCP bridgeName = L"theBridge");
    void DoMoveEmbeddedReferenceToDifferentFile(bool simulateOldBridge);
    void DoDetectDeletionsInEmbeddedFiles(bool simulateOldBridge);
    void DoAddRemoveNestedEmbeddedRefs(bool simulateOldBridge);
    void DoAddRemoveNestedEmbeddedRefsV2(bool simulateOldBridge);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, TestDenySchemaLock)
    {
    if (nullptr == GetIModelBankServerJs())
        {
        fprintf(stderr, "Lock tests are supported only when using imodel-bank as the server\n.");
        return;
        }

    auto testDir = CreateTestDir();

    SetRulesFileInEnv setRulesFileVar(WriteRulesFile(L"testDenySchemaLock.json",    // Specify user1's permissions:
          R"(       [
                        {
                            "user": "user1",
                            "rules": [
                                {
                                    "request": "Lock/Create",
                                    "rule": { "verb": "deny" }
                                }
                            ]
                        }
                    ]
            )"));

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);

    SetupClient(ComputeAccessToken("user1"));
    CreateRepository();
    {
    auto runningServer = StartServer();


    FwkArgvMaker argvMaker;
    argvMaker.SetUpBridgeProcessingArgs(testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, GetDgnv8BridgeDllName(), DEFAULT_IMODEL_NAME, true, L"imodel-bank.rsp");
    argvMaker.ReplaceArgValue(L"--imodel-bank-access-token", ComputeAccessTokenW("user1").c_str());

    argvMaker.SetInputFileArg(inputFile);
    argvMaker.SetSkipAssignmentCheck();

    iModelBridgeFwk fwk;

    ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argvMaker.GetArgC(), argvMaker.GetArgV()));

    bool fatalFwkMsgFound = false;
    bool lockDeniedHttpMsgFound = false;
    LogProcessor processLog(
        [] (BentleyApi::Utf8StringCR ns, BentleyApi::NativeLogging::SEVERITY sev) {
            return ns.Equals("WSClient") || ns.Equals("iModelBridge");
        },
        [&] (BentleyApi::Utf8StringCR ns, BentleyApi::NativeLogging::SEVERITY sev, BentleyApi::Utf8StringCR msg) {
            if ((BentleyApi::NativeLogging::SEVERITY::LOG_FATAL == sev) && ns.Equals("iModelBridge") && msg.ContainsI("schema"))
                fatalFwkMsgFound = true;
            else if ((BentleyApi::NativeLogging::SEVERITY::LOG_INFO == sev) && ns.Equals("WSClient") && msg.ContainsI(R"({"errorId":"iModelHub.UserDoesNotHavePermission","errorMessage":"user: user1, request: Lock/Create"})"))
                lockDeniedHttpMsgFound = true;
        }
    );

    ASSERT_NE(0, fwk.Run(argvMaker.GetArgC(), argvMaker.GetArgV())) << "Expect bridge to fail";

    ASSERT_TRUE(fatalFwkMsgFound);
    ASSERT_TRUE(lockDeniedHttpMsgFound);
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void createFile(BentleyApi::BeFileName const& fn)
    {
    FILE* fp = _wfopen(fn.c_str(), L"w+");
    fputs("go", fp);
    fclose(fp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, MultiBridgeSequencing)
    {
    if (nullptr == GetIModelBankServerJs())
        {
        fprintf(stderr, "Lock tests are supported only when using imodel-bank as the server\n.");
        return;
        }

    auto testDir = CreateTestDir();

    BeFileName a_can_run(testDir);
    a_can_run.AppendToPath(L"a_can_run.txt");

    BeFileName b_can_run(testDir);
    b_can_run.AppendToPath(L"b_can_run.txt");

    Utf8PrintfString rules(
        R"(   [
                  {
                      "user": "A",
                      "rules": [
                          {
                              "request": "Lock/Create",
                              "rule": { "verb": "wait file", "object": "%s", "retryCount": 0 }
                          }
                      ]
                  },
                  {
                      "user": "B",
                      "rules": [
                          {
                              "request": "Lock/Create",
                              "rule": { "verb": "wait file", "object": "%s", "retryCount": 0 }
                          }
                      ]
                  }
              ]
          )", Utf8String(a_can_run).c_str(), Utf8String(b_can_run).c_str());
    rules.ReplaceAll("\\", "/");

    SetRulesFileInEnv setRulesFileVar(WriteRulesFile(L"testDenySchemaLock.json", rules.c_str()));


    // -------------------------------------------------------
    // Create the iModel directory and start the server
    // -------------------------------------------------------
    CreateImodelBankRepository(GetSeedFile());

    bool bridge_a_sawRetryMsg = false;
    BentleyApi::BeFileName bridge_a_briefcaseName;
    BentleyApi::BeFileName bridge_b_briefcaseName;
    {
    std::unique_ptr<BentleyApi::Dgn::IModelBankClient> bankClient(CreateIModelBankClient(""));
    { // make sure server is stopped before releasing bankClient, as that is used by runningServer's Stop method.
    auto runningServer = StartImodelBankServer(GetIModelDir(), *bankClient);

    ASSERT_TRUE(m_client == nullptr);
    iModelBridgeFwk::ClearIModelClientForBridgesForTesting(); // nobody should have set the test client, but clear it just in case.

    // Must do an initial conversion, just to get the domains schemas in (plus any profile and/or BIS core upgrades).
    // That cannot be done safely by two users concurrently.
    printf ("********************\n");
    printf ("******************** Running Admin\n");
    printf ("********************\n");

    BentleyApi::BeFileName adminInputFile;
    MakeCopyOfFile(adminInputFile, L"Test3d.dgn", L"_admin");

    BeFileName adminBriefcaseName;
    RunBridgeAsUser(adminBriefcaseName, testDir, "admin", adminInputFile, L"anyBridge");

    printf ("********************\n");
    printf ("******************** Running A and B\n");
    printf ("********************\n");

    // -------------------------------------------------------
    // Bridge A
    // -------------------------------------------------------
    std::thread bridge_a([&]
        {
        BeFileName aDir(testDir);
        aDir.AppendToPath(L"A");
        BeFileName::CreateNewDirectory(aDir.c_str());

        FwkArgvMaker argvMaker;
        argvMaker.SetUpBridgeProcessingArgs(aDir.c_str(), L"bridge_a", GetDgnv8BridgeDllName(), DEFAULT_IMODEL_NAME, true, L"imodel-bank.rsp");
        argvMaker.ReplaceArgValue(L"--imodel-bank-access-token", ComputeAccessTokenW("A").c_str());

        BentleyApi::BeFileName inputFile;
        MakeCopyOfFile(inputFile, L"Test3d.dgn", L"_A");

        argvMaker.SetInputFileArg(inputFile);
        argvMaker.SetSkipAssignmentCheck();

        argvMaker.SetMaxRetries(255, true);   // must allow lots of time for bridge B to run to completion. Unfortunately, there is no way to predict how many retries will be required.

        iModelBridgeFwk fwk;
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argvMaker.GetArgC(), argvMaker.GetArgV()));

        LogProcessor processLog(
            [] (BentleyApi::Utf8StringCR ns, BentleyApi::NativeLogging::SEVERITY sev) {
                return ns.StartsWith("iModelBridge") && (BentleyApi::NativeLogging::SEVERITY::LOG_INFO == sev);
            },
            [&] (BentleyApi::Utf8StringCR ns, BentleyApi::NativeLogging::SEVERITY sev, BentleyApi::Utf8StringCR msg) {
                if (msg.ContainsI("retrying"))
                    bridge_a_sawRetryMsg = true;
            }
        );

        ASSERT_EQ(0, fwk.Run(argvMaker.GetArgC(), argvMaker.GetArgV()));

        bridge_a_briefcaseName = fwk.GetBriefcaseName();
        });

    // (Note: bridge a should be blocked and not making progress, because we have not yet created the "a_can_run" file.)

    // -------------------------------------------------------
    // Bridge B
    // -------------------------------------------------------
    // Note: You cannot run multiple fwk's concurrently in the same process, as they fight over globals such as DgnViewLibHost.
    // That's why we have to run the second bridge in a separate process.
        {
        BeFileName bDir(testDir);
        bDir.AppendToPath(L"B");
        BeFileName::CreateNewDirectory(bDir.c_str());

        FwkArgvMaker argvMaker;
        argvMaker.SetUpBridgeProcessingArgs(bDir.c_str(), L"bridge_b", GetDgnv8BridgeDllName(), DEFAULT_IMODEL_NAME, true, L"imodel-bank.rsp");
        argvMaker.ReplaceArgValue(L"--imodel-bank-access-token", ComputeAccessTokenW("B").c_str());

        BentleyApi::BeFileName inputFile;
        MakeCopyOfFile(inputFile, L"Test3d.dgn", L"_B");

        argvMaker.SetInputFileArg(inputFile);
        argvMaker.SetSkipAssignmentCheck();

        auto bridge_b = StartImodelBridgeFwkExe(argvMaker);

        createFile(b_can_run);      // let b run

        ASSERT_EQ(BSISUCCESS, bridge_b.Stop(5*60*1000));   // wait for up to 5 minutes for b to finish

        ASSERT_EQ(0, bridge_b.GetExitCode());

        bridge_b_briefcaseName = bDir;
        bridge_b_briefcaseName.AppendToPath(DEFAULT_IMODEL_NAME);
        bridge_b_briefcaseName.append(L".bim");
        }

    BeThreadUtilities::BeSleep(5000); // *** WIP flakey/depends on timings *** make sure thread A runs and has a chance to make its request for schema lock (so that it can be denied!)
    createFile(a_can_run);      // now let A have the schema lock

    bridge_a.join();

    runningServer.Stop(10*1000); // NEEDS WORK: must wait long enough for imodel-bank server to shut down. The time required can vary unpredictably but seems to increase when a second process has been running.
    }
    }

    // ASSERT_EQ(0, ProcessRunner::FindProcessId("node.exe"));
    // ASSERT_EQ(0, ProcessRunner::FindProcessId("iModelBridgeFwk.exe"));
    ASSERT_TRUE(bridge_a_sawRetryMsg);
    ASSERT_TRUE(bridge_a_briefcaseName.DoesPathExist());
    ASSERT_TRUE(bridge_b_briefcaseName.DoesPathExist());

    // Verify that B ran first and A ran second
    if (true)
        {
        // If bridge A ran second, it must have had to pull bridge B's changesets before it could push.
        // Therefore, A's briefcase should contain the RepositoryLink for B's input file as well as its own.
        DbFileInfo info(bridge_a_briefcaseName);
        auto aSourceFileId = info.GetRepositoryLinkByFileNameLike("%test3d_a.dgn");
        auto bSourceFileId = info.GetRepositoryLinkByFileNameLike("%test3d_b.dgn");
        ASSERT_TRUE(aSourceFileId.IsValid());
        ASSERT_TRUE(bSourceFileId.IsValid());
        }

    if (true)
        {
        // If bridge B ran first and finished before A could run, B's briefcase should not have a record of A's input file.
        DbFileInfo info(bridge_b_briefcaseName);
        auto aSourceFileId = info.GetRepositoryLinkByFileNameLike("%test3d_a.dgn");
        auto bSourceFileId = info.GetRepositoryLinkByFileNameLike("%test3d_b.dgn");
        ASSERT_FALSE(aSourceFileId.IsValid());
        ASSERT_TRUE(bSourceFileId.IsValid());
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTests::RunBridgeAsUser(BeFileNameR briefcaseName, BeFileNameCR testDir, Utf8CP userName, BeFileNameCR inputFile, WCharCP bridgeName)
    {
    BeFileName userStagingDir(testDir);
    userStagingDir.AppendToPath(WString(userName, true).c_str());
    BeFileName::CreateNewDirectory(userStagingDir.c_str());

    FwkArgvMaker argvMaker;
    argvMaker.SetUpBridgeProcessingArgs(userStagingDir.c_str(), bridgeName, GetDgnv8BridgeDllName(), DEFAULT_IMODEL_NAME, true, L"imodel-bank.rsp");
    argvMaker.ReplaceArgValue(L"--imodel-bank-access-token", ComputeAccessTokenW(userName).c_str());

    argvMaker.SetInputFileArg(inputFile);
    argvMaker.SetSkipAssignmentCheck();

    argvMaker.SetMaxRetries(255, true);   // must allow lots of time for bridge bridge to run to completion. Unfortunately, there is no way to predict how many retries will be required.

    iModelBridgeFwk fwk;
    ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argvMaker.GetArgC(), argvMaker.GetArgV()));
    ASSERT_EQ(0, fwk.Run(argvMaker.GetArgC(), argvMaker.GetArgV()));

    briefcaseName = fwk.GetBriefcaseName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, MultiUsersSameBridgeSequential)
    {
    auto testDir = CreateTestDir();

    BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", L"");

    CreateImodelBankRepository(GetSeedFile());

    BentleyApi::BeFileName bridge_a_briefcaseName;
    BentleyApi::BeFileName bridge_b_briefcaseName;
        {
        std::unique_ptr<BentleyApi::Dgn::IModelBankClient> bankClient(CreateIModelBankClient(""));
            { // make sure server is stopped before releasing bankClient, as that is used by runningServer's Stop method.
            auto runningServer = StartImodelBankServer(GetIModelDir(), *bankClient);

            ASSERT_TRUE(m_client == nullptr);
            iModelBridgeFwk::ClearIModelClientForBridgesForTesting(); // nobody should have set the test client, but clear it just in case.

            // A - converts input file for first time
            RunBridgeAsUser(bridge_a_briefcaseName, testDir, "A", inputFile);

            int countAfterA;
            int repositoryLinkCount;
                {
                DbFileInfo aInfo(bridge_a_briefcaseName);
                ASSERT_EQ(1, aInfo.GetJobSubjectCount());
                repositoryLinkCount = aInfo.GetRepositoryLinkCount();
                countAfterA = aInfo.GetElementCount();
                }

            // A hands off to B.

            // B - adds a line and pushes changes
            AddLine(inputFile);

            RunBridgeAsUser(bridge_b_briefcaseName, testDir, "B", inputFile);

            int countAfterB;
                {
                DbFileInfo bInfo(bridge_b_briefcaseName);
                ASSERT_EQ(repositoryLinkCount, bInfo.GetRepositoryLinkCount()) << "RepositoryLink count should be unchanged, as both users used the same input file";
                ASSERT_EQ(1, bInfo.GetJobSubjectCount()) << "Both users should have used the SAME Job Subject, since they use the same bridge and same input file.";
                ASSERT_EQ(1, bInfo.GetRepositoryLinkCount());
                countAfterB = bInfo.GetElementCount();
                ASSERT_EQ(countAfterA + 1, countAfterB) << "User B added 1 line";
                }

            // B hands off to A

            // A - adds another line and then synchronizes
            AddLine(inputFile);

            RunBridgeAsUser(bridge_a_briefcaseName, testDir, "A", inputFile);
                {
                DbFileInfo aInfo(bridge_a_briefcaseName);
                ASSERT_EQ(repositoryLinkCount, aInfo.GetRepositoryLinkCount()) << "RepositoryLink count should be unchanged, as both users used the same input file";
                ASSERT_EQ(1, aInfo.GetJobSubjectCount()) << "Both users should have used the SAME Job Subject, since they use the same bridge and same input file.";
                auto countAfterA2 = aInfo.GetElementCount();
                ASSERT_EQ(countAfterB + 1, countAfterA2) << "User A added another line, on top of what B did";
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
// Can't do this - you can't run DgnV8Convert in multiple threads in the same process. Each one will try to initialize DgnPlatform/DgnCore
// and will contend over the same set of globals. That will cause errors.
// TEST_F(MstnBridgeTests, MultiUsersSameBridgeParallel)
//     {
//     auto testDir = CreateTestDir();

//     BeFileName inputFile;
//     MakeCopyOfFile(inputFile, L"Test3d.dgn", L"");

//     CreateImodelBankRepository(GetSeedFile());

//     bool bridge_a_sawRetryMsg = false;
//     BentleyApi::BeFileName bridge_a_briefcaseName;
//     BentleyApi::BeFileName bridge_b_briefcaseName;
//         {
//         std::unique_ptr<BentleyApi::Dgn::IModelBankClient> bankClient(CreateIModelBankClient(""));
//             { // make sure server is stopped before releasing bankClient, as that is used by runningServer's Stop method.
//             auto runningServer = StartImodelBankServer(GetIModelDir(), *bankClient);

//             ASSERT_TRUE(m_client == nullptr);
//             iModelBridgeFwk::ClearIModelClientForBridgesForTesting(); // nobody should have set the test client, but clear it just in case.

//             std::thread user_a([&]
//                 {
//                 RunBridgeAsUser(bridge_a_briefcaseName, testDir, "A", inputFile);
//                 });

//             std::thread user_b([&]
//                 {
//                 RunBridgeAsUser(bridge_b_briefcaseName, testDir, "B", inputFile);
//                 });

//             user_a.join();
//             user_b.join();

//             // Run once more, to make sure both have pulled.
//             RunBridgeAsUser(bridge_a_briefcaseName, testDir, "A", inputFile);
//             RunBridgeAsUser(bridge_b_briefcaseName, testDir, "B", inputFile);

//             DbFileInfo aInfo(bridge_a_briefcaseName);
//             DbFileInfo bInfo(bridge_b_briefcaseName);
//             ASSERT_EQ(aInfo.GetElementCount(), bInfo.GetElementCount());
//             }
//         }
//     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, ConvertLinesUsingBridgeFwk)
    {
    // if (nullptr == GetIModelBankServerJs())
    //   ProcessRunner::DoSleep(5000);

    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY);

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));
    args.push_back(L"--fwk-skip-assignment-check");

    SetupClient();
    CreateRepository();
    // ASSERT_EQ(0, ProcessRunner::FindProcessId("node.exe"));
    auto runningServer = StartServer();

    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);
        }

    CleanupElementECExtensions();
    int prevCount = DbFileInfo(m_briefcaseName).GetElementCount();
    if (true)
        {
        //Make sure a second run of the bridge does not add any elements.
        RunTheBridge(args);
        EXPECT_EQ(prevCount, DbFileInfo(m_briefcaseName).GetElementCount());
        }

    CleanupElementECExtensions();
    AddLine(inputFile);
    if (true)
        {
        // and run an update
        RunTheBridge(args);
        EXPECT_EQ(1 + prevCount, DbFileInfo(m_briefcaseName).GetElementCount());
        }

    CleanupElementECExtensions();
    if (true)
        {
        //Make sure a second run of the bridge does not add any elements.
        RunTheBridge(args);
        EXPECT_EQ(1 + prevCount, DbFileInfo(m_briefcaseName).GetElementCount());
        }
    CleanupElementECExtensions();

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, ConvertCve)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY);

    BentleyApi::BeFileName emptyFile = GetTestDataFileName(L"Test3d.dgn");
    BentleyApi::BeFileName cveFile = GetTestDataFileName(L"cve.dgn");

    args.push_back(L"--fwk-skip-assignment-check");

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", emptyFile.c_str()));
    RunTheBridge(args);
    CleanupElementECExtensions();

    args.pop_back();
    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", cveFile.c_str()));
    RunTheBridge(args);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, TestSourceElementIdAspect)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY);

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));
    args.push_back(L"--fwk-skip-assignment-check");

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    int64_t srcId = AddLine(inputFile);
    if (true)
        {
        // and run an update
        RunTheBridge(args);
        }
    DbFileInfo info(m_briefcaseName);
    DgnElementId id;
    ASSERT_EQ(SUCCESS, info.GetiModelElementByDgnElementId(id, srcId));

    ASSERT_TRUE(id.IsValid());
    CleanupElementECExtensions();

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST_F (MstnBridgeTests, TestModelAspect)
//{
//    auto testDir = CreateTestDir();
//
//    bvector<WString> args;
//    SetUpBridgeProcessingArgs (args, testDir.c_str (), MSTN_BRIDGE_REG_SUB_KEY);
//
//    args.push_back (L"--fwk-storeElementIdsInBIM");
//    BentleyApi::BeFileName inputFile;
//    MakeCopyOfFile (inputFile, L"Test3d.dgn", NULL);
//
//    args.push_back (WPrintfString (L"--fwk-input=\"%ls\"", inputFile.c_str ()));
//    args.push_back (L"--fwk-skip-assignment-check");
//
//    SetupClient();
//    CreateRepository ();
//    auto runningServer = StartServer();
//
//    int64_t modelid = AddModel (inputFile, "TestModel");
//    if (true)
//    {
//        // and run an update
//        RunTheBridge (args);
//    }
//    DbFileInfo info (m_briefcaseName);
//
//    BentleyApi::BeSQLite::EC::ECSqlStatement estmt;
//    estmt.Prepare (*info.m_db, "SELECT kind,SourceId,Properties FROM "
//        BIS_SCHEMA (BIS_CLASS_Model) " AS m,"
//        XTRN_SRC_ASPCT_FULLCLASSNAME " AS sourceInfo"
//        " WHERE (sourceInfo.Element.Id=m.ModeledElement.Id) AND (sourceInfo.SourceId = ?)");
//    estmt.BindInt64 (1, modelid);
//
//    ASSERT_TRUE (BentleyApi::BeSQLite::BE_SQLITE_ROW == estmt.Step ());
//    BentleyApi::Utf8String kind, properties, modelName;
//    int64_t srcid;
//    rapidjson::Document json;
//
//    kind = estmt.GetValueText (0);
//    ASSERT_TRUE (kind.Equals ("Model"));
//
//    srcid = estmt.GetValueId<int64_t> (1);
//    ASSERT_TRUE (srcid == modelid);
//
//    properties = estmt.GetValueText (2);
//    json.Parse (properties.c_str ());
//    modelName = json["v8ModelName"].GetString ();
//    ASSERT_TRUE (modelName.Equals ("TestModel"));
//}
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Mayuresh.Kanade                 01/2019
//+---------------+---------------+---------------+---------------+---------------+------*/
//TEST_F (MstnBridgeTests, TestViewAspect)
//{
//    auto testDir = CreateTestDir();
//
//    bvector<WString> args;
//    SetUpBridgeProcessingArgs (args, testDir.c_str (), MSTN_BRIDGE_REG_SUB_KEY);
//
//    args.push_back (L"--fwk-storeElementIdsInBIM");
//    BentleyApi::BeFileName inputFile;
//    MakeCopyOfFile (inputFile, L"Test3d.dgn", NULL);
//
//    args.push_back (WPrintfString (L"--fwk-input=\"%ls\"", inputFile.c_str ()));
//    args.push_back (L"--fwk-skip-assignment-check");
//
//    SetupClient();
//    CreateRepository();
//    auto runningServer = StartServer();
//
//    int64_t modelid = AddView (inputFile, "TestView");
//    if (true)
//    {
//        // and run an update
//        RunTheBridge (args);
//    }
//    DbFileInfo info (m_briefcaseName);
//
//    BentleyApi::BeSQLite::EC::ECSqlStatement estmt;
//    estmt.Prepare (*info.m_db, "SELECT kind,SourceId,Properties FROM "
//        BIS_SCHEMA (BIS_CLASS_ViewDefinition) " AS v,"
//        XTRN_SRC_ASPCT_FULLCLASSNAME " AS sourceInfo"
//        " WHERE (sourceInfo.Element.Id=v.ModeledElement.Id) AND (sourceInfo.SourceId = ?)");
//    estmt.BindInt64 (1, modelid);
//
//    ASSERT_TRUE (BentleyApi::BeSQLite::BE_SQLITE_ROW == estmt.Step ());
//    BentleyApi::Utf8String kind, properties, viewName;
//    int64_t srcid;
//    rapidjson::Document json;
//
//    kind = estmt.GetValueText (0);
//    ASSERT_TRUE (kind.Equals ("ViewDefinition"));
//
//    srcid = estmt.GetValueId<int64_t> (1);
//    ASSERT_TRUE (srcid == modelid);
//
//    properties = estmt.GetValueText (2);
//    json.Parse (properties.c_str ());
//    viewName = json["v8ViewName"].GetString ();
//    ASSERT_TRUE (viewName.Equals ("TestView"));
//}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, ConvertAttachmentSingleBridge)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);
    AddLine(inputFile);

    BentleyApi::BeFileName refFile;
    MakeCopyOfFile(refFile, L"Test3d.dgn", L"-Ref-1");
    AddLine(refFile);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, DEFAULT_IMODEL_NAME_A);
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, iModelBridge_getAffinity);

    BentleyApi::BeSQLite::BeGuid guid, refGuid;
    guid.Create();
    refGuid.Create();

    iModelBridgeDocumentProperties docProps(guid.ToString().c_str(), "wurn1", "durn1", "other1", "");
    iModelBridgeDocumentProperties refDocProps(refGuid.ToString().c_str(), "wurn2", "durn2", "other2", "");
    testRegistry.SetDocumentProperties(docProps, inputFile);
    testRegistry.SetDocumentProperties(refDocProps, refFile);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, inputFile, L"");
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, refFile, L"");
    testRegistry.Save();
    TerminateHost();

    int modelCount = 0;
    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);

        modelCount = DbFileInfo(m_briefcaseName).GetModelCount();
        ASSERT_EQ(8, modelCount);
        }

    CleanupElementECExtensions();
    AddAttachment(inputFile, refFile, 1, true);
    AddAttachment(inputFile, refFile, 1, true);
    if (true)
        {
        //We added a new attachment.
        RunTheBridge(args);
        ASSERT_EQ(modelCount + 1, DbFileInfo(m_briefcaseName).GetModelCount());
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void countElementsInModelByClass(DgnModel const& model, DgnClassId classId, int expected)
    {
    auto stmt = model.GetDgnDb().GetPreparedECSqlStatement("SELECT COUNT(*) from " BIS_SCHEMA(BIS_CLASS_Element) " WHERE Model.Id=? AND ECClassId=?");
    stmt->BindId(1, model.GetModelId());
    stmt->BindId(2, classId);
    stmt->Step();
    EXPECT_EQ(expected, stmt->GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/20
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, ConvertIfcAttachmentSingleBridge)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);
    
    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);

    BentleyApi::BeFileName refFile;
    MakeCopyOfFile(refFile, L"roof.ifc", NULL);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, DEFAULT_IMODEL_NAME_A);
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, iModelBridge_getAffinity);

    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
        const size_t bufferSize,
        iModelBridgeAffinityLevel& affinityLevel,
        BentleyApi::WCharCP affinityLibraryPath,
            BentleyApi::WCharCP sourceFileName)
        {
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        };

    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);
    BentleyApi::BeSQLite::BeGuid guid, refGuid;
    guid.Create();
    refGuid.Create();

    iModelBridgeDocumentProperties docProps(guid.ToString().c_str(), "wurn1", "durn1", "other1", "");
    iModelBridgeDocumentProperties refDocProps(refGuid.ToString().c_str(), "wurn2", "durn2", "other2", "");
    testRegistry.SetDocumentProperties(docProps, inputFile);
    testRegistry.SetDocumentProperties(refDocProps, refFile);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, inputFile, L"");
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, refFile, L"");
    testRegistry.Save();
    TerminateHost();

    int modelCount = 0;
    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);
        
        modelCount = DbFileInfo(m_briefcaseName).GetModelCount();
        ASSERT_EQ(8, modelCount);
        }

    CleanupElementECExtensions();
    AddAttachment(inputFile, refFile, 1, true);
    CleanupElementECExtensions();
    if (true)
        {
        //We added a new attachment.
        RunTheBridge(args);
        ASSERT_EQ(modelCount + 2, DbFileInfo(m_briefcaseName).GetModelCount()); // The bridge adds the ifc model and the special "PresentationRules" definition model.
        }

    DbFileInfo dbInfo(m_briefcaseName);
    auto db = dbInfo.m_db;
    auto ids = db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalModel), nullptr, "ORDER BY ECInstanceId ASC").BuildIdList();
    auto roof = db->Models().GetModel(ids[1]);
    ASSERT_TRUE(roof.IsValid()) << "Null physical model created from the IFC DgnAttachment!";

    // The roof model should contain these IFC elements
    countElementsInModelByClass(*roof, db->Schemas().GetClassId("IFC2x3", "IfcSlab"), 14);
    countElementsInModelByClass(*roof, db->Schemas().GetClassId("IFC2x3", "IfcWallStandardCase"), 10);
    countElementsInModelByClass(*roof, db->Schemas().GetClassId("IFC2x3", "IfcShapeRepresentation"), 41);
    countElementsInModelByClass(*roof, db->Schemas().GetClassId("IFC2x3", "IfcStyledRepresentation"), 27);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, ConvertAttachmentSingleBridgeAlternateRegistry)
    {
    auto testDir = CreateTestDir();

    BentleyApi::BeFileName stagingDir(testDir);
    stagingDir.AppendToPath(L"staging");
    EXPECT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::CreateNewDirectory(stagingDir.c_str()));

    BentleyApi::BeFileName registryDir(testDir);
    registryDir.AppendToPath(L"assignments");
    EXPECT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::CreateNewDirectory(registryDir.c_str()));

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, stagingDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);
    args.push_back(_wcsdup(BentleyApi::WPrintfString(L"--registry-dir=%s", registryDir.c_str()).c_str()));

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);
    AddLine(inputFile);

    BentleyApi::BeFileName refFile;
    MakeCopyOfFile(refFile, L"Test3d.dgn", L"-Ref-1");
    AddLine(refFile);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(registryDir, DEFAULT_IMODEL_NAME_A);
    FakeRegistry testRegistry(registryDir, assignDbName);
    testRegistry.WriteAssignments();

    BentleyApi::WString mstnbridgeRegSubKey = L"iModelBridgeForMstn";

    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
        const size_t bufferSize,
        iModelBridgeAffinityLevel& affinityLevel,
        BentleyApi::WCharCP affinityLibraryPath,
            BentleyApi::WCharCP sourceFileName)
        {
        wcsncpy(buffer, mstnbridgeRegSubKey.c_str(), mstnbridgeRegSubKey.length());
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        };

    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);

    BentleyApi::BeSQLite::BeGuid guid, refGuid;
    guid.Create();
    refGuid.Create();

    iModelBridgeDocumentProperties docProps(guid.ToString().c_str(), "wurn1", "durn1", "other1", "");
    iModelBridgeDocumentProperties refDocProps(refGuid.ToString().c_str(), "wurn2", "durn2", "other2", "");
    testRegistry.SetDocumentProperties(docProps, inputFile);
    testRegistry.SetDocumentProperties(refDocProps, refFile);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, inputFile, L"");
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, refFile, L"");
    testRegistry.Save();
    TerminateHost();

    int modelCount = 0;
    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);

        modelCount = DbFileInfo(m_briefcaseName).GetModelCount();
        ASSERT_EQ(8, modelCount);
        }

    CleanupElementECExtensions();
    AddAttachment(inputFile, refFile, 1, true);
    AddAttachment(inputFile, refFile, 1, true);
    if (true)
        {
        //We added a new attachment.
        RunTheBridge(args);
        ASSERT_EQ(modelCount + 1, DbFileInfo(m_briefcaseName).GetModelCount());
        }

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, ConvertAttachmentMultiBridge)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), nullptr, DEFAULT_IMODEL_NAME);

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);
    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));
    AddLine(inputFile);

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, DEFAULT_IMODEL_NAME_A);
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();

    BentleyApi::BeFileName refFile;
    MakeCopyOfFile(refFile, L"Test3d.dgn", L"-Ref-1");

    AddLine(refFile);

    BentleyApi::WString mstnbridgeRegSubKey = L"iModelBridgeForMstn";
    BentleyApi::WString abdbridgeRegSubKey = L"ABD";

    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
                                                           const size_t bufferSize,
                                                           iModelBridgeAffinityLevel& affinityLevel,
                                                           BentleyApi::WCharCP affinityLibraryPath,
                                                           BentleyApi::WCharCP sourceFileName)
        {
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        BentleyApi::BeFileName srcFile(sourceFileName);
        if (0 == srcFile.CompareTo(refFile))
            {
            wcsncpy(buffer, abdbridgeRegSubKey.c_str(), abdbridgeRegSubKey.length());
            }
        else
            wcsncpy(buffer, mstnbridgeRegSubKey.c_str(), mstnbridgeRegSubKey.length());
        };

    testRegistry.AddBridge(mstnbridgeRegSubKey, lambda);
    testRegistry.AddBridge(abdbridgeRegSubKey, lambda);

    BentleyApi::BeSQLite::BeGuid guid, refGuid;
    guid.Create();
    refGuid.Create();

    iModelBridgeDocumentProperties docProps(guid.ToString().c_str(), "wurn1", "durn1", "other1", "");
    iModelBridgeDocumentProperties refDocProps(refGuid.ToString().c_str(), "wurn2", "durn2", "other2", "");
    testRegistry.SetDocumentProperties(docProps, inputFile);
    testRegistry.SetDocumentProperties(refDocProps, refFile);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, inputFile, L"");
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, refFile, L"");
    testRegistry.Save();
    //We need to shut down v8host at the end so that rest of the processing works.
    TerminateHost();

    // Run each bridge once, before we add the ABD-specific attachment.
    int modelCount = 0;
    if (true)
        {
        bvector<WString> margs(args);
        margs.push_back(L"--fwk-bridge-regsubkey=iModelBridgeForMstn");
        RunTheBridge(margs);
        modelCount = DbFileInfo(m_briefcaseName).GetModelCount(); // MstnBridge will create some definition models for its own use, plus a PhysicalModel for the root input model
        }

    CleanupElementECExtensions();
    if (true)
        {
        bvector<WString> rargs(args);
        rargs.push_back(L"--fwk-bridge-regsubkey=ABD");
        RunTheBridge(rargs);
        modelCount = DbFileInfo(m_briefcaseName).GetModelCount(); // ABD bridge will create some definition models for its own use
        }

    CleanupElementECExtensions();
    // Add an ABD-specific attachment (twice)
    AddAttachment(inputFile, refFile, 1, true);
    AddAttachment(inputFile, refFile, 1, true);

    if (true)
        {
        bvector<WString> margs(args);
        margs.push_back(L"--fwk-bridge-regsubkey=iModelBridgeForMstn");
        RunTheBridge(margs);
        ASSERT_EQ(modelCount, DbFileInfo(m_briefcaseName).GetModelCount()) << "MstnBridge should not have converted or created any more models";
        }
    CleanupElementECExtensions();
    if (true)
        {
        bvector<WString> rargs(args);
        rargs.push_back(L"--fwk-bridge-regsubkey=ABD");
        RunTheBridge(rargs);
        ASSERT_EQ(modelCount + 1, DbFileInfo(m_briefcaseName).GetModelCount()) << "ABD bridge should have detected and converted the new attachments, and it should have that both point to the same model.";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, ConvertAttachmentMultiBridgeSharedReference)
    {
    CreateTestDir();

    WCharCP iModelName = L"ConvertAttachmentMultiBridgeSharedReference";
    bvector<WString> args;
    SetUpBridgeProcessingArgs(args,nullptr, nullptr, iModelName);

    SetupClient();
    CreateRepository("ConvertAttachmentMultiBridgeSharedReference");
    auto runningServer = StartServer();

    BentleyApi::BeFileName inputFile1;
    MakeCopyOfFile(inputFile1, L"Test3d.dgn", L"1");
    AddLine(inputFile1);

    BentleyApi::BeFileName inputFile2;
    MakeCopyOfFile(inputFile2, L"Test3d.dgn", L"2");
    AddLine(inputFile2);

    BentleyApi::BeFileName refFile;
    MakeCopyOfFile(refFile, L"Test3d.dgn", L"-Ref-1");

    AddLine(refFile);

    AddAttachment(inputFile1, refFile, 1, true);
    AddAttachment(inputFile1, refFile, 1, true);
    AddAttachment(inputFile2, refFile, 1, true);
    //AddAttachment(inputFile1, refFile, 1, true);

    BentleyApi::BeSQLite::BeGuid guid1, refGuid, guid2;
    guid1.FromString("C9A63366-81F3-471D-B7CB-F4827E94BB03");
    guid2.FromString("85E950D7-6F9C-4FA8-B195-F1B9982D1E2C");
    refGuid.FromString("586AF9AD-8AA3-4126-9410-8E7F6B4AB5E2");

    // Note the SetupTestDirectory creates a registry that says that 
    // * the reference file is assigned to ABD 
    // * all other files are assigned to iModelBridgeForMstn

    int modelCount = 0;
    BeFileName b1BriefcaseName;
    if (true)
        {
        // Run iModelBridgeForMstn on inputFile1. It should convert the master file, but not the reference file.
        BentleyApi::BeFileName testDir1;
        SetupTestDirectory(testDir1, L"1", iModelName, inputFile1, guid1, refFile, refGuid);

        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        bvector<WString> margs(args);
        margs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile1.c_str()));
        margs.push_back(L"--fwk-bridge-regsubkey=iModelBridgeForMstn");
        margs.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir1.c_str()));
        RunTheBridge(margs);

        DbFileInfo info(m_briefcaseName);
        modelCount = info.GetPhysicalModelCount();
        int provenanceCount1 = info.GetModelProvenanceCount(guid1);
        int provenanceCountRef = info.GetModelProvenanceCount(refGuid);
        ASSERT_EQ(1, provenanceCount1);
        ASSERT_EQ(0, provenanceCountRef);
        ASSERT_EQ(1, modelCount);

        b1BriefcaseName = m_briefcaseName;
        }

    CleanupElementECExtensions();
    if (true)
        {
        // Run ABD on inputFile1. It should convert the reference file.
        BentleyApi::BeFileName testDir2;
        SetupTestDirectory(testDir2, L"2", iModelName, inputFile1, guid1, refFile, refGuid);

        //We added a new attachment.
        bvector<WString> rargs(args);
        rargs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile1.c_str()));
        rargs.push_back(L"--fwk-bridge-regsubkey=ABD");
        rargs.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir2.c_str()));

        RunTheBridge(rargs);

        DbFileInfo info(m_briefcaseName);
        ASSERT_EQ(++modelCount, info.GetPhysicalModelCount());

        int provenanceCount1 = info.GetModelProvenanceCount(guid1);
        int provenanceCountRef = info.GetModelProvenanceCount(refGuid);
        ASSERT_EQ(1, provenanceCount1);
        ASSERT_EQ(1, provenanceCountRef);

        ASSERT_TRUE(!b1BriefcaseName.EqualsI(m_briefcaseName)) << "Different bridges should use different briefcases";
        }

    CleanupElementECExtensions();
    if (true)
        {
        // Run iModelBridgeForMstn on inputFile2. It should convert the second master file only.
        BentleyApi::BeFileName testDir3;
        SetupTestDirectory(testDir3, L"3", iModelName, inputFile2, guid2, refFile, refGuid);

        bvector<WString> margs(args);
        margs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile2.c_str()));
        margs.push_back(L"--fwk-bridge-regsubkey=iModelBridgeForMstn");
        margs.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir3.c_str()));
        RunTheBridge(margs);

        DbFileInfo info(m_briefcaseName);
        ASSERT_EQ(++modelCount, info.GetPhysicalModelCount());

        int provenanceCount1 = info.GetModelProvenanceCount(guid1);
        int provenanceCountRef = info.GetModelProvenanceCount(refGuid);
        int provenanceCount2 = info.GetModelProvenanceCount(guid2);
        ASSERT_EQ(1, provenanceCount1);
        ASSERT_EQ(1, provenanceCountRef);
        ASSERT_EQ(1, provenanceCount2);
        }

    CleanupElementECExtensions();
    size_t referencesSubjectsCount = 0;
    if (true)
        {
        // Run ABD on inputFile2. It should do nothing. I should detect that the reference file was already converted.
        BentleyApi::BeFileName testDir4;
        SetupTestDirectory(testDir4, L"4", iModelName, inputFile2, guid2, refFile, refGuid);

        bvector<WString> rargs(args);
        rargs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile2.c_str()));
        rargs.push_back(L"--fwk-bridge-regsubkey=ABD");
        rargs.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir4.c_str()));
        RunTheBridge(rargs);
        DbFileInfo info(m_briefcaseName);
        ASSERT_EQ(modelCount, info.GetPhysicalModelCount());

        int provenanceCount1 = info.GetModelProvenanceCount(guid1);
        int provenanceCountRef = info.GetModelProvenanceCount(refGuid);
        int provenanceCount2 = info.GetModelProvenanceCount(guid2);
        ASSERT_EQ(1, provenanceCount1);
        ASSERT_EQ(1, provenanceCountRef);
        ASSERT_EQ(1, provenanceCount2);
        }
    if (true)
        {
        referencesSubjectsCount = DbFileInfo(m_briefcaseName).GetReferencesSubjects().size();
        }


    // Delete the attachments to the common reference file from both masterfiles ...
    DeleteAllAttachments(inputFile1);
    DeleteAllAttachments(inputFile2);

    // ... and verify that "ABD" bridge deletes the reference models, EVEN IF we run iModelBridgeForMstn first.
    //      (Do all of this in briefcase #5 - there is no need for each step to have a separate briefcase)
    BentleyApi::BeFileName testDir5;
    SetupTestDirectory(testDir5, L"5", iModelName, inputFile1, guid1, refFile, refGuid, inputFile2, guid2);
    if (true)
        {
        // Run iModelBridgeForMstn on inputFile1. It should delete its own References Subjects, but it should not do anything to the model, as the reference is not assigned to it.
        bvector<WString> margs(args);
        margs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile1.c_str()));
        margs.push_back(L"--fwk-bridge-regsubkey=iModelBridgeForMstn");
        margs.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir5.c_str()));
        RunTheBridge(margs);

        DbFileInfo info(m_briefcaseName);
        int provenanceCount1 = info.GetModelProvenanceCount(guid1);
        int provenanceCountRef = info.GetModelProvenanceCount(refGuid);
        int provenanceCount2 = info.GetModelProvenanceCount(guid2);
        ASSERT_EQ(1, provenanceCount1);
        ASSERT_EQ(1, provenanceCountRef);
        ASSERT_EQ(1, provenanceCount2);
        ASSERT_EQ(modelCount, info.GetPhysicalModelCount());

        b1BriefcaseName = m_briefcaseName;
        }
    if (true)
        {
        auto newReferencesSubjectsCount = DbFileInfo(m_briefcaseName).GetReferencesSubjects().size();
        ASSERT_EQ(referencesSubjectsCount - 2, newReferencesSubjectsCount);
        referencesSubjectsCount = newReferencesSubjectsCount;
        }

    CleanupElementECExtensions();
    if (true)
        {
        // Run ABD on inputFile1. It should delete its References Subjects. It not delete the referenced model but only reparent it to inputFile2
        bvector<WString> rargs(args);
        rargs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile1.c_str()));
        rargs.push_back(L"--fwk-bridge-regsubkey=ABD");
        rargs.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir5.c_str()));

        RunTheBridge(rargs);

        DbFileInfo info(m_briefcaseName);
        int provenanceCount1 = info.GetModelProvenanceCount(guid1);
        int provenanceCountRef = info.GetModelProvenanceCount(refGuid);
        int provenanceCount2 = info.GetModelProvenanceCount(guid2);
        ASSERT_EQ(1, provenanceCount1);
        ASSERT_EQ(1, provenanceCountRef);
        ASSERT_EQ(1, provenanceCount2);
        ASSERT_EQ(modelCount, info.GetPhysicalModelCount());
        }
    if (true)
        {
        auto newReferencesSubjectsCount = DbFileInfo(m_briefcaseName).GetReferencesSubjects().size();
        ASSERT_EQ(referencesSubjectsCount - 2, newReferencesSubjectsCount);
        referencesSubjectsCount = newReferencesSubjectsCount;
        }

    CleanupElementECExtensions();
    if (true)
        {
        // Run iModelBridgeForMstn on inputFile2. It should delete its References Subject, but it should not do anything to the model, as the reference is not assigned to it.
        bvector<WString> margs(args);
        margs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile2.c_str()));
        margs.push_back(L"--fwk-bridge-regsubkey=iModelBridgeForMstn");
        margs.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir5.c_str()));
        RunTheBridge(margs);

        DbFileInfo info(m_briefcaseName);
        int provenanceCount1 = info.GetModelProvenanceCount(guid1);
        int provenanceCountRef = info.GetModelProvenanceCount(refGuid);
        int provenanceCount2 = info.GetModelProvenanceCount(guid2);
        ASSERT_EQ(1, provenanceCount1);
        ASSERT_EQ(1, provenanceCountRef);
        ASSERT_EQ(1, provenanceCount2);
        ASSERT_EQ(modelCount, info.GetPhysicalModelCount());
        }
    if (true)
        {
        auto newReferencesSubjectsCount = DbFileInfo(m_briefcaseName).GetReferencesSubjects().size();
        ASSERT_EQ(referencesSubjectsCount - 1, newReferencesSubjectsCount);
        referencesSubjectsCount = newReferencesSubjectsCount;
        }

    CleanupElementECExtensions();
    if (true)
        {
        // Run ABD on inputFile2. It should delete its References Subject, and reparent the model to element #1 as an orphan.
        bvector<WString> rargs(args);
        rargs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile2.c_str()));
        rargs.push_back(L"--fwk-bridge-regsubkey=ABD");
        rargs.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir5.c_str()));
        RunTheBridge(rargs);

        DbFileInfo info(m_briefcaseName);
        int provenanceCount1 = info.GetModelProvenanceCount(guid1);
        int provenanceCountRef = info.GetModelProvenanceCount(refGuid);
        int provenanceCount2 = info.GetModelProvenanceCount(guid2);
        ASSERT_EQ(1, provenanceCount1);
        ASSERT_EQ(1, provenanceCountRef);
        ASSERT_EQ(1, provenanceCount2);
        ASSERT_EQ(modelCount, info.GetPhysicalModelCount());
        }
    if (true)
        {
        auto newReferencesSubjectsCount = DbFileInfo(m_briefcaseName).GetReferencesSubjects().size();
        ASSERT_EQ(referencesSubjectsCount - 1, newReferencesSubjectsCount);
        referencesSubjectsCount = newReferencesSubjectsCount;
        ASSERT_EQ(0, referencesSubjectsCount) << "all dropped reference attachments should have been detected by now";
        }

    CleanupElementECExtensions();
    if (true)
        {
        // Run iModelBridgeForMstn specifying --fwk-all-docs-processed. It should make no changes at all.
        bvector<WString> margs(args);
        margs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile2.c_str()));
        margs.push_back(L"--fwk-bridge-regsubkey=iModelBridgeForMstn");
        margs.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        margs.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir5.c_str()));
        RunTheBridge(margs);

        DbFileInfo info(m_briefcaseName);
        int provenanceCount1 = info.GetModelProvenanceCount(guid1);
        int provenanceCountRef = info.GetModelProvenanceCount(refGuid);
        int provenanceCount2 = info.GetModelProvenanceCount(guid2);
        ASSERT_EQ(1, provenanceCount1);
        ASSERT_EQ(1, provenanceCountRef);
        ASSERT_EQ(1, provenanceCount2);
        ASSERT_EQ(modelCount, info.GetPhysicalModelCount());
        }
    if (true)
        {
        auto newReferencesSubjectsCount = DbFileInfo(m_briefcaseName).GetReferencesSubjects().size();
        ASSERT_EQ(referencesSubjectsCount, newReferencesSubjectsCount);
        }

    CleanupElementECExtensions();
    if (true)
        {
        // Run ABD specifying --fwk-all-docs-processed. It should delete the reference model itself.
        BentleyApi::BeFileName testDir4;
        SetupTestDirectory(testDir4, L"5", iModelName, inputFile2, guid2, refFile, refGuid);

        bvector<WString> rargs(args);
        rargs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile2.c_str()));
        rargs.push_back(L"--fwk-bridge-regsubkey=ABD");
        rargs.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        rargs.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir4.c_str()));
        RunTheBridge(rargs);

        DbFileInfo info(m_briefcaseName);
        int provenanceCount1 = info.GetModelProvenanceCount(guid1);
        int provenanceCountRef = info.GetModelProvenanceCount(refGuid);
        int provenanceCount2 = info.GetModelProvenanceCount(guid2);
        ASSERT_EQ(1, provenanceCount1);
        ASSERT_EQ(0, provenanceCountRef);
        ASSERT_EQ(1, provenanceCount2);
        ASSERT_EQ(modelCount-1, info.GetPhysicalModelCount());
        }
    if (true)
        {
        auto newReferencesSubjectsCount = DbFileInfo(m_briefcaseName).GetReferencesSubjects().size();
        ASSERT_EQ(referencesSubjectsCount, newReferencesSubjectsCount);
        }
    }

//Sandwich test ?

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTests::VerifyCodeIsRegistered(DbFileInfo& bcInfo, DgnCodeCR code, Utf8CP codeValuePrefix, bool codeShouldBeRecorded)
    {
    // Verify that the server has or has not registered the element's code as expected.
    DgnCodeInfo codeInfo;
    auto status = bcInfo.GetCodeInfo(codeInfo, code, GetClient());
    if (codeShouldBeRecorded)
        {
        ASSERT_EQ(BentleyApi::BSISUCCESS, status);
        ASSERT_TRUE(codeInfo.IsUsed());
        ASSERT_TRUE(codeInfo.GetCode().GetValueUtf8().StartsWith(codeValuePrefix));
        }
    else
        {
        ASSERT_NE(BentleyApi::BSISUCCESS, status);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTests::VerifyElementHasCode(DgnElementCR el, Utf8CP codeValuePrefix)
    {
    ASSERT_TRUE(el.GetCode().IsValid());
    ASSERT_TRUE(el.GetCode().GetValueUtf8().StartsWith(codeValuePrefix));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTests::VerifyElementHasCode(BeFileNameCR bcName, DgnElementId eid, Utf8CP codeValuePrefix, bool codeShouldBeRecorded)
    {
    DbFileInfo bcInfo(bcName);

    auto el = bcInfo.m_db->Elements().GetElement(eid);
    ASSERT_TRUE(el.IsValid());

    VerifyElementHasCode(*el, codeValuePrefix);

    VerifyCodeIsRegistered(bcInfo, el->GetCode(), codeValuePrefix, codeShouldBeRecorded);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTests::VerifyConvertedElementHasCode(int prevCount, BeFileNameCR bcName, int64_t srcId, Utf8CP codeValuePrefix, bool codeShouldBeRecorded)
    {
    DbFileInfo bcInfo(bcName);

    DgnElementId eid;
    ASSERT_EQ(BSISUCCESS, bcInfo.GetiModelElementByDgnElementId(eid, srcId));
    auto el = bcInfo.m_db->Elements().GetElement(eid);
    ASSERT_TRUE(el.IsValid());

    VerifyElementHasCode(*el, codeValuePrefix);

    EXPECT_EQ(1 + prevCount, bcInfo.GetElementCount());
    VerifyCodeIsRegistered(bcInfo, el->GetCode(), codeValuePrefix, codeShouldBeRecorded);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, CodeReservation)
    {
    auto testDir = CreateTestDir();

    BeFileName noCodesAllowed(testDir);
    noCodesAllowed.AppendToPath(L"noCodesAllowed.txt");

    bool usingIModelBank = (GetIModelBankServerJs() != nullptr);

    Utf8PrintfString rules(
        R"(   [
                    {
                        "user": "user1",
                        "rules": [
                            {
                                "request": "Code/Create",
                                "rule": { "verb": "ifnofile", "object": "%s" }
                            }
                        ]
                    }
                ]
            )", Utf8String(noCodesAllowed).c_str());
    rules.ReplaceAll("\\", "/");

    SetRulesFileInEnv setRulesFileVar(WriteRulesFile(L"testCodeReservation.json", rules.c_str()));

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);

    SetupClient(ComputeAccessToken("user1"));

    if (GetClientAsMock())
        {
        fprintf(stderr, "CodeReservation tests are supported only when using real servers\n.");
        return;
        }

    CreateRepository();
    {
    auto runningServer = StartServer();


    FwkArgvMaker argvMaker;
    argvMaker.SetUpBridgeProcessingArgs(testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, GetDgnv8BridgeDllName(), DEFAULT_IMODEL_NAME, true, L"imodel-bank.rsp");
    argvMaker.ReplaceArgValue(L"--imodel-bank-access-token", ComputeAccessTokenW("user1").c_str());

    argvMaker.SetInputFileArg(inputFile);
    argvMaker.SetSkipAssignmentCheck();

    BeFileName bcName;
    bool expectCodesInLockedModelsToBeReported = true;
    BentleyApi::Dgn::LockLevel expectedRetainedChannelLockLevel = BentleyApi::Dgn::LockLevel::Exclusive;
    if (true)
        {
        // Must allow Code reservations during the initial conversion. That is where the bridge creates the Subject
        // elements in the repository model, and each Subject has a Code that must be reserved (because they must
        // be unique across bridges). So, do not create the "noCodesAllowed" file yet.

        iModelBridgeFwk fwk;
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argvMaker.GetArgC(), argvMaker.GetArgV()));
        ASSERT_EQ(0, fwk.Run(argvMaker.GetArgC(), argvMaker.GetArgV()));
        bcName = fwk.GetBriefcaseName();

        // TRICKY! Must ask fwk for this information now!
        expectCodesInLockedModelsToBeReported = fwk.AreCodesInLockedModelsReported();
        expectedRetainedChannelLockLevel = fwk.GetRetainedChannelLockLevel();
        }

    // Get information about the jobsubject and the spatial model that we will be using.
    int prevCount = 0;
    DgnElementId jobSubjectId;
    DgnModelId spatialModelId;
    RepositoryLinkId repositoryLinkId;
    if (true)
        {
        DbFileInfo bcInfo(bcName);
        prevCount = bcInfo.GetElementCount();
        auto jSubj = bcInfo.GetFirstJobSubject();
        ASSERT_TRUE(jSubj.IsValid());
        jobSubjectId = jSubj->GetElementId();
        bcInfo.MustFindFileByName(repositoryLinkId, inputFile, 1);
        bcInfo.MustFindModelByV8ModelId(spatialModelId, repositoryLinkId, GetDefaultV8ModelId(inputFile), 1);

        // Verify that the root subject is NOT locked by this bridge
        ASSERT_EQ(BentleyApi::Dgn::LockLevel::None, bcInfo.QueryLockLevel(*bcInfo.m_db->Elements().GetRootSubject(), GetClient()));

        // Verify that the jobSubject and this spatial model are exclusively locked by this bridge
        // ... or that they are NOT locked, according to the briefcasemanager's policy.
        auto spatialModel = bcInfo.m_db->Models().GetModel(spatialModelId);
        ASSERT_EQ(expectedRetainedChannelLockLevel, bcInfo.QueryLockLevel(*jSubj, GetClient()));
        ASSERT_EQ(expectedRetainedChannelLockLevel, bcInfo.QueryLockLevel(*spatialModel, GetClient()));
        ASSERT_EQ(BentleyApi::Dgn::LockLevel::None, bcInfo.QueryLockLevel(bcInfo.m_db->Schemas(), GetClient())) << "Never hold onto schema lock";

        // Verify that the server has registered Subject elements that are in the RepositoryModel.
        auto repositoryLink = bcInfo.m_db->Elements().GetElement(repositoryLinkId);
        ASSERT_TRUE(repositoryLink.IsValid());
        DgnCodeInfo codeInfo;
        EXPECT_EQ(BentleyApi::BSISUCCESS, bcInfo.GetCodeInfo(codeInfo, repositoryLink->GetCode(), GetClient()));
        EXPECT_TRUE(codeInfo.IsUsed());

        // expectCodesInLockedModelsToBeReported = bcInfo.m_db->BriefcaseManager().ShouldReportCodesInLockedModels();  -- NO! It's too late to ask this here. You will get the wrong BriefcaseManager!
        }

    VerifyElementHasCode(bcName, repositoryLinkId, "test3d.dgn", true); // Code should be recorded. This element is not inside the Job channel.
    VerifyElementHasCode(bcName, jobSubjectId, MSTN_BRIDGE_REG_SUB_KEY_A, true); // Code should be recorded. This element is not inside the Job channel.
    VerifyElementHasCode(bcName, DgnElementId(spatialModelId.GetValue()), "Test3d", true /* expectCodesInLockedModelsToBeReported */); // While this Code should NOT be recorded because this element is inside the Job channel, we don't yet have enough control over revision manager to make that true.

    // Add a line using MODEL-SCOPED CODES, where the model is one that is exclusively owned by the bridge.
    Utf8CP prefix = nullptr;
    int64_t srcId = AddLine(inputFile);
        {
        if (usingIModelBank && !expectCodesInLockedModelsToBeReported)
            createFile(noCodesAllowed);

        ScopedCodeAssignerXDomain assignCodes(spatialModelId, prefix="Model");

        iModelBridgeFwk fwk;
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argvMaker.GetArgC(), argvMaker.GetArgV()));
        ASSERT_EQ(0, fwk.Run(argvMaker.GetArgC(), argvMaker.GetArgV()));
        }

    VerifyConvertedElementHasCode(prevCount, bcName, srcId, prefix, expectCodesInLockedModelsToBeReported);
    ++prevCount;

    // Add a line using RELATED-ELEMENT-SCOPED CODES, where the related element is exclusively owned by the bridge.
    srcId = AddLine(inputFile);
        {
        if (usingIModelBank && !expectCodesInLockedModelsToBeReported)
            createFile(noCodesAllowed);

        ScopedCodeAssignerXDomain assignCodes(jobSubjectId, prefix="Related", CodeScope::Related);

        iModelBridgeFwk fwk;
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argvMaker.GetArgC(), argvMaker.GetArgV()));
        ASSERT_EQ(0, fwk.Run(argvMaker.GetArgC(), argvMaker.GetArgV()));
        }

    VerifyConvertedElementHasCode(prevCount, bcName, srcId, prefix, expectCodesInLockedModelsToBeReported);
    ++prevCount;

    // Add a line using RELATED-ELEMENT-SCOPED CODES, where the related is NOT exclusively owned by the bridge. *** Removed this case - a bridge may not do this during the data phase.
    // srcId = AddLine(inputFile);
    //     {
    //     if (usingIModelBank)
    //         BeFileName::BeDeleteFile(noCodesAllowed.c_str());

    //     ScopedCodeAssignerXDomain assignCodes(jobSubjectId, prefix="RelatedNotLocked", CodeScope::Related);

    //     iModelBridgeFwk fwk;
    //     ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argvMaker.GetArgC(), argvMaker.GetArgV()));
    //     ASSERT_EQ(0, fwk.Run(argvMaker.GetArgC(), argvMaker.GetArgV()));
    //     }

    // VerifyConvertedElementHasCode(prevCount, bcName, srcId, prefix, true);
    // ++prevCount;

    // Add a line using REPOSITORY-SCOPED CODES -- note that we must still provide a scope element id that is exclusively owned by the bridge for codes added in the data phase.. Changing the code scope type does not remove this constraint.
    srcId = AddLine(inputFile);
        {
        if (usingIModelBank && !expectCodesInLockedModelsToBeReported)
            createFile(noCodesAllowed);

        ScopedCodeAssignerXDomain assignCodes(jobSubjectId, prefix="Repository", CodeScope::Repository);

        iModelBridgeFwk fwk;
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argvMaker.GetArgC(), argvMaker.GetArgV()));
        ASSERT_EQ(0, fwk.Run(argvMaker.GetArgC(), argvMaker.GetArgV()));
        }

    VerifyConvertedElementHasCode(prevCount, bcName, srcId, prefix, expectCodesInLockedModelsToBeReported);
    ++prevCount;


    } // ~runningServer => stop Server
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTests::SetupTwoRefs(bvector<WString>& args, BentleyApi::BeFileName& masterFile, BentleyApi::BeFileName& refFile1, BentleyApi::BeFileName& refFile2, BentleyApi::BeFileName const& testDir, FakeRegistry& testRegistry)
    {
    MakeCopyOfFile(masterFile, L"Test3d.dgn", NULL);
    AddLine(masterFile);

    MakeCopyOfFile(refFile1, L"Test3d.dgn", L"-Ref-1");
    AddLine(refFile1);

    MakeCopyOfFile(refFile2, L"Test3d.dgn", L"-Ref-2");
    AddLine(refFile2);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", masterFile.c_str()));

    BentleyApi::BeSQLite::BeGuid guid, ref1Guid, ref2Guid;
    guid.Create();
    ref1Guid.Create();
    ref2Guid.Create();

    iModelBridgeDocumentProperties docProps(guid.ToString().c_str(), "wurn1", "durn1", "other1", "");
    iModelBridgeDocumentProperties ref1DocProps(ref1Guid.ToString().c_str(), "wurn21", "durn21", "other21", "");
    iModelBridgeDocumentProperties ref2DocProps(ref2Guid.ToString().c_str(), "wurn22", "durn22", "other22", "");
    testRegistry.SetDocumentProperties(docProps, masterFile);
    testRegistry.SetDocumentProperties(ref1DocProps, refFile1);
    testRegistry.SetDocumentProperties(ref2DocProps, refFile2);

    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, masterFile, L"");
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, refFile1, L"");
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, refFile2, L"");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, DISABLED_PushAfterEachModel)
    {
    auto testDir = CreateTestDir();

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    // Set up to process a master file and two reference files, all mapped to MstnBridge
    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, DEFAULT_IMODEL_NAME_A);
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer, const size_t bufferSize, iModelBridgeAffinityLevel& affinityLevel,
                                                           BentleyApi::WCharCP affinityLibraryPath, BentleyApi::WCharCP sourceFileName)
        {
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        };
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);

    BentleyApi::BeFileName masterFile, refFile1, refFile2;
    SetupTwoRefs(args, masterFile, refFile1, refFile2, testDir, testRegistry);

    testRegistry.Save();

    int modelCount = 0;
    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);

        modelCount = DbFileInfo(m_briefcaseName).GetModelCount();
        ASSERT_EQ(8, modelCount);
        }

    // Add two attachments => two new models should be discovered.
    AddAttachment(masterFile, refFile1, 1, true);
    AddAttachment(masterFile, refFile2, 1, true);
    if (true)
        {
        auto csCountBefore = GetChangesetCount();
        RunTheBridge(args);
        DbFileInfo fileInfo(m_briefcaseName);
        EXPECT_EQ(modelCount + 2, fileInfo.GetModelCount());
        // There will be more than just the two model-specific changesets. Assert at least 2 more.
        EXPECT_GE(GetChangesetCount(), csCountBefore + 2) << "each model should have been pushed in its own changeset";

        auto revstats = ComputeRevisionStats(*fileInfo.m_db, csCountBefore);
        EXPECT_EQ(revstats.nSchemaRevs, 0) << "no schema changes expected, since the initial conversion did that";
        EXPECT_GE(revstats.nDataRevs, 2) << "each model should have been pushed in its own changeset";
        EXPECT_TRUE(revstats.descriptions.find("Test3d-Ref-1") != revstats.descriptions.end()) << "first ref should have been pushed in its own revision";
        EXPECT_TRUE(revstats.descriptions.find("Test3d-Ref-2") != revstats.descriptions.end()) << "second ref should have been pushed in its own revision";
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, PushAfterEachFile)
    {
    auto testDir = CreateTestDir();

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    // Set up to process a master file and two reference files, all mapped to MstnBridge
    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, DEFAULT_IMODEL_NAME_A);
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer, const size_t bufferSize, iModelBridgeAffinityLevel& affinityLevel,
                                                           BentleyApi::WCharCP affinityLibraryPath, BentleyApi::WCharCP sourceFileName)
        {
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        };
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);

    BentleyApi::BeFileName masterFile, refFile1, refFile2;
    SetupTwoRefs(args, masterFile, refFile1, refFile2, testDir, testRegistry);

    testRegistry.Save();

    int modelCount = 0;
    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);

        modelCount = DbFileInfo(m_briefcaseName).GetModelCount();
        ASSERT_EQ(8, modelCount);
        }

    CleanupElementECExtensions();
    // Add two attachments => two new models should be discovered.
    AddAttachment(masterFile, refFile1, 1, true);
    AddAttachment(masterFile, refFile2, 1, true);
    if (true)
        {
        auto csCountBefore = GetChangesetCount();
        RunTheBridge(args);
        DbFileInfo fileInfo(m_briefcaseName);
        EXPECT_EQ(modelCount + 2, fileInfo.GetModelCount());
        // There will be more than just the two file-specific changesets. Assert at least 2 more.
        EXPECT_GE(GetChangesetCount(), csCountBefore + 2) << "each model should have been pushed in its own changeset";

        auto revstats = ComputeRevisionStats(*fileInfo.m_db, csCountBefore);
        EXPECT_EQ(revstats.nSchemaRevs, 0) << "no schema changes expected, since the initial conversion did that";
        EXPECT_GE(revstats.nDataRevs, 2) << "each file should have been pushed in its own changeset";
        EXPECT_TRUE(revstats.fileNames.end() != revstats.fileNames.find (Utf8String("Test3d-Ref-1.dgn"))) << "first ref should have been pushed in its own revision";
        EXPECT_TRUE(revstats.fileNames.end() != revstats.fileNames.find(Utf8String("Test3d-Ref-2.dgn"))) << "second ref should have been pushed in its own revision";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, CreateSnapshot)
    {
    auto testDir = CreateTestDir();

    // Set up to process a master file and two reference files, all mapped to MstnBridge
    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

    WCharCP snapshotName = L"Test Snap shot";
    WCharCP expectedRegistryDbName = L"Test Snap shot.fwk-registry.db";

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, Utf8String(snapshotName));
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer, const size_t bufferSize, iModelBridgeAffinityLevel& affinityLevel,
                                                           BentleyApi::WCharCP affinityLibraryPath, BentleyApi::WCharCP sourceFileName)
        {
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        };
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);

    BentleyApi::BeFileName masterFile, refFile1, refFile2;
    SetupTwoRefs(args, masterFile, refFile1, refFile2, testDir, testRegistry);
    args.push_back(WPrintfString(L"--fwk-snapshot=\"%ls\"", snapshotName));

    testRegistry.Save();

    // Ask the framework to run our test bridge to do the initial conversion and create the repo
    RunTheBridge(args);
    CleanupElementECExtensions();

    args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
    RunTheBridge(args);
    CleanupElementECExtensions();

    if (true)
        {
        DbFileInfo dbFileInfo(m_briefcaseName);
        ASSERT_TRUE(dbFileInfo.m_db->IsSnapshot());
        ASSERT_FALSE(dbFileInfo.m_db->Txns().HasPendingTxns());
        auto modelCount = dbFileInfo.GetModelCount();
        ASSERT_EQ(8, modelCount);

        bvector<BeFileName> matches;
        BeDirectoryIterator::WalkDirsAndMatch(matches, testDir, L"*.db", true);
        ASSERT_EQ(2, matches.size());
        auto x = std::find_if(matches.begin(), matches.end(), [&](BeFileNameCR fn) {return fn.EndsWith(expectedRegistryDbName);});
        ASSERT_TRUE(x != matches.end());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      04/202
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(MstnBridgeTests, CreateSnapshotTwoBridges)
    {
    auto testDir = CreateTestDir();

    auto master1 = GetOutputFileName(L"master1.i.dgn");      // These are the names of the staged input files
    auto master2 = GetOutputFileName(L"master2.i.dgn");      //          "
    BeFileName commonRef(L"commonRef.i.dgn");                   // The common embedded file will be identified by its basename only

    WCharCP snapshotName = L"CreateSnapshotTwoBridges";

    putenv("iModelBridge_MatchOnEmbeddedFileBasename=1");     // TODO: Replace this with a settings service parameter check

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, Utf8String(snapshotName));
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, iModelBridge_getAffinity);

    BentleyApi::BeSQLite::BeGuid guid1, guid2, guidC;
    guid1.Create();
    guid2.Create();
    guidC.Create();

    iModelBridgeDocumentProperties docProps1(guid1.ToString().c_str(), "wurn1", "durn1", "other1", "");
    iModelBridgeDocumentProperties docProps2(guid2.ToString().c_str(), "wurn2", "durn2", "other2", "");
    iModelBridgeDocumentProperties docPropsC(guidC.ToString().c_str(), "wurnC", "durnC", "otherC", "");
    testRegistry.SetDocumentProperties(docProps1, master1);
    testRegistry.SetDocumentProperties(docProps2, master2);
    testRegistry.SetDocumentProperties(docPropsC, commonRef);
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
                                                           const size_t bufferSize,
                                                           iModelBridgeAffinityLevel& affinityLevel,
                                                           BentleyApi::WCharCP affinityLibraryPath,
                                                           BentleyApi::WCharCP sourceFileName)
        {
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        };

    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, master1, L"");
    ASSERT_TRUE(bridgeName.Equals(MSTN_BRIDGE_REG_SUB_KEY));
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, master2, L"");
    ASSERT_TRUE(bridgeName.Equals(MSTN_BRIDGE_REG_SUB_KEY));
    testRegistry.Save();
    TerminateHost();

    int modelCount = 0;
    DgnElementId master1JobSubjectId;
    DgnElementId master2JobSubjectId;
    if (true)
        {
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, snapshotName);

        args.push_back(WPrintfString(L"--fwk-snapshot=\"%ls\"", snapshotName));

        auto master_1_v3 = GetTestDataFileName(L"SharedEmbeddedReferences/v3/master1.i.dgn"); // no embedded ref
        auto master_2_v1 = GetTestDataFileName(L"SharedEmbeddedReferences/v1/master2.i.dgn"); // embedded ref

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_1_v3.c_str(), master1.c_str()));
        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_2_v1.c_str(), master2.c_str()));

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master1.c_str()));
        RunTheBridge(args);
        master1JobSubjectId = m_jobSubjectId;

        CleanupElementECExtensions();
        args.pop_back();
        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master2.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();
        master2JobSubjectId = m_jobSubjectId;

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo dbInfo(m_briefcaseName);
        modelCount = dbInfo.GetPhysicalModelCount();
        ASSERT_EQ(3, modelCount);
        
        auto references = dbInfo.GetReferencesSubjects();
        ASSERT_EQ(1, references.size());
        ASSERT_TRUE(dbInfo.IsChildOf(references[0], master2JobSubjectId));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, DISABLED_OidcTest)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY);

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));
    args.push_back(L"--fwk-skip-assignment-check");

    iModelBridgeFwk fwk;
    bvector<WCharCP> argptrs;

    for (auto& arg : args)
        argptrs.push_back(arg.c_str());

    int argc = (int) argptrs.size();
    wchar_t const** argv = argptrs.data();

    ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));

    ASSERT_EQ(0, fwk.Run(argc, argv));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, DISABLED_TestCodeRemovalPerformance)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY);

    args.push_back(L"--set-DebugCodes");

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));
    args.push_back(L"--fwk-skip-assignment-check");

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    int64_t srcId = AddLine(inputFile,200000);
    if (true)
        {
        // and run an update
        StopWatch watch(true);
        RunTheBridge(args);
        watch.Stop();
        //Utf8PrintfString message("Importing RunTheBridge|%.0f millisecs", schemaTimer.GetElapsedSeconds() * 1000.0);
        LOGTODB("MstnBridgeTests", "TestCodeRemovalPerformance,", watch.GetElapsedSeconds() * 1000.0);

        }
    DbFileInfo info(m_briefcaseName);
    DgnElementId id;
    ASSERT_EQ(SUCCESS, info.GetiModelElementByDgnElementId(id, srcId));

    ASSERT_TRUE(id.IsValid());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTests::DoDetectDeletionsInEmbeddedFiles(bool simulateOldBridge)
    {
    if (nullptr != GetIModelBankServerJs())
        {
        // ??! Including this test in a full run causes all tests that follow to fail to start the IMB server. ??!
        return;
        }

    auto testDir = CreateTestDir();

    putenv("iModelBridge_MatchOnEmbeddedFileBasename=1");     // TODO: Replace this with a settings service parameter check
    putenv("MS_PROTECTION_PASSWORD_CACHE_LIFETIME=0"); // must disable pw caching, as we copy new files on top of old ones too quickly
    static const uint32_t s_v8PasswordCacheLifetime = 2*1000; // the timeout is 1 second. Wait for 2 to be safe.

    BentleyApi::BeFileName inputStagingDir = GetOutputDir();

    auto master1 = GetOutputFileName(L"master1.i.dgn");      // These are the names of the staged input files
    auto master2 = GetOutputFileName(L"master2.i.dgn");      //          "
    BeFileName commonRef(L"commonRef.i.dgn");                   // The common embedded file will be identified by its basename only

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, DEFAULT_IMODEL_NAME_A);
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, iModelBridge_getAffinity);

    BentleyApi::BeSQLite::BeGuid guid1, guid2, guidC;
    guid1.Create();
    guid2.Create();
    guidC.Create();

    iModelBridgeDocumentProperties docProps1(guid1.ToString().c_str(), "wurn1", "durn1", "other1", "");
    iModelBridgeDocumentProperties docProps2(guid2.ToString().c_str(), "wurn2", "durn2", "other2", "");
    // iModelBridgeDocumentProperties docPropsC(guidC.ToString().c_str(), "wurnC", "durnC", "otherC", "");
    testRegistry.SetDocumentProperties(docProps1, master1);
    testRegistry.SetDocumentProperties(docProps2, master2);
    // testRegistry.SetDocumentProperties(docPropsC, commonRef);
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
        const size_t bufferSize,
        iModelBridgeAffinityLevel& affinityLevel,
        BentleyApi::WCharCP affinityLibraryPath,
            BentleyApi::WCharCP sourceFileName)
        {
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        };

    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, master1, L"");
    ASSERT_TRUE(bridgeName.Equals(MSTN_BRIDGE_REG_SUB_KEY));
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, master2, L"");
    ASSERT_TRUE(bridgeName.Equals(MSTN_BRIDGE_REG_SUB_KEY));
    testRegistry.Save();
    TerminateHost();

    int modelCount = 0;
    DgnElementId master1JobSubjectId;
    DgnElementId master2JobSubjectId;
    if (true)
        {
        // In v1, master1 and master2 both embed commonRef
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        if (simulateOldBridge)
            args.push_back(L"--do-not-track-references-subjects");
            
        auto master_1_v1 = GetTestDataFileName(L"SharedEmbeddedReferences/v1/master1.i.dgn");
        auto master_2_v1 = GetTestDataFileName(L"SharedEmbeddedReferences/v1/master2.i.dgn");

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_1_v1.c_str(), master1.c_str()));
        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_2_v1.c_str(), master2.c_str()));

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master1.c_str()));
        RunTheBridge(args);
        master1JobSubjectId = m_jobSubjectId;
        
        CleanupElementECExtensions();
        args.pop_back();
        
        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master2.c_str()));
        RunTheBridge(args);
        master2JobSubjectId = m_jobSubjectId;
        
        CleanupElementECExtensions();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        
        CleanupElementECExtensions();

        DbFileInfo dbInfo(m_briefcaseName);
        modelCount = dbInfo.GetPhysicalModelCount();
        ASSERT_EQ(3, modelCount);
        auto references = dbInfo.GetReferencesSubjects();
        ASSERT_EQ(2, references.size()) << "There should be two References to commonRef";
        ASSERT_TRUE(dbInfo.IsChildOf(references[0], master1JobSubjectId)) << "Since master1 was the first to see commonRef, it is the parent";
        }

    if (true)
        {
        // In v2, master1 still embeds commonRef, but master2 has dropped it.
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        if (simulateOldBridge)
            args.push_back(L"--do-not-track-references-subjects");

        auto master_1_v2 = GetTestDataFileName(L"SharedEmbeddedReferences/v2/master1.i.dgn");
        auto master_2_v2 = GetTestDataFileName(L"SharedEmbeddedReferences/v2/master2.i.dgn");

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_1_v2.c_str(), master1.c_str()));
        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_2_v2.c_str(), master2.c_str()));

        // NEEDS WORK temporary work-around for V8 password cache entry lifetime
        BentleyApi::BeThreadUtilities::BeSleep(s_v8PasswordCacheLifetime);

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master1.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();

        args.pop_back();
        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master2.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo dbInfo(m_briefcaseName);
        ASSERT_EQ(modelCount, dbInfo.GetPhysicalModelCount()) << "commonRef should still be in the iModel";
        auto references = dbInfo.GetReferencesSubjects();
        if (simulateOldBridge)
            {
            // This is how old bridges used to leave it. There is an orphan References Subject still in the iModel.
            ASSERT_EQ(2, references.size());
            }
        else
            {
            ASSERT_EQ(1, references.size()) << "There should be just one Reference to commonRef left";
            ASSERT_TRUE(dbInfo.IsChildOf(references[0], master1JobSubjectId)) << "The Reference to commonRef should be from master1";
            }
        }

     if (true)
        {
        // In v3, master1 has dropped its reference to commonRef, and so commonRef should be removed from the iModel.
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        if (simulateOldBridge)
            args.push_back(L"--do-not-track-references-subjects");

        auto master_1_v3 = GetTestDataFileName(L"SharedEmbeddedReferences/v3/master1.i.dgn");
        auto master_2_v3 = GetTestDataFileName(L"SharedEmbeddedReferences/v3/master2.i.dgn");

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_1_v3.c_str(), master1.c_str()));
        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_2_v3.c_str(), master2.c_str()));

        // NEEDS WORK temporary work-around for V8 password cache entry lifetime
        BentleyApi::BeThreadUtilities::BeSleep(s_v8PasswordCacheLifetime);

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master1.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();

        args.pop_back();
        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master2.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);

        DbFileInfo dbInfo(m_briefcaseName);
        ASSERT_EQ(modelCount - 1, dbInfo.GetPhysicalModelCount()) << "commonRef should have been deleted";
        auto references = dbInfo.GetReferencesSubjects();
        if (simulateOldBridge)
            {
            // This is how old bridges used to leave it. There is an orphan References Subject still in the iModel.
            ASSERT_EQ(2, references.size());
            }
        else
            {
            ASSERT_EQ(0, references.size()) << "There should be no References to commonRef left";
            }
        }

    if (simulateOldBridge)
        {
        // In the steps above, the bridges ran as though they had the old logic that neglected to track
        // References Subjects with ExternalSourceAspects.
        // Now run the bridges again using the new logic that both tracks Refs Subjects and also cleans
        // up orphan Refs Subjects that were left over by old bridges. In this test, the above bridges
        // left both master1->commonRef and master2->commonRef Refs Subjects in place. We expect the "new" bridges to detect and delete them.
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master1.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();
        ASSERT_EQ(master1JobSubjectId.GetValue(), m_jobSubjectId.GetValue());

        args.pop_back();

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master2.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();
        ASSERT_EQ(master2JobSubjectId.GetValue(), m_jobSubjectId.GetValue());

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo dbInfo(m_briefcaseName);
        ASSERT_EQ(modelCount - 1, dbInfo.GetPhysicalModelCount()) << "commonRef should still be gone";

        auto references = dbInfo.GetReferencesSubjects();
        ASSERT_EQ(0, references.size()) << "There should be no References to commonRef left";
        }

    putenv("MS_PROTECTION_PASSWORD_CACHE_LIFETIME=1"); // restore default
    putenv("iModelBridge_MatchOnEmbeddedFileBasename=");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, DetectDeletionsInEmbeddedFiles)
    {
    DoDetectDeletionsInEmbeddedFiles(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, DetectDeletionsInEmbeddedFilesWithOrphanCleanup)
    {
    DoDetectDeletionsInEmbeddedFiles(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2020
//---------------+---------------+---------------+---------------+---------------+-------
void MstnBridgeTests::DoMoveEmbeddedReferenceToDifferentFile(bool simulateOldBridge)
    {
    auto testDir = CreateTestDir();

    auto master1 = GetOutputFileName(L"master1.i.dgn");      // These are the names of the staged input files
    auto master2 = GetOutputFileName(L"master2.i.dgn");      //          "
    BeFileName commonRef(L"commonRef.i.dgn");                   // The common embedded file will be identified by its basename only

    putenv("iModelBridge_MatchOnEmbeddedFileBasename=1");     // TODO: Replace this with a settings service parameter check

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, DEFAULT_IMODEL_NAME_A);
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, iModelBridge_getAffinity);

    BentleyApi::BeSQLite::BeGuid guid1, guid2, guidC;
    guid1.Create();
    guid2.Create();
    guidC.Create();

    iModelBridgeDocumentProperties docProps1(guid1.ToString().c_str(), "wurn1", "durn1", "other1", "");
    iModelBridgeDocumentProperties docProps2(guid2.ToString().c_str(), "wurn2", "durn2", "other2", "");
    iModelBridgeDocumentProperties docPropsC(guidC.ToString().c_str(), "wurnC", "durnC", "otherC", "");
    testRegistry.SetDocumentProperties(docProps1, master1);
    testRegistry.SetDocumentProperties(docProps2, master2);
    testRegistry.SetDocumentProperties(docPropsC, commonRef);
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
                                                           const size_t bufferSize,
                                                           iModelBridgeAffinityLevel& affinityLevel,
                                                           BentleyApi::WCharCP affinityLibraryPath,
                                                           BentleyApi::WCharCP sourceFileName)
        {
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        };

    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, master1, L"");
    ASSERT_TRUE(bridgeName.Equals(MSTN_BRIDGE_REG_SUB_KEY));
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, master2, L"");
    ASSERT_TRUE(bridgeName.Equals(MSTN_BRIDGE_REG_SUB_KEY));
    testRegistry.Save();
    TerminateHost();

    int modelCount = 0;
    DgnElementId master1JobSubjectId;
    DgnElementId master2JobSubjectId;
    if (true)
        {
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        if (simulateOldBridge)
            args.push_back(L"--do-not-track-references-subjects");

        auto master_1_v3 = GetTestDataFileName(L"SharedEmbeddedReferences/v3/master1.i.dgn"); // no embedded ref
        auto master_2_v1 = GetTestDataFileName(L"SharedEmbeddedReferences/v1/master2.i.dgn"); // embedded ref

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_1_v3.c_str(), master1.c_str()));
        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_2_v1.c_str(), master2.c_str()));

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master1.c_str()));
        RunTheBridge(args);
        master1JobSubjectId = m_jobSubjectId;

        CleanupElementECExtensions();
        args.pop_back();
        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master2.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();
        master2JobSubjectId = m_jobSubjectId;

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo dbInfo(m_briefcaseName);
        modelCount = dbInfo.GetPhysicalModelCount();
        ASSERT_EQ(3, modelCount);
        
        auto references = dbInfo.GetReferencesSubjects();
        ASSERT_EQ(1, references.size());
        ASSERT_TRUE(dbInfo.IsChildOf(references[0], master2JobSubjectId));
        }

    if (true)
        {
        // In v2, master1 embeds commonRef, but master2 has dropped it.
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        if (simulateOldBridge)
            args.push_back(L"--do-not-track-references-subjects");

        auto master_1_v2 = GetTestDataFileName(L"SharedEmbeddedReferences/v2/master1.i.dgn");
        auto master_2_v2 = GetTestDataFileName(L"SharedEmbeddedReferences/v2/master2.i.dgn");

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_1_v2.c_str(), master1.c_str()));
        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_2_v2.c_str(), master2.c_str()));

        // Process master2 first. That will cause commonRef to be orphaned temporarily.
        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master2.c_str()));
        RunTheBridge(args);
        args.pop_back();
        CleanupElementECExtensions();
        ASSERT_EQ(master2JobSubjectId.GetValue(), m_jobSubjectId.GetValue());

        // When we process master1, it should pick up the orphan.
        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master1.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();
        ASSERT_EQ(master1JobSubjectId.GetValue(), m_jobSubjectId.GetValue());

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo dbInfo(m_briefcaseName);
        ASSERT_EQ(modelCount, dbInfo.GetPhysicalModelCount()) << "commonRef should still be in the iModel";

        auto references = dbInfo.GetReferencesSubjects();
        if (simulateOldBridge)
            {
            // This is how old bridges used to leave it. There is an orphan References Subject still in the iModel.
            ASSERT_EQ(2, references.size());
            }
        else
            {
            ASSERT_EQ(1, references.size());
            ASSERT_TRUE(dbInfo.IsChildOf(references[0], master1JobSubjectId));
            }
        }

    if (simulateOldBridge)
        {
        // In the steps above, the bridges ran as though they had the old logic that neglected to track
        // References Subjects with ExternalSourceAspects.
        // Now run the bridges again using the new logic that both tracks Refs Subjects and also cleans
        // up orphan Refs Subjects that were left over by old bridges. In this test, the above bridges
        // left the master2->commonRef Refs Subject in place. We expect the "new" bridge to detect and delete that.
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master1.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();
        ASSERT_EQ(master1JobSubjectId.GetValue(), m_jobSubjectId.GetValue());

        args.pop_back();

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", master2.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();
        ASSERT_EQ(master2JobSubjectId.GetValue(), m_jobSubjectId.GetValue());

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo dbInfo(m_briefcaseName);
        ASSERT_EQ(modelCount, dbInfo.GetPhysicalModelCount()) << "commonRef should still be in the iModel";

        auto references = dbInfo.GetReferencesSubjects();
        ASSERT_EQ(1, references.size()) << "The orphan References Subject should have been detected and deleted";
        ASSERT_TRUE(dbInfo.IsChildOf(references[0], master1JobSubjectId));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2020
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(MstnBridgeTests, MoveEmbeddedReferenceToDifferentFile)
    {
    DoMoveEmbeddedReferenceToDifferentFile(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2020
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(MstnBridgeTests, MoveEmbeddedReferenceToDifferentFileWithOrphanCleanup)
    {
    DoMoveEmbeddedReferenceToDifferentFile(true);
    }

static Json::Value collectElementAndChildren(DgnDbR db, DgnElementId elementId)
    {
    auto el = db.Elements().GetElement(elementId);

    Json::Value json = Json::objectValue;
    json["class"] = el->GetElementClass()->GetFullName();
    json["code"] = el->GetCode().GetValueUtf8CP();

    json["Children"] = Json::arrayValue;
    for (auto child : el->QueryChildren())
        {
        auto c = collectElementAndChildren(db, child);
        json["Children"].append(c);
        }
    return json;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                       02/2020
//---------------+---------------+---------------+---------------+---------------+-------
void MstnBridgeTests::DoAddRemoveNestedEmbeddedRefs(bool simulateOldBridge)
    {
    auto testDir = CreateTestDir();

    auto masterfile = GetOutputFileName(L"nestedrefs.i.dgn");      // The name of the staged input file

    putenv("iModelBridge_MatchOnEmbeddedFileBasename=1");     // TODO: Replace this with a settings service parameter check

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, DEFAULT_IMODEL_NAME_A);
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, iModelBridge_getAffinity);

    BentleyApi::BeSQLite::BeGuid guid1, guid2, guidC;
    guid1.Create();
    guid2.Create();
    guidC.Create();

    iModelBridgeDocumentProperties docProps1(guid1.ToString().c_str(), "wurn1", "durn1", "other1", "");
    testRegistry.SetDocumentProperties(docProps1, masterfile);
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
                                                           const size_t bufferSize,
                                                           iModelBridgeAffinityLevel& affinityLevel,
                                                           BentleyApi::WCharCP affinityLibraryPath,
                                                           BentleyApi::WCharCP sourceFileName)
        {
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        };

    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, masterfile, L"");
    ASSERT_TRUE(bridgeName.Equals(MSTN_BRIDGE_REG_SUB_KEY));
    testRegistry.Save();
    TerminateHost();

    int modelCount = 0;
    DgnElementId jobSubjectId;
    Json::Value v1Json;
    if (true)
        {
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        if (simulateOldBridge)
            args.push_back(L"--do-not-track-references-subjects");

        auto master_v1 = GetTestDataFileName(L"nestedrefs-v1.i.dgn");

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_v1.c_str(), masterfile.c_str()));

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", masterfile.c_str()));
        RunTheBridge(args);
        jobSubjectId = m_jobSubjectId;

        CleanupElementECExtensions();
        args.pop_back();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo dbInfo(m_briefcaseName);
        v1Json = collectElementAndChildren(*dbInfo.m_db, jobSubjectId);
        }

    Json::Value v2Json;
    if (true)
        {
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        if (simulateOldBridge)
            args.push_back(L"--do-not-track-references-subjects");

        auto master_v2 = GetTestDataFileName(L"nestedrefs-v2.i.dgn");

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_v2.c_str(), masterfile.c_str()));

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", masterfile.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();
        ASSERT_EQ(jobSubjectId.GetValue(), m_jobSubjectId.GetValue());

        args.pop_back();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo dbInfo(m_briefcaseName);
        v2Json = collectElementAndChildren(*dbInfo.m_db, jobSubjectId);
        }

    if (simulateOldBridge)
        {
        // In the steps above, the bridges ran as though they had the old logic that neglected to track
        // References Subjects with ExternalSourceAspects.
        // Now run the bridges again using the new logic that both tracks Refs Subjects and also cleans
        // up orphan Refs Subjects that were left over by old bridges. In this test, the above bridges
        // left the master2->commonRef Refs Subject in place. We expect the "new" bridge to detect and delete that.
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", masterfile.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();
        ASSERT_EQ(jobSubjectId.GetValue(), m_jobSubjectId.GetValue());

        args.pop_back();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo dbInfo(m_briefcaseName);
        // ASSERT_EQ(modelCount, dbInfo.GetPhysicalModelCount()) << "commonRef should still be in the iModel";

        v2Json = collectElementAndChildren(*dbInfo.m_db, jobSubjectId);
        }
  
    // printf("----------------------- v1 --------------------------\n%s\n", v1Json.toStyledString().c_str());
    // printf("----------------------- v2 --------------------------\n%s\n", v2Json.toStyledString().c_str());

    auto expected_master_v1 = readJsonFromFile(GetTestDataFileName(L"nestedrefs-v1.json"));
    auto expected_master_v2 = readJsonFromFile(GetTestDataFileName(L"nestedrefs-v2.json"));

    EXPECT_TRUE(expected_master_v1 == v1Json);
    EXPECT_TRUE(expected_master_v2 == v2Json);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      02/20
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(MstnBridgeTests, AddRemoveNestedEmbeddedRefs)
    {
    DoAddRemoveNestedEmbeddedRefs(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      02/20
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(MstnBridgeTests, AddRemoveNestedEmbeddedRefsOldBridge)
    {
    DoAddRemoveNestedEmbeddedRefs(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                       02/2020
//---------------+---------------+---------------+---------------+---------------+-------
void MstnBridgeTests::DoAddRemoveNestedEmbeddedRefsV2(bool simulateOldBridge)
    {
    auto testDir = CreateTestDir();

    auto masterfile = GetOutputFileName(L"Master_nested.i.dgn");      // The name of the staged input file

    putenv("iModelBridge_MatchOnEmbeddedFileBasename=1");     // TODO: Replace this with a settings service parameter check

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, DEFAULT_IMODEL_NAME_A);
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, iModelBridge_getAffinity);

    BentleyApi::BeSQLite::BeGuid guid1, guid2, guidC;
    guid1.Create();
    guid2.Create();
    guidC.Create();

    iModelBridgeDocumentProperties docProps1(guid1.ToString().c_str(), "wurn1", "durn1", "other1", "");
    testRegistry.SetDocumentProperties(docProps1, masterfile);
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
                                                           const size_t bufferSize,
                                                           iModelBridgeAffinityLevel& affinityLevel,
                                                           BentleyApi::WCharCP affinityLibraryPath,
                                                           BentleyApi::WCharCP sourceFileName)
        {
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        };

    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, masterfile, L"");
    ASSERT_TRUE(bridgeName.Equals(MSTN_BRIDGE_REG_SUB_KEY));
    testRegistry.Save();
    TerminateHost();

    int modelCount = 0;
    DgnElementId jobSubjectId;
    Json::Value v1Json;
    if (true)
        {
        // In version 1, we have master -> ref1 -> model1,nestedv1 (has a circle)
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        if (simulateOldBridge)
            args.push_back(L"--do-not-track-references-subjects");

        auto master_v1 = GetTestDataFileName(L"Master_nested_v1.i.dgn");

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_v1.c_str(), masterfile.c_str()));

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", masterfile.c_str()));
        RunTheBridge(args);
        jobSubjectId = m_jobSubjectId;

        CleanupElementECExtensions();
        args.pop_back();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo dbInfo(m_briefcaseName);
        v1Json = collectElementAndChildren(*dbInfo.m_db, jobSubjectId);
        }

    Json::Value v2Json;
    if (true)
        {
        // In version 2, we have master -> ref1 -> model1,nestedv2 (has a square)           Note that the reference is to a different file. The referenced model has the same name as the ref in version 1.
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        if (simulateOldBridge)
            args.push_back(L"--do-not-track-references-subjects");

        auto master_v2 = GetTestDataFileName(L"Master_nested_v2.i.dgn");

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_v2.c_str(), masterfile.c_str()));

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", masterfile.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();
        ASSERT_EQ(jobSubjectId.GetValue(), m_jobSubjectId.GetValue());

        args.pop_back();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo dbInfo(m_briefcaseName);
        v2Json = collectElementAndChildren(*dbInfo.m_db, jobSubjectId);
        }

    if (simulateOldBridge)
        {
        // In the steps above, the bridges ran as though they had the old logic that neglected to track
        // References Subjects with ExternalSourceAspects.
        // Now run the bridges again using the new logic that both tracks Refs Subjects and also cleans
        // up orphan Refs Subjects that were left over by old bridges. In this test, the above bridges
        // left the master2->commonRef Refs Subject in place. We expect the "new" bridge to detect and delete that.
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", masterfile.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();
        ASSERT_EQ(jobSubjectId.GetValue(), m_jobSubjectId.GetValue());

        args.pop_back();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo dbInfo(m_briefcaseName);
        v2Json = collectElementAndChildren(*dbInfo.m_db, jobSubjectId);
        }
  
    // printf("----------------------- v1 --------------------------\n%s\n", v1Json.toStyledString().c_str());
    // printf("----------------------- v2 --------------------------\n%s\n", v2Json.toStyledString().c_str());

    auto expected_master_v1 = readJsonFromFile(GetTestDataFileName(L"Master_nested_v1.json"));
    auto expected_master_v2 = readJsonFromFile(GetTestDataFileName(L"Master_nested_v2.json"));

    EXPECT_TRUE(expected_master_v1 == v1Json);
    EXPECT_TRUE(expected_master_v2 == v2Json);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      02/20
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(MstnBridgeTests, AddRemoveNestedEmbeddedRefsV2)
    {
    DoAddRemoveNestedEmbeddedRefsV2(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      02/20
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(MstnBridgeTests, AddRemoveNestedEmbeddedRefsV2OldBridge)
    {
    DoAddRemoveNestedEmbeddedRefsV2(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, DetectCommonReferencesUsingRecipes)
    {
    if (nullptr != GetIModelBankServerJs())
        {
        // ??! Including this test in a full run causes all tests that follow to fail to start the IMB server. ??!
        return;
        }

    auto testDir = CreateTestDir();

    putenv("MS_PROTECTION_PASSWORD_CACHE_LIFETIME=0"); // must disable pw caching, as we copy new files on top of old ones too quickly
    static const uint32_t s_v8PasswordCacheLifetime = 2*1000; // the timeout is 1 second. Wait for 2 to be safe.

    BentleyApi::BeFileName inputStagingDir = GetOutputDir();

    putenv("iModelBridge_MatchOnEmbeddedFileBasename=-v[0-9]$");    // The recipe specifies that any -vn suffix should be stripped when forming internal file identifiers.
    // The recipe also says that file extensions should be stripped, PW URNs should be ignored, and filename case should be ignored. That is the default, when a regex is supplied.

    // In this test:
    // In v0, master embeds commonref.dgn.i.dgn
    // In v1, master embeds commonref-v1.i.dgn. According to the recipe, that should be recognized as referring to the same file as the original commonRef.dgn.i.dgn
    // So, the recipe that we supply above should enable the bridge to recognize that both versions are based on the same reference file, just different versions of it.

    auto masterFileName = GetOutputFileName(L"master.i.dgn");      // The name of the staged input file
    BeFileName commonRef(L"commonref");                            // Because of the recipe, all versions of the reference file will be mapped to this name.

    // This is the full pseudo-filename of the first (embedded) file where the bridge will encounter "commonref".
    BeFileName commonRefEmbeddedFileName_firstOccurence(masterFileName);
    commonRefEmbeddedFileName_firstOccurence.append(L"<1>commonref.dgn.i.dgn");

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, DEFAULT_IMODEL_NAME_A);
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, iModelBridge_getAffinity);

    BentleyApi::BeSQLite::BeGuid guidMaster, guidRef;
    guidMaster.Create();
    guidRef.Create();

    iModelBridgeDocumentProperties docPropsMaster(guidMaster.ToString().c_str(), "wurnMaster", "durnMaster", "otherMaster", "");
    iModelBridgeDocumentProperties docPropsRef(guidRef.ToString().c_str(), "wurnRef", "durnRef", "otherRef", "");
    testRegistry.SetDocumentProperties(docPropsMaster, masterFileName);
    testRegistry.SetDocumentProperties(docPropsRef, commonRef);           // <--------- the reference file will have a doc id. (The recipe will tell the bridge to ignore it.)
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
        const size_t bufferSize,
        iModelBridgeAffinityLevel& affinityLevel,
        BentleyApi::WCharCP affinityLibraryPath,
            BentleyApi::WCharCP sourceFileName)
        {
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        };

    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, masterFileName, L"");
    ASSERT_TRUE(bridgeName.Equals(MSTN_BRIDGE_REG_SUB_KEY));
    testRegistry.Save();
    TerminateHost();

    DgnCode commonRefCode;
    int modelCount = 0;
    if (true)
        {
        // In v0, master embeds commonref.dgn.i.dgn
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        auto master_v0 = GetTestDataFileName(L"master_common.i.dgn");

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_v0.c_str(), masterFileName.c_str()));

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", masterFileName.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo bcInfo(m_briefcaseName);
        modelCount = bcInfo.GetPhysicalModelCount();
        ASSERT_EQ(2, modelCount);
        RepositoryLinkId rid;
        bcInfo.MustFindFileByName(rid, masterFileName, 1);
        bcInfo.MustFindFileByName(rid, commonRefEmbeddedFileName_firstOccurence, 1);

        auto rlink = bcInfo.m_db->Elements().Get<RepositoryLink>(rid);
        ASSERT_TRUE(rlink.IsValid());
        ASSERT_STREQ(rlink->GetUserLabel(), "commonref.dgn.i.dgn");
        commonRefCode = rlink->GetCode();
        }

    if (true)
        {
        // In v1, master embeds commonRef-v1.i.dgn. According to the recipe, that should be recognized as referring to the same file as the original commonRef.dgn.i.dgn
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        auto master_v1 = GetTestDataFileName(L"master_common-v1.i.dgn");

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_v1.c_str(), masterFileName.c_str()));

        // NEEDS WORK temporary work-around for V8 password cache entry lifetime
        BentleyApi::BeThreadUtilities::BeSleep(s_v8PasswordCacheLifetime);

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", masterFileName.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        // No change in model count is expected, as we still have the same two files in v1 as we did in v0
        DbFileInfo bcInfo(m_briefcaseName);
        auto newModelCount = bcInfo.GetPhysicalModelCount();
        ASSERT_EQ(modelCount, newModelCount) << "No change in model count is expected, as we still have the same two files in v1 as we did in v0";
        RepositoryLinkId rid;
        bcInfo.MustFindFileByName(rid, masterFileName, 1);
        bcInfo.MustFindFileByName(rid, commonRefEmbeddedFileName_firstOccurence, 1); // The RepositoryLink still refers to the first occurrence.

        auto rlink = bcInfo.m_db->Elements().Get<RepositoryLink>(rid);
        ASSERT_TRUE(rlink.IsValid());
        ASSERT_STREQ(rlink->GetUserLabel(), "commonref-v1.i.dgn") << "while the reference file's unique ID should not change, its user label should update to match the actual filename";
        auto code2 = rlink->GetCode();
        auto code2JsonStr = code2.ToJson2().toStyledString();
        auto originalJsonStr = commonRefCode.ToJson2().toStyledString();
        ASSERT_STREQ(code2JsonStr.c_str(), originalJsonStr.c_str()) << "the reference file's Code must not change!";
        }

    putenv("MS_PROTECTION_PASSWORD_CACHE_LIFETIME=1"); // restore default
    putenv("iModelBridge_MatchOnEmbeddedFileBasename=");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2019
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(MstnBridgeTests, UpdateDocumentProperties)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, DEFAULT_IMODEL_NAME_A);
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, iModelBridge_getAffinity);

    BentleyApi::BeSQLite::BeGuid guid;
    guid.Create();

    Utf8String json1 = R"json({"Name": "Time", "Value":"1234"})json";
    iModelBridgeDocumentProperties docProps(guid.ToString().c_str(), "wurn1", "durn1", json1.c_str(), "");
    testRegistry.SetDocumentProperties(docProps, inputFile);
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
                                                           const size_t bufferSize,
                                                           iModelBridgeAffinityLevel& affinityLevel,
                                                           BentleyApi::WCharCP affinityLibraryPath,
                                                           BentleyApi::WCharCP sourceFileName)
        {
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        };

    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, inputFile, L"");
    testRegistry.Save();
    TerminateHost();

    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);
        CleanupElementECExtensions();
        }

    Utf8String json2 = R"json({"Name": "Time", "Value":"5678"})json";
    iModelBridgeDocumentProperties docProps2(guid.ToString().c_str(), "wurn1", "durn1", json2.c_str(), "");
    testRegistry.SetDocumentProperties(docProps2, inputFile);
    testRegistry.Save();
    if (true)
        {
        // Run an update.  This should update the document properties
        RunTheBridge(args);
        DbFileInfo info(m_briefcaseName);
        BeSQLite::EC::ECSqlStatement stmt;
        EXPECT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*info.m_db, "SELECT JsonProperties FROM BisCore.RepositoryLink"));
        while (stmt.Step() != BentleyApi::BeSQLite::DbResult::BE_SQLITE_DONE)
            {
            Utf8CP json = stmt.GetValueText(0);
            Utf8String expected = R"json({"DocumentProperties":{"attributes":{"Name":"Time","Value" : "5678"},"desktopURN" : "durn1","webURN" : "wurn1"}})json";
            ASSERT_TRUE(0 == expected.Equals(json)) << expected.c_str() << " not equal to " << json;
            }
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2020
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(MstnBridgeTests, ChangeFileNameShouldUpdateModelName)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, DEFAULT_IMODEL_NAME_A);
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, iModelBridge_getAffinity);

    BentleyApi::BeSQLite::BeGuid guid;
    guid.Create();

    Utf8String json1 = R"json({"Name": "Time", "Value":"1234"})json";
    iModelBridgeDocumentProperties docProps(guid.ToString().c_str(), "wurn1", "durn1", json1.c_str(), "");
    testRegistry.SetDocumentProperties(docProps, inputFile);
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
                                                           const size_t bufferSize,
                                                           iModelBridgeAffinityLevel& affinityLevel,
                                                           BentleyApi::WCharCP affinityLibraryPath,
                                                           BentleyApi::WCharCP sourceFileName)
        {
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        };

    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, inputFile, L"");
    testRegistry.Save();
    TerminateHost();

    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);
        CleanupElementECExtensions();
        }

    WString baseName = inputFile.GetFileNameWithoutExtension();
    baseName.AppendA("_2");
    BentleyApi::BeFileName newFile(inputFile.GetDirectoryName());
    newFile.AppendToPath(baseName.c_str());
    newFile.AppendExtension(inputFile.GetExtension().c_str());
    BeFileName::BeMoveFile(inputFile.GetName(), newFile.GetName());
    bvector<WString> newArgs;
    SetUpBridgeProcessingArgs(newArgs, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);
    newArgs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", newFile.c_str()));
    testRegistry.SetDocumentProperties(docProps, newFile);
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, newFile, L"");
    testRegistry.Save();

    if (true)
        {
        // Run an update.  This should update the model name
        RunTheBridge(newArgs);
        DbFileInfo info(m_briefcaseName);
            {
            BeSQLite::EC::ECSqlStatement stmt;
            EXPECT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*info.m_db, "SELECT UserLabel FROM BisCore.Subject WHERE Parent.Id=1"));
            bool foundIt = false;
            while (stmt.Step() != BentleyApi::BeSQLite::DbResult::BE_SQLITE_DONE)
                {
                Utf8CP text = stmt.GetValueText(0);
                Utf8String label(text);
                if (label.Equals("Test3d_2"))
                    {
                    foundIt = true;
                    break;
                    }
                }
            ASSERT_TRUE(foundIt);
            }
            {
            BeSQLite::EC::ECSqlStatement stmt;
            EXPECT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*info.m_db, "SELECT CodeValue FROM BisCore.PhysicalPartition"));
            bool foundIt = false;
            while (stmt.Step() != BentleyApi::BeSQLite::DbResult::BE_SQLITE_DONE)
                {
                Utf8String code(stmt.GetValueText(0));
                if (code.Equals("Test3d_2"))
                    {
                    foundIt = true;
                    break;
                    }
                }
            ASSERT_TRUE(foundIt);
            }
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2020
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(MstnBridgeTests, ChangeFileNameShouldUpdateModelName2d)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test2d.dgn", NULL);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName = iModelBridgeRegistry::MakeDbName(testDir, DEFAULT_IMODEL_NAME_A);
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, iModelBridge_getAffinity);

    BentleyApi::BeSQLite::BeGuid guid;
    guid.Create();

    Utf8String json1 = R"json({"Name": "Time", "Value":"1234"})json";
    iModelBridgeDocumentProperties docProps(guid.ToString().c_str(), "wurn1", "durn1", json1.c_str(), "");
    testRegistry.SetDocumentProperties(docProps, inputFile);
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
                                                           const size_t bufferSize,
                                                           iModelBridgeAffinityLevel& affinityLevel,
                                                           BentleyApi::WCharCP affinityLibraryPath,
                                                           BentleyApi::WCharCP sourceFileName)
        {
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        };

    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, inputFile, L"");
    testRegistry.Save();
    TerminateHost();

    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);
        CleanupElementECExtensions();
        }

    WString baseName = inputFile.GetFileNameWithoutExtension();
    baseName.AppendA("_2");
    BentleyApi::BeFileName newFile(inputFile.GetDirectoryName());
    newFile.AppendToPath(baseName.c_str());
    newFile.AppendExtension(inputFile.GetExtension().c_str());
    BeFileName::BeMoveFile(inputFile.GetName(), newFile.GetName());
    bvector<WString> newArgs;
    SetUpBridgeProcessingArgs(newArgs, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);
    newArgs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", newFile.c_str()));
    testRegistry.SetDocumentProperties(docProps, newFile);
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, newFile, L"");
    testRegistry.Save();

    if (true)
        {
        // Run an update.  This should update the model name
        RunTheBridge(newArgs);
        DbFileInfo info(m_briefcaseName);
        {
        BeSQLite::EC::ECSqlStatement stmt;
        EXPECT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*info.m_db, "SELECT UserLabel FROM BisCore.Subject WHERE Parent.Id=1"));
        bool foundIt = false;
        while (stmt.Step() != BentleyApi::BeSQLite::DbResult::BE_SQLITE_DONE)
            {
            Utf8String label(stmt.GetValueText(0));
            if (label.Equals("Test2d_2"))
                {
                foundIt = true;
                break;
                }
            }
        ASSERT_TRUE(foundIt);
        }
        {
        BeSQLite::EC::ECSqlStatement stmt;
        EXPECT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*info.m_db, "SELECT CodeValue FROM BisCore.Drawing"));
        bool foundIt = false;
        while (stmt.Step() != BentleyApi::BeSQLite::DbResult::BE_SQLITE_DONE)
            {
            Utf8String code(stmt.GetValueText(0));
            if (code.Equals("Test2d_2"))
                {
                foundIt = true;
                break;
                }
            }
        ASSERT_TRUE(foundIt);
        }
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static bool jsonHasMember(Utf8StringCR jsonStr, Utf8CP memberName)
    {
    auto json = Json::Value::From(jsonStr);
    if (json.isNull())
        return false;
    return json.isMember(memberName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, TestRegistryUpgrade)
    {
    auto testDir = CreateTestDir();

    BeFileName inputDbName = iModelBridgeRegistry::MakeDbName(GetTestDataDir(), "v1_0");
    BeFileName outputDbName = iModelBridgeRegistry::MakeDbName(testDir, "v1_0");

    if (outputDbName.DoesPathExist())
        outputDbName.BeDeleteFile();
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(inputDbName.c_str(), outputDbName.c_str()));

    static BeSQLite::PropertySpec s_schemaVerPropSpec("SchemaVersion", "be_iModelBridgeFwk", BeSQLite::PropertySpec::Mode::Normal, BeSQLite::PropertySpec::Compress::No);

    if (true)
        {
        BeSQLite::Db db;
        ASSERT_EQ(BeSQLite::BE_SQLITE_OK, db.OpenBeSQLiteDb(outputDbName, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::Readonly)));
        Utf8String propStr;
        ASSERT_EQ(BeSQLite::BE_SQLITE_ROW, db.QueryProperty(propStr, s_schemaVerPropSpec));
        BeSQLite::ProfileVersion ver(propStr.c_str());
        ASSERT_TRUE(ver.GetMajor() == 1 && ver.GetMinor() == 0) << "registry db should start out with schema version 1.0";
        }

    if (true)
        {
        FakeRegistry testRegistry(GetTestDataDir(), outputDbName);
        ASSERT_EQ(BeSQLite::BE_SQLITE_OK, testRegistry.Open());

        auto insert = testRegistry.GetDb().GetCachedStatement("INSERT INTO fwk_BridgeAssignments (SourceFile,Bridge,Affinity) VALUES(?,?,?)");
        ASSERT_TRUE(insert.IsValid()) << "should have been upgraded to schema version 1.1, where fwk_BridgeAssignments has a column called Affinity";
        }

    if (true)
        {
        BeSQLite::Db db;
        ASSERT_EQ(BeSQLite::BE_SQLITE_OK, db.OpenBeSQLiteDb(outputDbName, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::Readonly)));
        Utf8String propStr;
        ASSERT_EQ(BeSQLite::BE_SQLITE_ROW, db.QueryProperty(propStr, s_schemaVerPropSpec));
        BeSQLite::ProfileVersion ver(propStr.c_str());
        ASSERT_TRUE(ver.GetMajor() == 1 && ver.GetMinor() == 1) << "registry db should have been upgraded to 1.1";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, TestCallAssignToDisclose)
    {
    auto testDir = CreateTestDir();

    BeFileName stagingDir(GetTestDataDir());
    stagingDir.AppendToPath(L"attachmentExample");

    auto masterFileName = GetTestDataFileName(L"attachmentExample/master.dgn");
    auto ref1FileName = GetTestDataFileName(L"attachmentExample/ref1.dgn");
    auto ref2FileName = GetTestDataFileName(L"attachmentExample/ref2.dgn");
    auto commonrefFileName = GetTestDataFileName(L"attachmentExample/commonref.dgn");
    auto dwgFileName = GetTestDataFileName(L"attachmentExample/d.dwg");
    auto obdFileName = GetTestDataFileName(L"attachmentExample/obd.dgn");

    WPrintfString inputArg(L"--fwk-input=%ls", masterFileName.c_str());
    WPrintfString registryDirArg(L"--registry-dir=%ls", testDir.c_str());
    WPrintfString stagingDirArg(L"--fwk-staging-dir=%ls", stagingDir.c_str());

    BeFileName regDbName = iModelBridgeRegistry::MakeDbName(testDir, "TestCallAssignToDisclose");
    if (true)
        {
        FakeRegistry testRegistry(stagingDir, regDbName);
        FakeRegistry::FakeBridgeDef bridgeDef;
        bridgeDef.m_regSubKey = MSTN_BRIDGE_REG_SUB_KEY;
        bridgeDef.m_libraryFilename = GetDgnv8BridgeDllName();
        bridgeDef.m_bridgeAssetsDir = bridgeDef.m_libraryFilename.GetDirectoryName();
        bridgeDef.m_bridgeAssetsDir.AppendToPath(L"Assets");
        testRegistry.WriteInstalledBridgesTable({ bridgeDef });
        }

    if (true)
        {
        WCharCP argv[] = {
            L"argv0",
            L"--server-repository=TestCallAssignToDisclose",
            stagingDirArg.c_str(),
            registryDirArg.c_str(),
            inputArg.c_str(),
            L"--affinity-db-name=foo.db",   // This argument tells iModelBridgeAssign to run the new disclose files and affinities on the specified input file
            L"--no-bridge-search"
        };
        int argc = (int)_countof(argv);

        ASSERT_EQ(0, iModelBridgeRegistry::AssignMain(argc, argv));
        }

    bmap<WString, int64_t> assignments;
    bset<WString> docs;
    if (true)
        {
        BeSQLite::Db reg;
        ASSERT_EQ(BeSQLite::BE_SQLITE_OK, reg.OpenBeSQLiteDb(regDbName, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::Readonly)));

        auto dstmt = reg.GetCachedStatement("select LocalFilePath from DocumentProperties");
        while (dstmt->Step() == BeSQLite::BE_SQLITE_ROW)
            {
            auto file = WString(dstmt->GetValueText(0), true);
            docs.insert(file);
            }

        ASSERT_EQ(6, docs.size());
        ASSERT_TRUE(docs.find(masterFileName) != docs.end());
        ASSERT_TRUE(docs.find(ref1FileName) != docs.end());
        ASSERT_TRUE(docs.find(ref2FileName) != docs.end());
        ASSERT_TRUE(docs.find(commonrefFileName) != docs.end());
        ASSERT_TRUE(docs.find(dwgFileName) != docs.end());
        ASSERT_TRUE(docs.find(obdFileName) != docs.end());

        auto astmt = reg.GetCachedStatement("select SourceFile, Bridge from fwk_BridgeAssignments");
        while (astmt->Step() == BeSQLite::BE_SQLITE_ROW)
            {
            auto file = WString(astmt->GetValueText(0), true);
            auto bridgeRowId =  astmt->GetValueInt64(1);
            assignments[file] = bridgeRowId;
            }

        ASSERT_EQ(5, assignments.size());
        ASSERT_EQ(1, assignments[masterFileName]);
        ASSERT_EQ(1, assignments[ref1FileName]);
        ASSERT_EQ(1, assignments[ref2FileName]);
        ASSERT_EQ(1, assignments[commonrefFileName]);
        ASSERT_EQ(1, assignments[dwgFileName]);

        bmap<WString, Utf8String> recommendations;
        auto rstmt = reg.GetCachedStatement("select LocalFilePath, Bridge from Recommendations");
        while (rstmt->Step() == BeSQLite::BE_SQLITE_ROW)
            {
            auto file = WString(rstmt->GetValueText(0), true);
            auto bridgeKey = rstmt->GetValueText(1);
            recommendations[file] = bridgeKey;
            }

        ASSERT_EQ(1, recommendations.size());
        ASSERT_TRUE(recommendations[obdFileName].EqualsI("OpenBuildingsDesigner"));
        }

    // Run again, pretending that MstnBridge is a legacy bridge that does not support disclosing files and assignments.
    // We just want to verify that assign does not change any of the assignments computed above.
    if (true)
        {
        WCharCP argv[] = {
            L"argv0",
            L"--server-repository=TestCallAssignToDisclose",
            stagingDirArg.c_str(),
            registryDirArg.c_str(),
            L"--fwk-search-in-staging-dir",
            L"--update-assignments",
            L"--no-bridge-search"
        };
        int argc = (int)_countof(argv);

        ASSERT_EQ(0, iModelBridgeRegistry::AssignMain(argc, argv));
        }

    if (true)
        {
        BeSQLite::Db reg;
        ASSERT_EQ(BeSQLite::BE_SQLITE_OK, reg.OpenBeSQLiteDb(regDbName, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::Readonly)));

        bset<WString> updatedDocs;
        auto dstmt = reg.GetCachedStatement("select LocalFilePath from DocumentProperties");
        while (dstmt->Step() == BeSQLite::BE_SQLITE_ROW)
            {
            auto file = WString(dstmt->GetValueText(0), true);
            updatedDocs.insert(file);
            }

        EXPECT_EQ(docs, updatedDocs);

        bmap<WString, int64_t> updatedAssignments;
        auto astmt = reg.GetCachedStatement("select SourceFile, Bridge from fwk_BridgeAssignments");
        while (astmt->Step() == BeSQLite::BE_SQLITE_ROW)
            {
            auto file = WString(astmt->GetValueText(0), true);
            auto bridgeRowId =  astmt->GetValueInt64(1);
            updatedAssignments[file] = bridgeRowId;
            }
        
        EXPECT_EQ(assignments, updatedAssignments);
        }

    
    if (true)
        {
        // Now pretend that a bridge called "OpenBuildingsDesigner" is installed. 
        // Pretend that it did not have a discloseFilesAndAffinities function.
        // Instead, we now have to call its getAffinity function and *update* the
        // registry with this result. It should take the obd file. 
        // We'll also have it take over commonref.dgn, just to demonstrate that the update
        // can change an existing assignment.
        FakeRegistry testRegistry(stagingDir, regDbName);

        std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
                                                            const size_t bufferSize,
                                                            iModelBridgeAffinityLevel& affinityLevel,
                                                            BentleyApi::WCharCP affinityLibraryPath,
                                                            BentleyApi::WCharCP sourceFileName)
            {
            wcscpy(buffer, L"OpenBuildingsDesigner");
            if (obdFileName.Equals(sourceFileName) || commonrefFileName.Equals(sourceFileName))
                affinityLevel = iModelBridgeAffinityLevel::High;
            else if (WString(sourceFileName).EndsWith(L".dgn"))
                affinityLevel = iModelBridgeAffinityLevel::Low;
            else
                affinityLevel = iModelBridgeAffinityLevel::None;
            };
        testRegistry.AddBridge(L"OpenBuildingsDesigner", lambda);

        WCharCP argv[] = {
            L"argv0",
            L"--server-repository=TestCallAssignToDisclose",
            registryDirArg.c_str(),
            L"--fwk-search-in-staging-dir",
            L"--update-assignments",
            L"--no-bridge-search"
        };
        int argc = (int)_countof(argv);

        ASSERT_EQ(0, testRegistry.RunAssign(argc, argv));
        }

    if (true)
        {
        BeSQLite::Db reg;
        ASSERT_EQ(BeSQLite::BE_SQLITE_OK, reg.OpenBeSQLiteDb(regDbName, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::Readonly)));

        bset<WString> updatedDocs;
        auto dstmt = reg.GetCachedStatement("select LocalFilePath from DocumentProperties");
        while (dstmt->Step() == BeSQLite::BE_SQLITE_ROW)
            {
            auto file = WString(dstmt->GetValueText(0), true);
            updatedDocs.insert(file);
            }

        EXPECT_EQ(docs, updatedDocs);

        bmap<WString, int64_t> updatedAssignments;
        auto astmt = reg.GetCachedStatement("select SourceFile, Bridge from fwk_BridgeAssignments");
        while (astmt->Step() == BeSQLite::BE_SQLITE_ROW)
            {
            auto file = WString(astmt->GetValueText(0), true);
            auto bridgeRowId =  astmt->GetValueInt64(1);
            updatedAssignments[file] = bridgeRowId;
            }
        
        EXPECT_EQ(6, updatedAssignments.size());

        auto obdAssignment = updatedAssignments.find(obdFileName);
        ASSERT_TRUE(obdAssignment != updatedAssignments.end());
        ASSERT_EQ(2, obdAssignment->second) << "The (fictious) OpenBuildingsDesigner should have been found. It would be the second bridge in the installedBridges table.";

        auto commonRefAssignment = updatedAssignments.find(commonrefFileName);
        ASSERT_TRUE(commonRefAssignment != updatedAssignments.end());
        ASSERT_EQ(2, commonRefAssignment->second);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef TEST_DATA_IS_TOO_BIG
TEST_F(MstnBridgeTests, TestCallAssignToDisclose_BigFile)
    {
    auto testDir = CreateTestDir();

    BeFileName affinityDbName(testDir);
    affinityDbName.AppendToPath(L"affinity.db");
    
    BeFileName stagingDir(GetTestDataDir());

    auto masterFileName = GetTestDataFileName(L"house_model_keith.dgn");

    WPrintfString inputArg(L"--fwk-input=%ls", masterFileName.c_str());
    WPrintfString registryDirArg(L"--registry-dir=%ls", testDir.c_str());
    WPrintfString stagingDirArg(L"--fwk-staging-dir=%ls", stagingDir.c_str());
    WPrintfString affinityDbNameArg(L"--affinity-db-name=%ls", affinityDbName.c_str());

    BeFileName regDbName = iModelBridgeRegistry::MakeDbName(testDir, "TestCallAssignToDisclose");
    if (true)
        {
        FakeRegistry testRegistry(stagingDir, regDbName);
        FakeRegistry::FakeBridgeDef bridgeDef;
        bridgeDef.m_regSubKey = MSTN_BRIDGE_REG_SUB_KEY;
        bridgeDef.m_libraryFilename = GetDgnv8BridgeDllName();
        bridgeDef.m_bridgeAssetsDir = bridgeDef.m_libraryFilename.GetDirectoryName();
        bridgeDef.m_bridgeAssetsDir.AppendToPath(L"Assets");
        testRegistry.WriteInstalledBridgesTable({ bridgeDef });
        }

    if (true)
        {
        WCharCP argv[] = {
            L"argv0",
            L"--server-repository=TestCallAssignToDisclose",
            stagingDirArg.c_str(),
            registryDirArg.c_str(),
            inputArg.c_str(),
            affinityDbNameArg.c_str(),
            L"--no-bridge-search"
        };
        int argc = (int)_countof(argv);

        ASSERT_EQ(0, iModelBridgeRegistry::AssignMain(argc, argv));
        }

    bmap<WString, int64_t> assignments;
    bset<WString> docs;
    if (true)
        {
        BeSQLite::Db reg;
        ASSERT_EQ(BeSQLite::BE_SQLITE_OK, reg.OpenBeSQLiteDb(regDbName, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::Readonly)));

        auto dstmt = reg.GetCachedStatement("select LocalFilePath from DocumentProperties");
        while (dstmt->Step() == BeSQLite::BE_SQLITE_ROW)
            {
            auto file = WString(dstmt->GetValueText(0), true);
            docs.insert(file);
            }

        ASSERT_EQ(1, docs.size()) << "All models are in this one file";;
        ASSERT_TRUE(docs.find(masterFileName) != docs.end());

        auto countAssignmentsStmt = reg.GetCachedStatement("select count(*) from fwk_BridgeAssignments");
        countAssignmentsStmt->Step();
        EXPECT_EQ(1, countAssignmentsStmt->GetValueInt(0));

        auto detectAssignmentStmt = reg.GetCachedStatement("select SourceFile from fwk_BridgeAssignments where Bridge=1");
        detectAssignmentStmt->Step();
        auto file = detectAssignmentStmt->GetValueText(0);
        EXPECT_TRUE(Utf8String(masterFileName).EqualsI(file));
        }

    if (true)
        {
        auto adb = iModelBridgeAffinityDb::Open(affinityDbName);

        auto mstmt = adb->GetDb().GetCachedStatement("select count(*) from Model");
        mstmt->Step();
        auto modelCount = mstmt->GetValueInt(0);
        EXPECT_EQ(5, modelCount);

        auto attstmt = adb->GetDb().GetCachedStatement("select count(*) from Attachment");
        attstmt->Step();
        auto attachmentCount = attstmt->GetValueInt(0);
        EXPECT_EQ(4, attachmentCount);
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, DiscloseFilesAndAffinities)
    {
    auto testDir = CreateTestDir();
    BeFileName outDir = GetOutputDir();
    BeFileName affinityDbName(outDir);
    affinityDbName.AppendToPath(L"affinity.db");

    auto masterFileName = GetTestDataFileName(L"attachmentExample/master.dgn");
    auto ref1FileName = GetTestDataFileName(L"attachmentExample/ref1.dgn");
    auto ref2FileName = GetTestDataFileName(L"attachmentExample/ref2.dgn");
    auto commonrefFileName = GetTestDataFileName(L"attachmentExample/commonref.dgn");
    auto dwgFileName = GetTestDataFileName(L"attachmentExample/d.dwg");
    auto obdFileName = GetTestDataFileName(L"attachmentExample/obd.dgn");

    auto affinityLibraryPath = GetDgnv8BridgeDllName();
    BeFileName assetsPath(affinityLibraryPath.GetDirectoryName());
    assetsPath.AppendToPath(L"assets");

    auto discloseFilesAndAffinities = (T_iModelBridge_discloseFilesAndAffinities*)iModelBridgeRegistryUtils::GetBridgeFunction(affinityLibraryPath, "iModelBridge_discloseFilesAndAffinities");
    ASSERT_TRUE(discloseFilesAndAffinities != nullptr);
    ASSERT_EQ(0, discloseFilesAndAffinities(affinityDbName, affinityLibraryPath.c_str(), assetsPath.c_str(), masterFileName.c_str(), MSTN_BRIDGE_REG_SUB_KEY));

    auto db = iModelBridgeAffinityDb::Open(affinityDbName);
    ASSERT_TRUE(db.IsValid());
    auto mid = db->FindFile(masterFileName);
    auto r1id = db->FindFile(ref1FileName);
    auto r2id = db->FindFile(ref2FileName);
    auto crid = db->FindFile(commonrefFileName);
    auto did = db->FindFile(dwgFileName);
    auto oid = db->FindFile(obdFileName);
    ASSERT_NE(0, mid);
    ASSERT_EQ(1, mid);
    ASSERT_NE(0, r1id);
    ASSERT_NE(0, r2id);
    ASSERT_NE(0, crid);
    ASSERT_NE(0, did);
    ASSERT_NE(0, oid);

    auto mstnBridgeId = db->FindBridge(MSTN_BRIDGE_REG_SUB_KEY_A);
    ASSERT_NE(0, mstnBridgeId);
    auto obdBridgeId = db->FindBridge("OpenBuildingsDesigner");
    ASSERT_NE(0, obdBridgeId);

    iModelBridgeAffinityLevel level;
    ASSERT_EQ(BSISUCCESS, db->FindAffinity(&level, nullptr, mid, mstnBridgeId));
    ASSERT_EQ(iModelBridgeAffinityLevel::Low, level);
    ASSERT_EQ(BSISUCCESS, db->FindAffinity(&level, nullptr, r1id, mstnBridgeId));
    ASSERT_EQ(iModelBridgeAffinityLevel::Low, level);
    ASSERT_EQ(BSISUCCESS, db->FindAffinity(&level, nullptr, r2id, mstnBridgeId));
    ASSERT_EQ(iModelBridgeAffinityLevel::Low, level);
    ASSERT_EQ(BSISUCCESS, db->FindAffinity(&level, nullptr, crid, mstnBridgeId));
    ASSERT_EQ(iModelBridgeAffinityLevel::Low, level);
    ASSERT_EQ(BSISUCCESS, db->FindAffinity(&level, nullptr, did, mstnBridgeId));
    ASSERT_EQ(iModelBridgeAffinityLevel::Low, level);
    ASSERT_EQ(BSISUCCESS, db->FindAffinity(&level, nullptr, oid, obdBridgeId));
    ASSERT_EQ(iModelBridgeAffinityLevel::High, level);

    Utf8String json;
    auto mmid = db->FindModel(nullptr, &json, mid, "0");
    ASSERT_NE(0, mmid);
    ASSERT_EQ(1, mmid);
    ASSERT_TRUE(jsonHasMember(json, "transform"));
    auto r1mid = db->FindModel(nullptr, &json, r1id, "0");
    ASSERT_NE(0, r1mid);
    ASSERT_TRUE(jsonHasMember(json, "transform"));
    auto r2mid = db->FindModel(nullptr, &json, r2id, "0");
    ASSERT_NE(0, r2mid);
    ASSERT_TRUE(jsonHasMember(json, "transform"));
    auto crmid = db->FindModel(nullptr, &json, crid, "0");
    ASSERT_NE(0, crmid);
    ASSERT_TRUE(jsonHasMember(json, "transform"));
    auto dmid = db->FindModel(nullptr, &json, did, "0");
    ASSERT_NE(0, dmid);
    ASSERT_TRUE(jsonHasMember(json, "transform"));
    auto omid = db->FindModel(nullptr, &json, oid, "0");
    ASSERT_NE(0, omid);
    ASSERT_TRUE(jsonHasMember(json, "transform"));

    ASSERT_EQ(BSISUCCESS, db->FindAttachment(nullptr, mmid, r1mid));
    ASSERT_EQ(BSISUCCESS, db->FindAttachment(nullptr, mmid, r2mid));
    ASSERT_EQ(BSISUCCESS, db->FindAttachment(nullptr, r1id, crmid));
    ASSERT_EQ(BSISUCCESS, db->FindAttachment(nullptr, r2id, crmid));
    ASSERT_EQ(BSISUCCESS, db->FindAttachment(nullptr, mmid, dmid));
    ASSERT_EQ(BSISUCCESS, db->FindAttachment(nullptr, r1mid, dmid));
    ASSERT_EQ(BSISUCCESS, db->FindAttachment(nullptr, r2mid, omid));

    bset<int64_t> attachedToMaster;
    db->QueryAttachmentsToFile(mid, [&attachedToMaster](int64_t refFileRowId) 
        {
        attachedToMaster.insert(refFileRowId);
        });
    ASSERT_EQ(3, attachedToMaster.size());
    ASSERT_TRUE(attachedToMaster.find(r1id) != attachedToMaster.end());
    ASSERT_TRUE(attachedToMaster.find(r2id) != attachedToMaster.end());
    ASSERT_TRUE(attachedToMaster.find(did) != attachedToMaster.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, ChangeEmbeddedFileNameShouldUpdateModelName)
    {
    auto testDir = CreateTestDir();

    putenv("MS_PROTECTION_PASSWORD_CACHE_LIFETIME=0"); // must disable pw caching, as we copy new files on top of old ones too quickly
    static const uint32_t s_v8PasswordCacheLifetime = 2*1000; // the timeout is 1 second. Wait for 2 to be safe.

    BentleyApi::BeFileName inputStagingDir = GetOutputDir();

    putenv("iModelBridge_MatchOnEmbeddedFileBasename=_v[0-9]$");    // The recipe specifies that any -vn suffix should be stripped when forming internal file identifiers.
    // The recipe also says that file extensions should be stripped, PW URNs should be ignored, and filename case should be ignored. That is the default, when a regex is supplied.

    // In this test:
    // In v1, master embeds ref_v1.dgn.i.dgn
    // In v2, master embeds ref_v2.i.dgn. According to the recipe, that should be recognized as referring to the same file as the original ref_v1.dgn.i.dgn
    // So, the recipe that we supply above should enable the bridge to recognize that both versions are based on the same reference file, just different versions of it.

    auto masterFileName = GetOutputFileName(L"master.i.dgn");      // The name of the staged input file
    BeFileName refFileName(L"ref");                            // Because of the recipe, all versions of the reference file will be mapped to this name.

    BeFileName refV1FileName_firstOccurence(masterFileName);
    Utf8CP ref_v1_name = "ref_v1.dgn.i.dgn";
    Utf8CP ref_v1_ref_name = "logical, ref_v1.dgn.i.dgn, Default"; // the name of the reference attachment in v1
    refV1FileName_firstOccurence.append(L"<1>").append(WString(ref_v1_name,true));

    BeFileName refv2FileName_firstOccurence(masterFileName);
    Utf8CP ref_v2_name = "ref_v2.dgn.i.dgn";
    Utf8CP ref_v2_ref_name = "logical, ref_v2.dgn.i.dgn, Default"; // the name of the reference attachment in v2
    refv2FileName_firstOccurence.append(L"<1>").append(WString(ref_v2_name,true));

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(DEFAULT_IMODEL_NAME L".fwk-registry.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, iModelBridge_getAffinity);

    BentleyApi::BeSQLite::BeGuid guidMaster, guidRef;
    guidMaster.Create();
    guidRef.Create();

    iModelBridgeDocumentProperties docPropsMaster(guidMaster.ToString().c_str(), "wurnMaster", "durnMaster", "otherMaster", "");
    iModelBridgeDocumentProperties docPropsRef(guidRef.ToString().c_str(), "wurnRef", "durnRef", "otherRef", "");
    testRegistry.SetDocumentProperties(docPropsMaster, masterFileName);
    testRegistry.SetDocumentProperties(docPropsRef, refFileName);           // <--------- the reference file will have a doc id. (The recipe will tell the bridge to ignore it.)
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
        const size_t bufferSize,
        iModelBridgeAffinityLevel& affinityLevel,
        BentleyApi::WCharCP affinityLibraryPath,
            BentleyApi::WCharCP sourceFileName)
        {
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        };

    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, masterFileName, L"");
    ASSERT_TRUE(bridgeName.Equals(MSTN_BRIDGE_REG_SUB_KEY));
    testRegistry.Save();
    TerminateHost();

    DgnCode refCode;
    int modelCount = 0;
    DgnElementId refModelId;
    if (true)
        {
        // In v1, master embeds ref_v1.dgn.i.dgn
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        auto master_v1 = GetTestDataFileName(L"versionedEmbeddedRef\\master_v1.i.dgn");

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_v1.c_str(), masterFileName.c_str()));

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", masterFileName.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo bcInfo(m_briefcaseName);
        modelCount = bcInfo.GetPhysicalModelCount();
        ASSERT_EQ(2, modelCount);
        RepositoryLinkId rid;
        bcInfo.MustFindFileByName(rid, masterFileName, 1);
        bcInfo.MustFindFileByName(rid, refV1FileName_firstOccurence, 1);

        auto rlink = bcInfo.m_db->Elements().Get<RepositoryLink>(rid);
        ASSERT_TRUE(rlink.IsValid());
        EXPECT_STREQ(rlink->GetUserLabel(), ref_v1_name);
        refCode = rlink->GetCode();
        EXPECT_STREQ(rlink->GetUrl(), ref_v1_name);

        // Verify that the model imported from ref_v1 is named according to the rules and reflects the name of the v1 file.
        BeSQLite::EC::ECSqlStatement stmt;
        ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*bcInfo.m_db, "SELECT ecinstanceid FROM BisCore.PhysicalPartition WHERE CodeValue=?"));
        stmt.BindText(1, ref_v1_ref_name, BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
        auto rc = stmt.Step();
        ASSERT_EQ(BentleyApi::BeSQLite::DbResult::BE_SQLITE_ROW, rc);
        refModelId = stmt.GetValueId<DgnElementId>(0);
        }

    if (true)
        {
        // In v2, master embeds ref_v2.i.dgn. According to the recipe, that should be recognized as referring to the same file as the original ref_v1.dgn.i.dgn
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        auto master_v2 = GetTestDataFileName(L"versionedEmbeddedRef\\master_v2.i.dgn");

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_v2.c_str(), masterFileName.c_str()));

        // NEEDS WORK temporary work-around for V8 password cache entry lifetime
        BentleyApi::BeThreadUtilities::BeSleep(s_v8PasswordCacheLifetime);

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", masterFileName.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        // No change in model count is expected, as we still have the same two files in v2 as we did in v0
        DbFileInfo bcInfo(m_briefcaseName);
        auto newModelCount = bcInfo.GetPhysicalModelCount();
        ASSERT_EQ(modelCount, newModelCount) << "No change in model count is expected, as we still have the same two files in v2 as we did in v0";
        RepositoryLinkId rid;
        bcInfo.MustFindFileByName(rid, masterFileName, 1);
        bcInfo.MustFindFileByName(rid, refV1FileName_firstOccurence, 1); // The RepositoryLink still refers to the first occurrence.

        auto rlink = bcInfo.m_db->Elements().Get<RepositoryLink>(rid);
        ASSERT_TRUE(rlink.IsValid());
        EXPECT_STREQ(rlink->GetUserLabel(), ref_v2_name);
        EXPECT_STREQ(rlink->GetUrl(), ref_v2_name);
        auto code2 = rlink->GetCode();
        auto code2JsonStr = code2.ToJson2().toStyledString();
        auto originalJsonStr = refCode.ToJson2().toStyledString();
        EXPECT_STREQ(code2JsonStr.c_str(), originalJsonStr.c_str()) << "the reference file's Code must not change!";

        // Verify that the model imported from ref_v2 is named according to the rules and reflects the name of the v2 file.
        BeSQLite::EC::ECSqlStatement stmt;
        ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*bcInfo.m_db, "SELECT ecinstanceid FROM BisCore.PhysicalPartition WHERE CodeValue=?"));
        stmt.BindText(1, ref_v2_ref_name, BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
        auto rc = stmt.Step();
        EXPECT_EQ(BentleyApi::BeSQLite::DbResult::BE_SQLITE_ROW, rc);
        EXPECT_EQ(refModelId.GetValue(), stmt.GetValueId<DgnElementId>(0).GetValue());
        }

    putenv("MS_PROTECTION_PASSWORD_CACHE_LIFETIME=1"); // restore default
    putenv("iModelBridge_MatchOnEmbeddedFileBasename=");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/20
+---------------+---------------+---------------+---------------+---------------+------*/
static void setBridgeSchemaVersion(DgnDbR db, SubjectCR jobSubject, Utf8StringCR newVersion)
    {
    auto jobSubjEd = db.Elements().GetForEdit<Subject>(jobSubject.GetElementId());
    
    auto props = JobSubjectUtils::GetProperty(*jobSubjEd, "Properties");
    props["BridgeSchemaVersion"] = newVersion;
    JobSubjectUtils::SetProperty(*jobSubjEd, "Properties", props);
    ASSERT_TRUE(jobSubjEd->Update().IsValid());
    }

struct ScopedNewBcMgr
    {
    Utf8String m_was;

    ScopedNewBcMgr() 
        {
        m_was.AssignOrClear(getenv("imodel-bridge-fwk-briefcase-manager"));
        putenv("imodel-bridge-fwk-briefcase-manager=1"); // Must use new bc mgr, so that new locking rules are used, so that we can directly edit the model after the bridge runs.
        }

    ~ScopedNewBcMgr()
        {
        if (m_was.empty() || m_was == "0")
            putenv("imodel-bridge-fwk-briefcase-manager=");
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, FixIncorrectlyNamedEmbeddedFileModels)
    {
    auto testDir = CreateTestDir();

    ScopedNewBcMgr setNewBcMgrInScope; // Must use new bc mgr, so that new locking rules are used, so that we can directly edit the model after the bridge runs.

    putenv("MS_PROTECTION_PASSWORD_CACHE_LIFETIME=0"); // must disable pw caching, as we copy new files on top of old ones too quickly
    static const uint32_t s_v8PasswordCacheLifetime = 2*1000; // the timeout is 1 second. Wait for 2 to be safe.

    BentleyApi::BeFileName inputStagingDir = GetOutputDir();

    putenv("iModelBridge_MatchOnEmbeddedFileBasename=_v[0-9]$");    // The recipe specifies that any -vn suffix should be stripped when forming internal file identifiers.
    // The recipe also says that file extensions should be stripped, PW URNs should be ignored, and filename case should be ignored. That is the default, when a regex is supplied.

    auto masterFileName = GetOutputFileName(L"master.i.dgn");      // The name of the staged input file
    BeFileName refFileName(L"ref");                            // Because of the recipe, all versions of the reference file will be mapped to this name.

    BeFileName refV1FileName_firstOccurence(masterFileName);
    Utf8CP ref_v1_name = "ref_v1.dgn.i.dgn";
    Utf8CP ref_v1_ref_name = "logical, ref_v1.dgn.i.dgn, Default"; // the name of the reference attachment in v1
    refV1FileName_firstOccurence.append(L"<1>").append(WString(ref_v1_name,true));

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(DEFAULT_IMODEL_NAME L".fwk-registry.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, iModelBridge_getAffinity);

    BentleyApi::BeSQLite::BeGuid guidMaster, guidRef;
    guidMaster.Create();
    guidRef.Create();

    iModelBridgeDocumentProperties docPropsMaster(guidMaster.ToString().c_str(), "wurnMaster", "durnMaster", "otherMaster", "");
    iModelBridgeDocumentProperties docPropsRef(guidRef.ToString().c_str(), "wurnRef", "durnRef", "otherRef", "");
    testRegistry.SetDocumentProperties(docPropsMaster, masterFileName);
    testRegistry.SetDocumentProperties(docPropsRef, refFileName);           // <--------- the reference file will have a doc id. (The recipe will tell the bridge to ignore it.)
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
        const size_t bufferSize,
        iModelBridgeAffinityLevel& affinityLevel,
        BentleyApi::WCharCP affinityLibraryPath,
            BentleyApi::WCharCP sourceFileName)
        {
        wcscpy(buffer, MSTN_BRIDGE_REG_SUB_KEY);
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        };

    testRegistry.AddBridge(MSTN_BRIDGE_REG_SUB_KEY, lambda);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, masterFileName, L"");
    ASSERT_TRUE(bridgeName.Equals(MSTN_BRIDGE_REG_SUB_KEY));
    testRegistry.Save();
    TerminateHost();

    DgnCode refCode;
    int modelCount = 0;
    DgnElementId refPartitionElementId;
    if (true)
        {
        // In v1, master embeds ref_v1.dgn.i.dgn
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        auto master_v1 = GetTestDataFileName(L"versionedEmbeddedRef\\master_v1.i.dgn");

        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(master_v1.c_str(), masterFileName.c_str()));

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", masterFileName.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        DbFileInfo bcInfo(m_briefcaseName);
        modelCount = bcInfo.GetPhysicalModelCount();
        ASSERT_EQ(2, modelCount);
        RepositoryLinkId rid;
        bcInfo.MustFindFileByName(rid, masterFileName, 1);
        bcInfo.MustFindFileByName(rid, refV1FileName_firstOccurence, 1);

        auto rlink = bcInfo.m_db->Elements().Get<RepositoryLink>(rid);
        ASSERT_TRUE(rlink.IsValid());
        EXPECT_STREQ(rlink->GetUserLabel(), ref_v1_name);
        refCode = rlink->GetCode();
        EXPECT_STREQ(rlink->GetUrl(), ref_v1_name);

        // Verify that the model imported from ref_v1 is named according to the rules and reflects the name of the v1 file.
        BeSQLite::EC::ECSqlStatement stmt;
        ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*bcInfo.m_db, "SELECT ecinstanceid FROM BisCore.PhysicalPartition WHERE CodeValue=?"));
        stmt.BindText(1, ref_v1_ref_name, BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
        auto rc = stmt.Step();
        ASSERT_EQ(BentleyApi::BeSQLite::DbResult::BE_SQLITE_ROW, rc);
        refPartitionElementId = stmt.GetValueId<DgnElementId>(0);
        }

    if (true)
        {
        // Tweak the .bim file so that it looks like a) an old bridge wrote to it, and b) the name of the reference model is wrong.

        DbFileInfo bcInfo(m_briefcaseName, true);

        bcInfo.SetRepositoryAdminFromBriefcaseClient(*m_client);
        bcInfo.m_db->BriefcaseManager().StartBulkOperation();

        auto partition = bcInfo.m_db->Elements().GetForEdit<PhysicalPartition>(refPartitionElementId);
        ASSERT_TRUE(partition.IsValid());

        auto oldCode = partition->GetCode();
        DgnCode newCode(oldCode.GetCodeSpecId(), oldCode.GetScopeElementId(*bcInfo.m_db), "Foo");
        ASSERT_EQ(DgnDbStatus::Success, partition->SetCode(newCode));
        ASSERT_TRUE(partition->Update().IsValid());

        auto jobSubject = bcInfo.GetFirstJobSubject();
        ASSERT_TRUE(jobSubject.IsValid());
        setBridgeSchemaVersion(*bcInfo.m_db, *jobSubject, "1.0");

        ASSERT_EQ(BeSQLite::BE_SQLITE_OK, bcInfo.m_db->SaveChanges());
        }

    if (true)
        {
        // Now run the bridge again (same file) and verify that it a) fixed the schema version and b)  fixed the name of the reference model.
        bvector<WString> args;
        SetUpBridgeProcessingArgs(args, testDir.c_str(), MSTN_BRIDGE_REG_SUB_KEY, DEFAULT_IMODEL_NAME);

        // NEEDS WORK temporary work-around for V8 password cache entry lifetime
        BentleyApi::BeThreadUtilities::BeSleep(s_v8PasswordCacheLifetime);

        args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", masterFileName.c_str()));
        RunTheBridge(args);
        CleanupElementECExtensions();

        args.push_back(WPrintfString(L"--fwk-all-docs-processed"));
        RunTheBridge(args);
        CleanupElementECExtensions();

        // No change in model count is expected.
        DbFileInfo bcInfo(m_briefcaseName);
        auto newModelCount = bcInfo.GetPhysicalModelCount();
        ASSERT_EQ(modelCount, newModelCount);
        RepositoryLinkId rid;
        bcInfo.MustFindFileByName(rid, masterFileName, 1);
        bcInfo.MustFindFileByName(rid, refV1FileName_firstOccurence, 1); // The RepositoryLink still refers to the first occurrence.

        // No change in the properties of the reference RepositoryLink is expected.
        auto rlink = bcInfo.m_db->Elements().Get<RepositoryLink>(rid);
        ASSERT_TRUE(rlink.IsValid());
        EXPECT_STREQ(rlink->GetUserLabel(), ref_v1_name);
        EXPECT_STREQ(rlink->GetUrl(), ref_v1_name);
        auto code2 = rlink->GetCode();
        auto code2JsonStr = code2.ToJson2().toStyledString();
        auto originalJsonStr = refCode.ToJson2().toStyledString();
        EXPECT_STREQ(code2JsonStr.c_str(), originalJsonStr.c_str()) << "the reference file's Code must not change!";

        // Verify that the model's CodeValue is set back to match its parent
        BeSQLite::EC::ECSqlStatement stmt;
        ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*bcInfo.m_db, "SELECT ecinstanceid FROM BisCore.PhysicalPartition WHERE CodeValue=?"));
        stmt.BindText(1, ref_v1_ref_name, BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
        auto rc = stmt.Step();
        EXPECT_EQ(BentleyApi::BeSQLite::DbResult::BE_SQLITE_ROW, rc);
        EXPECT_EQ(refPartitionElementId.GetValue(), stmt.GetValueId<DgnElementId>(0).GetValue());
        }

    putenv("MS_PROTECTION_PASSWORD_CACHE_LIFETIME=1"); // restore default
    putenv("iModelBridge_MatchOnEmbeddedFileBasename=");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P (SynchInfoTests, TestSynchInfoAspect)
{
    WString type = GetParam ();

    auto testDir = CreateTestDir(type.c_str());

    bvector<WString> args;
    SetUpBridgeProcessingArgs (args, testDir.c_str (), MSTN_BRIDGE_REG_SUB_KEY);

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile (inputFile, L"Test3d.dgn", NULL);

    args.push_back (WPrintfString (L"--fwk-input=\"%ls\"", inputFile.c_str ()));
    args.push_back (L"--fwk-skip-assignment-check");

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    int64_t srcId = 0;
    if (0 == type.CompareToI (L"Model"))
        srcId = AddModel (inputFile, "TestModel");
    else if (0 == type.CompareToI (L"NamedView"))
        srcId = AddNamedView (inputFile, "TestNamedView");
    else if (0 == type.CompareToI (L"Element"))
        srcId = AddLine (inputFile);
    else if (0 == type.CompareToI (L"Level"))
        srcId = AddLevel (inputFile, "TestLevel");

    if (true)
    {
        // and run an update
        RunTheBridge (args);
    }

    if (m_briefcaseName.empty()) // RunTheBridge failed
        return;

    if (0 == type.CompareToI (L"Model"))
        ValidateModelSynchInfo (m_briefcaseName, srcId);
    else if (0 == type.CompareToI (L"NamedView"))
        ValidateNamedViewSynchInfo (m_briefcaseName, srcId);
    else if (0 == type.CompareToI (L"Element"))
        ValidateElementSynchInfo (m_briefcaseName, srcId);
    else if (0 == type.CompareToI (L"Level"))
        ValidateLevelSynchInfo (m_briefcaseName, srcId);
}

const WString params[] = {L"Model", L"Element", L"Level", /*L"NamedView"*/ };
INSTANTIATE_TEST_CASE_P (AllSynchInfoTests, SynchInfoTests, ::testing::ValuesIn (params));

//=======================================================================================
// @bsistruct
//=======================================================================================
struct Bridge : public MstnBridgeTestsFixture
    {
    static WCharCP s_loggingConfigFile;

    static BentleyStatus GetFileNameFromEnv (BeFileName& fn, CharCP envname)
        {
        WString filepath (getenv(envname), BentleyCharEncoding::Utf8);
        if (filepath.empty())
            return ERROR;
        fn.SetName (filepath);
        return SUCCESS;
        }

    static BentleyStatus GetLogConfigurationFilename (BeFileName& configFile)
        {
        if (SUCCESS == GetFileNameFromEnv (configFile, "BRIDGE_FILE_LOGGING_CONFIG"))
            {
            if (BeFileName::DoesPathExist (configFile))
                {
                printf ("Configuring logging with %s (Set by BRIDGE_FILE_LOGGING_CONFIG environment variable.)\n", Utf8String(configFile.GetName()).c_str());
                return SUCCESS;
                }
            }

        configFile = BeFileName(BentleyApi::Desktop::FileSystem::GetExecutableDir());
        configFile.AppendToPath (s_loggingConfigFile);
        configFile.BeGetFullPathName ();
        if (BeFileName::DoesPathExist (configFile))
            {
            printf ("Configuring logging using %s. Override by setting BRIDGE_FILE_LOGGING_CONFIG in environment.\n", Utf8String(configFile.GetName()).c_str());
            return SUCCESS;
            }

        return ERROR;
        }

    static void SetUpTestCase()
        {
        BeFileName configFile;
        if (SUCCESS == GetLogConfigurationFilename(configFile))
            {
            NativeLogging::LoggingConfig::SetOption (CONFIG_OPTION_CONFIG_FILE, configFile);
            NativeLogging::LoggingConfig::ActivateProvider (NativeLogging::LOG4CXX_LOGGING_PROVIDER);
            }
        else
            {
            printf ("Logging.config.xml not found. Set by BRIDGE_FILE_LOGGING_CONFIG environment variable. Configuring default logging using console provider.\n");
            NativeLogging::LoggingConfig::ActivateProvider (NativeLogging::CONSOLE_LOGGING_PROVIDER);
            }
        InitializeHost();
        }
    };

WCharCP Bridge::s_loggingConfigFile = L"MstnBridgeTests.logging.config.xml";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Bridge, File)
    {
    if (s_argc != 3)
        {
        FAIL() << Utf8PrintfString("syntax: %s <bridgedllpath> <filename>", s_argv[0]);
        }

    BeFileName bridgeDllName(s_argv[1], true);
    BeFileName inputFile(s_argv[2], true);
    WCharCP bridgeRegSubkey = L"bridgeRegSubKey";
    WCharCP iModelName = L"Test";

    if (!bridgeDllName.DoesPathExist())
        {
        FAIL() << WPrintfString(L"%ls - Bridge DLL file not found", bridgeDllName.c_str()).c_str();
        }

    if (!inputFile.DoesPathExist())
        {
        FAIL() << WPrintfString(L"%ls - Input file not found", inputFile.c_str()).c_str();
        }

    auto testDir = CreateTestDir();

    auto useBank = GetIModelBankServerJs();
    auto rspFileName = useBank? L"imodel-bank.rsp": L"imodel-hub.rsp";

    FwkArgvMaker argvMaker;
    argvMaker.SetUpBridgeProcessingArgs(testDir.c_str(), bridgeRegSubkey, bridgeDllName, iModelName, useBank, rspFileName);

    argvMaker.SetSkipAssignmentCheck();

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    argvMaker.SetInputFileArg(GetTestDataFileName(L"Test3d.dgn"));
    RunTheBridge(argvMaker.GetArgVector());

    argvMaker.PopArg();
    argvMaker.SetInputFileArg(inputFile);
    RunTheBridge(argvMaker.GetArgVector());
    }

/*
    If you run multiple bridges simultaneously, and if they both import the same schemas, then there is a race condition.
    Scenario: User A and User B register the same domains, and the domains import the same schemas in both briefcases.
                A pushes.
                B pulls.
                B gets an error when applying A's schema change revision, because B already made the same changes.

    You can produce this race condition in MstnBridgeTests.MultiBridgeSequencing if you remove the initial "admin" bridge run.

    *** First, we import Functional ***

    BeSQLiteECM02.dll!BentleyM0200::BeSQLite::EC::SchemaWriter::ImportSchemas(BentleyM0200::Bstdcxx::bvector<BentleyM0200::ECN::ECSchema const *,BentleyM0200::BentleyAllocator<BentleyM0200::ECN::ECSchema const *> > & schemasToMap, BentleyM0200::BeSQLite::EC::SchemaImportContext & schemaImportCtx, const BentleyM0200::Bstdcxx::bvector<BentleyM0200::ECN::ECSchema const *,BentleyM0200::BentleyAllocator<BentleyM0200::ECN::ECSchema const *> > & schemasRaw) Line 33 (d:\imodel02\source\imodel02\iModelCore\ECDb\ECDb\SchemaWriter.cpp:33)
    BeSQLiteECM02.dll!BentleyM0200::BeSQLite::EC::MainSchemaManager::ImportSchemas(BentleyM0200::BeSQLite::EC::SchemaImportContext & ctx, const BentleyM0200::Bstdcxx::bvector<BentleyM0200::ECN::ECSchema const *,BentleyM0200::BentleyAllocator<BentleyM0200::ECN::ECSchema const *> > & schemas, const BentleyM0200::BeSQLite::EC::SchemaImportToken * schemaImportToken) Line 699 (d:\imodel02\source\imodel02\iModelCore\ECDb\ECDb\SchemaManagerDispatcher.cpp:699)
    BeSQLiteECM02.dll!BentleyM0200::BeSQLite::EC::MainSchemaManager::ImportSchemas(const BentleyM0200::Bstdcxx::bvector<BentleyM0200::ECN::ECSchema const *,BentleyM0200::BentleyAllocator<BentleyM0200::ECN::ECSchema const *> > & schemas, BentleyM0200::BeSQLite::EC::SchemaManager::SchemaImportOptions options, const BentleyM0200::BeSQLite::EC::SchemaImportToken * token) Line 656 (d:\imodel02\source\imodel02\iModelCore\ECDb\ECDb\SchemaManagerDispatcher.cpp:656)
    BeSQLiteECM02.dll!BentleyM0200::BeSQLite::EC::SchemaManager::ImportSchemas(const BentleyM0200::Bstdcxx::bvector<BentleyM0200::ECN::ECSchema const *,BentleyM0200::BentleyAllocator<BentleyM0200::ECN::ECSchema const *> > & schemas, BentleyM0200::BeSQLite::EC::SchemaManager::SchemaImportOptions options, const BentleyM0200::BeSQLite::EC::SchemaImportToken * token) Line 36 (d:\imodel02\source\imodel02\iModelCore\ECDb\ECDb\SchemaManager.cpp:36)
    DgnPlatformM02.dll!BentleyM0200::Dgn::DgnDomains::DoImportSchemas(const BentleyM0200::Bstdcxx::bvector<BentleyM0200::ECN::ECSchema const *,BentleyM0200::BentleyAllocator<BentleyM0200::ECN::ECSchema const *> > & importSchemas, BentleyM0200::BeSQLite::EC::SchemaManager::SchemaImportOptions importOptions) Line 754 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\DgnDomain.cpp:754)
    DgnPlatformM02.dll!BentleyM0200::Dgn::DgnDomains::UpgradeSchemas() Line 440 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\DgnDomain.cpp:440)
            InitializeSchemas discovered that Functional needs to import its schema.
    DgnPlatformM02.dll!BentleyM0200::Dgn::DgnDb::InitializeSchemas(const BentleyM0200::BeSQLite::Db::OpenParams & params) Line 194 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\DgnDb.cpp:194)
    DgnPlatformM02.dll!BentleyM0200::Dgn::DgnDb::_OnDbOpened(const BentleyM0200::BeSQLite::Db::OpenParams & params) Line 160 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\DgnDb.cpp:160)
    BeSQLiteM02.dll!BentleyM0200::BeSQLite::Db::OpenBeSQLiteDb(const char * dbName, const BentleyM0200::BeSQLite::Db::OpenParams & params) Line 2636 (d:\imodel02\source\imodel02\iModelCore\BeSQLite\BeSQLite.cpp:2636)
    DgnPlatformM02.dll!BentleyM0200::BeSQLite::Db::OpenBeSQLiteDb(const BentleyM0200::BeFileName & dbName, const BentleyM0200::BeSQLite::Db::OpenParams & openParams) Line 2560 (d:\imodel02\out\Winx64\BuildContexts\DgnPlatform\PublicAPI\BeSQLite\BeSQLite.h:2560)
    DgnPlatformM02.dll!BentleyM0200::Dgn::DgnDb::DoOpenDgnDb(const BentleyM0200::BeFileName & projectNameIn, const BentleyM0200::Dgn::DgnDb::OpenParams & params) Line 660 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\DgnDb.cpp:660)
    DgnPlatformM02.dll!BentleyM0200::Dgn::DgnDb::OpenDgnDb(BentleyM0200::BeSQLite::DbResult * outResult, const BentleyM0200::BeFileName & fileName, const BentleyM0200::Dgn::DgnDb::OpenParams & openParams) Line 684 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\DgnDb.cpp:684)
    iModelBridgeM02.dll!BentleyM0200::Dgn::iModelBridge::OpenBimAndMergeSchemaChanges(BentleyM0200::BeSQLite::DbResult & dbres, bool & madeSchemaChanges, const BentleyM0200::BeFileName & dbName, BentleyM0200::Dgn::DgnDb::OpenParams & oparams) Line 189 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\iModelBridge.cpp:189)
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::iModelBridgeFwk::TryOpenBimWithOptions(BentleyM0200::Dgn::DgnDb::OpenParams & oparams) Line 1642 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\iModelBridgeFwk.cpp:1642)
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::iModelBridgeFwk::TryOpenBimWithBisSchemaUpgrade() Line 1698 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\iModelBridgeFwk.cpp:1698)
            Just initialized the bridge. It registered its required Domains, including Functional.
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::iModelBridgeFwk::UpdateExistingBim() Line 2163 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\iModelBridgeFwk.cpp:2163)
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::iModelBridgeFwk::UpdateExistingBimWithExceptionHandling() Line 1808 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\iModelBridgeFwk.cpp:1808)
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::iModelBridgeFwk::RunExclusive(int argc, const wchar_t * * argv) Line 1515 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\iModelBridgeFwk.cpp:1515)
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::iModelBridgeFwk::Run(int argc, const wchar_t * * argv) Line 2392 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\iModelBridgeFwk.cpp:2392)
    MstnBridgeTests.exe!MstnBridgeTests_MultiBridgeSequencing_Test::TestBody::__l7::<lambda>() Line 210 (d:\imodel02\source\imodel02\Bridges\Mstn\Tests\MstnBridgeTests.cpp:210)

    *** Later, we pull a schema change revision that also imports Functional ***

    BeSQLiteM02.dll!BentleyM0200::BeSQLite::Db::ExecuteSql(const char * sql, int(*)(void *, int, char * *, char * *) callback, void * arg, char * * errmsg) Line 918 (d:\imodel02\source\imodel02\iModelCore\BeSQLite\BeSQLite.cpp:918)
    DgnPlatformM02.dll!BentleyM0200::Dgn::TxnManager::ApplyDbSchemaChangeSet(const BentleyM0200::BeSQLite::DbSchemaChangeSet & dbSchemaChanges) Line 1171 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\TxnManager.cpp:1171)
    DgnPlatformM02.dll!BentleyM0200::Dgn::TxnManager::MergeDbSchemaChangesInRevision(const BentleyM0200::Dgn::DgnRevision & revision, BentleyM0200::Dgn::RevisionChangesFileReader & changeStream) Line 833 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\TxnManager.cpp:833)
    DgnPlatformM02.dll!BentleyM0200::Dgn::TxnManager::MergeRevision(const BentleyM0200::Dgn::DgnRevision & revision) Line 1032 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\TxnManager.cpp:1032)
    DgnPlatformM02.dll!BentleyM0200::Dgn::RevisionManager::DoMergeRevision(const BentleyM0200::Dgn::DgnRevision & revision) Line 987 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\RevisionManager.cpp:987)
    DgnPlatformM02.dll!BentleyM0200::Dgn::RevisionManager::DoProcessRevisions(const BentleyM0200::Bstdcxx::bvector<BentleyM0200::Dgn::DgnRevision const *,BentleyM0200::BentleyAllocator<BentleyM0200::Dgn::DgnRevision const *> > & revisions, BentleyM0200::Dgn::RevisionProcessOption processOptions) Line 1654 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\RevisionManager.cpp:1654)
    DgnPlatformM02.dll!BentleyM0200::Dgn::DgnDb::ProcessRevisions(const BentleyM0200::BeSQLite::Db::OpenParams & params) Line 239 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\DgnDb.cpp:239)
    DgnPlatformM02.dll!BentleyM0200::Dgn::DgnDb::InitializeSchemas(const BentleyM0200::BeSQLite::Db::OpenParams & params) Line 190 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\DgnDb.cpp:190)
    DgnPlatformM02.dll!BentleyM0200::Dgn::DgnDb::_OnDbOpened(const BentleyM0200::BeSQLite::Db::OpenParams & params) Line 160 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\DgnDb.cpp:160)
    BeSQLiteM02.dll!BentleyM0200::BeSQLite::Db::OpenBeSQLiteDb(const char * dbName, const BentleyM0200::BeSQLite::Db::OpenParams & params) Line 2636 (d:\imodel02\source\imodel02\iModelCore\BeSQLite\BeSQLite.cpp:2636)
    DgnPlatformM02.dll!BentleyM0200::BeSQLite::Db::OpenBeSQLiteDb(const BentleyM0200::BeFileName & dbName, const BentleyM0200::BeSQLite::Db::OpenParams & openParams) Line 2560 (d:\imodel02\out\Winx64\BuildContexts\DgnPlatform\PublicAPI\BeSQLite\BeSQLite.h:2560)
    DgnPlatformM02.dll!BentleyM0200::Dgn::DgnDb::DoOpenDgnDb(const BentleyM0200::BeFileName & projectNameIn, const BentleyM0200::Dgn::DgnDb::OpenParams & params) Line 660 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\DgnDb.cpp:660)
    DgnPlatformM02.dll!BentleyM0200::Dgn::DgnDb::OpenDgnDb(BentleyM0200::BeSQLite::DbResult * outResult, const BentleyM0200::BeFileName & fileName, const BentleyM0200::Dgn::DgnDb::OpenParams & openParams) Line 684 (d:\imodel02\source\imodel02\iModelCore\DgnPlatform\DgnCore\DgnDb.cpp:684)
    iModelBridgeFwkLibM02.dll!tryPullAndMergeSchemaRevisions(BentleyM0200::RefCountedPtr<BentleyM0200::Dgn::DgnDb> & db, BentleyM0200::RefCountedPtr<BentleyM0200::iModel::Hub::Briefcase> briefcase) Line 369 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\IModelClientForBridges.cpp:369)
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::IModelClientBase::PullAndMergeSchemaRevisions(BentleyM0200::RefCountedPtr<BentleyM0200::Dgn::DgnDb> & db) Line 386 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\IModelClientForBridges.cpp:386)
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::iModelBridgeFwk::Briefcase_PullMergePush(const char * descIn, bool doPullAndMerge, bool doPush) Line 557 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\Briefcase.cpp:557)
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::iModelBridgeFwk::PullMergeAndPushChange(const BentleyM0200::Utf8String & description, bool releaseLocks, bool reopenDb) Line 1562 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\iModelBridgeFwk.cpp:1562)
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::iModelBridgeFwk::TryOpenBimWithOptions(BentleyM0200::Dgn::DgnDb::OpenParams & oparams) Line 1670 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\iModelBridgeFwk.cpp:1670)
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::iModelBridgeFwk::TryOpenBimWithBisSchemaUpgrade() Line 1698 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\iModelBridgeFwk.cpp:1698)
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::iModelBridgeFwk::UpdateExistingBim() Line 2163 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\iModelBridgeFwk.cpp:2163)
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::iModelBridgeFwk::UpdateExistingBimWithExceptionHandling() Line 1808 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\iModelBridgeFwk.cpp:1808)
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::iModelBridgeFwk::RunExclusive(int argc, const wchar_t * * argv) Line 1515 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\iModelBridgeFwk.cpp:1515)
    iModelBridgeFwkLibM02.dll!BentleyM0200::Dgn::iModelBridgeFwk::Run(int argc, const wchar_t * * argv) Line 2392 (d:\imodel02\source\imodel02\iModelBridgeCore\DgnDbSync\iModelBridge\Fwk\iModelBridgeFwk.cpp:2392)
    MstnBridgeTests.exe!MstnBridgeTests_MultiBridgeSequencing_Test::TestBody::__l7::<lambda>() Line 210 (d:\imodel02\source\imodel02\Bridges\Mstn\Tests\MstnBridgeTests.cpp:210)
*/

POP_DISABLE_DEPRECATION_WARNINGS
