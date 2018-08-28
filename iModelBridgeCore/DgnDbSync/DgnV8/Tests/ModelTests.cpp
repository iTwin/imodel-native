/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/ModelTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include "ImportConfigEditor.h"
#include <Bentley/BeTest.h>
#include <VersionedDgnV8Api/ECObjects/ECObjectsAPI.h>

#define TESTMODELNEW   "TestModelNew"
#define TESTMODELNEWW L"TestModelNew"

#define JustWaitHere //system("pause");

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      08/15
//----------------------------------------------------------------------------------------
struct ModelTests : public ConverterTestBaseFixture
    {
    DEFINE_T_SUPER(ConverterTestBaseFixture);
    
    void SetUp();
    void TearDown();
    void DoConvert(BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR input);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelTests::DoConvert(BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR input)
    {
    // *** TRICKY: the converter takes a reference to and will MODIFY its Params. Make a copy, so that it does not pollute m_params.
    RootModelConverter::RootModelSpatialParams params(m_params);

    params.SetInputFileName(input);
    params.SetBridgeRegSubKey(RootModelConverter::GetRegistrySubKey());

    RootModelConverter creator(params);
    creator.SetWantDebugCodes(true);
    auto db = OpenExistingDgnDb(output);
    ASSERT_TRUE(db.IsValid());
    creator.SetDgnDb(*db);
    creator.SetIsUpdating(false);
    creator.AttachSyncInfo();
    ASSERT_EQ(BentleyApi::SUCCESS, creator.InitRootModel());
    creator.MakeSchemaChanges();
    ASSERT_FALSE(creator.WasAborted());
    ASSERT_EQ(RootModelConverter::ImportJobCreateStatus::Success, creator.InitializeJob());
    creator.Process();
    ASSERT_FALSE(creator.WasAborted());
    db->SaveChanges();

    m_count = creator.GetElementsConverted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelTests::SetUp()
    {
    T_Super::SetUp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelTests::TearDown()
    {
    T_Super::TearDown();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, Design3dReferenceSameFile)
    {
    LineUpFiles(L"Design3dReferenceSameFile.ibim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    Bentley::DgnModelP newModel = v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Normal, /*is3D*/ true);
    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid, newModel);
    v8editor.AddLine(&eid);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    //  Add refV8File as an attachment to v8File
    Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(m_v8FileName.c_str());
    DgnV8Api::DgnAttachment* attachment;
    ASSERT_EQ(BentleyApi::SUCCESS, v8editor.m_defaultModel->CreateDgnAttachment(attachment, *moniker, TESTMODELNEWW, true));
    ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());
    auto refsSubject = GetReferencesChildSubjectOf(*GetJobHierarchySubject(*db));
    ASSERT_TRUE(refsSubject.IsValid());
    DgnCode partitionCode = InformationPartitionElement::CreateCode(*refsSubject, TESTMODELNEW);
    DgnModelId modelId = db->Models().QuerySubModelId(partitionCode);
    ASSERT_TRUE(modelId.IsValid()) << "Model is direct reference , It should be imported";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, Design3dSelfReference)
    {
    LineUpFiles(L"Design3dSelfReference.ibim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    //  Add refV8File as an attachment to v8File
    Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(m_v8FileName.c_str());
    DgnV8Api::DgnAttachment* attachment;
    ASSERT_EQ(BentleyApi::SUCCESS, v8editor.m_defaultModel->CreateDgnAttachment(attachment, *moniker, L"Default", true));
    attachment->SetNestDepth(99);

    RotMatrix matrix = RotMatrix::FromRowValues(0, -1.0, 0, 1, 0, 0, 0, 0, 1);
    Transform inputTransform = Transform::From(matrix);
    EXPECT_TRUE(SUCCESS == attachment->TransformAttachment(inputTransform));
    ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());
    v8editor.Save();

    // This self-reference inherently causes the same element to be converted twice with the same code, which we intentionally assert on.
    BentleyApi::BeTest::SetFailOnAssert(false);
    DoConvert(m_dgnDbFileName, m_v8FileName);
    BentleyApi::BeTest::SetFailOnAssert(true);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());
    auto jobSubject = GetJobHierarchySubject(*db);
    ASSERT_TRUE(jobSubject.IsValid());
    DgnCode partitionCode = InformationPartitionElement::CreateCode(*jobSubject, "Test3d");
    DgnModelId modelId = db->Models().QuerySubModelId(partitionCode);
    ASSERT_TRUE(modelId.IsValid());

    // We also expect a reference
    auto refsSubject = GetReferencesChildSubjectOf(*GetJobHierarchySubject(*db));
    ASSERT_TRUE(refsSubject.IsValid());
    auto childids = refsSubject->QueryChildren();
    int referencedPartionCount = 0;
    int otherCount = 0;
    for (auto childid : childids)
        {
        auto ppart = db->Elements().Get<PhysicalPartition>(childid);
        if (ppart.IsValid())
            ++referencedPartionCount;
        else
            ++otherCount;
        }
    ASSERT_EQ(1, referencedPartionCount) << " We expect a single reference directly to the physical partition of the attached model";
    ASSERT_EQ(0, otherCount)  << " We don't expect any nested references";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, Design3dReference2DSameFile)
    {
    LineUpFiles(L"Design3d.ibim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Drawing, /*is3D*/ false);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    //  Add refV8File as an attachment to v8File
    Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(m_v8FileName.c_str());
    DgnV8Api::DgnAttachment* attachment;
    ASSERT_EQ(BentleyApi::SUCCESS, v8editor.m_defaultModel->CreateDgnAttachment(attachment, *moniker, TESTMODELNEWW, true));
    ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    auto jobSubject = GetJobHierarchySubject(*db);
    ASSERT_TRUE(jobSubject.IsValid());
    DgnCode partitionCode = InformationPartitionElement::CreateCode(*jobSubject, TESTMODELNEW);
    DgnModelId modelId = db->Models().QuerySubModelId(partitionCode);
    ASSERT_FALSE(modelId.IsValid());        // 3D -> 2D attachments are NOT supported.
    }

/*---------------------------------------------------------------------------------**//**
// WIP
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, DrawingModelUnicodeName)
    {
    LineUpFiles(L"DrawingModelUnicodeName.ibim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    WString unicodeName(L"ماڈل");
    v8editor.m_file->CreateNewModel(&modelStatus, unicodeName.c_str(), DgnV8Api::DgnModelType::Drawing, /*is3D*/ false);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_Drawing)));
    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, NoDefaultRoot)
    {
    LineUpFiles(L"NoDefaultRoot.ibim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    DgnV8Api::DgnModel* model = v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Normal, /*is3D*/ true);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    ASSERT_EQ(model->GetModelId(), v8editor.m_file->FindModelIdByName(TESTMODELNEWW));
    v8editor.Save();

    m_params.SetRootModelChoice(RootModelConverter::RootModelChoice(TESTMODELNEW));
    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    auto jobSubject = GetJobHierarchySubject(*db);
    ASSERT_TRUE(jobSubject.IsValid());
    DgnCode partitionCode = InformationPartitionElement::CreateCode(*jobSubject, TESTMODELNEW);
    DgnModelId modelId = db->Models().QuerySubModelId(partitionCode);
    ASSERT_TRUE(modelId.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, Design2D)
    {
    LineUpFiles(L"Design3d.ibim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_Drawing)));
    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, Design3D)
    {
    LineUpFiles(L"Design3d.ibim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Normal, /*is3D*/ true);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    auto jobSubject = GetJobHierarchySubject(*db);
    ASSERT_TRUE(jobSubject.IsValid());
    DgnCode partitionCode = InformationPartitionElement::CreateCode(*jobSubject, TESTMODELNEW);
    DgnModelId modelId = db->Models().QuerySubModelId(partitionCode);
    ASSERT_FALSE(modelId.IsValid()) << "Only root 3D model should be imported";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, Design2dAsDefault)
    {
    LineUpFiles(L"design2dasdefault.ibim", L"Test2d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    DgnV8Api::DgnModel* model = v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    v8editor.m_file->SetDefaultModelID(model->GetModelId());
    v8editor.Save();
    DgnV8Api::ModelId mId = v8editor.m_file->FindModelIdByName(L"Default");
    DgnV8Api::DgnModel* rootModel = v8editor.m_file->LoadModelById(mId).get();
    ASSERT_TRUE(NULL != rootModel);

    ASSERT_EQ(SUCCESS, v8editor.m_file->DeleteModel(*rootModel));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_Drawing)));
    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, Design3dAsDefault)
    {
    LineUpFiles(L"design2dasdefault.ibim", L"Test2d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    DgnV8Api::DgnModel* model = v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Normal, /*is3D*/ true);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    v8editor.m_file->SetDefaultModelID(model->GetModelId());
    v8editor.Save();
    DgnV8Api::ModelId mId = v8editor.m_file->FindModelIdByName(L"Default");
    DgnV8Api::DgnModel* rootModel = v8editor.m_file->LoadModelById(mId).get();
    ASSERT_TRUE(NULL != rootModel);

    ASSERT_EQ(SUCCESS, v8editor.m_file->DeleteModel(*rootModel));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    auto jobSubject = GetJobHierarchySubject(*db);
    ASSERT_TRUE(jobSubject.IsValid());
    DgnCode modelCode = InformationPartitionElement::CreateCode(*jobSubject, TESTMODELNEW);
    DgnModelId rootModelId = db->Models().QuerySubModelId(modelCode);
    ASSERT_TRUE(rootModelId.IsValid());
    ASSERT_TRUE(db->Models().GetModel(rootModelId)->Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, ConvertDesign2dRootModelTo3d)
    {
    LineUpFiles(L"design2drootmodelto3d.ibim", L"Test2d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    DgnV8Api::DgnModel* model = v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    v8editor.m_file->SetDefaultModelID(model->GetModelId());
    v8editor.Save();
    DgnV8Api::ModelId mId = v8editor.m_file->FindModelIdByName(L"Default");
    DgnV8Api::DgnModel* rootModel = v8editor.m_file->LoadModelById(mId).get();
    ASSERT_TRUE(NULL != rootModel);

    ASSERT_EQ(SUCCESS, v8editor.m_file->DeleteModel(*rootModel));
    v8editor.Save();

    m_params.SetConsiderNormal2dModelsSpatial(true);
    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    auto jobSubject = GetJobHierarchySubject(*db);
    ASSERT_TRUE(jobSubject.IsValid());
    DgnCode modelCode = InformationPartitionElement::CreateCode(*jobSubject, TESTMODELNEW);
    DgnModelId rootModelId = db->Models().QuerySubModelId(modelCode);
    ASSERT_TRUE(rootModelId.IsValid());
    ASSERT_TRUE(db->Models().GetModel(rootModelId)->Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, DrawingModel2D)
    {
    LineUpFiles(L"drawingmodel2d.ibim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Drawing, /*is3D*/ false);
    ASSERT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_Drawing)));
    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel)));

    //Drawing_Model_not2D
    auto jobSubject = GetJobHierarchySubject(*db);
    ASSERT_TRUE(jobSubject.IsValid());
    DgnCode partitionCode = InformationPartitionElement::CreateCode(*jobSubject, TESTMODELNEW);
    DgnModelId modelId = db->Models().QuerySubModelId(partitionCode);
    ASSERT_FALSE(modelId.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, DrawingModel_Only2d)
    {
    LineUpFiles(L"DrawingModel_Only2d.ibim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Drawing, /*is3D*/ false);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_Drawing)));
    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel)));

    auto jobSubject = GetJobHierarchySubject(*db);
    ASSERT_TRUE(jobSubject.IsValid());
    DgnCode partitionCode = InformationPartitionElement::CreateCode(*jobSubject, "Default");
    DgnModelId modelId = db->Models().QuerySubModelId(partitionCode);
    ASSERT_FALSE(modelId.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, DrawingModelAsDefault)
    {
    LineUpFiles(L"drawingmodelasdefault.ibim", L"Test2d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    DgnV8Api::DgnModel* drawingModel = v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Drawing, /*is3D*/ false);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    v8editor.m_file->SetDefaultModelID(drawingModel->GetModelId());
    v8editor.Save();
    DgnV8Api::ModelId mId = v8editor.m_file->FindModelIdByName(L"Default");
    DgnV8Api::DgnModel* rootModel = v8editor.m_file->LoadModelById(mId).get();
    ASSERT_TRUE(NULL != rootModel);

    ASSERT_EQ(SUCCESS, v8editor.m_file->DeleteModel(*rootModel));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_Drawing)));
    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, SheetModel2D)
    {
    LineUpFiles(L"sheetmodel3d.ibim", L"Test3d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_Sheet)));
    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_SheetModel)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, SheetModel3D)
    {
    LineUpFiles(L"SheetModel3d.ibim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Sheet, /*is3D*/ true);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    auto jobSubject = GetJobHierarchySubject(*db);
    ASSERT_TRUE(jobSubject.IsValid());
    DgnCode modelCode = InformationPartitionElement::CreateCode(*jobSubject, TESTMODELNEW);
    DgnModelId modelId = db->Models().QuerySubModelId(modelCode);
    ASSERT_FALSE(modelId.IsValid()) << "Only root 3D model should be imported";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, SheetModelAsDefault)
    {
    LineUpFiles(L"sheetmodelasdefault.ibim", L"Test2d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    DgnV8Api::DgnModel* sheetmodel = v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    v8editor.m_file->SetDefaultModelID(sheetmodel->GetModelId());
    v8editor.Save();
    DgnV8Api::ModelId mId = v8editor.m_file->FindModelIdByName(L"Default");
    DgnV8Api::DgnModel* rootModel = v8editor.m_file->LoadModelById(mId).get();
    ASSERT_TRUE(NULL != rootModel);

    ASSERT_EQ(SUCCESS, v8editor.m_file->DeleteModel(*rootModel));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_Sheet)));
    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_SheetModel)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
