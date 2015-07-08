/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ElementGeomPart_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnHandlers/ScopedDgnHost.h>
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECDbApi.h>
#include <Logging/bentleylogging.h>
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_LOGGING
USING_DGNDB_UNIT_TESTS_NAMESPACE

#define TMTEST_SCHEMA_NAME                               "DgnPlatformTest"
#define TMTEST_SCHEMA_NAMEW                             L"DgnPlatformTest"
#define TMTEST_TEST_ELEMENT_CLASS_NAME                   "TestElement"
#define TMTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME    "TestElementDrivesElement"
#define TMTEST_TEST_ELEMENT_TestElementProperty         L"TestElementProperty"
#define TMTEST_TEST_ITEM_CLASS_NAME                       "TestItem"
#define TMTEST_TEST_ITEM_TestItemProperty               L"TestItemProperty"
#define TMTEST_TEST_ITEM_TestItemPropertyA               "TestItemProperty"


namespace {

static bool s_GTShouldFail;

//=======================================================================================
//! A test IDgnElementDependencyHandler
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct GTHandler : public Dgn::DgnElementDependencyHandler
{
    DOMAINHANDLER_DECLARE_MEMBERS(TMTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME, GTHandler, Dgn::DgnDomain::Handler, )

    bvector<EC::ECInstanceId> m_relIds;

    virtual void _GetLocalizedDescription(Utf8StringR descr, uint32_t desiredLength) override { descr = "The GT rule"; }

    
    virtual void _OnRootChanged(DgnDbR db, BeSQLite::EC::ECInstanceId relationshipId, DgnElementId source, DgnElementId target) override
    {
        if (s_GTShouldFail)
            db.Txns().ReportError(*new TxnManager::ValidationError(TxnManager::ValidationError::Severity::Warning, "GT failed"));
        m_relIds.push_back(relationshipId);
    }

    void Clear() { m_relIds.clear(); }
};

HANDLER_DEFINE_MEMBERS(GTHandler)

struct GTShouldFail
{
    GTShouldFail() { s_GTShouldFail = true; }
    ~GTShouldFail() { s_GTShouldFail = false; }
};

struct GTestElementHandler;

//=======================================================================================
//! A test Element
// @bsiclass                                                     Sam.Wilson      04/15
//=======================================================================================
struct GTestElement : Dgn::PhysicalElement
{
    DEFINE_T_SUPER(Dgn::PhysicalElement)

private:
    friend struct GTestElementHandler;

    GTestElement(CreateParams const& params) : T_Super(params) {}
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static CurveVectorPtr computeShape()
{
    static const double PLANE_LEN = 100;

    DPoint3d GTs[6];
    GTs[0] = DPoint3d::From(-PLANE_LEN, -PLANE_LEN);
    GTs[1] = DPoint3d::From(+PLANE_LEN, -PLANE_LEN);
    GTs[2] = DPoint3d::From(+PLANE_LEN, +PLANE_LEN);
    GTs[3] = DPoint3d::From(-PLANE_LEN, +PLANE_LEN);
    GTs[4] = GTs[0];
    GTs[5] = GTs[0];
    GTs[5].z = 1;

    return CurveVector::CreateLinear(GTs, _countof(GTs), CurveVector::BOUNDARY_TYPE_Open);
}

//=======================================================================================
//! A test ElementHandler
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct GTestElementHandler : dgn_ElementHandler::Element
{
    ELEMENTHANDLER_DECLARE_MEMBERS("TestElement", GTestElement, GTestElementHandler, dgn_ElementHandler::Element, )


    ECN::ECClassCP GetTestElementECClass(DgnDbR db)
    {
        return db.Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_ELEMENT_CLASS_NAME);
    }

    DgnElementKey InsertElement(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode)
    {
     
        DgnElementPtr testElement = GTestElementHandler::Create(GTestElement::CreateParams(db, mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId, Placement3d(),nullptr, elementCode));
        GeometricElementP geomElem = const_cast<GeometricElementP>(testElement->ToGeometricElement());

#ifdef WIP_ITEM_HANDLER
        geomElem->SetItemClassId(ElementItemHandler::GetHandler().GetItemClassId(db));
#endif

        ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*geomElem);

        builder->Append(*computeShape());

        if (SUCCESS != builder->SetGeomStreamAndPlacement(*geomElem))
            return DgnElementKey();

        return db.Elements().Insert(*testElement)->GetElementKey();
    }

    DgnElementKey InsertElementUsingGeomPart(DgnDbR db, Utf8CP gpCode, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode)
    {
        DgnModelP model = db.Models().GetModel(mid).get();
        DgnElementPtr testElement = GTestElementHandler::Create(GTestElement::CreateParams(db, mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId, Placement3d(),nullptr, elementCode));
        GeometricElementP geomElem = const_cast<GeometricElementP>(testElement->ToGeometricElement());

#ifdef WIP_ITEM_HANDLER
        geomElem->SetItemClassId(ElementItemHandler::GetHandler().GetItemClassId(db));
#endif

        ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, categoryId, DPoint3d::FromXYZ(0.0, 0.0, 0.0));

