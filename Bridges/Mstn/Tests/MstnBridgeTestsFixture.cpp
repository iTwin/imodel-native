/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/MstnBridgeTestsFixture.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
    //testDir.AppendToPath(L"iModelBridgeTests");
    //testDir.AppendToPath(L"Dgnv8Bridge");
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
void MstnBridgeTestsFixture::SetUpBridgeProcessingArgs(BentleyApi::bvector<BentleyApi::WString>& args, WCharCP stagingDir, WCharCP bridgeRegSubkey, bool setCredentials, WCharCP iModelName)
    {
    args.push_back(L"iModelBridgeTests.ConvertLinesUsingBridgeFwk");                                                 // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-environment=Qa");
    if (NULL == iModelName)
        args.push_back(L"--server-repository=iModelBridgeTests_Test1");                             // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    else
        args.push_back(BentleyApi::WPrintfString(L"--server-repository=%s", iModelName).c_str());
    args.push_back(L"--server-project-guid=iModelBridgeTests_Project");                         // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--fwk-revision-comment=\"comment in quotes\"");
    if (setCredentials)
        {
        args.push_back(L"--server-user=username");                                         // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
        args.push_back(L"--server-password=\"password><!@\"");                                      // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
        }
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
int64_t MstnBridgeTestsFixture::AddLine(BentleyApi::BeFileName& inputFile, int num)
    {
    int64_t elementId = 0;
    ScopedDgnv8Host testHost;
    bool adoptHost = NULL == DgnV8Api::DgnPlatformLib::QueryHost();
    if (adoptHost)
        testHost.Init();
    {
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
    Utf8String ecsql("SELECT Element.Id FROM " XTRN_SRC_ASPCT_FULLCLASSNAME " WHERE ( (Scope.Id=?) AND (Kind ='DocumentWithBeGuid') AND (Identifier= ?))");
    
    auto stmt = m_db->GetPreparedECSqlStatement(ecsql.c_str());

    if (!stmt.IsValid())
        return 0;

    stmt->BindId(1, m_db->Elements().GetRootSubjectId());
    BentleyApi::Utf8String guidString = fileGuid.ToString();
    stmt->BindText(2, guidString.c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
    
    if (BE_SQLITE_ROW != stmt->Step())
        return 0;
    
    DgnElementId repoLink = stmt->GetValueId<DgnElementId>(0);

    Utf8String modelSQL("SELECT count(*) FROM " XTRN_SRC_ASPCT_FULLCLASSNAME " WHERE ( (Scope.Id=?) AND (Kind = 'Model') )");

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
void MstnBridgeTestsFixture::SetupTestDirectory(BentleyApi::BeFileNameR testDir, BentleyApi::WCharCP dirName, BentleyApi::WCharCP iModelName,
                                                BentleyApi::BeFileNameCR inputFile, BentleyApi::BeSQLite::BeGuidCR inputGuid,
                                                BentleyApi::BeFileNameCR refFile, BentleyApi::BeSQLite::BeGuidCR refGuid)
    {
    testDir = getiModelBridgeTestsOutputDir(dirName);

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
    estmt.Prepare(*m_db, "SELECT sourceInfo.Element.Id FROM "
                  BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " AS g,"
                  XTRN_SRC_ASPCT_FULLCLASSNAME " AS sourceInfo"
                  " WHERE (sourceInfo.Element.Id=g.ECInstanceId) AND (sourceInfo.Identifier = ?)");
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
    estmt.Prepare(*info.m_db, "SELECT kind, Identifier, sourceInfo.JsonProperties FROM "
                  BIS_SCHEMA (BIS_CLASS_ViewDefinition) " AS v,"
        XTRN_SRC_ASPCT_FULLCLASSNAME " AS sourceInfo"
        " WHERE (sourceInfo.Element.Id=v.ECInstanceId) AND (sourceInfo.Identifier = ?)");
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
                  XTRN_SRC_ASPCT_FULLCLASSNAME " AS sourceInfo WHERE (sourceInfo.Identifier = ?)");
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
    estmt.Prepare (*info.m_db, "SELECT kind, sourceInfo.Identifier, sourceInfo.JsonProperties FROM "
        BIS_SCHEMA (BIS_CLASS_Model) " AS m,"
        XTRN_SRC_ASPCT_FULLCLASSNAME " AS sourceInfo"
        " WHERE (sourceInfo.Element.Id=m.ModeledElement.Id) AND (sourceInfo.Identifier = ?)");
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
        XTRN_SRC_ASPCT_FULLCLASSNAME " AS sourceInfo"
        " WHERE (sourceInfo.Element.Id=g.ECInstanceId) AND (sourceInfo.Identifier = ?)");
    estmt.BindText(1, Utf8PrintfString("%lld", srcId).c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);

    ASSERT_TRUE (BentleyApi::BeSQLite::BE_SQLITE_ROW == estmt.Step ());
    BentleyApi::Utf8String kind, properties, modelName;
    int64_t id;

    kind = estmt.GetValueText (0);
    ASSERT_TRUE (kind.Equals ("Element"));

    id = estmt.GetValueId<int64_t> (1);
    ASSERT_TRUE (id == srcId);
}

