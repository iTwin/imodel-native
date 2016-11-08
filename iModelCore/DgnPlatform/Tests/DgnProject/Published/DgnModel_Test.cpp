/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnModel_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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

    void InsertElement(DgnDbR, DgnModelId mid, bool is3d, bool expectSuccess);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
void DgnModelTests::InsertElement(DgnDbR db, DgnModelId mid, bool is3d, bool expectSuccess)
    {
    DgnCategoryId cat = DgnCategory::QueryHighestCategoryId(db);

    DgnElementPtr gelem;
    if (is3d)
        {
        Placement3d placement(DPoint3d::From(2,2,0), YawPitchRollAngles(AngleInDegrees::FromDegrees(90), AngleInDegrees::FromDegrees(0), AngleInDegrees::FromDegrees(0)));
        gelem = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject)), cat, placement));
        }
    else
        {
        Placement2d placement(DPoint2d::From(2,2), AngleInDegrees::FromDegrees(90));
        gelem = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AnnotationElement2d)), cat, placement));
        }

    GeometryBuilderPtr builder = GeometryBuilder::Create(*gelem->ToGeometrySource());
    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1, 0, 0))));

    if (SUCCESS != builder->Finish(*gelem->ToGeometrySourceP())) // Won't catch 2d/3d mismatch from just GeometrySource as we don't know a DgnModel...
        {
        ASSERT_FALSE(expectSuccess);
        return;
        }

    DgnElementCPtr newElem = db.Elements().Insert(*gelem);
    ASSERT_TRUE(expectSuccess == (newElem.IsValid() && newElem->GetElementId().IsValid()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, GetRange)
    {
    SetupSeedProject();
    auto model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "RangeTest");

    model->FillRangeIndex(); // test maintaining the range index as add elements

    InsertElement(*m_db, model->GetModelId(), true,  true);
    InsertElement(*m_db, model->GetModelId(), true,  true);
    InsertElement(*m_db, model->GetModelId(), true,  true);
    InsertElement(*m_db, model->GetModelId(), true,  true);

    auto rangeIndex = model->GetRangeIndex();
    EXPECT_TRUE(4 == rangeIndex->GetCount());
    EXPECT_TRUE(4 == rangeIndex->DebugElementCount());

    int count = 0;
    for (auto& el : model->MakeIterator())
        {
        EXPECT_TRUE(nullptr != rangeIndex->FindElement(el.GetId()));
        ++count;
        }
    EXPECT_TRUE(count == 4);

    model->RemoveRangeIndex();  // drop the range index and recreate it from a query
    model->FillRangeIndex();
    rangeIndex = model->GetRangeIndex();
    EXPECT_TRUE(4 == rangeIndex->GetCount());
    EXPECT_TRUE(4 == rangeIndex->DebugElementCount());

    count = 0;
    for (auto& el : model->MakeIterator())
        {
        EXPECT_TRUE(nullptr != rangeIndex->FindElement(el.GetId()));
        ++count;
        }
    
    AxisAlignedBox3d range = model->QueryModelRange();
    EXPECT_TRUE(range.IsValid());
    DPoint3d low; low.Init(1.9995000000000001, 2.0000000000000000, -0.00050000000000000001);
    DPoint3d high; high.Init(2.0005000000000002, 3.0000000000000000, 0.00050000000000000001);

    AxisAlignedBox3d box(low, high);
    AxisAlignedBox3d indexbox(rangeIndex->GetExtents().ToRange3d());

    EXPECT_TRUE(box.IsEqual(range, .00000001));
    EXPECT_TRUE(indexbox.IsEqual(range, .00001));

    // check the range of an empty 3d model
    auto model2 = LoadModel("DefaultModel");
    AxisAlignedBox3d thirdRange = model2->ToGeometricModel()->QueryModelRange();
    EXPECT_FALSE(thirdRange.IsValid());

    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "DrawingListModel");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, DgnCode(), "TestDrawing");
    DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);
    drawingModel->FillRangeIndex(); // test maintaining the range index as add elements

    InsertElement(*m_db, drawingModel->GetModelId(), false, true);
    InsertElement(*m_db, drawingModel->GetModelId(), false, true);
    InsertElement(*m_db, drawingModel->GetModelId(), false, true);
    InsertElement(*m_db, drawingModel->GetModelId(), false, true);
    rangeIndex = drawingModel->GetRangeIndex();
    EXPECT_TRUE(4 == rangeIndex->GetCount());
    EXPECT_TRUE(4 == rangeIndex->DebugElementCount());
    count = 0;
    for (auto& el : drawingModel->MakeIterator())
        {
        EXPECT_TRUE(nullptr != rangeIndex->FindElement(el.GetId()));
        ++count;
        }
    EXPECT_TRUE(count == 4);

    m_db->SaveChanges();

    AxisAlignedBox3d range2d = drawingModel->ToGeometricModel()->QueryModelRange();
    AxisAlignedBox3d indexbox2d (rangeIndex->GetExtents().ToRange3d());
    EXPECT_TRUE(box.IsEqual(range2d, .00000001));
    EXPECT_TRUE(indexbox2d.IsEqual(range, .00001));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static int countSheetModels(DgnDbR db)
    {
    int count = 0;
    auto sheetClassId = DgnClassId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SheetModel));
    for (auto const& sheet : db.Models().MakeIterator())
        {
        if (sheetClassId == sheet.GetClassId())
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
        DocumentListModelPtr sheetListModel = DgnDbTestUtils::InsertDocumentListModel(*db, "SheetListModel");
        SheetPtr sheet1 = DgnDbTestUtils::InsertSheet(*sheetListModel, scale1, height1, width1, DgnCode(), "Sheet1");
        SheetModelPtr sheetModel1 = DgnDbTestUtils::InsertSheetModel(*sheet1);
        sheetModelId1 = sheetModel1->GetModelId();

        ASSERT_EQ(1, countSheetModels(*db));
        ASSERT_NE(DgnDbStatus::Success, sheetModel1->Insert()) << "Should be illegal to INSERT a SheetModel that is already persistent";

        // Create a second sheet
        SheetPtr sheet2 = DgnDbTestUtils::InsertSheet(*sheetListModel, scale2, height2, width2, DgnCode(), "Sheet2");
        SheetModelPtr sheetModel2 = DgnDbTestUtils::InsertSheetModel(*sheet2);
        sheetModelId2 = sheetModel2->GetModelId();

        ASSERT_EQ(2, countSheetModels(*db));

        ASSERT_EQ(scale1, sheet1->GetScale());
        ASSERT_EQ(scale2, sheet2->GetScale());
        ASSERT_EQ(height1, sheet1->GetHeight());
        ASSERT_EQ(height2, sheet2->GetHeight());
        ASSERT_EQ(width1, sheet1->GetWidth());
        ASSERT_EQ(width2, sheet2->GetWidth());

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

        SheetModelPtr sheetModel1 = db->Models().Get<SheetModel>(sheetModelId1);
        ASSERT_TRUE(sheetModel1.IsValid());

        // Delete Sheet2
        ASSERT_EQ(2, countSheetModels(*db));
        DgnModelPtr sheetModel2 = db->Models().GetModel(sheetModelId2);
        ASSERT_EQ(DgnDbStatus::Success, sheetModel2->Delete());
        ASSERT_EQ(1, countSheetModels(*db));
        db->SaveChanges();
        db->CloseDb();
        }

    if (true)
        {
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, dbFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE(db.IsValid());
        ASSERT_EQ(1, countSheetModels(*db));

        // Verify that we can only place drawing elements in a sheet
        InsertElement(*db, sheetModelId1, false, true);
        InsertElement(*db, sheetModelId1, true, false);
        db->SaveChanges();
        db->CloseDb();
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
* @bsimethod                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, ModelsIterator)
    {
    SetupSeedProject();
    DgnDbR db = GetDgnDb();

    //Inserts models
    PhysicalModelPtr m1 = DgnDbTestUtils::InsertPhysicalModel(db, "Model1");
    db.SaveChanges("changeSet1");

    PhysicalModelPtr m2 = DgnDbTestUtils::InsertPhysicalModel(db, "Model2");
    PhysicalModelPtr m3 = DgnDbTestUtils::InsertPhysicalModel(db, "Model3");
    db.SaveChanges("changeSet2");

    DgnModelId m1id = m1->GetModelId();
    DgnModelId m2id = m2->GetModelId();
    DgnModelId m3id = m3->GetModelId();

    DgnModels& models = db.Models();
    DgnModels::Iterator iter = models.MakeIterator();
    int i = 0;
    for (auto const& entry : iter)
        {
        if (entry.GetModelId() == m1id)
            {
            EXPECT_EQ (m1->GetClassId().GetValue(), entry.GetClassId().GetValue());
            EXPECT_EQ (true, entry.GetInGuiList());
            i++;
            }
        else if (entry.GetModelId() == m2id)
            {
            EXPECT_EQ (m2->GetClassId().GetValue(), entry.GetClassId().GetValue());
            EXPECT_EQ(true, entry.GetInGuiList());
            i++;
            }
        else if (entry.GetModelId() == m3id)
            {
            EXPECT_EQ (m3->GetClassId().GetValue(), entry.GetClassId().GetValue());;
            EXPECT_EQ(true, entry.GetInGuiList());
            i++;
            }
        }

    EXPECT_EQ(3, i);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, AbandonChanges)
    {
    SetupSeedProject();
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
* @bsimethod                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, ReplaceInvalidCharacter)
    {
    SetupSeedProject();

    Utf8String name = "Invalid*Name";
    Utf8CP InvalidChar = "*";
    Utf8Char replace = ' ';

    bool check = DgnDbTable::IsValidName(name, InvalidChar);
    EXPECT_FALSE(check);
    DgnDbTable::ReplaceInvalidCharacters(name, InvalidChar, replace);
    EXPECT_EQ("Invalid Name", (Utf8String) name);
    check = DgnDbTable::IsValidName(name, InvalidChar);
    EXPECT_TRUE(check);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin                      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, UnitDefinitionLabel)
    {
    SetupSeedProject();
    PhysicalModelPtr model = GetDefaultPhysicalModel();

    GeometricModel::DisplayInfo const& displayInfo = model->GetDisplayInfo();
    EXPECT_STREQ("m", displayInfo.GetMasterUnits().GetLabel().c_str());
    EXPECT_STREQ("mm", displayInfo.GetSubUnits().GetLabel().c_str());
    }
