/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnModel_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <UnitTests/BackDoor/DgnProject/DgnPlatformTestDomain.h>

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
    bool            tmIs3d;
    DgnModelType    tmModelType;

    void SetTestModelProperties(WString Name, WString Desc, bool is3D, DgnModelType modType)
    {
        tmName = Name;
        tmDescription = Desc;
        tmIs3d = is3D;
        tmModelType = modType;
    };
    void IsEqual(TestModelProperties Model)
    {
        EXPECT_STREQ(tmName.c_str(), Model.tmName.c_str()) << "Names don't match";
        EXPECT_STREQ(tmDescription.c_str(), Model.tmDescription.c_str()) << "Descriptions don't match";
        EXPECT_TRUE(tmIs3d == Model.tmIs3d) << "3dness doesn't match";
        EXPECT_TRUE(tmModelType == Model.tmModelType) << "Model Types don't match";
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

    AxisAlignedBox3d range = m_model->QueryModelRange();
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

    AxisAlignedBox3d thirdRange = m_model->QueryModelRange();
    EXPECT_FALSE(thirdRange.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static int countSheets(DgnDbR db)
    {
    int count = 0;
    for (auto const& sheet : db.Models().MakeIterator("Type=1"))
        {
        DgnModelType mtype = sheet.GetModelType();
        if (mtype != DgnModelType::Sheet)
            return -1;

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
    ASSERT_EQ(2, iter.QueryCount());

    //Set up testmodel properties as we know what the models in this file contain
    TestModelProperties models[3], testModel;
    models[0].SetTestModelProperties(L"Default", L"Master Model", false, DgnModelType::Drawing);
    models[1].SetTestModelProperties(L"Model2d", L"", false, DgnModelType::Drawing);

    //Iterate through the model and verify it's contents. TODO: Add more checks
    int i = 0;
    for (DgnModels::Iterator::Entry const& entry : iter)
    {
        ASSERT_TRUE(entry.GetModelId().IsValid()) << "Model Id is not Valid";
        WString entryNameW(entry.GetName(), true);               // string conversion
        WString entryDescriptionW(entry.GetDescription(), true); // string conversion
        testModel.SetTestModelProperties(entryNameW.c_str(), entryDescriptionW.c_str(), entry.Is3d(), entry.GetModelType());
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, ImportGroups)
    {
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP();

    // ******************************
    //  Create model1
        
    DgnClassId mclassId = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr model1 = new PhysicalModel(PhysicalModel::CreateParams(*db, mclassId, "Model1"));
    ASSERT_EQ( DgnDbStatus::Success , model1->Insert() );

    // Put a group into moddel1
    ElementGroupCPtr group;
        {
        DgnCategoryId gcatid = db->Categories().QueryHighestId();
        DgnClassId gclassid = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ElementGroup));
        DgnElementCPtr groupEl = ElementGroup::Create(ElementGroup::CreateParams(*db, model1->GetModelId(), gclassid, gcatid))->Insert();
        group = dynamic_cast<ElementGroupCP>(groupEl.get());
        ASSERT_TRUE( group.IsValid() );
        }

    //  Add a member
    DgnElementCPtr member;
    if (true)
        {
        DgnClassId mclassid = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Element));
        DgnCategoryId mcatid = db->Categories().QueryHighestId();
        member = DgnElement::Create(DgnElement::CreateParams(*db, model1->GetModelId(), mclassid, mcatid, "member"))->Insert();
        ASSERT_TRUE( member.IsValid() );
        ASSERT_EQ( DgnDbStatus::Success , group->InsertMember(*member) );
        }

    checkGroupHasOneMemberInModel(*model1);

    //  ******************************
    //  Create model2 as a copy of model1

    PhysicalModelPtr model2 = new PhysicalModel(PhysicalModel::CreateParams(*db, mclassId, "Model2"));
    ASSERT_EQ( DgnDbStatus::Success , model2->Insert() );

    DgnImportContext import2(*db, *db);
    ASSERT_EQ( DgnDbStatus::Success , model2->_ImportContentsFrom(*model1, import2) );

    checkGroupHasOneMemberInModel(*model2);

    //  ******************************
    //  You can't "Import" a model into the same DgnDb
    DgnImportContext import3(*db, *db);
    DgnDbStatus stat;
    PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
    ASSERT_TRUE( !model3.IsValid() );
    ASSERT_NE( DgnDbStatus::Success , stat );
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

    PhysicalModelPtr model2 = new PhysicalModel(PhysicalModel::CreateParams(*db, mclassId, "Model2"));
    ASSERT_EQ( DgnDbStatus::Success , model2->Insert() );

    DgnImportContext import2(*db, *db);
    ASSERT_EQ( DgnDbStatus::Success , model2->_ImportContentsFrom(*model1, import2) );

    if (true)
        {
        DgnElementCPtr el = getSingleElementInModel(*model2);
        ASSERT_NE( nullptr , DgnElement::Item::GetItem(*el) );
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

    PhysicalModelPtr model2 = new PhysicalModel(PhysicalModel::CreateParams(*db, mclassId, "Model2"));
    ASSERT_EQ( DgnDbStatus::Success , model2->Insert() );

    DgnImportContext import2(*db, *db);
    ASSERT_EQ( DgnDbStatus::Success , model2->_ImportContentsFrom(*model1, import2) );

    db->SaveChanges();
    ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1 );
    TestElementDrivesElementHandler::GetHandler().Clear();
    }