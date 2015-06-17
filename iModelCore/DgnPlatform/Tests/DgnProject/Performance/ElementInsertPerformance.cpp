/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/ElementInsertPerformance.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnHandlers/ScopedDgnHost.h>
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECDbApi.h>
#include <Logging/bentleylogging.h>
#include "PerformanceTestFixture.h"

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

static bool s_ptShouldFail;

//=======================================================================================
//! A test IDgnElementDependencyHandler
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct PTHandler : Dgn::DgnElementDrivesElementDependencyHandler
{
    DOMAINHANDLER_DECLARE_MEMBERS(TMTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME, PTHandler, Dgn::DgnDomain::Handler, )

    bvector<EC::ECInstanceId> m_relIds;

    virtual void _GetLocalizedDescription(Utf8StringR descr, uint32_t desiredLength) override { descr = "The PT rule"; }

    void _OnRootChanged(DgnDbR db, BeSQLite::EC::ECInstanceId relationshipId, DgnElementId source, DgnElementId target, TxnSummaryR summary) override
    {
        if (s_ptShouldFail)
            summary.ReportError(*new TxnSummary::ValidationError(TxnSummary::ValidationError::Severity::Warning, "PT failed"));
        m_relIds.push_back(relationshipId);
    }

    void Clear() { m_relIds.clear(); }
};

HANDLER_DEFINE_MEMBERS(PTHandler)

struct PTShouldFail
{
    PTShouldFail() { s_ptShouldFail = true; }
    ~PTShouldFail() { s_ptShouldFail = false; }
};

struct PTestElementHandler;

//=======================================================================================
//! A test Element
// @bsiclass                                                     Sam.Wilson      04/15
//=======================================================================================
struct PTestElement : Dgn::PhysicalElement
{
    DEFINE_T_SUPER(Dgn::PhysicalElement)

private:
    friend struct PTestElementHandler;

    PTestElement(CreateParams const& params) : T_Super(params) {}
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static CurveVectorPtr computeShape()
{
    static const double PLANE_LEN = 100;

    DPoint3d pts[6];
    pts[0] = DPoint3d::From(-PLANE_LEN, -PLANE_LEN);
    pts[1] = DPoint3d::From(+PLANE_LEN, -PLANE_LEN);
    pts[2] = DPoint3d::From(+PLANE_LEN, +PLANE_LEN);
    pts[3] = DPoint3d::From(-PLANE_LEN, +PLANE_LEN);
    pts[4] = pts[0];
    pts[5] = pts[0];
    pts[5].z = 1;

    return CurveVector::CreateLinear(pts, _countof(pts), CurveVector::BOUNDARY_TYPE_Open);
}

//=======================================================================================
//! A test ElementHandler
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct PTestElementHandler : Dgn::ElementHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS("TestElement", PTestElement, PTestElementHandler, Dgn::ElementHandler, )


    ECN::ECClassCP GetTestElementECClass(DgnDbR db)
    {
        return db.Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_ELEMENT_CLASS_NAME);
    }

    DgnElementKey InsertElement(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode)
    {
        DgnModelP model = db.Models().GetModel(mid);
        DgnElementPtr testElement = PTestElementHandler::Create(PTestElement::CreateParams(*model, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId, Placement3d(), elementCode));
        GeometricElementP geomElem = const_cast<GeometricElementP>(testElement->ToGeometricElement());

        ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*geomElem);

        builder->Append(*computeShape());

        if (SUCCESS != builder->SetGeomStreamAndPlacement(*geomElem))
            return DgnElementKey();

        return db.Elements().Insert(*testElement)->GetElementKey();
    }

    DgnDbStatus DeleteElement(DgnDbR db, DgnElementId eid)
    {
        return db.Elements().Delete(eid);
    }
};

