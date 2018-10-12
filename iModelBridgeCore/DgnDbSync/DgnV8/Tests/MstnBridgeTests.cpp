/*--------------------------------------------------------------------------------------+
|
|  $Source: DgnV8/Tests/MstnBridgeTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ConverterInternal.h"
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <Bentley/BeTest.h>
#include <iModelBridge/TestIModelHubClientForBridges.h>
#include <iModelBridge/iModelBridgeFwk.h>
#include <iModelBridge/FakeRegistry.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#include "V8FileEditor.h"

#define MAKE_ARGC_ARGV(argptrs, args)\
for (auto& arg: args)\
   argptrs.push_back(arg.c_str());\
int argc = (int)argptrs.size();\
wchar_t const** argv = argptrs.data();\


//=======================================================================================
// @bsistruct                                                   Sam.Wilson   04/17
//=======================================================================================
struct MstnBridgeTests : ::testing::Test
    {
    static BentleyApi::BeFileName GetOutputDir()
        {
        BentleyApi::BeFileName testDir;
        BentleyApi::BeTest::GetHost().GetOutputRoot(testDir);
        testDir.AppendToPath(L"iModelBridgeTests");
        testDir.AppendToPath(L"Dgnv8Bridge");
        return testDir;
        }

    static BentleyApi::BeFileName  GetOutputFileName(BentleyApi::WCharCP filename)
        {
        BentleyApi::BeFileName filepath = GetOutputDir();
        filepath.AppendToPath(filename);
        return filepath;
        }

    static BentleyApi::BeFileName GetSeedFilePath() { auto path = GetOutputDir(); path.AppendToPath(L"seed.bim"); return path; }

    static BentleyApi::BeFileName GetDgnv8BridgeDllName()
        {
        auto fileName =  BentleyApi::Desktop::FileSystem::GetExecutableDir();
        fileName.AppendToPath(L"Dgnv8BridgeB02.dll");
        return fileName;
        }

    static void SetUpTestCase()
        {
        //BentleyApi::BeFileName tmpDir;
        //BentleyApi::BeTest::GetHost().GetTempDir(tmpDir);
        //BentleyApi::BeFileName::CreateNewDirectory(tmpDir.c_str());

        Converter::InitializeDllPath(GetDgnv8BridgeDllName());

        BentleyApi::BeFileName seedDbName(GetSeedFilePath());
        BeFileName::CreateNewDirectory(seedDbName.GetDirectoryName().c_str());

        BentleyApi::BeFileName platformAssetsDir;
        BentleyApi::BeTest::GetHost().GetDgnPlatformAssetsDirectory(platformAssetsDir);
        BentleyApi::BeFileName sqLangFile(platformAssetsDir);
        sqLangFile.AppendToPath(L"sqlang\\MstnBridgeTests_en-US.sqlang.db3");
        L10N::Initialize(BentleyApi::BeSQLite::L10N::SqlangFiles(sqLangFile));
        
        ScopedDgnHost host;
        

        // Initialize parameters needed to create a DgnDb
        CreateDgnDbParams createProjectParams;
        createProjectParams.SetRootSubjectName("iModelBridgeTests");

        // Create the seed DgnDb file. The BisCore domain schema is also imported. 
        BentleyApi::BeSQLite::DbResult createStatus;
        DgnDbPtr db = DgnDb::CreateDgnDb(&createStatus, seedDbName, createProjectParams);
        ASSERT_TRUE(db.IsValid());

        // Force the seed db to have non-zero briefcaseid, so that changes made to it will be in a txn
        db->SetAsBriefcase(BentleyApi::BeSQLite::BeBriefcaseId(BentleyApi::BeSQLite::BeBriefcaseId::Standalone()));
        db->SaveChanges();
        }

    static BentleyApi::BeFileName getiModelBridgeTestsOutputDir(WCharCP subdir)
        {
        BentleyApi::BeFileName testDir = GetOutputDir();
        testDir.AppendToPath(subdir);
        return testDir;
        }

    void MakeCopyOfFile(BentleyApi::BeFileNameR outFile, BentleyApi::WCharCP filename,  BentleyApi::WCharCP suffix)
        {
        BentleyApi::BeFileName filepath;
        BentleyApi::BeTest::GetHost().GetDocumentsRoot(filepath);
        filepath.AppendToPath(L"TestData");
        filepath.AppendToPath(filename);

        outFile = GetOutputFileName(filename).GetDirectoryName();
        outFile.AppendToPath(filepath.GetFileNameWithoutExtension().c_str());
        if (suffix)
            outFile.append(suffix);
        outFile.append(L".");
        outFile.append(filepath.GetExtension().c_str());

        
        ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(filepath.c_str(), outFile.c_str()));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, ConvertLinesUsingBridgeFwk)
    {
    auto bridgeRegSubKey = L"iModelBridgeForMstn";

    auto testDir = getiModelBridgeTestsOutputDir(L"ConvertLinesUsingBridgeFwk");

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));

    // I have to create a file that I represent as the bridge "library", so that the fwk's argument validation logic will see that it exists.
    // The fwk won't try to load this file, since we will register a fake bridge.
    

    bvector<WString> args;
    args.push_back(L"iModelBridgeTests.ConvertLinesUsingBridgeFwk");                                                 // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir.c_str()));
    args.push_back(L"--server-environment=Qa");
    args.push_back(L"--server-repository=iModelBridgeTests_Test1");                             // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-project-guid=iModelBridgeTests_Project");                         // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--fwk-create-repository-if-necessary");
    args.push_back(L"--fwk-revision-comment=\"comment in quotes\"");
    args.push_back(L"--server-user=username=username");                                         // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-password=\"password><!@\"");                                      // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(WPrintfString(L"--fwk-bridge-library=\"%s\"", GetDgnv8BridgeDllName().c_str()));     // must refer to a path that exists! 
    args.push_back(L"--fwk-skip-assignment-check");
    args.push_back(L"--fwk-bridge-regsubkey=iModelBridegForMstn");
    
    BentleyApi::BeFileName platformAssetsDir;
    BentleyApi::BeTest::GetHost().GetDgnPlatformAssetsDirectory(platformAssetsDir);
    args.push_back(WPrintfString(L"--fwk-bridgeAssetsDir=\"%ls\"", platformAssetsDir.c_str())); // must be a real assets dir! the platform's assets dir will serve just find as the test bridge's assets dir.

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    BentleyApi::Dgn::TestIModelHubClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);

    // Register the test bridge that fwk should run
    //iModelBridgeTests_Test1_Bridge testBridge(testIModelHubClientForBridges);
    //iModelBridgeFwk::SetBridgeForTesting(testBridge);

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"test1Assignments.db");
    //FakeRegistry testRegistry(testDir, assignDbName);
    //testRegistry.WriteAssignments();

    //testRegistry.AddRef(); // prevent ~iModelBridgeFwk from deleting this object.
    //iModelBridgeFwk::SetRegistryForTesting(testRegistry);   // (takes ownership of pointer)

    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        }
    
    V8FileEditor v8editor;
    v8editor.Open(inputFile);
    DgnV8Api::ElementId eid1;
    v8editor.AddLine(&eid1);
    v8editor.Save();

    if (true)
        {
        // Modify an item 
        

        // and run an update
        // This time, we expect to find the repo and briefcase already there.
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        }

    if (true)
        {
        // Run an update with no changes
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        }
    }