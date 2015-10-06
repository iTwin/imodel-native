/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnModel_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DPTEST

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct DgnModelTests : public testing::Test
{
    ScopedDgnHost m_autoDgnHost;
    DgnDbPtr m_dgndb;    
    DgnModelPtr m_model;

    DgnModelTests()
        {
        // Must register my domain whenever I initialize a host
        DgnPlatformTestDomain::Register(); 
        }

    void SetUp()
        {
        DgnDbTestDgnManager tdm(L"XGraphicsElements.idgndb", __FILE__, Db::OpenMode::ReadWrite);
        m_dgndb = tdm.GetDgnProjectP();
        }

    void LoadModel(Utf8CP name)
        {
        DgnModels& modelTable =  m_dgndb->Models();
        DgnModelId id = modelTable.QueryModelId(name);
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
    LoadModel("Splines");
    Utf8String name(m_model->GetModelName());
    EXPECT_TRUE(name.CompareTo("Splines")==0);
    Utf8String newName("New Long model name Longer than expectedNew Long model name Longer"
        " than expectedNew Long model name Longer than expectedNew Long model name Longer than expectedNew Long model");
    DgnDbStatus status;
    DgnModels& modelTable =  m_dgndb->Models();
    DgnModelPtr seedModel = modelTable.GetModel(m_model->GetModelId());
    DgnModelPtr newModel = seedModel->Clone(newName.c_str());
    status = newModel->Insert();
    EXPECT_TRUE(status == DgnDbStatus::Success);
    DgnModelId id = modelTable.QueryModelId(newName.c_str());
    ASSERT_TRUE(id.IsValid());
    m_model =  modelTable.GetModel (id);
    Utf8String nameToVerify(m_model->GetModelName());
    EXPECT_TRUE(newName.CompareTo(nameToVerify.c_str())==0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     07/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, EmptyList)
    {
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
    DgnDbTestDgnManager tdm(L"ModelRangeTest.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    m_dgndb = tdm.GetDgnProjectP();
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
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    m_dgndb = tdm.GetDgnProjectP();
    LoadModel("Default");

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
    DgnCategoryId cat = db.Categories().QueryHighestId();

    GeometricElementPtr gelem;
    if (is3d)
        gelem = PhysicalElement::Create(PhysicalElement::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "PhysicalElement")), cat, Placement3d()));
    else
        gelem = DrawingElement::Create(DrawingElement::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "DrawingElement")), cat, Placement2d()));

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*gelem);
    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1,0,0))));

    if (SUCCESS != builder->SetGeomStreamAndPlacement(*gelem))  // We actually catch 2d3d mismatch in SetGeomStreamAndPlacement
        {
        ASSERT_FALSE(expectSuccess);
        return;
        }

    ASSERT_EQ( expectSuccess , db.Elements().Insert(*gelem)->GetElementKey().IsValid() );
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, SheetModelCRUD)
    {
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);

    static Utf8CP s_sheet1Name = "Sheet1";
    static Utf8CP s_sheet1NameUPPER = "SHEET1";
    static Utf8CP s_sheet2Name = "Sheet2";
    
    DPoint2d sheet1Size;
    BeFileName dbFileName;
    if (true)
        {
        DgnDbPtr db = tdm.GetDgnProjectP();

        ASSERT_EQ( 0 , countSheets(*db) );

        //  Create a sheet
        DPoint2d sheetSize = DPoint2d::From(.100, .100);
        SheetModel::CreateParams params(*db, DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SheetModel)),
                                s_sheet1Name, sheetSize);
        SheetModelPtr sheet1 = SheetModel::Create(params);
        ASSERT_TRUE( sheet1.IsValid() );
        ASSERT_EQ( DgnDbStatus::Success, sheet1->Insert("a sheet model (in mm)") );
        ASSERT_TRUE( sheet1->GetModelId().IsValid() );

        ASSERT_EQ( 1 , countSheets(*db) );

        //  Test some insert errors
        ASSERT_NE( DgnDbStatus::Success, sheet1->Insert("") ) << "Should be illegal to add sheet that is already persistent";

        SheetModelPtr sheetSameName = SheetModel::Create(params);
        ASSERT_NE( DgnDbStatus::Success, sheetSameName->Insert("") ) << "Should be illegal to add a second sheet with the same name";

        //  Create a second sheet
        SheetModel::CreateParams params2(*db, DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SheetModel)),
                                s_sheet2Name, sheetSize);
        SheetModelPtr sheet2 = SheetModel::Create(params2);
        ASSERT_TRUE( sheet1.IsValid() );
        ASSERT_EQ( DgnDbStatus::Success, sheet2->Insert("a second sheet model") );
        ASSERT_TRUE( sheet2->GetModelId().IsValid() );
        ASSERT_NE( sheet2->GetModelId() , sheet1->GetModelId() );

        ASSERT_EQ( 2 , countSheets(*db) );
        ASSERT_TRUE( db->Models().QueryModelId(s_sheet2Name).IsValid() );

        //  Look up a sheet     ... by name
        DgnModelId mid = db->Models().QueryModelId(s_sheet1Name);
        ASSERT_EQ( mid , sheet1->GetModelId() );
        ASSERT_EQ( mid , db->Models().QueryModelId(s_sheet1NameUPPER) ) << "Sheet model names should be case-insensitive";
        //                      ... by id
        ASSERT_EQ( sheet1.get() , db->Models().Get<SheetModel>(mid).get() );
        Utf8String mname;
        // Look up a sheet's name by id
        db->Models().GetModelName(mname, mid);
        ASSERT_STREQ( sheet1->GetModelName() , mname.c_str() );

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

        DgnModelId mid = db->Models().QueryModelId(s_sheet1Name);
        SheetModelPtr sheet1 = db->Models().Get<SheetModel>(mid);
        ASSERT_TRUE( sheet1.IsValid() );
        ASSERT_EQ( mid , sheet1->GetModelId() );
        ASSERT_STREQ( s_sheet1Name , sheet1->GetModelName() );

        ASSERT_EQ( sheet1Size.x , sheet1->GetSize().x );
        ASSERT_EQ( sheet1Size.y , sheet1->GetSize().y );

        // Delete Sheet2
        ASSERT_EQ( 2 , countSheets(*db) );
        auto sheet2Id = db->Models().QueryModelId(s_sheet2Name);
        DgnModelPtr sheet2Model = db->Models().GetModel(sheet2Id);
        ASSERT_EQ( DgnDbStatus::Success, sheet2Model->Delete());
        ASSERT_EQ( 1 , countSheets(*db) );
        }        

    if (true)
        {
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, dbFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE( db.IsValid() );

        ASSERT_EQ( 1 , countSheets(*db) );

        DgnModelId mid = db->Models().QueryModelId(s_sheet1Name);

        // Verify that we can only place drawing elements in a sheet
        InsertElement(*db, mid, false, true);
        InsertElement(*db, mid, true, false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Getting the list of Dgn Models in a project and see if they work
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, WorkWithDgnModelTable)
{
    DgnDbTestDgnManager tdm(L"ElementsSymbologyByLevel.idgndb", __FILE__, Db::OpenMode::Readonly);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    //Iterating through the models
    DgnModels& modelTable = project->Models();
    DgnModels::Iterator iter = modelTable.MakeIterator();
    ASSERT_EQ(3, iter.QueryCount()); // including dictionary model...

    //Set up testmodel properties as we know what the models in this file contain
    TestModelProperties models[3], testModel;
    models[0].SetTestModelProperties(L"Default", L"Master Model");
    models[1].SetTestModelProperties(L"Model2d", L"");

    //Iterate through the model and verify it's contents. TODO: Add more checks
    int i = 0;
    for (DgnModels::Iterator::Entry const& entry : iter)
    {
        ASSERT_TRUE(entry.GetModelId().IsValid()) << "Model Id is not Valid";
        if (DgnModel::DictionaryId() == entry.GetModelId())
            continue;

        WString entryNameW(entry.GetName(), true);               // string conversion
        WString entryDescriptionW(entry.GetDescription(), true); // string conversion
        testModel.SetTestModelProperties(entryNameW.c_str(), entryDescriptionW.c_str());
        testModel.IsEqual(models[i]);
        i++;
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static void checkGroupHasOneMemberInModel(DgnModelR model)
    {
    BeSQLite::Statement stmt(model.GetDgnDb(), "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE (ECClassId=? AND ModelId=?)");
    stmt.BindInt64(1, model.GetDgnDb().Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ElementGroup));
    stmt.BindId(2, model.GetModelId());
    ASSERT_EQ( BE_SQLITE_ROW , stmt.Step() );
    DgnElementId gid = stmt.GetValueId<DgnElementId>(0);
    ASSERT_EQ( BE_SQLITE_DONE , stmt.Step() );

    ElementGroupCPtr group = model.GetDgnDb().Elements().Get<ElementGroup>(gid);
    ASSERT_TRUE( group.IsValid() );

    DgnElementIdSet members = group->QueryMembers();
    ASSERT_EQ( 1 , members.size() );
    DgnElementCPtr member = model.GetDgnDb().Elements().Get<DgnElement>(*members.begin());
    ASSERT_TRUE( member.IsValid() );
    ASSERT_EQ( model.GetModelId() , member->GetModelId() );
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static DgnElementCPtr getSingleElementInModel(DgnModelR model)
    {
    BeSQLite::Statement stmt(model.GetDgnDb(), "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE (ModelId=?)");
    stmt.BindId(1, model.GetModelId());
    if (BE_SQLITE_ROW != stmt.Step())   
        return nullptr;
    DgnElementId gid = stmt.GetValueId<DgnElementId>(0);
    if (BE_SQLITE_DONE != stmt.Step())
        return nullptr;

    return model.GetDgnDb().Elements().Get<DgnElement>(gid);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName copyDb (WCharCP inputFileName, WCharCP outputFileName)
    {
    BeFileName fullInputFileName;
    BeTest::GetHost().GetDocumentsRoot (fullInputFileName);
    fullInputFileName.AppendToPath (inputFileName);

    BeFileName fullOutputFileName;
    BeTest::GetHost().GetOutputRoot(fullOutputFileName);
    fullOutputFileName.AppendToPath(outputFileName);

    if (BeFileNameStatus::Success != BeFileName::BeCopyFile (fullInputFileName, fullOutputFileName))
        return BeFileName();

    return fullOutputFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void openDb (DgnDbPtr& db, BeFileNameCR name, DgnDb::OpenMode mode)
    {
    DbResult result = BE_SQLITE_OK;
    db = DgnDb::OpenDgnDb(&result, name, DgnDb::OpenParams(mode));
    ASSERT_TRUE( db.IsValid() ) << (WCharCP)WPrintfString(L"Failed to open %ls in mode %d => result=%x", name.c_str(), (int)mode, (int)result);
    ASSERT_EQ( BE_SQLITE_OK , result );
    db->Txns().EnableTracking(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static PhysicalModelPtr copyPhysicalModelSameDb(PhysicalModelCR model, Utf8CP newName)
    {
    return dynamic_cast<PhysicalModel*>(DgnModel::CopyModel(model, newName).get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static PhysicalModelPtr createPhysicalModel(DgnDbR db, Utf8CP newName)
    {
    DgnClassId mclassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr model = new PhysicalModel(PhysicalModel::CreateParams(db, mclassId, newName));
    if (!model.IsValid())
        return nullptr;
    if (DgnDbStatus::Success != model->Insert())
        return nullptr;
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static DgnDbPtr openCopyOfDb(WCharCP sourceName, WCharCP destName, DgnDb::OpenMode mode, bool importDummySchemaFirst = true)
    {
    DgnDbPtr db2;
    openDb(db2, copyDb(sourceName, destName), mode);
    if (!db2.IsValid())
        return nullptr;
    if (importDummySchemaFirst)
        DgnPlatformTestDomain::ImportDummySchema(*db2);
    DgnPlatformTestDomain::ImportSchema(*db2);
    return db2;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, ImportGroups)
    {
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP();

    // ******************************
    //  Create model1
        
    PhysicalModelPtr model1 = createPhysicalModel(*db, "Model1");
    ASSERT_TRUE( model1.IsValid() );
        {
        // Put a group into moddel1
        ElementGroupCPtr group;
            {
            DgnClassId gclassid = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ElementGroup));
            DgnElementCPtr groupEl = ElementGroup::Create(ElementGroup::CreateParams(*db, model1->GetModelId(), gclassid))->Insert();
            group = dynamic_cast<ElementGroupCP>(groupEl.get());
            ASSERT_TRUE( group.IsValid() );
            }

        //  Add a member
        if (true)
            {
            DgnClassId mclassid = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement));
            DgnCategoryId mcatid = db->Categories().QueryHighestId();
            auto member = PhysicalElement::Create(PhysicalElement::CreateParams(*db, model1->GetModelId(), mclassid, mcatid, Placement3d()))->Insert();
            //auto member = PhysicalElement::Create(*model1, mcatid)->Insert();
            ASSERT_TRUE( member.IsValid() );
            ASSERT_EQ( DgnDbStatus::Success , group->InsertMember(*member) );
            }

        checkGroupHasOneMemberInModel(*model1);
        }

    //  ******************************
    //  Create model2 as a copy of model1
    if (true)
        {
        PhysicalModelPtr model2 = copyPhysicalModelSameDb(*model1, "Model2");
        ASSERT_TRUE( model2.IsValid() );

        checkGroupHasOneMemberInModel(*model2);
        }

    //  ******************************
    //  You can't "Import" a model into the same DgnDb. For one thing, the name will conflict.
    if (true)
        {
        DgnImportContext import3(*db, *db);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE( !model3.IsValid() );
        ASSERT_NE( DgnDbStatus::Success , stat );
        }

    //  *******************************
    //  Import into separate db
    if (true)
        {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE( db2.IsValid() );

        DgnImportContext import3(*db, *db2);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE( model3.IsValid() );
        ASSERT_EQ( DgnDbStatus::Success , stat );

        checkGroupHasOneMemberInModel(*model3);
        db2->SaveChanges();
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, ImportElementsWithAuthorities)
    {
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP();

    ASSERT_EQ( DgnDbStatus::Success , DgnPlatformTestDomain::ImportSchema(*db) );

    // ******************************
    //  Create some Authorities. 
    DgnAuthorityId sourceAuthorityId;
    RefCountedPtr<NamespaceAuthority> auth1;
        {
        auto auth0 = NamespaceAuthority::CreateNamespaceAuthority("TestAuthority_NotUsed", *db);
        auth1 = NamespaceAuthority::CreateNamespaceAuthority("TestAuthority", *db);
        auto auth2 = NamespaceAuthority::CreateNamespaceAuthority("TestAuthority_AlsoNotUsed", *db);
        ASSERT_EQ(DgnDbStatus::Success, auth0->Insert());
        ASSERT_EQ(DgnDbStatus::Success, auth1->Insert());
        ASSERT_EQ(DgnDbStatus::Success, auth2->Insert());
        
        // We'll use the *second one*, so that the source and destination authority IDs will be different.
        sourceAuthorityId = auth1->GetAuthorityId();
        }

    // ******************************
    //  Create model1
        
    DgnClassId mclassId = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr model1 = new PhysicalModel(PhysicalModel::CreateParams(*db, mclassId, "Model1"));
    ASSERT_EQ( DgnDbStatus::Success , model1->Insert() );

    // Put an element with an Item into moddel1
        {
        DgnElement::Code code = auth1->CreateCode("TestElement");
        DgnCategoryId gcatid = db->Categories().QueryHighestId();
        TestElementPtr tempEl = TestElement::Create(*db, model1->GetModelId(), gcatid, code);
        DgnElement::Item::SetItem(*tempEl, *TestItem::Create("Line"));
        ASSERT_TRUE( db->Elements().Insert(*tempEl).IsValid() );
        db->SaveChanges();
        }

    if (true)
        {
        DgnElementCPtr el = getSingleElementInModel(*model1);
        ASSERT_TRUE( el.IsValid() );
        DgnAuthorityId said = el->GetCode().GetAuthority();
        ASSERT_TRUE( said == sourceAuthorityId );
        auto sourceAuthority = db->Authorities().GetAuthority(sourceAuthorityId);
        ASSERT_STREQ( sourceAuthority->GetName().c_str(), "TestAuthority" );
        }

    //  *******************************
    //  Import model1 into separate db
    if (true)
        {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE( db2.IsValid() );

        DgnImportContext import3(*db, *db2);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE( model3.IsValid() );
        ASSERT_EQ( DgnDbStatus::Success , stat );

        DgnElementCPtr el = getSingleElementInModel(*model3);
        ASSERT_TRUE( el.IsValid() );

        // Verify that Authority was copied over
        DgnAuthorityId daid = el->GetCode().GetAuthority();
        ASSERT_TRUE( daid.IsValid() );
        ASSERT_NE( daid , sourceAuthorityId ) << "Authority ID should have been remapped";
        auto destAuthority = db2->Authorities().GetAuthority(daid);
        ASSERT_STREQ( destAuthority->GetName().c_str(), "TestAuthority" );
        db2->SaveChanges();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, ImportElementsWithItems)
    {
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP();

    ASSERT_EQ( DgnDbStatus::Success , DgnPlatformTestDomain::ImportSchema(*db) );

    // ******************************
    //  Create model1
        
    DgnClassId mclassId = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr model1 = new PhysicalModel(PhysicalModel::CreateParams(*db, mclassId, "Model1"));
    ASSERT_EQ( DgnDbStatus::Success , model1->Insert() );

    // Put an element with an Item into moddel1
        {
        DgnCategoryId gcatid = db->Categories().QueryHighestId();
        TestElementPtr tempEl = TestElement::Create(*db, model1->GetModelId(), gcatid, "TestElement");
        DgnElement::Item::SetItem(*tempEl, *TestItem::Create("Line"));
        ASSERT_TRUE( db->Elements().Insert(*tempEl).IsValid() );
        db->SaveChanges();
        }

    if (true)
        {
        DgnElementCPtr el = getSingleElementInModel(*model1);
        ASSERT_NE( nullptr , DgnElement::Item::GetItem(*el) );
        }

    //  ******************************
    //  Create model2 as a copy of model1
    if (true)
        {
        PhysicalModelPtr model2 = copyPhysicalModelSameDb(*model1, "Model2");
        ASSERT_TRUE( model2.IsValid() );

        DgnElementCPtr el = getSingleElementInModel(*model2);
        ASSERT_NE( nullptr , DgnElement::Item::GetItem(*el) );
        }

    //  *******************************
    //  Import into separate db
    if (true)
        {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE( db2.IsValid() );

        DgnImportContext import3(*db, *db2);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE( model3.IsValid() );
        ASSERT_EQ( DgnDbStatus::Success , stat );

        DgnElementCPtr el = getSingleElementInModel(*model3);
        ASSERT_NE( nullptr , DgnElement::Item::GetItem(*el) );
        db2->SaveChanges();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, ImportElementsWithDependencies)
    {
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP();
    db->Txns().EnableTracking(true);

    ASSERT_EQ( DgnDbStatus::Success , DgnPlatformTestDomain::ImportSchema(*db) );

    TestElementDrivesElementHandler::GetHandler().Clear();

    // ******************************
    //  Create model1
        
    DgnClassId mclassId = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr model1 = new PhysicalModel(PhysicalModel::CreateParams(*db, mclassId, "Model1"));
    ASSERT_EQ( DgnDbStatus::Success , model1->Insert() );

    // Create 2 elements and make the first depend on the second
        {
        DgnCategoryId gcatid = db->Categories().QueryHighestId();

        TestElementPtr e1 = TestElement::Create(*db, model1->GetModelId(), gcatid, "e1");
        ASSERT_TRUE( db->Elements().Insert(*e1).IsValid() );

        TestElementPtr e2 = TestElement::Create(*db, model1->GetModelId(), gcatid, "e2");
        ASSERT_TRUE( db->Elements().Insert(*e2).IsValid() );

        TestElementDrivesElementHandler::Insert(*db, e2->GetElementId(), e1->GetElementId());

        db->SaveChanges();

        ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1 );
        TestElementDrivesElementHandler::GetHandler().Clear();
        }

    //  ******************************
    //  Create model2 as a copy of model1
    if (true)
        {
        PhysicalModelPtr model2 = copyPhysicalModelSameDb(*model1, "Model2");
        ASSERT_TRUE( model2.IsValid() );

        db->SaveChanges();
        ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1 );
        TestElementDrivesElementHandler::GetHandler().Clear();
        }

    //  *******************************
    //  Import into separate db
    if (true)
        {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE( db2.IsValid() );

        DgnImportContext import3(*db, *db2);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE( model3.IsValid() );
        ASSERT_EQ( DgnDbStatus::Success , stat );

        db2->SaveChanges();
        ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1 );
        TestElementDrivesElementHandler::GetHandler().Clear();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTests, DictionaryModel)
    {
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbR db = *tdm.GetDgnProjectP();

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
    DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
    DgnImportContext importer(db, *db2);
    EXPECT_TRUE(DgnModel::Import(nullptr, dictModelR, importer).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnModelTests, ModelsIterator)
    {
    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP ();
    ASSERT_TRUE (db != nullptr);

    DgnModelPtr seedModel = db->Models ().GetModel (db->Models ().QueryFirstModelId ());
    seedModel->FillModel ();
    EXPECT_TRUE (seedModel != nullptr);

    //Inserts models
    DgnModelPtr M1 = seedModel->Clone ("Model1");
    M1->Insert ("Test Model 1");
    Utf8CP m1_name = M1->GetModelName ();
    db->SaveChanges ("changeSet1");

    DgnModelPtr M2 = seedModel->Clone ("Model2");
    M2->Insert ("Test Model 2", true);

    DgnModelPtr M3 = seedModel->Clone ("Model3");
    M3->Insert ("Test Model 3", true);
    db->SaveChanges ("changeSet1");

    EXPECT_TRUE (db->Models ().QueryModelId ("Model1").IsValid ());
    EXPECT_TRUE (db->Models ().QueryModelId ("Model2").IsValid ());
    EXPECT_TRUE (db->Models ().QueryModelId ("Model3").IsValid ());

    DgnModelId m1id = db->Models ().QueryModelId ("Model1");
    DgnModelId m2id = db->Models ().QueryModelId ("Model2");
    DgnModelId m3id = db->Models ().QueryModelId ("Model3");

    BentleyStatus ModelName = db->Models ().GetModelName ((Utf8StringR)m1_name, m1id);
    EXPECT_EQ (0, ModelName);

    DgnModels& models = db->Models ();
    DgnModels::Iterator iter = models.MakeIterator ();
    EXPECT_EQ (5, iter.QueryCount ()); // including the dictionary model...
    DgnModels::Iterator::Entry entry = iter.begin ();
    int i = 0;
    for (auto const& entry : iter)
        {
        if (entry.GetModelId () == m1id)
            {
            EXPECT_EQ (M1->GetClassId ().GetValue (), entry.GetClassId ().GetValue ());
            EXPECT_STREQ ("Model1", entry.GetName ());
            EXPECT_STREQ ("Test Model 1", entry.GetDescription ());
            EXPECT_EQ (true, entry.InGuiList ());
            }
        else if (entry.GetModelId () == m2id)
            {
            EXPECT_EQ (M2->GetClassId ().GetValue (), entry.GetClassId ().GetValue ());
            EXPECT_STREQ ("Model2", entry.GetName ());
            EXPECT_STREQ ("Test Model 2", entry.GetDescription ());
            EXPECT_EQ (true, entry.InGuiList ());
            }
        else if (entry.GetModelId () == m3id)
            {
            EXPECT_EQ (M3->GetClassId ().GetValue (), entry.GetClassId ().GetValue ());;
            EXPECT_STREQ ("Model3", entry.GetName ());
            EXPECT_STREQ ("Test Model 3", entry.GetDescription ());
            EXPECT_EQ (true, entry.InGuiList ());
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
    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP ();
    ASSERT_TRUE (db != nullptr);

    DgnModelPtr seedModel = db->Models ().GetModel (db->Models ().QueryFirstModelId ());
    seedModel->FillModel ();
    EXPECT_TRUE (seedModel != nullptr);

    //Inserts a model
    DgnModelPtr M1 = seedModel->Clone ("Model1");
    M1->Insert ("Test Model 1");
    EXPECT_TRUE (M1 != nullptr);
    db->SaveChanges ("changeSet1");

    EXPECT_TRUE (db->Models ().QueryModelId ("Model1").IsValid ());
    M1->Delete ();
    EXPECT_FALSE (db->Models ().QueryModelId ("Model1").IsValid ());

    DgnModelPtr model2 = seedModel->Clone ("Model2");
    model2->Insert ();

    //Model 1 should be back. Model 2 shouldnt be in the db anymore.
    DbResult rzlt = db->AbandonChanges ();
    EXPECT_TRUE (rzlt == 0);
    EXPECT_TRUE (db->Models ().QueryModelId ("Model1").IsValid ());
    EXPECT_FALSE (db->Models ().QueryModelId ("Model2").IsValid ());
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
    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP ();
    ASSERT_TRUE (db != nullptr);

    DgnModelPtr seedModel = db->Models ().GetModel (db->Models ().QueryFirstModelId ());
    seedModel->FillModel ();
    EXPECT_TRUE (seedModel != nullptr);

    //Inserts a model
    DgnModelPtr M1 = seedModel->Clone ("Model1");
    M1->Insert ("Test Model 1");
    EXPECT_TRUE (M1 != nullptr);
    EXPECT_TRUE (db->Models ().QueryModelId ("Model1").IsValid ());

    static DgnModel::AppData::Key m_key;
    TestAppData *m_AppData = new TestAppData ();
    M1->AddAppData (m_key, m_AppData);
    M1->FillModel ();
    EXPECT_TRUE (m_AppData->isFilled);
    EXPECT_TRUE (M1->FindAppData (m_key) != nullptr);
    }

/*---------------------------------------------------------------------------------**//**
//! Test for dropping AppData from a model.
* @bsimethod                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnModelTests, DropAppData)
    {
    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP ();
    ASSERT_TRUE (db != nullptr);

    DgnModelPtr seedModel = db->Models ().GetModel (db->Models ().QueryFirstModelId ());
    seedModel->FillModel ();
    EXPECT_TRUE (seedModel != nullptr);

    //Inserts a model
    DgnModelPtr M1 = seedModel->Clone ("Model1");
    M1->Insert ("Test Model 1");
    EXPECT_TRUE (M1 != nullptr);
    EXPECT_TRUE (db->Models ().QueryModelId ("Model1").IsValid ());

    static DgnModel::AppData::Key m_key;
    TestAppData *m_AppData = new TestAppData ();
    M1->AddAppData (m_key, m_AppData);
    M1->FillModel ();
    EXPECT_TRUE (m_AppData->isFilled);
    StatusInt status = M1->DropAppData (m_key);
    EXPECT_TRUE (status == 0);
    EXPECT_TRUE (M1->FindAppData (m_key) == nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnModelTests, ReplaceInvalidCharacter)
    {
    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP ();
    ASSERT_TRUE (db != nullptr);

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

