/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DPTEST

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct DgnModelTests : public DgnDbTestFixture
    {
    DgnModelPtr LoadModel(Utf8CP name)
        {
        DgnCode partitionCode = InformationPartitionElement::CreateCode(*m_db->Elements().GetRootSubject(), name);
        DgnModelId modelId = m_db->Models().QuerySubModelId(partitionCode);
        DgnModelPtr model =  m_db->Models().GetModel(modelId);
        BeAssert(model.IsValid());
        return model;
        }

    DgnElementId InsertElement3d(DgnModelId mid, Placement3dCR placement, DPoint3dCR pt1, DPoint3dCR pt2);
    DgnElementId InsertElement2d(DgnModelId mid, Placement2dCR placement, DPoint3dCR pt1, DPoint3dCR pt2);

    void TestRangeIndex3d();
    void TestRangeIndex2d();
    void CheckEmptyModel();
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
DgnElementId DgnModelTests::InsertElement3d(DgnModelId mid, Placement3dCR placement, DPoint3dCR pt1, DPoint3dCR pt2)
    {
    DgnCategoryId cat = DgnDbTestUtils::GetFirstSpatialCategoryId(*m_db);
    DgnElementPtr elem = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(*m_db, mid, DgnClassId(m_db->Schemas().GetClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject)), cat, placement));

    GeometryBuilderPtr builder = GeometryBuilder::Create(*elem->ToGeometrySource());
    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(pt1, pt2)));
    builder->Finish(*elem->ToGeometrySourceP());

    auto newElem = m_db->Elements().Insert(*elem);
    return newElem.IsValid() ? newElem->GetElementId() : DgnElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