        DgnGeomPartId existingPartId = db.GeomParts().QueryGeomPartId(gpCode);
        EXPECT_TRUE(existingPartId.IsValid());

        if (!(builder->Append(existingPartId, Transform::From(0.0, 0.0, 0.0))))
            return DgnElementKey();

        if (SUCCESS != builder->SetGeomStreamAndPlacement(*geomElem))
            return DgnElementKey();

        return db.Elements().Insert(*testElement)->GetElementKey();
    }

    DgnElementKey InsertElementUsingGeomPart(DgnDbR db, DgnGeomPartId gpId, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode)
    {
        DgnModelP model = db.Models().GetModel(mid).get();
        DgnElementPtr testElement = GTestElementHandler::Create(GTestElement::CreateParams(db, mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId, Placement3d(), nullptr, elementCode));
        GeometricElementP geomElem = const_cast<GeometricElementP>(testElement->ToGeometricElement());

#ifdef WIP_ITEM_HANDLER
        geomElem->SetItemClassId(ElementItemHandler::GetHandler().GetItemClassId(db));
#endif

        ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, categoryId, DPoint3d::FromXYZ(0.0, 0.0, 0.0));
        
        if (!(builder->Append(gpId, Transform::From(0.0, 0.0, 0.0))))
            return DgnElementKey();

        if (SUCCESS != builder->SetGeomStreamAndPlacement(*geomElem))
            return DgnElementKey();

        return db.Elements().Insert(*testElement)->GetElementKey();
    }
    DgnDbStatus DeleteElement(DgnDbR db, DgnElementId eid)
    {
        return db.Elements().Delete(eid);
    }
};

HANDLER_DEFINE_MEMBERS(GTestElementHandler)

//=======================================================================================
//! A test Domain
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct ElementInsertPerformanceTestDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(ElementInsertPerformanceTestDomain, )
public:
    ElementInsertPerformanceTestDomain();
};

