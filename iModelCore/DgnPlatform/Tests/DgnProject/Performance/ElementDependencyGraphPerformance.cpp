/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/ElementDependencyGraphPerformance.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include "../TestFixture/GenericDgnModelTestFixture.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnElementHelpers.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnDbUtilities.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECSqlBuilder.h>
#include <DgnPlatform/DgnCore/DgnElementDependency.h>

#define LOCALIZED_STR(str) str
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_DGNDB_UNIT_TESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

BEGIN_UNNAMED_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct TxnMonitorVerifier : TxnMonitor
    {
    bool m_OnTxnClosedCalled;
    bool m_OnTxnReversedCalled;
    bset<ECInstanceId> m_adds, m_deletes, m_mods;

    TxnMonitorVerifier();
    ~TxnMonitorVerifier();
    void Clear();
    void _OnCommit(TxnManager&) override;
    void _OnReversedChanges(TxnManager&) override {m_OnTxnReversedCalled = true;}
    };

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct TestElementDrivesElementHandlerShouldFail
    {
    TestElementDrivesElementHandlerShouldFail() {TestElementDrivesElementHandler::SetShouldFail(true);}
    ~TestElementDrivesElementHandlerShouldFail() {TestElementDrivesElementHandler::SetShouldFail(false);}
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
    void SetupProject(WCharCP projFile, WCharCP testFile, Db::OpenMode mode);
    DgnElementCPtr InsertElement(Utf8CP elementCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId());
    void TwiddleTime(DgnElementCPtr);
};

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct ElementDependencyGraph : TransactionManagerTests
{
    enum class ElementDrivesElementColumn {DependentElementId,DependentElementClassId,RootElementId,RootElementClassId,Status};

    struct ElementsAndRelationships
        {
        DgnElementCPtr e99, e3, e31, e2, e1;
        ECInstanceKey r99_3, r99_31, r3_2, r31_2, r2_1;
        };

    WString GetTestFileName(WCharCP testname);
    ECN::ECClassCR GetElementDrivesElementClass();

    CachedECSqlStatementPtr GetSelectElementDrivesElementById();
    void SetUpForRelationshipTests(WCharCP testname);
    ECInstanceKey InsertElementDrivesElementRelationship(DgnElementCPtr root, DgnElementCPtr dependent);

    void TestTPS(DgnElementCPtr e1, DgnElementCPtr e2, size_t ntimes);
    void TestOverlappingOrder(DgnElementCPtr r1, ECInstanceKeyCR r1_d3, ECInstanceKeyCR r2_d3, bool r1First);
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
    m_OnTxnClosedCalled = m_OnTxnReversedCalled = false;
    m_adds.clear(); m_deletes.clear(); m_mods.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnMonitorVerifier::_OnCommit(TxnManager& txnMgr)
    {
    m_OnTxnClosedCalled = true;

    for (auto it : txnMgr.Elements().MakeIterator())
        {
        DgnElementId eid =  it.GetElementId();
        switch (it.GetChangeType())
            {
            case TxnTable::ChangeType::Insert: m_adds.insert(eid); break;
            case TxnTable::ChangeType::Delete: m_deletes.insert(eid); break;
            case TxnTable::ChangeType::Update: m_mods.insert(eid); break;
            default:
                FAIL();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static bvector<ECInstanceId>::const_iterator findRelId(bvector<ECInstanceId> const& rels, ECInstanceKey eid)
    {
    return std::find(rels.begin(), rels.end(), eid.GetECInstanceId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TransactionManagerTests::TransactionManagerTests()
    {
    // Must register my domain whenever I initialize a host
    DgnPlatformTestDomain::Register();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TransactionManagerTests::~TransactionManagerTests()
    {
    if (m_db.IsValid())
        m_db->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .dgndb project file after copying it to out
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TransactionManagerTests::SetupProject(WCharCP projFile, WCharCP testFile, Db::OpenMode mode)
    {
    BeFileName outFileName;
    ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, projFile, testFile, __FILE__));
    DbResult result;
    m_db = DgnDb::OpenDgnDb(&result, outFileName, DgnDb::OpenParams(mode));
    ASSERT_TRUE(m_db.IsValid());
    ASSERT_TRUE( result == BE_SQLITE_OK);

    ASSERT_EQ( DgnDbStatus::Success , DgnPlatformTestDomain::ImportSchema(*m_db) );

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    DgnModelPtr defaultModel = m_db->Models().GetModel(m_defaultModelId);
    ASSERT_TRUE(defaultModel.IsValid());
    GetDefaultModel().FillModel();

    m_defaultCategoryId = DgnCategory::QueryFirstCategoryId(*m_db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr TransactionManagerTests::InsertElement(Utf8CP elementCode, DgnModelId mid, DgnCategoryId categoryId )
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement::Create(*m_db, mid, categoryId, elementCode);
    return m_db->Elements().Insert(*el);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TransactionManagerTests::TwiddleTime(DgnElementCPtr el)
    {
    BeThreadUtilities::BeSleep(1); // make sure the new timestamp is after the one that's on the Element now
    DgnElementPtr mod = el->CopyForEdit();
    mod->Update();
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
    return TestElementDrivesElementHandler::GetECClass(*m_db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr ElementDependencyGraph::GetSelectElementDrivesElementById()
    {
    ECSqlSelectBuilder b;
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
    SetupProject(L"3dMetricGeneral.idgndb", GetTestFileName(testname).c_str(), Db::OpenMode::ReadWrite);

    m_db->Txns().EnableTracking(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ElementDependencyGraph::InsertElementDrivesElementRelationship(DgnElementCPtr root, DgnElementCPtr dependent)
    {
    return TestElementDrivesElementHandler::Insert(*m_db, root->GetElementId(), dependent->GetElementId());
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
    TestElementDrivesElementHandler::GetHandler().Clear();
    db.SaveChanges();
    if (true)
        {
        auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;
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
    TestElementDrivesElementHandler::GetHandler().Clear();
    db.SaveChanges();   // ==> Triggers callbacks to TestElementDrivesElementHandler::GetHandler()
    if (true)
        {
        auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;
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
    TestElementDrivesElementHandler::GetHandler().Clear();
    db.SaveChanges();   // ==> Triggers callbacks to TestElementDrivesElementHandler::GetHandler()
    if (true)
        {
        auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;
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
    TestElementDrivesElementHandler::GetHandler().Clear();
    db.SaveChanges();   // ==> Triggers callbacks to TestElementDrivesElementHandler::GetHandler()
    if (true)
        {
        auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;
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
    TestElementDrivesElementHandler::GetHandler().Clear();
    db.SaveChanges();   // ==> Triggers callbacks to TestElementDrivesElementHandler::GetHandler()
    if (true)
        {
        auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;
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
    TestElementDrivesElementHandler::GetHandler().Clear();
    db.SaveChanges();   // ==> Triggers callbacks to TestElementDrivesElementHandler::GetHandler()
    if (true)
        {
        auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;
        ASSERT_EQ( rels.size() , 1);
        auto i2_1   = findRelId(rels, g.r2_1);      ASSERT_NE(i2_1  , rels.end());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Performance_ElementDependencyGraph, PerformanceDeep1)
    {
    SetUpForRelationshipTests(L"PerformanceDeep1");

    static size_t s_nElements = 512;

    //  Create 10000 Elements, and make each depend on the previous one.
    ECInstanceKey firstRel,previousRel,thisRel,lastRel;
    DgnElementCPtr firstElement, previousElement, thisElement, lastElement;

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
        TestElementDrivesElementHandler::GetHandler().Clear();
        m_db->SaveChanges();
        timer.Stop();
        BeTest::Log("ElementDependencyGraph", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("Mod 1st: %lf seconds", timer.GetElapsedSeconds()));
        auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;
        ASSERT_EQ( rels.size()  , s_nElements-1);
        ASSERT_EQ( rels.front() , firstRel.GetECInstanceId() );
        ASSERT_EQ( rels.back()  , lastRel.GetECInstanceId() );
        }

    // Modify the last Element => triggers last handler
    if (true)
        {
        TwiddleTime(lastElement);
        StopWatch timer("Mod last", true);
        TestElementDrivesElementHandler::GetHandler().Clear();
        m_db->SaveChanges();
        timer.Stop();
        BeTest::Log("ElementDependencyGraph", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("Mod last: %lf seconds", timer.GetElapsedSeconds()));
        auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;
        ASSERT_EQ( rels.size() , 1);
        ASSERT_EQ( rels.front(), lastRel.GetECInstanceId() );
        }

    // Modify the next-to-last Element => triggers 2 handlers, the last one, and the previous
    if (true)
        {
        TwiddleTime(previousElement);
        StopWatch timer("Mod next to last", true);
        TestElementDrivesElementHandler::GetHandler().Clear();
        m_db->SaveChanges();
        timer.Stop();
        BeTest::Log("ElementDependencyGraph", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("Mod next to last: %lf seconds", timer.GetElapsedSeconds()));
        auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;
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
    DgnElementCPtr rootElement = InsertElement("Root");

    //  Create a bunch of Elements, and make each depend on the single rootElement
    ECInstanceKey firstRel, lastRel;
    DgnElementCPtr firstDependentElement, lastDependentElement;
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
        TestElementDrivesElementHandler::GetHandler().Clear();
        m_db->SaveChanges();
        timer.Stop();
        BeTest::Log("ElementDependencyGraph", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("Mod Root: %lf seconds", timer.GetElapsedSeconds()));
        auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;
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
        TestElementDrivesElementHandler::GetHandler().Clear();
        m_db->SaveChanges();
        timer.Stop();
        BeTest::Log("ElementDependencyGraph", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("Mod dependents: %lf seconds", timer.GetElapsedSeconds()));
        auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;
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
void ElementDependencyGraph::TestTPS(DgnElementCPtr e1, DgnElementCPtr e2, size_t ntimes)
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
        TestElementDrivesElementHandler::GetHandler().Clear();
        for (size_t i=0; i<ntimes; ++i)
            {
            TwiddleTime(e1);
            m_db->SaveChanges();
            }
        timer.Stop();
        printf("change e1, propagate to e2: %lf\n", ntimes/timer.GetElapsedSeconds());
        ASSERT_EQ( ntimes, TestElementDrivesElementHandler::GetHandler().m_relIds.size() );
        }

    if (true)
        {
        //  ------------------------------------------------
        StopWatch timer("change e2", true);
        TestElementDrivesElementHandler::GetHandler().Clear();
        for (size_t i=0; i<ntimes; ++i)
            {
            TwiddleTime(e2);
            m_db->SaveChanges();
            }
        timer.Stop();
        printf("change e12: %lf\n", ntimes/timer.GetElapsedSeconds());
        ASSERT_EQ( ntimes, TestElementDrivesElementHandler::GetHandler().m_relIds.size() );
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
