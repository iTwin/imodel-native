/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DwgBridgeTestsFixture.h"
#include <iModelBridge/TestIModelHubClientForBridges.h>
#include <iModelBridge/iModelBridgeFwk.h>
#include <iModelBridge/FakeRegistry.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#include <Bentley/Desktop/FileSystem.h>

USING_NAMESPACE_BENTLEY_DGN


//=======================================================================================
// @bsistruct                              
//=======================================================================================
struct DwgBridgeTests : public DwgBridgeTestsFixture
    {
    void SetupTwoRefs(bvector<WString>& args, BentleyApi::BeFileName& masterFile, BentleyApi::BeFileName& refFile1, 
                      BentleyApi::BeFileName& refFile2, BentleyApi::BeFileName const& testDir, FakeRegistry& testRegistry);
    void VerifyLineHasCode(int prevCount, BeFileNameCR bcName, int64_t srcId, Utf8CP codeValuePrefix, bool codeShouldBeRecorded);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DwgBridgeTests, TestDenySchemaLock)
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
    MakeCopyOfFile(inputFile, L"basictype.dwg", NULL);

    SetupClient(ComputeAccessToken("user1"));
    CreateRepository();
    {
    auto runningServer = StartServer();

    FwkArgvMaker argvMaker;
    argvMaker.SetUpBridgeProcessingArgs(testDir.c_str(), DwgBridgeTestsFixture::GetDwgBridgeRegSubKey(), GetDwgBridgeDllName(), DEFAULT_IMODEL_NAME, true, L"imodel-bank.rsp");
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
TEST_F(DwgBridgeTests, MultiBridgeSequencing)
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
                              "rule": { "verb": "wait file", "object": "%s" }                        
                          }                        
                      ]                            
                  },
                  {                            
                      "user": "B",             
                      "rules": [                   
                          {                        
                              "request": "Lock/Create",
                              "rule": { "verb": "wait file", "object": "%s" }                        
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

    // -------------------------------------------------------
    // Bridge A
    // -------------------------------------------------------
    std::thread bridge_a([&] 
        {
        BeFileName aDir(testDir);
        aDir.AppendToPath(L"A");
        BeFileName::CreateNewDirectory(aDir.c_str());

        FwkArgvMaker argvMaker;
        argvMaker.SetUpBridgeProcessingArgs(aDir.c_str(), L"bridge_a", GetDwgBridgeDllName(), DEFAULT_IMODEL_NAME, true, L"imodel-bank.rsp");
        argvMaker.ReplaceArgValue(L"--imodel-bank-access-token", ComputeAccessTokenW("A").c_str());

        BentleyApi::BeFileName inputFile;
        MakeCopyOfFile(inputFile, L"basictype.dwg", L"_A");

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

    // (Note: bridge a should be blocked and not making progress, because we have not yet created the "let_a_run" file.)

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
        argvMaker.SetUpBridgeProcessingArgs(bDir.c_str(), L"bridge_b", GetDwgBridgeDllName(), DEFAULT_IMODEL_NAME, true, L"imodel-bank.rsp");
        argvMaker.ReplaceArgValue(L"--imodel-bank-access-token", ComputeAccessTokenW("B").c_str());

        BentleyApi::BeFileName inputFile;
        MakeCopyOfFile(inputFile, L"basictype.dwg", L"_B");

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
    
    createFile(a_can_run);      // now let a run

    bridge_a.join();

    runningServer.Stop(10*1000); // NEEDS WORK: must wait long enough for imodel-bank server to shut down. The time required can vary unpredictably but seems to increase when a second process has been running.
    }
    }

    // ASSERT_EQ(0, ProcessRunner::FindProcessId("node.exe"));
    // ASSERT_EQ(0, ProcessRunner::FindProcessId("iModelBridgeFwk.exe"));
    ASSERT_TRUE(bridge_a_sawRetryMsg);

    // Verify that B ran first and A ran second
    if (true)
        {
        // If bridge A ran second, it must have had to pull bridge B's changesets before it could push.
        // Therefore, A's briefcase should contain the RepositoryLink for B's input file as well as its own. 
        DbFileInfo info(bridge_a_briefcaseName);
        auto aSourceFileId = info.GetRepositoryLinkByFileNameLike("%basictype_a.dwg");
        auto bSourceFileId = info.GetRepositoryLinkByFileNameLike("%basictype_b.dwg");
        ASSERT_TRUE(aSourceFileId.IsValid());
        ASSERT_TRUE(bSourceFileId.IsValid());
        }

    if (true)
        {
        // If bridge B ran first and finished before A could run, B's briefcase should not have a record of A's input file.
        DbFileInfo info(bridge_b_briefcaseName);
        auto aSourceFileId = info.GetRepositoryLinkByFileNameLike("%basictype_a.dwg");
        auto bSourceFileId = info.GetRepositoryLinkByFileNameLike("%basictype_b.dwg");
        ASSERT_FALSE(aSourceFileId.IsValid());
        ASSERT_TRUE(bSourceFileId.IsValid());
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DwgBridgeTests, ConvertLinesUsingBridgeFwk)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), DwgBridgeTestsFixture::GetDwgBridgeRegSubKey());
    
    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"basictype.dwg", NULL);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));
    args.push_back(L"--fwk-skip-assignment-check");

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();
    
    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);
        }

    int prevCount = DbFileInfo(m_briefcaseName).GetElementCount();
    if (true)
        {
        //Make sure a second run of the bridge does not add any elements.
        RunTheBridge(args);
        EXPECT_EQ(prevCount, DbFileInfo(m_briefcaseName).GetElementCount());
        }

    AddLine(inputFile);
    if (true)
        {
        // and run an update
        RunTheBridge(args);
        EXPECT_EQ(1 + prevCount, DbFileInfo(m_briefcaseName).GetElementCount());
        }

    if (true)
        {
        //Make sure a second run of the bridge does not add any elements.
        RunTheBridge(args);
        EXPECT_EQ(1 + prevCount, DbFileInfo(m_briefcaseName).GetElementCount());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DwgBridgeTests, TestSourceElementIdAspect)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), DwgBridgeTestsFixture::GetDwgBridgeRegSubKey());
    
    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"basictype.dwg", NULL);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));
    args.push_back(L"--fwk-skip-assignment-check");

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();
    
    uint64_t srcId = AddLine(inputFile);
    if (true)
        {
        // and run an update
        RunTheBridge(args);
        }
    DbFileInfo info(m_briefcaseName);
    DgnElementId id;
    ASSERT_EQ(SUCCESS, info.GetiModelElementByDgnElementId(id, srcId));

    ASSERT_TRUE(id.IsValid());

    }

