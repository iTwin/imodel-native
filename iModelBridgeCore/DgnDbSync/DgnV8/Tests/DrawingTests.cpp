/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/DrawingTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      08/15
//----------------------------------------------------------------------------------------
struct DrawingTests : public ConverterTestBaseFixture
{
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AddAttachment(BentleyApi::BeFileNameCR attachmentFileName, DgnV8ModelP v8model, WCharCP modelrefName, DgnV8Api::DgnAttachment* &attachment)
    {
    //  Add refV8File as an attachment to v8File
    Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(attachmentFileName.c_str());
    ASSERT_EQ( BentleyApi::SUCCESS, v8model->CreateDgnAttachment(attachment, *moniker, modelrefName, true));
    ASSERT_TRUE(nullptr != attachment);        
        attachment->SetNestDepth(99);
    ASSERT_EQ( BentleyApi::SUCCESS, attachment->WriteToModel());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId FindElementByCodeValue(DgnDbR db, Utf8CP className , Utf8CP codeValue)
    {
    DgnClassId classId= db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, className);
    auto stmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId from " BIS_SCHEMA(BIS_CLASS_Element) " WHERE ECClassId=? AND CodeValue=?");
    stmt->BindId(1, classId);
    stmt->BindText(2, codeValue, BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnElementId();
    return stmt->GetValueId<DgnElementId>(0);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DrawingTests, Basic3dAttachment)
    {
    LineUpFiles(L"Basic3dAttachment.bim", L"Test3d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ( 0 , m_count ) << L"The initial V8 file is supposed to be empty!";

    DgnV8Api::ElementId lineElementId;
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;

        // Put a line in the 3D model
        Bentley::DgnModelP threeDModel = v8editor.m_defaultModel;
        ASSERT_TRUE(threeDModel->Is3d());
        v8editor.AddLine(&lineElementId, threeDModel);

        // Create a drawing model ...
        Bentley::DgnModelP drawingModel = v8editor.m_file->CreateNewModel(&modelStatus, L"Drawing1", DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);

        // ... and attach the 3D model as a reference to the new drawing model
        Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(m_v8FileName.c_str());
        DgnV8Api::DgnAttachment* attachment;
        ASSERT_EQ(BentleyApi::SUCCESS, drawingModel->CreateDgnAttachment(attachment, *moniker, threeDModel->GetModelName(), true));
        attachment->SetNestDepth(99);
        ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());
        v8editor.Save();

        //  Default (3D)
        //      ^
        //      DgnAttachment
        //      |
        //  Drawing1
        }

    DoConvert(m_dgnDbFileName, m_v8FileName); // creates Basic3dAttachment.bim from Test3d.dgn
    if (true)
        {
        //  Results of conversion:
        //  Root
        //                              DictionaryModel
        //                                  Default Views - View 1
        //      RealityDataSourcesPartition
        //          ...             <-- 2 PhysicalModels
        //      PhysicalPartition   <-- PhysicalModel {1 PhysicalObject}
        //      "Converted Drawings"
        //          Drawing1        <-- DrawingModel  {1 DrawingGraphic (copy of PhysicalObject)}

        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        countModels(*db, 1, 2); // expect 1 drawing model and 2 physical models (includes the default WebMercatorModel)

        // There should be 1 spatial (orthographic) view
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_SpatialViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DisplayStyle3d), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_ModelSelector), 1);

        // The PhysicalPartition should lead to the converted 3D model, which contains a line.
        auto physical = db->Elements().Get<PhysicalPartition>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_PhysicalPartition)));
        ASSERT_TRUE(physical.IsValid());
        auto physicalModel = physical->GetSub<PhysicalModel>();
        ASSERT_TRUE(physicalModel.IsValid());
        countElements(*physicalModel, 1);
        
        // The converted DrawingModel should contain a 2D copy of that line. (3D attachments are merged into drawings.)
        auto drawing = db->Elements().Get<Drawing>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_Drawing)));
        ASSERT_TRUE(drawing.IsValid());
        auto drawingModel = drawing->GetSub<DrawingModel>();
        ASSERT_TRUE(drawingModel.IsValid());
        countElements(*drawingModel, 1);
        }
    
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;

        // Create a sheet model ...
        Bentley::DgnModelP sheetModel = v8editor.m_file->CreateNewModel(&modelStatus, L"Sheet1", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);

        // ... and attach the drawing as a reference to the new sheet model
        Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(m_v8FileName.c_str());
        DgnV8Api::DgnAttachment* attachment;
        ASSERT_EQ(BentleyApi::SUCCESS, sheetModel->CreateDgnAttachment(attachment, *moniker, L"Drawing1", true));
        attachment->SetNestDepth(99);
        ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());
        v8editor.Save();

        //  Default (3D)
        //      ^
        //      DgnAttachment
        //      |
        //  Drawing1
        //      ^
        //      DgnAttachment
        //      |
        //  Sheet1
        }

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    //ASSERT_TRUE(m_count >= 2) << "Should have inserted a sheet and generated a view of the drawing";
    if (true)
        {
        //  Results of conversion:
        //  Root
        //                              DictionaryModel
        //                                  "Default Views - View 1" (SpatialViewDefinition)
        //      RealityDataSourcesPartition
        //          ...             <-- 2 PhysicalModels
        //      PhysicalPartition   <-- PhysicalModel {1 PhysicalObject}
        //      "Converted Drawings"
        //          Drawing1        <-- DrawingModel  {1 DrawingGraphic (copy of PhysicalObject)}   <---+
        //      "Converted Sheets"                                                                      |BaseModel
        //          Sheet1          <-- Sheet::Model  {1 Sheet::ViewAttachment --}---------> DrawingViewDefinition


        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        countModels(*db, 2, 2); // expect 2 2d models and 2 physical models (includes WebMercatorModel)

        // There should be 1 spatial view and 1 generated DrawingViewDefinition
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_SpatialViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DrawingViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 2);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DisplayStyle3d), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_ModelSelector), 1);
        }
    
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;

        // Modify the line in the 3D model
        Bentley::DgnModelP threeDModel = v8editor.m_defaultModel;
        threeDModel->FillSections(DgnV8Api::DgnModelSections::Model);
        DgnV8Api::EditElementHandle v8eh(threeDModel->FindByElementId(lineElementId));
        ASSERT_TRUE(v8eh.IsValid());
        auto xlat = Bentley::Transform::From(100, 0, 0);
        v8eh.GetHandler (DgnV8Api::MISSING_HANDLER_PERMISSION_Transform).ApplyTransform (v8eh, DgnV8Api::TransformInfo(xlat));
        v8eh.ReplaceInModel(v8eh.GetElementRef());

        v8editor.Save();
        }

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    ASSERT_TRUE(m_count >= 1) << "The line itself is updated. Note that the converter does not generate proxy graphics, because the 3D attachment is not a section and is not clipped in 3D.";
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        countModels(*db, 2, 2); // still expect 2 2d models and 2 physical model (includes WebMercatorModel)

        // There should still be 1 spatial view and 1 generated DrawingViewDefinition
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_SpatialViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DrawingViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 2);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DisplayStyle3d), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_ModelSelector), 1);

        auto physical = db->Elements().Get<PhysicalPartition>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_PhysicalPartition)));
        ASSERT_TRUE(physical.IsValid());
        auto physicalModel = physical->GetSub<PhysicalModel>();
        ASSERT_TRUE(physicalModel.IsValid());
        countElements(*physicalModel, 1); // there should still only be 1 element in the spatial model

        }
    
    // ASSERT_TRUE(m_count == 0); *** WIP_SHEETS - when we compute proxy graphics, we end up changing the type-100's XAttribute linkages, which causes us to record a changed hash in syncinfo
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DrawingTests, Sheet_SheetAttachment)
    {
    LineUpFiles(L"Sheet_SheetAttachment.bim", L"Test3d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ( 0 , m_count ) << L"The initial V8 file is supposed to be empty!";
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;

        // Create a SheetModel1 ...
        Bentley::DgnModelP SheetModel1 = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet1", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        // Put a line in the 2D model
        DgnV8Api::ElementId eid;
        v8editor.AddLine(&eid, SheetModel1);
        v8editor.AddLine(&eid);
        // Create a SheetModel2 ...
        Bentley::DgnModelP SheetModel2 = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet2", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        DgnV8Api::ElementId eid2;
        v8editor.AddLine(&eid2, SheetModel2);
        v8editor.AddLine(&eid2);
        // Create a SheetModel3 ...
        Bentley::DgnModelP SheetModel3 = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet3", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        DgnV8Api::ElementId eid3;
        v8editor.AddLine(&eid3, SheetModel3);
        v8editor.AddLine(&eid3);
        //attach the 2D sheetmodel2 as a reference to the sheetmodel1
        DgnV8Api::DgnAttachment* attachment = NULL;
        AddAttachment(m_v8FileName, SheetModel1, SheetModel2->GetModelName(), attachment);
        //attach the 2D sheetmodel3 as a reference to the sheetmodel2
        DgnV8Api::DgnAttachment* attachment2 = NULL;
        AddAttachment(m_v8FileName, SheetModel2, SheetModel3->GetModelName(), attachment2);
        v8editor.Save();
        }

    DoConvert(m_dgnDbFileName, m_v8FileName); // creates Sheet_SheetAttachment.bim from Test3d.dgn
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        countModels(*db, 3, 2); // expect 1 sheet model and 1 physical model

        // There should be 1 spatial (orthographic) view
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_SpatialViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DisplayStyle2d), 0);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_ModelSelector), 1);

        auto ele1 = db->Elements().Get<Sheet::Element>(FindElementByCodeValue(*db, BIS_CLASS_Sheet, "sheet1"));
        ASSERT_TRUE(ele1.IsValid());
        Sheet::ModelPtr sheetmodel1 = ele1->GetSub<Sheet::Model>();
        ASSERT_TRUE(sheetmodel1.IsValid());
        countElements(*sheetmodel1, 1);
        countElementsInModelByClass(*sheetmodel1, getBisClassId(*db, "ViewAttachment"), 0);

        auto ele2 = db->Elements().Get<Sheet::Element>(FindElementByCodeValue(*db, BIS_CLASS_Sheet, "sheet2"));
        ASSERT_TRUE(ele2.IsValid());
        Sheet::ModelPtr sheetmodel2 = ele2->GetSub<Sheet::Model>();
        ASSERT_TRUE(sheetmodel2.IsValid());
        countElements(*sheetmodel2, 1);
        countElementsInModelByClass(*sheetmodel2, getBisClassId(*db, "ViewAttachment"), 0);
        auto ele3 = db->Elements().Get<Sheet::Element>(FindElementByCodeValue(*db, BIS_CLASS_Sheet, "sheet3"));
        ASSERT_TRUE(ele3.IsValid());

        Sheet::ModelPtr sheetmodel3 = ele3->GetSub<Sheet::Model>();
        ASSERT_TRUE(sheetmodel3.IsValid());
        countElements(*sheetmodel3, 1);
        countElementsInModelByClass(*sheetmodel3, getBisClassId(*db, "ViewAttachment"), 0);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DrawingTests, SheetProperties)
    {
    LineUpFiles(L"SheetProperties.bim", L"Test3d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ( 0 , m_count ) << L"The initial V8 file is supposed to be empty!";
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;
        DgnV8Api::ElementId eid;
        // Create a drawing1 model ...
        Bentley::DgnModelP drawingModel = v8editor.m_file->CreateNewModel(&modelStatus, L"Drawing1", DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        v8editor.AddLine(&eid, drawingModel);
        v8editor.AddLine(&eid);
        // Create a SheetModel1 ...
        Bentley::DgnModelP SheetModel = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet1", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        // and attach the 2D drawing as a reference to the 2D sheet1 
        DgnV8Api::DgnAttachment* attachment = NULL;
        AddAttachment(m_v8FileName, SheetModel, L"Drawing1", attachment);
        DgnV8Api::ModelId SheetmodelId = v8editor.m_file->FindModelIdByName(L"sheet1");
        ASSERT_TRUE(SheetmodelId != NULL);
        SheetModel = v8editor.m_file->LoadModelById(SheetmodelId).get();
        Bentley::DgnPlatform::ModelInfoPtr  SheetModelifo = SheetModel->GetModelInfo().MakeCopy();
        SheetDefP sheetdef= SheetModelifo->GetSheetDefP();
        ASSERT_TRUE(sheetdef !=NULL);
        sheetdef->SetSize(2000000,2000000);
        double x,y;
        sheetdef->GetSize(x, y);
        ASSERT_EQ(x,2000000);
        ASSERT_EQ(y,2000000);
        SheetModel->SetModelInfo(*SheetModelifo);
        v8editor.Save();
        // Put a line in the 2D model
        v8editor.AddLine(&eid, SheetModel);
        v8editor.AddLine(&eid);
        v8editor.AddLine(&eid, SheetModel,DPoint3d::From(10,10,0));
        v8editor.AddLine(&eid);
        v8editor.Save();
        }

    DoConvert(m_dgnDbFileName, m_v8FileName); // creates SheetProperties.bim from Test3d.dgn
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        countModels(*db, 2, 2); // expect 1 sheet model and 2 physical model (includes WebMercatorModel)

        // There should be 1 spatial (orthographic) view 
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_SpatialViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DrawingViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 2);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DisplayStyle2d), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_ModelSelector), 1);

        Sheet::ElementCPtr sheet = db->Elements().Get<Sheet::Element>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_Sheet)));
        ASSERT_TRUE(sheet.IsValid());
        Sheet::ModelPtr sheetModel = sheet->GetSub<Sheet::Model>();
        ASSERT_TRUE(sheetModel.IsValid());
        //Count elements on sheet 
        countElements(*sheetModel, 3);
        ASSERT_EQ(2,sheet->GetWidth());
        ASSERT_EQ(2,sheet->GetHeight());
        ASSERT_EQ(1,sheet->GetScale());
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DrawingTests, SheetScale_WithMultiAttachmentOfSameStoredScale)
    {
    LineUpFiles(L"SheetScale.bim", L"Test3d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ( 0 , m_count ) << L"The initial V8 file is supposed to be empty!";
    DgnV8Api::ElementId eid;
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;

        // Put a line in the 3D model
        Bentley::DgnModelP threeDModel = v8editor.m_defaultModel;
        ASSERT_TRUE(threeDModel->Is3d());
        v8editor.AddLine(&eid, threeDModel);

        // Create a drawing1 model ...
        Bentley::DgnModelP drawingModel = v8editor.m_file->CreateNewModel(&modelStatus, L"Drawing1", DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        v8editor.AddLine(nullptr, drawingModel, DPoint3d::From(0.1, 0.1, 0.1)); // note: must add at least one element to the drawing, or else the converter will notice that it is empty and not create an attachment to it from the sheet.

        // Create a drawing2 model ...
        Bentley::DgnModelP drawingModel2 = v8editor.m_file->CreateNewModel(&modelStatus, L"Drawing2", DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        v8editor.AddLine(nullptr, drawingModel2, DPoint3d::From(0.1, 0.1, 0.1));

        DgnV8Api::DgnAttachment* attachment1 = NULL;
        // and attach the 3D model as a reference to the new drawing1 model
        AddAttachment(m_v8FileName, drawingModel, threeDModel->GetModelName(), attachment1);
        // and attach the 3D model as a reference to the new drawing2 model
        DgnV8Api::DgnAttachment* attachment2 = NULL;
        AddAttachment(m_v8FileName, drawingModel2, threeDModel->GetModelName(), attachment2);
 
        // Create a SheetModel1 ...
        Bentley::DgnModelP SheetModel = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet1", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        // and attach the 2D drawing1 as a reference to the 2D sheet1 
        DgnV8Api::DgnAttachment* attachment3 = NULL;
        AddAttachment(m_v8FileName, SheetModel, L"Drawing1", attachment3);
        attachment3->SetScaleByUnits(true);
        ASSERT_EQ(true, attachment3->IsScaleByUnits());
        attachment3->SetStoredScale(100);
        ASSERT_EQ(100, attachment3->GetScaleStored());
        ASSERT_EQ(BentleyApi::SUCCESS, attachment3->WriteToModel());
        // and attach the 2D drawing2 as a reference to the 2D sheet1 
        DgnV8Api::DgnAttachment* attachment4 = NULL;
        AddAttachment(m_v8FileName, SheetModel, L"Drawing2", attachment4);
        attachment4->SetScaleByUnits(true);
        ASSERT_EQ(true, attachment4->IsScaleByUnits());
        attachment4->SetStoredScale(100);
        ASSERT_EQ(100, attachment4->GetScaleStored());
        ASSERT_EQ(BentleyApi::SUCCESS, attachment4->WriteToModel());

        DgnV8Api::ModelId SheetmodelId = v8editor.m_file->FindModelIdByName(L"sheet1");
        ASSERT_TRUE(SheetmodelId != NULL);
        SheetModel = v8editor.m_file->LoadModelById(SheetmodelId).get();
        Bentley::DgnPlatform::ModelInfoPtr  SheetModelifo = SheetModel->GetModelInfo().MakeCopy();
        SheetDefP sheetdef= SheetModelifo->GetSheetDefP();
        ASSERT_TRUE(sheetdef !=NULL);
        sheetdef->SetSize(2000000,2000000);
        double x,y;
        sheetdef->GetSize(x, y);
        ASSERT_EQ(x,2000000);
        ASSERT_EQ(y,2000000);
        SheetModel->SetModelInfo(*SheetModelifo);
        v8editor.Save();
        // Put a line in the 2D model
        v8editor.AddLine(&eid, SheetModel, DPoint3d::From(0.1, 0.1, 0.1));
        v8editor.Save();
        }

    DoConvert(m_dgnDbFileName, m_v8FileName); // creates SheetProperties.bim from Test3d.dgn
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        countModels(*db, 3, 2);

        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_SpatialViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DrawingViewDefinition), 2);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 3);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DisplayStyle2d), 2);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_ModelSelector), 1);

        Sheet::ElementCPtr sheet = db->Elements().Get<Sheet::Element>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_Sheet)));
        ASSERT_TRUE(sheet.IsValid());
        Sheet::ModelPtr sheetModel = sheet->GetSub<Sheet::Model>();
        ASSERT_TRUE(sheetModel.IsValid());
        //Count elements on sheet 
        countElements(*sheetModel, 3); // 3 ViewAttachments + 1 line
        ASSERT_EQ(2,sheet->GetWidth());
        ASSERT_EQ(2,sheet->GetHeight());
        ASSERT_EQ(0.01,sheet->GetScale());
        countElementsInModelByClass(*sheetModel, getBisClassId(*db, "ViewAttachment"), 2);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DrawingTests, SheetScale_WithMultiAttachmentOfDiffStoredScale)
    {
    LineUpFiles(L"SheetScale.bim", L"Test3d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ( 0 , m_count ) << L"The initial V8 file is supposed to be empty!";
    DgnV8Api::ElementId eid;
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;

        // Put a line in the 3D model
        Bentley::DgnModelP threeDModel = v8editor.m_defaultModel;
        ASSERT_TRUE(threeDModel->Is3d());
        v8editor.AddLine(&eid, threeDModel);

        // Create a drawing1 model ...
        Bentley::DgnModelP drawingModel = v8editor.m_file->CreateNewModel(&modelStatus, L"Drawing1", DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        v8editor.AddLine(nullptr, drawingModel);    // note: must add at least one element to the drawing, or else the converter will notice that it is empty and not create an attachment to it from the sheet.
        // Create a drawing2 model ...
        Bentley::DgnModelP drawingModel2 = v8editor.m_file->CreateNewModel(&modelStatus, L"Drawing2", DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        v8editor.AddLine(nullptr, drawingModel2);

        // Create a drawing3 model ...
        Bentley::DgnModelP drawingModel3 = v8editor.m_file->CreateNewModel(&modelStatus, L"Drawing3", DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        v8editor.AddLine(nullptr, drawingModel3);

        DgnV8Api::DgnAttachment* attachment1 = NULL;
        // and attach the 3D model as a reference to the new drawing1 model
        AddAttachment(m_v8FileName, drawingModel, threeDModel->GetModelName(),attachment1);
        DgnV8Api::DgnAttachment* attachment2 = NULL;
        // and attach the 3D model as a reference to the new drawing2 model
        AddAttachment(m_v8FileName, drawingModel2, threeDModel->GetModelName(),attachment2);
        DgnV8Api::DgnAttachment* attachment3 = NULL;
        // and attach the 3D model as a reference to the new drawing3 model
        AddAttachment(m_v8FileName, drawingModel3, threeDModel->GetModelName(),attachment3);

        // Create a SheetModel1 ...
        Bentley::DgnModelP SheetModel = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet1", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        // and attach the 2D drawing1 as a reference to the 2D sheet1 
        DgnV8Api::DgnAttachment* attachment4 = NULL;
        AddAttachment(m_v8FileName, SheetModel, L"Drawing1", attachment4);
        attachment4->SetScaleByUnits(true);
        ASSERT_EQ(true, attachment4->IsScaleByUnits());
        attachment4->SetStoredScale(20);
        ASSERT_EQ(20, attachment4->GetScaleStored());
        ASSERT_EQ(BentleyApi::SUCCESS, attachment4->WriteToModel());
        // and attach the 2D drawing2 as a reference to the 2D sheet1 
        DgnV8Api::DgnAttachment* attachment5 = NULL;
        AddAttachment(m_v8FileName, SheetModel, L"Drawing2", attachment5);
        attachment5->SetScaleByUnits(true);
        ASSERT_EQ(true, attachment5->IsScaleByUnits());
        attachment5->SetStoredScale(20);
        ASSERT_EQ(20, attachment5->GetScaleStored());
        ASSERT_EQ(BentleyApi::SUCCESS, attachment5->WriteToModel());
        DgnV8Api::DgnAttachment* attachment6 = NULL;
        // and attach the 2D drawing3 as a reference to the 2D sheet1 
        AddAttachment(m_v8FileName, SheetModel, L"Drawing3", attachment6);
        attachment6->SetScaleByUnits(true);
        ASSERT_EQ(true, attachment6->IsScaleByUnits());
        attachment6->SetStoredScale(100);
        ASSERT_EQ(100, attachment6->GetScaleStored());
        ASSERT_EQ(BentleyApi::SUCCESS, attachment6->WriteToModel());

        DgnV8Api::ModelId SheetmodelId = v8editor.m_file->FindModelIdByName(L"sheet1");
        ASSERT_TRUE(SheetmodelId != NULL);
        SheetModel = v8editor.m_file->LoadModelById(SheetmodelId).get();
        Bentley::DgnPlatform::ModelInfoPtr  SheetModelifo = SheetModel->GetModelInfo().MakeCopy();
        SheetDefP sheetdef= SheetModelifo->GetSheetDefP();
        ASSERT_TRUE(sheetdef !=NULL);
        sheetdef->SetSize(2000000,2000000);
        double x,y;
        sheetdef->GetSize(x, y);
        ASSERT_EQ(x,2000000);
        ASSERT_EQ(y,2000000);
        SheetModel->SetModelInfo(*SheetModelifo);
        // Put a line in the 2D model
        v8editor.AddLine(&eid, SheetModel, DPoint3d::From(0.1, 0.1, 0.1));
        // Create a SheetModel2 ...
        Bentley::DgnModelP SheetModel2 = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet2", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        // and attach the 2D drawing1 as a reference to the 2D sheet2 
        DgnV8Api::DgnAttachment* attachment7 = NULL;
        AddAttachment(m_v8FileName, SheetModel2, L"Drawing1",attachment7);
        attachment7->SetScaleByUnits(true);
        ASSERT_EQ(true, attachment7->IsScaleByUnits());
        attachment7->SetStoredScale(0.1);
        ASSERT_EQ(0.1, attachment7->GetScaleStored());
        ASSERT_EQ(BentleyApi::SUCCESS, attachment7->WriteToModel());
        // and attach the 2D drawing2 as a reference to the 2D sheet2 
        DgnV8Api::DgnAttachment* attachment8 = NULL;
        AddAttachment(m_v8FileName, SheetModel2, L"Drawing2", attachment8);
        attachment8->SetScaleByUnits(true);
        ASSERT_EQ(true, attachment8->IsScaleByUnits());
        attachment8->SetStoredScale(0.1);
        ASSERT_EQ(0.1, attachment8->GetScaleStored());
        ASSERT_EQ(BentleyApi::SUCCESS, attachment8->WriteToModel());
        // and attach the 2D drawing3 as a reference to the 2D sheet2 
        DgnV8Api::DgnAttachment* attachment9 = NULL;
        AddAttachment(m_v8FileName, SheetModel2, L"Drawing3", attachment9);
        attachment9->SetScaleByUnits(true);
        ASSERT_EQ(true, attachment9->IsScaleByUnits());
        attachment9->SetStoredScale(0.2);
        ASSERT_EQ(0.2, attachment9->GetScaleStored());
        ASSERT_EQ(BentleyApi::SUCCESS, attachment9->WriteToModel());

        DgnV8Api::ModelId SheetmodelId2 = v8editor.m_file->FindModelIdByName(L"sheet2");
        ASSERT_TRUE(SheetmodelId2 != NULL);
        SheetModel2 = v8editor.m_file->LoadModelById(SheetmodelId2).get();
        Bentley::DgnPlatform::ModelInfoPtr  SheetModelifo2 = SheetModel2->GetModelInfo().MakeCopy();
        SheetDefP sheetdef2 = SheetModelifo2->GetSheetDefP();
        ASSERT_TRUE(sheetdef2 != NULL);
        sheetdef2->SetSize(2000000, 2000000);
        sheetdef2->GetSize(x, y);
        ASSERT_EQ(x, 2000000);
        ASSERT_EQ(y, 2000000);
        SheetModel2->SetModelInfo(*SheetModelifo2);
        // Put a line in the 2D model
        v8editor.AddLine(&eid, SheetModel2, DPoint3d::From(0.1, 0.1, 0.1));
        v8editor.AddLine(&eid);
        v8editor.Save();
        }

    DoConvert(m_dgnDbFileName, m_v8FileName); // creates SheetProperties.bim from Test3d.dgn
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        countModels(*db, 5, 2);

        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_SpatialViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DrawingViewDefinition), 6);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 7);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DisplayStyle2d), 6);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_ModelSelector), 1);

        auto ele1 = db->Elements().Get<Sheet::Element>(FindElementByCodeValue(*db, BIS_CLASS_Sheet, "sheet1"));
        ASSERT_TRUE(ele1.IsValid());
        Sheet::ModelPtr sheetmodel1 = ele1->GetSub<Sheet::Model>();
        ASSERT_TRUE(sheetmodel1.IsValid());
        Sheet::ElementCPtr sheet1 = db->Elements().Get<Sheet::Element>(sheetmodel1->GetModeledElementId());
        ASSERT_TRUE(sheet1.IsValid());
        //Count elements on sheet 
        countElements(*sheetmodel1, 4); 
        ASSERT_EQ(2,sheet1->GetWidth());
        ASSERT_EQ(2,sheet1->GetHeight());
        ASSERT_EQ(0.05,sheet1->GetScale());
        countElementsInModelByClass(*sheetmodel1, getBisClassId(*db, "ViewAttachment"), 3);
        auto ele2 = db->Elements().Get<Sheet::Element>(FindElementByCodeValue(*db, BIS_CLASS_Sheet, "sheet2"));
        ASSERT_TRUE(ele2.IsValid());
        Sheet::ModelPtr sheetmodel2 = ele2->GetSub<Sheet::Model>();
        ASSERT_TRUE(sheetmodel2.IsValid());
        Sheet::ElementCPtr sheet2 = db->Elements().Get<Sheet::Element>(sheetmodel2->GetModeledElementId());
        ASSERT_TRUE(sheet2.IsValid());
        //Count elements on sheet 
        countElements(*sheetmodel2, 4);
        ASSERT_EQ(2, sheet2->GetWidth());
        ASSERT_EQ(2, sheet2->GetHeight());
        //when relationshipfound the scale is
        ASSERT_EQ(10, sheet2->GetScale());
        countElementsInModelByClass(*sheetmodel2, getBisClassId(*db, "ViewAttachment"), 3);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DrawingTests, Attach3dmodeltoSheet)
    {
    LineUpFiles(L"Attach3dmodeltoSheet.bim", L"Test3d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";
    DgnV8Api::ElementId eid;
    if (true)
       {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;
        // Put a line in the 3D model
        Bentley::DgnModelP threeDModel = v8editor.m_defaultModel;
        ASSERT_TRUE(threeDModel->Is3d());
        v8editor.AddLine(&eid, threeDModel);
        // Create a SheetModel1 ...
        Bentley::DgnModelP SheetModel = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet1", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        DgnV8Api::DgnAttachment* attachment = NULL;
        // and attach the 3D model as a reference to the 2D sheet1 
        AddAttachment(m_v8FileName, SheetModel, threeDModel->GetModelName(), attachment);
        v8editor.Save();
        }
    DoConvert(m_dgnDbFileName, m_v8FileName); 
    if (true)
       {

        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        countModels(*db, 2, 2);

        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_SpatialViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 2);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DisplayStyle3d), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_ModelSelector), 1);

        // The PhysicalPartition should lead to the converted 3D model, which contains a line.
        auto physical = db->Elements().Get<PhysicalPartition>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_PhysicalPartition)));
        ASSERT_TRUE(physical.IsValid());
        auto physicalModel = physical->GetSub<PhysicalModel>();
        ASSERT_TRUE(physicalModel.IsValid());
        countElements(*physicalModel, 1);

        // There should be 1 sheet with 1 attachments
        auto sheet = db->Elements().Get<Sheet::Element>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_Sheet)));
        ASSERT_TRUE(sheet.IsValid());
        auto sheetModel = sheet->GetSub<Sheet::Model>();
        ASSERT_TRUE(sheetModel.IsValid());
        countElements(*sheetModel, 1);
        countElementsInModelByClass(*sheetModel, getBisClassId(*db, "ViewAttachment"), 1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DrawingTests, BorderAttachmenttoSheet)
    {
    LineUpFiles(L"BorderAttachmenttoSheet.bim", L"Test3d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";
    DgnV8Api::ElementId eid;
    m_wantCleanUp = false;
    if (true)
       {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;
        // Put a line in the 3D model
        Bentley::DgnModelP threeDModel = v8editor.m_defaultModel;
        ASSERT_TRUE(threeDModel->Is3d());
        v8editor.AddLine(&eid, threeDModel);
        // Create a SheetModel1 ...
        Bentley::DgnModelP SheetModel = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet1", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        Bentley::DgnPlatform::ModelInfoPtr  SheetModelifo = SheetModel->GetModelInfo().MakeCopy();
        SheetDefP sheetdef = SheetModelifo->GetSheetDefP();
        // and attach the 3D model as a reference to the new 2D sheet1 
        DgnV8Api::DgnAttachment* attachmentToBorder = NULL;
        AddAttachment(m_v8FileName, SheetModel, threeDModel->GetModelName(), attachmentToBorder);

        ASSERT_EQ(INVALID_ELEMENTID, sheetdef->GetBorderAttachmentId());
        sheetdef->SetBorderAttachmentId(attachmentToBorder->GetElementId());
        ASSERT_EQ(SUCCESS, SheetModel->SetModelInfo(*SheetModelifo));
        EXPECT_EQ(attachmentToBorder->GetElementId(), SheetModel->GetModelInfo().GetSheetDefCP()->GetBorderAttachmentId()); // make sure border attachment id propagates to the model.
        
        // Create a drawing1 model ...
        Bentley::DgnModelP drawingModel = v8editor.m_file->CreateNewModel(&modelStatus, L"Drawing1", DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        v8editor.AddLine(nullptr, drawingModel); // must put something in drawingmodel, or converter won't create an attachment of it to the sheet

        // and attach the the default 3d model as a reference to drawingModel
        DgnV8Api::DgnAttachment* attachment = NULL;
        AddAttachment(m_v8FileName, drawingModel, threeDModel->GetModelName(), attachment);
        // Create a SheetModel2...
        Bentley::DgnModelP SheetModel2 = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet2", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        // and attach the Drawing model as a reference to the new 2D sheet2 
        DgnV8Api::DgnAttachment* attachmentToBorder2 = NULL;
        AddAttachment(m_v8FileName, SheetModel2, L"Drawing1", attachmentToBorder2);

        Bentley::DgnPlatform::ModelInfoPtr  SheetModel2ifo = SheetModel2->GetModelInfo().MakeCopy();
        SheetDefP sheetdef2 = SheetModel2ifo->GetSheetDefP();
        ASSERT_EQ(INVALID_ELEMENTID, sheetdef2->GetBorderAttachmentId());
        sheetdef2->SetBorderAttachmentId(attachmentToBorder2->GetElementId());
        ASSERT_EQ(SUCCESS, SheetModel2->SetModelInfo(*SheetModel2ifo));
        EXPECT_EQ(attachmentToBorder2->GetElementId(), SheetModel2->GetModelInfo().GetSheetDefCP()->GetBorderAttachmentId()); // make sure border attachment id propagates to the model.

        v8editor.Save();
        }
    DoConvert(m_dgnDbFileName, m_v8FileName); 
    if (true)
       {

        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        countModels(*db, 4, 2);
        db->Schemas().CreateClassViewsInDb();
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_SpatialViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 3);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DisplayStyle3d), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DrawingViewDefinition), 2);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_ModelSelector), 1);

        // The PhysicalPartition should lead to the converted 3D model, which contains a line.
        auto physical = db->Elements().Get<PhysicalPartition>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_PhysicalPartition)));
        ASSERT_TRUE(physical.IsValid());
        auto physicalModel = physical->GetSub<PhysicalModel>();
        ASSERT_TRUE(physicalModel.IsValid());
        countElements(*physicalModel, 1);

        auto drawing = db->Elements().Get<Drawing>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_Drawing)));
        ASSERT_TRUE(drawing.IsValid());
        auto drawingModel = drawing->GetSub<DrawingModel>();
        ASSERT_TRUE(drawingModel.IsValid());
        countElements(*drawingModel, 2);


        auto ele1 = db->Elements().Get<Sheet::Element>(FindElementByCodeValue(*db, BIS_CLASS_Sheet, "sheet1"));
        ASSERT_TRUE(ele1.IsValid());
        Sheet::ModelPtr sheetmodel1 = ele1->GetSub<Sheet::Model>();
        ASSERT_TRUE(sheetmodel1.IsValid());
        countElements(*sheetmodel1, 1);
        countElementsInModelByClass(*sheetmodel1, getBisClassId(*db, "ViewAttachment"), 1);

        auto ele2 = db->Elements().Get<Sheet::Element>(FindElementByCodeValue(*db, BIS_CLASS_Sheet, "sheet2"));
        ASSERT_TRUE(ele2.IsValid());
        Sheet::ModelPtr sheetmodel2 = ele2->GetSub<Sheet::Model>();
        countElements(*sheetmodel2, 1);
        countElementsInModelByClass(*sheetmodel2, getBisClassId(*db, "ViewAttachment"), 1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DrawingTests, AttachDwg)
    {
    LineUpFiles(L"Dwg.bim", L"Test3d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ( 0 , m_count ) << L"The initial V8 file is supposed to be empty!";
    DgnV8Api::ElementId eid;
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;
        BentleyApi::BeFileName InputFileName = GetInputFileName(L"basictype.dwg");
        BentleyApi::BeFileName OutputFileName = GetOutputFileName(L"basictype.dwg");
        MakeWritableCopyOf(OutputFileName, InputFileName, InputFileName.GetFileNameAndExtension().c_str());
        // Put a line in the 3D model
        Bentley::DgnModelP threeDModel = v8editor.m_defaultModel;
        ASSERT_TRUE(threeDModel->Is3d());
        v8editor.AddLine(&eid, threeDModel);
        // and attach the model from dwg file as a reference to the 3d model in dgn file
        DgnV8Api::DgnAttachment* attachment1 = NULL;
        AddAttachment(OutputFileName, threeDModel, L"Model", attachment1);
        // Create a SheetModel1 ...
        Bentley::DgnModelP SheetModel = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet1", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        // and attach the 3D model as a reference to the 2D sheet1 
        DgnV8Api::DgnAttachment* attachment2 = NULL;
        AddAttachment(m_v8FileName, SheetModel, threeDModel->GetModelName(), attachment2);
        v8editor.Save();
        }

    DoConvert(m_dgnDbFileName, m_v8FileName); 

    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        countModels(*db, 6, 3);

        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_SpatialViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DrawingViewDefinition), 3);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 6);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_ModelSelector), 1);
        // 2 sheets from DWG file and 1 sheet that we created in dgn fle
        auto ele1 = db->Elements().Get<Sheet::Element>(FindElementByCodeValue(*db, BIS_CLASS_Sheet, "sheet1"));
        ASSERT_TRUE(ele1.IsValid());
        Sheet::ModelPtr sheetmodel1 = ele1->GetSub<Sheet::Model>();
        ASSERT_TRUE(sheetmodel1.IsValid());
        countElementsInModelByClass(*sheetmodel1, getBisClassId(*db, "ViewAttachment"), 1);
        auto ele2 = db->Elements().Get<Sheet::Element>(FindElementByCodeValue(*db, BIS_CLASS_Sheet, "Layout1"));
        ASSERT_TRUE(ele2.IsValid());
        Sheet::ModelPtr sheetmodel2 = ele2->GetSub<Sheet::Model>();
        ASSERT_TRUE(sheetmodel2.IsValid());
        countElementsInModelByClass(*sheetmodel2, getBisClassId(*db, "ViewAttachment"), 1);
        auto ele3 = db->Elements().Get<Sheet::Element>(FindElementByCodeValue(*db, BIS_CLASS_Sheet, "Layout2"));
        ASSERT_TRUE(ele3.IsValid());
        Sheet::ModelPtr sheetmodel3 = ele3->GetSub<Sheet::Model>();
        ASSERT_TRUE(sheetmodel3.IsValid());
        countElementsInModelByClass(*sheetmodel3, getBisClassId(*db, "ViewAttachment"), 1);
        BentleyApi::Bstdcxx::bvector<DgnModelId>idlist;
        idlist = db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalModel), nullptr, "ORDER BY ECInstanceId ASC").BuildIdList();
        ASSERT_EQ(2, idlist.size());
        auto physicalmodel1 = db->Models().GetModel(idlist[0]);
        ASSERT_TRUE(physicalmodel1.IsValid());
        //Count elements
        countElements(*physicalmodel1, 1);
        auto physicalmodel2 = db->Models().GetModel(idlist[1]);
        ASSERT_TRUE(physicalmodel2.IsValid());
        //Count elements in DWG model 
        countElements(*physicalmodel2, 5);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DrawingTests, AttachNameViewtoSheet)
    {
    LineUpFiles(L"Attach3dmodeltoSheet.bim", L"Test3d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";
    DgnV8Api::ElementId eid;
    if (true)
       {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;
        // Put a line in the 3D model
        Bentley::DgnModelP threeDModel = v8editor.m_defaultModel;
        ASSERT_TRUE(threeDModel->Is3d());
        v8editor.AddLine(&eid, threeDModel);
        // Create a SheetModel1 ...
        Bentley::DgnModelP SheetModel = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet1", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        DgnV8Api::DgnAttachment* attachment = NULL;
        // and attach the 3D model as a reference to the 2D sheet1 
        Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(m_v8FileName.c_str());
        ASSERT_EQ(BentleyApi::SUCCESS, SheetModel->CreateDgnAttachment(attachment, *moniker, threeDModel->GetModelName(), true));
        ASSERT_TRUE(nullptr != attachment);
        attachment->SetNestDepth(99);

        DgnV8Api::NamedViewPtr    namedView;
        EXPECT_TRUE(DgnV8Api::NamedViewStatus::Success == DgnV8Api::NamedView::Create(namedView, *v8editor.m_file, L"Test View"));
        EXPECT_EQ(true, namedView.IsValid());
        EXPECT_TRUE(DgnV8Api::NamedViewStatus::Success == namedView->WriteToFile());
        RotMatrix I = RotMatrix::FromIdentity();
        attachment->SetRotMatrix(RotMatrix::FromAxisAndRotationAngle(2, msGeomConst_piOver2));

        ASSERT_EQ(threeDModel->GetModelName(), attachment->GetModelNameCP());
        ASSERT_TRUE(!attachment->IsClipped());
        ASSERT_TRUE(!attachment->GetRotMatrix().IsEqual(I));

        ASSERT_TRUE(SUCCESS == attachment->ApplyNamedView(L"Test View", 1.0));
        ASSERT_TRUE(!attachment->GetRotMatrix().IsEqual(I));
        ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());
        v8editor.Save();
        }
    DoConvert(m_dgnDbFileName, m_v8FileName);
    if (true)
       {

        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        countModels(*db, 2, 2);

        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_SpatialViewDefinition), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 2);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DisplayStyle3d), 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_ModelSelector), 1);

        // The PhysicalPartition should lead to the converted 3D model, which contains a line.
        auto physical = db->Elements().Get<PhysicalPartition>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_PhysicalPartition)));
        ASSERT_TRUE(physical.IsValid());
        auto physicalModel = physical->GetSub<PhysicalModel>();
        ASSERT_TRUE(physicalModel.IsValid());
        countElements(*physicalModel, 1);

        // There should be 1 sheet with 1 attachments
        auto sheet = db->Elements().Get<Sheet::Element>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_Sheet)));
        ASSERT_TRUE(sheet.IsValid());
        auto sheetModel = sheet->GetSub<Sheet::Model>();
        ASSERT_TRUE(sheetModel.IsValid());
        countElements(*sheetModel, 0);
        countElementsInModelByClass(*sheetModel, getBisClassId(*db, "ViewAttachment"), 0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DrawingTests, MultipleSheets_WithNoAttachedModels)
    {
    LineUpFiles(L"MultipleSheet.bim", L"Test3d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";
    DgnV8Api::ElementId eid;
    if (true)
       {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;

        // Create a SheetModel1 ...
        Bentley::DgnModelP SheetModel1 = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet1", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        // Create a SheetModel2 ...
        Bentley::DgnModelP SheetModel2 = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet2", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        // Create a SheetModel3 ...
        Bentley::DgnModelP SheetModel3 = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet3", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        v8editor.Save();
        }
    DoConvert(m_dgnDbFileName, m_v8FileName);
    if (true)
       {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        countModels(*db, 3, 2);
        ASSERT_EQ(3 , db->Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Sheet)).BuildIdList<DgnElementId>().size());
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DrawingTests, Attachments_With2dRootModel)
    {
    LineUpFiles(L"Attachments_With2dRootModel.bim", L"Test2d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";
    m_wantCleanUp = false;
    DgnV8Api::ElementId eid;
    if (true)
       {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;
        Bentley::DgnModelP twoDModel = v8editor.m_defaultModel;

        // Create a DrawingModel1 ...
        Bentley::DgnModelP DrawingModel1 = v8editor.m_file->CreateNewModel(&modelStatus, L"drawing1", DgnV8Api::DgnModelType::Drawing, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        v8editor.AddLine(&eid, DrawingModel1);
        // Create a DrawingModel2 ...
        Bentley::DgnModelP DrawingModel2 = v8editor.m_file->CreateNewModel(&modelStatus, L"drawing2", DgnV8Api::DgnModelType:: Drawing, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        DgnV8Api::EditElementHandle eeh;
        v8editor.CreateArc(eeh ,TRUE ,DrawingModel2);
        // Create a DrawingModel3 ...
        Bentley::DgnModelP DrawingModel3 = v8editor.m_file->CreateNewModel(&modelStatus, L"drawing3", DgnV8Api::DgnModelType::Drawing, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        v8editor.AddLine(&eid, DrawingModel3);
        // and attach the 2D drawing1 as a reference to the default 2d rootmodel 
        DgnV8Api::DgnAttachment* attachment1 = NULL;
        AddAttachment(m_v8FileName, twoDModel, L"drawing1" ,attachment1);
        attachment1->SetNestDepth(1);
        ASSERT_EQ(BentleyApi::SUCCESS, attachment1->WriteToModel());
        // and attach the 2D drawing1 as a reference to the default 2d rootmodel 
        DgnV8Api::DgnAttachment* attachment2 = NULL;
        AddAttachment(m_v8FileName, twoDModel, L"drawing2", attachment2);
        attachment2->SetNestDepth(1);
        ASSERT_EQ(BentleyApi::SUCCESS, attachment2->WriteToModel());
        // and attach the 2D Drawing3 as a reference to the default 2d rootmodel 
        DgnV8Api::DgnAttachment* attachment3 = NULL;
        AddAttachment(m_v8FileName, twoDModel, L"drawing3", attachment3);
        attachment3->SetNestDepth(2);
        ASSERT_EQ(BentleyApi::SUCCESS, attachment3->WriteToModel());
        v8editor.Save();
        }
    DoConvert(m_dgnDbFileName, m_v8FileName);
    if (true)
       {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        countModels(*db, 4, 1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DrawingViewDefinition), 2);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 2);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DisplayStyle2d), 2);

        ASSERT_EQ(4, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_Drawing)));
        ASSERT_EQ(4, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel)));

        DgnElementCPtr ele= db->Elements().GetElement(FindElementByCodeValue(*db, BIS_CLASS_Drawing, "Test2d"));
        ASSERT_TRUE(ele.IsValid());
        auto eleModel = ele->GetSub<DrawingModel>();
        ASSERT_TRUE(eleModel.IsValid());
        countElements(*eleModel, 3);
        //There should be three DrawingGrapic in total maps on on root2dmodel
        countElementsInModelByClass(*eleModel, getBisClassId(*db, "DrawingGraphic"), 3);
      }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DrawingTests, Cyclic2dModels)
    {
    LineUpFiles(L"cyclic2dModels.bim", L"Test2d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";
    m_wantCleanUp = false;
    DgnV8Api::ElementId eid;
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;
        Bentley::DgnModelP twoDModel = v8editor.m_defaultModel;

        // Create a DrawingModel1 ...
        Bentley::DgnModelP DrawingModel1 = v8editor.m_file->CreateNewModel(&modelStatus, L"drawing1", DgnV8Api::DgnModelType::Drawing, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        v8editor.AddLine(&eid, DrawingModel1);

        // and attach the 2D rootmodel as a reference to the 2D Drawing1
        DgnV8Api::DgnAttachment* attachment1 = NULL;
        AddAttachment(m_v8FileName, DrawingModel1, twoDModel->GetModelName(), attachment1);
        attachment1->SetNestDepth(1);
        ASSERT_EQ(BentleyApi::SUCCESS, attachment1->WriteToModel());
        // and attach the 2D drawing1 as a reference to the 2D rootmodel
        DgnV8Api::DgnAttachment* attachment2 = NULL;
        AddAttachment(m_v8FileName, twoDModel, L"Drawing1", attachment2);
        attachment2->SetNestDepth(1);
        ASSERT_EQ(BentleyApi::SUCCESS, attachment2->WriteToModel());
        v8editor.Save();
        }
    DoConvert(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        db->Schemas().CreateClassViewsInDb();
        countModels(*db, 2, 1);
        ASSERT_EQ(2, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_Drawing)));
        ASSERT_EQ(2, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel)));
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      07/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DrawingTests, Cyclic2dModels_AttachToSheet)
    {
    LineUpFiles(L"Cyclic2dModels_AttachToSheet.bim", L"Test2d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ(0, m_count) << L"The initial V8 file is supposed to be empty!";
    DgnV8Api::ElementId eid;
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;
        Bentley::DgnModelP twoDModel = v8editor.m_defaultModel;
        v8editor.AddLine(&eid, twoDModel);  // make sure the model has something in it. Otherwise, the view will be empty, and the sheet will not create a ViewAttachment to an empty view.

        // Create a drawingModel1 ...
        Bentley::DgnModelP drawingModel1 = v8editor.m_file->CreateNewModel(&modelStatus, L"drawing1", DgnV8Api::DgnModelType::Drawing, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        v8editor.AddLine(&eid, drawingModel1);
        // Create a sheetModel1 ...
        Bentley::DgnModelP sheetModel1 = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet1", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        // and attach the 2D rootmodel as a reference to the 2D Drawing1
        DgnV8Api::DgnAttachment* attachment1 = NULL;
        AddAttachment(m_v8FileName, drawingModel1, twoDModel->GetModelName(), attachment1);
        attachment1->SetNestDepth(1);
        ASSERT_EQ(BentleyApi::SUCCESS, attachment1->WriteToModel());
        // and attach the 2D drawing1 as a reference to the 2D rootmodel
        DgnV8Api::DgnAttachment* attachment2 = NULL;
        AddAttachment(m_v8FileName, twoDModel, L"Drawing1", attachment2);
        attachment2->SetNestDepth(1);
        ASSERT_EQ(BentleyApi::SUCCESS, attachment2->WriteToModel());
        // and attach the 2D rootmodel as a reference to the sheet
        DgnV8Api::DgnAttachment* attachment3 = NULL;
        AddAttachment(m_v8FileName, sheetModel1, twoDModel->GetModelName(), attachment3);
        attachment3->SetNestDepth(1);
        ASSERT_EQ(BentleyApi::SUCCESS, attachment3->WriteToModel());
        v8editor.Save();
        }
    DoConvert(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        // We expect 3 DrawingModels and 1 SheetModel, making four 2-D models in total.
        // The 3 drawings are: "drawing1", the default model, and the sheet's private copy of the default model.
        countModels(*db, 4, -1);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DrawingViewDefinition), 3);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_CategorySelector), 3);
        countElementsInModelByClass(*GetJobDefinitionModel(*db), getBisClassId(*db, BIS_CLASS_DisplayStyle2d), 3);
        ASSERT_EQ(3, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_Drawing)));
        ASSERT_EQ(3, SelectCountFromECClass(*db, BIS_SCHEMA(BIS_CLASS_DrawingModel)));

        // Root model "Test2d". We expect 2 DrawingGraphics.
        // (Why 2? The root model's line, plus the line from "Test2d" that the converter merged in.)
        DgnElementCPtr ele = db->Elements().GetElement(FindElementByCodeValue(*db, BIS_CLASS_Drawing, "Test2d"));
        ASSERT_TRUE(ele.IsValid());
        auto eleModel = ele->GetSub<DrawingModel>();
        ASSERT_TRUE(eleModel.IsValid());
        countElements(*eleModel, 2);
        countElementsInModelByClass(*eleModel, getBisClassId(*db, "DrawingGraphic"), 2);
        
        // Sheet1. We expect 2 Sheet.ViewAttachment elements.
        auto ele1 = db->Elements().Get<Sheet::Element>(FindElementByCodeValue(*db, BIS_CLASS_Sheet, "sheet1"));
        ASSERT_TRUE(ele1.IsValid());
        Sheet::ModelPtr sheetmodel1 = ele1->GetSub<Sheet::Model>();
        countElements(*sheetmodel1, 1);
        countElementsInModelByClass(*sheetmodel1, getBisClassId(*db, "ViewAttachment"), 1);
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct SheetCompositionTests : public ConverterTestBaseFixture
    {
    Bentley::SheetDefCP GetV8SheetDef (Bentley::WString sheetName)
        {
        V8FileEditor v8editor;
        v8editor.Open (m_v8FileName);
        Bentley::DgnPlatform::ModelId sheetModelId = v8editor.m_file->FindModelIdByName (L"Section_Case1");
        Bentley::DgnModelPtr v8Model = v8editor.m_file->LoadModelById (sheetModelId);
        return v8Model->GetModelInfoCP ()->GetSheetDefCP ();
        }

    BentleyApi::Dgn::Sheet::ModelCP GetSheetModel (DgnDbR db, BentleyApi::Utf8String sheetName)
        {
        BentleyApi::Dgn::DgnModelPtr model;
        for (auto const& modelEntry : db.Models ().MakeIterator (BIS_SCHEMA (BIS_CLASS_GeometricModel)))
            {
            model = db.Models ().GetModel (modelEntry.GetModelId ());
            if (0 == model->GetName ().CompareTo (sheetName))
                break;
            }

        return model.IsValid () ? model->ToSheetModel () : nullptr;
        }
    void AnalyzeProxyGraphics(std::vector<BentleyApi::DRange3d>& dgRanges, DrawingModelR drawingModel);
    void FindSheetModelSource(DgnV8Api::ModelId& v8SheetModelId, DgnDbR db, DgnModelId sheetModelId);
    void AnalyzeDrawingModels(std::vector<DgnElementId>& drawingElementIds, int& attachmentAspectCount, DgnDbR db);
    void CheckExtractedDrawingGraphics(
        Utf8CP expectedSeetName,
        Utf8CP expectedDrawingUserLabel,
        int expectedDrawingElementCount,
        int expectedAttachmentAspectCount,
        int expectedDrawingGraphicCount,
        DgnV8Api::ModelId expectedV8SheetModelId
        );
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 03/2018
* Test name of the converted sheet
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SheetCompositionTests, SheetNameTest)
    {
    LineUpFiles (L"SheetNameTest.bim", L"DVTest_Case1.dgn", true);
    m_wantCleanUp = true;
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    BentleyApi::Dgn::Sheet::ModelCP sheetModel = GetSheetModel (*db, "Section_Case1");

    ASSERT_TRUE (nullptr != sheetModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 03/2018
* Test size of the converted sheet
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SheetCompositionTests, SheetSizeTest)
    {
    LineUpFiles (L"SheetSizeTest.bim", L"DVTest_Case1.dgn", true);
    m_wantCleanUp = true;

    Bentley::SheetDefCP v8SheetDef = GetV8SheetDef(L"Section_Case1");
    double v8SheetWidth = 0.0;
    double v8SheetHeight = 0.0;
    v8SheetDef->GetSize (v8SheetWidth, v8SheetHeight);
    DgnDbPtr db = OpenExistingDgnDb (m_dgnDbFileName);

    auto sheetElement = db->Elements ().Get<BentleyApi::Dgn::Sheet::Element> (findFirstElementByClass (*db, getBisClassId (*db, BIS_CLASS_Sheet)));
    double height = sheetElement->GetHeight ();
    double width = sheetElement->GetWidth ();
    double unitsScaleFactor = height / v8SheetHeight;

    ASSERT_DOUBLE_EQ (height, v8SheetHeight*unitsScaleFactor);
    ASSERT_DOUBLE_EQ (width, v8SheetWidth*unitsScaleFactor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 03/2018
* Test if the attachement came through
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SheetCompositionTests, HasDgnAttachment)
    {
    LineUpFiles (L"HasDgnAttachment.bim", L"DVTest_Case1.dgn", true);
    m_wantCleanUp = true;
    DgnDbPtr db = OpenExistingDgnDb (m_dgnDbFileName);
    BentleyApi::Dgn::Sheet::ModelCP sheetModel = GetSheetModel (*db, "Section_Case1");
    auto attachments = sheetModel->GetSheetAttachmentViews (*db);
    ASSERT_TRUE (0 != attachments.size ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 03/2018
* Test sheet has a drawing boundary in it
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SheetCompositionTests, HasDrawingBoundary)
    {
    LineUpFiles (L"HasDrawingBoundary.bim", L"DVTest_Case1.dgn", true);
    m_wantCleanUp = true;
    DgnDbPtr db = OpenExistingDgnDb (m_dgnDbFileName);
    
    auto drawingBoundary= db->Elements ().Get<BentleyApi::Dgn::GenericViewAttachmentLabel> (findFirstElementByClass (*db, db->Schemas ().GetClassId (GENERIC_DOMAIN_NAME, GENERIC_CLASS_ViewAttachmentLabel)));
    ASSERT_TRUE (drawingBoundary.IsValid ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SheetCompositionTests::FindSheetModelSource(DgnV8Api::ModelId& v8SheetModelId, DgnDbR db, DgnModelId sheetModelId)
    {
    auto selSheetModelSourceId = db.GetPreparedECSqlStatement(
        "SELECT x.Identifer from " BIS_SCHEMA(BIS_CLASS_Sheet) " d JOIN " XTRN_SRC_ASPCT_FULLCLASSNAME " x ON (x.Element.id=d.ECInstanceId)"
        " WHERE x.Element.Id=? and x.Kind='Model'");
    selSheetModelSourceId->BindId(1, sheetModelId);
    ASSERT_EQ(BE_SQLITE_ROW, selSheetModelSourceId->Step());
    ASSERT_EQ(1, sscanf(selSheetModelSourceId->GetValueText(0), "%d", &v8SheetModelId));
    ASSERT_EQ(BE_SQLITE_DONE, selSheetModelSourceId->Step()) << "Should only be one Model aspect on specified sheet element";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SheetCompositionTests::AnalyzeDrawingModels(std::vector<DgnElementId>& drawingElementIds, int& attachmentAspectCount, DgnDbR db)
    {
    auto selDrawingModelAspect = db.GetPreparedECSqlStatement(
        "SELECT d.ECInstanceId, x.ECInstanceId from " BIS_SCHEMA(BIS_CLASS_Drawing) " d JOIN " XTRN_SRC_ASPCT_FULLCLASSNAME " x ON (x.Element.id=d.ECInstanceId)"
        " WHERE x.Kind='Element'");
    while (BE_SQLITE_ROW == selDrawingModelAspect->Step())
        {
        DgnElementId eid = selDrawingModelAspect->GetValueId<DgnElementId>(0);
        auto aspectId = selDrawingModelAspect->GetValueId<EC::ECInstanceId>(1);

        if (std::find(drawingElementIds.begin(), drawingElementIds.end(), eid) == drawingElementIds.end())
            drawingElementIds.push_back(eid);

        auto el = db.Elements().GetElement(eid);
        auto aspect = iModelExternalSourceAspect::GetAspect(*el, aspectId);
        auto aspectDump = aspect.FormatForDump(db, true, true);
        printf("%s\n", aspectDump.c_str());

        auto props = aspect.GetProperties();
        if (props.HasMember("v8AttachmentInfo"))
            ++attachmentAspectCount;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SheetCompositionTests::AnalyzeProxyGraphics(std::vector<BentleyApi::DRange3d>& dgRanges, DrawingModelR drawingModel)
    {
    auto& db = drawingModel.GetDgnDb();

    // Find all ExternalSourceAspects on all DrawingGraphics in the DrawingModel
    auto selGraphics = db.GetPreparedECSqlStatement(
        "SELECT x.Identifer, x.Scope.Id, dg.ECInstanceId, x.ECInstanceId from " BIS_SCHEMA(BIS_CLASS_DrawingGraphic) " dg JOIN " XTRN_SRC_ASPCT_FULLCLASSNAME " x ON (x.Element.id=dg.ECInstanceId)"
        " WHERE dg.Model.Id=? and x.Kind='ProxyGraphic'");
    selGraphics->BindId(1, drawingModel.GetModelId());
    
    while (BE_SQLITE_ROW == selGraphics->Step())
        {
        Utf8String sourceId = selGraphics->GetValueText(0);
        DgnElementId scopeId = selGraphics->GetValueId<DgnElementId>(1);
        DgnElementId dgEid = selGraphics->GetValueId<DgnElementId>(2);
        auto aspectId = selGraphics->GetValueId<EC::ECInstanceId>(3);

        auto dgEl = db.Elements().Get<DrawingGraphic>(dgEid);
        auto scopeEl = db.Elements().GetElement(scopeId);
        printf("SourceId: %s scopeId=%lld (%s) dgEid=%lld (%s)", sourceId.c_str(), scopeId.GetValue(), scopeEl->GetElementClass()->GetFullName(), dgEid.GetValue(), dgEl->GetElementClass()->GetFullName());
            
        auto aspect = iModelExternalSourceAspect::GetAspect(*dgEl, aspectId);
        auto aspectDump = aspect.FormatForDump(db, true, true);
        printf("%s\n", aspectDump.c_str());

        dgRanges.push_back(dgEl->GetPlacement().CalculateRange());
        }

    // The loop above examined and counted the DrawingGraphic elements in the drawingModel that have XSAs.
    // In fact, *all* DrawingGraphic elements should have XSAs. The following check asserts that.
    countElementsInModelByClass(drawingModel, getBisClassId(db, BIS_CLASS_DrawingGraphic), dgRanges.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SheetCompositionTests::CheckExtractedDrawingGraphics(
    Utf8CP expectedSeetName,
    Utf8CP expectedDrawingUserLabel,
    int expectedDrawingElementCount,
    int expectedAttachmentAspectCount,
    int expectedDrawingGraphicCount,
    DgnV8Api::ModelId expectedV8SheetModelId
    )
    {
    std::vector<BentleyApi::DRange3d> dgRanges;
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb (m_dgnDbFileName);
    
        auto itModels = db->Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Document));
        for (auto entry: itModels)
            {
            auto el = db->Elements().GetElement(entry.GetElementId());
            iModelExternalSourceAspect::Dump(*el, nullptr, BentleyApi::NativeLogging::SEVERITY::LOG_INFO);
            }

        auto itDrawingGraphics = db->Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_DrawingGraphic));
        for (auto entry: itDrawingGraphics)
            {
            auto el = db->Elements().GetElement(entry.GetElementId());
            iModelExternalSourceAspect::Dump(*el, nullptr, BentleyApi::NativeLogging::SEVERITY::LOG_INFO);
            }

        BentleyApi::Dgn::Sheet::ModelCP sheetModel = GetSheetModel (*db, expectedSeetName);
        ASSERT_TRUE (nullptr != sheetModel);

        DgnV8Api::ModelId v8SheetModelId{};
        FindSheetModelSource(v8SheetModelId, *db, sheetModel->GetModelId());
        ASSERT_EQ(expectedV8SheetModelId, v8SheetModelId);

        std::vector<DgnElementId> drawingElementIds;
        int attachmentAspectCount = 0;
        AnalyzeDrawingModels(drawingElementIds, attachmentAspectCount, *db);
        ASSERT_EQ(expectedDrawingElementCount, drawingElementIds.size());
        ASSERT_EQ(expectedAttachmentAspectCount, attachmentAspectCount);

        auto drawing = db->Elements().Get<Drawing>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_Drawing)));
        ASSERT_TRUE(drawing.IsValid());
        ASSERT_STREQ(drawing->GetUserLabel(), expectedDrawingUserLabel);
        auto drawingModel = drawing->GetSub<DrawingModel>();
        ASSERT_TRUE(drawingModel.IsValid());

        AnalyzeProxyGraphics(dgRanges, *drawingModel);
        ASSERT_EQ(expectedDrawingGraphicCount, dgRanges.size());
        }

    if (true)
        {
        /*
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);

        auto selSheetModelSourceId = db->GetPreparedECSqlStatement("SELECT x.Identifer from " BIS_SCHEMA(BIS_CLASS_Sheet) " d JOIN " XTRN_SRC_ASPCT_FULLCLASSNAME " x ON (x.Element.id=d.ECInstanceId)"
                                                        " WHERE x.Element.Id=? and x.Kind='Model'");
        selSheetModelSourceId->BindId(1, sheetModel->GetModeledElementId());
        ASSERT_EQ(BE_SQLITE_ROW, selSheetModelSourceId->Step());
        DgnV8Api::ModelId v8SheetModelId;
        ASSERT_EQ(1, sscanf(selSheetModelSourceId->GetValueText(0), "%d", &v8SheetModelId));


        auto v8SheetModel = v8editor.m_file->LoadRootModelById(nullptr, v8SheetModelId, true, true, false);
        ASSERT_TRUE(nullptr != v8SheetModel);
        */

        }

    // Do an update *with no changes*
    DoUpdate(m_dgnDbFileName, m_v8FileName);

    // Verify that *nothing changed*
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb (m_dgnDbFileName);
    
        BentleyApi::Dgn::Sheet::ModelCP sheetModel = GetSheetModel (*db, expectedSeetName);
        ASSERT_TRUE (nullptr != sheetModel);

        DgnV8Api::ModelId v8SheetModelId{};
        FindSheetModelSource(v8SheetModelId, *db, sheetModel->GetModelId());
        ASSERT_EQ(expectedV8SheetModelId, v8SheetModelId);

        std::vector<DgnElementId> drawingElementIds;
        int attachmentAspectCount = 0;
        AnalyzeDrawingModels(drawingElementIds, attachmentAspectCount, *db);
        ASSERT_EQ(expectedDrawingElementCount, drawingElementIds.size());
        ASSERT_EQ(expectedAttachmentAspectCount, attachmentAspectCount);

        auto drawing = db->Elements().Get<Drawing>(findFirstElementByClass(*db, getBisClassId(*db, BIS_CLASS_Drawing)));
        ASSERT_TRUE(drawing.IsValid());
        ASSERT_STREQ(drawing->GetUserLabel(), expectedDrawingUserLabel);
        auto drawingModel = drawing->GetSub<DrawingModel>();
        ASSERT_TRUE(drawingModel.IsValid());

        // Verify that there are the same number of DrawingGraphics and that they did not change in size, location, or rotation
        std::vector<BentleyApi::DRange3d> dgRangesAfterUpdate;
        AnalyzeProxyGraphics(dgRangesAfterUpdate, *drawingModel);
        ASSERT_EQ(dgRangesAfterUpdate.size(), dgRanges.size()) << "No change expected in the number of DrawingGraphics";
        for (size_t i=0; i<dgRanges.size(); ++i)
            {
            ASSERT_TRUE(dgRangesAfterUpdate[i].IsEqual(dgRanges[i], 1.0e-10)) << "No change expected in the range of an DrawingGraphic";
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SheetCompositionTests, ExtractedDrawingGraphics)
    {
    m_params.SetWantProvenanceInBim(true);

    LineUpFiles (L"HasDrawingBoundary.bim", L"DVTest_Case1.dgn", true);
    m_wantCleanUp = false;

    static constexpr Utf8CP expectedSeetName = "Section_Case1";
    static constexpr Utf8CP expectedDrawingUserLabel = "Section_Case1";

    int expectedDrawingElementCount = 1;
    int expectedAttachmentAspectCount = 4;
    int expectedDrawingGraphicCount = 7;

    DgnV8Api::ModelId expectedV8SheetModelId = 219;


    }