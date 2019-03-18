/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/MstnBridgeTestsFixture.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <windows.h>
#include "ConverterInternal.h"
#include "MstnBridgeTestsFixture.h"
#include <iModelBridge/TestIModelHubClientForBridges.h>
#include <iModelBridge/iModelBridgeFwk.h>
#include <iModelBridge/FakeRegistry.h>
#include <BeHttp/HttpClient.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#include "V8FileEditor.h"

#define MAKE_ARGC_ARGV(argptrs, args)\
for (auto& arg: args)\
   argptrs.push_back(arg.c_str());\
int argc = (int)argptrs.size();\
wchar_t const** argv = argptrs.data();\

BentleyApi::Dgn::IModelClientForBridges* MstnBridgeTestsFixture::s_client;

static PROCESS_INFORMATION s_iModelBankProcess;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName MstnBridgeTestsFixture::GetOutputDir()
    {
    BentleyApi::BeFileName testDir;
    BentleyApi::BeTest::GetHost().GetOutputRoot(testDir);
    return testDir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct ScopedDgnv8Host
    {
    DgnV8Api::DgnPlatformLib::Host m_host;
    bool m_initDone {};
    void Init()
        {
        DgnV8Api::DgnPlatformLib::Initialize(m_host, true, true);
        m_initDone = true;
        }
    ~ScopedDgnv8Host()
        {
        if (m_initDone)
            m_host.Terminate(false);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName  MstnBridgeTestsFixture::GetOutputFileName(BentleyApi::WCharCP filename)
    {
    BentleyApi::BeFileName filepath = GetOutputDir();
    filepath.AppendToPath(filename);
    return filepath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName MstnBridgeTestsFixture::GetDgnv8BridgeDllName()
    {
    auto fileName = BentleyApi::Desktop::FileSystem::GetExecutableDir();
    fileName.AppendToPath(L"Dgnv8BridgeM02.dll");
    return fileName;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName MstnBridgeTestsFixture::GetSeedFile()
    {
    ScopedDgnHost host;

    //Initialize parameters needed to create a DgnDb
    BentleyApi::BeSQLite::BeGuid guid;
    guid.FromString("233e1f55-561d-42a4-8e80-d6f91743863e");    // Set the DbGuid, which is required by imodel-bank server
    CreateDgnDbParams createProjectParams(guid);
    createProjectParams.SetRootSubjectName("iModelBridgeTests");

    BentleyApi::BeFileName seedDbName = GetSeedFilePath();
    BeFileName::CreateNewDirectory(seedDbName.GetDirectoryName().c_str());

    BeFileName::BeDeleteFile(seedDbName.c_str());

    // Create the seed DgnDb file. The BisCore domain schema is also imported. 
    BentleyApi::BeSQLite::DbResult createStatus;
    DgnDbPtr db = DgnDb::CreateDgnDb(&createStatus, seedDbName, createProjectParams);

    db->SaveChanges();
    return seedDbName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::SetUpTestCase()
    {
    BentleyApi::BeFileName tmpDir;
    BentleyApi::BeTest::GetHost().GetTempDir(tmpDir);
    BentleyApi::BeFileName::CreateNewDirectory(tmpDir.c_str());
    BentleyApi::BeFileName::CreateNewDirectory(GetOutputDir());

    Converter::InitializeDllPath(GetDgnv8BridgeDllName());

    BentleyApi::BeFileName platformAssetsDir;
    BentleyApi::BeTest::GetHost().GetDgnPlatformAssetsDirectory(platformAssetsDir);
    BentleyApi::BeFileName sqLangFile(platformAssetsDir);
    sqLangFile.AppendToPath(L"sqlang\\MstnBridgeTests_en-US.sqlang.db3");
    L10N::Initialize(BentleyApi::BeSQLite::L10N::SqlangFiles(sqLangFile));
    ScopedDgnHost host; // This statically initializes SQLite

    SetupClient();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::TearDownTestCase()
    {
    StopImodelBankServer();
    BentleyApi::BeFileName::EmptyAndRemoveDirectory(GetOutputDir());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName MstnBridgeTestsFixture::GetTestDataDir()
    {
    BentleyApi::BeFileName dir;
    BentleyApi::BeTest::GetHost().GetDocumentsRoot(dir);
    dir.AppendToPath(L"TestData");
    return dir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::MakeCopyOfFile(BentleyApi::BeFileNameR outFile, BentleyApi::WCharCP filename, BentleyApi::WCharCP suffix)
    {
    BentleyApi::BeFileName filepath = GetTestDataDir();
    filepath.AppendToPath(filename);

    outFile = GetOutputFileName(filename).GetDirectoryName();
    outFile.AppendToPath(filepath.GetFileNameWithoutExtension().c_str());
    if (suffix)
        outFile.append(suffix);
    outFile.append(L".");
    outFile.append(filepath.GetExtension().c_str());


    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(filepath.c_str(), outFile.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::SetUpBridgeProcessingArgs(BentleyApi::bvector<BentleyApi::WString>& args, WCharCP stagingDir, WCharCP bridgeRegSubkey, WCharCP iModelName)
    {
    args.push_back(L"iModelBridgeTests.argv0"); // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg

    auto rspFileName = UsingIModelBank()? L"imodel-bank.rsp": L"imodel-hub.rsp";
    BentleyApi::bvector<BentleyApi::WString> strings;
    BentleyApi::bvector<BentleyApi::WCharCP> ptrs;
    BentleyApi::bvector<WCharCP> bargptrs;
    ASSERT_EQ(BSISUCCESS, ParseArgsFromRspFile(strings, ptrs, ReadRspFile(rspFileName)));
    for (auto arg : ptrs)
        args.push_back(arg);

    if (nullptr == iModelName)
        iModelName = DEFAULT_IMODEL_NAME;

    if (UsingIModelBank())
        args.push_back(BentleyApi::WPrintfString(L"--imodel-bank-imodel-name=%s", iModelName).c_str()); // TRICKY: This determines the name of the briefcase (not the name of the iModel inside iModelBank).
    else
        args.push_back(BentleyApi::WPrintfString(L"--server-repository=%s", iModelName).c_str());

    args.push_back(L"--fwk-revision-comment=\"comment in quotes\"");

    args.push_back(BentleyApi::WPrintfString(L"--fwk-bridge-library=\"%s\"", GetDgnv8BridgeDllName().c_str()));     // must refer to a path that exists! 

    BentleyApi::BeFileName platformAssetsDir;
    BentleyApi::BeTest::GetHost().GetDgnPlatformAssetsDirectory(platformAssetsDir);
    args.push_back(BentleyApi::WPrintfString(L"--fwk-bridgeAssetsDir=\"%ls\"", platformAssetsDir.c_str())); // must be a real assets dir! the platform's assets dir will serve just find as the test bridge's assets dir.

    if (stagingDir)
        args.push_back(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", stagingDir));
    if (bridgeRegSubkey)
        args.push_back(BentleyApi::WPrintfString(L"--fwk-bridge-regsubkey=%ls", bridgeRegSubkey));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::AddAttachment(BentleyApi::BeFileName& inputFile, BentleyApi::BeFileNameR refV8File, int32_t num, bool useOffsetForElement)
    {
    ScopedDgnv8Host testHost;
    bool adoptHost = NULL == DgnV8Api::DgnPlatformLib::QueryHost();
    if (adoptHost)
        testHost.Init();
    
    V8FileEditor v8editor;
    v8editor.Open(inputFile);
    int offset = useOffsetForElement ? num : 0;
    v8editor.AddAttachment(refV8File, nullptr, Bentley::DPoint3d::FromXYZ((double) offset * 1000, (double) offset * 1000, 0));
    v8editor.Save();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t MstnBridgeTestsFixture::AddLine(BentleyApi::BeFileName& inputFile, int num)
    {
    int64_t elementId = 0;
    ScopedDgnv8Host testHost;
    bool adoptHost = NULL == DgnV8Api::DgnPlatformLib::QueryHost();
    if (adoptHost)
        testHost.Init();
    
    V8FileEditor v8editor;
    v8editor.Open(inputFile);
    for (int index = 0; index < num; ++index)
        {
        DgnV8Api::ElementId eid1;
        v8editor.AddLine(&eid1,nullptr, Bentley::DPoint3d::FromXYZ((double) index * 1000, (double) index * 1000, 0));
        if (index == 0)
            elementId = eid1;
        }
    v8editor.Save();
    
    return elementId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t MstnBridgeTestsFixture::DbFileInfo::GetElementCount()
    {
    CachedStatementPtr stmt = m_db->Elements().GetStatement("SELECT count(*) FROM " BIS_TABLE(BIS_CLASS_Element) "");
    stmt->Step();
    return stmt->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MstnBridgeTestsFixture::DbFileInfo::DbFileInfo(BentleyApi::BeFileNameCR fileName)
    {
    
    BentleyApi::BeSQLite::DbResult result;
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(BentleyApi::BeSQLite::Db::OpenMode::Readonly));
    BeAssert(BentleyApi::BeSQLite::DbResult::BE_SQLITE_OK ==  result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t MstnBridgeTestsFixture::DbFileInfo::GetModelCount ()
    {
    CachedStatementPtr stmt = m_db->Elements().GetStatement("SELECT count(*) FROM " BIS_TABLE(BIS_CLASS_Model) "");
    stmt->Step();
    return stmt->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t MstnBridgeTestsFixture::DbFileInfo::GetPhysicalModelCount()
    {
    ModelIterator iterator = m_db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalModel));
    return iterator.BuildIdSet().size();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t MstnBridgeTestsFixture::DbFileInfo::GetBISClassCount(CharCP className)
    {
    CachedStatementPtr stmt = m_db->Elements().GetStatement(Utf8PrintfString ("SELECT count(*) FROM %s ", className));
    stmt->Step();
    return stmt->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t MstnBridgeTestsFixture::DbFileInfo::GetModelProvenanceCount(BentleyApi::BeSQLite::BeGuidCR fileGuid)
    {
    Utf8String ecsql("SELECT Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE ( (Scope.Id=?) AND (Kind ='DocumentWithBeGuid') AND (Identifier= ?))");
    
    auto stmt = m_db->GetPreparedECSqlStatement(ecsql.c_str());

    if (!stmt.IsValid())
        return 0;

    stmt->BindId(1, m_db->Elements().GetRootSubjectId());
    BentleyApi::Utf8String guidString = fileGuid.ToString();
    stmt->BindText(2, guidString.c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
    
    if (BE_SQLITE_ROW != stmt->Step())
        return 0;
    
    DgnElementId repoLink = stmt->GetValueId<DgnElementId>(0);

    Utf8String modelSQL("SELECT count(*) FROM " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE ( (Scope.Id=?) AND (Kind = 'Model') )");

    auto modelStmt = m_db->GetPreparedECSqlStatement(modelSQL.c_str());
    modelStmt->BindId(1, repoLink);
    if (BE_SQLITE_ROW != modelStmt->Step())
        return 0;
    
    return  modelStmt->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::TerminateHost()
    {
    //We need to shut down v8host at the end so that rest of the processing works.
    DgnV8Api::DgnPlatformLib::Host* host = DgnV8Api::DgnPlatformLib::QueryHost();
    if (NULL != host)
        host->Terminate(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName MstnBridgeTestsFixture::CreateTestDir(WCharCP testDir)
    {
    // Start with a COMPLETELY CLEAN SLATE
    BentleyApi::BeFileName::EmptyAndRemoveDirectory (GetOutputDir().c_str());
    EXPECT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::CreateNewDirectory (GetOutputDir().c_str()));

    if (testDir == nullptr)
        return GetOutputDir();

    // If the test wants to run in a subdirectory, make it.
    BentleyApi::BeFileName testDirPath(testDir);
    if (!testDirPath.StartsWith(GetOutputDir()))
        {
        BeAssert(!testDirPath.IsAbsolutePath() && "Test directories must be under the output directory");
        testDirPath = GetOutputDir();
        testDirPath.AppendToPath(testDir);
        }
    
    BeAssert(testDirPath.IsAbsolutePath() && testDirPath.StartsWith(GetOutputDir()) && "Test directories must be under the output directory");

    EXPECT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::CreateNewDirectory(testDirPath.c_str()));
    
    return testDirPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void getInfoForAllChangeSets(BentleyApi::bvector<BentleyApi::iModel::Hub::ChangeSetInfoPtr>& infos, IModelClientBase* clientWrapper)
    {
    BentleyApi::Http::HttpClient::Reinitialize();
    
    auto info = clientWrapper->GetIModelInfo();
    ASSERT_TRUE(info.IsValid());
    auto client = clientWrapper->GetImodelHubClientPtr();
    ASSERT_TRUE(client.IsValid());
    auto connectionResult = client->ConnectToiModel(*info)->GetResult();
    ASSERT_TRUE(connectionResult.IsSuccess());
    auto connection = connectionResult.GetValue();
    auto changesetsResult = connection->GetAllChangeSets()->GetResult();
    ASSERT_TRUE(changesetsResult.IsSuccess());
    infos = changesetsResult.GetValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MstnBridgeTestsFixture::GetChangesetCount()
    {
    auto testClient = dynamic_cast<BentleyApi::Dgn::TestIModelHubClientForBridges*>(&GetClient());
    if (nullptr != testClient)
        return testClient->GetDgnRevisions().size();

    BentleyApi::bvector<BentleyApi::iModel::Hub::ChangeSetInfoPtr> changesets;
    getInfoForAllChangeSets(changesets, dynamic_cast<IModelClientBase*>(&GetClient()));
    return changesets.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
RevisionStats MstnBridgeTestsFixture::ComputeRevisionStats(BentleyApi::Dgn::DgnDbR db, size_t start, size_t end)
    {
    RevisionStats stats{};

    auto testClient = dynamic_cast<BentleyApi::Dgn::TestIModelHubClientForBridges*>(&GetClient());
    if (nullptr != testClient)
        {
        for (auto rev : testClient->GetDgnRevisions(start, end))
            {
            stats.descriptions.insert(rev->GetSummary());
            stats.userids.insert(rev->GetUserName());
            if (rev->ContainsSchemaChanges(db))
                ++stats.nSchemaRevs;
            else
                ++stats.nDataRevs;
            }
        return stats;
        }

    BentleyApi::bvector<BentleyApi::iModel::Hub::ChangeSetInfoPtr> changesets;
    getInfoForAllChangeSets(changesets, dynamic_cast<IModelClientBase*>(&GetClient()));

    if (end < 0 || end > changesets.size())
        end = changesets.size();

    for (size_t i = start; i < end; ++i)
        {
        auto csInfo = changesets[i];
        stats.descriptions.insert(csInfo->GetDescription());
        stats.userids.insert(csInfo->GetUserCreated());
        bool isSchemaChange = csInfo->GetContainingChanges() == BentleyApi::iModel::Hub::ChangeSetInfo::ContainingChanges::Schema;
        if (isSchemaChange)
            ++stats.nSchemaRevs;
        else
            ++stats.nDataRevs;
        }
    return stats;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus readEntireFile(BentleyApi::WStringR contents, BentleyApi::BeFileNameCR fileName)
    {
    BentleyApi::BeFile errfile;
    if (BentleyApi::BeFileStatus::Success != errfile.Open(fileName.c_str(), BentleyApi::BeFileAccess::Read))
        return BSIERROR;

    BentleyApi::bvector<Byte> bytes;
    if (BentleyApi::BeFileStatus::Success != errfile.ReadEntireFile(bytes))
        return BSIERROR;

    if (bytes.empty())
        return BSISUCCESS;

    bytes.push_back('\0'); // End of stream

    const Byte utf8BOM[] = {0xef, 0xbb, 0xbf};
    if (bytes[0] == utf8BOM[0] || bytes[1] == utf8BOM[1] || bytes[2] == utf8BOM[2])
        contents.AssignUtf8((Utf8CP) (bytes.data() + 3));
    else
        contents.AssignA((char*) bytes.data());

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::WString MstnBridgeTestsFixture::ReadRspFile(BentleyApi::WCharCP fn)
    {
    BentleyApi::BeFileName rspFileName;
    BentleyApi::BeTest::GetHost().GetDocumentsRoot(rspFileName);
    rspFileName.AppendToPath(L"TestData");
    rspFileName.AppendToPath(fn);

    BentleyApi::WString wargs;
    if (BSISUCCESS != readEntireFile(wargs, rspFileName))
        {
        fwprintf(stderr, L"%ls - response file not found\n", rspFileName.c_str());
        return L"";
        }

    return wargs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BentleyStatus MstnBridgeTestsFixture::ParseArgsFromRspFile(BentleyApi::bvector<BentleyApi::WString>& strings, BentleyApi::bvector<BentleyApi::WCharCP>& ptrs, BentleyApi::WStringCR wargs)
    {
    BentleyApi::BeStringUtilities::ParseArguments(strings, wargs.c_str(), L"\n\r");

    if (strings.empty())
        return BentleyApi::BSIERROR;
        
    ptrs.push_back(L"<argv0 placeholder>");
    for (auto const& str: strings)
        {
        if (!str.empty())
            ptrs.push_back(str.c_str());
        }

    return BentleyApi::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::SetupClient()
    {
    // TODO: Read client info from .json file on disk;
    static Utf8CP s_productId = "1654"; // Navigator Desktop
    // MT Note: C++11 guarantees that the following line of code will be executed only once and in a thread-safe manner:
    BentleyApi::WebServices::ClientInfoPtr clientInfo = BentleyApi::WebServices::ClientInfoPtr(
        new BentleyApi::WebServices::ClientInfo("Bentley-Test", BentleyApi::BeVersion(1, 0), "{41FE7A91-A984-432D-ABCF-9B860A8D5360}", "TestDeviceId", "TestSystem", s_productId, nullptr));

    // TODO: Consult argv and check for --imodel-bank or --imodel-hub
    auto imbPath = getenv("MSTN_BRIDGE_TESTS_IMODEL_BANK_SERVER_JS");
    BentleyApi::bvector<BentleyApi::WString> strings;
    BentleyApi::bvector<BentleyApi::WCharCP> ptrs;
    BentleyApi::bvector<WCharCP> bargptrs;
    if (imbPath != nullptr)
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("MstnBridgeTests")->trace("Using IModelBankClient");
        ASSERT_EQ(BSISUCCESS, ParseArgsFromRspFile(strings, ptrs, ReadRspFile(L"imodel-bank.rsp")));
        iModelBridgeFwk::IModelBankArgs imbArgs;
        ASSERT_EQ(BSISUCCESS, imbArgs.ParseCommandLine(bargptrs, (int)ptrs.size(), &ptrs.front()));
        ASSERT_EQ(0, bargptrs.size());
        s_client = new IModelBankClient(imbArgs, clientInfo);
        }
    else if (getenv("MSTN_BRIDGE_TESTS_IMODEL_HUB"))
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("MstnBridgeTests")->trace("Using IModelHubClient");
        iModelBridgeFwk::IModelHubArgs args;
        s_client = new IModelHubClient(args, clientInfo);
        }
    else
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("MstnBridgeTests")->trace("Using TestIModelHubClientForBridges");
        s_client = new TestIModelHubClientForBridges(GetOutputDir());
        }

    iModelBridgeFwk::SetIModelClientForBridgesForTesting(*s_client);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::StopImodelBankServer()
    {
    if (!UsingIModelBank())
        return;

    if (!s_iModelBankProcess.hProcess)
        return;

#ifdef EXPERIMENT_SEND_KILL_SIGNAL
    // This does indeed send a nice, clean kill signal to imodel-bank, but it also seems to mess up the console in which I run the tests.
    SetConsoleCtrlHandler(nullptr, true);                       // This process should ignore the Ctrl-C event that I am about to send to my console.
    GenerateConsoleCtrlEvent (CTRL_C_EVENT, 0);                 // Send all (other) processes that share my console a Ctrl-C event
    WaitForSingleObject (s_iModelBankProcess.hProcess, 5000);   // Wait for the subprocess to end
    SetConsoleCtrlHandler(nullptr, false);                      // This process can go back to handling Ctrl-C events
#else
    auto bankClient = dynamic_cast<BentleyApi::Dgn::IModelBankClient*>(&GetClient());
    ASSERT_EQ(BSISUCCESS, bankClient->Shutdown());

    WaitForSingleObject (s_iModelBankProcess.hProcess, 5000);
#endif
    DWORD dwExitCode = 0;
    GetExitCodeProcess (s_iModelBankProcess.hProcess, &dwExitCode);
    // EXPECT_EQ(0, dwExitCode); -- usually I get 259, which is STILL_ACTIVE, meaning that the process has not in fact ended ?!?
    CloseHandle(s_iModelBankProcess.hProcess);
    CloseHandle(s_iModelBankProcess.hThread);
    ZeroMemory(&s_iModelBankProcess, sizeof(s_iModelBankProcess));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::StartImodelBankServer(BentleyApi::BeFileNameCR imodelDir)
    {
    // Run the imodel-bank server, telling it the imodel directory
    BentleyApi::BeFileName runbatfile = GetTestDataDir();
    runbatfile.AppendToPath(L"runImodelBankServer.bat");
    WString runbatcmd = runbatfile;
    runbatcmd.append(L" ").append(imodelDir.c_str());

    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&s_iModelBankProcess, sizeof(s_iModelBankProcess));
    ASSERT_TRUE(::CreateProcessW(nullptr, (LPWSTR)runbatcmd.c_str(), nullptr, nullptr, false, NORMAL_PRIORITY_CLASS, nullptr, nullptr, &si, &s_iModelBankProcess));
    
    DWORD sleepinterval = 1;
    for (int i=0; i<10; ++i)
        {
        if (GetClient().GetIModelInfo().IsValid()) // The real purpose of calling GetIModelInfo is to make the test wait until the bank server is up and responding.
            break;
        ::Sleep(sleepinterval);
        sleepinterval *= 2;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName MstnBridgeTestsFixture::CreateImodelBankRepository(BentleyApi::BeFileNameCR seedFile)
    {
    // Copy the entire imodelfs directory to output
    BentleyApi::BeFileName inImodelFs = GetTestDataDir();
    inImodelFs.AppendToPath(L"imodelfs");                                   // must match Tests/data/imodelfs and imodel.json
    BentleyApi::BeFileName outImodelFs = GetOutputDir();
    outImodelFs.AppendToPath(L"imodelfs");
    BentleyApi::BeFileName::EmptyAndRemoveDirectory(outImodelFs.c_str());
    //        BentleyApi::BeFileName::CreateNewDirectory(outImodelFs.c_str());  -- Clone creates the dir
    BentleyApi::BeFileName::CloneDirectory(inImodelFs.c_str(), outImodelFs.c_str());

    BentleyApi::BeFileName outImodelDir = outImodelFs;
    outImodelDir.AppendToPath(L"233e1f55-561d-42a4-8e80-d6f91743863e");     // must match Tests/data/imodelfs and imodel.json

    // Copy the seed file into the imodelfs
    BentleyApi::BeFileName outSeedFile = outImodelDir;
    outSeedFile.AppendToPath(L"seed.bim");                                  //      "                   "
    outSeedFile.BeDeleteFile();
    BentleyApi::BeFileName::BeCopyFile(seedFile.c_str(), outSeedFile.c_str());

    return outImodelDir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::CreateRepository(Utf8CP repoName)
    {
    if (nullptr == repoName)
        repoName = DEFAULT_IMODEL_NAME_A;

    auto seedFile = GetSeedFile(); // tricky - GetSeedFile initializes and terminates the host. 

    if (UsingIModelBank())                                                          // iModelBank server does not create the repo. The repo must be set up before starting imb.
        {
        StopImodelBankServer();
        auto iModelDir = CreateImodelBankRepository(seedFile);
        StartImodelBankServer(iModelDir);
        return;
        }

    auto hubClient = dynamic_cast<IModelHubClientForBridges*>(&GetClient());        // mock or real iModelHubClient
    if (nullptr != hubClient)
        {
        ScopedDgnHost host;
        hubClient->DeleteRepository();
        ASSERT_EQ(BSISUCCESS, hubClient->CreateRepository(repoName, seedFile));
        return;
        }

    BeAssert(false && "unknown type of IModelClientForBridges");
    FAIL() << "unknown type of IModelClientForBridges";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::RunTheBridge(BentleyApi::bvector<BentleyApi::WString> const& args)
    {
    iModelBridgeFwk fwk;
    bvector<WCharCP> argptrs;
    
    MAKE_ARGC_ARGV(argptrs, args);

    ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));

    ASSERT_EQ(0, fwk.Run(argc, argv));

    m_briefcaseName = fwk.GetBriefcaseName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::SetupTestDirectory(BentleyApi::BeFileNameR testDir, BentleyApi::WCharCP dirName, BentleyApi::WCharCP iModelName,
                                                BentleyApi::BeFileNameCR inputFile, BentleyApi::BeSQLite::BeGuidCR inputGuid,
                                                BentleyApi::BeFileNameCR refFile, BentleyApi::BeSQLite::BeGuidCR refGuid)
    {
    testDir = GetOutputDir();
    if (!testDir.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(iModelName);
    assignDbName.AppendExtension(L"fwk-registry.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();

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

    iModelBridgeDocumentProperties docProps1(inputGuid.ToString().c_str(), "wurn1", "durn1", "other1", "");
    iModelBridgeDocumentProperties refDocProps(refGuid.ToString().c_str(), "wurn2", "durn2", "other2", "");
    testRegistry.SetDocumentProperties(docProps1, inputFile);
    testRegistry.SetDocumentProperties(refDocProps, refFile);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, inputFile, L"");
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, refFile, L"");
    testRegistry.Save();
    //We need to shut down v8host at the end so that rest of the processing works.
    TerminateHost();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BentleyStatus MstnBridgeTestsFixture::DbFileInfo::GetiModelElementByDgnElementId(BentleyApi::Dgn::DgnElementId& elementId, int64_t srcElementId)
    {
    BentleyApi::BeSQLite::EC::ECSqlStatement estmt;
    estmt.Prepare(*m_db, "SELECT xsa.Element.Id FROM "
                  BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " AS g,"
                  BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " AS xsa"
                  " WHERE (xsa.Element.Id=g.ECInstanceId) AND (xsa.Identifier = ?)");
    estmt.BindText(1, Utf8PrintfString("%lld", srcElementId).c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
    if (BE_SQLITE_ROW != estmt.Step())
        return BentleyApi::BentleyStatus::BSIERROR;
    elementId = estmt.GetValueId<DgnElementId>(0);
    return BentleyApi::BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t SynchInfoTests::AddModel (BentleyApi::BeFileName& inputFile, BentleyApi::Utf8StringCR modelName)
{
    Bentley::WString wModelName (modelName.c_str ());
    int64_t modelId = 0;
    DgnV8Api::ModelId modelid;
    ScopedDgnv8Host testHost;
    bool adoptHost = NULL == DgnV8Api::DgnPlatformLib::QueryHost ();
    if (adoptHost)
        testHost.Init ();
    {
        V8FileEditor v8editor;
        v8editor.Open (inputFile);
        v8editor.AddModel (modelid, wModelName.c_str ());
    }
    modelId = modelid;
    return modelId;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t SynchInfoTests::AddNamedView (BentleyApi::BeFileName& inputFile, BentleyApi::Utf8StringCR viewName)
{
    Bentley::WString wViewName (viewName.c_str ());
    int64_t viewElementId = 0;
    DgnV8Api::ElementId elementId;
    ScopedDgnv8Host testHost;
    bool adoptHost = NULL == DgnV8Api::DgnPlatformLib::QueryHost ();
    if (adoptHost)
        testHost.Init ();
    {
        V8FileEditor v8editor;
        v8editor.Open (inputFile);
        v8editor.AddView (elementId, wViewName.c_str ());
    }
    viewElementId = elementId;
    return viewElementId;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t SynchInfoTests::AddLevel (BentleyApi::BeFileName& inputFile, BentleyApi::Utf8StringCR levelName)
{
    Bentley::WString wLavelName (levelName.c_str ());
    ScopedDgnv8Host testHost;
    bool adoptHost = NULL == DgnV8Api::DgnPlatformLib::QueryHost ();
    int64_t levelid;
    if (adoptHost)
        testHost.Init ();
    {
        DgnV8Api::LevelId id;
        V8FileEditor v8editor;
        v8editor.Open (inputFile);
        v8editor.AddLevel (id, wLavelName);
        levelid = id;
    }
    return levelid;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SynchInfoTests::ValidateNamedViewSynchInfo (BentleyApi::BeFileName& dbFile, int64_t srcId)
{
    DbFileInfo info (dbFile);

    BentleyApi::BeSQLite::EC::ECSqlStatement estmt;
    estmt.Prepare(*info.m_db, "SELECT kind, Identifier, xsa.JsonProperties FROM "
                  BIS_SCHEMA (BIS_CLASS_ViewDefinition) " AS v,"
                  BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " AS xsa"
        " WHERE (xsa.Element.Id=v.ECInstanceId) AND (xsa.Identifier = ?)");
    estmt.BindText(1, Utf8PrintfString("%lld", srcId).c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);

    ASSERT_TRUE (BentleyApi::BeSQLite::BE_SQLITE_ROW == estmt.Step ());
    BentleyApi::Utf8String kind, properties, viewName;
    int64_t id;
    rapidjson::Document json;

    kind = estmt.GetValueText (0);
    ASSERT_TRUE (kind.Equals ("ViewDefinition"));

    id = estmt.GetValueId<int64_t> (1);
    ASSERT_TRUE (id == srcId);

    properties = estmt.GetValueText (2);
    json.Parse (properties.c_str ());
    viewName = json["v8ViewName"].GetString ();
    ASSERT_TRUE (viewName.Equals ("TestView"));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SynchInfoTests::ValidateLevelSynchInfo (BentleyApi::BeFileName& dbFile, int64_t srcId)
{
    DbFileInfo info (dbFile);

    BentleyApi::BeSQLite::EC::ECSqlStatement estmt;
    estmt.Prepare(*info.m_db, "SELECT kind, Identifier, JsonProperties FROM "
                  BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " AS xsa WHERE (xsa.Identifier = ?)");
    estmt.BindText(1, Utf8PrintfString("%lld", srcId).c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);

    ASSERT_TRUE (BentleyApi::BeSQLite::BE_SQLITE_ROW == estmt.Step ());
    BentleyApi::Utf8String kind, properties, levelName;
    int64_t id;
    rapidjson::Document json;

    kind = estmt.GetValueText (0);
    ASSERT_TRUE (kind.Equals ("Level"));

    id = estmt.GetValueId<int64_t> (1);
    ASSERT_TRUE (id == srcId);

    properties = estmt.GetValueText (2);
    json.Parse (properties.c_str ());
    levelName = json["v8LevelName"].GetString ();
    ASSERT_TRUE (levelName.Equals ("TestLevel"));

    BentleyApi::BeSQLite::EC::ECSqlStatement estmt1;
    estmt1.Prepare (*info.m_db, "SELECT * FROM "
        BIS_SCHEMA (BIS_CLASS_Category) " AS c WHERE (c.CodeValue = ?)");

    estmt1.BindText (1, levelName.c_str (), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    ASSERT_TRUE (BentleyApi::BeSQLite::BE_SQLITE_ROW == estmt1.Step ());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SynchInfoTests::ValidateModelSynchInfo (BentleyApi::BeFileName& dbFile, int64_t srcId)
{
    DbFileInfo info (dbFile);

    BentleyApi::BeSQLite::EC::ECSqlStatement estmt;
    estmt.Prepare (*info.m_db, "SELECT kind, xsa.Identifier, xsa.JsonProperties FROM "
        BIS_SCHEMA (BIS_CLASS_Model) " AS m,"
        BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " AS xsa"
        " WHERE (xsa.Element.Id=m.ModeledElement.Id) AND (xsa.Identifier = ?)");
    estmt.BindText(1, Utf8PrintfString("%lld", srcId).c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);

    ASSERT_TRUE (BentleyApi::BeSQLite::BE_SQLITE_ROW == estmt.Step ());
    BentleyApi::Utf8String kind, properties, modelName;
    int64_t id;
    rapidjson::Document json;

    kind = estmt.GetValueText (0);
    ASSERT_TRUE (kind.Equals ("Model"));

    id = estmt.GetValueId<int64_t> (1);
    ASSERT_TRUE (id == srcId);

    properties = estmt.GetValueText (2);
    json.Parse (properties.c_str ());
    modelName = json["v8ModelName"].GetString ();
    ASSERT_TRUE (modelName.Equals ("TestModel"));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SynchInfoTests::ValidateElementSynchInfo (BentleyApi::BeFileName& dbFile, int64_t srcId)
{
    DbFileInfo info (dbFile);

    BentleyApi::BeSQLite::EC::ECSqlStatement estmt;
    estmt.Prepare (*info.m_db, "SELECT kind,Identifier FROM "
        BIS_SCHEMA (BIS_CLASS_GeometricElement3d) " AS g,"
        BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " AS xsa"
        " WHERE (xsa.Element.Id=g.ECInstanceId) AND (xsa.Identifier = ?)");
    estmt.BindText(1, Utf8PrintfString("%lld", srcId).c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);

    ASSERT_TRUE (BentleyApi::BeSQLite::BE_SQLITE_ROW == estmt.Step ());
    BentleyApi::Utf8String kind, properties, modelName;
    int64_t id;

    kind = estmt.GetValueText (0);
    ASSERT_TRUE (kind.Equals ("Element"));

    id = estmt.GetValueId<int64_t> (1);
    ASSERT_TRUE (id == srcId);
}

