/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/DimensionTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include <VersionedDgnV8Api/DgnPlatform/LevelTypes.h>
#include "GeomTestHelper.h"

#define _QUOTEME(x) _CRT_WIDE( #x)
#define TMPNAME(x) _QUOTEME(x)

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      02/16
//----------------------------------------------------------------------------------------
struct DimensionTests : public GeomTestFixture 
{
    DEFINE_T_SUPER(GeomTestFixture);
    static WChar* noteStyleName;

    void        SetUpStyles(V8FileEditor& v8editor);
    DgnV8Api::DimensionStylePtr   GetDimStyle(V8FileEditor& v8editor) const { return DgnV8Api::DimensionStyle::GetByName(noteStyleName, *v8editor.m_defaultModel->GetDgnFileP()); }
    DgnV8Api::TextBlockPtr        GetText(WChar* text, V8FileEditor& v8editor) const;
    BentleyStatus       CreateNote(EditElementHandleR leader, EditElementHandleR note, V8FileEditor& v8editor);
};

WChar* DimensionTests::noteStyleName = L"NoteStyle";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
struct  DimCreateData : DgnV8Api::IDimCreateData
{
    DimensionStyleCR    m_dimStyle;
    DgnTextStyleCR      m_textStyle;
    RotMatrix           m_dimRMatrix;
    DgnV8Api::Symbology           m_symbology;

    DimCreateData (DimensionStyleCR dimStyle, DgnTextStyleCR textStyle, RotMatrixCR rMatrix)
        :
        m_dimStyle   (dimStyle),
        m_textStyle  (textStyle),
        m_dimRMatrix (rMatrix)
        {
        memset (&m_symbology, 0, sizeof m_symbology);
        }

