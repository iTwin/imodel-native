/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnModel_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_SQLITE

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct DgnModelTests : public testing::Test
    {
     public:
        ScopedDgnHost m_autoDgnHost;
        DgnDbPtr m_dgndb;    
        DgnModelP m_modelP;
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Julija Suboc     07/13
        // Prepares test data file
        //---------------------------------------------------------------------------------------
        void SetUp()
            {
            DgnDbTestDgnManager tdm(L"XGraphicsElements.idgndb", __FILE__, Db::OPEN_ReadWrite);
            m_dgndb = tdm.GetDgnProjectP();
           
            }
         //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Julija Suboc     07/13
        //---------------------------------------------------------------------------------------
        void LoadModel(Utf8CP name)
            {
            DgnModels& modelTable =  m_dgndb->Models();
            DgnModelId id = modelTable.QueryModelId(name);
            m_modelP =  modelTable.GetModel(id);
            if (m_modelP)
                m_modelP->FillModel();
            }

        void InsertElement(DgnDbR, DgnModelId, bool is3d, bool expectSuccess);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetGraphicElements)
    {
    LoadModel("Splines");
    uint32_t graphicElementCount = m_modelP->CountElements();
    ASSERT_NE(graphicElementCount, 0);
    ASSERT_TRUE(graphicElementCount > 0)<<"Please provide model with graphics elements, otherwise this test case makes no sense";
    int count = 0;
    for (auto const& elm : *m_modelP)
        {
        EXPECT_TRUE(&elm.second->GetDgnModel() == m_modelP);
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
    Utf8String name(m_modelP->GetModelName());
    EXPECT_TRUE(name.CompareTo("Splines")==0);
    Utf8String newName("New Long model name Longer than expectedNew Long model name Longer"
        " than expectedNew Long model name Longer than expectedNew Long model name Longer than expectedNew Long model");
    DgnDbStatus status;
    DgnModels& modelTable =  m_dgndb->Models();
    modelTable.CreateNewModelFromSeed(&status, newName.c_str(), m_modelP->GetModelId());
    EXPECT_TRUE(status == DgnDbStatus::Success)<<"Failed to create model";
    DgnModelId id = modelTable.QueryModelId(newName.c_str());
    ASSERT_TRUE(id.IsValid());
    m_modelP =  modelTable.GetModel (id);
    Utf8String nameToVerify(m_modelP->GetModelName());
    EXPECT_TRUE(newName.CompareTo(nameToVerify.c_str())==0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     07/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, EmptyList)
    {
    LoadModel("Splines");
    m_modelP->Empty();
    ASSERT_EQ(0, m_modelP->CountElements())<<"Failed to empty element list in model";
    LoadModel("Splines");
    ASSERT_EQ(0, m_modelP->CountElements())<<"Failed to empty element list in model";
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     07/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetRange)
    {
    DgnDbTestDgnManager tdm(L"ModelRangeTest.idgndb", __FILE__, Db::OPEN_ReadWrite);
    m_dgndb = tdm.GetDgnProjectP();
    LoadModel("RangeTest");

    AxisAlignedBox3d range = m_modelP->QueryModelRange();
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
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OPEN_ReadWrite);
    m_dgndb = tdm.GetDgnProjectP();
    LoadModel("Default");

    AxisAlignedBox3d thirdRange = m_modelP->QueryModelRange();
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
    DgnModelP model = db.Models().GetModel(mid);

    DgnCategoryId cat = db.Categories().QueryHighestId();

    GeometricElementPtr gelem;
    if (is3d)
        gelem = PhysicalElement::Create(PhysicalElement::CreateParams(*model, DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "PhysicalElement")), cat, Placement3d()));
    else
        gelem = DrawingElement::Create(DrawingElement::CreateParams(*model, DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "DrawingElement")), cat, Placement2d()));

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
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OPEN_ReadWrite);

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
        ASSERT_EQ( DgnDbStatus::Success, db->Models().Insert(*sheet1, "a sheet model (in mm)") );
        ASSERT_TRUE( sheet1->GetModelId().IsValid() );

        ASSERT_EQ( 1 , countSheets(*db) );

        //  Test some insert errors
        ASSERT_NE( DgnDbStatus::Success, db->Models().Insert(*sheet1, "") ) << "Should be illegal to add sheet that is already persistent";

        SheetModelPtr sheetSameName = SheetModel::Create(params);
        ASSERT_NE( DgnDbStatus::Success, db->Models().Insert(*sheetSameName, "") ) << "Should be illegal to add a second sheet with the same name";

        //  Create a second sheet
        SheetModel::CreateParams params2(*db, DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SheetModel)),
                                s_sheet2Name, sheetSize);
        SheetModelPtr sheet2 = SheetModel::Create(params2);
        ASSERT_TRUE( sheet1.IsValid() );
        ASSERT_EQ( DgnDbStatus::Success, db->Models().Insert(*sheet2, "a second sheet model") );
        ASSERT_TRUE( sheet2->GetModelId().IsValid() );
        ASSERT_NE( sheet2->GetModelId() , sheet1->GetModelId() );

        ASSERT_EQ( 2 , countSheets(*db) );
        ASSERT_TRUE( db->Models().QueryModelId(s_sheet2Name).IsValid() );

        //  Look up a sheet     ... by name
        DgnModelId mid = db->Models().QueryModelId(s_sheet1Name);
        ASSERT_EQ( mid , sheet1->GetModelId() );
        ASSERT_EQ( mid , db->Models().QueryModelId(s_sheet1NameUPPER) ) << "Sheet model names should be case-insensitive";
        //                      ... by id
        ASSERT_EQ( sheet1.get() , db->Models().Get<SheetModel>(mid) );
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
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, dbFileName, DgnDb::OpenParams(Db::OPEN_ReadWrite));
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
        ASSERT_EQ( DgnDbStatus::Success, db->Models().DeleteModel(db->Models().QueryModelId(s_sheet2Name)) );
        ASSERT_EQ( 1 , countSheets(*db) );
        }        

    if (true)
        {
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, dbFileName, DgnDb::OpenParams(Db::OPEN_ReadWrite));
        ASSERT_TRUE( db.IsValid() );

        ASSERT_EQ( 1 , countSheets(*db) );

        DgnModelId mid = db->Models().QueryModelId(s_sheet1Name);

        // Verify that we can only place drawing elements in a sheet
        InsertElement(*db, mid, false, true);
        InsertElement(*db, mid, true, false);
        }
    }