HANDLER_DEFINE_MEMBERS(PTestElementHandler)

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
        RegisterHandler(PTestElementHandler::GetHandler());
        RegisterHandler(PTHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Transaction Manager
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct PerformanceElementItem : public ::testing::Test
{
public:
    ScopedDgnHost m_host;
    DgnDbPtr      m_db;
    DgnModelId    m_defaultModelId;
    DgnCategoryId m_defaultCategoryId;

    PerformanceElementItem()
    {
        // Must register my domain whenever I initialize a host
        DgnDomains::RegisterDomain(ElementInsertPerformanceTestDomain::GetDomain());
    }

    ~PerformanceElementItem()
    {
        FinalizeStatements();
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

    BentleyStatus status = ElementInsertPerformanceTestDomain::GetDomain().ImportSchema(*m_db, schemaFile);
    ASSERT_TRUE(BentleyStatus::SUCCESS == status);

    auto schema = m_db->Schemas().GetECSchema(TMTEST_SCHEMA_NAME, true);
    ASSERT_NE(nullptr, schema);
    ASSERT_NE(nullptr, PTestElementHandler::GetHandler().GetTestElementECClass(*m_db));
    ASSERT_NE(nullptr, m_db->Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME));

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    DgnModelP defaultModel = m_db->Models().GetModel(m_defaultModelId);
    ASSERT_NE(nullptr, defaultModel);
    GetDefaultModel().FillModel();

    m_defaultCategoryId = m_db->Categories().MakeIterator().begin().GetCategoryId();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey InsertElement(Utf8CP elementCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId())
{
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    return PTestElementHandler::GetHandler().InsertElement(*m_db, mid, categoryId, elementCode);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TwiddleTime(DgnElementKeyCR ekey)
{
    BeThreadUtilities::BeSleep(1); // make sure the new timestamp is after the one that's on the Element now
#if defined (NEEDS_WORK_TXN_MANAGER)
    m_db->Elements().UpdateLastModifiedTime(ekey.GetElementId());
#endif
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool insertElementItem(DgnElementId id, WCharCP propValue)
{
    Utf8String stmt("INSERT INTO ");
    stmt.append(TMTEST_SCHEMA_NAME);
    stmt.append(".");
    stmt.append(TMTEST_TEST_ITEM_CLASS_NAME);
    stmt.append("(ECInstanceId, ");
    stmt.append(TMTEST_TEST_ITEM_TestItemPropertyA);
    stmt.append(") Values (?, ?);");

    CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement(stmt.c_str());
    if (insertStmt.IsNull())
        return false;
    if (insertStmt->BindId(1, id) != ECSqlStatus::Success)
        return false;
    if (insertStmt->BindText(2, ECN::ECValue(propValue).GetUtf8CP(), BeSQLite::EC::IECSqlBinder::MakeCopy::No) != ECSqlStatus::Success)
        return false;
    if (insertStmt->Step() != BeSQLite::EC::ECSqlStepStatus::Done)
        return false;
    
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool updateElementItem(DgnElementId id, WCharCP propValue)
{
    Utf8String stmt("UPDATE ");
    stmt.append(TMTEST_SCHEMA_NAME);
    stmt.append(".");
    stmt.append(TMTEST_TEST_ITEM_CLASS_NAME);
    stmt.append(" SET ");
    stmt.append(TMTEST_TEST_ITEM_TestItemPropertyA);
    stmt.append("=? WHERE ECInstanceId = ?;");

    CachedECSqlStatementPtr upStmt = m_db->GetPreparedECSqlStatement(stmt.c_str());
    if (upStmt.IsNull())
        return false;
    if (upStmt->BindId(2, id) != ECSqlStatus::Success)
        return false;
    if (upStmt->BindText(1, ECN::ECValue(propValue).GetUtf8CP(), BeSQLite::EC::IECSqlBinder::MakeCopy::No) != ECSqlStatus::Success)
        return false;
    if (upStmt->Step() != BeSQLite::EC::ECSqlStepStatus::Done)
        return false;

    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool deleteElementItem(DgnElementId id)
{
    Utf8String stmt("DELETE FROM ");
    stmt.append(TMTEST_SCHEMA_NAME);
    stmt.append(".");
    stmt.append(TMTEST_TEST_ITEM_CLASS_NAME);
    stmt.append(" WHERE ECInstanceId = ?;");

    CachedECSqlStatementPtr delStmt = m_db->GetPreparedECSqlStatement(stmt.c_str());
    if (delStmt.IsNull())
        return false;
    if (delStmt->BindId(1, id) != ECSqlStatus::Success)
        return false;
    if (delStmt->Step() != BeSQLite::EC::ECSqlStepStatus::Done)
        return false;

    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool selectElementItem(DgnElementId id)
{
    Utf8String stmt("SELECT ");
    stmt.append(TMTEST_TEST_ITEM_TestItemPropertyA);
    stmt.append(" FROM ");
    stmt.append(TMTEST_SCHEMA_NAME);
    stmt.append(".");
    stmt.append(TMTEST_TEST_ITEM_CLASS_NAME);
    stmt.append(" WHERE ECInstanceId = ?;");

    CachedECSqlStatementPtr selStmt = m_db->GetPreparedECSqlStatement(stmt.c_str());
    if (selStmt.IsNull())
        return false;
    if (selStmt->BindId(1, id) != ECSqlStatus::Success)
        return false;
    if (selStmt->Step() != BeSQLite::EC::ECSqlStepStatus::HasRow)
        return false;

    //Utf8CP value = selStmt->GetValueText(0);

    return true;
}


};

} // anonymous ns



/*---------------------------------------------------------------------------------**//**
* Test to measure time of Insert, Select, Update and Delete of an Element Item
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementItem, CRUD)
{
    //Set the start, maximum and increment number to run the test
    int startCount      = 1000;
    int maxCount        = 10000;
    int increment       = 1000;

    StopWatch elementTimer("Insert Element", false);
    PerformanceTestingFrameWork testObj;

    SetupProject(L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceTests.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    int counter;
    double elementTime, elementItemTime, selectTime, updateTime, deleteTime;
    for (counter = startCount; counter <= maxCount; counter = counter + increment)
    {
        elementTime = elementItemTime = selectTime = updateTime = deleteTime = 0.0;
        for (int i = 1; i <= counter; i++)
        {
            //First inser the Element
            elementTimer.Start();
            auto key1 = InsertElement("E" + i);
            elementTimer.Stop();
            elementTime = elementTime + elementTimer.GetElapsedSeconds();

            EXPECT_TRUE(key1.GetElementId().IsValid());
            GeometricElementCPtr el = m_db->Elements().GetElement(key1.GetElementId())->ToGeometricElement();
            EXPECT_TRUE(el != nullptr);
            EXPECT_EQ(&el->GetElementHandler(), &PTestElementHandler::GetHandler());

            // ECSQL to add ElementItem
            elementTimer.Start();
            EXPECT_TRUE (insertElementItem(el->GetElementId(), L"Test"));
            elementTimer.Stop();
            elementItemTime = elementItemTime + elementTimer.GetElapsedSeconds();

            //Time to select a single ElementItem
            elementTimer.Start();
            EXPECT_TRUE(selectElementItem(el->GetElementId()));
            elementTimer.Stop();
            selectTime = selectTime + elementTimer.GetElapsedSeconds();
            
            //Now Update data and measure time for Update
            elementTimer.Start();
            EXPECT_TRUE(updateElementItem(el->GetElementId(), L"Test - New"));
            elementTimer.Stop();
            updateTime = updateTime + elementTimer.GetElapsedSeconds();

            //Now delete data and measure time for Delete
            elementTimer.Start();
            EXPECT_TRUE(deleteElementItem(el->GetElementId()));
            elementTimer.Stop();
            deleteTime = deleteTime + elementTimer.GetElapsedSeconds();

        }
        //Write results to Db for analysis
        testObj.writeTodb(elementTime + elementItemTime, "ElementCRUDPerformance,InsertElementItem_Total", "", counter);
        testObj.writeTodb(selectTime, "ElementCRUDPerformance,SelectSignleElementItem", "", counter);
        testObj.writeTodb(updateTime, "ElementCRUDPerformance,UpdateElementItem", "", counter);
        testObj.writeTodb(deleteTime, "ElementCRUDPerformance,DeleteElementItem", "", counter);

        //Uncomment below lines to store breakup of time in Insertion
        //testObj.writeTodb(elementTime, "ElementInsertPerformance,InsertElementItem_Element", "", counter);
        //testObj.writeTodb(elementItemTime, "ElementInsertPerformance,InsertElementItem_Item", "", counter);

    }
}