DgnElementId DgnModelTests::InsertElement2d(DgnModelId mid, Placement2dCR placement, DPoint3dCR pt1, DPoint3dCR pt2)
    {
    DgnCategoryId cat = DgnDbTestUtils::GetFirstDrawingCategoryId(*m_db);
    DgnElementPtr elem = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(*m_db, mid, DgnClassId(m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AnnotationElement2d)), cat, placement));

    GeometryBuilderPtr builder = GeometryBuilder::Create(*elem->ToGeometrySource());
    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(pt1, pt2)));
    builder->Finish(*elem->ToGeometrySourceP());

    auto newElem = m_db->Elements().Insert(*elem);
    return newElem.IsValid() ? newElem->GetElementId() : DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModelTests::TestRangeIndex3d()
    {
    auto model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "RangeTest");

    model->FillRangeIndex(); // test maintaining the range index as add elements

    Placement3d placement(DPoint3d::From(2,2,0), YawPitchRollAngles(AngleInDegrees::FromDegrees(90), AngleInDegrees::FromDegrees(0), AngleInDegrees::FromDegrees(0)));
    DPoint3d pt1 = DPoint3d::FromZero();
    DPoint3d pt2 = DPoint3d::From(1, 0, 0);

    EXPECT_TRUE(InsertElement3d(model->GetModelId(), placement, pt1, pt2).IsValid());
    EXPECT_TRUE(InsertElement3d(model->GetModelId(), placement, pt1, pt2).IsValid());
    EXPECT_TRUE(InsertElement3d(model->GetModelId(), placement, pt1, pt2).IsValid());
    EXPECT_TRUE(InsertElement3d(model->GetModelId(), placement, pt1, pt2).IsValid());

    auto rangeIndex = model->GetRangeIndex();
    EXPECT_TRUE(4 == rangeIndex->GetCount());
    EXPECT_TRUE(4 == rangeIndex->DebugElementCount());

    int count = 0;
    for (auto& el : model->MakeIterator())
        {
        EXPECT_TRUE(nullptr != rangeIndex->FindElement(el.GetElementId()));
        ++count;
        }
    EXPECT_TRUE(count == 4);

    model->RemoveRangeIndex();  // drop the range index and recreate it from a query
    AxisAlignedBox3d range = model->QueryElementsRange();

    model->FillRangeIndex();
    rangeIndex = model->GetRangeIndex();
    EXPECT_TRUE(4 == rangeIndex->GetCount());
    EXPECT_TRUE(4 == rangeIndex->DebugElementCount());

    count = 0;
    for (auto& el : model->MakeIterator())
        {
        EXPECT_TRUE(nullptr != rangeIndex->FindElement(el.GetElementId()));
        ++count;
        }
    
    EXPECT_TRUE(range.IsValid());
    DPoint3d low; low.Init(1.9995000000000001, 2.0000000000000000, -0.00050000000000000001);
    DPoint3d high; high.Init(2.0005000000000002, 3.0000000000000000, 0.00050000000000000001);

    AxisAlignedBox3d box(low, high);
    AxisAlignedBox3d indexbox(rangeIndex->GetExtents().ToRange3d());

    EXPECT_TRUE(box.IsEqual(range, .00000001));
    EXPECT_TRUE(indexbox.IsEqual(range, .00001));

    Placement3d placement2(DPoint3d::FromZero(), YawPitchRollAngles());
    pt1 = DPoint3d::From(-10.,-10,-10);
    pt2 = DPoint3d::From(20,20,20);
    auto id1 = InsertElement3d(model->GetModelId(), placement2, pt1, pt2);
    indexbox = AxisAlignedBox3d(rangeIndex->GetExtents().ToRange3d());
    EXPECT_TRUE(indexbox.IsEqual(AxisAlignedBox3d(pt1,pt2), .00001));

    auto el2 = m_db->Elements().GetElement(id1)->CopyForEdit();
    EXPECT_TRUE(el2.IsValid());
    
    placement2 = el2->ToGeometrySource3d()->GetPlacement();
    placement2.SetOrigin(DPoint3d::From(1,0,0));
    el2->ToGeometrySource3dP()->SetPlacement(placement2);
    EXPECT_TRUE(el2->Update().IsValid()); // modify the largest range element in the model. This should cause the range tree extent to change by x=1

    indexbox = AxisAlignedBox3d(rangeIndex->GetExtents().ToRange3d());
    pt1.x += 1;
    pt2.x += 1;

    EXPECT_TRUE(indexbox.IsEqual(AxisAlignedBox3d(pt1,pt2), .00001));

    EXPECT_TRUE(DgnDbStatus::Success == el2->Delete());  // deleting the element should remove it from the range index
    EXPECT_TRUE(nullptr == rangeIndex->FindElement(id1));
    indexbox = AxisAlignedBox3d(rangeIndex->GetExtents().ToRange3d()); // and the new extent of the model should be back to what it was before we added the large element
    EXPECT_TRUE(indexbox.IsEqual(range, .00001));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModelTests::CheckEmptyModel()
    {
    // check the range of an empty 3d model
    auto model2 = LoadModel("DefaultModel");
    AxisAlignedBox3d thirdRange = model2->ToGeometricModel()->QueryElementsRange();
    EXPECT_FALSE(thirdRange.IsValid());

    int count = 0;
    for (auto& el : model2->MakeIterator())
        {
        EXPECT_TRUE(el.GetElementId().IsValid()); // primarily to silence 'unused variable' warnings...
        ++count;
        }

    EXPECT_TRUE(0 == count);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModelTests::TestRangeIndex2d()
    {
    DgnDbTestUtils::InsertDrawingCategory(*m_db, "TestDrawingCategory");
    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "DrawingListModel");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "TestDrawing");
    DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);
    drawingModel->FillRangeIndex(); // test maintaining the range index as we add elements

    Placement2d placement(DPoint2d::From(2,2), AngleInDegrees::FromDegrees(90));
    DPoint3d pt1 = DPoint3d::FromZero();
    DPoint3d pt2 = DPoint3d::From(1, 0, 0);

    EXPECT_TRUE(InsertElement2d(drawingModel->GetModelId(), placement, pt1, pt2).IsValid());
    EXPECT_TRUE(InsertElement2d(drawingModel->GetModelId(), placement, pt1, pt2).IsValid());
    EXPECT_TRUE(InsertElement2d(drawingModel->GetModelId(), placement, pt1, pt2).IsValid());
    EXPECT_TRUE(InsertElement2d(drawingModel->GetModelId(), placement, pt1, pt2).IsValid());

    auto rangeIndex = drawingModel->GetRangeIndex();
    EXPECT_TRUE(4 == rangeIndex->GetCount());
    EXPECT_TRUE(4 == rangeIndex->DebugElementCount());
    int count = 0;
    for (auto& el : drawingModel->MakeIterator())
        {
        EXPECT_TRUE(nullptr != rangeIndex->FindElement(el.GetElementId()));
        ++count;
        }
    EXPECT_TRUE(count == 4);

    m_db->SaveChanges();

    DPoint3d low; low.Init(1.9995000000000001, 2.0000000000000000, -1.0);
    DPoint3d high; high.Init(2.0005000000000002, 3.0000000000000000, 1.0);
    AxisAlignedBox3d box(low, high);

    AxisAlignedBox3d indexbox2d (rangeIndex->GetExtents().ToRange3d());
    AxisAlignedBox3d range2d = drawingModel->ToGeometricModel()->QueryElementsRange();
    drawingModel->ToGeometricModelP()->RemoveRangeIndex();  // drop the range so query wo
    AxisAlignedBox3d range2d2 = drawingModel->ToGeometricModel()->QueryElementsRange();

    EXPECT_TRUE(box.IsEqual(range2d2, .00000001));
    EXPECT_TRUE(indexbox2d.IsEqual(range2d2, .00001)); // float vs. double
    EXPECT_TRUE(indexbox2d.IsEqual(range2d));  // should be identical, they both come from range index
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, RangeIndex)
    {
    SetupSeedProject();

    TestRangeIndex3d();
    TestRangeIndex2d();
    CheckEmptyModel();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static int countSheetModels(DgnDbR db)
    {
    int count = 0;
    for (ModelIteratorEntryCR sheet : db.Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_SheetModel)))
        {
        EXPECT_TRUE(sheet.GetModelId().IsValid()); // to silence 'unused variable' warnings...
        ++count;
        }

    return count;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, SheetModelCRUD)
    {
    SetupSeedProject();

    DgnModelId sheetModelId1, sheetModelId2;
    
    BeFileName dbFileName;

    if (true)
        {
        DgnDbPtr db = m_db;
        ASSERT_EQ(0, countSheetModels(*db));

        double scale1 = 1.0;
        double height1 = 1.5;
        double width1 = 1.1;

        double scale2 = 2.0;
        double height2 = 2.5;
        double width2 = 2.2;

        // Create a sheet
        DgnDbTestUtils::InsertDrawingCategory(*db, "TestDrawingCategory");
        DocumentListModelPtr sheetListModel = DgnDbTestUtils::InsertDocumentListModel(*db, "SheetListModel");
        Sheet::ElementPtr sheet1 = DgnDbTestUtils::InsertSheet(*sheetListModel, scale1, height1, width1, "Sheet1");
        Sheet::ModelPtr sheetModel1 = DgnDbTestUtils::InsertSheetModel(*sheet1);
        sheetModelId1 = sheetModel1->GetModelId();

        ASSERT_EQ(1, countSheetModels(*db));
        ASSERT_NE(DgnDbStatus::Success, sheetModel1->Insert()) << "Should be illegal to INSERT a SheetModel that is already persistent";

        // Create a second sheet
        Sheet::ElementPtr sheet2 = DgnDbTestUtils::InsertSheet(*sheetListModel, scale2, height2, width2, "Sheet2");
        Sheet::ModelPtr sheetModel2 = DgnDbTestUtils::InsertSheetModel(*sheet2);
        sheetModelId2 = sheetModel2->GetModelId();

        ASSERT_EQ(2, countSheetModels(*db));

        ASSERT_EQ(scale1, sheet1->GetScale());
        ASSERT_EQ(scale2, sheet2->GetScale());
        ASSERT_EQ(height1, sheet1->GetHeight());
        ASSERT_EQ(height2, sheet2->GetHeight());
        ASSERT_EQ(width1, sheet1->GetWidth());
        ASSERT_EQ(width2, sheet2->GetWidth());
        DgnCode Code1 = Sheet::Element::CreateCode(*sheetListModel, "Sheet1");
        ASSERT_EQ(Code1,sheet1->GetCode());
        DgnCode Code2 = Sheet::Element::CreateCode(*sheetListModel, "Sheet2");
        ASSERT_EQ(Code2, sheet2->GetCode());

        //Set Sheet properies 
        sheet1->SetScale(2);
        ASSERT_EQ(2, sheet1->GetScale());
        sheet1->SetHeight(4);
        ASSERT_EQ(4, sheet1->GetHeight());
        sheet1->SetWidth(3.5);
        ASSERT_EQ(3.5, sheet1->GetWidth());
        sheet2->SetScale(2.5);
        ASSERT_EQ(2.5, sheet2->GetScale());
        sheet2->SetHeight(4);
        ASSERT_EQ(4, sheet2->GetHeight());
        sheet2->SetWidth(3);
        ASSERT_EQ(3, sheet2->GetWidth());

        ASSERT_TRUE(sheet1->Update().IsValid());
        ASSERT_TRUE(sheet2->Update().IsValid());

        sheet1 = nullptr;
        sheet2 = nullptr;
        dbFileName = db->GetFileName();
        db->SaveChanges();
        db->CloseDb();
        }

    // Verify that loading works
    if (true)
        {
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, dbFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE(db.IsValid());

        Sheet::ModelPtr sheetModel1 = db->Models().Get<Sheet::Model>(sheetModelId1);
        ASSERT_TRUE(sheetModel1.IsValid());

        Sheet::ElementCPtr sheet1 = db->Elements().Get<Sheet::Element>(sheetModel1->GetModeledElementId());
        ASSERT_TRUE(sheet1.IsValid());
        ASSERT_EQ(2, sheet1->GetScale());
        ASSERT_EQ(4, sheet1->GetHeight());
        ASSERT_EQ(3.5, sheet1->GetWidth());

        DgnModelPtr sheetModel2 = db->Models().GetModel(sheetModelId2);
        Sheet::ElementCPtr sheet2 = db->Elements().Get<Sheet::Element>(sheetModel2->GetModeledElementId());
        ASSERT_EQ(2.5, sheet2->GetScale());
        ASSERT_EQ(4, sheet2->GetHeight());
        ASSERT_EQ(3, sheet2->GetWidth());
        // Delete Sheet2
        ASSERT_EQ(2, countSheetModels(*db));
        ASSERT_EQ(DgnDbStatus::Success, sheetModel2->Delete());
        ASSERT_EQ(1, countSheetModels(*db));
        sheet1 = nullptr;
        sheet2 = nullptr;
        db->SaveChanges();
        db->CloseDb();
        }

    if (true)
        {
        m_db = DgnDb::OpenDgnDb(nullptr, dbFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE(m_db.IsValid());
        ASSERT_EQ(1, countSheetModels(*m_db));

        // Verify that we can only place drawing elements in a sheet
        Placement3d placement(DPoint3d::From(2,2,0), YawPitchRollAngles(AngleInDegrees::FromDegrees(90), AngleInDegrees::FromDegrees(0), AngleInDegrees::FromDegrees(0)));
        Placement2d placement2d(DPoint2d::From(2,2), AngleInDegrees::FromDegrees(90));
        DPoint3d pt1 = DPoint3d::FromZero();
        DPoint3d pt2 = DPoint3d::From(1, 0, 0);

        EXPECT_TRUE(!InsertElement3d(sheetModelId1, placement, pt1, pt2).IsValid());
        EXPECT_TRUE(InsertElement2d(sheetModelId1, placement2d, pt1, pt2).IsValid());
        m_db->SaveChanges();
        m_db->CloseDb();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, ImportDictionaryModel)
    {
    SetupSeedProject();
    DgnDbR db = GetDgnDb();

    DgnModelPtr model = db.Models().GetModel(DgnModel::DictionaryId());
    EXPECT_TRUE(model.IsValid());

    DictionaryModelPtr dictModel = db.Models().Get<DictionaryModel>(DgnModel::DictionaryId());
    EXPECT_TRUE(dictModel.IsValid());
    EXPECT_EQ(dictModel.get(), model.get());

    DictionaryModelR dictModelR = db.GetDictionaryModel();
    EXPECT_EQ(&dictModelR, dictModel.get());
    EXPECT_FALSE(dictModelR.IsPrivate());

    // The dictionary model cannot be copied
    struct DisableAssertions
        {
        DisableAssertions() { BeTest::SetFailOnAssert(false); }
        ~DisableAssertions() { BeTest::SetFailOnAssert(true); }
        };

    DisableAssertions _V_V_V;
    DgnImportContext cc(db, db);
    DefinitionPartitionCPtr partitionForCopy = DefinitionPartition::CreateAndInsert(*db.Elements().GetRootSubject(), "PartitionForCopy");
    EXPECT_TRUE(partitionForCopy.IsValid());
    EXPECT_TRUE(DgnModel::Import(nullptr, dictModelR, cc, *partitionForCopy).IsNull());

    Utf8CP dbFilePath = db.GetDbFileName();
    WString fileName;
    BeStringUtilities::Utf8ToWChar(fileName, dbFilePath);

    BeFileName fullOutputFileName;
    BeTest::GetHost().GetOutputRoot(fullOutputFileName);
    fullOutputFileName.AppendToPath(BeFileName(BeTest::GetNameOfCurrentTestCase()));
    fullOutputFileName.AppendToPath(L"ImportDictionaryModelcc.dgndb");

    EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(fileName.c_str(), fullOutputFileName));

    DbResult result = BE_SQLITE_OK;
    DgnDb::OpenMode mode = DgnDb::OpenMode::ReadWrite;
    DgnDbPtr dbcopy = DgnDb::OpenDgnDb(&result, fullOutputFileName, DgnDb::OpenParams(mode));
    ASSERT_EQ(BE_SQLITE_OK, result);
    ASSERT_TRUE(dbcopy.IsValid());

    // The dictionary model cannot be imported
    DgnImportContext importer(db, *dbcopy);
    DefinitionPartitionCPtr partitionForImport = DefinitionPartition::CreateAndInsert(*dbcopy->Elements().GetRootSubject(), "PartitionForImport");
    EXPECT_TRUE(partitionForImport.IsValid());
    EXPECT_TRUE(DgnModel::Import(nullptr, dictModelR, importer, *partitionForImport).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, ModelIterator)
    {
    SetupSeedProject();
    PhysicalModelPtr physicalModel1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "PhysicalModel1");
    PhysicalModelPtr physicalModel2 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "PhysicalModel2");
    PhysicalModelPtr physicalModel3 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "PhysicalModel3");
    DgnClassId physicalModelClassId = m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalModel);

    ASSERT_TRUE(physicalModel1->IsSpatiallyLocated());
    ASSERT_FALSE(physicalModel1->IsNotSpatiallyLocated());
    ASSERT_FALSE(physicalModel1->IsPlanProjection());

    SpatialLocationModelPtr spatialLocationModel1 = DgnDbTestUtils::InsertSpatialLocationModel(*m_db, "SpatialLocationModel1");
    SpatialLocationModelPtr spatialLocationModel2 = DgnDbTestUtils::InsertSpatialLocationModel(*m_db, "SpatialLocationModel2");

    ASSERT_TRUE(spatialLocationModel1->IsSpatiallyLocated());
    ASSERT_FALSE(spatialLocationModel1->IsNotSpatiallyLocated());
    ASSERT_FALSE(spatialLocationModel1->IsPlanProjection());

    DocumentListModelPtr documentListModel1 = DgnDbTestUtils::InsertDocumentListModel(*m_db, "DocumentListModel1");

    const int numPhysicalModels = 3 + 1; // 1 PhysicalModel created by SetupSeedProject
    const int numSpatialLocationModels = 2;
    const int numSpatialModels = numPhysicalModels + numSpatialLocationModels;
    const int numDocumentListModels = 1;

    ModelIterator iterator = m_db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalModel));
    ASSERT_EQ(numPhysicalModels, iterator.BuildIdSet().size());
    ASSERT_EQ(numPhysicalModels, iterator.BuildIdList().size());

    bvector<DgnModelId> idList;
    iterator.BuildIdList(idList);
    ASSERT_EQ(numPhysicalModels, idList.size());

    int count = 0;
    for (ModelIteratorEntryCR entry : iterator)
        {
        ASSERT_EQ(physicalModelClassId, entry.GetClassId());
        ++count;
        }

    ASSERT_EQ(numPhysicalModels, count);
    ASSERT_EQ(numSpatialLocationModels, m_db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialLocationModel)).BuildIdSet().size());
    ASSERT_EQ(numSpatialModels, m_db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialModel)).BuildIdSet().size());
    ASSERT_EQ(numDocumentListModels, m_db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_DocumentListModel)).BuildIdSet().size());
    ASSERT_EQ(1, m_db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_DictionaryModel)).BuildIdSet().size());

    Utf8PrintfString whereClause("WHERE ECInstanceId=%" PRIu64, physicalModel1->GetModelId().GetValue());
    count = 0;
    for (ModelIteratorEntryCR entry : m_db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalModel), whereClause.c_str()))
        {
        ASSERT_EQ(physicalModel1->GetModelId(), entry.GetModelId());
        ASSERT_EQ(physicalModel1->GetModeledElementId(), entry.GetModeledElementId());
        ASSERT_EQ(physicalModelClassId, entry.GetClassId());
        ASSERT_TRUE(!entry.IsPrivate());
        ASSERT_FALSE(entry.IsTemplate());
        ++count;
        }

    ASSERT_EQ(1, count);

    idList = m_db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalModel), nullptr, "ORDER BY ECInstanceId ASC").BuildIdList();
    ASSERT_EQ(numPhysicalModels, idList.size());
    ASSERT_EQ(physicalModel1->GetModelId(), idList[1]);
    ASSERT_EQ(physicalModel2->GetModelId(), idList[2]);
    ASSERT_EQ(physicalModel3->GetModelId(), idList[3]);

    idList = m_db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalModel), nullptr, "ORDER BY ECInstanceId DESC").BuildIdList();
    ASSERT_EQ(numPhysicalModels, idList.size());
    ASSERT_EQ(physicalModel1->GetModelId(), idList[2]);
    ASSERT_EQ(physicalModel2->GetModelId(), idList[1]);
    ASSERT_EQ(physicalModel3->GetModelId(), idList[0]);

    ASSERT_EQ(numSpatialModels, m_db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialModel), "WHERE [IsTemplate]=false", "ORDER BY ECInstanceId").BuildIdSet().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, AbandonChanges)
    {
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*=needBriefcase*/);
    DgnDbR db = GetDgnDb();

    PhysicalModelPtr model1 = DgnDbTestUtils::InsertPhysicalModel(db, "Model1");
    db.SaveChanges("changeSet1");

    model1->Delete();
    EXPECT_FALSE(db.Models().GetModel(model1->GetModelId()).IsValid());

    PhysicalModelPtr model2 = DgnDbTestUtils::InsertPhysicalModel(db, "Model2");

    //Model 1 should be back. Model 2 shouldnt be in the db anymore.
    DbResult result = db.AbandonChanges();
    EXPECT_EQ(result, BE_SQLITE_OK);
    EXPECT_TRUE(db.Models().GetModel(model1->GetModelId()).IsValid());
    EXPECT_FALSE(db.Models().GetModel(model2->GetModelId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, GetSetModelUnitDefinition)
    {
    SetupSeedProject();
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    BeFileName outFileName = (BeFileName)m_db->GetDbFileName();

    GeometricModel::Formatter displayInfo = model->GetFormatterR();
    EXPECT_STREQ("m", displayInfo.GetMasterUnits().GetLabel().c_str());
    EXPECT_STREQ("mm", displayInfo.GetSubUnits().GetLabel().c_str());
    Utf8String name = "Invalid*Name";
    Utf8CP InvalidChar = "*";
    Utf8Char replace = ' ';

    bool check = DgnDbTable::IsValidName(name, InvalidChar);
    EXPECT_FALSE(check);
    DgnDbTable::ReplaceInvalidCharacters(name, InvalidChar, replace);
    EXPECT_EQ("Invalid Name", (Utf8String) name);
    check = DgnDbTable::IsValidName(name, InvalidChar);
    EXPECT_TRUE(check);

    // Try Update model unit definition with wrong values
    GeometricModel::Formatter displayInfo2 = model->GetFormatterR();
    UnitDefinition newMasterUnitw(UnitBase::Meter, UnitSystem::Metric, 100.0, 10.0, "newMasterUnit");
    UnitDefinition newSubUnit(UnitBase::Meter, UnitSystem::Metric, 25.0, 25.0, "newSubUnit");
    ASSERT_TRUE(BentleyStatus::ERROR == displayInfo2.SetUnits(newMasterUnitw, newSubUnit));
    UnitDefinition newMasterUnitw2(UnitBase::Meter, UnitSystem::Metric, -100.0, 10.0, "newMasterUnit");
    ASSERT_TRUE(BentleyStatus::ERROR == displayInfo2.SetUnits(newMasterUnitw2, newSubUnit));
    // Update model unit definition with correct values
    UnitDefinition newMasterUnit(UnitBase::Meter, UnitSystem::Metric, 10.0, 10.0, "newMasterUnit");
    ASSERT_TRUE(BentleyStatus::SUCCESS == displayInfo2.SetUnits(newMasterUnit, newSubUnit));
    model->GetFormatterR() = displayInfo2;
    ASSERT_TRUE(DgnDbStatus::Success == model->Update());
    m_db->SaveChanges();
    m_db->CloseDb();
    OpenDb(m_db,outFileName, BeSQLite::Db::OpenMode::Readonly);
    displayInfo = model->GetFormatterR();
    ASSERT_STREQ("newMasterUnit", displayInfo.GetMasterUnits().GetLabel().c_str());
    ASSERT_STREQ("newSubUnit", displayInfo.GetSubUnits().GetLabel().c_str());
    ASSERT_TRUE(25 == displayInfo.GetSubUnits().GetNumerator());
    ASSERT_TRUE(25 == displayInfo.GetSubUnits().GetDenominator());
    ASSERT_TRUE(10 == displayInfo.GetMasterUnits().GetNumerator());
    ASSERT_TRUE(10 == displayInfo.GetMasterUnits().GetDenominator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, CodeUniqueness)
    {
    SetupSeedProject();
    DgnDbR db = GetDgnDb();
    PhysicalModelPtr model1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Testcode1");
    ASSERT_TRUE(model1->IsPhysicalModel());
    DgnModelId modelid1 = model1->GetModelId();
    ASSERT_TRUE(DgnDbTestUtils::CodeValueExists(*m_db, "Testcode1"));
    DgnCode partitionCode1 = InformationPartitionElement::CreateCode(*m_db->Elements().GetRootSubject(), "Testcode1");
    DgnModelId modelId1 = db.Models().QuerySubModelId(partitionCode1);
    ASSERT_TRUE(modelid1 == modelId1);

    PhysicalModelPtr model2 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Testcode2");
    ASSERT_TRUE(model2->IsPhysicalModel());
    DgnModelId modelid2 = model2->GetModelId();
    ASSERT_TRUE(DgnDbTestUtils::CodeValueExists(*m_db, "Testcode2"));
    DgnCode partitionCode2 = InformationPartitionElement::CreateCode(*m_db->Elements().GetRootSubject(), "Testcode2");
    DgnModelId modelId2 = db.Models().QuerySubModelId(partitionCode2);
    ASSERT_TRUE(modelid2 == modelId2);

    // Checking models are identifed by unique DgnCode 
    DgnElementId eleid=model1->GetModeledElementId();
    auto ele=m_db->Elements().GetElement(eleid)->CopyForEdit();
    ASSERT_TRUE("Testcode1" == ele->GetCode().GetValueUtf8());
    DgnCode updatepartitionCode = InformationPartitionElement::CreateCode(*m_db->Elements().GetRootSubject(), "Testcode2");
    ele->SetCode(updatepartitionCode);
    ASSERT_TRUE(updatepartitionCode == ele->GetCode());
    DgnDbStatus stat;
    ASSERT_FALSE(ele->Update(&stat).IsValid());
    EXPECT_EQ(DgnDbStatus::DuplicateCode, stat);
    // Update Dgncode by getting uniqueDgncode
    updatepartitionCode = InformationPartitionElement::CreateUniqueCode(*m_db->Elements().GetRootSubject(), "Testcode2");
    ele->SetCode(updatepartitionCode);
    ASSERT_TRUE(updatepartitionCode == ele->GetCode());
    DgnElementCPtr updatedele = ele->Update(&stat);
    ASSERT_TRUE(updatedele.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, stat);
    ASSERT_TRUE(updatepartitionCode == updatedele->GetCode());
   }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, DefinitionModelCreation)
    {
    // TODO how to create DefinitionElement and insert
    SetupSeedProject();
    DefinitionPartitionCPtr defp = DefinitionPartition::CreateAndInsert(*m_db->Elements().GetRootSubject(), "DefinitionPartitionElement", "This is new DefinitionPartition");
    ASSERT_TRUE(defp.IsValid());
    ASSERT_TRUE(DgnDbTestUtils::CodeValueExists(*m_db, "DefinitionPartitionElement"));
    DefinitionModelPtr defmodel = DefinitionModel::CreateAndInsert(*defp);
    ASSERT_TRUE(defmodel.IsValid());
    ASSERT_EQ(defmodel->GetModeledElementId(), defp->GetElementId());
    DefinitionModelPtr defmodelt = DefinitionModel::CreateAndInsert(*defp);
    ASSERT_FALSE(defmodelt.IsValid());

    DefinitionPartitionPtr defp_c = DefinitionPartition::Create(*m_db->Elements().GetRootSubject(), "DefinitionPartitionElement2", "This is second DefinitionPartition");
    DefinitionPartitionCPtr defp2 = m_db->Elements().Insert<DefinitionPartition>(*defp_c);
    ASSERT_TRUE(defp2.IsValid());
    ASSERT_TRUE(DgnDbTestUtils::CodeValueExists(*m_db, "DefinitionPartitionElement2"));
    DefinitionModelPtr defmodel2c=DefinitionModel::Create(*defp2);
    ASSERT_TRUE(defmodel2c.IsValid());
    ASSERT_EQ(DgnDbStatus::Success ,defmodel2c->Insert());
    ASSERT_EQ(defmodel2c->GetModeledElementId(), defp2->GetElementId());
    ASSERT_EQ(defmodel->DictionaryId(), defmodel2c->DictionaryId());

    DgnCode partitionCode1 = InformationPartitionElement::CreateCode(*m_db->Elements().GetRootSubject(), "DefinitionPartitionElement");
    DgnElementId eleId1=m_db->Elements().QueryElementIdByCode(partitionCode1);
    RefCountedCPtr<InformationPartitionElement> Infele1 = m_db->Elements().Get<InformationPartitionElement>(eleId1);
    ASSERT_EQ(Infele1->GetDescription(), "This is new DefinitionPartition");

    DgnCode partitionCode2 = InformationPartitionElement::CreateCode(*m_db->Elements().GetRootSubject(), "DefinitionPartitionElement2");
    DgnElementId eleId2 = m_db->Elements().QueryElementIdByCode(partitionCode2);
    RefCountedCPtr<InformationPartitionElement> Infele2 = m_db->Elements().Get<InformationPartitionElement>(eleId2);
    ASSERT_EQ(Infele2->GetDescription(), "This is second DefinitionPartition");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, GenericGroupModelCreation)
    {
    SetupSeedProject();
    GroupInformationPartitionPtr ginfop=GroupInformationPartition::Create(*m_db->Elements().GetRootSubject(), "GroupInformationPartitionElement", "This is GroupInformationPartitionElement");
    DgnElementCPtr elep = ginfop->Insert();
    ASSERT_TRUE(elep.IsValid());
    GenericGroupModelPtr genericgroupmodel=GenericGroupModel::CreateAndInsert(*elep);
    ASSERT_TRUE(genericgroupmodel.IsValid());
    GenericGroupPtr group=GenericGroup::Create(*genericgroupmodel);
    DgnElementCPtr ele=group->Insert();
    ASSERT_TRUE(ele.IsValid());
    ASSERT_EQ(ele->GetModelId(), genericgroupmodel->GetModeledElementId());

    GroupInformationPartitionCPtr ginfop2 = GroupInformationPartition::CreateAndInsert(*m_db->Elements().GetRootSubject(), "GroupInformationPartitionElement2", "GroupInformationPartitionElement2");
    ASSERT_TRUE(ginfop2.IsValid());
    GenericGroupModelPtr genericgroupmodelc = GenericGroupModel::Create(*ginfop2);
    ASSERT_EQ(DgnDbStatus::Success, genericgroupmodelc->Insert());
    GenericGroupPtr group2 = GenericGroup::Create(*genericgroupmodelc);
    DgnElementCPtr ele2 = group2->Insert();
    ASSERT_TRUE(ele2.IsValid());
    ASSERT_EQ(ele2->GetModelId(), genericgroupmodelc->GetModeledElementId());
    GenericGroupPtr group3 = GenericGroup::Create(*genericgroupmodelc);
    DgnElementCPtr ele3 = group3->Insert();
    ASSERT_TRUE(ele3.IsValid());
    ASSERT_EQ(ele3->GetModelId(), genericgroupmodelc->GetModeledElementId());
    bvector<DgnModelId> idList;
    idList =m_db->Models().MakeIterator(GENERIC_SCHEMA(GENERIC_CLASS_GroupModel), nullptr, "ORDER BY ECInstanceId ASC").BuildIdList();
    ASSERT_EQ(2,idList.size());
    ASSERT_EQ(genericgroupmodel->GetModelId(), idList[0]);
    ASSERT_EQ(genericgroupmodelc->GetModelId(), idList[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, GraphicalModel3d)
    {
    SetupSeedProject();
    SubjectCPtr subject = m_db->Elements().GetRootSubject();
    DgnClassId classId = m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_GraphicalPartition3d);
    InformationPartitionElementCPtr partition = InformationPartitionElement::CreateAndInsert(classId, *subject, "GraphicalPartition3d", "Description of GraphicalPartition3d");
    ASSERT_TRUE(partition.IsValid());
	GeometricModel3dPtr model = GenericGraphicalModel3d::CreateAndInsert(*partition);
    ASSERT_TRUE(model.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, ModelSelectorAndDelete)
    {
    SetupSeedProject();
    ASSERT_EQ(0, DgnDbTestUtils::SelectCountFromTable(*m_db, BIS_TABLE(BIS_REL_ModelSelectorRefersToModels)));

    PhysicalModelPtr physicalModel1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TestPhysical1");
    PhysicalModelPtr physicalModel2 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TestPhysical2");
    PhysicalModelPtr physicalModel3 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TestPhysical3");

    DefinitionModelPtr definitionModel = DgnDbTestUtils::InsertDefinitionModel(*m_db, "TestDefinitions");
    DgnElementId modelSelectorId;

    // Insert ModelSelector
        {
        ModelSelector modelSelector(*definitionModel, "TestModelSelector");
        modelSelector.AddModel(physicalModel1->GetModelId());
        modelSelector.AddModel(physicalModel2->GetModelId());
        modelSelector.AddModel(physicalModel3->GetModelId());
        ASSERT_TRUE(modelSelector.Insert().IsValid());
        modelSelectorId = modelSelector.GetElementId();
        ASSERT_TRUE(modelSelectorId.IsValid());
        ASSERT_EQ(3, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels)));
        ASSERT_EQ(3, DgnDbTestUtils::SelectCountFromTable(*m_db, BIS_TABLE(BIS_REL_ModelSelectorRefersToModels)));
        }

    // Verify ModelSelector was persisted properly
        {
        m_db->Elements().ClearCache();
        ModelSelectorCPtr modelSelector = m_db->Elements().Get<ModelSelector>(modelSelectorId);
        ASSERT_TRUE(modelSelector.IsValid());
        ASSERT_TRUE(modelSelector->ContainsModel(physicalModel1->GetModelId()));
        ASSERT_TRUE(modelSelector->ContainsModel(physicalModel2->GetModelId()));
        ASSERT_TRUE(modelSelector->ContainsModel(physicalModel3->GetModelId()));
        }

    // Verify DgnModel::Delete is handled properly
        {
        m_db->Elements().ClearCache();
        ASSERT_EQ(DgnDbStatus::Success, physicalModel2->Delete());
        ModelSelectorCPtr modelSelector = m_db->Elements().Get<ModelSelector>(modelSelectorId);
        ASSERT_TRUE(modelSelector.IsValid());
        ASSERT_TRUE(modelSelector->ContainsModel(physicalModel1->GetModelId()));
        ASSERT_FALSE(modelSelector->ContainsModel(physicalModel2->GetModelId()));
        ASSERT_TRUE(modelSelector->ContainsModel(physicalModel3->GetModelId()));
        ASSERT_EQ(2, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels)));
        ASSERT_EQ(2, DgnDbTestUtils::SelectCountFromTable(*m_db, BIS_TABLE(BIS_REL_ModelSelectorRefersToModels)));
        }

    // Verify ModelSelector::Delete is handled properly
        {
        m_db->Elements().ClearCache();
        m_db->Elements().Delete(modelSelectorId);
        ASSERT_FALSE(m_db->Elements().Get<ModelSelector>(modelSelectorId).IsValid());
        ASSERT_EQ(0, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels)));
        ASSERT_EQ(0, DgnDbTestUtils::SelectCountFromTable(*m_db, BIS_TABLE(BIS_REL_ModelSelectorRefersToModels)));
        }
    }

