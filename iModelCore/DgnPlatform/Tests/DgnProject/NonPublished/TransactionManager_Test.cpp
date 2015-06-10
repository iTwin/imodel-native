/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/TransactionManager_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECSqlBuilder.h>
#include <DgnPlatform/DgnCore/WebMercator.h>

#define TMTEST_SCHEMA_NAME                               "DgnPlatformTest"
#define TMTEST_SCHEMA_NAMEW                             L"DgnPlatformTest"
#define TMTEST_TEST_ELEMENT_CLASS_NAME                   "TestElement"
#define TMTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME    "TestElementDrivesElement"
#define TMTEST_TEST_ELEMENT_TestElementProperty          "TestElementProperty"
#define TMTEST_TEST_ITEM_CLASS_NAME                       "TestItem"
#define TMTEST_TEST_ITEM_TestItemProperty                "TestItemProperty"

USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_UNNAMED_NAMESPACE

static bool s_abcShouldFail;

//=======================================================================================
//! A test IDgnElementDependencyHandler
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct ABCHandler : DgnPlatform::DgnElementDrivesElementDependencyHandler
    {
    DOMAINHANDLER_DECLARE_MEMBERS(TMTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME, ABCHandler, DgnPlatform::DgnDomain::Handler, )

    bvector<EC::ECInstanceId> m_relIds;
    void _OnRootChanged(DgnDbR db, BeSQLite::EC::ECInstanceId relationshipId, DgnElementId source, DgnElementId target, TxnSummaryR) override;
    void Clear() {m_relIds.clear();}
    };

HANDLER_DEFINE_MEMBERS(ABCHandler)

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct AbcShouldFail
    {
    AbcShouldFail() {s_abcShouldFail = true;}
    ~AbcShouldFail() {s_abcShouldFail = false;}
    };

struct TestElementHandler;

//=======================================================================================
//! A test Element. Has an item.
// @bsiclass                                                     Sam.Wilson      04/15
//=======================================================================================
struct TestElement : DgnPlatform::PhysicalElement
{
    DEFINE_T_SUPER(DgnPlatform::PhysicalElement)

    friend struct TestElementHandler;

private:    
    // Item caching states:
    enum class ItemState
        {
        Unknown,        // don't know yet
        DoesNotExist,   // no item exists in the Db, and nothing is cached in memory
        Exists,         // item exists in the Db, is cached in memory, and is un-modified 
        Modified,       // item may or may not exist in the Db and is modified in memory
        Deleted         // item exists in the Db and should be deleted
        };
    ItemState  m_itemState;
    Utf8String m_testItemProperty;

    virtual DgnModelStatus _InsertInDb() override;
    virtual DgnModelStatus _UpdateInDb() override;
    virtual DgnModelStatus _LoadFromDb() override;
    virtual DgnModelStatus _CopyFrom(DgnElementCR) override;

public:
    TestElement(CreateParams const& params) : T_Super(params), m_itemState(ItemState::Unknown) {} 

    static ECN::ECClassCP GetTestElementECClass(DgnDbR db) {return db.Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_ELEMENT_CLASS_NAME);}
    static RefCountedPtr<TestElement> Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode);

    // Provide an API for getting and modifying my item's properties.
    bool HasTestItem() const {return ItemState::Exists == m_itemState || ItemState::Modified == m_itemState;}
    Utf8String GetTestItemProperty() const {return HasTestItem()? m_testItemProperty: "";}
    void SetTestItemProperty(Utf8CP value);
    void DeleteTestItem() {if (HasTestItem()) m_itemState=ItemState::Deleted;}
};

typedef RefCountedPtr<TestElement> TestElementPtr;
typedef RefCountedCPtr<TestElement> TestElementCPtr;
typedef TestElement& TestElementR;
typedef TestElement const& TestElementCR;

//=======================================================================================
//! A test ElementHandler
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct TestElementHandler : DgnPlatform::ElementHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS("TestElement", TestElement, TestElementHandler, DgnPlatform::ElementHandler, )
};

HANDLER_DEFINE_MEMBERS(TestElementHandler)

//=======================================================================================
//! A test Domain
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct TransactionManagerTestDomain : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(TransactionManagerTestDomain, )
public:
    TransactionManagerTestDomain();
    };

