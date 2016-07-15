/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ElementDependencyGraph.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "../TestFixture/GenericDgnModelTestFixture.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnElementHelpers.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnDbUtilities.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnElementDependency.h>
#include "../TestFixture/DgnDbTestFixtures.h"

#define GROUP_SUBDIR L"ElementDependencyGraph"
#define GROUP_SEED_FILENAME GROUP_SUBDIR L"/Test.bim"
#define DEFAULT_MODEL_NAME "Default"
#define DEFAULT_CATEGORY_NAME "DefaultCat"
#define DEFAULT_VIEW_NAME "DefaultView"

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
    bset<BeInt64Id> m_adds, m_deletes, m_mods;

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
struct ElementDependencyGraph : DgnDbTestFixture
{
    BETEST_DECLARE_TC_TEARDOWN

    enum class ElementDrivesElementColumn {TargetECInstanceId,TargetECClassId,SourceECInstanceId,SourceECClassId,Status};

    struct ElementsAndRelationships
        {
        DgnElementCPtr e99, e3, e31, e2, e1;
        ECInstanceKey r99_3, r99_31, r3_2, r31_2, r2_1;
        };

    ElementDependencyGraph();
    ~ElementDependencyGraph();
    void CloseDb() {m_db->CloseDb();}
    DgnModelR GetDefaultModel() {return *m_db->Models().GetModel(m_defaultModelId);}
    DgnElementCPtr InsertElement(Utf8CP elementCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId());
    void TwiddleTime(DgnElementCPtr);
    
    WString GetTestFileName(WCharCP testname);
    ECN::ECClassCR GetElementDrivesElementClass();

    CachedECSqlStatementPtr GetSelectElementDrivesElementById();
    void SetUpForRelationshipTests(WCharCP testname);
    ECInstanceKey InsertElementDrivesElementRelationship(DgnElementCPtr root, DgnElementCPtr dependent);

    void TestTPS(DgnElementCPtr e1, DgnElementCPtr e2, size_t ntimes);
    void TestOverlappingOrder(DgnElementCPtr r1, ECInstanceKeyCR r1_d3, ECInstanceKeyCR r2_d3, bool r1First);
    void TestRelationships(DgnDb& db, ElementsAndRelationships const&);
};

