/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/EntityAndItem_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <Geom/CurveVector.h>

#ifdef NEEDS_WORK_ELEMENT_REFACTOR

#define TMTEST_SCHEMA_NAME   "DgnPlatformTest"
#define TMTEST_SCHEMA_NAMEW L"DgnPlatformTest"

USING_NAMESPACE_BENTLEY_SQLITE

namespace {

//=======================================================================================
//! A test PhysicalElementHandler
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct TestElementHandler : PhysicalElementHandler
{
    DEFINE_T_SUPER (DgnPlatform::PhysicalElementHandler)

protected:
    static TestElementHandler* s_handler;

    virtual Utf8CP _GetElementSchemaName()      const override {return TMTEST_SCHEMA_NAME;}
    virtual Utf8CP _GetElementClassName()       const override {return "TestElement";}
    virtual Utf8CP _GetElementClassECSqlName()  const override {return TMTEST_SCHEMA_NAME ".TestElement";}
};

static TestElementHandler s_testElementHandler;

//=======================================================================================
// @bsiclass                                                BentleySystems
//=======================================================================================
struct TestItemHandler : DgnPlatform::ElementItemHandler
{
    friend struct MoveReferencePlaneTool;
    DEFINE_T_SUPER(DgnPlatform::ElementItemHandler)
    
protected:
    virtual Utf8CP _GetItemSchemaName() const {return TMTEST_SCHEMA_NAME;}
    virtual Utf8CP _GetItemClassName() const {return "TestItem";}
    virtual Utf8CP _GetItemClassECSqlName() const {return TMTEST_SCHEMA_NAME ".TestItem";}
};

static TestItemHandler s_testItemHandler;

#if defined (NEEDS_WORK_ELEMENT_REFACTOR)
//=======================================================================================
//! A test IDgnElementDependencyHandler
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct TestHandlerAdmin : DgnPlatformLib::Host::HandlerAdmin
    {
    DEFINE_T_SUPER(DgnPlatformLib::Host::HandlerAdmin)

    PhysicalElementHandler* _SupplyPhysicalElementHandler (Utf8StringCR name, DgnDbR db) override 
        {
        if (name.Equals (s_testElementHandler.GetElementClassECSqlName()))
            return &s_testElementHandler;

        return T_Super::_SupplyPhysicalElementHandler (name, db);
        }

    ElementItemHandler* _SupplyElementItemHandler (Utf8StringCR name, DgnDbR db) override
        {
        if (name.Equals (s_testItemHandler.GetItemClassECSqlName()))
            return &s_testItemHandler;

        return T_Super::_SupplyElementItemHandler (name, db);
        }
    };

static TestHandlerAdmin s_handlerAdmin;
#endif

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Elements and Items
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementAndItemTests : public ::testing::Test
{
public:
    ScopedDgnHost m_host;
    DgnDbPtr      m_db;
    PhysicalModelP m_defaultModel;
    DgnCategoryId m_testCategory;

    ElementAndItemTests()
        {
#if defined (NEEDS_WORK_ELEMENT_REFACTOR)
        m_host.SetHandlerAdmin (&s_handlerAdmin);
#endif
        }
    
    ~ElementAndItemTests()
        {
        m_db->GetTxnManager().Deactivate(); // finalizes Txns's prepared statements
        }

    void SetupProject (WCharCP projFile, WCharCP testFile, BeSQLite::Db::OpenMode mode);

    DgnCategoryId GetCategoryId() {return m_testCategory;}
    DgnSubCategoryId GetSubCategoryId() {return DgnCategories::DefaultSubCategoryId(m_testCategory);}

    CurveVectorPtr ComputeLine();
    PlacedGeomPart CreatePlacedPart(CurveVectorPtr);
    void CreateGeomAspect(ElementItemKey&, DgnGeomPartId& partId, DgnElementId& elementId, DgnElementKeyCR, CurveVectorPtr);
};

} // anonymous namespace

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr ElementAndItemTests::ComputeLine()
    {
    static const double L = 10000; // *** WIP

    DPoint3d pts[2];
    pts[0] = DPoint3d::From (0,0);
    pts[1] = DPoint3d::From (0,L);

    return CurveVector::CreateLinear(pts, _countof(pts), CurveVector::BOUNDARY_TYPE_Open);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
PlacedGeomPart ElementAndItemTests::CreatePlacedPart(CurveVectorPtr curveVector)
    {
    auto geomPart = DgnGeomPart::Create(GetSubCategoryId(), BeSQLite::EC::ECInstanceId());
    geomPart->GetGeometryR().push_back(ElementGeometry::Create(curveVector));
    PlacedGeomPart placement;
    placement.SetPartPtr(geomPart);
    return placement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementAndItemTests::CreateGeomAspect(ElementItemKey& ikey, DgnGeomPartId& partId, DgnElementId& elementId, DgnElementKeyCR ekey, CurveVectorPtr curveVector)
    {
#if defined (WIP_ELEMENT_CHANGES) // DgnElementKey input is wrong...just want element class...
    DgnCategoryId categoryId;
    auto geometry = PhysicalGeometry::Create();
    geometry->AddPart(CreatePlacedPart(curveVector));
    for (auto& placedGeomPart : *geometry)
        {
        ASSERT_EQ( BSISUCCESS, m_db->GeomParts().InsertGeomPart(*placedGeomPart.GetPartPtr()) );
        partId = placedGeomPart.GetPartPtr()->GetId();
        if (!categoryId.IsValid())
            categoryId = m_db->Categories().QueryCategoryId(placedGeomPart.GetPartPtr()->GetSubCategoryId()))
        }

//    ikey = s_testItemHandler.InsertGeom(*m_defaultModel, ekey, DPoint3d::FromZero(), YawPitchRollAngles(), *geometry);
//    ASSERT_TRUE( ikey.GetElementId().IsValid() );
    EditElementHandle  eeh;
    eeh.CreateNewElement(*m_defaultModel, ekey.GetECClassId(), categoryId);
    ASSERT_EQ(BSISUCCESS, ElementItemHandler::Handler().SetElementGeom(eeh, *geometry, DPoint3d::FromZero(), YawPitchRollAngles()));
    ASSERT_EQ(BSISUCCESS, eeh.AddToModel());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .dgndb project file after copying it to out
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementAndItemTests::SetupProject (WCharCP projFile, WCharCP testFile, BeSQLite::Db::OpenMode mode)
    {
    BeFileName outFileName;
    ASSERT_EQ (SUCCESS, DgnDbTestDgnManager::GetTestDataOut (outFileName, projFile, testFile, __FILE__));
    DbResult result;
    m_db = DgnDb::OpenDgnDb (&result, outFileName, DgnDb::OpenParams(mode));
    ASSERT_TRUE (m_db.IsValid());
    ASSERT_TRUE( result == BE_SQLITE_OK);

    auto assetsDir = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    auto schemaFileRelPath = L"ECSchemas/" TMTEST_SCHEMA_NAMEW L".01.00.ecschema.xml";
    ASSERT_TRUE( BSISUCCESS == DgnDbUtilities::ImportSchema (*m_db, schemaFileRelPath, assetsDir) );
    auto schema = m_db->Schemas().GetECSchema (TMTEST_SCHEMA_NAME, true);
    ASSERT_TRUE( NULL != schema );
    ASSERT_TRUE( NULL != s_testElementHandler.GetElementClass(*m_db) );
    ASSERT_TRUE( NULL != s_testItemHandler.GetItemClass(*m_db) );

    m_defaultModel = dynamic_cast<PhysicalModel*>(m_db->Models().GetModel(m_db->Models().QueryFirstModelId()));
    ASSERT_TRUE( m_defaultModel != NULL );
    m_defaultModel->FillModel();

    DgnCategories::Category categoryRow("Test-Category", DgnCategories::Scope::Physical);
    categoryRow.SetRank(DgnCategories::Rank::Application);
    DgnCategories::SubCategory::Appearance appearance;
    appearance.SetInvisible(false);
    appearance.SetColor(ColorDef(128,200,128));
    appearance.SetWeight(0);
    appearance.SetTransparency(150);
    ASSERT_TRUE(BE_SQLITE_OK == m_db->Categories().InsertCategory(categoryRow, appearance));
    m_testCategory = categoryRow.GetCategoryId();
    ASSERT_TRUE(m_testCategory.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ElementAndItemTests, Test1)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"ElementAndItemTests_Test1.idgndb", BeSQLite::Db::OPEN_ReadWrite);
    
    // Create a TestElement and a TestItem
    auto ekey = s_testElementHandler.InsertElement(*m_defaultModel, m_testCategory, "test1");
    ASSERT_TRUE( ekey.GetElementId().IsValid() );

    ElementItemKey ikey;
    DgnGeomPartId partId;
    DgnElementId elementId;
    CreateGeomAspect(ikey, partId, elementId, ekey, ComputeLine());

    ASSERT_EQ( BE_SQLITE_OK, m_db->SaveChanges() );

#if defined (WIP_ELEMENTGEOM_REFACTOR)
    //  Run queries against them
    auto geometry = m_db->Items().QueryPhysicalGeometry(ikey);
    ASSERT_TRUE( geometry.IsValid() );
    ASSERT_TRUE( geometry->begin() != geometry->end() );
    ASSERT_EQ( geometry->begin()->GetPartPtr()->GetId(), partId );
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ElementAndItemTests, ElementOwnsChildElements)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementOwnsChildElements.idgndb", BeSQLite::Db::OPEN_ReadWrite);
    
    DgnElementKey parentElementKey = s_testElementHandler.InsertElement(*m_defaultModel, m_testCategory, "Parent");
    ASSERT_TRUE(parentElementKey.IsValid());

    DgnElementKey childElementKey = s_testElementHandler.InsertElement(*m_defaultModel, m_testCategory, "Child");
    ASSERT_TRUE(childElementKey.IsValid());

    BentleyStatus status = m_db->Elements().UpdateParentElementId(parentElementKey.GetElementId(), childElementKey.GetElementId());
    ASSERT_TRUE(BentleyStatus::SUCCESS == status);

    DgnElementId parentElementId = m_db->Elements().QueryParentElementId(childElementKey.GetElementId());
    ASSERT_TRUE(parentElementId == parentElementKey.GetElementId());

    DgnElementId invalidElementId = m_db->Elements().QueryParentElementId(parentElementKey.GetElementId());
    ASSERT_FALSE(invalidElementId.IsValid());
    }

#endif