DOMAIN_DEFINE_MEMBERS(TransactionManagerTestDomain)

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct TxnMonitorVerifier : TxnMonitor
    {
    bool m_OnTxnClosedCalled;
    bool m_OnTxnReverseCalled;
    bool m_OnTxnReversedCalled;
    bset<EC::ECInstanceId> m_adds, m_deletes, m_mods;

    TxnMonitorVerifier();
    ~TxnMonitorVerifier();
    void Clear();
    virtual void _OnTxnCommit(TxnSummaryCR summary) override;
    virtual void _OnTxnReverse(TxnSummaryCR, TxnDirection isUndo) override {m_OnTxnReverseCalled = true;}
    virtual void _OnTxnReversed(TxnSummaryCR, TxnDirection isUndo) override {m_OnTxnReversedCalled = true;}
    };

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct TransactionManagerTests : public ::testing::Test
{
public:
    ScopedDgnHost m_host;
    DgnDbPtr      m_db;
    DgnModelId    m_defaultModelId;
    DgnCategoryId m_defaultCategoryId;

    TransactionManagerTests();
    ~TransactionManagerTests();
    void CloseDb() {m_db->CloseDb();}
    DgnModelR GetDefaultModel() {return *m_db->Models().GetModel(m_defaultModelId);}
    void SetupProject(WCharCP projFile, WCharCP testFile, BeSQLite::Db::OpenMode mode);
    DgnElementKey InsertElement(Utf8CP elementCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId());
    void TwiddleTime(DgnElementKeyCR ekey);
};

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct ElementDependencyGraph : TransactionManagerTests
{
    enum class ElementDrivesElementColumn {DependentElementId,DependentElementClassId,RootElementId,RootElementClassId,Status};

    struct ElementsAndRelationships
        {
        DgnElementKey e99, e3, e31, e2, e1;
        EC::ECInstanceKey r99_3, r99_31, r3_2, r31_2, r2_1;
        };

    WString GetTestFileName(WCharCP testname);
    ECN::ECClassCR GetElementDrivesElementClass();

    EC::CachedECSqlStatementPtr GetSelectElementDrivesElementById();
    void SetUpForRelationshipTests(WCharCP testname);
    EC::ECInstanceKey InsertElementDrivesElementRelationship(DgnElementKeyCR root, DgnElementKeyCR dependent);

    void TestTPS(DgnElementKeyCR e1, DgnElementKeyCR e2, size_t ntimes);
    void TestOverlappingOrder(DgnElementKeyCR r1, EC::ECInstanceKeyCR r1_d3, EC::ECInstanceKeyCR r2_d3, bool r1First);
    void TestRelationships(DgnDb& db, ElementsAndRelationships const&);
};

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct Performance_ElementDependencyGraph : ElementDependencyGraph
{
    void DoPerformanceShallow(size_t depCount);
};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .dgndb project file after copying it to out
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isElementIdInKeySet(bset<EC::ECInstanceId> const& theSet, DgnElementId element)
    {
    return theSet.find(element) != theSet.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static bvector<EC::ECInstanceId>::const_iterator findRelId(bvector<EC::ECInstanceId> const& rels, EC::ECInstanceKey eid)
    {
    return std::find(rels.begin(), rels.end(), eid.GetECInstanceId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnMonitorVerifier::TxnMonitorVerifier() 
    {
    DgnPlatformLib::GetHost().GetTxnAdmin().AddTxnMonitor(*this);
    Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnMonitorVerifier::~TxnMonitorVerifier()
    {
    DgnPlatformLib::GetHost().GetTxnAdmin().DropTxnMonitor(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnMonitorVerifier::Clear() 
    {
    m_OnTxnClosedCalled = m_OnTxnReverseCalled = m_OnTxnReversedCalled = false;
    m_adds.clear(); m_deletes.clear(); m_mods.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnMonitorVerifier::_OnTxnCommit(TxnSummaryCR summary)
    {
    m_OnTxnClosedCalled = true;
    Statement stmt;
    stmt.Prepare(summary.GetDgnDb(), "SELECT ElementId,ChangeType FROM " TEMP_TABLE(TXN_TABLE_Elements));
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto eid = stmt.GetValueId<DgnElementId>(0);
        switch ((TxnSummary::ChangeType) stmt.GetValueInt(1))
            {
            case TxnSummary::ChangeType::Add:    m_adds.insert(eid); break;
            case TxnSummary::ChangeType::Delete: m_deletes.insert(eid); break;
            case TxnSummary::ChangeType::Update: m_mods.insert(eid); break;
            default:
                FAIL();
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TransactionManagerTestDomain::TransactionManagerTestDomain() : DgnDomain(TMTEST_SCHEMA_NAME, "DgnProject Test Schema", 1)
    {
    RegisterHandler(TestElementHandler::GetHandler());
    RegisterHandler(ABCHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
void ABCHandler::_OnRootChanged(DgnDbR db, BeSQLite::EC::ECInstanceId relationshipId, DgnElementId source, DgnElementId target, TxnSummaryR summary)
    {
    if (s_abcShouldFail)
        summary.ReportError(*new TxnSummary::ValidationError(TxnSummary::ValidationError::Severity::Warning, "ABC failed"));
    m_relIds.push_back(relationshipId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TransactionManagerTests::TransactionManagerTests()
    {
    // Must register my domain whenever I initialize a host
    DgnDomains::RegisterDomain(TransactionManagerTestDomain::GetDomain()); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TransactionManagerTests::~TransactionManagerTests()
    {
    }

/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .dgndb project file after copying it to out
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TransactionManagerTests::SetupProject(WCharCP projFile, WCharCP testFile, BeSQLite::Db::OpenMode mode)
    {
    BeFileName outFileName;
    ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, projFile, testFile, __FILE__));
    DbResult result;
    m_db = DgnDb::OpenDgnDb(&result, outFileName, DgnDb::OpenParams(mode));
    ASSERT_TRUE(m_db.IsValid());
    ASSERT_TRUE( result == BE_SQLITE_OK);

    BeFileName schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    schemaFile.AppendToPath(L"ECSchemas/" TMTEST_SCHEMA_NAMEW L".01.00.ecschema.xml");

    BentleyStatus status = TransactionManagerTestDomain::GetDomain().ImportSchema(*m_db, schemaFile);
    ASSERT_TRUE(BentleyStatus::SUCCESS == status);

    auto schema = m_db->Schemas().GetECSchema(TMTEST_SCHEMA_NAME, true);
    ASSERT_NE( nullptr , schema );
    ASSERT_NE( nullptr ,  TestElement::GetTestElementECClass(*m_db) );
    ASSERT_NE( nullptr ,  m_db->Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME) );

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    DgnModelP defaultModel = m_db->Models().GetModel(m_defaultModelId);
    ASSERT_NE( nullptr , defaultModel );
    GetDefaultModel().FillModel();

    m_defaultCategoryId = m_db->Categories().MakeIterator().begin().GetCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey TransactionManagerTests::InsertElement(Utf8CP elementCode, DgnModelId mid, DgnCategoryId categoryId )
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement::Create(*m_db, mid, categoryId, elementCode);
    return m_db->Elements().Insert(*el)->GetElementKey();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TransactionManagerTests::TwiddleTime(DgnElementKeyCR ekey)
    {
    BeThreadUtilities::BeSleep(1); // make sure the new timestamp is after the one that's on the Element now
    m_db->Elements().UpdateLastModifiedTime(ekey.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
WString ElementDependencyGraph::GetTestFileName(WCharCP testname)
    {
    return WPrintfString(L"ElementDependencyGraph_%ls.idgndb",testname);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCR ElementDependencyGraph::GetElementDrivesElementClass()
    {
    return *m_db->Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME);//"dgn", DGN_RELNAME_ElementDrivesElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
EC::CachedECSqlStatementPtr ElementDependencyGraph::GetSelectElementDrivesElementById()
    {
    BeSQLite::EC::ECSqlSelectBuilder b;
    #ifdef WIP_ECSQL_BUG
        // ERROR ECDb - Invalid ECSQL 'SELECT DependentElementId,DependentElementClassId,RootElementId,RootElementClassId,HandlerId,Status FROM ONLY [dgn].[ElementDrivesElement] WHERE ECInstanceId=?': ECProperty 'DependentElementId' not found in any of the ECClasses used in the ECSQL statement.
        b.Select("DependentElementId,DependentElementClassId,RootElementId,RootElementClassId,Status").From(GetElementDrivesElementClass(),false).Where("ECInstanceId=?");
    #else
        b.Select("TargetECInstanceId,TargetECClassId,SourceECInstanceId,SourceECClassId,Status").From(GetElementDrivesElementClass(),false).Where("ECInstanceId=?");
    #endif

    return m_db->GetPreparedECSqlStatement(b.ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementDependencyGraph::SetUpForRelationshipTests(WCharCP testname)
    {
    SetupProject(L"3dMetricGeneral.idgndb", GetTestFileName(testname).c_str(), BeSQLite::Db::OPEN_ReadWrite);

    auto abcHandlerInternalId = m_db->Domains().GetClassId(ABCHandler::GetHandler());

    auto dh = DgnElementDrivesElementDependencyHandler::GetHandler().FindHandler(*m_db, abcHandlerInternalId);
    auto ah = &ABCHandler::GetHandler();
    ASSERT_EQ((void*)dh,(void*)ah );

    m_db->Txns().EnableTracking(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
EC::ECInstanceKey ElementDependencyGraph::InsertElementDrivesElementRelationship(DgnElementKeyCR root, DgnElementKeyCR dependent)
    {
    EC::ECSqlInsertBuilder b;
    b.InsertInto(GetElementDrivesElementClass());
    b.AddValue("SourceECClassId", "?");
    b.AddValue("SourceECInstanceId", "?");
    b.AddValue("TargetECClassId", "?");
    b.AddValue("TargetECInstanceId", "?");

    EC::CachedECSqlStatementPtr stmt = m_db->GetPreparedECSqlStatement(b.ToString().c_str());

    stmt->BindInt64(1, root.GetECClassId());
    stmt->BindId(2, root.GetECInstanceId());
    stmt->BindInt64(3, dependent.GetECClassId());
    stmt->BindId(4, dependent.GetECInstanceId());

    EC::ECInstanceKey rkey;
    if (EC::ECSqlStepStatus::Done != stmt->Step(rkey))
        return EC::ECInstanceKey();

    return rkey;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static CurveVectorPtr computeShape()
    {
    static const double PLANE_LEN = 100;

    DPoint3d pts[6];
    pts[0] = DPoint3d::From(-PLANE_LEN,-PLANE_LEN);
    pts[1] = DPoint3d::From(+PLANE_LEN,-PLANE_LEN);
    pts[2] = DPoint3d::From(+PLANE_LEN,+PLANE_LEN);
    pts[3] = DPoint3d::From(-PLANE_LEN,+PLANE_LEN);
    pts[4] = pts[0];
    pts[5] = pts[0];
    pts[5].z = 1;

    return CurveVector::CreateLinear(pts, _countof(pts), CurveVector::BOUNDARY_TYPE_Open);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementPtr TestElement::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode)
    {
    DgnModelP model = db.Models().GetModel(mid);

    TestElementPtr testElement = new TestElement(CreateParams(*model, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId));

    //  Add some hard-wired geometry
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*testElement);
    builder->Append(*computeShape());
    if (SUCCESS != builder->SetGeomStreamAndPlacement(*testElement))
        return nullptr;

    testElement->m_itemState = ItemState::DoesNotExist;

    return testElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestElement::SetTestItemProperty(Utf8CP value)
    {
    m_testItemProperty.AssignOrClear(value);
    m_itemState = ItemState::Modified;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus TestElement::_InsertInDb()
    {
    DgnModelStatus status = T_Super::_InsertInDb();
    if (DGNMODEL_STATUS_Success != status)
        return status;

    if (HasTestItem())
        {
        BeSQLite::EC::CachedECSqlStatementPtr insertStmt = GetDgnDb().GetPreparedECSqlStatement("INSERT INTO " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME "(ECInstanceId," TMTEST_TEST_ITEM_TestItemProperty ") VALUES(?,?)");
        insertStmt->BindId(1, GetElementId());
        insertStmt->BindText(2, m_testItemProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        if (BeSQLite::EC::ECSqlStepStatus::Done != insertStmt->Step())
            return DGNMODEL_STATUS_ElementWriteError;
        }

    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus TestElement::_UpdateInDb()
    {
    DgnModelStatus status = T_Super::_UpdateInDb();
    if (DGNMODEL_STATUS_Success != status)
        return status;

    BeSQLite::EC::ECSqlStepStatus rc = BeSQLite::EC::ECSqlStepStatus::Done;
    if (ItemState::Deleted == m_itemState)
        {
        BeSQLite::EC::CachedECSqlStatementPtr delStmt = GetDgnDb().GetPreparedECSqlStatement("DELETE FROM " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME " WHERE(ECInstanceId=?)");
        delStmt->BindId(1, GetElementId());
        rc = delStmt->Step();
        if (BeSQLite::EC::ECSqlStepStatus::Done == rc)
            m_itemState = ItemState::DoesNotExist;
        }
    else if (ItemState::Modified == m_itemState)
        {
#ifdef ECSQL_SUPPORTS_INSERT_OR_REPLACE
        BeSQLite::EC::CachedECSqlStatementPtr writeStmt = GetDgnDb().GetPreparedECSqlStatement("INSERT OR REPLACE INTO " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME "(ECInstanceId," TMTEST_TEST_ITEM_TestItemProperty ") VALUES(?,?)");
        writeStmt->BindId(2, GetElementId());
        writeStmt->BindText(1, m_testItemProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        rc = writeStmt->Step();
#else
        BeSQLite::EC::CachedECSqlStatementPtr updStmt = GetDgnDb().GetPreparedECSqlStatement("UPDATE " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME " SET " TMTEST_TEST_ITEM_TestItemProperty "=? WHERE(ECInstanceId=?)");
        updStmt->BindId(2, GetElementId());
        updStmt->BindText(1, m_testItemProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        if (BeSQLite::EC::ECSqlStepStatus::Done !=(rc = updStmt->Step()))
            {
            // Update failed. There's no way to tell why. Maybe it's because the item doesn't exist yet in the DB. Try an insert.
            BeSQLite::EC::CachedECSqlStatementPtr insertStmt = GetDgnDb().GetPreparedECSqlStatement("INSERT INTO " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME "(ECInstanceId," TMTEST_TEST_ITEM_TestItemProperty ") VALUES(?,?)");
            insertStmt->BindId(1, GetElementId());
            insertStmt->BindText(2, m_testItemProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
            rc = insertStmt->Step();
            }
#endif
        if (BeSQLite::EC::ECSqlStepStatus::Done == rc)
            m_itemState = ItemState::Exists;
        }

    if (BeSQLite::EC::ECSqlStepStatus::Done != rc)
        status = DGNMODEL_STATUS_ElementWriteError;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus TestElement::_LoadFromDb()
    {
    DgnModelStatus status = T_Super::_LoadFromDb();
    if (DGNMODEL_STATUS_Success != status)
        return status;

    BeSQLite::EC::CachedECSqlStatementPtr itemStmt = GetDgnDb().GetPreparedECSqlStatement("SELECT " TMTEST_TEST_ITEM_TestItemProperty " FROM " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME " WHERE(ECInstanceId=?)");
    itemStmt->BindId(1, GetElementId());

    if (BeSQLite::EC::ECSqlStepStatus::HasRow == itemStmt->Step())
        {
        m_testItemProperty = itemStmt->GetValueText(0);
        m_itemState = ItemState::Exists;
        }
    else
        {
        m_itemState = ItemState::DoesNotExist;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus TestElement::_CopyFrom(DgnElementCR rhs)
    {
    DgnModelStatus status = T_Super::_CopyFrom(rhs);
    if (DGNMODEL_STATUS_Success != status)
        return status;

    auto trhs = dynamic_cast<TestElement const*>(&rhs);
    m_testItemProperty = trhs->m_testItemProperty;
    m_itemState = trhs->m_itemState;

    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* Test of StreetMapModel
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, StreetMapModel)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests_StreetMapModel.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    auto newModelId = StreetMapModelHandler::CreateStreetMapModel(*m_db, StreetMapModelHandler::MapService::MapQuest, StreetMapModelHandler::MapType::SatelliteImage, true);
    ASSERT_TRUE( newModelId.IsValid() );

    DgnModelP model = m_db->Models().GetModel(newModelId);
    ASSERT_NE( nullptr , model );

    WebMercatorModel* webmercatormodel = dynamic_cast<WebMercatorModel*>(model);
    ASSERT_NE( nullptr , webmercatormodel );
    }

/*---------------------------------------------------------------------------------**//**
* Test of Element CRUD
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, CRUD)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests_CRUD.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    m_db->SaveChanges();
    m_db->Txns().EnableTracking(true);
    TxnMonitorVerifier monitor;

    //  -------------------------------------------------------------
    //  Test adds
    //  -------------------------------------------------------------
    auto key1 = InsertElement("E1");
    ASSERT_TRUE( key1.GetElementId().IsValid() );

    auto key2 = InsertElement("E2");
    ASSERT_TRUE( key2.GetElementId().IsValid() );

    m_db->SaveChanges();

    EXPECT_TRUE(monitor.m_OnTxnClosedCalled);
    ASSERT_EQ(monitor.m_adds.size()     , 2);
    ASSERT_EQ(monitor.m_deletes.size()  , 0);
    ASSERT_EQ(monitor.m_mods.size()     , 0);
    ASSERT_TRUE(isElementIdInKeySet(monitor.m_adds, key1.GetElementId()) );
    ASSERT_TRUE(isElementIdInKeySet(monitor.m_adds, key2.GetElementId()) );

    monitor.Clear();

    //  -------------------------------------------------------------
    //  Test mods
    //  -------------------------------------------------------------
    TwiddleTime(key1);
    m_db->SaveChanges();

    EXPECT_TRUE(monitor.m_OnTxnClosedCalled);
    ASSERT_EQ(monitor.m_adds.size()     , 0);
    ASSERT_EQ(monitor.m_deletes.size()  , 0);
    ASSERT_EQ(monitor.m_mods.size()     , 1);
    ASSERT_TRUE(isElementIdInKeySet(monitor.m_mods, key1.GetElementId()) );

    monitor.Clear();

    //  -------------------------------------------------------------
    //  Test deletes
    //  -------------------------------------------------------------
    auto delStatus = m_db->Elements().Delete(key2.GetElementId());
    ASSERT_TRUE( BSISUCCESS == delStatus );

    m_db->SaveChanges();

    EXPECT_TRUE(monitor.m_OnTxnClosedCalled);
    ASSERT_EQ(monitor.m_adds.size()     , 0);
    ASSERT_EQ(monitor.m_deletes.size()  , 1);
    ASSERT_EQ(monitor.m_mods.size()     , 0);
    ASSERT_TRUE(isElementIdInKeySet(monitor.m_deletes, key2.GetElementId()) );

    monitor.Clear();
    }

/*---------------------------------------------------------------------------------**//**
* Test of element instance access
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, ElementInstance)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    auto key1 = InsertElement("E1");
    ASSERT_TRUE( key1.GetElementId().IsValid() );

    DgnElementCPtr el = m_db->Elements().GetElement(key1.GetElementId());
    ASSERT_TRUE( el.IsValid() );

    ASSERT_EQ( &el->GetElementHandler(), &TestElementHandler::GetHandler() );

#ifdef WIP_NEED_SOME_OTHER_WAY_TO_ACCESS_SUB_CLASS_PROPERTIES
    ECN::IECInstanceCR e1props = el->GetSubclassProperties();
    ECN::ECValue v;
    ASSERT_EQ( e1props.GetValue(v, TMTEST_TEST_ELEMENT_TestElementProperty) , ECN::ECOBJECTS_STATUS_Success );
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void checkItemProperty(TestElementCR el, bool shouldBeThere, Utf8CP propValue)
    {
    if (!shouldBeThere)
        {
        ASSERT_TRUE( !el.HasTestItem() );
        return;
        }

    Utf8String value = el.GetTestItemProperty();
    ASSERT_TRUE( value.Equals(propValue) );
    }

/*---------------------------------------------------------------------------------**//**
* Test of inserting, modifying, and deleting an element's item.
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, ElementItem)
    {
    Utf8CP initialTestPropValue = "Test";
    Utf8CP changedTestPropValue = "Test - changed";

    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    ECN::ECClassCP testItemClass = m_db->Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_ITEM_CLASS_NAME);
    ASSERT_NE( nullptr , testItemClass );

    DgnElementId elementId = InsertElement("E1").GetElementId();
    ASSERT_TRUE( elementId.IsValid() );

    TestElementCPtr el = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE( el != nullptr );

    ASSERT_EQ( &el->GetElementHandler(), &TestElementHandler::GetHandler() );

    checkItemProperty(*el, false, nullptr);

    TestElementPtr mod = m_db->Elements().GetForEdit<TestElement>(el->GetElementId());

    DgnModelStatus mstatus;

    //  Add an item
    mod->SetTestItemProperty(initialTestPropValue);
    mod->Update(&mstatus);
    ASSERT_EQ( DGNMODEL_STATUS_Success , mstatus );

    // *** NB: I am assuming that 'el' is still valid and still points to the REAL element!

    checkItemProperty(*el, true, initialTestPropValue);

    //  Update the item
    mod->SetTestItemProperty(changedTestPropValue);
    mod->Update(&mstatus);
    ASSERT_EQ( DGNMODEL_STATUS_Success , mstatus );

    checkItemProperty(*el, true, changedTestPropValue); // item should now be in the DB

    //  Delete the item
    mod->DeleteTestItem();
    mod->Update(&mstatus);
    ASSERT_EQ( DGNMODEL_STATUS_Success , mstatus );

    checkItemProperty(*el, false, nullptr); // item should now be gone in the DB
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementDependencyGraph::TestRelationships(DgnDb& db, ElementsAndRelationships const& g)
    {
    db.SaveChanges();

    TxnMonitorVerifier monitor;

    //  ----------------
    //     o->e31-o
    //    /        \
    // e99          ->e2-o->e1
    //    \        /
    //     o->e3-o
    //
    // (The little "o"s represent the ECRelationships.)
    //  ----------------

    //  ----------------
    //  change e99 =>
    //  first   next    last
    //  r99_3   r3_2    r2_1
    //  r99_31  r31_2
    //  Note that, since r99_3 and r99_31 don't depend on each other, they could be scheduled in either order. Ditto for r3_2 and r31_2.
    //  ----------------
    TwiddleTime(g.e99);

    monitor.Clear();
    ABCHandler::GetHandler().Clear();
    db.SaveChanges();
    if (true)
        {
        auto const& rels = ABCHandler::GetHandler().m_relIds;
        ASSERT_EQ( rels.size() ,5 );
        auto i99_3  = findRelId(rels, g.r99_3);    ASSERT_NE(i99_3 , rels.end());
        auto i99_31 = findRelId(rels, g.r99_31);   ASSERT_NE(i99_31, rels.end());
        auto i3_2   = findRelId(rels, g.r3_2);     ASSERT_NE(i3_2  , rels.end());
        auto i31_2  = findRelId(rels, g.r31_2);    ASSERT_NE(i31_2 , rels.end());
        auto i2_1   = findRelId(rels, g.r2_1);     ASSERT_NE(i2_1  , rels.end());

        ASSERT_LT(i99_3  , i3_2);
        ASSERT_LT(i99_31 , i31_2);
        ASSERT_LT(i3_2   , i2_1);
        ASSERT_LT(i31_2  , i2_1);
        }

    //  ----------------
    //  change e99, e2 => same as above
    //  ----------------
    TwiddleTime(g.e99);
    TwiddleTime(g.e2);

    monitor.Clear();
    ABCHandler::GetHandler().Clear();
    db.SaveChanges();   // ==> Triggers callbacks to ABCHandler::GetHandler()
    if (true)
        {
        auto const& rels = ABCHandler::GetHandler().m_relIds;
        ASSERT_EQ( rels.size() , 5 );
        auto i99_3  = findRelId(rels, g.r99_3);    ASSERT_NE(i99_3 , rels.end());
        auto i99_31 = findRelId(rels, g.r99_31);   ASSERT_NE(i99_31, rels.end());
        auto i3_2   = findRelId(rels, g.r3_2);     ASSERT_NE(i3_2  , rels.end());
        auto i31_2  = findRelId(rels, g.r31_2);    ASSERT_NE(i31_2 , rels.end());
        auto i2_1   = findRelId(rels, g.r2_1);     ASSERT_NE(i2_1  , rels.end());
        
        ASSERT_LT(i99_3  , i3_2);
        ASSERT_LT(i99_31 , i31_2); 
        ASSERT_LT(i3_2   , i2_1);
        ASSERT_LT(i31_2  , i2_1);
        }

    //  ----------------
    //  change e31 =>
    //  r99_31 r31_2    r2_1
    //         r3_2
    //  ----------------
    TwiddleTime(g.e31);

    monitor.Clear();
    ABCHandler::GetHandler().Clear();
    db.SaveChanges();   // ==> Triggers callbacks to ABCHandler::GetHandler()
    if (true)
        {
        auto const& rels = ABCHandler::GetHandler().m_relIds;
        ASSERT_EQ( rels.size() , 4 );
        auto i99_31 = findRelId(rels, g.r99_31);   ASSERT_NE(i99_31  , rels.end());
        auto i31_2  = findRelId(rels, g.r31_2);    ASSERT_NE(i31_2  , rels.end());
        auto i3_2   = findRelId(rels, g.r3_2);     ASSERT_NE(i3_2  , rels.end());
        auto i2_1   = findRelId(rels, g.r2_1);     ASSERT_NE(i2_1  , rels.end());
        
        ASSERT_LT(i99_31 , i31_2);
        ASSERT_LT(i31_2 , i2_1);
        ASSERT_LT(i3_2 , i2_1);
        }

    //  ----------------
    //  change e31,e3 =>
    //  r99_3   r3_2
    //  r99_31  r31_2    r2_1
    //  ----------------
    TwiddleTime(g.e3);
    TwiddleTime(g.e31);

    monitor.Clear();
    ABCHandler::GetHandler().Clear();
    db.SaveChanges();   // ==> Triggers callbacks to ABCHandler::GetHandler()
    if (true)
        {
        auto const& rels = ABCHandler::GetHandler().m_relIds;
        ASSERT_EQ( rels.size() , 5 );
        auto i99_3  = findRelId(rels, g.r99_3);        ASSERT_NE(i99_3  , rels.end());
        auto i99_31 = findRelId(rels, g.r99_31);       ASSERT_NE(i99_31  , rels.end());
        auto i3_2   = findRelId(rels, g.r3_2);         ASSERT_NE(i3_2  , rels.end());
        auto i31_2  = findRelId(rels, g.r31_2);        ASSERT_NE(i31_2  , rels.end());
        auto i2_1   = findRelId(rels, g.r2_1);         ASSERT_NE(i2_1  , rels.end());
        
        ASSERT_LT(i99_3  , i3_2);
        ASSERT_LT(i99_31 , i31_2);
        ASSERT_LT(i3_2  , i2_1);
        }

    //  ----------------
    //  change e2 =>
    //  r3_2    r2_1
    //  r31_2    
    //  ----------------
    TwiddleTime(g.e2);

    monitor.Clear();
    ABCHandler::GetHandler().Clear();
    db.SaveChanges();   // ==> Triggers callbacks to ABCHandler::GetHandler()
    if (true)
        {
        auto const& rels = ABCHandler::GetHandler().m_relIds;
        ASSERT_EQ( rels.size() , 3);
        auto i3_2   = findRelId(rels, g.r3_2);     ASSERT_NE(i3_2  , rels.end());
        auto i31_2  = findRelId(rels, g.r31_2);    ASSERT_NE(i31_2 , rels.end());
        auto i2_1   = findRelId(rels, g.r2_1);     ASSERT_NE(i2_1  , rels.end());
        
        ASSERT_LT(i3_2  , i2_1);
        ASSERT_LT(i31_2 , i2_1);
        }

    //  ----------------
    //  change e1 =>
    //  r2_1 should get a "check" callback
    //  ----------------
    TwiddleTime(g.e1);

    monitor.Clear();
    ABCHandler::GetHandler().Clear();
    db.SaveChanges();   // ==> Triggers callbacks to ABCHandler::GetHandler()
    if (true)
        {
        auto const& rels = ABCHandler::GetHandler().m_relIds;
        ASSERT_EQ( rels.size() , 1);
        auto i2_1   = findRelId(rels, g.r2_1);      ASSERT_NE(i2_1  , rels.end());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, DiamondTest1)
    {
    SetUpForRelationshipTests(L"DiamondTest1");

    ElementsAndRelationships g;
    g.e99 = InsertElement("E99");
    g.e3  = InsertElement("E3");
    g.e31 = InsertElement("E31");
    g.e2  = InsertElement("E2");
    g.e1  = InsertElement("E1");

    g.r99_3  = InsertElementDrivesElementRelationship(g.e99, g.e3);
    g.r99_31 = InsertElementDrivesElementRelationship(g.e99, g.e31);
    g.r3_2   = InsertElementDrivesElementRelationship(g.e3,  g.e2);
    g.r31_2  = InsertElementDrivesElementRelationship(g.e31, g.e2);
    g.r2_1   = InsertElementDrivesElementRelationship(g.e2,  g.e1);

    TestRelationships(*m_db, g);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, DiamondTest2)
    {
    SetUpForRelationshipTests(L"DiamondTest2");

    // This is the same as DiamondTest1, except that we create the elements and relationships in a different order.
    //  The hope is to catch things that only happen to work because of the order of the rows in the tables.

    ElementsAndRelationships g;
    g.e1  = InsertElement("E1");
    g.e31 = InsertElement("E31");
    g.e2  = InsertElement("E2");
    g.e3  = InsertElement("E3");
    g.e99 = InsertElement("E99");

    g.r99_31 = InsertElementDrivesElementRelationship(g.e99, g.e31);
    g.r31_2  = InsertElementDrivesElementRelationship(g.e31, g.e2);
    g.r3_2   = InsertElementDrivesElementRelationship(g.e3,  g.e2);
    g.r2_1   = InsertElementDrivesElementRelationship(g.e2,  g.e1);
    g.r99_3  = InsertElementDrivesElementRelationship(g.e99, g.e3);

    TestRelationships(*m_db, g);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Performance_ElementDependencyGraph, PerformanceDeep1)
    {
    SetUpForRelationshipTests(L"PerformanceDeep1");

    static size_t s_nElements = 512;

    //  Create 10000 Elements, and make each depend on the previous one.
    EC::ECInstanceKey firstRel,previousRel,thisRel,lastRel;
    DgnElementKey firstElement, previousElement, thisElement, lastElement;

    if (true)
        {
        StopWatch timer("Inserts", true);

        for (size_t i=0; i<s_nElements; ++i)
            {
            previousElement = thisElement;
            thisElement = InsertElement(Utf8PrintfString("E%d",(int)i));
        
            if (!firstElement.IsValid())
                firstElement = thisElement;
            else
                lastElement = thisElement;

            if (previousElement.IsValid())
                {
                previousRel = thisRel;
                thisRel = InsertElementDrivesElementRelationship(previousElement, thisElement);

                if (!firstRel.IsValid())
                    firstRel = thisRel;
                else
                    lastRel = thisRel;
                }
            }

        m_db->SaveChanges();

        timer.Stop();
        BeTest::Log("ElementDependencyGraph", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("Inserts: %lf seconds", timer.GetElapsedSeconds()));
        }

    // Modify the first Element => triggers all handlers, in order
    if (true)
        {
        TwiddleTime(firstElement);
        StopWatch timer("Mod 1st", true);
        ABCHandler::GetHandler().Clear();
        m_db->SaveChanges();
        timer.Stop();
        BeTest::Log("ElementDependencyGraph", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("Mod 1st: %lf seconds", timer.GetElapsedSeconds()));
        auto const& rels = ABCHandler::GetHandler().m_relIds;
        ASSERT_EQ( rels.size()  , s_nElements-1);
        ASSERT_EQ( rels.front() , firstRel.GetECInstanceId() );
        ASSERT_EQ( rels.back()  , lastRel.GetECInstanceId() );
        }

    // Modify the last Element => triggers last handler
    if (true)
        {
        TwiddleTime(lastElement);
        StopWatch timer("Mod last", true);
        ABCHandler::GetHandler().Clear();
        m_db->SaveChanges();
        timer.Stop();
        BeTest::Log("ElementDependencyGraph", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("Mod last: %lf seconds", timer.GetElapsedSeconds()));
        auto const& rels = ABCHandler::GetHandler().m_relIds;
        ASSERT_EQ( rels.size() , 1);
        ASSERT_EQ( rels.front(), lastRel.GetECInstanceId() );
        }

    // Modify the next-to-last Element => triggers 2 handlers, the last one, and the previous
    if (true)
        {
        TwiddleTime(previousElement);
        StopWatch timer("Mod next to last", true);
        ABCHandler::GetHandler().Clear();
        m_db->SaveChanges();
        timer.Stop();
        BeTest::Log("ElementDependencyGraph", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("Mod next to last: %lf seconds", timer.GetElapsedSeconds()));
        auto const& rels = ABCHandler::GetHandler().m_relIds;
        ASSERT_EQ( rels.size() , 2);
        ASSERT_EQ( rels.back() , lastRel.GetECInstanceId() );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Performance_ElementDependencyGraph::DoPerformanceShallow(size_t depCount)
    {
    SetUpForRelationshipTests(L"PerformanceShallow");

    //  Create the "root" Element. All other Elements will depend on this.
    DgnElementKey rootElement = InsertElement("Root");

    //  Create a bunch of Elements, and make each depend on the single rootElement
    EC::ECInstanceKey firstRel, lastRel;
    DgnElementKey firstDependentElement, lastDependentElement;
    if (true)
        {
        StopWatch timer("Inserts", true);

        for (size_t i=0; i<depCount; ++i)
            {
            auto thisElement = InsertElement(Utf8PrintfString("E%d",(int)i));

            if (!firstDependentElement.IsValid())
                firstDependentElement = thisElement;
            else
                lastDependentElement = thisElement;
        
            auto thisRel = InsertElementDrivesElementRelationship(rootElement, thisElement);

            if (!firstRel.IsValid())
                firstRel = thisRel;
            else
                lastRel = thisRel;
            }

        m_db->SaveChanges();

        timer.Stop();
        BeTest::Log("ElementDependencyGraph", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("Inserts: %lf seconds", timer.GetElapsedSeconds()));
        }

    // Modify rootElement => triggers all handlers (in no particular order)
    if (true)
        {
        TwiddleTime(rootElement);
        StopWatch timer("Mod Root", true);
        ABCHandler::GetHandler().Clear();
        m_db->SaveChanges();
        timer.Stop();
        BeTest::Log("ElementDependencyGraph", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("Mod Root: %lf seconds", timer.GetElapsedSeconds()));
        auto const& rels = ABCHandler::GetHandler().m_relIds;
        ASSERT_EQ( rels.size()  , depCount);
        ASSERT_EQ( rels.front() , firstRel.GetECInstanceId() );
        ASSERT_EQ( rels.back()  , lastRel.GetECInstanceId() );
        }

    // Modify a couple of the dependent Elements => triggers the handlers that output them (as checks)
    if (true)
        {
        TwiddleTime(firstDependentElement);
        TwiddleTime(lastDependentElement);
        StopWatch timer("Mod dependents", true);
        ABCHandler::GetHandler().Clear();
        m_db->SaveChanges();
        timer.Stop();
        BeTest::Log("ElementDependencyGraph", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("Mod dependents: %lf seconds", timer.GetElapsedSeconds()));
        auto const& rels = ABCHandler::GetHandler().m_relIds;
        ASSERT_EQ( rels.size() , 2);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Performance_ElementDependencyGraph, PerformanceShallow10)    {DoPerformanceShallow(10);}
TEST_F(Performance_ElementDependencyGraph, PerformanceShallow100)   {DoPerformanceShallow(100);}
TEST_F(Performance_ElementDependencyGraph, PerformanceShallow1000)  {DoPerformanceShallow(1000);}
TEST_F(Performance_ElementDependencyGraph, PerformanceShallow10000) {DoPerformanceShallow(10000);}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, NonDependencyOrderTest)
    {
    SetUpForRelationshipTests(L"NonDependencyOrderTest");
    auto w1 = InsertElement("w1");
    auto c1 = InsertElement("c1");
    auto w2 = InsertElement("w2");
    auto w3 = InsertElement("w3");

    //  w2 --> c1 <--- w3
    //          |
    //          v
    //          w1
    auto w2_c1 = InsertElementDrivesElementRelationship(w2, c1);
    auto w3_c1 = InsertElementDrivesElementRelationship(w3, c1);
    auto c1_w1 = InsertElementDrivesElementRelationship(c1, w1);

    m_db->SaveChanges();

    if (true)
        {
        TwiddleTime(w2);
        TwiddleTime(w3);

        ABCHandler::GetHandler().Clear();
        m_db->SaveChanges();

        auto const& rels = ABCHandler::GetHandler().m_relIds;

        //  w3_c1 and w2_c1 are fired first. Then c1_w1 is fired.
        ASSERT_EQ( rels.size() , 3);
        auto iw2_c1    = findRelId(rels, w2_c1);     ASSERT_NE( iw2_c1,  rels.end() );
        auto iw3_c1    = findRelId(rels, w3_c1);     ASSERT_NE( iw3_c1,  rels.end() );
        auto ic1_w1    = findRelId(rels, c1_w1);     ASSERT_NE( ic1_w1,  rels.end() );
        ASSERT_LT( iw2_c1, ic1_w1 );
        ASSERT_LT( iw3_c1, ic1_w1 );

        // w2_c1 is fired before w3_c1, because w2_c1 was created first
        ASSERT_LT( iw2_c1   ,   iw3_c1 );
        }

    // Add a direct w2->w1 edge
    //
    //  w2 --> c1 <--- w3
    //   |      |
    //   |      v
    //   +--->  w1
    auto w2_w1 = InsertElementDrivesElementRelationship(w2, w1);

    m_db->SaveChanges();

    if (true)
        {
        TwiddleTime(w2);

        ABCHandler::GetHandler().Clear();
        m_db->SaveChanges();

        auto const& rels = ABCHandler::GetHandler().m_relIds;

        ASSERT_EQ( rels.size() , 4);
        auto iw2_c1    = findRelId(rels, w2_c1);     ASSERT_NE( iw2_c1,  rels.end() );
        auto iw2_w1    = findRelId(rels, w2_w1);     ASSERT_NE( iw2_w1,  rels.end() );
        auto ic1_w1    = findRelId(rels, c1_w1);     ASSERT_NE( ic1_w1,  rels.end() );
        auto iw3_c1    = findRelId(rels, w3_c1);     ASSERT_NE( iw3_c1,  rels.end() );

        //  w2_c1 preceeds c1_w1 because of the explicit dependency
        ASSERT_LT( iw2_c1   ,   ic1_w1 );
        
        //  w3_c1 is invoked because it must have another crack at its output. It preceeded c1_w1 because of the explicit dependency.
        ASSERT_LT( iw3_c1   ,   ic1_w1 );

        // c1_w1 is fired before w2_w1, because c1_c1 was created first.
        ASSERT_LT( ic1_w1   ,   iw2_w1 );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementDependencyGraph::TestOverlappingOrder(DgnElementKeyCR r1, EC::ECInstanceKeyCR r1_d3, EC::ECInstanceKeyCR r2_d3, bool r1First)
    {
    m_db->SaveChanges();

    TwiddleTime(r1);

    ABCHandler::GetHandler().Clear();
    m_db->SaveChanges();

    auto const& rels = ABCHandler::GetHandler().m_relIds;
    ASSERT_EQ( rels.size(), 2 );
    auto ir1_d3 = findRelId(rels, r1_d3);       ASSERT_NE( ir1_d3, rels.end() );
    auto ir2_d3 = findRelId(rels, r2_d3);       ASSERT_NE( ir2_d3, rels.end() );
    ASSERT_EQ((ir1_d3 < ir2_d3 ) , r1First );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, OverlappingOutputOrderTest1)
    {
    SetUpForRelationshipTests(L"NonDependencyOrderTest");

    auto r1 = InsertElement("r1");
    auto r2 = InsertElement("r2");
    auto d3 = InsertElement("d2");

    // r1->d3 comes first
    auto r1_d3 = InsertElementDrivesElementRelationship(r1, d3);
    auto r2_d3 = InsertElementDrivesElementRelationship(r2, d3);

    TestOverlappingOrder(r1, r1_d3, r2_d3, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, OverlappingOutputOrderTest2)
    {
    SetUpForRelationshipTests(L"NonDependencyOrderTest");

    auto r1 = InsertElement("r1");
    auto r2 = InsertElement("r2");
    auto d3 = InsertElement("d2");

    // r2->d3 comes first
    auto r2_d3 = InsertElementDrivesElementRelationship(r2, d3);
    auto r1_d3 = InsertElementDrivesElementRelationship(r1, d3);

    TestOverlappingOrder(r1, r1_d3, r2_d3, false);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, FailureTest1)
    {
    SetUpForRelationshipTests(L"FailureTest1");

    DgnElementKey e1 = InsertElement("E1");
    DgnElementKey e2 = InsertElement("E2");
    EC::ECInstanceKey e1_e2 = InsertElementDrivesElementRelationship(e1, e2);

    EC::CachedECSqlStatementPtr selectDepRel = GetSelectElementDrivesElementById();
    selectDepRel->BindId(1, e1_e2.GetECInstanceId());

    m_db->SaveChanges();

    ASSERT_EQ( selectDepRel->Step(), EC::ECSqlStepStatus::HasRow );
    ASSERT_EQ( selectDepRel->GetValueInt((int)ElementDrivesElementColumn::Status),(int)DgnElementDependencyGraph::EdgeStatus::EDGESTATUS_Satisfied );

    AbcShouldFail fail;
    TwiddleTime(e1);
    m_db->SaveChanges();

    selectDepRel->Reset();
    ASSERT_EQ( selectDepRel->Step(), EC::ECSqlStepStatus::HasRow );
    ASSERT_EQ( selectDepRel->GetValueInt((int)ElementDrivesElementColumn::Status),(int)DgnElementDependencyGraph::EdgeStatus::EDGESTATUS_Failed );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, CycleTest1)
    {
    SetUpForRelationshipTests(L"CycleTest1");

    //  Two Elements
    auto e1 = InsertElement("E1");
    auto e2 = InsertElement("E2");

    //  Forward dependency relationship
    auto e1_e2 = InsertElementDrivesElementRelationship(e1, e2);
    ASSERT_TRUE( e1_e2.IsValid() );

    m_db->SaveChanges();

    if (true)
        {
        // Attempt to create backward relationship, which would cause a cycle.
        auto e2_e1 = InsertElementDrivesElementRelationship(e2, e1);
        ASSERT_TRUE( e2_e1.IsValid() );

        // Trigger graph evaluation, which will detect the cycle.
        TwiddleTime(e1);

        ABCHandler::GetHandler().Clear();
        m_db->SaveChanges();
 
// It's still undecided as to whether handlers should be called or not.  
//        auto const& rels = ABCHandler::GetHandler().m_relIds;
//        ASSERT_EQ( rels.size() , 0) << L" my dependency handler should not have been called, because of the graph-building error";


        // Verify that the txn was rolled back. If so, my insert of e2_e1 should have been cancelled, and e2_e1 should not exist.
        EC::ECSqlSelectBuilder b;
        b.Select("*").From(*m_db->Schemas().GetECClass(e2_e1.GetECClassId())).Where("ECInstanceId = ?");
        EC::ECSqlStatement s;
        s.Prepare(*m_db, b.ToString().c_str());
        s.BindId(1, e2_e1.GetECInstanceId());
        ASSERT_EQ( s.Step() , EC::ECSqlStepStatus::Done );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, CycleTest2)
    {
    SetUpForRelationshipTests(L"CycleTest1");

    //  Two Elements
    DgnElementKey e1 = InsertElement("E1");
    DgnElementKey e2 = InsertElement("E2");
    DgnElementKey e3 = InsertElement("E3");
    DgnElementKey e4 = InsertElement("E4");

    //  Forward dependency relationship
    InsertElementDrivesElementRelationship(e1, e2);
    InsertElementDrivesElementRelationship(e2, e3);
    InsertElementDrivesElementRelationship(e3, e4);

    m_db->SaveChanges();

    if (true)
        {
        // Attempt to create backward relationship, which would cause a cycle.
        EC::ECInstanceKey e4_e2 = InsertElementDrivesElementRelationship(e4, e2);

        // Trigger graph evaluation, which will detect the cycle.
        TwiddleTime(e1);

        ABCHandler::GetHandler().Clear();
        m_db->SaveChanges();
 
        // Verify that the txn was rolled back. If so, my insert of e2_e1 should have been cancelled, and e2_e1 should not exist.
        EC::CachedECSqlStatementPtr getRelDep = GetSelectElementDrivesElementById();
        getRelDep->BindId(1, e4_e2.GetECInstanceId());
        ASSERT_EQ( getRelDep->Step() , EC::ECSqlStepStatus::Done );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementDependencyGraph::TestTPS(DgnElementKeyCR e1, DgnElementKeyCR e2, size_t ntimes)
    {
    m_db->SaveChanges();

    //  At this point there are no relationships or dependency handlers

    if (true)
        {
        //  ------------------------------------------------
        StopWatch timer("Nops", true);
        for (size_t i=0; i<ntimes; ++i)
            {
            m_db->SaveChanges();
            }
        timer.Stop();
        printf("Nops: %lf\n", ntimes/timer.GetElapsedSeconds());
        }

    if (true)
        {
        //  ------------------------------------------------
        StopWatch timer("change", true);
        for (size_t i=0; i<ntimes; ++i)
            {
            TwiddleTime(e1);
            m_db->SaveChanges();
            }
        timer.Stop();
        printf("change: %lf\n", ntimes/timer.GetElapsedSeconds());
        }

    //  Add a relationship with a dependency handler

    auto rel = InsertElementDrivesElementRelationship(e1, e2);
    m_db->SaveChanges();

    if (true)
        {
        //  ------------------------------------------------
        StopWatch timer("change e1, propagate to e2", true);
        ABCHandler::GetHandler().Clear();
        for (size_t i=0; i<ntimes; ++i)
            {
            TwiddleTime(e1);
            m_db->SaveChanges();
            }
        timer.Stop();
        printf("change e1, propagate to e2: %lf\n", ntimes/timer.GetElapsedSeconds());
        ASSERT_EQ( ntimes, ABCHandler::GetHandler().m_relIds.size() );
        }

    if (true)
        {
        //  ------------------------------------------------
        StopWatch timer("change e2", true);
        ABCHandler::GetHandler().Clear();
        for (size_t i=0; i<ntimes; ++i)
            {
            TwiddleTime(e2);
            m_db->SaveChanges();
            }
        timer.Stop();
        printf("change e12: %lf\n", ntimes/timer.GetElapsedSeconds());
        ASSERT_EQ( ntimes, ABCHandler::GetHandler().m_relIds.size() );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Performance_ElementDependencyGraph, TPS20000)
    {
    SetUpForRelationshipTests(L"TPS");

    // Create a pretty large DgnDb. We will be working with just a couple of the Elements in it.

    for (auto i=0; i<10000; ++i)
        InsertElement(Utf8PrintfString("X%d", i));

    auto e1 = InsertElement("E1");
    auto e2 = InsertElement("E2");

    for (auto i=0; i<10000; ++i)
        InsertElement(Utf8PrintfString("X%d", 100000+i));

    TestTPS(e1,e2,50);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Performance_ElementDependencyGraph, TPS200000)
    {
    SetUpForRelationshipTests(L"TPS");

    // Create a pretty large DgnDb. We will be working with just a couple of the Elements in it.

    for (auto i=0; i<100000; ++i)
        InsertElement(Utf8PrintfString("X%d", i));

    auto e1 = InsertElement("E1");
    auto e2 = InsertElement("E2");

    for (auto i=0; i<100000; ++i)
        InsertElement(Utf8PrintfString("X%d", 100000+i));

    TestTPS(e1,e2,50);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Performance_ElementDependencyGraph, TPS4000000)
    {
    SetUpForRelationshipTests(L"TPS");

    // Create a pretty large DgnDb. We will be working with just a couple of the Elements in it.

    for (auto i=0; i<200000; ++i)
        InsertElement(Utf8PrintfString("X%d", i));

    auto e1 = InsertElement("E1");
    auto e2 = InsertElement("E2");

    for (auto i=0; i<200000; ++i)
        InsertElement(Utf8PrintfString("X%d", 100000+i));

    TestTPS(e1,e2,20);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, ModelDependenciesTest)
    {
    SetUpForRelationshipTests(L"ModelDependenciesTest");

    //  Create models 1-4
    auto seedModelId = m_defaultModelId;
    auto m4 = m_db->Models().CreateNewModelFromSeed(NULL, "m4", seedModelId);
    auto m3 = m_db->Models().CreateNewModelFromSeed(NULL, "m3", seedModelId);
    auto m1 = m_db->Models().CreateNewModelFromSeed(NULL, "m1", seedModelId);
    auto m2 = m_db->Models().CreateNewModelFromSeed(NULL, "m2", seedModelId);

    auto m1id = m1->GetModelId();
    auto m2id = m2->GetModelId();
    auto m3id = m3->GetModelId();
    auto m4id = m4->GetModelId();

    //       ---> m2
    //     /         \
    //m1 -+           +--> m4
    //     \         /
    //       ---> m3
    //

    auto modelClassId = m_db->Schemas().GetECClass("dgn", "Model")->GetId();
    auto m1key = EC::ECInstanceKey(modelClassId, EC::ECInstanceId(m1id.GetValue()));
    auto m2key = EC::ECInstanceKey(modelClassId, EC::ECInstanceId(m2id.GetValue()));
    auto m3key = EC::ECInstanceKey(modelClassId, EC::ECInstanceId(m3id.GetValue()));
    auto m4key = EC::ECInstanceKey(modelClassId, EC::ECInstanceId(m4id.GetValue()));

    EC::ECSqlStatement mrelstmt;
    EC::ECInstanceKey rkey;
    DgnDbUtilities::InsertRelationship(rkey, mrelstmt, *m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m1key, m2key);
    DgnDbUtilities::InsertRelationship(rkey, mrelstmt, *m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m1key, m3key);
    DgnDbUtilities::InsertRelationship(rkey, mrelstmt, *m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m2key, m4key);
    DgnDbUtilities::InsertRelationship(rkey, mrelstmt, *m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m3key, m4key);
    m_db->SaveChanges();

    //  Put an element in each model
    auto e3 = InsertElement("E3", m3id);
    auto e1 = InsertElement("E1", m1id);
    auto e4 = InsertElement("E4", m4id);
    auto e2 = InsertElement("E2", m2id);
    m_db->SaveChanges();

    //  Element dependencies mirror model dependencies
    //       ---> e2
    //     /         \
    //e1 -+           +--> e4
    //     \         /
    //       ---> e3
    //
    auto e1_e2 = InsertElementDrivesElementRelationship(e1, e2);
    auto e1_e3 = InsertElementDrivesElementRelationship(e1, e3);
    auto e2_e4 = InsertElementDrivesElementRelationship(e2, e4);
    auto e3_e4 = InsertElementDrivesElementRelationship(e3, e4);
    m_db->SaveChanges();

    //  drive a change from e1 through the graph
    TwiddleTime(e1);

    ABCHandler::GetHandler().Clear();
    m_db->SaveChanges();

    auto const& rels = ABCHandler::GetHandler().m_relIds;
    ASSERT_TRUE( rels.size() == 4);
    auto i1_2 = findRelId(rels, e1_e2);
    auto i1_3 = findRelId(rels, e1_e3);
    auto i2_4 = findRelId(rels, e2_e4);
    auto i3_4 = findRelId(rels, e3_e4);
    ASSERT_TRUE( i1_2 != rels.end() );
    ASSERT_TRUE( i1_3 != rels.end() );
    ASSERT_TRUE( i2_4 != rels.end() );
    ASSERT_TRUE( i3_4 != rels.end() );
    ASSERT_LT( i1_2 , i2_4 );
    ASSERT_LT( i1_3 , i2_4 );
    ASSERT_LT( i1_2 , i3_4 );
    ASSERT_LT( i1_3 , i3_4 );
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
static int64_t countModelDrivesModelInstances(DgnDb& db)
    {
    Statement modelsCount;
    modelsCount.Prepare(db, "SELECT COUNT(*) FROM " DGN_TABLE(DGN_RELNAME_ModelDrivesModel));
    modelsCount.Step();
    return modelsCount.GetValueInt64(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, ModelDependenciesWithCycleTest)
    {
    SetUpForRelationshipTests(L"ModelDependenciesWithCycleTest");

    //  Create models 1-4
    auto seedModelId = m_defaultModelId;
    auto m4 = m_db->Models().CreateNewModelFromSeed(NULL, "m4", seedModelId);
    auto m3 = m_db->Models().CreateNewModelFromSeed(NULL, "m3", seedModelId);
    auto m1 = m_db->Models().CreateNewModelFromSeed(NULL, "m1", seedModelId);
    auto m2 = m_db->Models().CreateNewModelFromSeed(NULL, "m2", seedModelId);

    auto m1id = m1->GetModelId();
    auto m2id = m2->GetModelId();
    auto m3id = m3->GetModelId();
    auto m4id = m4->GetModelId();

    // +-----------------------+
    // |     ---> m2           |
    // v   /         \         |
    //m1 -+           +--> m4--+
    //     \         /
    //       ---> m3
    //

    auto modelClassId = m_db->Schemas().GetECClass ("dgn", "PhysicalModel")->GetId();
    auto m1key = EC::ECInstanceKey (modelClassId, EC::ECInstanceId(m1id.GetValue()));
    auto m2key = EC::ECInstanceKey (modelClassId, EC::ECInstanceId(m2id.GetValue()));
    auto m3key = EC::ECInstanceKey (modelClassId, EC::ECInstanceId(m3id.GetValue()));
    auto m4key = EC::ECInstanceKey (modelClassId, EC::ECInstanceId(m4id.GetValue()));

    EC::ECSqlStatement mrelstmt;
    EC::ECInstanceKey rkey;
    DgnDbUtilities::InsertRelationship(rkey, mrelstmt, *m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m1key, m2key);
    DgnDbUtilities::InsertRelationship(rkey, mrelstmt, *m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m1key, m3key);
    DgnDbUtilities::InsertRelationship(rkey, mrelstmt, *m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m2key, m4key);
    DgnDbUtilities::InsertRelationship(rkey, mrelstmt, *m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m3key, m4key);
    m_db->SaveChanges();

    ASSERT_TRUE(!DgnDbUtilities::QueryRelationshipSourceFromTarget(*m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m1key).IsValid() ) << L"m1 is not the target of any deprel";
    ASSERT_TRUE( DgnDbUtilities::QueryRelationshipSourceFromTarget(*m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m4key).IsValid() ) << L"m4 is the target two deprels. Pick either";

    auto count = countModelDrivesModelInstances(*m_db);
    ASSERT_EQ( count, 4 );

    DgnDbUtilities::InsertRelationship(rkey, mrelstmt, *m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m4key, m1key);
    m_db->SaveChanges();

    ASSERT_TRUE(!DgnDbUtilities::QueryRelationshipSourceFromTarget(*m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m1key).IsValid() ) << L"m1 is not the target of any deprel";

    auto count2 = countModelDrivesModelInstances(*m_db);
    ASSERT_EQ( count, count2 ) << L"addition of 5th deprel should have been rolled back";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, ModelDependenciesInvalidDirectionTest)
    {
    SetUpForRelationshipTests(L"ModelDependenciesInvalidDirectionTest");

    //  Create models 1 and 2
    auto seedModelId = m_defaultModelId;
    auto m1 = m_db->Models().CreateNewModelFromSeed(NULL, "m1", seedModelId);
    auto m2 = m_db->Models().CreateNewModelFromSeed(NULL, "m2", seedModelId);

    auto m1id = m1->GetModelId();
    auto m2id = m2->GetModelId();

    // Make m2 depend on m1
    auto modelClassId = m_db->Schemas().GetECClass("dgn", "PhysicalModel")->GetId();
    auto m1key = EC::ECInstanceKey(modelClassId, EC::ECInstanceId(m1id.GetValue()));
    auto m2key = EC::ECInstanceKey(modelClassId, EC::ECInstanceId(m2id.GetValue()));

    EC::ECSqlStatement mrelstmt;
    EC::ECInstanceKey rkey;                                                                          /* source  target */
    DgnDbUtilities::InsertRelationship(rkey, mrelstmt, *m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m1key, m2key);
    m_db->SaveChanges();

    ASSERT_TRUE(!DgnDbUtilities::QueryRelationshipSourceFromTarget(*m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m1key).IsValid() ) << L"m1 is not the target of any dependency";
    ASSERT_TRUE( DgnDbUtilities::QueryRelationshipSourceFromTarget(*m_db, DGN_SCHEMA(DGN_RELNAME_ModelDrivesModel), m2key).IsValid() ) << L"m2 is the target a dependency";

    auto count = countModelDrivesModelInstances(*m_db);
    ASSERT_EQ( count, 1 );

    //  Put elements in each model
    auto e1 = InsertElement("E1", m1id);
    auto e2 = InsertElement("E2", m2id);
    auto e22 = InsertElement("E22", m2id);
    m_db->SaveChanges();

    //  Create a valid dependency
    //e1 --> e2
    auto e1_e2 = InsertElementDrivesElementRelationship(e1, e2);
    m_db->SaveChanges();

    //  drive a change from e1 through the graph to e2
    TwiddleTime(e1);

    ABCHandler::GetHandler().Clear();
    m_db->SaveChanges();
    ASSERT_EQ( ABCHandler::GetHandler().m_relIds.size(), 1 );
    ASSERT_EQ( ABCHandler::GetHandler().m_relIds.front(), e1_e2.GetECInstanceId() );

    //  Create an valid dependency
    //e1 <-- e22
    auto e22_e1 = InsertElementDrivesElementRelationship(e22, e1);

    TwiddleTime(e22);

    ABCHandler::GetHandler().Clear();
    m_db->SaveChanges();
    // Verify that the txn was rolled back. If so, my insert of e22_e1 should have been cancelled, and e22_e1 should not exist.
    EC::ECSqlSelectBuilder b;
    b.Select("COUNT(*)").From(*m_db->Schemas().GetECClass(e22_e1.GetECClassId())).Where("ECInstanceId = ?");
    EC::ECSqlStatement s;
    s.Prepare(*m_db, b.ToString().c_str());
    s.BindId(1, e22_e1.GetECInstanceId());
    ASSERT_EQ( s.Step() , EC::ECSqlStepStatus::HasRow );
    ASSERT_EQ( s.GetValueInt(0) , 0 );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, PersistentHandlerTest)
    {
    SetUpForRelationshipTests(L"PersistentHandlerTest");
    
    auto e1 = InsertElement("E1");
    auto e2 = InsertElement("E2");

    auto rel = InsertElementDrivesElementRelationship(e1, e2);
    m_db->SaveChanges();

    BeFileName theFile = m_db->GetFileName();

    CloseDb();

    // Make sure that we can reopen the file. Opening the file entails checking that all registered handlers are supplied.
    DbResult result;
    m_db = DgnDb::OpenDgnDb(&result, theFile, DgnDb::OpenParams(BeSQLite::Db::OPEN_Readonly));
    ASSERT_TRUE( m_db.IsValid() );
    ASSERT_EQ( result, BeSQLite::BE_SQLITE_OK );

    //  Make sure that the handler is still registered
    auto abcHandlerInternalId = m_db->Domains().GetClassId(ABCHandler::GetHandler());
    ASSERT_EQ( DgnElementDrivesElementDependencyHandler::GetHandler().FindHandler(*m_db, abcHandlerInternalId) , &ABCHandler::GetHandler() );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, ChangeDepTest)
    {
    SetUpForRelationshipTests(L"ChangeDepTest");

    //  Create elements in first txn
    auto e1 = InsertElement("E1");
    auto e2 = InsertElement("E2");

    m_db->SaveChanges();

    //  Create dependency relationship in a separate txn
    auto rel = InsertElementDrivesElementRelationship(e1, e2);

    ABCHandler::GetHandler().Clear();
    m_db->SaveChanges();

    // ABC should have gotten a callback, because it was created, even though neither of its targets was changed.
    ASSERT_EQ( ABCHandler::GetHandler().m_relIds.size(), 1 );

    //  Modify a property of the dependency relationship itself
    ECSqlStatement stmt;
    stmt.Prepare(*m_db, "UPDATE ONLY " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME " SET Property1='changed'  WHERE(ECInstanceId=?)");
    stmt.BindId(1, rel.GetECInstanceId());
    ASSERT_EQ( stmt.Step(), EC::ECSqlStepStatus::Done );

    // Commit this change. 
    ABCHandler::GetHandler().Clear();
    m_db->SaveChanges();

    // ABC should have gotten a callback, because it was changed, even though neither of its targets was changed.
    ASSERT_EQ( ABCHandler::GetHandler().m_relIds.size(), 1 );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestEdgeProcessor : DgnElementDependencyGraph::IEdgeProcessor
    {
    bool m_hadError;
    bvector<EC::ECInstanceId> m_relIds;

    TestEdgeProcessor() : m_hadError(false) {;}

    void Clear() {m_hadError=false; m_relIds.clear();}

    virtual void _ProcessEdge(DgnElementDependencyGraph::Edge const& edge, DgnElementDrivesElementDependencyHandler* handler) override;
    virtual void _ProcessEdgeForValidation(DgnElementDependencyGraph::Edge const& edge, DgnElementDrivesElementDependencyHandler* handler) override;
    virtual void _OnValidationError(TxnSummary::ValidationError const& error, DgnElementDependencyGraph::Edge const* edge) override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestEdgeProcessor::_ProcessEdge(DgnElementDependencyGraph::Edge const& edge, DgnElementDrivesElementDependencyHandler* handler)
    {
    m_relIds.push_back(edge.GetECRelationshipId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestEdgeProcessor::_ProcessEdgeForValidation(DgnElementDependencyGraph::Edge const& edge, DgnElementDrivesElementDependencyHandler* handler)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestEdgeProcessor::_OnValidationError(TxnSummary::ValidationError const& error, DgnElementDependencyGraph::Edge const* edge) 
    {
    m_hadError = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, WhatIfTest1)
    {
    SetUpForRelationshipTests(L"WhatIfTest");

    auto e1 = InsertElement("E1");
    auto e2 = InsertElement("E2");
    auto rel = InsertElementDrivesElementRelationship(e1, e2);

    m_db->SaveChanges();

    // Prepare the list of elements for what-if
    bvector<DgnElementId> changedEntities;
    bvector<BeSQLite::EC::ECInstanceId> changedDepRels;
    changedEntities.push_back(e1.GetElementId());

    // Check that WhatIfChanged finds this edge and invokes processor on it
    ABCHandler::GetHandler().Clear();
        {
        TestEdgeProcessor proc;
        TxnSummary summary(*m_db);
        DgnElementDependencyGraph graph(*m_db, summary);
        ASSERT_EQ( BSISUCCESS , graph.WhatIfChanged(proc, changedEntities, changedDepRels) );

        ASSERT_EQ( proc.m_hadError , false );
        ASSERT_EQ( proc.m_relIds.size() , 1 );
        ASSERT_EQ( ABCHandler::GetHandler().m_relIds.size(), 0 ) << L"Real dependency handler should not have been called";
        }

    // Repeat the test, but show that we can do WhatIfChanged without writing to anything.
    BeFileName theFile = m_db->GetFileName();
    CloseDb();
    DbResult result;
    m_db = DgnDb::OpenDgnDb(&result, theFile, DgnDb::OpenParams(BeSQLite::Db::OPEN_ReadWrite));
    ASSERT_TRUE( m_db.IsValid() );
    ASSERT_EQ( result, BeSQLite::BE_SQLITE_OK );

    ABCHandler::GetHandler().Clear();
        {
        TestEdgeProcessor proc;
        TxnSummary summary(*m_db);
        DgnElementDependencyGraph graph(*m_db, summary);
        ASSERT_EQ( BSISUCCESS , graph.WhatIfChanged(proc, changedEntities, changedDepRels) );

        ASSERT_EQ( proc.m_hadError , false );
        ASSERT_EQ( proc.m_relIds.size() , 1 );
        ASSERT_EQ( ABCHandler::GetHandler().m_relIds.size(), 0 ) << L"Real dependency handler should not have been called";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, TestPriority)
    {
    SetUpForRelationshipTests(L"TestPriority");

    auto e11 = InsertElement("E11");
    auto e12 = InsertElement("E12");
    auto e2 = InsertElement("E2");
    auto e11_e2 = InsertElementDrivesElementRelationship(e11, e2);
    auto e12_e2 = InsertElementDrivesElementRelationship(e12, e2);

    m_db->SaveChanges();

    // Prepare the list of elements for what-if
    bvector<DgnElementId> changedEntities;
    bvector<BeSQLite::EC::ECInstanceId> changedDepRels;
    changedEntities.push_back(e2.GetElementId());

    // Check that we get e11_e2, then e12_e2
    ABCHandler::GetHandler().Clear();
        {
        TestEdgeProcessor proc;
        TxnSummary summary(*m_db);
        DgnElementDependencyGraph graph(*m_db, summary);
        ASSERT_EQ( BSISUCCESS , graph.WhatIfChanged(proc, changedEntities, changedDepRels) );

        ASSERT_EQ( ABCHandler::GetHandler().m_relIds.size(), 0 ) << L"Real dependency handler should not have been called";
        ASSERT_EQ( proc.m_hadError , false );
        auto rels = proc.m_relIds;
        ASSERT_EQ( rels.size() , 2 );
        auto ie11_e2 = findRelId(rels, e11_e2);    ASSERT_TRUE( ie11_e2 != rels.end() );
        auto ie12_e2 = findRelId(rels, e12_e2);    ASSERT_TRUE( ie12_e2 != rels.end() );
        ASSERT_LT( ie11_e2, ie12_e2 ) << L"default priority should put e11_e2 first";
        }

    // Change the priority of e12_e2 to be greater. Now, it should be called first.
        {
        TxnSummary summary(*m_db);
        DgnElementDependencyGraph graph(*m_db, summary);
        DgnElementDependencyGraph::Edge edge_e12_e2 = graph.QueryEdgeByRelationshipId(e12_e2.GetECInstanceId());
        ASSERT_TRUE( edge_e12_e2.GetECRelationshipId().IsValid() );
        ASSERT_TRUE( edge_e12_e2.GetECRelationshipId() == e12_e2.GetECInstanceId() );
        int64_t priority = edge_e12_e2.GetPriority();
        ASSERT_EQ( graph.SetElementDrivesElementPriority(edge_e12_e2.GetECRelationshipId(), priority + 100), BSISUCCESS );
        }

    ABCHandler::GetHandler().Clear();
        {
        TestEdgeProcessor proc;
        TxnSummary summary(*m_db);
        DgnElementDependencyGraph graph(*m_db, summary);
        ASSERT_EQ( BSISUCCESS , graph.WhatIfChanged(proc, changedEntities, changedDepRels) );

        ASSERT_EQ( ABCHandler::GetHandler().m_relIds.size(), 0 ) << L"Real dependency handler should not have been called";
        ASSERT_EQ( proc.m_hadError , false );
        auto rels = proc.m_relIds;
        ASSERT_EQ( rels.size() , 2 );
        auto ie11_e2 = findRelId(rels, e11_e2);    ASSERT_TRUE( ie11_e2 != rels.end() );
        auto ie12_e2 = findRelId(rels, e12_e2);    ASSERT_TRUE( ie12_e2 != rels.end() );
        ASSERT_LT( ie12_e2, ie11_e2 ) << L"new priority should put e12_e2 first";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, ElementAssembly)
{
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    DgnClassId testClass(TestElement::GetTestElementECClass(*m_db)->GetId());
    DgnModelP model = m_db->Models().GetModel(m_defaultModelId);

    TestElement::CreateParams params(*model, testClass, m_defaultCategoryId);
    TestElementPtr e1 = new TestElement(params);

    DgnElementCPtr el1 = e1->Insert();
    ASSERT_TRUE(el1.IsValid());

    params.SetParentId(el1->GetElementId());     // set the parent id in the CreateParams
    TestElementPtr e2 = new TestElement(params);
    DgnElementCPtr el2 = e2->Insert();
    ASSERT_TRUE(el2.IsValid());
    ASSERT_EQ(el2->GetParentId(), el1->GetElementId());

    DgnElementCPtr el3 = e2->Insert(); // insert a second copy of the same element (that should not be a problem).
    ASSERT_TRUE(el3.IsValid());

    int count =(int) el1->QueryChildren().size(); // now make sure there are two children of el1
    ASSERT_EQ(count, 2);

    auto stat = el3->Delete();      // delete one of the children
    ASSERT_EQ(stat, DGNMODEL_STATUS_Success);
    ASSERT_TRUE(!el3->IsPersistent()); // make sure it is no longer persistent

    count =(int) el1->QueryChildren().size(); // should be down to 1 child
    ASSERT_EQ(count, 1);

    ASSERT_TRUE(el1->IsPersistent());    // both parent and child are currently persistent
    ASSERT_TRUE(el2->IsPersistent());

    stat = el1->Delete();               // delete the parent, should also delete child.
    ASSERT_EQ(stat, DGNMODEL_STATUS_Success);

    ASSERT_TRUE(!el1->IsPersistent());  // neither should now be persistent
    ASSERT_TRUE(!el2->IsPersistent());
}