// only 2d Design and Drawing models can be promoted to 3d
TEST_F(ModelTests, ConvertSheet2dRootModelTo3d)
    {
    LineUpFiles(L"convertsheetrootmodelto3d.ibim", L"Test2d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    DgnV8Api::DgnModel* sheetmodel = v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    v8editor.m_file->SetDefaultModelID(sheetmodel->GetModelId());
    v8editor.Save();
    DgnV8Api::ModelId mId = v8editor.m_file->FindModelIdByName(L"Default");
    DgnV8Api::DgnModel* rootModel = v8editor.m_file->LoadModelById(mId).get();
    ASSERT_TRUE(NULL != rootModel);

    ASSERT_EQ(SUCCESS, v8editor.m_file->DeleteModel(*rootModel));
    v8editor.Save();

    m_params.SetConsiderNormal2dModelsSpatial(true);   // this will not work - a sheet is never promoted to 3D
    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    auto jobSubject = GetJobHierarchySubject(*db);
    ASSERT_TRUE(jobSubject.IsValid());
    DgnCode modelCode = InformationPartitionElement::CreateCode(*jobSubject, TESTMODELNEW);
    DgnModelId rootModelId = db->Models().QuerySubModelId(modelCode);
    ASSERT_FALSE(rootModelId.IsValid()) << " We do not expect a sheet model to become the subject";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, SpecifyRootModel_ByName)
    {
    LineUpFiles(L"drawingmodel2d.ibim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
    ASSERT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    // Root Model choice
    m_params.SetRootModelChoice(RootModelConverter::RootModelChoice(TESTMODELNEW));
    // Set Convert Root Drawing to 3D
    m_params.SetConsiderNormal2dModelsSpatial(true);
    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    auto jobSubject = GetJobHierarchySubject(*db);
    ASSERT_TRUE(jobSubject.IsValid());
    DgnCode partitionCode = InformationPartitionElement::CreateCode(*jobSubject, TESTMODELNEW);
    DgnModelId modelId = db->Models().QuerySubModelId(partitionCode);
    ASSERT_TRUE(modelId.IsValid()); db->Models();
    ASSERT_TRUE(db->Models().GetModel(modelId)->Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, SpecifyRootModel_ById)
    {
    LineUpFiles(L"drawingmodel2d.ibim", L"Test3d.dgn", false);

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    DgnV8Api::DgnModel* model = v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
    ASSERT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    // Root Model choice
    m_params.SetRootModelChoice(RootModelConverter::RootModelChoice(model->GetModelId()));
    // Set Convert Root Drawing to 3D
    m_params.SetConsiderNormal2dModelsSpatial(true);
    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    auto jobSubject = GetJobHierarchySubject(*db);
    ASSERT_TRUE(jobSubject.IsValid());
    DgnCode partitionCode = InformationPartitionElement::CreateCode(*jobSubject, TESTMODELNEW);
    DgnModelId modelId = db->Models().QuerySubModelId(partitionCode);
    ASSERT_TRUE(modelId.IsValid()); db->Models();
    ASSERT_TRUE(db->Models().GetModel(modelId)->Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, SpecifyRootModel_UseDefaultModel)
    {
    LineUpFiles(L"drawingmodel2d.ibim", L"Test3d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    DgnV8Api::DgnModel* model = v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
    ASSERT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    v8editor.m_file->SetDefaultModelID(model->GetModelId());
    v8editor.Save();
    DgnV8Api::ModelId mId = v8editor.m_file->FindModelIdByName(L"Default");
    DgnV8Api::DgnModel* rootModel = v8editor.m_file->LoadModelById(mId).get();
    ASSERT_TRUE(NULL != rootModel);

    ASSERT_EQ(SUCCESS, v8editor.m_file->DeleteModel(*rootModel));
    v8editor.Save();

    // Root Model choice
    RootModelConverter::RootModelChoice rootModelChoice;
    rootModelChoice.SetUseDefaultModel();
    m_params.SetRootModelChoice(rootModelChoice);
    // Set Convert Root Drawing to 3D
    m_params.SetConsiderNormal2dModelsSpatial(true);
    DoConvert(m_dgnDbFileName, m_v8FileName);
    EXPECT_EQ(RootModelConverter::RootModelChoice::Method::UseDefaultModel, rootModelChoice.m_method);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    auto jobSubject = GetJobHierarchySubject(*db);
    ASSERT_TRUE(jobSubject.IsValid());
    DgnCode partitionCode = InformationPartitionElement::CreateCode(*jobSubject, TESTMODELNEW);
    DgnModelId modelId = db->Models().QuerySubModelId(partitionCode);
    ASSERT_TRUE(modelId.IsValid()); db->Models();
    ASSERT_TRUE(db->Models().GetModel(modelId)->Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, SpecifyRootModel_FromActiveViewGroup)
    {
    LineUpFiles(L"drawingmodel2d.ibim", L"Test3d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    DgnV8Api::DgnModel* model = v8editor.m_file->CreateNewModel(&modelStatus, TESTMODELNEWW, DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
    ASSERT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();

    DgnV8Api::ViewGroupPtr viewGroup;
    DgnFileR dgnFile = *v8editor.m_file.get();
    ASSERT_EQ(DgnV8Api::VG_Success, DgnV8Api::ViewGroup::Create(viewGroup, *model, false, L"NewViewGroup", true));
    ASSERT_TRUE(viewGroup.IsValid());
    EXPECT_EQ(DgnV8Api::VG_Success, viewGroup->WriteImmediatelyToFile());
    v8editor.Save();

    DgnV8Api::ViewGroupPtr activeViewGroup;
    DgnV8Api::ViewGroup::FindActiveViewGroup(activeViewGroup, dgnFile);
    if (L"Default Views" == activeViewGroup.get()->GetName())
        {
        //printf("%ls", activeViewGroup.get()->GetName().c_str());
        DgnV8Api::ViewGroupCollection& viewGroupCollection = dgnFile.GetViewGroupsR();
        // deleting the active view will make the newly created view "NewViewGroup" as active
        viewGroupCollection.Delete(activeViewGroup.get());
        //ASSERT_EQ(DgnV8Api::VG_Success, viewGroupCollection.MakeActive(viewGroup.get(), false));
        //EXPECT_EQ(DgnV8Api::VG_Success, viewGroupCollection.SaveChanges());
        //EXPECT_EQ(DgnV8Api::VG_Success, viewGroup->WriteImmediatelyToFile());
        }

    v8editor.Save();

    // Root Model choice
    RootModelConverter::RootModelChoice rootModelChoice;
    rootModelChoice.SetUseActiveViewGroup();
    m_params.SetRootModelChoice(rootModelChoice);
    // Set Convert Root Drawing to 3D
    m_params.SetConsiderNormal2dModelsSpatial(true);
    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db->IsDbOpen());

    auto jobSubject = GetJobHierarchySubject(*db);
    ASSERT_TRUE(jobSubject.IsValid());
    DgnCode partitionCode = InformationPartitionElement::CreateCode(*jobSubject, TESTMODELNEW);
    DgnModelId modelId = db->Models().QuerySubModelId(partitionCode);
    ASSERT_TRUE(modelId.IsValid()); db->Models();
    ASSERT_TRUE(db->Models().GetModel(modelId)->Is3d());
    }

static void addDrawingModel(BentleyApi::BeFileName filename, WCharCP modelName)
    {
    V8FileEditor v8editor;
    v8editor.Open(filename);
    DgnV8Api::DgnModelStatus modelStatus;
    v8editor.m_file->CreateNewModel(&modelStatus, modelName, DgnV8Api::DgnModelType::Drawing, /*is3D*/ false);
    ASSERT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();
    }

static void addLine(BentleyApi::BeFileName filename, DgnV8Api::ModelId mid)
    {
    V8FileEditor v8editor;
    v8editor.Open(filename);
    DgnV8Api::DgnModel* v8Model = v8editor.m_file->LoadModelById(mid).get();
    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid, v8Model);
    v8editor.AddLine(&eid);
    v8editor.Save();
    }

static void addRefAttachment(BentleyApi::BeFileName filename, DgnV8Api::ModelId masterModelId, BentleyApi::BeFileName const& refFileName, WCharCP refModelName)
    {
    V8FileEditor v8editor;
    v8editor.Open(filename);
    DgnV8Api::DgnModel* masterModel = v8editor.m_file->LoadModelById(masterModelId).get();
    Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(refFileName.c_str());
    DgnV8Api::DgnAttachment* attachment;
    ASSERT_EQ(BentleyApi::SUCCESS, masterModel->CreateDgnAttachment(attachment, *moniker, refModelName, true));
    ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());
    v8editor.Save();
    }

static size_t countJobSubjects(DgnDbR db)
    {
    size_t actualCount = 0;
    auto childids = db.Elements().GetRootSubject()->QueryChildren();
    for (auto childid : childids)
        {
        auto subj = db.Elements().Get<Subject>(childid);
        if (subj.IsValid() && JobSubjectUtils::IsJobSubject(*subj))
            ++actualCount;
        }
    return actualCount;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ModelTests, MultipleRootFiles)
    {
    // Preconditions: We must have 2 "root" or "master" files

    LineUpFiles(L"DeletedModelDetectionWithMultipleRoots.ibim", L"Test3d.dgn", false);
    BentleyApi::BeFileName root1(m_v8FileName);

    BentleyApi::BeFileName root2(root1.GetDirectoryName());
    root2.AppendToPath(L"Root2.dgn");
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(root1, root2));

    BentleyApi::BeFileName commonRefFile(root1.GetDirectoryName());
    commonRefFile.AppendToPath(L"Ref.dgn");
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(root1, commonRefFile));

    // --------------------------------------
    //  Test: No duplicate drawings
    // --------------------------------------

    // Preconditions: each masterfile has both a 3d model and a drawing model ...
    addDrawingModel(root1, L"Drawing1");
    addDrawingModel(root2, L"Drawing2");

    // ... and the converter searches for drawings in both files. (This is what the iModel bridge framework causes bridges to do.)
    m_params.AddDrawingAndSheetFile(root1);
    m_params.AddDrawingAndSheetFile(root2);

    // Convert Root1.dgn.
    // Verify that it creates a spatial model and 2 drawing models (that is, it finds the drawings in both root files).
    m_v8FileName = root1;
    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

    ASSERT_EQ(1, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_SpatialModel)));
    ASSERT_EQ(2, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel)));

    db->CloseDb();
    db = nullptr;

    // Convert Root2.dgn, writing to the same iModel.
    // Verify that it creates a spatial model.
    // Verify that it creates no additional drawing models.
    // Verify that it does not delete any spatial models.
    m_v8FileName = root2;
    DoConvert(m_dgnDbFileName, m_v8FileName);

    db = OpenExistingDgnDb(m_dgnDbFileName);

    ASSERT_EQ(2, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_SpatialModel)));
    ASSERT_EQ(2, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel)));

    // Also verify that we have 2 job subjects
    ASSERT_EQ(2, countJobSubjects(*db));

    db->CloseDb();
    db = nullptr;

    // --------------------------------------
    // Test: one job does not delete the models created by the other job.
    // --------------------------------------

    //  Preconditions: we must make a change to each of the "master" files, so that the updates will process them.
    addLine(root1, 0);
    addLine(root2, 0);

    m_v8FileName = root1;
    DoUpdate(m_dgnDbFileName, m_v8FileName);
    db = OpenExistingDgnDb(m_dgnDbFileName);
    EXPECT_EQ(2, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_SpatialModel)));
    EXPECT_EQ(2, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel)));
    db->CloseDb();
    db = nullptr;

    m_v8FileName = root2;
    DoUpdate(m_dgnDbFileName, m_v8FileName);
    db = OpenExistingDgnDb(m_dgnDbFileName);
    EXPECT_EQ(2, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_SpatialModel)));
    EXPECT_EQ(2, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel)));
    db->CloseDb();
    db = nullptr;

    // --------------------------------------
    // Test: do not duplicate a common reference attachment file
    // --------------------------------------

    //  Add commonRefFile as an attachment to both root1 and root2
    addRefAttachment(root1, 0, commonRefFile, L"Default");
    addRefAttachment(root2, 0, commonRefFile, L"Default");

    //  Update, starting from root1. Verify that it imports commonRefFile
    m_v8FileName = root1;
    DoUpdate(m_dgnDbFileName, m_v8FileName);
    db = OpenExistingDgnDb(m_dgnDbFileName);
    EXPECT_EQ(3, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_SpatialModel))) << "Expect one more spatial model to have been imported";
    EXPECT_EQ(2, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel))) << "No additional drawings expected";
    db->CloseDb();
    db = nullptr;

    //      update, starting from root2. Verify that it does NOT import commonRefFile
    m_v8FileName = root2;
    DoUpdate(m_dgnDbFileName, m_v8FileName);
    db = OpenExistingDgnDb(m_dgnDbFileName);
    EXPECT_EQ(3, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_SpatialModel))) << "No additional spatial models expected";
    EXPECT_EQ(2, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel))) << "No additional drawings expected";
    db->CloseDb();
    db = nullptr;
    }