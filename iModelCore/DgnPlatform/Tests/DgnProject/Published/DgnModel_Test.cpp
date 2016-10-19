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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static DgnDbPtr openCopyOfDb(WCharCP sourceName, WCharCP destName, DgnDb::OpenMode mode, bool importDummySchemaFirst = true)
    {
    DgnDbPtr db2;
    DgnDbTestFixture::OpenDb(db2, DgnDbTestFixture::CopyDb(sourceName, destName), mode);
    if (!db2.IsValid())
        return nullptr;
    if (importDummySchemaFirst)
        DgnPlatformTestDomain::ImportDummySchema(*db2);
    DgnPlatformTestDomain::ImportSchema(*db2);
    return db2;
    }

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct DgnModelTests : public DgnDbTestFixture
{
    DgnModelPtr m_model;
    
    void LoadModel(Utf8CP name)
        {
        DgnModels& modelTable =  m_db->Models();
        DgnModelId id = modelTable.QueryModelId(DgnModel::CreateModelCode(name));
        m_model =  modelTable.GetModel(id);
        if (m_model.IsValid())
            m_model->FillModel();
        }

    void InsertElement(DgnDbR, DgnModelId, bool is3d, bool expectSuccess);
};

//=======================================================================================
// @bsiclass                                                    Majd.Uddin   04/12
//=======================================================================================
struct TestModelProperties
{
public:
    DgnModelId      tmId;
    WString         tmName;

