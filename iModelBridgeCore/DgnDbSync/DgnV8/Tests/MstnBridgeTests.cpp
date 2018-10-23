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
    static DgnV8Api::DgnPlatformLib::Host& Gev8TesttHost()
        {
        static DgnV8Api::DgnPlatformLib::Host s_host;
        DgnV8Api::DgnPlatformLib::Initialize(s_host, true, false);
        return s_host;
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


    static BentleyApi::BeFileName GetSeedFile()
        {
        ScopedDgnHost host;

        //Initialize parameters needed to create a DgnDb
        CreateDgnDbParams createProjectParams;
        createProjectParams.SetRootSubjectName("iModelBridgeTests");

        BentleyApi::BeFileName seedDbName = GetSeedFilePath();
        BeFileName::CreateNewDirectory(seedDbName.GetDirectoryName().c_str());

        // Create the seed DgnDb file. The BisCore domain schema is also imported. 
        BentleyApi::BeSQLite::DbResult createStatus;
        DgnDbPtr db = DgnDb::CreateDgnDb(&createStatus, seedDbName, createProjectParams);
        
        // Force the seed db to have non-zero briefcaseid, so that changes made to it will be in a txn
        //db->SetAsBriefcase(BentleyApi::BeSQLite::BeBriefcaseId(BentleyApi::BeSQLite::BeBriefcaseId::Master()));
        db->SaveChanges();
        return seedDbName;
        }

    static void SetUpTestCase()
        {
        BentleyApi::BeFileName tmpDir;
        BentleyApi::BeTest::GetHost().GetTempDir(tmpDir);
        BentleyApi::BeFileName::CreateNewDirectory(tmpDir.c_str());

        Converter::InitializeDllPath(GetDgnv8BridgeDllName());

        BentleyApi::BeFileName platformAssetsDir;
        BentleyApi::BeTest::GetHost().GetDgnPlatformAssetsDirectory(platformAssetsDir);
        BentleyApi::BeFileName sqLangFile(platformAssetsDir);
        sqLangFile.AppendToPath(L"sqlang\\MstnBridgeTests_en-US.sqlang.db3");
        L10N::Initialize(BentleyApi::BeSQLite::L10N::SqlangFiles(sqLangFile));
        ScopedDgnHost host;
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

    void SetUpBridgeProcessingArgs(bvector<WString>& args, WCharCP testDir)
        {
        args.push_back(L"iModelBridgeTests.ConvertLinesUsingBridgeFwk");                                                 // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
        args.push_back(WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir));
        args.push_back(L"--server-environment=Qa");
        args.push_back(L"--server-repository=iModelBridgeTests_Test1");                             // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
        args.push_back(L"--server-project-guid=iModelBridgeTests_Project");                         // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
        args.push_back(L"--fwk-revision-comment=\"comment in quotes\"");
        args.push_back(L"--server-user=username=username");                                         // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
        args.push_back(L"--server-password=\"password><!@\"");                                      // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
        args.push_back(WPrintfString(L"--fwk-bridge-library=\"%s\"", GetDgnv8BridgeDllName().c_str()));     // must refer to a path that exists! 
        args.push_back(L"--fwk-skip-assignment-check");
        

        BentleyApi::BeFileName platformAssetsDir;
        BentleyApi::BeTest::GetHost().GetDgnPlatformAssetsDirectory(platformAssetsDir);
        args.push_back(WPrintfString(L"--fwk-bridgeAssetsDir=\"%ls\"", platformAssetsDir.c_str())); // must be a real assets dir! the platform's assets dir will serve just find as the test bridge's assets dir.
        }

    void AddAttachment(BentleyApi::BeFileName& inputFile, BentleyApi::BeFileNameR refV8File, int32_t num, bool useOffsetForElement)
        {
        bool adoptHost = NULL == DgnV8Api::DgnPlatformLib::QueryHost();
        if (adoptHost)
            DgnV8Api::DgnPlatformLib::AdoptHost(Gev8TesttHost());
        {
        V8FileEditor v8editor;
        v8editor.Open(inputFile);
        int offset = useOffsetForElement ? num : 0;
        v8editor.AddAttachment(refV8File, nullptr, Bentley::DPoint3d::FromXYZ((double) offset * 1000, (double) offset * 1000, 0));
        v8editor.Save();
        }
        if (adoptHost)
            DgnV8Api::DgnPlatformLib::ForgetHost();
        }

    void AddLine(BentleyApi::BeFileName& inputFile)
        {
        bool adoptHost = NULL == DgnV8Api::DgnPlatformLib::QueryHost();
        if (adoptHost)
            DgnV8Api::DgnPlatformLib::AdoptHost(Gev8TesttHost());
        {
        V8FileEditor v8editor;
        v8editor.Open(inputFile);
        DgnV8Api::ElementId eid1;
        v8editor.AddLine(&eid1);
        v8editor.Save();
        }
        if (adoptHost)
            DgnV8Api::DgnPlatformLib::ForgetHost();
        }

    int32_t GetElementCount(BentleyApi::BeFileName fileName)
        {
        ScopedDgnHost host;
        BentleyApi::BeSQLite::DbResult result;
        DgnDbPtr db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(BentleyApi::BeSQLite::Db::OpenMode::Readonly));
        EXPECT_EQ(BentleyApi::BeSQLite::DbResult::BE_SQLITE_OK, result);

        CachedStatementPtr stmt = db->Elements().GetStatement("SELECT count(*) FROM " BIS_TABLE(BIS_CLASS_Element) "");
        stmt->Step();
        return stmt->GetValueInt(0);
        }

    int32_t GetModelCount(BentleyApi::BeFileName fileName)
        {
        ScopedDgnHost host;
        BentleyApi::BeSQLite::DbResult result;
        DgnDbPtr db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(BentleyApi::BeSQLite::Db::OpenMode::Readonly));
        EXPECT_EQ(BentleyApi::BeSQLite::DbResult::BE_SQLITE_OK, result);

        CachedStatementPtr stmt = db->Elements().GetStatement("SELECT count(*) FROM " BIS_TABLE(BIS_CLASS_Model) "");
        stmt->Step();
        return stmt->GetValueInt(0);
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

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str());
    args.push_back(L"--fwk-bridge-regsubkey=iModelBridegForMstn");

    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);

    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    BentleyApi::Dgn::TestIModelHubClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);

    testIModelHubClientForBridges.CreateRepository("iModelBridgeTests_Test1", GetSeedFile());
    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"test1Assignments.db");
    //FakeRegistry testRegistry(testDir, assignDbName);
    //testRegistry.WriteAssignments();

    //testRegistry.AddRef(); // prevent ~iModelBridgeFwk from deleting this object.
    //iModelBridgeFwk::SetRegistryForTesting(testRegistry);   // (takes ownership of pointer)

    BentleyApi::BeFileName dbFile(testDir);
    dbFile.AppendToPath(L"iModelBridgeTests_Test1.bim");

    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        }

    int prevCount = GetElementCount(dbFile);
    if (true)
        {
        //Make sure a second run of the bridge does not add any elements.
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        EXPECT_EQ(prevCount, GetElementCount(dbFile));
        }


    AddLine(inputFile);
    if (true)
        {
        // and run an update
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        EXPECT_EQ(1 + prevCount, GetElementCount(dbFile));
        }

    if (true)
        {
        //Make sure a second run of the bridge does not add any elements.
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        EXPECT_EQ(1 + prevCount, GetElementCount(dbFile));
        }
    }