extern "C"
    {
    EXPORT_ATTRIBUTE T_iModelBridge_getAffinity iModelBridge_getAffinity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DwgBridgeTests, ConvertAttachmentSingleBridge)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), DwgBridgeTestsFixture::GetDwgBridgeRegSubKey(), DEFAULT_IMODEL_NAME);
    
    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"basictype.dwg", NULL);
    AddLine(inputFile);

    BentleyApi::BeFileName refFile;
    MakeCopyOfFile(refFile, L"basictype.dwg", L"-Ref-1");
    AddLine(refFile);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(DEFAULT_IMODEL_NAME L".fwk-registry.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(DwgBridgeTestsFixture::GetDwgBridgeRegSubKey(), iModelBridge_getAffinity);

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

    int modelCount = 0;
    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);
        
        modelCount = DbFileInfo(m_briefcaseName).GetModelCount();
        ASSERT_EQ(10, modelCount);
        }

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
TEST_F(DwgBridgeTests, ConvertAttachmentSingleBridgeAlternateRegistry)
    {
    auto testDir = CreateTestDir();
    
    BentleyApi::BeFileName stagingDir(testDir);
    stagingDir.AppendToPath(L"staging");
    EXPECT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::CreateNewDirectory(stagingDir.c_str()));
    
    BentleyApi::BeFileName registryDir(testDir);
    registryDir.AppendToPath(L"assignments");
    EXPECT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::CreateNewDirectory(registryDir.c_str()));

    auto regsubKey = DwgBridgeTestsFixture::GetDwgBridgeRegSubKey();
    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, stagingDir.c_str(), regsubKey, DEFAULT_IMODEL_NAME);
    args.push_back(_wcsdup(BentleyApi::WPrintfString(L"--registry-dir=%s", registryDir.c_str()).c_str()));
    
    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"basictype.dwg", NULL);
    AddLine(inputFile);

    BentleyApi::BeFileName refFile;
    MakeCopyOfFile(refFile, L"basictype.dwg", L"-Ref-1");
    AddLine(refFile);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    BentleyApi::BeFileName assignDbName(registryDir);
    assignDbName.AppendToPath(DEFAULT_IMODEL_NAME L".fwk-registry.db");
    FakeRegistry testRegistry(registryDir, assignDbName);
    testRegistry.WriteAssignments();

    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
        const size_t bufferSize,
        iModelBridgeAffinityLevel& affinityLevel,
        BentleyApi::WCharCP affinityLibraryPath,
            BentleyApi::WCharCP sourceFileName)
        {
        wcsncpy(buffer, regsubKey, ::wcslen(regsubKey));
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        };

    testRegistry.AddBridge(regsubKey, lambda);

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

    int modelCount = 0;
    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);
        
        modelCount = DbFileInfo(m_briefcaseName).GetModelCount();
        ASSERT_EQ(10, modelCount);
        }

   
    AddAttachment(inputFile, refFile, 1, true);
    if (true)
        {
        //We added a new attachment.
        RunTheBridge(args);
        ASSERT_EQ(modelCount + 1, DbFileInfo(m_briefcaseName).GetModelCount());
        }

    }