    void SetTestModelProperties(WString Name)
    {
        tmName = Name;
    };
    void IsEqual(TestModelProperties Model)
    {
        EXPECT_STREQ(tmName.c_str(), Model.tmName.c_str()) << "Names don't match";
    };
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetGraphicElements)
    {
    SetupWithPrePublishedFile(L"XGraphicsElements.ibim", L"GetGraphicElements.ibim", Db::OpenMode::ReadWrite);
    LoadModel("Splines");
    uint32_t graphicElementCount = (uint32_t) m_model->GetElements().size();
    ASSERT_NE(graphicElementCount, 0);
    ASSERT_TRUE(graphicElementCount > 0)<<"Please provide model with graphics elements, otherwise this test case makes no sense";
    int count = 0;
    for (auto const& elm : *m_model)
        {
        EXPECT_TRUE(elm.second->GetModel() == m_model);
        ++count;
        }
    EXPECT_EQ(graphicElementCount, count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetName)
    {
    SetupWithPrePublishedFile(L"XGraphicsElements.ibim", L"GetName.ibim", Db::OpenMode::ReadWrite);
    LoadModel("Splines");
    Utf8String name = m_model->GetCode().GetValue();
    EXPECT_TRUE(name.CompareTo("Splines")==0);
    Utf8String newName("New Long model name Longer than expectedNew Long model name Longer"
        " than expectedNew Long model name Longer than expectedNew Long model name Longer than expectedNew Long model");
    PhysicalModelPtr newModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, DgnModel::CreateModelCode(newName));
    DgnModelId id = m_db->Models().QueryModelId(DgnModel::CreateModelCode(newName));
    ASSERT_TRUE(id.IsValid());
    m_model = m_db->Models().GetModel (id);
    Utf8String nameToVerify = m_model->GetCode().GetValue();
    EXPECT_TRUE(newName.CompareTo(nameToVerify.c_str())==0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     07/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, EmptyList)
    {
    SetupWithPrePublishedFile(L"XGraphicsElements.ibim", L"EmptyList.ibim", Db::OpenMode::ReadWrite);
    LoadModel("Splines");
    ASSERT_TRUE(0 != m_model->GetElements().size());
    m_model->EmptyModel();
    ASSERT_TRUE(0 == m_model->GetElements().size());

    m_model->FillModel();
    ASSERT_TRUE(0 != m_model->GetElements().size());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     07/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetRange)
    {
    SetupWithPrePublishedFile(L"ModelRangeTest.ibim", L"GetRange.ibim", Db::OpenMode::ReadWrite);
    LoadModel("RangeTest");

    AxisAlignedBox3d range = m_model->ToGeometricModel()->QueryModelRange();
    EXPECT_TRUE(range.IsValid());
    DPoint3d low; low.Init(-1.4011580427821895, 0.11538461538461531, -0.00050000000000000001);
    DPoint3d high; high.Init(-0.59795039550813156, 0.60280769230769227, 0.00050000000000000001);
    AxisAlignedBox3d box(low,high);

    EXPECT_TRUE(box.IsEqual(range,.00000001));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     07/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetRangeOfEmptyModel)
    {
    SetupSeedProject();
    LoadModel("DefaultModel");

    AxisAlignedBox3d thirdRange = m_model->ToGeometricModel()->QueryModelRange();
    EXPECT_FALSE(thirdRange.IsValid());
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
void DgnModelTests::InsertElement(DgnDbR db,   DgnModelId mid, bool is3d, bool expectSuccess)
    {
    DgnCategoryId cat = DgnCategory::QueryHighestCategoryId(db);

    DgnElementPtr gelem;
    if (is3d)
        gelem = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject)), cat, Placement3d()));
    else
        gelem = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AnnotationElement2d)), cat, Placement2d()));

    GeometryBuilderPtr builder = GeometryBuilder::Create(*gelem->ToGeometrySource());
    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1,0,0))));

    if (SUCCESS != builder->Finish(*gelem->ToGeometrySourceP())) // Won't catch 2d/3d mismatch from just GeometrySource as we don't know a DgnModel...
        {
        ASSERT_FALSE(expectSuccess);
        return;
        }

    DgnElementCPtr newElem = db.Elements().Insert(*gelem);
    ASSERT_EQ(expectSuccess, newElem.IsValid() && newElem->GetElementId().IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, SheetModelCRUD)
    {
    SetupSeedProject();

    static Utf8CP s_sheetModel1Name = "SheetModel1";
    static Utf8CP s_sheetModel1NameUPPER = "SHEETMODEL1";
    static Utf8CP s_sheetModel2Name = "SheetModel2";
    
    DPoint2d sheet1Size;
    BeFileName dbFileName;
    
    if (true)
        {
        DgnDbPtr db = m_db;
        ASSERT_EQ(0, countSheetModels(*db));

        // Create a sheet
        DocumentListModelPtr sheetListModel = DgnDbTestUtils::InsertDocumentListModel(*db, DgnModel::CreateModelCode("SheetListModel"));
        DPoint2d sheetSize = DPoint2d::From(.100, .100);
        SheetPtr sheet1 = DgnDbTestUtils::InsertSheet(*sheetListModel, DgnCode(), "Sheet1");
        SheetModelPtr sheetModel1 = DgnDbTestUtils::InsertSheetModel(*sheet1, DgnModel::CreateModelCode(s_sheetModel1Name), sheetSize);

        ASSERT_EQ(1, countSheetModels(*db));
        ASSERT_NE(DgnDbStatus::Success, sheetModel1->Insert()) << "Should be illegal to INSERT a SheetModel that is already persistent";

        DgnModelId modelId = db->Models().QueryModelId(DgnModel::CreateModelCode(s_sheetModel1Name));
        ASSERT_EQ(modelId, sheetModel1->GetModelId());
        ASSERT_EQ(modelId, db->Models().QueryModelId(DgnModel::CreateModelCode(s_sheetModel1NameUPPER))) << "Sheet model names should be case-insensitive";
        ASSERT_EQ(sheetModel1.get(), db->Models().Get<SheetModel>(modelId).get());

        DgnCode modelCode;
        db->Models().GetModelCode(modelCode, modelId);
        ASSERT_STREQ(sheetModel1->GetCode().GetValueCP(), modelCode.GetValueCP());

        sheet1Size = sheetModel1->GetSize();
        ASSERT_TRUE(sheet1Size.IsEqual(sheetSize));

        // Create a second sheet
        SheetPtr sheet2 = DgnDbTestUtils::InsertSheet(*sheetListModel, DgnCode(), "Sheet2");
        SheetModelPtr sheetModel2 = DgnDbTestUtils::InsertSheetModel(*sheet2, DgnModel::CreateModelCode(s_sheetModel2Name), sheetSize);

        ASSERT_EQ(2, countSheetModels(*db));
        ASSERT_TRUE(db->Models().QueryModelId(DgnModel::CreateModelCode(s_sheetModel2Name)).IsValid());
        ASSERT_NE(sheetModel2->GetModelId(), sheetModel1->GetModelId());

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

        DgnModelId modelId = db->Models().QueryModelId(DgnModel::CreateModelCode(s_sheetModel1Name));
        SheetModelPtr sheetModel1 = db->Models().Get<SheetModel>(modelId);
        ASSERT_TRUE(sheetModel1.IsValid());
        ASSERT_EQ(modelId, sheetModel1->GetModelId());
        ASSERT_STREQ(s_sheetModel1Name, sheetModel1->GetCode().GetValueCP());

        ASSERT_EQ(sheet1Size.x, sheetModel1->GetSize().x);
        ASSERT_EQ(sheet1Size.y, sheetModel1->GetSize().y);

        // Delete Sheet2
        ASSERT_EQ(2, countSheetModels(*db));
        DgnModelId sheetModel2Id = db->Models().QueryModelId(DgnModel::CreateModelCode(s_sheetModel2Name));
        DgnModelPtr sheet2Model = db->Models().GetModel(sheetModel2Id);
        ASSERT_EQ(DgnDbStatus::Success, sheet2Model->Delete());
        ASSERT_EQ(1, countSheetModels(*db));
        db->SaveChanges();
        db->CloseDb();
        }        

    if (true)
        {
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, dbFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE(db.IsValid());
        ASSERT_EQ(1, countSheetModels(*db));

        DgnModelId modelId = db->Models().QueryModelId(DgnModel::CreateModelCode(s_sheetModel1Name));

        // Verify that we can only place drawing elements in a sheet
        InsertElement(*db, modelId, false, true);
        InsertElement(*db, modelId, true, false);
        db->SaveChanges();
        db->CloseDb();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, DictionaryModel)
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

    // The dictionary model cannot be imported
    DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.ibim", L"3dMetricGeneralcc.ibim", DgnDb::OpenMode::ReadWrite);
    DgnImportContext importer(db, *db2);
    DefinitionPartitionCPtr partitionForImport = DefinitionPartition::CreateAndInsert(*db2->Elements().GetRootSubject(), "PartitionForImport");
    EXPECT_TRUE(partitionForImport.IsValid());
    EXPECT_TRUE(DgnModel::Import(nullptr, dictModelR, importer, *partitionForImport).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnModelTests, ModelsIterator)
    {
    SetupSeedProject();
    DgnDbR db = GetDgnDb();

    //Inserts models
    PhysicalModelPtr m1 = InsertPhysicalModel("Model1");
    db.SaveChanges ("changeSet1");

    PhysicalModelPtr m2 = InsertPhysicalModel("Model2");
    PhysicalModelPtr m3 = InsertPhysicalModel("Model3");
    db.SaveChanges ("changeSet1");

    EXPECT_TRUE (db.Models ().QueryModelId (DgnModel::CreateModelCode("Model1")).IsValid ());
    EXPECT_TRUE (db.Models ().QueryModelId (DgnModel::CreateModelCode("Model2")).IsValid ());
    EXPECT_TRUE (db.Models ().QueryModelId (DgnModel::CreateModelCode("Model3")).IsValid ());

    DgnModelId m1id = db.Models ().QueryModelId (DgnModel::CreateModelCode("Model1"));
    DgnModelId m2id = db.Models ().QueryModelId (DgnModel::CreateModelCode("Model2"));
    DgnModelId m3id = db.Models ().QueryModelId (DgnModel::CreateModelCode("Model3"));

    DgnCode m1_code;
    BentleyStatus ModelName = db.Models ().GetModelCode (m1_code, m1id);
    EXPECT_EQ (0, ModelName);

    DgnModels& models = db.Models ();
    DgnModels::Iterator iter = models.MakeIterator ();
    int i = 0;
    for (auto const& entry : iter)
        {
        if (entry.GetModelId () == m1id)
            {
            EXPECT_EQ (m1->GetClassId ().GetValue (), entry.GetClassId ().GetValue ());
            EXPECT_STREQ ("Model1", entry.GetCodeValue ());
            EXPECT_EQ (true, entry.GetInGuiList ());
            EXPECT_STREQ(Utf8PrintfString("%" PRId64, db.Authorities().GetAuthority("DgnModels")->GetAuthorityId().GetValue()).c_str(), entry.GetCodeNamespace());
            EXPECT_TRUE(db.Authorities().QueryAuthorityId("dgn") == entry.GetCodeAuthorityId());
            i++;
            }
        else if (entry.GetModelId () == m2id)
            {
            EXPECT_EQ (m2->GetClassId ().GetValue (), entry.GetClassId ().GetValue ());
            EXPECT_STREQ ("Model2", entry.GetCodeValue ());
            EXPECT_EQ(true, entry.GetInGuiList());
            EXPECT_STREQ(Utf8PrintfString("%" PRId64, db.Authorities().GetAuthority("DgnModels")->GetAuthorityId().GetValue()).c_str(), entry.GetCodeNamespace());
            EXPECT_TRUE(db.Authorities().QueryAuthorityId("dgn") == entry.GetCodeAuthorityId());
            i++;
            }
        else if (entry.GetModelId () == m3id)
            {
            EXPECT_EQ (m3->GetClassId ().GetValue (), entry.GetClassId ().GetValue ());;
            EXPECT_STREQ ("Model3", entry.GetCodeValue ());
            EXPECT_EQ(true, entry.GetInGuiList());
            EXPECT_STREQ(Utf8PrintfString("%" PRId64, db.Authorities().GetAuthority("DgnModels")->GetAuthorityId().GetValue()).c_str(), entry.GetCodeNamespace());
            EXPECT_TRUE(db.Authorities().QueryAuthorityId("dgn") == entry.GetCodeAuthorityId());
            i++;
            }
        }

    EXPECT_EQ(3, i);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnModelTests, AbandonChanges)
    {
    SetupSeedProject();
    DgnDbR db = GetDgnDb();

    //Inserts a model
    PhysicalModelPtr m1 = InsertPhysicalModel("Model1");
    EXPECT_TRUE(m1.IsValid());
    db.SaveChanges ("changeSet1");

    EXPECT_TRUE (db.Models().QueryModelId (DgnModel::CreateModelCode("Model1")).IsValid());
    m1->Delete ();
    EXPECT_FALSE (db.Models().QueryModelId (DgnModel::CreateModelCode("Model1")).IsValid());

    PhysicalModelPtr model2 = InsertPhysicalModel("Model2");

    //Model 1 should be back. Model 2 shouldnt be in the db anymore.
    DbResult rzlt = db.AbandonChanges ();
    EXPECT_TRUE (rzlt == 0);
    EXPECT_TRUE (db.Models ().QueryModelId (DgnModel::CreateModelCode("Model1")).IsValid ());
    EXPECT_FALSE (db.Models ().QueryModelId (DgnModel::CreateModelCode("Model2")).IsValid ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestAppData : DgnModel::AppData
    {
    bool isFilled;
    virtual DropMe _OnFilled (DgnModelCR model) override
        {
        isFilled = true;
        return DropMe::No;
        }
    };

/*---------------------------------------------------------------------------------**//**
//! Test for adding AppData on a model.
* @bsimethod                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnModelTests, AddAppData)
    {
    SetupSeedProject();
    DgnDbR db = GetDgnDb();

    //Inserts a model
    PhysicalModelPtr m1 = InsertPhysicalModel("Model1");
    m1->Insert ();
    EXPECT_TRUE (m1 != nullptr);
    EXPECT_TRUE (db.Models ().QueryModelId (DgnModel::CreateModelCode("Model1")).IsValid ());

    // Add Appdata
    static DgnModel::AppData::Key key;
    TestAppData *AppData = new TestAppData ();
    m1->AddAppData (key, AppData);
    m1->FillModel ();
    EXPECT_TRUE (AppData->isFilled);
    EXPECT_TRUE (m1->FindAppData (key) != nullptr);

   // Add appdata again with same key
    //TestAppData *AppData2 = new TestAppData();
    //m1->AddAppData(key, AppData2);

    }

/*---------------------------------------------------------------------------------**//**
//! Test for dropping AppData from a model.
* @bsimethod                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnModelTests, DropAppData)
    {
    SetupSeedProject();
    DgnDbR db = GetDgnDb();

    //Inserts a model
    PhysicalModelPtr m1 = InsertPhysicalModel("Model1");
    m1->Insert();
    EXPECT_TRUE (m1 != nullptr);
    EXPECT_TRUE (db.Models ().QueryModelId (DgnModel::CreateModelCode("Model1")).IsValid ());

    static DgnModel::AppData::Key m_key;
    TestAppData *m_AppData = new TestAppData ();
    m1->AddAppData (m_key, m_AppData);
    m1->FillModel ();
    EXPECT_TRUE (m_AppData->isFilled);
    StatusInt status = m1->DropAppData (m_key);
    EXPECT_TRUE (status == 0);
    EXPECT_TRUE (m1->FindAppData (m_key) == nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnModelTests, ReplaceInvalidCharacter)
    {
    SetupSeedProject();

    Utf8String name = "Invalid*Name";
    Utf8CP InvalidChar = "*";
    Utf8Char replace = ' ';

    bool check = DgnDbTable::IsValidName (name, InvalidChar);
    EXPECT_FALSE (check);
    DgnDbTable::ReplaceInvalidCharacters (name, InvalidChar, replace);
    EXPECT_EQ ("Invalid Name", (Utf8String)name);
    check = DgnDbTable::IsValidName (name, InvalidChar);
    EXPECT_TRUE (check);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin                      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, UnitDefinitionLabel)
    {
    SetupSeedProject();
    PhysicalModelPtr model = GetDefaultPhysicalModel();

    // For TFS 473760: The returned label was truncated before the fix i.e. 'm' instead of 'mm'
    // Adding the test so that this doesn't happen again
    GeometricModel::DisplayInfo const& displayInfo = model->GetDisplayInfo();
    EXPECT_STREQ("m", displayInfo.GetMasterUnits().GetLabel().c_str());
    EXPECT_STREQ("m", displayInfo.GetSubUnits().GetLabel().c_str());
    }