DOMAIN_DEFINE_MEMBERS(ElementInsertPerformanceTestDomain)

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Sam.Wilson      01/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    ElementInsertPerformanceTestDomain::ElementInsertPerformanceTestDomain() : DgnDomain(TMTEST_SCHEMA_NAME, "DgnProject Test Schema", 1)
    {
        RegisterHandler(GTestElementHandler::GetHandler());
        RegisterHandler(GTHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Transaction Manager
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementGeomPartTests : public ::testing::Test
{
public:
    ScopedDgnHost m_host;
    DgnDbPtr      m_db;
    DgnModelId    m_defaultModelId;
    DgnCategoryId m_defaultCategoryId;

    ElementGeomPartTests()
    {
        // Must register my domain whenever I initialize a host
        DgnDomains::RegisterDomain(ElementInsertPerformanceTestDomain::GetDomain());
    }

    ~ElementGeomPartTests()
    {
        FinalizeStatements();
        //m_db->Txns.Deactivate(); // finalizes TxnManager's prepared statements
        
    }

    void CloseDb()
    {
        FinalizeStatements();
        m_db->CloseDb();
    }

    DgnModelR GetDefaultModel()
    {
        return *m_db->Models().GetModel(m_defaultModelId);
    }

    virtual void FinalizeStatements() {}

/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .dgndb project file after copying it to out
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SetupProject(WCharCP projFile, WCharCP testFile, BeSQLite::Db::OpenMode mode)
{
    BeFileName outFileName;
    ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, projFile, testFile, __FILE__));
    DbResult result;
    m_db = DgnDb::OpenDgnDb(&result, outFileName, DgnDb::OpenParams(mode));
    ASSERT_TRUE(m_db.IsValid());
    ASSERT_TRUE(result == BE_SQLITE_OK);

    BeFileName schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    schemaFile.AppendToPath(L"ECSchemas/" TMTEST_SCHEMA_NAMEW L".01.00.ecschema.xml");

    auto status = ElementInsertPerformanceTestDomain::GetDomain().ImportSchema(*m_db, schemaFile);
    ASSERT_TRUE(DgnDbStatus::Success== status);

    auto schema = m_db->Schemas().GetECSchema(TMTEST_SCHEMA_NAME, true);
    ASSERT_NE(nullptr, schema);
    ASSERT_NE(nullptr, GTestElementHandler::GetHandler().GetTestElementECClass(*m_db));
    ASSERT_NE(nullptr, m_db->Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME));

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    DgnModelP defaultModel = m_db->Models().GetModel(m_defaultModelId).get();
    ASSERT_NE(nullptr, defaultModel);
    GetDefaultModel().FillModel();

    m_defaultCategoryId = m_db->Categories().MakeIterator().begin().GetCategoryId();
}

};

} // anonymous ns


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, CreateElements)
{
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*computeShape());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create("TestGeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId(geomPartPtr->GetCode());
    EXPECT_TRUE(existingPartId.IsValid());

    auto key1 = GTestElementHandler::GetHandler().InsertElementUsingGeomPart(*m_db, geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId, "Test");
    EXPECT_TRUE(key1.GetElementId().IsValid());
    
    auto key2 = GTestElementHandler::GetHandler().InsertElementUsingGeomPart(*m_db, geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId, "Test2");
    EXPECT_TRUE(key2.GetElementId().IsValid());

    auto key3 = GTestElementHandler::GetHandler().InsertElement(*m_db, m_defaultModelId, m_defaultCategoryId, "Test3");
    EXPECT_TRUE(key3.GetElementId().IsValid());
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, GeomPartWithoutCode)
{
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*computeShape());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create();
    EXPECT_TRUE(geomPartPtr != NULL);
    ASSERT_STREQ("", geomPartPtr->GetCode());
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));

    DgnGeomPartId existingPartId = geomPartPtr->GetId();
    EXPECT_TRUE(existingPartId.IsValid());

    auto key1 = GTestElementHandler::GetHandler().InsertElementUsingGeomPart(*m_db, existingPartId, m_defaultModelId, m_defaultCategoryId, "Test");
    EXPECT_TRUE(key1.GetElementId().IsValid());
    
    auto key2 = GTestElementHandler::GetHandler().InsertElementUsingGeomPart(*m_db, existingPartId, m_defaultModelId, m_defaultCategoryId, "Test2");
    EXPECT_TRUE(key2.GetElementId().IsValid());

    auto key3 = GTestElementHandler::GetHandler().InsertElement(*m_db, m_defaultModelId, m_defaultCategoryId, "Test3");
    EXPECT_TRUE(key3.GetElementId().IsValid());
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, ElementGeomUsesParts)
{
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*computeShape());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create("TestGeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId("TestGeomPart");
    EXPECT_TRUE(existingPartId.IsValid());

    auto key1 = GTestElementHandler::GetHandler().InsertElement(*m_db, m_defaultModelId, m_defaultCategoryId, "Test1");
    EXPECT_TRUE(key1.GetElementId().IsValid());

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertElementGeomUsesParts(key1.GetElementId(), existingPartId) );
    DgnElementCPtr elem = m_db->Elements().GetElement(key1.GetElementId());
    
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK ,stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementGeomUsesParts)));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1,stmt.GetValueInt(0));
    ASSERT_EQ(key1.GetElementId().GetValue(), (int64_t)stmt.GetValueInt(1));
    ASSERT_EQ(existingPartId.GetValue(), (int64_t)stmt.GetValueInt(2));
    
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, ElementGeomUsesParts_DeleteGeomPart)
{
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*computeShape());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create("TestGeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId("TestGeomPart");
    EXPECT_TRUE(existingPartId.IsValid());

    auto key1 = GTestElementHandler::GetHandler().InsertElement(*m_db, m_defaultModelId, m_defaultCategoryId, "Test1");
    EXPECT_TRUE(key1.GetElementId().IsValid());

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertElementGeomUsesParts(key1.GetElementId(), existingPartId) );
    DgnElementCPtr elem = m_db->Elements().GetElement(key1.GetElementId());

    // Delete Geom Part
    EXPECT_EQ(SUCCESS, m_db->GeomParts().DeleteGeomPart(existingPartId));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementGeomUsesParts)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, ElementGeomUsesParts_DeleteElement)
{
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*computeShape());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create("TestGeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId("TestGeomPart");
    EXPECT_TRUE(existingPartId.IsValid());

    auto key1 = GTestElementHandler::GetHandler().InsertElement(*m_db, m_defaultModelId, m_defaultCategoryId, "Test1");
    EXPECT_TRUE(key1.GetElementId().IsValid());

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertElementGeomUsesParts(key1.GetElementId(), existingPartId) );
    DgnElementCPtr elem = m_db->Elements().GetElement(key1.GetElementId());

    // Delete Element
    ASSERT_EQ(DgnDbStatus::Success, m_db->Elements().Delete(*m_db->Elements().GetElement(key1.GetElementId())));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementGeomUsesParts)));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, CreateElementsAndDeleteGemPart)
{
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*computeShape());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create("TestGeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId(geomPartPtr->GetCode());
    EXPECT_TRUE(existingPartId.IsValid());

    //Add two elements using this GeomPart
    auto key1 = GTestElementHandler::GetHandler().InsertElementUsingGeomPart(*m_db, geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId, "Test");
    EXPECT_TRUE(key1.GetElementId().IsValid());

    auto key2 = GTestElementHandler::GetHandler().InsertElementUsingGeomPart(*m_db, geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId, "Test2");
    EXPECT_TRUE(key2.GetElementId().IsValid());

    auto key3 = GTestElementHandler::GetHandler().InsertElement(*m_db, m_defaultModelId, m_defaultCategoryId, "Test3");
    EXPECT_TRUE(key3.GetElementId().IsValid());

    // Delete Geom Part
    EXPECT_EQ(SUCCESS, m_db->GeomParts().DeleteGeomPart(existingPartId));
    //EXPECT_EQ(-1,m_db->GeomParts().QueryGeomPartId(geomPartPtr->GetCode()).GetValue());
    EXPECT_TRUE(m_db->Elements().GetElement(key1.GetElementId()).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(key2.GetElementId()).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(key3.GetElementId()).IsValid());
    //m_db->Get
}

