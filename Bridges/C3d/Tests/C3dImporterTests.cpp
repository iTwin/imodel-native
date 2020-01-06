/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "C3dImporterTestsFixture.h"
#include "../../Dwg/DwgConverter/Tests/DwgFileEditor.h"

USING_NAMESPACE_BENTLEY_LINEARREFERENCING
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT

C3dImporterTestsHost    C3dImporterTests::s_testsHost;
WString     C3dImporterTests::s_c3dBridgeRegistryKey;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName C3dImporterTests::GetDgnDbFileName(BentleyApi::BeFileName& inFile)
    {
    BentleyApi::BeFileName ibimFile(inFile.GetDirectoryName(inFile.c_str()));
    ibimFile.AppendToPath(inFile.GetFileNameWithoutExtension().c_str());
    ibimFile.AppendExtension(L"bim");
    return ibimFile;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat 11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::WString C3dImporterTests::GetDataSourcePath()
    {
    BentleyApi::BeFileName filepath;
    BentleyApi::BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"TestData");
    BentleyApi::WString returnStr(filepath.c_str());
    return returnStr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName C3dImporterTests::GetInputFileName(BentleyApi::WCharCP filename)
    {
    BentleyApi::BeFileName filepath;
    BentleyApi::BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"TestData");
    filepath.AppendToPath(filename);
    return filepath;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName C3dImporterTests::GetOutputFileName(BentleyApi::WCharCP filename)
    {
    BentleyApi::BeFileName filepath = GetOutputDir();
    filepath.AppendToPath(filename);
    return filepath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void C3dImporterTests::MakeWritableCopyOf(BentleyApi::BeFileName& outFile, BentleyApi::WCharCP filename)
    {
    outFile = GetOutputFileName(filename);
    BentleyApi::BeFileName inFile = GetInputFileName(filename);
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(inFile, outFile)) << "Unable to copy file \nSource: [" << BentleyApi::Utf8String(inFile.c_str()).c_str() << "]\nDestination: [" << BentleyApi::Utf8String(outFile.c_str()).c_str() << "]";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
void C3dImporterTests::MakeWritableCopyOf(BentleyApi::BeFileName& outFile, BentleyApi::BeFileNameCR inputFileName, BentleyApi::WCharCP filename)
    {
    outFile = GetOutputFileName(filename);
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(inputFileName, outFile)) << "Unable to copy file \nSource: [" << Utf8String(inputFileName.c_str()).c_str() << "]\nDestination: [" << Utf8String(outFile.c_str()).c_str() << "]";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat 11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::WString C3dImporterTests::GetOutRoot()
    {
    BentleyApi::BeFileName outPath;
    BentleyApi::BeTest::GetHost().GetOutputRoot(outPath);
    BentleyApi::WString returnStr(outPath.c_str());
    return returnStr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName C3dImporterTests::GetOutputDir()
    {
    BentleyApi::BeFileName filepath;
    BentleyApi::BeTest::GetHost().GetOutputRoot (filepath);
    filepath.AppendToPath(L"Output");
    BentleyApi::BeFileName::CreateNewDirectory (filepath);
    return filepath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void C3dImporterTests::DeleteExistingDgnDb(BentleyApi::BeFileNameCR dgnDbFileName)
    {
    BentleyApi::BeFileName dgnDbFileNameAndRelatedFiles(dgnDbFileName);
    dgnDbFileNameAndRelatedFiles.append(L".*");
    BentleyApi::BeFileListIterator iter(dgnDbFileNameAndRelatedFiles, false);
    BentleyApi::BeFileName file;
    while (BentleyApi::SUCCESS == iter.GetNextFileName(file))
        {
        ASSERT_EQ( BentleyApi::BeFileNameStatus::Success , BentleyApi::BeFileName::BeDeleteFile(file) );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr C3dImporterTests::OpenExistingDgnDb(BentleyApi::BeFileNameCR projectName, DgnDb::OpenMode mode)
    {
    DbResult fileStatus;

    DgnDb::OpenParams openParams(mode);
    return DgnDb::OpenDgnDb(&fileStatus, projectName, openParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    C3dImporterTests::SetUpTestCase ()
    {
    DgnPlatformLib::Initialize (s_testsHost);
    DwgImporter::Initialize (nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    C3dImporterTests::TearDownTestCase ()
    {
    s_testsHost.Terminate (false);
    DwgImporter::TerminateDwgHost ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::Utf8String  C3dImporterTests::BuildModelspaceModelname (BeFileNameCR dwgFilename)
    {
    // apply the model naming rule from ConvertConfig.xml
    return Utf8PrintfString ("%ls", dwgFilename.GetFileNameWithoutExtension().c_str());
    }


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          12/19
+===============+===============+===============+===============+===============+======*/
struct C3dImporterBasicTests : public C3dImporterTestsFixture
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbHandle AddCircle (uint64_t& dwgModelId) const
    {
    ScopedDwgHost   host(m_options);
    DwgFileEditor   editor(m_dwgFileName);
    editor.AddCircleInDefaultModel ();

    // save off the entity handle
    DwgDbHandle entityHandle = editor.GetCurrentObjectId().GetHandle ();
    EXPECT_TRUE (!entityHandle.IsNull());

    dwgModelId = editor.GetModelspaceId().ToUInt64 ();

    editor.SaveFile ();
    return  entityHandle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DeleteEntity (DwgDbHandleCR entityHandle, uint64_t& modelspaceId) const
    {
    ScopedDwgHost   host(m_options);
    DwgFileEditor   editor(m_dwgFileName);
    editor.DeleteEntity (entityHandle);
    modelspaceId = editor.GetModelspaceId().ToUInt64 ();
    editor.SaveFile ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  CountElements (Utf8CP elemClass=BIS_SCHEMA(BIS_CLASS_Element), Utf8CP where=nullptr) const
    {
    auto db = OpenExistingDgnDb (m_dgnDbFileName, Db::OpenMode::Readonly);
    EXPECT_TRUE (db.IsValid());
    EXPECT_TRUE (db->IsDbOpen());

    size_t numElements = db->Elements().MakeIterator(elemClass, where).BuildIdList<DgnElementId>().size ();
    return  numElements;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    FindModelElement (uint64_t dwgModelId, DgnDbR db, DwgSourceAspects::ModelAspect::SourceType sourceType) const
    {
    // matching DwgSourceAspects::ModelAspect::SourceType enumeration
    static Utf8CP s_sourcetypes[]={"Unknown","ModelSpace","PaperSpace","XRefAttachment","RasterAttachment"};
    uint32_t index = static_cast<uint32_t>(sourceType);

    Utf8PrintfString    idstr("0x%X", dwgModelId);
    Utf8PrintfString    typestr("%s", s_sourcetypes[index]);

    auto sql = db.GetPreparedECSqlStatement(
            "SELECT x.Element.Id, x.JsonProperties FROM " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " x "
            " WHERE (x.Kind=? AND x.Identifier=? AND json_extract(x.JsonProperties, '$.SourceType')=?)");

    EXPECT_TRUE(sql != nullptr) << "SQL query for model ExternalSourceAspect failed!";

    sql->BindText(1, DwgSourceAspects::BaseAspect::Kind::Model, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    sql->BindText(2, idstr.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    sql->BindText(3, typestr.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);

    DgnElementId    elementId;
    if (BE_SQLITE_ROW == sql->Step())
        elementId = sql->GetValueId<DgnElementId>(0);
    return  elementId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    FindElement (DwgDbHandleCR entityHandle, uint64_t dwgModelId, DgnDbR db, bool shouldExist = true) const
    {
    // find model mapped from the modelspace with its blockID
    auto modelElementId = FindModelElement(dwgModelId, db, DwgSourceAspects::ModelAspect::SourceType::ModelSpace);
    
    Utf8PrintfString    handleStr("0x%ls", entityHandle.AsAscii().c_str());
    Utf8PrintfString    modelIdStr("0x%X", modelElementId.GetValue());

    auto sql = db.GetPreparedECSqlStatement("SELECT Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE (Scope.Id=? AND Kind=? AND Identifier=?)");
    EXPECT_TRUE(sql != nullptr) << "SQL query for element ExternalSourceAspect failed!";

    sql->BindText(1, modelIdStr.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    sql->BindText(2, DwgSourceAspects::BaseAspect::Kind::Element, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    sql->BindText(3, handleStr.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);

    DgnElementId    elementId;
    if (BE_SQLITE_ROW == sql->Step())
        elementId = sql->GetValueId<DgnElementId>(0);

    EXPECT_EQ(shouldExist, elementId.IsValid()) << "Check for existing elememnt failed!";
    
    return  elementId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckDbElement (size_t expectedCount, DwgDbHandleCR entityHandle, uint64_t dwgModelId, bool shouldExist) const
    {
    // check expected element count matching imported header elements
    EXPECT_EQ (expectedCount, CountElements(BIS_SCHEMA(BIS_CLASS_SpatialElement), "WHERE Parent.Id IS NULL")) << "Spaitial element count in the DgnDb is incorrect.";

    // retreive element ID for the requested entity handle from the syncInfo:
    auto db = OpenExistingDgnDb (m_dgnDbFileName, Db::OpenMode::Readonly);
    auto elementId = FindElement (entityHandle, dwgModelId, *db, shouldExist);
    
    // check the presence of the imported element
    EXPECT_EQ (shouldExist, elementId.IsValid()) << "The requested elememnt is not added/deleted as it should!";
    }

};  // C3dImporterBasicTests


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(C3dImporterBasicTests, UpdateElements_AddDelete)
    {
    LineUpFiles(L"addDeleteTest.bim", L"c3dtest1.dwg", true); 
    size_t  allElements = CountElements ();
    size_t  numImported = CountElements (BIS_SCHEMA(BIS_CLASS_SpatialElement), "WHERE Parent.Id IS NULL");
    EXPECT_EQ (numImported, GetCount()) << "Imported element count differs";

    // add a circle in modelspace
    uint64_t    modelspaceId = 0;
    DwgDbHandle entityHandle = AddCircle (modelspaceId);

    // update DgnDb
    DoUpdate (m_dgnDbFileName, m_dwgFileName);
    // exactly 1 element imported?
    EXPECT_EQ (1, GetCount()) << "Should have only added one element";
    CheckDbElement (numImported + 1, entityHandle, modelspaceId, true);
    // total elements unchanged?
    EXPECT_EQ (allElements + 1, CountElements()) << "Total BIM element count differs";

    // delete the circle from the DWG file
    DeleteEntity (entityHandle, modelspaceId);

    // update DgnDb again
    DoUpdate (m_dgnDbFileName, m_dwgFileName);
    // none imported?
    EXPECT_EQ (0, GetCount()) << "No element should have been imported";
    // imported elements back to original count?
    EXPECT_EQ (numImported, CountElements(BIS_SCHEMA(BIS_CLASS_SpatialElement), "WHERE Parent.Id IS NULL")) << "Imported element count differs";
    // total elements back to original count?
    EXPECT_EQ (allElements, CountElements()) << "Total element count differs";
    }


    
/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          12/19
+===============+===============+===============+===============+===============+======*/
struct C3dImporterAppTests : public C3dImporterTests
{
Utf8String  m_command;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RunCommand (Utf8StringCR args)
    {
    if (m_command.empty())
        return  BentleyStatus::BSIERROR;

    Utf8String  cmd = m_command;
    if (!args.empty())
        cmd += " " + args;

    ::printf ("Running command: %s\n", cmd.c_str());

    int status = ::system (cmd.c_str());

    ::printf ("status returned from call to ::system = %d" , status);
    if (status != ::SUCCESS)
        {
        char msg[500] = {0};
        ::strerror_s (msg, sizeof(msg)-1, status);
        ::printf (", %s", msg);
        }
    ::printf ("\n");

    return static_cast<BentleyStatus>(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
void InitializeCommand ()
    {
    BeFileName exe;
    BeTest::GetHost().GetDocumentsRoot (exe);
    exe.AppendToPath (L"C3dImporter.exe");
    ASSERT_PRESENT (exe.c_str());
    m_command.Assign (exe.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
void AppendInputFile (BeFileNameCR path)
    {
    m_command += " --input=" + Utf8String(path.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
void AppendOutputFile (BeFileNameCR path)
    {
    m_command += " --output=" + Utf8String(path.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void  CheckAeccAlignments (DgnDbCR db, DgnModelId modelId, size_t expectedCount)
    {
    Utf8PrintfString classSpec("%s.%s", C3DSCHEMA_SchemaAlias, ECCLASSNAME_AeccAlignment);
    Utf8PrintfString where("WHERE Model.Id=%I64d", modelId.GetValue());

    size_t  count = 0;
    for (auto id : db.Elements().MakeIterator(classSpec.c_str(), where.c_str()).BuildIdSet<DgnElementId>())
        {
        auto aeccAlign = db.Elements().Get<DgnElement>(id);
        ASSERT_TRUE(aeccAlign.IsValid());
        auto userLabel = aeccAlign->GetUserLabel ();
        EXPECT_STREQ("Route97_Extension", userLabel) << "Wrong AeccAlignment user label!";
        
        ECN::ECValue value;
        EXPECT_DGNDBSUCCESS(aeccAlign->GetPropertyValue(value, ECPROPNAME_StartStation));
        if (!aeccAlign->GetParentId().IsValid())
            EXPECT_NEAR(value.GetDouble(), 3616.55, 0.01) << "StartStation property an AeccAlignment has a wrong value!";
        EXPECT_DGNDBSUCCESS(aeccAlign->GetPropertyValue(value, ECPROPNAME_EndStation));
        if (!aeccAlign->GetParentId().IsValid())
            EXPECT_NEAR(value.GetDouble(), 6604.04, 0.01) << "EndStation property on an AeccAlignment has a wrong value!";
        EXPECT_DGNDBSUCCESS(aeccAlign->GetPropertyValue(value, ECPROPNAME_DesignSpeeds));
        EXPECT_TRUE(value.IsArray()) << "The DesignSpeeds property on an AeccAlignment should be an array type";
        EXPECT_DGNDBSUCCESS(aeccAlign->GetPropertyValue(value, ECPROPNAME_VAlignments));
        EXPECT_TRUE(value.IsArray()) << "The VAlignments property on an AeccAlignment should be an array type";
        
        auto aspects = iModelExternalSourceAspect::GetAllByKind (*aeccAlign, "Element");
        EXPECT_GE(aspects.size(),1) << "Should have at least 1 ExternalSourceAspect on an AeccAlignment element!";
        EXPECT_TRUE(aspects[0].IsValid()) << "An AeccAlignment element has an invalid ExternalSourceAspect!";
        count++;
        }
    EXPECT_EQ(expectedCount,count) << "Wrong number of AeccAlignment elements in the modelspace model!";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckAeccVAlignments (DgnDbCR db, DgnModelId modelId, size_t expectedCount)
    {
    Utf8PrintfString classSpec("%s.%s", C3DSCHEMA_SchemaAlias, ECCLASSNAME_AeccVAlignment);
    Utf8PrintfString where("WHERE Model.Id=%I64d", modelId.GetValue());

    size_t  count = 0;
    for (auto id : db.Elements().MakeIterator(classSpec.c_str(), where.c_str()).BuildIdSet<DgnElementId>())
        {
        auto aeccValign = db.Elements().Get<DgnElement>(id);
        ASSERT_TRUE(aeccValign.IsValid());
        
        ECN::ECValue value;
        EXPECT_DGNDBSUCCESS(aeccValign->GetPropertyValue(value, ECPROPNAME_VAlignment));
        EXPECT_TRUE(value.IsStruct()) << "The value of VAlignment property on an AeccVAlignment should be a struct type";
        EXPECT_DGNDBSUCCESS(aeccValign->GetPropertyValue(value, "VAlignment.StartStation"));
        EXPECT_GT(value.GetDouble(), 0.0) << "Should have a non-zero value of StartStation property on an AeccVAlignment";
        EXPECT_DGNDBSUCCESS(aeccValign->GetPropertyValue(value, "VAlignment.EndStation"));
        EXPECT_GT(value.GetDouble(), 0.0) << "Should have a non-zero value of StartStation property on an AeccVAlignment";
        
        auto aspects = iModelExternalSourceAspect::GetAllByKind (*aeccValign, "Element");
        EXPECT_GE(aspects.size(),1) << "Should have at least 1 ExternalSourceAspect on an AeccVAlignment element!";
        EXPECT_TRUE(aspects[0].IsValid()) << "An AeccVAlignment element has an invalid ExternalSourceAspect!";
        count++;
        }
    EXPECT_EQ(expectedCount,count) << "Wrong number of AeccVAlignment elements in the modelspace model!";
    }

};  // ImporterAppTests


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(C3dImporterAppTests, CreateBim)
    {
    WString fileName(L"c3dtest1.dwg");
    BeFileName inFile;
    MakeWritableCopyOf(inFile, fileName.c_str());
    InitializeCommand();
    AppendInputFile(inFile);
    AppendOutputFile(GetOutputDir() );

    ASSERT_EQ(BSISUCCESS, RunCommand(""));

    BeFileName outFile = GetDgnDbFileName(inFile);
    EXPECT_PRESENT(outFile.c_str())
    EXPECT_PRESENT(inFile.c_str())

    auto db = OpenExistingDgnDb (outFile);
    ASSERT_TRUE (db.IsValid());

    size_t  count = 0;
    auto& models = db->Models ();
    SpatialModelPtr modelspaceModel;
    SpatialModelPtr alignedModel;

    // C3dImporter should have created 4 physical models
    for (auto const& entry : models.MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalModel)))
        {
        auto model = models.GetModel (entry.GetModelId());
        ASSERT_TRUE(model.IsValid());

        auto name = model->GetName ();
        if (name.EqualsI(BuildModelspaceModelname(inFile)))
            {
            modelspaceModel = model->ToSpatialModelP ();
            count++;
            }
        else if (name.EqualsI(ROADNETWORK_MODEL_NAME))
            {
            alignedModel = model->ToSpatialModelP ();
            count++;
            }
        else if (name.EqualsI(ALIGNMENTS_PARTITION_NAME) || name.EqualsI(RAILNETWORK_MODEL_NAME))
            {
            count++;
            }
        }
    EXPECT_EQ(4, count) << "Should have 4 physical models!";
    ASSERT_TRUE(alignedModel.IsValid()) << "Should have a Road/Rail Physical model!";
    ASSERT_TRUE(modelspaceModel.IsValid()) << "Should have a modelspace model!";
    
    auto modelId = modelspaceModel->GetModelId ();

    // expect 3 AeccAlignment elements in the modelspace model
    CheckAeccAlignments (*db, modelId, 3);

    // expect 3 AeccVAlignment elements in the modelspace model
    CheckAeccVAlignments (*db, modelId, 3);

    // check Civil domain elements
    auto horizElemsId = HorizontalAlignments::QueryId (*alignedModel);
    EXPECT_TRUE(horizElemsId.IsValid()) << "Should have HorizontalAlignments!";

    count = 0;
    for (auto id : db->Elements().MakeIterator(BRRA_SCHEMA(BRRA_CLASS_HorizontalAlignment)).BuildIdSet<DgnElementId>())
        {
        auto horizAlign = HorizontalAlignment::Get (*db, id);
        ASSERT_TRUE(horizAlign.IsValid()) << "Invalid HorizontalAlignment!";
        EXPECT_GE(horizAlign->GetGeometry().size(), 1) << "Null geometry on a HorizontalAlignment!";
        count++;
        }
    EXPECT_GE(count, 1) << "Should have 1 HorizontalAlignment in the AlignmentModel!";
    }
