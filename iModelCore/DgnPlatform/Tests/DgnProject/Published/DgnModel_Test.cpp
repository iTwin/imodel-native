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
    WString         tmDescription;

    void SetTestModelProperties(WString Name, WString Desc)
    {
        tmName = Name;
        tmDescription = Desc;
    };
    void IsEqual(TestModelProperties Model)
    {
        EXPECT_STREQ(tmName.c_str(), Model.tmName.c_str()) << "Names don't match";
        EXPECT_STREQ(tmDescription.c_str(), Model.tmDescription.c_str()) << "Descriptions don't match";
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
    DgnDbStatus status;
    DgnModels& modelTable =  m_db->Models();
    DgnModelPtr seedModel = modelTable.GetModel(m_model->GetModelId());
    DgnModelPtr newModel = seedModel->Clone(DgnModel::CreateModelCode(newName));
    status = newModel->Insert();
    EXPECT_TRUE(status == DgnDbStatus::Success);
    DgnModelId id = modelTable.QueryModelId(DgnModel::CreateModelCode(newName));
    ASSERT_TRUE(id.IsValid());
    m_model =  modelTable.GetModel (id);
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
static int countSheets(DgnDbR db)
    {
    int count = 0;
    auto sheetClassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SheetModel));
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
        gelem = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASSNAME_PhysicalObject)), cat, Placement3d()));
    else
        gelem = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationElement2d)), cat, Placement2d()));

    GeometryBuilderPtr builder = GeometryBuilder::CreateWorld(*gelem->ToGeometrySource());
    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1,0,0))));

    if (SUCCESS != builder->SetGeometryStreamAndPlacement(*gelem->ToGeometrySourceP())) // Won't catch 2d/3d mismatch from just GeometrySource as we don't know a DgnModel...
        {
        ASSERT_FALSE(expectSuccess);
        return;
        }

    DgnElementCPtr newElem = db.Elements().Insert(*gelem);
    ASSERT_EQ( expectSuccess , newElem.IsValid() && newElem->GetElementId().IsValid() );
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, SheetModelCRUD)
    {
    SetupSeedProject();

    static Utf8CP s_sheet1Name = "Sheet1";
    static Utf8CP s_sheet1NameUPPER = "SHEET1";
    static Utf8CP s_sheet2Name = "Sheet2";
    
    DPoint2d sheet1Size;
    BeFileName dbFileName;
    if (true)
        {
        DgnDbPtr db = m_db;

        ASSERT_EQ( 0 , countSheets(*db) );

        //  Create a sheet
        DPoint2d sheetSize = DPoint2d::From(.100, .100);
        SheetModel::CreateParams params(*db, DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SheetModel)),
                                DgnModel::CreateModelCode(s_sheet1Name), sheetSize);
        SheetModelPtr sheet1 = SheetModel::Create(params);
        ASSERT_TRUE( sheet1.IsValid() );
        sheet1->SetLabel("a sheet model (in mm)");
        ASSERT_EQ( DgnDbStatus::Success, sheet1->Insert() );
        ASSERT_TRUE( sheet1->GetModelId().IsValid() );

        ASSERT_EQ( 1 , countSheets(*db) );

        //  Test some insert errors
        ASSERT_NE( DgnDbStatus::Success, sheet1->Insert() ) << "Should be illegal to add sheet that is already persistent";

        SheetModelPtr sheetSameName = SheetModel::Create(params);
        ASSERT_NE( DgnDbStatus::Success, sheetSameName->Insert() ) << "Should be illegal to add a second sheet with the same name";

        //  Create a second sheet
        SheetModel::CreateParams params2(*db, DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SheetModel)),
                                DgnModel::CreateModelCode(s_sheet2Name), sheetSize);
        SheetModelPtr sheet2 = SheetModel::Create(params2);
        ASSERT_TRUE(sheet2.IsValid());
        sheet2->SetLabel("a second sheet model");
        ASSERT_EQ( DgnDbStatus::Success, sheet2->Insert() );
        ASSERT_TRUE( sheet2->GetModelId().IsValid() );
        ASSERT_NE( sheet2->GetModelId() , sheet1->GetModelId() );

        ASSERT_EQ( 2 , countSheets(*db) );
        ASSERT_TRUE( db->Models().QueryModelId(DgnModel::CreateModelCode(s_sheet2Name)).IsValid() );

        //  Look up a sheet     ... by name
        DgnModelId mid = db->Models().QueryModelId(DgnModel::CreateModelCode(s_sheet1Name));
        ASSERT_EQ( mid , sheet1->GetModelId() );
        ASSERT_EQ( mid , db->Models().QueryModelId(DgnModel::CreateModelCode(s_sheet1NameUPPER)) ) << "Sheet model names should be case-insensitive";
        //                      ... by id
        ASSERT_EQ( sheet1.get() , db->Models().Get<SheetModel>(mid).get() );
        DgnCode mcode;
        // Look up a sheet's name by id
        db->Models().GetModelCode(mcode, mid);
        ASSERT_STREQ( sheet1->GetCode().GetValueCP() , mcode.GetValueCP());

        sheet1Size = sheet1->GetSize();
        ASSERT_TRUE(sheet1Size.IsEqual(sheetSize));

        dbFileName = db->GetFileName();   
        db->SaveChanges();
        db->CloseDb();
        }

    // Verify that loading works
    if (true)
        {
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, dbFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE( db.IsValid() );

        DgnModelId mid = db->Models().QueryModelId(DgnModel::CreateModelCode(s_sheet1Name));
        SheetModelPtr sheet1 = db->Models().Get<SheetModel>(mid);
        ASSERT_TRUE( sheet1.IsValid() );
        ASSERT_EQ( mid , sheet1->GetModelId() );
        ASSERT_STREQ( s_sheet1Name , sheet1->GetCode().GetValueCP() );

        ASSERT_EQ( sheet1Size.x , sheet1->GetSize().x );
        ASSERT_EQ( sheet1Size.y , sheet1->GetSize().y );

        // Delete Sheet2
        ASSERT_EQ( 2 , countSheets(*db) );
        auto sheet2Id = db->Models().QueryModelId(DgnModel::CreateModelCode(s_sheet2Name));
        DgnModelPtr sheet2Model = db->Models().GetModel(sheet2Id);
        ASSERT_EQ( DgnDbStatus::Success, sheet2Model->Delete());
        ASSERT_EQ( 1 , countSheets(*db) );
        db->SaveChanges();
        db->CloseDb();
        }        

    if (true)
        {
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, dbFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE( db.IsValid() );

        ASSERT_EQ( 1 , countSheets(*db) );

        DgnModelId mid = db->Models().QueryModelId(DgnModel::CreateModelCode(s_sheet1Name));

        // Verify that we can only place drawing elements in a sheet
        InsertElement(*db, mid, false, true);
        InsertElement(*db, mid, true, false);
        db->SaveChanges();
        db->CloseDb();
        }
    }

