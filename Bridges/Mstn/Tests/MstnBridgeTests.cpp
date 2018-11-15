/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/MstnBridgeTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "MstnBridgeTestsFixture.h"
#include <iModelBridge/TestIModelHubClientForBridges.h>
#include <iModelBridge/iModelBridgeFwk.h>
#include <iModelBridge/FakeRegistry.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>

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
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, ConvertLinesUsingBridgeFwk)
    {
    auto bridgeRegSubKey = L"iModelBridgeForMstn";

    auto testDir = getiModelBridgeTestsOutputDir(L"ConvertLinesUsingBridgeFwk");

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args);
    args.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir.c_str()));
    args.push_back(L"--fwk-bridge-regsubkey=iModelBridgeForMstn");

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));
    args.push_back(L"--fwk-skip-assignment-check");

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    TestIModelHubClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);

    testIModelHubClientForBridges.CreateRepository("iModelBridgeTests_Test1", GetSeedFile());
    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"test1Assignments.db");
    
    BentleyApi::BeFileName dbFile(testDir);
    dbFile.AppendToPath(L"iModelBridgeTests_Test1.bim");

    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);
        }

    int prevCount = DbFileInfo(dbFile).GetElementCount();
    if (true)
        {
        //Make sure a second run of the bridge does not add any elements.
        RunTheBridge(args);
        EXPECT_EQ(prevCount, DbFileInfo(dbFile).GetElementCount());
        }


    AddLine(inputFile);
    if (true)
        {
        // and run an update
        RunTheBridge(args);
        EXPECT_EQ(1 + prevCount, DbFileInfo(dbFile).GetElementCount());
        }

    if (true)
        {
        //Make sure a second run of the bridge does not add any elements.
        RunTheBridge(args);
        EXPECT_EQ(1 + prevCount, DbFileInfo(dbFile).GetElementCount());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, TestSourceElementIdAspect)
    {
    auto bridgeRegSubKey = L"iModelBridgeForMstn";

    auto testDir = getiModelBridgeTestsOutputDir(L"TestSourceElementIdAspect");

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args);
    args.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir.c_str()));
    args.push_back(L"--fwk-bridge-regsubkey=iModelBridgeForMstn");
    args.push_back(L"--fwk-storeElementIdsInBIM");
    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));
    args.push_back(L"--fwk-skip-assignment-check");

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    TestIModelHubClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);

    testIModelHubClientForBridges.CreateRepository("iModelBridgeTests_Test1", GetSeedFile());
    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"test1Assignments.db");

    BentleyApi::BeFileName dbFile(testDir);
    dbFile.AppendToPath(L"iModelBridgeTests_Test1.bim");
    
    int64_t srcId = AddLine(inputFile);
    if (true)
        {
        // and run an update
        RunTheBridge(args);
        }
    DbFileInfo info(dbFile);
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
TEST_F(MstnBridgeTests, ConvertAttachmentSingleBridge)
    {
    auto bridgeRegSubKey = L"iModelBridgeForMstn";

    auto testDir = getiModelBridgeTestsOutputDir(L"ConvertAttachmentSingleBridge");

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args);
    args.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir.c_str()));
    args.push_back(L"--fwk-bridge-regsubkey=iModelBridgeForMstn");
    
    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);
    AddLine(inputFile);

    BentleyApi::BeFileName refFile;
    MakeCopyOfFile(refFile, L"Test3d.dgn", L"-Ref-1");
    AddLine(refFile);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    TestIModelHubClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);
    testIModelHubClientForBridges.CreateRepository("iModelBridgeTests_Test1", GetSeedFile());
    
    
    BentleyApi::BeFileName dbFile(testDir);
    dbFile.AppendToPath(L"iModelBridgeTests_Test1.bim");

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"iModelBridgeTests_Test1.fwk-registry.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    testRegistry.AddBridge(bridgeRegSubKey, iModelBridge_getAffinity);

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
        
        modelCount = DbFileInfo(dbFile).GetModelCount();
        ASSERT_EQ(8, modelCount);
        }

   
    AddAttachment(inputFile, refFile, 1, true);
    AddAttachment(inputFile, refFile, 1, true);
    if (true)
        {
        //We added a new attachment.
        RunTheBridge(args);
        ASSERT_EQ(modelCount + 1, DbFileInfo(dbFile).GetModelCount());
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, ConvertAttachmentMultiBridge)
    {
    
    auto testDir = getiModelBridgeTestsOutputDir(L"ConvertAttachmentMultiBridge");

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args);
    args.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir.c_str()));

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    TestIModelHubClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);
    testIModelHubClientForBridges.CreateRepository("iModelBridgeTests_Test1", GetSeedFile());

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);
    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));
    AddLine(inputFile);

    BentleyApi::BeFileName dbFile(testDir);
    dbFile.AppendToPath(L"iModelBridgeTests_Test1.bim");

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"iModelBridgeTests_Test1.fwk-registry.db");
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

        modelCount = DbFileInfo(dbFile).GetModelCount();
        ASSERT_EQ(8, modelCount);
        }


    if (true)
        {
        //We added a new attachment.
        bvector<WString> rargs(args);
        rargs.push_back(L"--fwk-bridge-regsubkey=ABD");

        RunTheBridge(rargs);
        ASSERT_EQ(modelCount + 1, DbFileInfo(dbFile).GetModelCount());
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, ConvertAttachmentMultiBridgeSharedReference)
    {
    BentleyApi::BeFileName iModelHubDir = getiModelBridgeTestsOutputDir(L"ConvertAttachmentMultiBridgeSharedReference-iModelHub");
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(iModelHubDir));
    
    bvector<WString> args;
    SetUpBridgeProcessingArgs(args);
    
    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    TestIModelHubClientForBridges testIModelHubClientForBridges(iModelHubDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);
    testIModelHubClientForBridges.CreateRepository("iModelBridgeTests_Test1", GetSeedFile());

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
        SetupTestDirectory(testDir1, L"ConvertAttachmentMultiBridgeSharedReference1", inputFile1, guid1, refFile, refGuid);

        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        bvector<WString> margs(args);
        margs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile1.c_str()));
        margs.push_back(L"--fwk-bridge-regsubkey=iModelBridgeForMstn");
        margs.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir1.c_str()));
        RunTheBridge(margs);

        BentleyApi::BeFileName dbFile(testDir1);
        dbFile.AppendToPath(L"iModelBridgeTests_Test1.bim");
        DbFileInfo info(dbFile);
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
        SetupTestDirectory(testDir2, L"ConvertAttachmentMultiBridgeSharedReference2", inputFile1, guid1, refFile, refGuid);

        //We added a new attachment.
        bvector<WString> rargs(args);
        rargs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile1.c_str()));
        rargs.push_back(L"--fwk-bridge-regsubkey=ABD");
        rargs.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir2.c_str()));

        RunTheBridge(rargs);
        BentleyApi::BeFileName dbFile(testDir2);
        dbFile.AppendToPath(L"iModelBridgeTests_Test1.bim");

        DbFileInfo info(dbFile);
        ASSERT_EQ(++modelCount, info.GetPhysicalModelCount());

        int provenanceCount1 = info.GetModelProvenanceCount(guid1);
        int provenanceCountRef = info.GetModelProvenanceCount(refGuid);
        ASSERT_EQ(1, provenanceCount1);
        ASSERT_EQ(1, provenanceCountRef);
        
        }

    
    if (true)
        {
        BentleyApi::BeFileName testDir3;
        SetupTestDirectory(testDir3, L"ConvertAttachmentMultiBridgeSharedReference3", inputFile2, guid2, refFile, refGuid);

        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        bvector<WString> margs(args);
        margs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile2.c_str()));
        margs.push_back(L"--fwk-bridge-regsubkey=iModelBridgeForMstn");
        margs.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir3.c_str()));
        RunTheBridge(margs);
        BentleyApi::BeFileName dbFile(testDir3);
        dbFile.AppendToPath(L"iModelBridgeTests_Test1.bim");

        DbFileInfo info(dbFile);
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
        SetupTestDirectory(testDir4, L"ConvertAttachmentMultiBridgeSharedReference4", inputFile2, guid2, refFile, refGuid);

        //We added a new attachment.
        bvector<WString> rargs(args);
        rargs.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile2.c_str()));
        rargs.push_back(L"--fwk-bridge-regsubkey=ABD");
        rargs.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir4.c_str()));
        BentleyApi::BeFileName dbFile(testDir4);
        dbFile.AppendToPath(L"iModelBridgeTests_Test1.bim");
        RunTheBridge(rargs);
        DbFileInfo info(dbFile);
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
    auto testDir = getiModelBridgeTestsOutputDir(L"PushAfterEachModel");

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    TestIModelHubClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);
    testIModelHubClientForBridges.CreateRepository("iModelBridgeTests_Test1", GetSeedFile());

    // Set up to process a master file and two reference files, all mapped to MstnBridge
    bvector<WString> args;
    SetUpBridgeProcessingArgs(args);
    SetUpBridgeProcessingArgs(args, testDir, MSTN_BRIDGE_REG_SUB_KEY);

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"iModelBridgeTests_Test1.fwk-registry.db");
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

    BentleyApi::BeFileName dbFile(testDir);
    dbFile.AppendToPath(L"iModelBridgeTests_Test1.bim");

    int modelCount = 0;
    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);
        
        modelCount = DbFileInfo(dbFile).GetModelCount();
        ASSERT_EQ(8, modelCount);
        }
   
    // Add two attachments => two new models should be discovered.
    AddAttachment(masterFile, refFile1, 1, true);
    AddAttachment(masterFile, refFile2, 1, true);
    if (true)
        {
        auto csCountBefore = testIModelHubClientForBridges.GetChangesetCount();
        RunTheBridge(args);
        DbFileInfo fileInfo(dbFile);
        EXPECT_EQ(modelCount + 2, fileInfo.GetModelCount());
        // There will be more than just the two model-specific changesets. Assert at least 2 more.
        EXPECT_GE(testIModelHubClientForBridges.GetChangesetCount(), csCountBefore + 2) << "each model should have been pushed in its own changeset";

        auto revstats = testIModelHubClientForBridges.ComputeRevisionStats(*fileInfo.m_db, csCountBefore);
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
    auto testDir = getiModelBridgeTestsOutputDir(L"PushAfterEachFile");

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));
     
    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    TestIModelHubClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);
    testIModelHubClientForBridges.CreateRepository("iModelBridgeTests_Test1", GetSeedFile());

    // Set up to process a master file and two reference files, all mapped to MstnBridge
    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir, MSTN_BRIDGE_REG_SUB_KEY);

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"iModelBridgeTests_Test1.fwk-registry.db");
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

    BentleyApi::BeFileName dbFile(testDir);
    dbFile.AppendToPath(L"iModelBridgeTests_Test1.bim");
    
    int modelCount = 0;
    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        RunTheBridge(args);
        
        modelCount = DbFileInfo(dbFile).GetModelCount();
        ASSERT_EQ(8, modelCount);
        }
   
    // Add two attachments => two new models should be discovered.
    AddAttachment(masterFile, refFile1, 1, true);
    AddAttachment(masterFile, refFile2, 1, true);
    if (true)
        {
        auto csCountBefore = testIModelHubClientForBridges.GetChangesetCount();
        RunTheBridge(args);
        DbFileInfo fileInfo(dbFile);
        EXPECT_EQ(modelCount + 2, fileInfo.GetModelCount());
        // There will be more than just the two file-specific changesets. Assert at least 2 more.
        EXPECT_GE(testIModelHubClientForBridges.GetChangesetCount(), csCountBefore + 2) << "each model should have been pushed in its own changeset";

        auto revstats = testIModelHubClientForBridges.ComputeRevisionStats(*fileInfo.m_db, csCountBefore);
        EXPECT_EQ(revstats.nSchemaRevs, 0) << "no schema changes expected, since the initial conversion did that";
        EXPECT_GE(revstats.nDataRevs, 2) << "each file should have been pushed in its own changeset";
        EXPECT_TRUE(containsSubstr(revstats.descriptions, "-Ref-1")) << "first ref should have been pushed in its own revision";
        EXPECT_TRUE(containsSubstr(revstats.descriptions, "-Ref-2")) << "second ref should have been pushed in its own revision";
        }
    }