#ifdef WIP_ASSIGN_CODE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DwgBridgeTests, CodeReservation)
    {
    auto testDir = CreateTestDir();

    BeFileName noCodesAllowed(testDir);
    noCodesAllowed.AppendToPath(L"noCodesAllowed.txt");
    
    bool usingIModelBank = (GetIModelBankServerJs() != nullptr);

    if (usingIModelBank)
        {
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
        }

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"basictype.dwg", NULL);

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
    argvMaker.SetUpBridgeProcessingArgs(testDir.c_str(), DwgBridgeTestsFixture::GetDwgBridgeRegSubKey(), GetDwgBridgeDllName(), DEFAULT_IMODEL_NAME, true, L"imodel-bank.rsp");
    argvMaker.ReplaceArgValue(L"--imodel-bank-access-token", ComputeAccessTokenW("user1").c_str());

    argvMaker.SetInputFileArg(inputFile);
    argvMaker.SetSkipAssignmentCheck();

    BeFileName bcName;
    if (true)
        {
        // Must allow Code reservations during the initial conversion. That is where the bridge creates the Subject 
        // elements in the repository model, and each Subject has a Code that must be reserved (because they must
        // be unique across bridges). So, do not create the "noCodesAllowed" file yet.
        
        iModelBridgeFwk fwk;
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argvMaker.GetArgC(), argvMaker.GetArgV()));
        ASSERT_EQ(0, fwk.Run(argvMaker.GetArgC(), argvMaker.GetArgV()));
        bcName = fwk.GetBriefcaseName();
        }

    // Get information about the jobsubject and the spatial model that we will be using.
    int prevCount = 0;
    DgnElementId rootSubjectId;
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
        bcInfo.MustFindModelByDwgModelId(spatialModelId, repositoryLinkId, GetDefaultDwgModelId(inputFile), 1);

        // Verify that the root subject is NOT locked by this bridge
        ASSERT_EQ(BentleyApi::Dgn::LockLevel::None, bcInfo.QueryLockLevel(*bcInfo.m_db->Elements().GetRootSubject(), GetClient()));

        // Verify that the jobSubject and this spatial model is exclusively locked by this bridge
        auto spatialModel = bcInfo.m_db->Models().GetModel(spatialModelId);
        ASSERT_EQ(BentleyApi::Dgn::LockLevel::Exclusive, bcInfo.QueryLockLevel(*jSubj, GetClient()));
        ASSERT_EQ(BentleyApi::Dgn::LockLevel::Exclusive, bcInfo.QueryLockLevel(*spatialModel, GetClient()));

        // Verify that the server has registered Subject elements that are in the RepositoryModel.
        auto repositoryLink = bcInfo.m_db->Elements().GetElement(repositoryLinkId);
        ASSERT_TRUE(repositoryLink.IsValid());
        DgnCodeInfo codeInfo;
        EXPECT_EQ(BentleyApi::BSISUCCESS, bcInfo.GetCodeInfo(codeInfo, repositoryLink->GetCode(), GetClient()));
        EXPECT_TRUE(codeInfo.IsUsed());

        rootSubjectId = bcInfo.m_db->Elements().GetRootSubjectId();
        }

    // Add a line using MODEL-SCOPED CODES, where the model is one that is exclusively owned by the bridge.
    Utf8CP prefix = nullptr;
    int64_t srcId = AddLine(inputFile);
        {
        if (usingIModelBank)
            createFile(noCodesAllowed);

        ScopedCodeAssignerXDomain assignCodes(spatialModelId, prefix="Model");

        iModelBridgeFwk fwk;
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argvMaker.GetArgC(), argvMaker.GetArgV()));
        ASSERT_EQ(0, fwk.Run(argvMaker.GetArgC(), argvMaker.GetArgV()));
        }

    VerifyLineHasCode(prevCount, bcName, srcId, prefix, true);  // TODO: This should be false -- when we finally control locks and codes, there will be no code reservation in this case.
    ++prevCount;

    // Add a line using RELATED-ELEMENT-SCOPED CODES, where the related element is exclusively owned by the bridge.
    srcId = AddLine(inputFile);
        {
        if (usingIModelBank)
            createFile(noCodesAllowed);

        ScopedCodeAssignerXDomain assignCodes(jobSubjectId, prefix="Related", CodeScope::Related);

        iModelBridgeFwk fwk;
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argvMaker.GetArgC(), argvMaker.GetArgV()));
        ASSERT_EQ(0, fwk.Run(argvMaker.GetArgC(), argvMaker.GetArgV()));
        }

    VerifyLineHasCode(prevCount, bcName, srcId, prefix, true);  // TODO: This should be false -- when we finally control locks and codes, there will be no code reservation in this case.
    ++prevCount;

    // Add a line using RELATED-ELEMENT-SCOPED CODES, where the related is NOT exclusively owned by the bridge.
    srcId = AddLine(inputFile);
        {
        if (usingIModelBank)
            BeFileName::BeDeleteFile(noCodesAllowed.c_str());

        ScopedCodeAssignerXDomain assignCodes(rootSubjectId, prefix="RelatedNotLocked", CodeScope::Related);

        iModelBridgeFwk fwk;
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argvMaker.GetArgC(), argvMaker.GetArgV()));
        ASSERT_EQ(0, fwk.Run(argvMaker.GetArgC(), argvMaker.GetArgV()));
        }
    
    VerifyLineHasCode(prevCount, bcName, srcId, prefix, true);
    ++prevCount;

    // Add a line using REPOSITORY-SCOPED CODES
    srcId = AddLine(inputFile);
        {
        if (usingIModelBank)
            BeFileName::BeDeleteFile(noCodesAllowed.c_str());

        ScopedCodeAssignerXDomain assignCodes(rootSubjectId, prefix="Repository", CodeScope::Repository);

        iModelBridgeFwk fwk;
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argvMaker.GetArgC(), argvMaker.GetArgV()));
        ASSERT_EQ(0, fwk.Run(argvMaker.GetArgC(), argvMaker.GetArgV()));
        }

    VerifyLineHasCode(prevCount, bcName, srcId, prefix, true);
    ++prevCount;
    } // ~runningServer => stop Server

    }
