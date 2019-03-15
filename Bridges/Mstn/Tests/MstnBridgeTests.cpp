/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/MstnBridgeTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "MstnBridgeTestsFixture.h"
#include <iModelBridge/TestIModelHubClientForBridges.h>
#include <iModelBridge/iModelBridgeFwk.h>
#include <iModelBridge/FakeRegistry.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#include <Bentley/Desktop/FileSystem.h>

USING_NAMESPACE_BENTLEY_DGN

#define MSTN_BRIDGE_REG_SUB_KEY L"iModelBridgeForMstn"

//=======================================================================================
// @bsistruct                              
//=======================================================================================
struct MstnBridgeTests : public MstnBridgeTestsFixture
    {
    void SetupTwoRefs(bvector<WString>& args, BentleyApi::BeFileName& masterFile, BentleyApi::BeFileName& refFile1, 
                      BentleyApi::BeFileName& refFile2, BentleyApi::BeFileName const& testDir, FakeRegistry& testRegistry);
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
        argvMaker.SetUpBridgeProcessingArgs(aDir.c_str(), L"bridge_a", GetDgnv8BridgeDllName(), DEFAULT_IMODEL_NAME, true, L"imodel-bank.rsp");
        argvMaker.ReplaceArgValue(L"--imodel-bank-access-token", ComputeAccessTokenW("A").c_str());

        BentleyApi::BeFileName inputFile;
        MakeCopyOfFile(inputFile, L"Test3d.dgn", L"_A");

        argvMaker.SetInputFileArg(inputFile);
        argvMaker.SetSkipAssignmentCheck();

        argvMaker.SetMaxRetries(INT_MAX);   // must allow lots of time for bridge B to run to completion. Unfortunately, there is no way to predict how many retries will be required.
    
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
    
    createFile(a_can_run);      // then a runs

    bridge_a.join();

    // TODO: Verify that B ran first and that A ran second and ran successfully

    runningServer.Stop(10*1000);    // must wait long enough for imodel-bank server to shut down. The time required can vary unpredictably but seems to increase when a second process has been running.
    }
    }

    // ASSERT_EQ(0, ProcessRunner::FindProcessId("node.exe"));
    // ASSERT_EQ(0, ProcessRunner::FindProcessId("iModelBridgeFwk.exe"));
    ASSERT_TRUE(bridge_a_sawRetryMsg);

    if (true)
        {
        // Since bridge A ran second, must have had to pull bridge B's changesets before it could push.
        // Therefore, A's briefcase should contain the RepositoryLink for B's input file as well as its own. 
        DbFileInfo info(bridge_a_briefcaseName);
        auto aSourceFileId = info.GetRepositoryLinkByFileNameLike("%test3d_a.dgn");
        auto bSourceFileId = info.GetRepositoryLinkByFileNameLike("%test3d_b.dgn");
        ASSERT_TRUE(aSourceFileId.IsValid());
        ASSERT_TRUE(bSourceFileId.IsValid());
        }

    if (true)
        {
        // Since bridge B ran first and finished before A could run, B's briefcase should not have a record of A's input file.
        DbFileInfo info(bridge_b_briefcaseName);
        auto aSourceFileId = info.GetRepositoryLinkByFileNameLike("%test3d_a.dgn");
        auto bSourceFileId = info.GetRepositoryLinkByFileNameLike("%test3d_b.dgn");
        ASSERT_FALSE(aSourceFileId.IsValid());
        ASSERT_TRUE(bSourceFileId.IsValid());
        }

    }

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

extern "C"
    {
    EXPORT_ATTRIBUTE T_iModelBridge_getAffinity iModelBridge_getAffinity;
    }

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

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(DEFAULT_IMODEL_NAME L".fwk-registry.db");
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

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(DEFAULT_IMODEL_NAME L".fwk-registry.db");
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

    AddAttachment(inputFile, refFile, 1, true);
    AddAttachment(inputFile, refFile, 1, true);
    int modelCount = 0;
    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        bvector<WString> margs(args);
        margs.push_back(L"--fwk-bridge-regsubkey=iModelBridgeForMstn");
        RunTheBridge(margs);

        modelCount = DbFileInfo(m_briefcaseName).GetModelCount();
        ASSERT_EQ(8, modelCount);
        }


    if (true)
        {
        //We added a new attachment.
        bvector<WString> rargs(args);
        rargs.push_back(L"--fwk-bridge-regsubkey=ABD");

        RunTheBridge(rargs);
        ASSERT_EQ(modelCount + 1, DbFileInfo(m_briefcaseName).GetModelCount());
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


    int modelCount = 0;
    if (true)
        {
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
        }
    
    if (true)
        {
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
        
        }

    
    if (true)
        {
        BentleyApi::BeFileName testDir3;
        SetupTestDirectory(testDir3, L"3", iModelName, inputFile2, guid2, refFile, refGuid);

        // Ask the framework to run our test bridge to do the initial conversion and create the repo
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

    if (true)
        {
        BentleyApi::BeFileName testDir4;
        SetupTestDirectory(testDir4, L"4", iModelName, inputFile2, guid2, refFile, refGuid);

        //We added a new attachment.
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
    }

//Sandwich test ?
//Test for locks and codes ?

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

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(DEFAULT_IMODEL_NAME L".fwk-registry.db");
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
static bool containsSubstr(BentleyApi::bset<BentleyApi::Utf8String> const& strings, BentleyApi::Utf8StringCR substr)
    {
    for (auto const& str: strings)
        {
        if (str.find(substr) != BentleyApi::Utf8String::npos)
            return true;
        }
    return false;
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

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(DEFAULT_IMODEL_NAME L".fwk-registry.db");
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
