/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/MstnBridgeTestsFixture.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include "MstnBridgeTestsFixture.h"
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName MstnBridgeTestsFixture::GetOutputDir()
    {
    BentleyApi::BeFileName testDir;
    BentleyApi::BeTest::GetHost().GetOutputRoot(testDir);
    testDir.AppendToPath(L"iModelBridgeTests");
    testDir.AppendToPath(L"Dgnv8Bridge");
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
    fileName.AppendToPath(L"Dgnv8BridgeB02.dll");
    return fileName;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName MstnBridgeTestsFixture::GetSeedFile()
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::SetUpTestCase()
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName MstnBridgeTestsFixture::getiModelBridgeTestsOutputDir(WCharCP subdir)
    {
    BentleyApi::BeFileName testDir = GetOutputDir();
    testDir.AppendToPath(subdir);
    return testDir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::MakeCopyOfFile(BentleyApi::BeFileNameR outFile, BentleyApi::WCharCP filename, BentleyApi::WCharCP suffix)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::SetUpBridgeProcessingArgs(BentleyApi::bvector<BentleyApi::WString>& args, WCharCP stagingDir, WCharCP bridgeRegSubkey)
    {
    args.push_back(L"iModelBridgeTests.ConvertLinesUsingBridgeFwk");                                                 // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-environment=Qa");
    args.push_back(L"--server-repository=iModelBridgeTests_Test1");                             // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-project-guid=iModelBridgeTests_Project");                         // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--fwk-revision-comment=\"comment in quotes\"");
    args.push_back(L"--server-user=username=username");                                         // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-password=\"password><!@\"");                                      // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
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
    {
    V8FileEditor v8editor;
    v8editor.Open(inputFile);
    int offset = useOffsetForElement ? num : 0;
    v8editor.AddAttachment(refV8File, nullptr, Bentley::DPoint3d::FromXYZ((double) offset * 1000, (double) offset * 1000, 0));
    v8editor.Save();
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t MstnBridgeTestsFixture::AddLine(BentleyApi::BeFileName& inputFile)
    {
    int64_t elementId = 0;
    ScopedDgnv8Host testHost;
    bool adoptHost = NULL == DgnV8Api::DgnPlatformLib::QueryHost();
    if (adoptHost)
        testHost.Init();
    {
    V8FileEditor v8editor;
    v8editor.Open(inputFile);
    DgnV8Api::ElementId eid1;
    v8editor.AddLine(&eid1);
    elementId = eid1;
    v8editor.Save();
    }
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
    CachedStatementPtr stmt = m_db->Elements().GetStatement("SELECT count(*) FROM " DGN_TABLE_ProvenanceModel " WHERE  V8FileId = ?");
    BentleyApi::Utf8String guidString = fileGuid.ToString();
    stmt->BindText(1, guidString.c_str(), Statement::MakeCopy::No);
    stmt->Step();
    return  stmt->GetValueInt(0);
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
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::RunTheBridge(BentleyApi::bvector<BentleyApi::WString> const& args)
    {
    iModelBridgeFwk fwk;
    bvector<WCharCP> argptrs;
    
    MAKE_ARGC_ARGV(argptrs, args);

    ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));

    ASSERT_EQ(0, fwk.Run(argc, argv));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void MstnBridgeTestsFixture::SetupTestDirectory(BentleyApi::BeFileNameR testDir,  BentleyApi::WCharCP dirName,
                                                BentleyApi::BeFileNameCR inputFile, BentleyApi::BeSQLite::BeGuidCR inputGuid,
                                                BentleyApi::BeFileNameCR refFile, BentleyApi::BeSQLite::BeGuidCR refGuid)
    {
    testDir = getiModelBridgeTestsOutputDir(dirName);

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));

    BentleyApi::BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"iModelBridgeTests_Test1.fwk-registry.db");
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
    estmt.Prepare(*m_db, "SELECT o.ECInstanceId FROM "
                  BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " AS g,"
                  SOURCEINFO_ECSCHEMA_NAME "." SOURCEINFO_CLASS_SoureElementInfo " AS o"
                  " WHERE (o.ECInstanceId=g.ECInstanceId) AND (o.SourceId = '?')");
    BentleyApi::Utf8PrintfString srcElementIdStr("%lld", srcElementId);
    estmt.BindText(1, srcElementIdStr.c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_ROW != estmt.Step())
        return BentleyApi::BentleyStatus::BSIERROR;
    elementId = estmt.GetValueId<DgnElementId>(0);
    return BentleyApi::BentleyStatus::BSISUCCESS;
    }