extern "C"
    {
    EXPORT_ATTRIBUTE T_iModelBridge_getAffinity iModelBridge_getAffinity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MstnBridgeTests, ConvertAttachment)
    {
    auto bridgeRegSubKey = L"iModelBridgeForMstn";

    auto testDir = getiModelBridgeTestsOutputDir(L"ConvertLinesUsingBridgeFwk2");

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));

    bvector<WString> args;
    SetUpBridgeProcessingArgs(args, testDir.c_str());
    args.push_back(L"--fwk-bridge-regsubkey=iModelBridegForMstn");
    
    BentleyApi::BeFileName inputFile;
    MakeCopyOfFile(inputFile, L"Test3d.dgn", NULL);
    args.push_back(WPrintfString(L"--fwk-input=\"%ls\"", inputFile.c_str()));

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    BentleyApi::Dgn::TestIModelHubClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);
    testIModelHubClientForBridges.CreateRepository("iModelBridgeTests_Test1", GetSeedFile());
    AddLine(inputFile);
    
    BentleyApi::BeFileName dbFile(testDir);
    dbFile.AppendToPath(L"iModelBridgeTests_Test1.bim");

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"test1Assignments.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();

    testRegistry.AddBridge(bridgeRegSubKey, iModelBridge_getAffinity);

    BentleyApi::BeFileName refFile;
    MakeCopyOfFile(refFile, L"Test3d.dgn", L"-Ref-1");

    AddLine(refFile);

    BentleyApi::BeSQLite::BeGuid guid, refGuid;
    guid.Create();
    refGuid.Create();

    iModelBridgeDocumentProperties docProps(guid.ToString().c_str(), "wurn1", "durn1", "other1", "");
    iModelBridgeDocumentProperties refDocProps(guid.ToString().c_str(), "wurn1", "durn1", "other1", "");
    testRegistry.SetDocumentProperties(docProps, inputFile);
    testRegistry.SetDocumentProperties(refDocProps, refFile);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, inputFile, L"");
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, refFile, L"");
    

    int modelCount = 0;
    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        modelCount = GetModelCount(dbFile);
        ASSERT_EQ(8, modelCount);
        }

   
    AddAttachment(inputFile, refFile, 1, true);
    if (true)
        {
        //We added a new attachment.
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        ASSERT_EQ(modelCount + 1, GetModelCount(dbFile));
        }

    }