    DimensionStyleCR    _GetDimStyle()      const override { return m_dimStyle;   }
    DgnTextStyleCR      _GetTextStyle()     const override { return m_textStyle;  }
    DgnV8Api::Symbology           _GetSymbology()     const override { return m_symbology; }
    DgnV8Api::LevelId             _GetLevelID()       const override { return (Bentley::DgnPlatform::LevelId)64;/*LEVEL_DEFAULT_LEVEL_ID */ }
    int                 _GetViewNumber()    const override { return 0;            }
    RotMatrixCR         _GetDimRMatrix()    const override { return m_dimRMatrix; }
    RotMatrixCR         _GetViewRMatrix()   const override { return m_dimRMatrix; }
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void  dimHandlerTest_createLinearDimension(DgnV8Api::EditElementHandle  &eeh, DimensionStyleCR dimStyle, Bentley::DgnModelR dgnCache)
    {
    DgnV8Api::DgnTextStylePtr textStyle = DgnV8Api::DgnTextStyle::Create(L"", *dgnCache.GetDgnFileP());

    RotMatrix       dimRMatrix;
    dimRMatrix.InitIdentity();

    DimCreateData   createData(dimStyle, *textStyle, dimRMatrix);

    DgnV8Api::DimensionType  dimType = DgnV8Api::DimensionType::SizeArrow;
    bool            is3d = dgnCache.Is3d();

    DgnV8Api::DimensionHandler::CreateDimensionElement(eeh, createData, dimType, is3d, dgnCache);

    ASSERT_EQ(BentleyApi::SUCCESS, eeh.AddToModel());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DimensionTests, Dimension)
    {
    LineUpFiles(L"Dimension.bim", L"Test3d.dgn", false); // creates TestAddRef.bim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    m_wantCleanUp = false;
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    DgnV8Api::EditElementHandle eeh;

    DgnV8Api::DimensionStylePtr   dimStyle = DgnV8Api::DimensionStyle::Create(L"", *v8editor.m_file);
    dimHandlerTest_createLinearDimension(eeh, *dimStyle, *v8editor.m_defaultModel);
    v8editor.SetActiveLevel(eeh);
    
    v8editor.Save();
    DoConvert(m_dgnDbFileName, m_v8FileName);
    // 
    //DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    //VerifyElement(*db, eeh.GetElementId(), GeometricPrimitive::GeometryType::CurveVector);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::TextBlockPtr    DimensionTests::GetText(WChar *text, V8FileEditor& v8editor) const
    {
    DgnV8Api::DgnTextStylePtr textstyle = DgnV8Api::DgnTextStyle::GetByName(noteStyleName, *v8editor.m_defaultModel->GetDgnFileP());
    DgnV8Api::TextBlockPtr textBlock = DgnV8Api::TextBlock::Create(*textstyle, *v8editor.m_defaultModel);
    textBlock->AppendText(text);
    return textBlock;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionTests::SetUpStyles(V8FileEditor& v8editor)
    {
    //T_Super::SetUp ();
    DgnV8Api::DimensionStylePtr dimStyle = DgnV8Api::DimensionStyle::Create(noteStyleName, *v8editor.m_defaultModel->GetDgnFileP());
    ASSERT_TRUE(SUCCESS == dimStyle->Add());

    DgnV8Api::DgnTextStylePtr textstyle = DgnV8Api::DgnTextStyle::Create(noteStyleName, *v8editor.m_defaultModel->GetDgnFileP());
    textstyle->SetProperty(DgnV8Api::TextStyle_Height, 10000.0);
    textstyle->SetProperty(DgnV8Api::TextStyle_Width, 10000.0);
    ASSERT_TRUE(SUCCESS == textstyle->Add());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionTests::CreateNote(EditElementHandleR leader, EditElementHandleR noteElem, V8FileEditor& v8editor)
    {
    DgnV8Api::DimensionStylePtr dimStyle = GetDimStyle(v8editor);
    
    DgnV8Api::TextBlockPtr text = GetText(TMPNAME(__LINE__), v8editor);

    DgnV8Api::NoteCellHeaderHandler::StdDPointVector points;
    DPoint3d point;
    point.Init(10000, 0, 0);
    points.push_back(point);
    point.Init(50000, 0, 0);
    points.push_back(point);
    return DgnV8Api::NoteCellHeaderHandler::CreateNote(noteElem, leader, *text, *dimStyle, v8editor.m_defaultModel->Is3d(), *v8editor.m_defaultModel, points);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void verifyview(DgnDbPtr db,BentleyApi::RefCountedCPtr<Sheet::ViewAttachment> viewattachment,double scale , BentleyApi::RefCountedCPtr<DrawingModel> drawingmodel)
{
    // Scaled copy ViewAttachment of Drawing model attached to sheet in bim world
    ASSERT_EQ(scale, viewattachment->GetScale());
    DgnViewId viewid1 = viewattachment->GetAttachedViewId();
    ASSERT_TRUE(viewid1.IsValid());
    BentleyApi::RefCountedCPtr<DrawingViewDefinition> viewdef = db->Elements().Get<DrawingViewDefinition>(viewid1);
    DgnModelId modeid = viewdef->GetBaseModelId();
    ASSERT_TRUE(modeid.IsValid());
    DgnModelCPtr model = db->Models().GetModel(modeid);
    ASSERT_TRUE(model.IsValid());
    ASSERT_TRUE(model->IsDrawingModel());
    ASSERT_TRUE(model->GetModelId() == drawingmodel->GetModelId());
}
/*+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DimensionTests, Note)
    {
    LineUpFiles(L"Note.bim", L"Test3d.dgn", false);
    m_wantCleanUp = false;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    SetUpStyles(v8editor);
    DgnV8Api::EditElementHandle noteElem;
    DgnV8Api::EditElementHandle leader;
    BentleyStatus status = CreateNote(leader, noteElem, v8editor);
    ASSERT_TRUE (SUCCESS == status);

    status = DgnV8Api::NoteCellHeaderHandler::AddToModel(noteElem, leader, *v8editor.m_defaultModel);
    ASSERT_TRUE (SUCCESS == status);
    v8editor.SetActiveLevel(noteElem);
    v8editor.SetActiveLevel(leader);
    v8editor.Save();

    
    DoConvert(m_dgnDbFileName, m_v8FileName);
    // 
    //DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    //VerifyElement(*db, eeh1.GetElementId(), GeometricPrimitive::GeometryType::CurveVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DimensionTests, NotetoSheet)
    {
    LineUpFiles(L"AnnotationstoSheet.bim", L"Test3d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ( 0 , m_count ) << L"The initial V8 file is supposed to be empty!";
    DgnV8Api::EditElementHandle noteElem;
    DgnV8Api::EditElementHandle leader;
    m_wantCleanUp = false;
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;
        DgnV8Api::ElementId eid;
        SetUpStyles(v8editor);
        Bentley::DgnModelP threeDModel = v8editor.m_defaultModel;
        ASSERT_TRUE(threeDModel->Is3d());

        DgnV8Api::NoteCellHeaderHandler*  m_noteObj;
        ElementRefP         m_oldRef;
        double const EPSILON = 0.000000001;
        //Set annotation scale for the 3d model.
        Bentley::DgnPlatform::ModelInfoPtr modelInfo = threeDModel->GetModelInfo().MakeCopy();
        EXPECT_NEAR(1.0, modelInfo->GetAnnotationScaleFactor(), EPSILON);
        EXPECT_EQ(SUCCESS, modelInfo->SetAnnotationScaleFactor(3.0));
        EXPECT_NEAR(3.0, modelInfo->GetAnnotationScaleFactor(), EPSILON);
        EXPECT_EQ(true, modelInfo->GetIsUseAnnotationScaleOn());
        EXPECT_EQ(Bentley::DgnPlatform::DgnModelStatus::DGNMODEL_STATUS_Success, threeDModel->SetModelInfo(*modelInfo));

        BentleyStatus status = CreateNote(leader, noteElem, v8editor);
        ASSERT_TRUE(SUCCESS == status);
        //Add annotation scale.
        status = DgnV8Api::NoteCellHeaderHandler::AddToModel(noteElem, leader, *v8editor.m_defaultModel);
        ASSERT_TRUE(SUCCESS == status);
        Bentley::DgnPlatform::Handler&  elmHandler = noteElem.GetHandler();
        m_oldRef = noteElem.GetElementRef();
        m_noteObj = dynamic_cast <DgnV8Api::NoteCellHeaderHandler*> (&elmHandler);
        EXPECT_TRUE(NULL != m_noteObj);

        double aScale3 = 0.0;
        EXPECT_EQ(true, m_noteObj->HasAnnotationScale(&aScale3, noteElem));
        EXPECT_EQ(3, aScale3);
        v8editor.SetActiveLevel(noteElem);
        v8editor.SetActiveLevel(leader);

        // Create a drawing1 model ...
        Bentley::DgnModelP drawingModel = v8editor.m_file->CreateNewModel(&modelStatus, L"Drawing1", DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        v8editor.AddLine(&eid, drawingModel);
        DgnV8Api::DgnAttachment* attachment = NULL;
        // and attach the 3D model as a reference to the 2D drawing
        Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(m_v8FileName.c_str());
        ASSERT_EQ(BentleyApi::SUCCESS, drawingModel->CreateDgnAttachment(attachment, *moniker, threeDModel->GetModelName(), true));
        ASSERT_TRUE(nullptr != attachment);
        ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());

        // Create a SheetModel1 ...
        Bentley::DgnModelP SheetModel = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet1", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        // and attach the 2D drawing as a reference to the 2D sheet1 
        DgnV8Api::DgnAttachment* attachment1 = NULL;
        Bentley::DgnDocumentMonikerPtr moniker1 = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(m_v8FileName.c_str());
        ASSERT_EQ(BentleyApi::SUCCESS, SheetModel->CreateDgnAttachment(attachment1, *moniker1, L"Drawing1", true));
        ASSERT_TRUE(nullptr != attachment1);
        attachment1->SetNestDepth(1);
        attachment1->SetScaleByUnits(true);
        ASSERT_EQ(true, attachment1->IsScaleByUnits());
        attachment1->SetStoredScale(50);
        ASSERT_EQ(50, attachment1->GetScaleStored());
        ASSERT_EQ(BentleyApi::SUCCESS, attachment1->WriteToModel());

        DgnV8Api::DgnAttachment* attachment2 = NULL;
        Bentley::DgnDocumentMonikerPtr moniker2 = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(m_v8FileName.c_str());
        ASSERT_EQ(BentleyApi::SUCCESS, SheetModel->CreateDgnAttachment(attachment2, *moniker2, L"Drawing1", true));
        ASSERT_TRUE(nullptr != attachment2);
        attachment2->SetNestDepth(1);
        attachment2->SetScaleByUnits(true);
        ASSERT_EQ(true, attachment2->IsScaleByUnits());
        attachment2->SetStoredScale(80);
        ASSERT_EQ(80, attachment2->GetScaleStored());
        ASSERT_EQ(BentleyApi::SUCCESS, attachment2->WriteToModel());
        v8editor.Save();
        }

    DoConvert(m_dgnDbFileName, m_v8FileName); 
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        countModels(*db, 2, 2);

        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_SpatialViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DrawingViewDefinition), 2);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 3);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DisplayStyle3d), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_ModelSelector), 1);

        auto drawingele = db->Elements().Get<Drawing>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_Drawing)));
        ASSERT_TRUE(drawingele.IsValid());
        auto drawingmodel = drawingele->GetSub<DrawingModel>();
        ASSERT_TRUE(drawingmodel.IsValid());
        //Count elements on drawing model 
        countElements(*drawingmodel, 3); 
        countElementsInModelByClass(*drawingmodel, getBisClassId(*db, "DrawingGraphic"), 3);

        BentleyApi::Bstdcxx::bvector<DgnElementId>idlist;
        idlist = db->Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_DrawingGraphic), nullptr, "ORDER BY ECInstanceId ASC").BuildIdList<DgnElementId>();
        ASSERT_EQ(3, idlist.size());
        BentleyApi::RefCountedCPtr<DrawingGraphic> Dragraphic1 = db->Elements().Get<DrawingGraphic>(idlist[0]);
        ASSERT_TRUE(Dragraphic1.IsValid());
//        ASSERT_TRUE(Dragraphic1->GetPlacement().GetElementBox().IsEqual(BentleyApi::ElementAlignedBox2d(0, 0, 1, 0))); [CGM] - This value is no longer correct, but I don't know how to confirm the actual value

        Sheet::ElementCPtr sheet = db->Elements().Get<Sheet::Element>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_Sheet)));
        ASSERT_TRUE(sheet.IsValid());
        Sheet::ModelPtr sheetModel = sheet->GetSub<Sheet::Model>();
        ASSERT_TRUE(sheetModel.IsValid());
        //Count elements on sheet 
        countElements(*sheetModel, 2);
        countElementsInModelByClass(*sheetModel, getBisClassId(*db, "ViewAttachment"), 2);
        //First Scaled Drawing attacment 
        idlist = db->Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_ViewAttachment), nullptr, "ORDER BY ECInstanceId ASC").BuildIdList<DgnElementId>();
        BentleyApi::RefCountedCPtr<Sheet::ViewAttachment> viewattachment1 = db->Elements().Get<Sheet::ViewAttachment>(idlist[0]);
        ASSERT_TRUE(viewattachment1.IsValid());
        verifyview(db,viewattachment1, 0.020, drawingmodel);
        //Second Scaled Drawing attacment 
        BentleyApi::RefCountedCPtr<Sheet::ViewAttachment> viewattachment2 = db->Elements().Get<Sheet::ViewAttachment>(idlist[1]);
        ASSERT_TRUE(viewattachment2.IsValid());
        verifyview(db, viewattachment2, 0.0125, drawingmodel);
       }
    }