END_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
BETEST_TC_TEARDOWN(ElementDependencyGraph)
    {
    // Note: leave your subdirectory in place. Don't remove it. That allows the 
    // base class to detect and throw an error if two groups try to use a directory of the same name.
    // Don't worry about stale data. The test runner will clean out everything at the start of the program.
    // You can empty the directory, if you want to save space.
    DgnDbTestUtils::EmptySubDirectory(GROUP_SUBDIR);
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
ElementDependencyGraph::ElementDependencyGraph()
    {
    // Must register my domain whenever I initialize a host
    DgnPlatformTestDomain::Register();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementDependencyGraph::~ElementDependencyGraph()
    {
    if (m_db.IsValid())
        m_db->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ElementDependencyGraph::InsertElement(Utf8CP elementCode, DgnModelId mid, DgnCategoryId categoryId )
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
void ElementDependencyGraph::TwiddleTime(DgnElementCPtr el)
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
    return WPrintfString(L"%ls.ibim",testname);
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
    Utf8String ecsql("SELECT TargetECInstanceId,TargetECClassId,SourceECInstanceId,SourceECClassId,Status FROM ONLY ");
    ecsql.append(GetElementDrivesElementClass().GetECSqlName()).append(" WHERE ECInstanceId=?");
    return m_db->GetPreparedECSqlStatement(ecsql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementDependencyGraph::SetUpForRelationshipTests(WCharCP testname)
    {
    SetupProject(GetTestFileName(testname).c_str(), Db::OpenMode::ReadWrite);
    ASSERT_TRUE(m_db->IsBriefcase());
    ASSERT_TRUE(m_db->Txns().IsTracking());
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
TEST_F(ElementDependencyGraph, DeleteSource)
    {
    SetUpForRelationshipTests(L"DeleteSource");

    TestElementDrivesElementHandler::GetHandler().Clear();

    auto e1 = InsertElement("E1");
    auto e2 = InsertElement("E2");
    auto r1  = InsertElementDrivesElementRelationship(e1, e2);
    ASSERT_TRUE(e1.IsValid() && e2.IsValid() && r1.IsValid());
    TestElementDrivesElementHandler::GetHandler().Clear();
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges());

    ASSERT_EQ(1, TestElementDrivesElementHandler::GetHandler().m_relIds.size());
    
    TestElementDrivesElementHandler::GetHandler().Clear();

    ASSERT_EQ(DgnDbStatus::Success, e1->Delete());
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges());

    ASSERT_EQ(0, TestElementDrivesElementHandler::GetHandler().m_relIds.size());
    ASSERT_EQ(1, TestElementDrivesElementHandler::GetHandler().m_deletedRels.size());
    ASSERT_EQ(r1, TestElementDrivesElementHandler::GetHandler().m_deletedRels[0].m_relKey);
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

        TestElementDrivesElementHandler::GetHandler().Clear();
        m_db->SaveChanges();

        auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;

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

        TestElementDrivesElementHandler::GetHandler().Clear();
        m_db->SaveChanges();

        auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;

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
void ElementDependencyGraph::TestOverlappingOrder(DgnElementCPtr r1, ECInstanceKeyCR r1_d3, ECInstanceKeyCR r2_d3, bool r1First)
    {
    m_db->SaveChanges();

    TwiddleTime(r1);

    TestElementDrivesElementHandler::GetHandler().Clear();
    m_db->SaveChanges();

    auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;
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

    DgnElementCPtr e1 = InsertElement("E1");
    DgnElementCPtr e2 = InsertElement("E2");
    ECInstanceKey e1_e2 = InsertElementDrivesElementRelationship(e1, e2);

    CachedECSqlStatementPtr selectDepRel = GetSelectElementDrivesElementById();
    selectDepRel->BindId(1, e1_e2.GetECInstanceId());

    m_db->SaveChanges();

    ASSERT_EQ( selectDepRel->Step(), BE_SQLITE_ROW );
    ASSERT_EQ( selectDepRel->GetValueInt((int)ElementDrivesElementColumn::Status),(int)DgnElementDependencyGraph::EdgeStatus::EDGESTATUS_Satisfied );

    TestElementDrivesElementHandlerShouldFail fail;
    TwiddleTime(e1);
    m_db->SaveChanges();

    selectDepRel->Reset();
    ASSERT_EQ( selectDepRel->Step(), BE_SQLITE_ROW );
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

        TestElementDrivesElementHandler::GetHandler().Clear();
        m_db->SaveChanges();

// It's still undecided as to whether handlers should be called or not.
//        auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;
//        ASSERT_EQ( rels.size() , 0) << L" my dependency handler should not have been called, because of the graph-building error";


        // Verify that the txn was rolled back. If so, my insert of e2_e1 should have been cancelled, and e2_e1 should not exist.
        Utf8String ecsql("SELECT * FROM ");
        ecsql.append(m_db->Schemas().GetECClass(e2_e1.GetECClassId())->GetECSqlName()).append(" WHERE ECInstanceId=?");
        ECSqlStatement s;
        s.Prepare(*m_db, ecsql.c_str());
        s.BindId(1, e2_e1.GetECInstanceId());
        ASSERT_EQ( s.Step() , BE_SQLITE_DONE );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, CycleTest2)
    {
    SetUpForRelationshipTests(L"CycleTest1");

    //  Two Elements
    DgnElementCPtr e1 = InsertElement("E1");
    DgnElementCPtr e2 = InsertElement("E2");
    DgnElementCPtr e3 = InsertElement("E3");
    DgnElementCPtr e4 = InsertElement("E4");

    //  Forward dependency relationship
    InsertElementDrivesElementRelationship(e1, e2);
    InsertElementDrivesElementRelationship(e2, e3);
    InsertElementDrivesElementRelationship(e3, e4);

    m_db->SaveChanges();

    if (true)
        {
        // Attempt to create backward relationship, which would cause a cycle.
        ECInstanceKey e4_e2 = InsertElementDrivesElementRelationship(e4, e2);

        // Trigger graph evaluation, which will detect the cycle.
        TwiddleTime(e1);

        TestElementDrivesElementHandler::GetHandler().Clear();
        m_db->SaveChanges();

        // Verify that the txn was rolled back. If so, my insert of e2_e1 should have been cancelled, and e2_e1 should not exist.
        CachedECSqlStatementPtr getRelDep = GetSelectElementDrivesElementById();
        getRelDep->BindId(1, e4_e2.GetECInstanceId());
        ASSERT_EQ( getRelDep->Step() , BE_SQLITE_DONE );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, ModelDependenciesTest)
    {
    SetUpForRelationshipTests(L"ModelDependenciesTest");

    //  Create models 1-4
    auto seedModelId = m_defaultModelId;

    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId);
    auto m4 = seedModel->Clone(DgnModel::CreateModelCode("m4"));
    auto m3 = seedModel->Clone(DgnModel::CreateModelCode("m3"));
    auto m1 = seedModel->Clone(DgnModel::CreateModelCode("m1"));
    auto m2 = seedModel->Clone(DgnModel::CreateModelCode("m2"));
    m4->Insert();
    m3->Insert();
    m1->Insert();
    m2->Insert();

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

    auto modelClassId = m_db->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Model)->GetId();
    auto m1key = ECInstanceKey(modelClassId, ECInstanceId(m1id.GetValue()));
    auto m2key = ECInstanceKey(modelClassId, ECInstanceId(m2id.GetValue()));
    auto m3key = ECInstanceKey(modelClassId, ECInstanceId(m3id.GetValue()));
    auto m4key = ECInstanceKey(modelClassId, ECInstanceId(m4id.GetValue()));

    ECSqlStatement mrelstmt;
    ECInstanceKey rkey;
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

    TestElementDrivesElementHandler::GetHandler().Clear();
    m_db->SaveChanges();

    auto const& rels = TestElementDrivesElementHandler::GetHandler().m_relIds;
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
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId);
    auto m4 = seedModel->Clone(DgnModel::CreateModelCode("m4"));
    auto m3 = seedModel->Clone(DgnModel::CreateModelCode("m3"));
    auto m1 = seedModel->Clone(DgnModel::CreateModelCode("m1"));
    auto m2 = seedModel->Clone(DgnModel::CreateModelCode("m2"));
    m4->Insert();
    m3->Insert();
    m1->Insert();
    m2->Insert();

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

    auto modelClassId = m_db->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel)->GetId();
    auto m1key = ECInstanceKey(modelClassId, ECInstanceId(m1id.GetValue()));
    auto m2key = ECInstanceKey(modelClassId, ECInstanceId(m2id.GetValue()));
    auto m3key = ECInstanceKey(modelClassId, ECInstanceId(m3id.GetValue()));
    auto m4key = ECInstanceKey(modelClassId, ECInstanceId(m4id.GetValue()));

    ECSqlStatement mrelstmt;
    ECInstanceKey rkey;
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
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId);
    auto m1 = seedModel->Clone(DgnModel::CreateModelCode("m1"));
    auto m2 = seedModel->Clone(DgnModel::CreateModelCode("m2"));
    m1->Insert();
    m2->Insert();

    auto m1id = m1->GetModelId();
    auto m2id = m2->GetModelId();

    // Make m2 depend on m1
    auto modelClassId = m_db->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel)->GetId();
    auto m1key = ECInstanceKey(modelClassId, ECInstanceId(m1id.GetValue()));
    auto m2key = ECInstanceKey(modelClassId, ECInstanceId(m2id.GetValue()));

    ECSqlStatement mrelstmt;
    ECInstanceKey rkey;                                                                          /* source  target */
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

    TestElementDrivesElementHandler::GetHandler().Clear();
    m_db->SaveChanges();
    ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1 );
    ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.front(), e1_e2.GetECInstanceId() );

    //  Create an valid dependency
    //e1 <-- e22
    auto e22_e1 = InsertElementDrivesElementRelationship(e22, e1);

    TwiddleTime(e22);

    TestElementDrivesElementHandler::GetHandler().Clear();
    m_db->SaveChanges();
    // Verify that the txn was rolled back. If so, my insert of e22_e1 should have been cancelled, and e22_e1 should not exist.
    Utf8String ecsql("SELECT COUNT(*) FROM ");
    ecsql.append(m_db->Schemas().GetECClass(e22_e1.GetECClassId())->GetECSqlName()).append(" WHERE ECInstanceId = ?");
    ECSqlStatement s;
    s.Prepare(*m_db, ecsql.c_str());
    s.BindId(1, e22_e1.GetECInstanceId());
    ASSERT_EQ( s.Step() , BE_SQLITE_ROW );
    ASSERT_EQ( s.GetValueInt(0) , 0 );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, PersistentHandlerTest)
    {
    SetUpForRelationshipTests(L"PersistentHandlerTest");

    {
    auto e1 = InsertElement("E1");
    auto e2 = InsertElement("E2");

    auto rel = InsertElementDrivesElementRelationship(e1, e2);
    m_db->SaveChanges();
    }

    BeFileName theFile = m_db->GetFileName();

    CloseDb();

    // Make sure that we can reopen the file. Opening the file entails checking that all registered handlers are supplied.
    DbResult result;
    m_db = DgnDb::OpenDgnDb(&result, theFile, DgnDb::OpenParams(Db::OpenMode::Readonly));
    ASSERT_TRUE( m_db.IsValid() );
    ASSERT_EQ( result, BE_SQLITE_OK );

    //  Make sure that the handler is still registered
    auto abcHandlerInternalId = m_db->Domains().GetClassId(TestElementDrivesElementHandler::GetHandler());
    ASSERT_EQ( DgnElementDependencyHandler::GetHandler().FindHandler(*m_db, abcHandlerInternalId) , &TestElementDrivesElementHandler::GetHandler() );
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

    TestElementDrivesElementHandler::GetHandler().Clear();
    m_db->SaveChanges();

    // TestElementDrivesElement should have gotten a callback, because it was created, even though neither of its targets was changed.
    ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1 );

    //  Modify a property of the dependency relationship itself
    TestElementDrivesElementHandler::UpdateProperty1(*m_db, rel);

    // Commit this change.
    TestElementDrivesElementHandler::GetHandler().Clear();
    m_db->SaveChanges();

    // TestElementDrivesElement should have gotten a callback, because it was changed, even though neither of its targets was changed.
    ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1 );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestEdgeProcessor : DgnElementDependencyGraph::IEdgeProcessor
    {
    bool m_hadError;
    bvector<ECInstanceId> m_relIds;

    TestEdgeProcessor() : m_hadError(false) {;}

    void Clear() {m_hadError=false; m_relIds.clear();}

    virtual void _ProcessEdge(DgnElementDependencyGraph::Edge const& edge, DgnElementDependencyHandler* handler) override;
    virtual void _ProcessEdgeForValidation(DgnElementDependencyGraph::Edge const& edge, DgnElementDependencyHandler* handler) override;
    virtual void _OnValidationError(TxnManager::ValidationError const& error, DgnElementDependencyGraph::Edge const* edge) override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestEdgeProcessor::_ProcessEdge(DgnElementDependencyGraph::Edge const& edge, DgnElementDependencyHandler* handler)
    {
    m_relIds.push_back(edge.GetECRelationshipId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestEdgeProcessor::_ProcessEdgeForValidation(DgnElementDependencyGraph::Edge const& edge, DgnElementDependencyHandler* handler)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestEdgeProcessor::_OnValidationError(TxnManager::ValidationError const& error, DgnElementDependencyGraph::Edge const* edge)
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
    bvector<ECInstanceId> changedDepRels;
    changedEntities.push_back(e1->GetElementId());

    e1=nullptr; // can't keep these after we close the file.
    e2=nullptr;

    // Check that WhatIfChanged finds this edge and invokes processor on it
    TestElementDrivesElementHandler::GetHandler().Clear();
        {
        TestEdgeProcessor proc;
        DgnElementDependencyGraph graph(m_db->Txns());
        ASSERT_EQ( BSISUCCESS , graph.WhatIfChanged(proc, changedEntities, changedDepRels) );

        ASSERT_EQ( proc.m_hadError , false );
        ASSERT_EQ( proc.m_relIds.size() , 1 );
        ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 0 ) << L"Real dependency handler should not have been called";
        }
    // Repeat the test, but show that we can do WhatIfChanged without writing to anything.
    BeFileName theFile = m_db->GetFileName();
    CloseDb();
    DbResult result;
    m_db = DgnDb::OpenDgnDb(&result, theFile, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE( m_db.IsValid() );
    ASSERT_EQ( result, BE_SQLITE_OK );

    TestElementDrivesElementHandler::GetHandler().Clear();
        {
        TestEdgeProcessor proc;
        DgnElementDependencyGraph graph(m_db->Txns());
        ASSERT_EQ( BSISUCCESS , graph.WhatIfChanged(proc, changedEntities, changedDepRels) );

        ASSERT_EQ( proc.m_hadError , false );
        ASSERT_EQ( proc.m_relIds.size() , 1 );
        ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 0 ) << L"Real dependency handler should not have been called";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDependencyGraph, TestPriority)
    {
    SetUpForRelationshipTests(L"TestPriority");

    DgnElementCPtr e11 = InsertElement("E11");
    ASSERT_TRUE(e11 != nullptr);
    DgnElementCPtr e12 = InsertElement("E12");
    ASSERT_TRUE(e12 != nullptr);
    DgnElementCPtr e2 = InsertElement("E2");
    ASSERT_TRUE(e2 != nullptr);
    ECInstanceKey e11_e2 = InsertElementDrivesElementRelationship(e11, e2);
    ASSERT_TRUE(e11_e2.IsValid());
    ECInstanceKey e12_e2 = InsertElementDrivesElementRelationship(e12, e2);
    ASSERT_TRUE(e12_e2.IsValid());

    m_db->SaveChanges();

    // Prepare the list of elements for what-if
    bvector<DgnElementId> changedEntities;
    bvector<ECInstanceId> changedDepRels;
    changedEntities.push_back(e2->GetElementId());

    // Check that we get e11_e2, then e12_e2
    TestElementDrivesElementHandler::GetHandler().Clear();
        {
        TestEdgeProcessor proc;
        DgnElementDependencyGraph graph(m_db->Txns());
        ASSERT_EQ( BSISUCCESS , graph.WhatIfChanged(proc, changedEntities, changedDepRels) );

        ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 0 ) << L"Real dependency handler should not have been called";
        ASSERT_EQ( proc.m_hadError , false );
        auto rels = proc.m_relIds;
        ASSERT_EQ( rels.size() , 2 );
        auto ie11_e2 = findRelId(rels, e11_e2);    ASSERT_TRUE( ie11_e2 != rels.end() );
        auto ie12_e2 = findRelId(rels, e12_e2);    ASSERT_TRUE( ie12_e2 != rels.end() );
        ASSERT_LT( ie11_e2, ie12_e2 ) << L"default priority should put e11_e2 first";
        }

    // Change the priority of e12_e2 to be greater. Now, it should be called first.
        {
        DgnElementDependencyGraph graph(m_db->Txns());
        DgnElementDependencyGraph::Edge edge_e12_e2 = graph.QueryEdgeByRelationshipId(e12_e2.GetECInstanceId());
        ASSERT_TRUE( edge_e12_e2.GetECRelationshipId().IsValid() );
        ASSERT_TRUE( edge_e12_e2.GetECRelationshipId() == e12_e2.GetECInstanceId() );
        int64_t priority = edge_e12_e2.GetPriority();
        ASSERT_EQ( graph.SetElementDrivesElementPriority(edge_e12_e2.GetECRelationshipId(), priority + 100), BSISUCCESS );
        }

    TestElementDrivesElementHandler::GetHandler().Clear();
        {
        TestEdgeProcessor proc;
        DgnElementDependencyGraph graph(m_db->Txns());
        ASSERT_EQ( BSISUCCESS , graph.WhatIfChanged(proc, changedEntities, changedDepRels) );

        ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 0 ) << L"Real dependency handler should not have been called";
        ASSERT_EQ( proc.m_hadError , false );
        auto rels = proc.m_relIds;
        ASSERT_EQ( rels.size() , 2 );
        auto ie11_e2 = findRelId(rels, e11_e2);    ASSERT_TRUE( ie11_e2 != rels.end() );
        auto ie12_e2 = findRelId(rels, e12_e2);    ASSERT_TRUE( ie12_e2 != rels.end() );
        ASSERT_LT( ie12_e2, ie11_e2 ) << L"new priority should put e12_e2 first";
        }
    }