/*---------------------------------------------------------------------------------**//**
* Getting the list of Dgn Models in a project and see if they work
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, WorkWithDgnModelTable)
    {
    SetupWithPrePublishedFile(L"ElementsSymbologyByLevel.ibim", L"WorkWithDgnModelTable.ibim", Db::OpenMode::ReadWrite);

    //Iterating through the models
    DgnModels& modelTable = m_db->Models();
    DgnModels::Iterator iter = modelTable.MakeIterator();
    ASSERT_EQ(4, iter.QueryCount()); // including DictionaryModel and GroupInformationModel...

    //Set up testmodel properties as we know what the models in this file contain
    TestModelProperties models[3], testModel;
    models[0].SetTestModelProperties(L"Default", L"Master Model");
    models[1].SetTestModelProperties(L"Model2d", L"");

    //Iterate through the model and verify it's contents. TODO: Add more checks
    int i = 0;
    for (DgnModels::Iterator::Entry const& entry : iter)
    {
        ASSERT_TRUE(entry.GetModelId().IsValid()) << "Model Id is not Valid";
        if ((DgnModel::DictionaryId() == entry.GetModelId()) || (DgnModel::GroupInformationId() == entry.GetModelId()))
            continue;

        WString entryNameW(entry.GetCodeValue(), true);               // string conversion
        WString entryDescriptionW(entry.GetLabel(), true); // string conversion
        testModel.SetTestModelProperties(entryNameW.c_str(), entryDescriptionW.c_str());
        testModel.IsEqual(models[i]);
        i++;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, DictionaryModel)
    {
    SetupSeedProject();
    DgnDbR db = *m_db;

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
    EXPECT_TRUE(DgnModel::Import(nullptr, dictModelR, cc).IsNull());

    // The dictionary model cannot be imported
    DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.ibim", L"3dMetricGeneralcc.ibim", DgnDb::OpenMode::ReadWrite);
    DgnImportContext importer(db, *db2);
    EXPECT_TRUE(DgnModel::Import(nullptr, dictModelR, importer).IsNull());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnModelTests, ModelsIterator)
    {
    SetupSeedProject();
    DgnDbR db = *m_db;

    DgnModelPtr seedModel = db.Models ().GetModel (db.Models ().QueryFirstModelId ());
    seedModel->FillModel ();
    EXPECT_TRUE (seedModel != nullptr);

    //Inserts models
    DgnModelPtr m1 = seedModel->Clone (DgnModel::CreateModelCode("Model1"));
    m1->SetLabel("Test Model 1");
    m1->Insert();
    db.SaveChanges ("changeSet1");

    DgnModelPtr m2 = seedModel->Clone (DgnModel::CreateModelCode("Model2"));
    m2->SetLabel("Test Model 2");
    m2->Insert ();

    DgnModelPtr m3 = seedModel->Clone (DgnModel::CreateModelCode("Model3"));
    m3->SetLabel("Test Model 3");
    m3->Insert();
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
    EXPECT_EQ (6, iter.QueryCount ()); // including the DictionaryModel and GroupInformationModel...
    DgnModels::Iterator::Entry entry = iter.begin ();
    int i = 0;
    for (auto const& entry : iter)
        {
        if (entry.GetModelId () == m1id)
            {
            EXPECT_EQ (m1->GetClassId ().GetValue (), entry.GetClassId ().GetValue ());
            EXPECT_STREQ ("Model1", entry.GetCodeValue ());
            EXPECT_STREQ ("Test Model 1", entry.GetLabel());
            EXPECT_EQ (true, entry.GetInGuiList ());
            EXPECT_STREQ(Utf8PrintfString("%" PRId64, db.Authorities().GetAuthority("DgnModels")->GetAuthorityId().GetValue()).c_str(), entry.GetCodeNamespace());
            EXPECT_TRUE(db.Authorities().QueryAuthorityId("dgn") == entry.GetCodeAuthorityId());
            }
        else if (entry.GetModelId () == m2id)
            {
            EXPECT_EQ (m2->GetClassId ().GetValue (), entry.GetClassId ().GetValue ());
            EXPECT_STREQ ("Model2", entry.GetCodeValue ());
            EXPECT_STREQ("Test Model 2", entry.GetLabel());
            EXPECT_EQ(true, entry.GetInGuiList());
            EXPECT_STREQ(Utf8PrintfString("%" PRId64, db.Authorities().GetAuthority("DgnModels")->GetAuthorityId().GetValue()).c_str(), entry.GetCodeNamespace());
            EXPECT_TRUE(db.Authorities().QueryAuthorityId("dgn") == entry.GetCodeAuthorityId());
            }
        else if (entry.GetModelId () == m3id)
            {
            EXPECT_EQ (m3->GetClassId ().GetValue (), entry.GetClassId ().GetValue ());;
            EXPECT_STREQ ("Model3", entry.GetCodeValue ());
            EXPECT_STREQ("Test Model 3", entry.GetLabel());
            EXPECT_EQ(true, entry.GetInGuiList());
            EXPECT_STREQ(Utf8PrintfString("%" PRId64, db.Authorities().GetAuthority("DgnModels")->GetAuthorityId().GetValue()).c_str(), entry.GetCodeNamespace());
            EXPECT_TRUE(db.Authorities().QueryAuthorityId("dgn") == entry.GetCodeAuthorityId());
            }
        i++;
        }
    iter.end ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnModelTests, AbandonChanges)
    {
    SetupSeedProject();
    DgnDbR db = *m_db;

    DgnModelPtr seedModel = db.Models ().GetModel (db.Models ().QueryFirstModelId ());
    seedModel->FillModel ();
    EXPECT_TRUE (seedModel != nullptr);

    //Inserts a model
    DgnModelPtr m1 = seedModel->Clone (DgnModel::CreateModelCode("Model1"));
    m1->SetLabel("Test Model 1");
    m1->Insert ();
    EXPECT_TRUE (m1 != nullptr);
    db.SaveChanges ("changeSet1");

    EXPECT_TRUE (db.Models ().QueryModelId (DgnModel::CreateModelCode("Model1")).IsValid ());
    m1->Delete ();
    EXPECT_FALSE (db.Models ().QueryModelId (DgnModel::CreateModelCode("Model1")).IsValid ());

    DgnModelPtr model2 = seedModel->Clone (DgnModel::CreateModelCode("Model2"));
    model2->Insert ();

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
    DgnDbR db = *m_db;

    DgnModelPtr seedModel = db.Models ().GetModel (db.Models ().QueryFirstModelId ());
    seedModel->FillModel ();
    EXPECT_TRUE (seedModel != nullptr);

    //Inserts a model
    DgnModelPtr m1 = seedModel->Clone (DgnModel::CreateModelCode("Model1"));
    m1->SetLabel("Test Model 1");
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
    DgnDbR db = *m_db;

    DgnModelPtr seedModel = db.Models ().GetModel (db.Models ().QueryFirstModelId ());
    seedModel->FillModel ();
    EXPECT_TRUE (seedModel != nullptr);

    //Inserts a model
    DgnModelPtr m1 = seedModel->Clone (DgnModel::CreateModelCode("Model1"));
    m1->SetLabel("Test Model 1");
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
    DgnDbR db = *m_db;

    DgnModelPtr seedModel = db.Models().GetModel(db.Models().QueryFirstModelId());
    seedModel->FillModel();
    EXPECT_TRUE(seedModel != nullptr);

    // For TFS 473760: The returned label was truncated before the fix i.e. 'm' instead of 'mm'
    // Adding the test so that this doesn't happen again
    GeometricModel::DisplayInfo const& displayInfo = seedModel->ToGeometricModel()->GetDisplayInfo();
    EXPECT_STREQ("m", displayInfo.GetMasterUnits().GetLabel().c_str());
    EXPECT_STREQ("m", displayInfo.GetSubUnits().GetLabel().c_str());
    }