#endif  // WIP_ASSIGN_CODE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridgeTests::SetupTwoRefs(bvector<WString>& args, BentleyApi::BeFileName& masterFile, BentleyApi::BeFileName& refFile1, BentleyApi::BeFileName& refFile2, BentleyApi::BeFileName const& testDir, FakeRegistry& testRegistry)
    {
    MakeCopyOfFile(masterFile, L"basictype.dwg", NULL);
    AddLine(masterFile);

    MakeCopyOfFile(refFile1, L"basictype.dwg", L"-Ref-1");
    AddLine(refFile1);

    MakeCopyOfFile(refFile2, L"basictype.dwg", L"-Ref-2");
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
TEST_F(DwgBridgeTests, DISABLED_PushAfterEachModel)
    {
    auto testDir = CreateTestDir();

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    // Set up to process a master file and two reference files, all mapped to DwgBridge
    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), DwgBridgeTestsFixture::GetDwgBridgeRegSubKey(), DEFAULT_IMODEL_NAME);

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(DEFAULT_IMODEL_NAME L".fwk-registry.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer, const size_t bufferSize, iModelBridgeAffinityLevel& affinityLevel,
                                                           BentleyApi::WCharCP affinityLibraryPath, BentleyApi::WCharCP sourceFileName)
        {
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        wcscpy(buffer, DwgBridgeTestsFixture::GetDwgBridgeRegSubKey());
        };
    testRegistry.AddBridge(DwgBridgeTestsFixture::GetDwgBridgeRegSubKey(), lambda);

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
static bool containsSubstr(BentleyApi::bset<BentleyApi::Utf8String> const& strings, BentleyApi::Utf8StringCR substr)
    {
    for (auto const& str: strings)
        {
        if (str.ContainsI(substr))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DwgBridgeTests, PushAfterEachFile)
    {
    auto testDir = CreateTestDir();
     
    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    // Set up to process a master file and two reference files, all mapped to DwgBridge
    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), DwgBridgeTestsFixture::GetDwgBridgeRegSubKey(), DEFAULT_IMODEL_NAME);

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(DEFAULT_IMODEL_NAME L".fwk-registry.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer, const size_t bufferSize, iModelBridgeAffinityLevel& affinityLevel,
                                                           BentleyApi::WCharCP affinityLibraryPath, BentleyApi::WCharCP sourceFileName)
        {
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        wcscpy(buffer, DwgBridgeTestsFixture::GetDwgBridgeRegSubKey());
        };
    testRegistry.AddBridge(DwgBridgeTestsFixture::GetDwgBridgeRegSubKey(), lambda);

    BentleyApi::BeFileName masterFile, refFile1, refFile2;
    SetupTwoRefs(args, masterFile, refFile1, refFile2, testDir, testRegistry);

    testRegistry.Save();
    
    int modelCount = 0;
    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);
        
        modelCount = DbFileInfo(m_briefcaseName).GetModelCount();
        ASSERT_EQ(10, modelCount);
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
        // There will be more than just the two file-specific changesets. Assert at least 2 more.
        EXPECT_GE(GetChangesetCount(), csCountBefore + 2) << "each model should have been pushed in its own changeset";

        auto revstats = ComputeRevisionStats(*fileInfo.m_db, csCountBefore);
        EXPECT_EQ(revstats.nSchemaRevs, 0) << "no schema changes expected, since the initial conversion did that";
        EXPECT_GE(revstats.nDataRevs, 2) << "each file should have been pushed in its own changeset";
        EXPECT_TRUE(containsSubstr(revstats.descriptions, "-Ref-1")) << "first ref should have been pushed in its own revision";
        EXPECT_TRUE(containsSubstr(revstats.descriptions, "-Ref-2")) << "second ref should have been pushed in its own revision";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DwgBridgeTests, DISABLED_OidcTest)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), DwgBridgeTestsFixture::GetDwgBridgeRegSubKey());
    
    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"basictype.dwg", NULL);

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
TEST_F(DwgBridgeTests, DISABLED_TestCodeRemovalPerformance)
    {
    auto testDir = CreateTestDir();

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str(), DwgBridgeTestsFixture::GetDwgBridgeRegSubKey());

    args.push_back(L"--set-DebugCodes");
    
    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"basictype.dwg", NULL);

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
        LOGTODB("DwgBridgeTests", "TestCodeRemovalPerformance,", watch.GetElapsedSeconds() * 1000.0);

        }
    DbFileInfo info(m_briefcaseName);
    DgnElementId id;
    ASSERT_EQ(SUCCESS, info.GetiModelElementByDgnElementId(id, srcId));

    ASSERT_TRUE(id.IsValid());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P (ExternalSourceAspectTests, TestExternalSourceAspectAspect)
{
    WString type = GetParam ();

    auto testDir = CreateTestDir(type.c_str());

    bvector<WString> args;
    SetUpBridgeProcessingArgs (args, testDir.c_str (), DwgBridgeTestsFixture::GetDwgBridgeRegSubKey());

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile (inputFile, L"basictype.dwg", NULL);

    args.push_back (WPrintfString (L"--fwk-input=\"%ls\"", inputFile.c_str ()));
    args.push_back (L"--fwk-skip-assignment-check");

    SetupClient();
    CreateRepository();
    auto runningServer = StartServer();

    int64_t srcId = 0;
    if (0 == type.CompareToI (L"Model"))
        srcId = AddModel (inputFile, "TestModel");
    else if (0 == type.CompareToI (L"Element"))
        srcId = AddLine (inputFile);
    else if (0 == type.CompareToI (L"Layer"))
        srcId = AddLayer (inputFile, "TestLayer");

    if (true)
    {
        // and run an update
        RunTheBridge (args);
    }

    if (0 == type.CompareToI (L"Model"))
        ValidateModelAspect (m_briefcaseName, srcId);
    else if (0 == type.CompareToI (L"Element"))
        ValidateElementAspect (m_briefcaseName, srcId);
    else if (0 == type.CompareToI (L"Layer"))
        ValidateLayerAspect (m_briefcaseName, srcId);
}

const WString params[] = {L"Model", L"Element", L"Layer"};
INSTANTIATE_TEST_CASE_P (AllExternalSourceAspectTests, ExternalSourceAspectTests, ::testing::ValuesIn (params));