TEST_F(DgnModelTests, ModelModelsElementSubClass)
    {
    // Create two models, one using a custom relationship to the modeled element and the other the default
    DgnModelId mid1, mid2;
    BeFileName dbFileName;
    DgnClassId testrelclassid1, testrelclassid2;
    if (true)
        {
        SetupSeedProject();
        ASSERT_EQ(DgnPlatformTestDomain::GetDomain().ImportSchema(*m_db), SchemaStatus::Success);
        
        if (true)
            {
            SubjectCPtr rootSubject = m_db->Elements().GetRootSubject();

            if (true)
                {
                // Model1 - use the custom TestModelModelsElement ECRelationship
                auto partition1 = PhysicalPartition::CreateAndInsert(*rootSubject, "DgnModelTests_ModelModelsElementSubClass_P1");
                ASSERT_TRUE(partition1.IsValid());

                auto& handler = dgn_ModelHandler::Physical::GetHandler();
                auto classId = m_db->Domains().GetClassId(handler);
                ASSERT_TRUE(classId.IsValid());
                DgnModel::CreateParams params(*m_db, classId, partition1->GetElementId());
                testrelclassid1 = m_db->Schemas().GetClassId("DgnPlatformTest", "TestModelModelsElement");
                ASSERT_TRUE(testrelclassid1.IsValid());
                params.SetModeledElementRelClassId(testrelclassid1);
                auto model = handler.Create(params);
                ASSERT_TRUE(model.IsValid());
                ASSERT_EQ(model->GetModeledElementRelClassId(), testrelclassid1);
                ASSERT_EQ(model->Insert(), DgnDbStatus::Success);
                mid1 = model->GetModelId();
                }

            if (true)
                {
                // Model 2 - use the default ModelModelsElement ECRelationship
                auto partition2 = PhysicalPartition::CreateAndInsert(*rootSubject, "DgnModelTests_ModelModelsElementSubClass_P2");
                ASSERT_TRUE(partition2.IsValid());
                auto model2 = PhysicalModel::CreateAndInsert(*partition2);
                mid2 = model2->GetModelId();
                testrelclassid2 = model2->GetModeledElementRelClassId();
                ASSERT_EQ(testrelclassid2, m_db->Schemas().GetClassId("BisCore", "ModelModelsElement"));
                }
            }

        dbFileName = m_db->GetFileName();
        m_db->CloseDb();
        }

    // Verify that the Model.ModeledElement property captures the correct ECRelationship in both cases
    m_db = DgnDb::OpenDgnDb(nullptr, dbFileName, DgnDb::OpenParams(Db::OpenMode::Readonly));
    ASSERT_TRUE(m_db.IsValid());

    DgnModelCPtr model1 = m_db->Models().GetModel(mid1);
    ASSERT_TRUE(model1.IsValid());
    DgnModelCPtr model2 = m_db->Models().GetModel(mid2);
    ASSERT_TRUE(model2.IsValid());
    ASSERT_EQ(model1->GetModeledElementRelClassId(), testrelclassid1);
    ASSERT_EQ(model2->GetModeledElementRelClassId(), testrelclassid2);
    ASSERT_NE(testrelclassid1, testrelclassid2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/20
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, FlagsAddedInBisCore108)
    {
    SetupSeedProject();

    auto testFlags = [](PhysicalModelCR model)
        {
        EXPECT_TRUE(model.IsPlanProjection());
        EXPECT_TRUE(model.IsNotSpatiallyLocated());

        auto json = model.ToJson(Json::nullValue);
        EXPECT_TRUE(json["isPlanProjection"].asBool());
        EXPECT_TRUE(json["isNotSpatiallyLocated"].asBool());
        };

    DgnModelId modelId;
    BeFileName dbFileName;
        {
        auto model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "NonSpatialPlanProjection");
        EXPECT_FALSE(model->IsPlanProjection());
        EXPECT_FALSE(model->IsNotSpatiallyLocated());

        model->SetIsPlanProjection(true);
        model->SetNotSpatiallyLocated();

        testFlags(*model);

        EXPECT_EQ(DgnDbStatus::Success, model->Update());

        modelId = model->GetModelId();
        dbFileName = m_db->GetFileName();

        m_db->SaveChanges("set flags");
        m_db->CloseDb();
        }

    m_db = DgnDb::OpenDgnDb(nullptr, dbFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    EXPECT_TRUE(m_db->Models().FindModel(modelId).IsNull());

    auto model = m_db->Models().Get<PhysicalModel>(modelId);
    EXPECT_TRUE(model.IsValid());

    testFlags(*model);
    }
