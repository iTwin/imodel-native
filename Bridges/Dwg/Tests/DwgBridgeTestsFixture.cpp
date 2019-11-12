/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DwgBridgeTestsFixture.h"
#include "Tests.h"
#include <tlhelp32.h>
#include <iModelBridge/TestIModelHubClientForBridges.h>
#include <iModelBridge/iModelBridgeFwk.h>
#include <iModelBridge/FakeRegistry.h>
#include <BeHttp/HttpClient.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#include <Bentley/Base64Utilities.h>
#include <Bentley/Desktop/FileSystem.h>

DwgBridgeTestsLogProvider DwgBridgeTestsFixture::s_logProvider;
WString DwgBridgeTestsFixture::s_dwgbridgeRegSubKey;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP DwgBridgeTestsFixture::GetDwgBridgeRegSubKey ()
    {
    if (s_dwgbridgeRegSubKey.empty())
        {
        auto dwgbridgeDll = ::LoadLibrary(DwgBridgeTestsFixture::GetDwgBridgeDllName().c_str());
        if (dwgbridgeDll != nullptr)
            {
            auto getRegistryKeyFunc = (bool(*)(wchar_t*,const size_t))::GetProcAddress (dwgbridgeDll, "?DwgBridge_getBridgeRegistryKey@@YA_NPEA_W_K@Z");
            if (getRegistryKeyFunc != nullptr)
                {
                WChar wstr[128] = {0};
                if ((*getRegistryKeyFunc)(wstr, sizeof(wstr)))
                    s_dwgbridgeRegSubKey.assign(wstr);
                else
                    BeAssert(false && "DwgBridge_getBridgeRegistryKey cannot find the registry sub-key for DwgBridge!");
                }
            }
        else
            {
            BeAssert(false && "Function DwgBridge_getBridgeRegistryKey is not found in DwgBridgeM02.dll!");
            }
        }
    return s_dwgbridgeRegSubKey.empty() ? L"RealDwgBridge" : s_dwgbridgeRegSubKey.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName DwgBridgeTestsFixture::GetOutputDir()
    {
    BentleyApi::BeFileName testDir;
    BentleyApi::BeTest::GetHost().GetOutputRoot(testDir);
    return testDir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName  DwgBridgeTestsFixture::GetOutputFileName(BentleyApi::WCharCP filename)
    {
    BentleyApi::BeFileName filepath = GetOutputDir();
    filepath.AppendToPath(filename);
    return filepath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName DwgBridgeTestsFixture::GetDwgBridgeDllName()
    {
    auto fileName = BentleyApi::Desktop::FileSystem::GetExecutableDir();
    fileName.AppendToPath(L"DwgBridgeM02.dll");
    return fileName;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName DwgBridgeTestsFixture::GetSeedFile()
    {
    BentleyApi::Dgn::ScopedDgnHost host;

    //Initialize parameters needed to create a DgnDb
    BentleyApi::BeSQLite::BeGuid guid;
    guid.FromString("f70498c6-a7a7-44ce-976e-1e5f63d1f9c4");    // Set the DbGuid, which is required by imodel-bank server
    BentleyApi::Dgn::CreateDgnDbParams createProjectParams(guid);
    createProjectParams.SetRootSubjectName("iModelBridgeTests");

    BentleyApi::BeFileName seedDbName = GetSeedFilePath();
    BeFileName::CreateNewDirectory(seedDbName.GetDirectoryName().c_str());

    BeFileName::BeDeleteFile(seedDbName.c_str());

    // Create the seed DgnDb file. The BisCore domain schema is also imported. 
    BentleyApi::BeSQLite::DbResult createStatus;
    auto db = DgnDb::CreateDgnDb(&createStatus, seedDbName, createProjectParams);

    db->SaveChanges();
    return seedDbName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridgeTestsFixture::SetUpTestCase()
    {
    BentleyApi::NativeLogging::LoggingConfig::ActivateProvider(&s_logProvider);

    BentleyApi::BeFileName tmpDir;
    BentleyApi::BeTest::GetHost().GetTempDir(tmpDir);
    BentleyApi::BeFileName::CreateNewDirectory(tmpDir.c_str());
    BentleyApi::BeFileName::CreateNewDirectory(GetOutputDir());

    BentleyApi::BeFileName platformAssetsDir;
    BentleyApi::BeTest::GetHost().GetDgnPlatformAssetsDirectory(platformAssetsDir);
    BentleyApi::BeFileName sqLangFile(platformAssetsDir);
    sqLangFile.AppendToPath(L"sqlang\\DwgBridgeTests_en-US.sqlang.db3");
    L10N::Initialize(BentleyApi::BeSQLite::L10N::SqlangFiles(sqLangFile));
    ScopedDgnHost host; // This statically initializes SQLite
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridgeTestsFixture::TearDownTestCase()
    {
    BentleyApi::BeFileName::EmptyAndRemoveDirectory(GetOutputDir());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName DwgBridgeTestsFixture::GetTestDataDir()
    {
    BentleyApi::BeFileName dir;
    BentleyApi::BeTest::GetHost().GetDocumentsRoot(dir);
    dir.AppendToPath(L"TestData");
    return dir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridgeTestsFixture::MakeCopyOfFile(BentleyApi::BeFileNameR outFile, BentleyApi::WCharCP filename, BentleyApi::WCharCP suffix)
    {
    BentleyApi::BeFileName filepath = GetTestDataFileName(filename);

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
void DwgBridgeTestsFixture::SetUpBridgeProcessingArgs(BentleyApi::bvector<BentleyApi::WString>& args, WCharCP stagingDir, WCharCP bridgeRegSubkey, WCharCP iModelName)
    {
    auto useBank = GetIModelBankServerJs();
    auto rspFileName = useBank? L"imodel-bank.rsp": L"imodel-hub.rsp";

    FwkArgvMaker argvMaker;
    argvMaker.SetUpBridgeProcessingArgs(stagingDir, bridgeRegSubkey, GetDwgBridgeDllName(), iModelName, useBank, rspFileName);

    auto argv = argvMaker.GetArgV();
    int argc = argvMaker.GetArgC();
    for (int i = 0; i < argc; ++i)
        args.push_back(argv[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridgeTestsFixture::AddAttachment(BentleyApi::BeFileName& inputFile, BentleyApi::BeFileNameR refFile, int32_t num, bool useOffsetForElement)
    {
    BentleyApi::Dgn::ScopedDgnHost dgnHost;
    ScopedDwgHost dwgHost;
    DwgFileEditor editor(inputFile);
    int offset = useOffsetForElement ? num : 0;
    editor.AttachXrefInDefaultModel(refFile, BentleyApi::DPoint3d::FromXYZ((double)offset, (double)offset, 0));
    editor.SaveFile();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t DwgBridgeTestsFixture::AddLine(BentleyApi::BeFileName& inputFile, int num)
    {
    BentleyApi::Dgn::ScopedDgnHost dgnHost;
    ScopedDwgHost dwgHost;
    uint64_t    elementId = 0;
    DPoint3d    point1 = DPoint3d::FromZero();
    DPoint3d    point2 = DPoint3d::FromZero();
    DwgFileEditor editor(inputFile);

    for (int index = 0; index < num; ++index)
        {
        point1.y += 0.5;
        point2.y = point1.y;
        editor.AddLineInDefaultModel(point1, point2);
        if (index == 0)
            elementId = editor.GetCurrentObjectId().ToUInt64();
        }
    editor.SaveFile();
    
    return elementId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::Dgn::DgnElementId DwgBridgeTestsFixture::DbFileInfo::GetRepositoryLinkByFileNameLike(BentleyApi::Utf8StringCR fn)
    {
    BentleyApi::BeSQLite::EC::ECSqlStatement stmt;
    EXPECT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(*m_db, "select Element.Id from " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " where (kind='DocumentWithBeGuid' AND identifier LIKE ?)"));
    stmt.BindText(1, fn.c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    if (stmt.Step() != BentleyApi::BeSQLite::BE_SQLITE_ROW)
        return BentleyApi::Dgn::DgnElementId();
    return stmt.GetValueId<BentleyApi::Dgn::DgnElementId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridgeTestsFixture::DbFileInfo::ClearRepositoryAdmin()
    {
    m_host.SetRepositoryAdmin(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridgeTestsFixture::DbFileInfo::SetRepositoryAdminFromBriefcaseClient(IModelClientForBridges& client)
    {
    m_host.SetRepositoryAdmin(new BriefClientRepositoryAdmin(client));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t DwgBridgeTestsFixture::DbFileInfo::GetElementCount()
    {
    CachedStatementPtr stmt = m_db->Elements().GetStatement("SELECT count(*) FROM " BIS_TABLE(BIS_CLASS_Element) "");
    stmt->Step();
    return stmt->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DwgBridgeTestsFixture::DbFileInfo::DbFileInfo(BentleyApi::BeFileNameCR fileName)
    {
    
    BentleyApi::BeSQLite::DbResult result;
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(BentleyApi::BeSQLite::Db::OpenMode::Readonly));
    BeAssert(BentleyApi::BeSQLite::DbResult::BE_SQLITE_OK ==  result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DwgBridgeTestsFixture::DbFileInfo::~DbFileInfo()
    {
    m_db = nullptr; // TRICKY do this first, before we allow ScopedDgnHost to be destructed. The DgnDb destrurctor releases the last ref to the briefcasemgr, and that dtor will access the host.
    ClearRepositoryAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t DwgBridgeTestsFixture::DbFileInfo::GetModelCount ()
    {
    CachedStatementPtr stmt = m_db->Elements().GetStatement("SELECT count(*) FROM " BIS_TABLE(BIS_CLASS_Model) "");
    stmt->Step();
    return stmt->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t DwgBridgeTestsFixture::DbFileInfo::GetPhysicalModelCount()
    {
    ModelIterator iterator = m_db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalModel));
    return iterator.BuildIdSet().size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t DwgBridgeTestsFixture::DbFileInfo::GetBISClassCount(CharCP className)
    {
    CachedStatementPtr stmt = m_db->Elements().GetStatement(Utf8PrintfString("SELECT count(*) FROM %s ", className).c_str());
    stmt->Step();
    return stmt->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t DwgBridgeTestsFixture::DbFileInfo::GetModelProvenanceCount(BentleyApi::BeSQLite::BeGuidCR fileGuid)
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
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName DwgBridgeTestsFixture::CreateTestDir(WCharCP testDir)
    {
    // Start with a COMPLETELY CLEAN SLATE
    BentleyApi::BeFileName::EmptyAndRemoveDirectory (GetOutputDir().c_str());
    EXPECT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::CreateNewDirectory(GetOutputDir().c_str()));

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
size_t DwgBridgeTestsFixture::GetChangesetCount()
    {
    auto testClient = GetClientAsMock();
    if (nullptr != testClient)
        return testClient->GetDgnRevisions().size();

    BentleyApi::bvector<BentleyApi::iModel::Hub::ChangeSetInfoPtr> changesets;
    getInfoForAllChangeSets(changesets, GetClientAsIModelClientBase());
    return changesets.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
RevisionStats DwgBridgeTestsFixture::ComputeRevisionStats(BentleyApi::Dgn::DgnDbR db, size_t start, size_t end)
    {
    RevisionStats stats{};

    auto testClient = GetClientAsMock();
    if (nullptr != testClient)
        {
        for (auto rev : testClient->GetDgnRevisions(start, end))
            {
            stats.descriptions.insert(rev->GetSummary());
#ifdef WIP_GetUserName_FOR_UNICODE
            stats.userids.insert(rev->GetUserName());
#endif
            if (rev->ContainsSchemaChanges(db))
                ++stats.nSchemaRevs;
            else
                ++stats.nDataRevs;
            }
        return stats;
        }

    BentleyApi::bvector<BentleyApi::iModel::Hub::ChangeSetInfoPtr> changesets;
    getInfoForAllChangeSets(changesets, GetClientAsIModelClientBase());

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
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::Utf8String DwgBridgeTestsFixture::ComputeAccessToken(Utf8CP uname)
    {
    // NB!                                                                     vv Must be no space between { and "
    return BentleyApi::Base64Utilities::Encode(BentleyApi::Utf8PrintfString(R"({"ForeignProjectAccessToken": {"userInfo": { "id": "%s" } } })", uname));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName DwgBridgeTestsFixture::WriteRulesFile(WCharCP fname, Utf8CP rules)
    {
    auto rulesFileName = GetOutputFileName(fname);

    rulesFileName.BeDeleteFile();

    FILE* fp = _wfopen(rulesFileName.c_str(), L"w+");
    fputs(rules, fp);
    fclose(fp);

    return rulesFileName;
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
BentleyApi::WString FwkArgvMaker::ReadRspFileFromTestData(BentleyApi::WCharCP rspFileBaseName)
    {
    BentleyApi::BeFileName rspFileName;
    BentleyApi::BeTest::GetHost().GetDocumentsRoot(rspFileName);
    rspFileName.AppendToPath(L"TestData");
    rspFileName.AppendToPath(rspFileBaseName);

    BentleyApi::WString wargs;
    if (BSISUCCESS != readEntireFile(wargs, rspFileName))
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("DwgBridgeTests")->errorv(L"%ls - response file not found\n", rspFileName.c_str());
        return L"";
        }

    return wargs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BentleyStatus FwkArgvMaker::ParseArgsFromRspFile(BentleyApi::WStringCR wargs)
    {
    BentleyApi::bvector<BentleyApi::WString> strings;
    BentleyApi::BeStringUtilities::ParseArguments(strings, wargs.c_str(), L"\n\r");

    if (strings.empty())
        return BentleyApi::BSIERROR;

    for (auto const& str : strings)
        {
        if (!str.empty())
            PushArg(str);
        }

    return BentleyApi::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
FwkArgvMaker::FwkArgvMaker()
    {
    PushArg(L"<argv0 placeholder>");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
FwkArgvMaker::~FwkArgvMaker()
    {
    Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void FwkArgvMaker::Clear()
    {
    for (auto ptr : m_ptrs)
        free((void*)ptr);
    m_ptrs.clear();
    m_bargptrs.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void FwkArgvMaker::PushArg(BentleyApi::WStringCR arg)
    {
    m_ptrs.push_back(_wcsdup(arg.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void FwkArgvMaker::SetUpBridgeProcessingArgs(WCharCP stagingDir, WCharCP bridgeRegSubkey, BentleyApi::BeFileNameCR bridgeDllName, WCharCP iModelName, bool useBank, WCharCP rspFileName)
    {
    ASSERT_EQ(BSISUCCESS, ParseArgsFromRspFile(ReadRspFileFromTestData(rspFileName)));

    if (nullptr == iModelName)
        iModelName = DEFAULT_IMODEL_NAME;

    if (useBank)
        PushArg(BentleyApi::WPrintfString(L"--imodel-bank-imodel-name=%s", iModelName).c_str()); // TRICKY: This determines the name of the briefcase (not the name of the iModel inside iModelBank).
    else
        PushArg(BentleyApi::WPrintfString(L"--server-repository=%s", iModelName).c_str());

    PushArg(L"--fwk-revision-comment=\"comment in quotes\"");

    PushArg(BentleyApi::WPrintfString(L"--fwk-bridge-library=\"%s\"", bridgeDllName.c_str()));     // must refer to a path that exists! 

    BentleyApi::BeFileName platformAssetsDir;
    BentleyApi::BeTest::GetHost().GetDgnPlatformAssetsDirectory(platformAssetsDir);
    PushArg(BentleyApi::WPrintfString(L"--fwk-bridgeAssetsDir=\"%ls\"", platformAssetsDir.c_str())); // must be a real assets dir! the platform's assets dir will serve just find as the test bridge's assets dir.

    if (stagingDir)
        PushArg(BentleyApi::WPrintfString(L"--fwk-staging-dir=\"%ls\"", stagingDir));
    if (bridgeRegSubkey)
        PushArg(BentleyApi::WPrintfString(L"--fwk-bridge-regsubkey=%ls", bridgeRegSubkey));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void FwkArgvMaker::ReplaceArgValue(BentleyApi::bvector<BentleyApi::WString>& args, WCharCP argName, WCharCP newValue)
    {
    for (size_t i = 0; i < args.size(); ++i)
        {
        auto arg = args[i];
        if (!arg.Contains(argName))
            continue;
        args[i] = WPrintfString(L"%ls=%ls", argName, newValue).c_str();
        break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void FwkArgvMaker::ReplaceArgValue(BentleyApi::bvector<BentleyApi::WCharCP>& ptrs, WCharCP argName, WCharCP newValue)
    {
    for (size_t i = 0; i < ptrs.size(); ++i)
        {
        auto arg = ptrs[i];
        if (wcsstr(arg, argName) == nullptr)
            continue;
        free((void*)arg);
        ptrs[i] = _wcsdup(WPrintfString(L"%ls=%ls", argName, newValue).c_str());
        break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void FwkArgvMaker::ReplaceArgValue(WCharCP argName, WCharCP newValue)
    {
    ReplaceArgValue(m_ptrs, argName, newValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::WebServices::ClientInfoPtr DwgBridgeTestsFixture::GetClientInfo()
    {
    // TODO: Read client info from .json file on disk;
    static Utf8CP s_productId = "1654"; // Navigator Desktop
    static Utf8CP s_appGuid = "22c1590c-4846-4909-954b-051cb7c45fe7";

    BentleyApi::BeVersion   appVersion(1, 0, 0, 0);
    Utf8String  versionString;
    if (BentleyStatus::BSISUCCESS == DwgHelper::GetImporterModuleVersion(versionString))
        appVersion.FromString (versionString.c_str());

    return BentleyApi::WebServices::ClientInfoPtr(
        new BentleyApi::WebServices::ClientInfo("Bentley-Test", appVersion, s_appGuid, "TestDeviceId", "TestSystem", s_productId, nullptr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::Dgn::IModelBankClient* DwgBridgeTestsFixture::CreateIModelBankClient(BentleyApi::Utf8StringCR accessToken)
    {
    FwkArgvMaker argvMaker;
    EXPECT_EQ(BSISUCCESS, argvMaker.ParseArgsFromRspFile(argvMaker.ReadRspFileFromTestData(L"imodel-bank.rsp")));

    iModelBridgeFwk::IModelBankArgs imbArgs;
    EXPECT_EQ(BSISUCCESS, imbArgs.ParseCommandLine(argvMaker.m_bargptrs, argvMaker.GetArgC(), argvMaker.GetArgV()));
    EXPECT_EQ(0, argvMaker.m_bargptrs.size());

    if (!accessToken.empty())
        imbArgs.m_accessToken = accessToken;

    return new IModelBankClient(imbArgs, GetClientInfo());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::Dgn::IModelHubClient* DwgBridgeTestsFixture::CreateIModelHubClient(BentleyApi::Utf8StringCR accessToken)
    {
    FwkArgvMaker argvMaker;
    EXPECT_EQ(BSISUCCESS, argvMaker.ParseArgsFromRspFile(argvMaker.ReadRspFileFromTestData(L"imodel-hub.rsp")));

    iModelBridgeFwk::IModelHubArgs imhArgs;
    EXPECT_EQ(BSISUCCESS, imhArgs.ParseCommandLine(argvMaker.m_bargptrs, argvMaker.GetArgC(), argvMaker.GetArgV()));
    EXPECT_EQ(0, argvMaker.m_bargptrs.size());

    // TODO: get userid from accessToken
    iModelBridgeError error;
    return new IModelHubClient(imhArgs, GetClientInfo(), error);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::Dgn::TestIModelHubClientForBridges* DwgBridgeTestsFixture::CreateMockClient()
    {
    return new TestIModelHubClientForBridges(GetOutputDir());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridgeTestsFixture::SetupClient(BentleyApi::Utf8StringCR accessToken)
    {
    // TODO: Consult argv and check for --imodel-bank or --imodel-hub
    auto imbPath = GetIModelBankServerJs();
    if (imbPath != nullptr)
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("DwgBridgeTests")->trace("Using IModelBankClient");
        m_client = CreateIModelBankClient(accessToken);
        }
    else if (getenv("DWGBRIDGE_TESTS_IMODEL_HUB"))
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("DwgBridgeTests")->trace("Using IModelHubClient");
        m_client = CreateIModelHubClient(accessToken);
        }
    else
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger("DwgBridgeTests")->trace("Using TestIModelHubClientForBridges");
        m_client = CreateMockClient();
        }
    
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(*m_client);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgBridgeTestsFixture::~DwgBridgeTestsFixture()
    {
    if (m_client != nullptr)
        {
        delete m_client;
        m_client = nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
ProcessRunner::ProcessRunner(BentleyApi::WStringCR cmd, int argc, wchar_t const** argv, BentleyApi::BeFileNameCR rspFile, T_SendKillSignal ks, BentleyApi::Dgn::IShutdownIModelServer* sds)
    : m_cmd(cmd), m_argc(argc), m_argv(argv), m_rspFile(rspFile), m_sendKillSignal(ks), m_shutdown(sds)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
ProcessRunner::ProcessRunner(ProcessRunner&& rhs)
    : m_cmd(std::move(rhs.m_cmd)),
    m_argc(rhs.m_argc), m_argv(rhs.m_argv),
    m_rspFile(std::move(rhs.m_rspFile)),
    m_hProcess(rhs.m_hProcess),
    m_hThread(rhs.m_hThread),
    m_pid(rhs.m_pid),
    m_exitCode(rhs.m_exitCode),
    m_sendKillSignal(rhs.m_sendKillSignal),
    m_shutdown(rhs.m_shutdown)
    {
    rhs.m_hProcess = rhs.m_hThread = 0;
    rhs.m_pid = 0;
    rhs.m_shutdown = nullptr;
    rhs.m_sendKillSignal = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
ProcessRunner::~ProcessRunner()
    {
    Stop(10*1000);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BentleyStatus ProcessRunner::Start(size_t waitMs)
    {
    BentleyApi::WString args;
    if (!m_rspFile.empty())
        {
        FILE* fp = _wfopen(m_rspFile.c_str(), L"w+");

        for (int i = 1; i < m_argc; ++i)
            fwprintf(fp, L"%ls\n", m_argv[i]);

        fclose(fp);
        args = L"@";
        args.append(m_rspFile);
        }
    else
        {
        for (int i = 1; i < m_argc; ++i)
            args.append(L" ").append(m_argv[i]);
        }

    m_cmdline.Sprintf(L"%ls %ls", m_cmd.c_str(), args.c_str());
        
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    EXPECT_TRUE(::CreateProcessW(nullptr, (LPWSTR)m_cmdline.c_str(), nullptr, nullptr, false, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi));
    
    m_hProcess = pi.hProcess;
    m_hThread = pi.hThread;
    m_pid = pi.dwProcessId;

    BentleyApi::NativeLogging::LoggingManager::GetLogger("DwgBridgeTests")->tracev(L"++++++++++++++++++ %ls\n", m_cmdline.c_str());

    ::Sleep(waitMs);

    return BentleyApi::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
int ProcessRunner::FindProcessId(BentleyApi::Utf8CP name)
    {
    HANDLE hProcessSnapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);

    PROCESSENTRY32 processEntry = { 0 };
    processEntry.dwSize = sizeof(processEntry);

    DWORD pid = 0;
    BOOL ok = ::Process32First(hProcessSnapShot, &processEntry);
    if (ok)
        {
        do  {
            if (::strstr(Utf8String(processEntry.szExeFile).c_str(), name))
                {
                pid = processEntry.th32ProcessID;
                BentleyApi::NativeLogging::LoggingManager::GetLogger("DwgBridgeTests")->tracev("%s %ld\n", processEntry.szExeFile, processEntry.th32ProcessID);
                break;
                }

            } while (::Process32Next(hProcessSnapShot, &processEntry));
        }
    ::CloseHandle(hProcessSnapShot);

    return (int)pid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BentleyStatus ProcessRunner::Stop(size_t waitMs)
    {
    if (nullptr == m_hProcess)
        return BentleyApi::BSIERROR;

    if (m_sendKillSignal != nullptr)
        m_sendKillSignal(*this);

    if (m_shutdown != nullptr)
        m_shutdown->Shutdown();

    DWORD status = WaitForSingleObject (m_hProcess, waitMs);

    DWORD dwExitCode = 0;
    GetExitCodeProcess (m_hProcess, &dwExitCode);

    // TODO: This won't work until we assert the PROCESS_TERMINATE right when we create the process
    if (WAIT_OBJECT_0 != status)
        {
        // TODO: This won't work until we assert the PROCESS_TERMINATE right when we create the process
        //    TerminateProcess(m_hProcess, 1);    
        BentleyApi::NativeLogging::LoggingManager::GetLogger("DwgBridgeTests")->warningv("%s wait = %x\n", m_cmdline.c_str(), status);
        }

    CloseHandle(m_hProcess);
    CloseHandle(m_hThread);

    m_hProcess = m_hThread = nullptr;
    m_pid = 0;

    m_exitCode = (int)dwExitCode;

    BentleyApi::NativeLogging::LoggingManager::GetLogger("DwgBridgeTests")->tracev(L"xxxxxxxxxxxxxxxxx %ls\n", m_cmdline.c_str());
    
    return (status == WAIT_OBJECT_0)? BentleyApi::BSISUCCESS: BentleyApi::BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
ProcessRunner DwgBridgeTestsFixture::StartImodelBankServer(BentleyApi::BeFileNameCR imodelDir, BentleyApi::Dgn::IModelBankClient& bankClient)
    {
    wchar_t const* argv[] = {L"", imodelDir.data()};
    ProcessRunner runner(GetTestDataFileName(L"runImodelBankServer.bat"), _countof(argv), argv, BentleyApi::BeFileName(), nullptr, &bankClient);
    runner.Start(100);

    DWORD sleepinterval = 1;
    for (int i=0; i<10; ++i)
        {
        if (bankClient.GetIModelInfo().IsValid()) // The real purpose of calling GetIModelInfo is to make the test wait until the bank server is up and responding.
            break;
        ::Sleep(sleepinterval);
        sleepinterval *= 2;
        }
    return runner;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
ProcessRunner DwgBridgeTestsFixture::StartImodelBridgeFwkExe(FwkArgvMaker const& argvMaker, size_t msWaitForStart)
    {
    BentleyApi::BeFileName rspFile = GetOutputDir();
    rspFile.AppendToPath(L"iModelBridgeFwk.rsp");

    BentleyApi::BeFileName fwkExeRel(BentleyApi::Desktop::FileSystem::GetExecutableDir());
    fwkExeRel.AppendToPath(L"..");
    fwkExeRel.AppendToPath(L"iModelBridgeFwk");
    fwkExeRel.AppendToPath(L"lib");
    fwkExeRel.AppendToPath(L"x64");
    fwkExeRel.AppendToPath(L"iModelBridgeFwk.exe");
    BentleyApi::BeFileName fwkExe;
    BentleyApi::BeFileName::FixPathName(fwkExe, fwkExeRel.c_str());

    ProcessRunner runner(fwkExe, argvMaker.GetArgC(), argvMaker.GetArgV(), rspFile, nullptr, nullptr);
    runner.Start(msWaitForStart);

    return runner;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName DwgBridgeTestsFixture::GetIModelParentDir()
    {
    BentleyApi::BeFileName outImodelFs = GetOutputDir();
    outImodelFs.AppendToPath(L"imodelfs");
    return outImodelFs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName DwgBridgeTestsFixture::GetIModelDir()
    {
    BentleyApi::BeFileName outImodelDir = GetIModelParentDir();
    outImodelDir.AppendToPath(L"f70498c6-a7a7-44ce-976e-1e5f63d1f9c4");     // must match Tests/data/imodelfs and imodel.json
    return outImodelDir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridgeTestsFixture::CreateImodelBankRepository(BentleyApi::BeFileNameCR seedFile)
    {
    // Copy the entire imodelfs directory to output
    BentleyApi::BeFileName inImodelFs = GetTestDataDir();
    inImodelFs.AppendToPath(L"imodelfs");

    BentleyApi::BeFileName outImodelFs = GetIModelParentDir();

    BentleyApi::BeFileName::EmptyAndRemoveDirectory(outImodelFs.c_str());
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::CloneDirectory(inImodelFs.c_str(), outImodelFs.c_str()));

    // Copy the seed file into the imodelfs
    BentleyApi::BeFileName outSeedFile = GetIModelDir();
    outSeedFile.AppendToPath(L"seed.bim");
    outSeedFile.BeDeleteFile();
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(seedFile.c_str(), outSeedFile.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridgeTestsFixture::CreateRepository(Utf8CP repoName)
    {
    if (nullptr == repoName)
        repoName = DEFAULT_IMODEL_NAME_A;

    auto seedFile = GetSeedFile();

    if (GetIModelBankServerJs())
        {
        CreateImodelBankRepository(seedFile);
        return;
        }

    auto hubClient = GetClientAsIModelHubClientForBridges(); // mock or real iModelHubClient
    if (nullptr != hubClient)
        {
        BentleyApi::Dgn::ScopedDgnHost host;
        hubClient->DeleteRepository();
        ASSERT_EQ(BSISUCCESS, hubClient->CreateRepository(repoName));
        return;
        }

    BeAssert(false && "unknown type of IModelClientForBridges");
    FAIL() << "unknown type of IModelClientForBridges";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
ProcessRunner DwgBridgeTestsFixture::StartServer()
    {
    BeAssert(m_client != nullptr);

    auto bankClient = GetClientAsIModelBank();
    if (bankClient != nullptr)
        return StartImodelBankServer(GetIModelDir(), *bankClient);
        
    return ProcessRunner();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridgeTestsFixture::RunTheBridge(BentleyApi::bvector<BentleyApi::WString> const& args)
    {
    iModelBridgeFwk fwk;
    bvector<WCharCP> argptrs;
    
    MAKE_ARGC_ARGV(argptrs, args);

    ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));

    ASSERT_EQ(0, fwk.Run(argc, argv));

    m_briefcaseName = fwk.GetBriefcaseName();
    }

void ProcessRunner::DoSleep(size_t ms)
    {
    ::Sleep(ms);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgBridgeTestsFixture::SetupTestDirectory(BentleyApi::BeFileNameR testDir, BentleyApi::WCharCP dirName, BentleyApi::WCharCP iModelName,
                                                BentleyApi::BeFileNameCR inputFile, BentleyApi::BeSQLite::BeGuidCR inputGuid,
                                                BentleyApi::BeFileNameCR refFile, BentleyApi::BeSQLite::BeGuidCR refGuid)
    {
    testDir = GetOutputDir();
    
    if (dirName && *dirName)
        testDir.AppendToPath(dirName);

    if (!testDir.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(iModelName);
    assignDbName.AppendExtension(L"fwk-registry.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();

    BentleyApi::WString dwgbridgeRegSubKey = GetDwgBridgeRegSubKey();
    BentleyApi::WString mstnbridgeRefSubKey = L"iModelBridgeForMstn";

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
            wcsncpy(buffer, mstnbridgeRefSubKey.c_str(), mstnbridgeRefSubKey.length());
            }
        else
            wcsncpy(buffer, dwgbridgeRegSubKey.c_str(), dwgbridgeRegSubKey.length());
        };

    testRegistry.AddBridge(dwgbridgeRegSubKey, lambda);
    testRegistry.AddBridge(mstnbridgeRefSubKey, lambda);

    iModelBridgeDocumentProperties docProps1(inputGuid.ToString().c_str(), "wurn1", "durn1", "other1", "");
    iModelBridgeDocumentProperties refDocProps(refGuid.ToString().c_str(), "wurn2", "durn2", "other2", "");
    testRegistry.SetDocumentProperties(docProps1, inputFile);
    testRegistry.SetDocumentProperties(refDocProps, refFile);
    BentleyApi::WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, inputFile, L"");
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName, refFile, L"");
    testRegistry.Save();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BentleyStatus DwgBridgeTestsFixture::DbFileInfo::GetiModelElementByDgnElementId(BentleyApi::Dgn::DgnElementId& elementId, uint64_t srcElementId)
    {
    auto idstr = DwgSourceAspects::BaseAspect::FormatHexUInt64(srcElementId);
    BentleyApi::BeSQLite::EC::ECSqlStatement estmt;
    estmt.Prepare(*m_db, "SELECT xsa.Element.Id FROM "
                  BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " AS g,"
                  BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " AS xsa"
                  " WHERE (xsa.Element.Id=g.ECInstanceId) AND (xsa.Identifier = ?)");
    estmt.BindText(1, idstr.c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
    if (BE_SQLITE_ROW != estmt.Step())
        return BentleyApi::BentleyStatus::BSIERROR;
    elementId = estmt.GetValueId<DgnElementId>(0);
    return BentleyApi::BentleyStatus::BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
void DwgBridgeTestsFixture::DbFileInfo::MustFindFileByName(RepositoryLinkId& fileid, BentleyApi::BeFileNameCR dwgFileNameIn, int expectedCount)
    {
    BentleyApi::Utf8String dwgFileName(dwgFileNameIn);

    DwgSourceAspects::RepositoryLinkAspectIterator files(*m_db);
    int count=0;
    for (auto entry : files)
        {
        if (entry.GetFileName().EqualsI(dwgFileName))
            {
            fileid = entry.GetRepositoryLinkId();
            ++count;
            }
        }
    ASSERT_EQ( expectedCount, count );
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
void DwgBridgeTestsFixture::DbFileInfo::MustFindModelByDwgModelId(DgnModelId& fmid, RepositoryLinkId ffid, uint64_t dwgHandle, int expectedCount)
    {
    auto rlink = m_db->Elements().Get<RepositoryLink>(ffid);

    DwgSourceAspects::ModelAspectIterator models(*rlink, dwgHandle);
    int count=0;
    for (auto entry : models)
        {
        fmid = entry.GetModelId();
        ++count;
        }
    ASSERT_EQ( expectedCount, count );
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
void DwgBridgeTestsFixture::DbFileInfo::MustFindElementByDwgModelId(DgnElementId& eid, DgnModelId fmid, uint64_t dwgHandle, int expectedCount)
    {
    //Find the element id from aspect.
    auto model = m_db->Models().GetModel(fmid);
    DwgSourceAspects::ObjectAspectIterator elements(*model, dwgHandle);
    int count=0;
    for (auto entry : elements)
        {
        ++count;
        eid = entry.GetElementId();
        }
    ASSERT_EQ(expectedCount, count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      04/2019
//---------------------------------------------------------------------------------------
uint64_t DwgBridgeTestsFixture::GetDefaultDwgModelId(BentleyApi::BeFileNameCR inputFile)
    {
    BentleyApi::Dgn::ScopedDgnHost dgnHost;
    ScopedDwgHost dwgHost;
    DwgFileEditor editor(inputFile);
    return editor.GetModelspaceId().ToUInt64();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t ExternalSourceAspectTests::AddModel (BentleyApi::BeFileName& inputFile, BentleyApi::Utf8StringCR modelName)
{
    BentleyApi::Dgn::ScopedDgnHost dgnHost;
    ScopedDwgHost dwgHost;
    BentleyApi::WString wModelName(modelName.c_str(), BentleyCharEncoding::Utf8);
    DwgDbObjectId paperspaceId;
    DwgFileEditor editor(inputFile);
    // add a sheet model
    editor.AddLayout (modelName, &paperspaceId);
    editor.SaveFile();
    return paperspaceId.ToUInt64();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t ExternalSourceAspectTests::AddLayer (BentleyApi::BeFileName& inputFile, BentleyApi::Utf8StringCR layerName)
{
    BentleyApi::Dgn::ScopedDgnHost dgnHost;
    ScopedDwgHost dwgHost;
    DwgString wLayerName(layerName.c_str());
    DwgFileEditor editor(inputFile);
    DwgDbObjectId id;
    editor.AddLayer (wLayerName, &id);
    editor.SaveFile();
    return id.ToUInt64();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalSourceAspectTests::ValidateLayerAspect (BentleyApi::BeFileName& dbFile, uint64_t sourceId)
{
    DwgBridgeTestsFixture::DbFileInfo info (dbFile);
    auto str = DwgSourceAspects::BaseAspect::FormatHexUInt64(sourceId);

    BentleyApi::BeSQLite::EC::ECSqlStatement estmt;
    estmt.Prepare(*info.m_db, "SELECT kind, Identifier, JsonProperties FROM "
                  BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " AS xsa WHERE (xsa.Identifier = ?)");
    estmt.BindText(1, str.c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);

    ASSERT_TRUE (BentleyApi::BeSQLite::BE_SQLITE_ROW == estmt.Step ());
    rapidjson::Document json;

    Utf8String kind(estmt.GetValueText(0));
    ASSERT_TRUE (kind.Equals ("Layer"));

    str.assign (estmt.GetValueText(1));
    uint64_t id = DwgSourceAspects::BaseAspect::ParseUInt64(str);
    ASSERT_TRUE (id == sourceId);

    Utf8String properties(estmt.GetValueText(2));
    json.Parse (properties.c_str ());

    Utf8String layerName(json["LayerName"].GetString());
    ASSERT_TRUE (layerName.Equals("TestLayer"));

    BentleyApi::BeSQLite::EC::ECSqlStatement estmt1;
    estmt1.Prepare (*info.m_db, "SELECT * FROM " BIS_SCHEMA (BIS_CLASS_Category) " AS c WHERE (c.CodeValue = ?)");

    estmt1.BindText (1, layerName.c_str (), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    ASSERT_TRUE (BentleyApi::BeSQLite::BE_SQLITE_ROW == estmt1.Step ());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalSourceAspectTests::ValidateModelAspect (BentleyApi::BeFileName& dbFile, uint64_t sourceId)
{
    DbFileInfo info (dbFile);
    auto str = DwgSourceAspects::BaseAspect::FormatHexUInt64(sourceId);

    BentleyApi::BeSQLite::EC::ECSqlStatement estmt;
    estmt.Prepare (*info.m_db, "SELECT kind, xsa.Identifier, xsa.JsonProperties FROM "
        BIS_SCHEMA (BIS_CLASS_Model) " AS m,"
        BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " AS xsa"
        " WHERE (xsa.Element.Id=m.ModeledElement.Id) AND (xsa.Identifier = ?)");
    estmt.BindText(1, str.c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);

    ASSERT_TRUE (BentleyApi::BeSQLite::BE_SQLITE_ROW == estmt.Step ());

    Utf8String kind(estmt.GetValueText(0));
    ASSERT_TRUE (kind.Equals ("Model"));

    str.assign (estmt.GetValueText(1));
    uint64_t id = DwgSourceAspects::BaseAspect::ParseUInt64(str);
    ASSERT_TRUE (id == sourceId);

    Utf8String properties(estmt.GetValueText(2));
    rapidjson::Document json;
    json.Parse (properties.c_str ());

    Utf8String modelName(json["DwgName"].GetString());
    ASSERT_TRUE (modelName.Equals("TestModel"));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalSourceAspectTests::ValidateElementAspect (BentleyApi::BeFileName& dbFile, uint64_t sourceId)
{
    DbFileInfo info (dbFile);
    auto str = DwgSourceAspects::BaseAspect::FormatHexUInt64(sourceId);

    BentleyApi::BeSQLite::EC::ECSqlStatement estmt;
    estmt.Prepare (*info.m_db, "SELECT kind,Identifier FROM "
        BIS_SCHEMA (BIS_CLASS_GeometricElement3d) " AS g,"
        BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " AS xsa"
        " WHERE (xsa.Element.Id=g.ECInstanceId) AND (xsa.Identifier = ?)");
    estmt.BindText(1, str.c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);

    ASSERT_TRUE (BentleyApi::BeSQLite::BE_SQLITE_ROW == estmt.Step ());
    BentleyApi::Utf8String kind, properties, modelName;

    kind = estmt.GetValueText (0);
    ASSERT_TRUE (kind.Equals ("Element"));

    str.assign (estmt.GetValueText(1));
    uint64_t id = DwgSourceAspects::BaseAspect::ParseUInt64(str);
    ASSERT_TRUE (id == sourceId);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::Dgn::SubjectCPtr DwgBridgeTestsFixture::DbFileInfo::GetFirstJobSubject()
    {
    EC::ECSqlStatement stmt;
    stmt.Prepare(*m_db, "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_Subject) " WHERE json_extract(JsonProperties, '$.Subject.Job') is not null");
    if (BE_SQLITE_ROW != stmt.Step())
        return nullptr;

    return m_db->Elements().Get<Subject>(stmt.GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::Dgn::DgnCodeInfoSet DwgBridgeTestsFixture::DbFileInfo::GetCodeInfos(DgnCodeSet const& codes, BentleyApi::Dgn::IModelClientForBridges& client)
    {
    BentleyApi::Http::HttpClient::Reinitialize(); // In case Unintialize was called prior to this.
    SetRepositoryAdminFromBriefcaseClient(client);
    BentleyApi::Dgn::DgnCodeInfoSet codeInfos;
    auto rc = m_db->BriefcaseManager().QueryCodeStates(codeInfos, codes);
    BeAssert(RepositoryStatus::Success == rc);
    return codeInfos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BentleyStatus DwgBridgeTestsFixture::DbFileInfo::GetCodeInfo(BentleyApi::Dgn::DgnCodeInfo& codeInfo, BentleyApi::Dgn::DgnCode const& code, BentleyApi::Dgn::IModelClientForBridges& client)
    {
    DgnCodeSet codes;
    codes.insert(code);
    auto infos = GetCodeInfos(codes, client);
    if (infos.empty())
        return BentleyApi::BSIERROR;
    BeAssert(infos.size() == 1);
    codeInfo = *infos.begin();
    return BentleyApi::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::Dgn::LockLevel DwgBridgeTestsFixture::DbFileInfo::QueryLockLevel(BentleyApi::Dgn::LockableId lockable, BentleyApi::Dgn::IModelClientForBridges& client)
    {
    BentleyApi::Http::HttpClient::Reinitialize(); // In case Unintialize was called prior to this.
    SetRepositoryAdminFromBriefcaseClient(client);
    return m_db->BriefcaseManager().QueryLockLevel(lockable);
    }
