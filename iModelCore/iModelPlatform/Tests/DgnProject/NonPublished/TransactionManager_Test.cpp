/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/PlatformLib.h>
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/WebMercator.h>
#include <DgnPlatform/DgnTexture.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_RENDER

BEGIN_UNNAMED_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TxnMonitorVerifier : TxnMonitor
    {
    bool m_OnTxnClosedCalled;
    bool m_OnTxnAppliedCalled;
    bset<BeInt64Id> m_adds, m_deletes, m_mods;

    TxnMonitorVerifier();
    ~TxnMonitorVerifier();
    void Clear();
    void _OnCommit(TxnManager&) override;
    void _OnAppliedChanges(TxnManager&) override {m_OnTxnAppliedCalled = true;}

    bool IsEmpty() const { return !m_OnTxnClosedCalled && !m_OnTxnAppliedCalled && !HasInstances(); }
    bool HasInstances() const { return m_adds.size() + m_deletes.size() + m_mods.size() > 0; }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TransactionManagerTests : public DgnDbTestFixture
{
    DEFINE_T_SUPER(DgnDbTestFixture);
public:
    TransactionManagerTests() {}
    ~TransactionManagerTests();
    void TwiddleTime(DgnElementCPtr);
};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .bim project file after copying it to out
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isElementIdInKeySet(bset<BeInt64Id> const& theSet, DgnElementId element)
    {
    return theSet.find(element) != theSet.end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnMonitorVerifier::TxnMonitorVerifier()
    {
    TxnManager::AddTxnMonitor(*this);
    Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TxnMonitorVerifier::~TxnMonitorVerifier()
    {
    TxnManager::DropTxnMonitor(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnMonitorVerifier::Clear()
    {
    m_OnTxnClosedCalled = m_OnTxnAppliedCalled = false;
    m_adds.clear(); m_deletes.clear(); m_mods.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TransactionManagerTests::~TransactionManagerTests()
    {
    SaveDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TransactionManagerTests::TwiddleTime(DgnElementCPtr el)
    {
    BeThreadUtilities::BeSleep(1); // make sure the new timestamp is after the one that's on the Element now
    DgnElementPtr mod = el->CopyForEdit();
    mod->Update();
    }

/*---------------------------------------------------------------------------------**//**
* Test of Element CRUD
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, CRUD)
    {
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*=needBriefcase*/);

    m_db->SaveChanges();
    TxnMonitorVerifier monitor;

    //  -------------------------------------------------------------
    //  Test adds
    //  -------------------------------------------------------------
    auto key1 = InsertElement("E1");
    ASSERT_TRUE( key1->GetElementId().IsValid() );

    auto key2 = InsertElement("E2");
    ASSERT_TRUE( key2->GetElementId().IsValid() );

    m_db->SaveChanges();

    EXPECT_TRUE(monitor.m_OnTxnClosedCalled);
    ASSERT_EQ(monitor.m_adds.size()     , 2);
    ASSERT_EQ(monitor.m_deletes.size()  , 0);
    ASSERT_EQ(monitor.m_mods.size()     , 0);
    ASSERT_TRUE(isElementIdInKeySet(monitor.m_adds, key1->GetElementId()) );
    ASSERT_TRUE(isElementIdInKeySet(monitor.m_adds, key2->GetElementId()) );

    monitor.Clear();

    //  -------------------------------------------------------------
    //  Test mods
    //  -------------------------------------------------------------
    TwiddleTime(key1.get());
    m_db->SaveChanges();

    EXPECT_TRUE(monitor.m_OnTxnClosedCalled);
    ASSERT_EQ(monitor.m_adds.size()     , 0);
    ASSERT_EQ(monitor.m_deletes.size()  , 0);
    ASSERT_EQ(monitor.m_mods.size()     , 1);
    ASSERT_TRUE(isElementIdInKeySet(monitor.m_mods, key1->GetElementId()) );

    monitor.Clear();

    //  -------------------------------------------------------------
    //  Test deletes
    //  -------------------------------------------------------------
    auto delStatus = key2->Delete();
    ASSERT_TRUE( DgnDbStatus::Success == delStatus );

    m_db->SaveChanges();

    EXPECT_TRUE(monitor.m_OnTxnClosedCalled);
    ASSERT_EQ(monitor.m_adds.size()     , 0);
    ASSERT_EQ(monitor.m_deletes.size()  , 1);
    ASSERT_EQ(monitor.m_mods.size()     , 0);
    ASSERT_TRUE(isElementIdInKeySet(monitor.m_deletes, key2->GetElementId()) );

    monitor.Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, ElementAssembly)
    {
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*=needBriefcase*/);
    DgnClassId parentRelClassId = m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);

    TestElementPtr e1 = TestElement::Create(*m_db, m_defaultModelId,m_defaultCategoryId);

    DgnElementCPtr el1 = e1->Insert();
    ASSERT_TRUE(el1.IsValid());

    TestElementPtr e2 = TestElement::Create(*m_db, m_defaultModelId,m_defaultCategoryId);
    e2->SetParentId(el1->GetElementId(), parentRelClassId);
    DgnElementCPtr el2 = e2->Insert();
    ASSERT_TRUE(el2.IsValid());
    ASSERT_EQ(el2->GetParentId(), el1->GetElementId());

    e2 = TestElement::Create(*m_db, m_defaultModelId,m_defaultCategoryId);
    e2->SetParentId(el1->GetElementId(), parentRelClassId);
    DgnElementCPtr el3 = e2->Insert(); // insert another element
    ASSERT_TRUE(el3.IsValid());

    int count =(int) el1->QueryChildren().size(); // now make sure there are two children of el1
    ASSERT_EQ(count, 2);

    auto stat = el3->Delete();      // delete one of the children
    ASSERT_EQ(stat, DgnDbStatus::Success);

    count =(int) el1->QueryChildren().size(); // should be down to 1 child
    ASSERT_EQ(count, 1);

    stat = el1->Delete();               // delete the parent, should also delete child.
    ASSERT_EQ(stat, DgnDbStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void testModelUndoRedo(DgnDbR db)
    {
    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(db, "TestPhysical");
    DgnCategoryId category = DgnDbTestUtils::GetFirstSpatialCategoryId(db);

    TestElementPtr templateEl = TestElement::Create(db, model->GetModelId(), category, "");
    DgnElementCPtr el1 = templateEl->Insert();

    templateEl = TestElement::Create(db, model->GetModelId(), category, "");
    DgnElementCPtr el2 = templateEl->Insert();

    templateEl = TestElement::Create(db, model->GetModelId(), category, "");
    DgnElementCPtr el3 = templateEl->Insert();

    db.SaveChanges("added model");

    auto& txns = db.Txns();

    auto stat = txns.ReverseSingleTxn();
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(!model->IsPersistent());

    stat = txns.ReinstateTxn();  // redo the changes
    ASSERT_TRUE(DgnDbStatus::Success == stat);

    el1 = db.Elements().GetElement(el1->GetElementId());
    el2 = db.Elements().GetElement(el2->GetElementId());
    el3 = db.Elements().GetElement(el3->GetElementId());
    model = db.Models().GetModel(model->GetModelId())->ToPhysicalModelP();
    ASSERT_TRUE(model->IsPersistent());

    stat = model->Delete();
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    db.SaveChanges("deleted model");

    stat = txns.ReverseSingleTxn();
    el1 = db.Elements().GetElement(el1->GetElementId());
    el2 = db.Elements().GetElement(el2->GetElementId());
    el3 = db.Elements().GetElement(el3->GetElementId());
    model = db.Models().GetModel(model->GetModelId())->ToPhysicalModelP();
    ASSERT_TRUE(model->IsPersistent());

    stat = txns.ReinstateTxn();  // redo the delete
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(!model->IsPersistent());

    stat = txns.ReverseSingleTxn(); // undo delete
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    model = db.Models().GetModel(model->GetModelId())->ToPhysicalModelP();
    ASSERT_TRUE(model->IsPersistent());

    auto& displayInfo = model->ToGeometricModelP()->GetFormatterR();
    displayInfo.SetRoundoffUnit(100.0, 200.0);
    ASSERT_TRUE(100.0 == displayInfo.GetRoundoffUnit());
    ASSERT_TRUE(200.0 == displayInfo.GetRoundoffRatio());

    model->Update();
    db.SaveChanges("updated model");
    stat = txns.ReverseSingleTxn(); // undo update
    ASSERT_TRUE(&displayInfo == &model->ToGeometricModelP()->GetFormatterR());
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(100.0 != displayInfo.GetRoundoffUnit());
    ASSERT_TRUE(200.0 != displayInfo.GetRoundoffRatio());

    stat = txns.ReinstateTxn();  // redo the update
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(100.0 == displayInfo.GetRoundoffUnit());
    ASSERT_TRUE(200.0 == displayInfo.GetRoundoffRatio());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void testRangeIndex()
    {
    bvector<RangeIndex::Entry> entries;

    double fuzz = 1.00231326341;
    size_t count = 10000;
    for (size_t i=0; i<count; ++i)
        {
        double v = ((double)(i) - 5000.) * 10.0 * fuzz;
        DRange3d range = DRange3d::From(v, v+1.0, v+2, v+3, v+4, v+5);
        RangeIndex::FBox box(range, true);
        ASSERT_TRUE(box.Low().x <= range.low.x);
        ASSERT_TRUE(box.Low().y <= range.low.y);
        ASSERT_TRUE(box.Low().y <= range.low.y);
        ASSERT_TRUE(box.High().x >= range.high.x);
        ASSERT_TRUE(box.High().y >= range.high.y);
        ASSERT_TRUE(box.High().y >= range.high.y);

        DRange3d range2 = box.ToRange3d();
        ASSERT_TRUE(box.Low().x == range2.low.x);
        ASSERT_TRUE(box.Low().y == range2.low.y);
        ASSERT_TRUE(box.Low().y == range2.low.y);
        ASSERT_TRUE(box.High().x == range2.high.x);
        ASSERT_TRUE(box.High().y == range2.high.y);
        ASSERT_TRUE(box.High().y == range2.high.y);

        entries.push_back(RangeIndex::Entry(box, DgnElementId((uint64_t)i+1)));
        }

    RangeIndex::Tree tree(true, 20);
    for (auto& entry : entries)
        tree.AddEntry(entry);

    ASSERT_TRUE(tree.DebugElementCount() == count);
    ASSERT_TRUE(tree.GetCount() == count);
    ASSERT_TRUE(tree.FindElement(DgnElementId((uint64_t) count+1)) == nullptr);

    for (auto& entry : entries)
        {
        auto found = tree.FindElement(entry.m_id);
        ASSERT_TRUE(found != nullptr);
        ASSERT_TRUE(found->m_id == entry.m_id);
        ASSERT_TRUE(found->m_range.IsBitwiseEqual(entry.m_range));
        }

    for (auto& entry : entries)
        ASSERT_TRUE(SUCCESS == tree.RemoveElement(entry.m_id));

    ASSERT_TRUE(tree.DebugElementCount() == 0);
    ASSERT_TRUE(tree.GetCount() == 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, UndoRedo)
    {
    SetupSeedProject();

    CloseDb();
    BeFileName outFileName = (BeFileName)m_db->GetDbFileName();
    BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite;
    DgnDbTestFixture::GetSeedDbCopy(outFileName,L"Test.bim");
    OpenDb(m_db, outFileName, mode);

    TestDataManager::SetAsStandaloneDb(m_db, mode);
    ASSERT_TRUE(m_db->IsStandalone());

    auto& txns = m_db->Txns();
    m_db->SaveChanges();

    SpatialModelPtr defaultModel = m_db->Models().Get<SpatialModel>(m_defaultModelId);
    ASSERT_TRUE(defaultModel.IsValid());

    TestElementPtr templateEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "", 101.0);

    ASSERT_TRUE(!txns.IsRedoPossible());

    DgnElementCPtr el1 = templateEl->Insert();
    m_db->SaveChanges("change 1");

    ASSERT_TRUE(txns.IsUndoPossible());  // we have an undoable Txn, but nothing undone.
    ASSERT_TRUE(!txns.IsRedoPossible());

    auto stat = txns.ReverseSingleTxn();
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(txns.IsRedoPossible());

    DgnElementCPtr afterUndo= m_db->Elements().GetElement(el1->GetElementId());
    ASSERT_TRUE(!afterUndo.IsValid()); // it should not be in database.
    stat = txns.ReinstateTxn();  // redo the add, put the added element back
    ASSERT_TRUE(DgnDbStatus::Success == stat);

    ASSERT_TRUE(txns.IsUndoPossible());
    ASSERT_TRUE(!txns.IsRedoPossible());

    DgnElementCPtr afterRedo = m_db->Elements().GetElement(el1->GetElementId());
    ASSERT_TRUE(afterRedo.IsValid());
    ASSERT_TRUE(el1.get() != afterRedo.get());

    // make sure that undo/redo of an update also is reflected in the RangeTree
    templateEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "");
    DgnElementCPtr el2 = templateEl->Insert();
    m_db->SaveChanges("create new");
    AxisAlignedBox3d extents1 = defaultModel->QueryElementsRange();

    templateEl->ChangeElement(201.);
    templateEl->Update();
    AxisAlignedBox3d extents2 = defaultModel->QueryElementsRange();
    ASSERT_TRUE (!extents1.IsEqual(extents2));
    m_db->SaveChanges("update one");

    stat = txns.ReverseSingleTxn();
    AxisAlignedBox3d extents3 = defaultModel->QueryElementsRange();
    ASSERT_TRUE (extents1.IsEqual(extents3));    // after undo, range should be back to where it was before we did the update
    ASSERT_TRUE(DgnDbStatus::Success == stat);

    stat = txns.ReinstateTxn();  // redo the update
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    AxisAlignedBox3d extents4 = defaultModel->QueryElementsRange();
    ASSERT_TRUE (extents4.IsEqual(extents2));    // now it should be back to the same as after we did the original update

    templateEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "");
    DgnElementCPtr el3 = templateEl->Insert();

    el1 = m_db->Elements().GetElement(el1->GetElementId()); // reload e11
    el1->Delete();

    DgnElementCPtr afterDelete= m_db->Elements().GetElement(el1->GetElementId());
    ASSERT_TRUE(!afterDelete.IsValid());

    m_db->AbandonChanges();

    DgnElementCPtr afterAbandon = m_db->Elements().GetElement(el1->GetElementId());
    ASSERT_TRUE(afterAbandon.IsValid());

    templateEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "");
    el2 = templateEl->Insert();
    stat = txns.ReverseSingleTxn(); // reversing a txn with pending uncommitted changes should abandon them.
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(nullptr == m_db->Elements().FindLoadedElement(el2->GetElementId()));
    ASSERT_TRUE(!m_db->Elements().GetElement(el2->GetElementId()).IsValid());

    testModelUndoRedo(*m_db);
    testRangeIndex();
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct ModelTxnMonitor : TxnMonitor
{
    typedef bmap<DgnModelId, TxnTable::ChangeType> ChangeMap;

    ChangeMap   m_changes;

    ModelTxnMonitor() { TxnManager::AddTxnMonitor(*this); }
    ~ModelTxnMonitor() { TxnManager::DropTxnMonitor(*this); }

    void CollectChanges(TxnManager& mgr)
        {
        for (auto it : mgr.Models().MakeIterator())
            {
            DgnModelId mid = it.GetModelId();
            auto changeType = it.GetChangeType();
            m_changes[mid] = changeType;
            }
        }

    void _OnCommit(TxnManager& mgr) override {CollectChanges(mgr);}
    void _OnAppliedChanges(TxnManager& mgr) override {CollectChanges(mgr);}
    void _OnUndoRedo(TxnManager& mgr, TxnAction action) override {CollectChanges(mgr);}

    void Clear() { m_changes.clear(); }
    ChangeMap& GetChanges() { return m_changes; }
    bool HasChange(DgnModelId mid, TxnTable::ChangeType type) const
        {
        auto iter = m_changes.find(mid);
        return m_changes.end() != iter && iter->second == type;
        }
    bool WasDeleted(DgnModelId mid) const { return HasChange(mid, TxnTable::ChangeType::Delete); }
    bool WasAdded(DgnModelId mid) const { return HasChange(mid, TxnTable::ChangeType::Insert); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, ModelInsertReverse)
    {
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*=needBriefcase*/);
    auto& txns = m_db->Txns();

    ModelTxnMonitor monitor;

    PhysicalModelPtr model1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "model1");
    m_db->SaveChanges("changeSet1");
    DgnModelId model1Id = model1->GetModelId();
    EXPECT_TRUE(monitor.WasAdded(model1Id));
    monitor.Clear();

    //Reverse insertion.Model 1 shouldn't be in the Db now.
    auto stat = txns.ReverseTxns(1);
    EXPECT_EQ(DgnDbStatus::Success, stat);
    EXPECT_FALSE(m_db->Models().GetModel(model1Id).IsValid());
    EXPECT_TRUE(monitor.WasDeleted(model1Id));
    monitor.Clear();

    //Reinstate Transaction.Model should be back.
    stat = txns.ReinstateTxn();
    EXPECT_TRUE(monitor.WasAdded(model1Id));
    monitor.Clear();
    EXPECT_EQ(DgnDbStatus::Success, stat);
    m_db->SaveChanges("changeSet2");

    EXPECT_TRUE(m_db->Models().GetModel(model1Id).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, ModelDeleteReverse)
    {
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*=needBriefcase*/);
    auto& txns = m_db->Txns();

    PhysicalModelPtr model1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "model1");
    DgnModelId model1Id = model1->GetModelId();
    m_db->SaveChanges("changeSet1");

    DgnDbStatus modelStatus = model1->Delete();
    EXPECT_EQ(DgnDbStatus::Success, modelStatus);
    EXPECT_FALSE(m_db->Models().GetModel(model1Id).IsValid());
    m_db->SaveChanges("changeSet2");

    //Reverse deletion.Model 1 should be in the Db now.
    auto stat = txns.ReverseTxns(1);
    EXPECT_EQ(DgnDbStatus::Success, stat);
    EXPECT_TRUE(m_db->Models().GetModel(model1Id).IsValid());

    //Reinstate Transaction.Model shouldn't be there anymore.
    stat = txns.ReinstateTxn();
    EXPECT_EQ(DgnDbStatus::Success, stat);
    m_db->SaveChanges("changeSet3");

    EXPECT_FALSE(m_db->Models().GetModel(model1Id).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, ElementInsertReverse)
    {
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*=needBriefcase*/);
    auto& txns = m_db->Txns();

    PhysicalModelPtr model1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "model1");
    m_db->SaveChanges("changeSet1");
    DgnModelId m1id = model1->GetModelId();
    EXPECT_TRUE(m1id.IsValid());

    auto keyE1 = InsertElement("E1", m1id);
    auto keyE2 = InsertElement("E2", m1id);
    m_db->SaveChanges("changeSet2");

    DgnElementId e1id = keyE1->GetElementId();
    DgnElementId e2id = keyE2->GetElementId();
    EXPECT_TRUE(e1id.IsValid());
    EXPECT_TRUE(e2id.IsValid());

    //Reverse Transaction.Elements shouldn't be in the model now.
    auto stat = txns.ReverseTxns(1);
    EXPECT_EQ(DgnDbStatus::Success, stat);

    EXPECT_EQ(nullptr, m_db->Elements().FindLoadedElement(e1id));
    EXPECT_EQ(nullptr, m_db->Elements().FindLoadedElement(e2id));

    //Reinstate transcation.The elements should be back in the model.
    stat = txns.ReinstateTxn();
    EXPECT_EQ (DgnDbStatus::Success, stat);
    m_db->SaveChanges("changeSet3");

    DgnElementCPtr e1 = m_db->Elements().GetElement(e1id);
    EXPECT_TRUE(e1 != nullptr);
    EXPECT_NE(nullptr, m_db->Elements().FindLoadedElement(e1id));

    DgnElementCPtr e2 = m_db->Elements().GetElement(e2id);
    EXPECT_TRUE(e2 != nullptr);
    EXPECT_NE(nullptr, m_db->Elements().FindLoadedElement(e2id));

    //Both the elements and the model shouldn't be in the database.
    txns.ReverseAll();
    EXPECT_FALSE(m_db->Models().GetModel(m1id).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TransactionManagerTests, ElementDeleteReverse)
    {
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*=needBriefcase*/);
    auto& txns = m_db->Txns();

    //Creates a model.
    PhysicalModelPtr model1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "model1");
    m_db->SaveChanges("changeSet1");
    DgnModelId m1id = model1->GetModelId();
    EXPECT_TRUE(m1id.IsValid());

    auto keyE1 = InsertElement("E1", m1id);
    m_db->SaveChanges("changeSet2");

    DgnElementId e1id = keyE1->GetElementId();
    EXPECT_TRUE(e1id.IsValid());
    DgnElementCP pE1 = m_db->Elements().FindLoadedElement(e1id);
    EXPECT_NE (nullptr, pE1);
    EXPECT_TRUE(txns.IsUndoPossible());

    //Deletes the Element.
    EXPECT_EQ (DgnDbStatus::Success, m_db->Elements().Delete(*pE1));
    m_db->SaveChanges("changeSet3");

    EXPECT_FALSE(m_db->Elements().GetElement(e1id).IsValid());

    //Reverse Transaction. Element should be back in the model now.
    auto stat = txns.ReverseTxns(1);
    EXPECT_EQ(DgnDbStatus::Success, stat);
    EXPECT_TRUE(m_db->Elements().GetElement(e1id) != nullptr);
    EXPECT_NE(nullptr, m_db->Elements().FindLoadedElement(e1id));

    //Reinstate transcation. The elements shouldn't be in the model.
    stat = txns.ReinstateTxn();
    EXPECT_EQ(DgnDbStatus::Success, stat);
    m_db->SaveChanges("changeSet4");

    EXPECT_FALSE(m_db->Elements().GetElement(e1id).IsValid());

    //Both the elements and the model shouldn't be in the database.
    txns.ReverseAll();
    EXPECT_FALSE(m_db->Models().GetModel(m1id).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TransactionManagerTests, ReverseToPos)
    {
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*=needBriefcase*/);
    auto& txns = m_db->Txns();
    auto txn_id = txns.GetCurrentTxnId();

    //creates model
    PhysicalModelPtr model1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "model1");
    DgnModelId m1id = model1->GetModelId();
    m_db->SaveChanges("changeSet1");

    //Reverse insertion.Model 1 shouldn't be in the Db now.
    DgnDbStatus stat = txns.ReverseTo(txn_id);
    EXPECT_EQ((DgnDbStatus)SUCCESS, stat);
    EXPECT_FALSE(m_db->Models().GetModel(m1id).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TransactionManagerTests, CancelToPos)
    {
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*=needBriefcase*/);
    auto& txns = m_db->Txns();

    //creates model
    DgnModelPtr model1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "model1");
    DgnModelId m1id = model1->GetModelId();
    m_db->SaveChanges("changeSet1");
    TxnManager::TxnId t1 = txns.GetCurrentTxnId();

    //Deletes the model.
    DgnDbStatus modelStatus = model1->Delete ();
    EXPECT_EQ(DgnDbStatus::Success, modelStatus);
    EXPECT_FALSE(m_db->Models().GetModel(m1id).IsValid());
    m_db->SaveChanges("changeSet2");

    //Model should be back in the db.
    DgnDbStatus status = txns.CancelTo(t1);
    EXPECT_EQ(DgnDbStatus::Success, status);
    EXPECT_TRUE(m_db->Models().GetModel(m1id).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TransactionManagerTests, MultiTxnOperation)
    {
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*=needBriefcase*/);
    auto& txns = m_db->Txns();

    //Inserts a  model
    PhysicalModelPtr model1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Model1");
    m_db->SaveChanges("changeSet1");

    txns.BeginMultiTxnOperation();

    //Inserts 2 models..
    PhysicalModelPtr model2 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Model2");
    m_db->SaveChanges("changeSet2");

    m_db->SaveChanges("changeSet3");

    PhysicalModelPtr model3 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Model3");
    auto t3 = txns.GetCurrentTxnId();
    m_db->SaveChanges("changeSet4");

    txns.EndMultiTxnOperation();

    DgnDbStatus status = txns.ReverseTxns(1);
    EXPECT_EQ ((DgnDbStatus)SUCCESS, status);

    //Model2 and Model3 shouldn't be in the db.
    EXPECT_FALSE (m_db->Models().GetModel(model2->GetModelId()).IsValid());
    EXPECT_FALSE (m_db->Models().GetModel(model3->GetModelId()).IsValid());

    status = txns.ReinstateTxn();
    EXPECT_EQ ((DgnDbStatus)SUCCESS, status);

    //Model2 and Model3 should be back in the db.
    EXPECT_TRUE (m_db->Models().GetModel(model2->GetModelId()).IsValid());
    EXPECT_TRUE (m_db->Models().GetModel(model3->GetModelId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct DynamicTxnsTest : TransactionManagerTests
{
    DEFINE_T_SUPER(TransactionManagerTests);

    void InsertElement(bvector<DgnElementId>& ids, bool saveIfNotInDynamics=true)
        {
        static char s_code = 'A';
        Utf8PrintfString code("%c", s_code++);
        DgnElementCPtr elem = DgnDbTestFixture::InsertElement(code.c_str());
        EXPECT_TRUE(elem.IsValid());
        if (saveIfNotInDynamics && !m_db->Txns().InDynamicTxn())
            EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges());

        ids.push_back(elem->GetElementId());
        }

    size_t CountExistingElements(bvector<DgnElementId> const& ids)
        {
        size_t count = 0;
        for (auto const& id : ids)
            if (m_db->Elements().GetElement(id).IsValid())
                ++count;

        return count;
        }
    void ExpectAllExist(bvector<DgnElementId> const& ids)
        {
        EXPECT_EQ(ids.size(), CountExistingElements(ids));
        }
    void ExpectNoneExist(bvector<DgnElementId> const& ids)
        {
        EXPECT_EQ(0, CountExistingElements(ids));
        }

    struct AssertScope
    {
        AssertScope() { BeTest::SetFailOnAssert(false); }
        ~AssertScope() { BeTest::SetFailOnAssert(true); }
    };
};

/*---------------------------------------------------------------------------------**//**
* Test how the API behaves during dynamic vs normal txns.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DynamicTxnsTest, BasicInvariants)
    {
    SetupSeedProject();

    // IsInDynamics accurately reflects pushing and popping of dynamic operations
    DgnDbR db = *m_db;
    auto& txns = db.Txns();
    EXPECT_FALSE(txns.InDynamicTxn());
    txns.BeginDynamicOperation();
    EXPECT_TRUE(txns.InDynamicTxn());
    txns.BeginDynamicOperation();
    EXPECT_TRUE(txns.InDynamicTxn());
    txns.EndDynamicOperation();
    EXPECT_TRUE(txns.InDynamicTxn());
    txns.EndDynamicOperation();
    EXPECT_FALSE(txns.InDynamicTxn());

    // Abandoning changes while in dynamics cancels dynamics
    txns.BeginDynamicOperation();
    EXPECT_TRUE(txns.InDynamicTxn());
    db.AbandonChanges();
    EXPECT_FALSE(txns.InDynamicTxn());

    AssertScope V_V_V_;

    // Cannot begin or end multi-txn operations during dynamics
    txns.BeginDynamicOperation();
    EXPECT_EQ(DgnDbStatus::InDynamicTransaction, txns.BeginMultiTxnOperation());
    txns.EndDynamicOperation();
    EXPECT_EQ(DgnDbStatus::Success, txns.BeginMultiTxnOperation());
    txns.BeginDynamicOperation();
    EXPECT_EQ(DgnDbStatus::InDynamicTransaction, txns.EndMultiTxnOperation());
    txns.EndDynamicOperation();
    EXPECT_EQ(DgnDbStatus::Success, txns.EndMultiTxnOperation());

    // Saving changes is not permitted during dynamics
    txns.BeginDynamicOperation();
    try {
        db.SaveChanges();
        FAIL() << "expected saveChanges() to throw exception";
    } catch(std::runtime_error err) {
        SUCCEED();
    } catch(...) {
        FAIL() << "expect a typed error";
    }
    txns.EndDynamicOperation();
    EXPECT_EQ(BE_SQLITE_ERROR, db.SaveChanges());
    m_db->CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* Test how temporary changes interact with persistent changes, undo/redo, etc.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DynamicTxnsTest, DynamicTxns)
    {
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*=needBriefcase*/);

    DgnDbR db = *m_db;
    auto& txns = db.Txns();
    bvector<DgnElementId> persistentElemIds,
                          dynamicElemIds;

    AssertScope V_V_V_;

    // Elements created during dynamics are reverted afterward
    // Dynamic txns have no effect on persistent txns
    InsertElement(persistentElemIds);
    ExpectAllExist(persistentElemIds);
    txns.BeginDynamicOperation();
    InsertElement(dynamicElemIds);
    ExpectAllExist(dynamicElemIds);
    ExpectAllExist(persistentElemIds);
    txns.EndDynamicOperation();
    ExpectNoneExist(dynamicElemIds);
    InsertElement(persistentElemIds);
    ExpectAllExist(persistentElemIds);

    // Undo/redo behave as if the dynamic txns never occurred
    txns.ReverseSingleTxn();
    EXPECT_EQ(1, CountExistingElements(persistentElemIds));
    ExpectNoneExist(dynamicElemIds);
    txns.ReverseSingleTxn();
    ExpectNoneExist(persistentElemIds);
    ExpectNoneExist(dynamicElemIds);
    txns.ReinstateTxn();
    EXPECT_EQ(1, CountExistingElements(persistentElemIds));
    ExpectNoneExist(dynamicElemIds);
    txns.ReinstateTxn();
    ExpectAllExist(persistentElemIds);
    ExpectNoneExist(dynamicElemIds);

    // Nested dynamic changes are undone independently of outer dynamic txns
    bvector<DgnElementId> innerElemIds,
                          outerElemIds;
    txns.BeginDynamicOperation();
        InsertElement(outerElemIds);
        ExpectAllExist(outerElemIds);
        txns.BeginDynamicOperation();
            InsertElement(innerElemIds);
            ExpectAllExist(outerElemIds);
            ExpectAllExist(innerElemIds);
        txns.EndDynamicOperation();
        ExpectAllExist(outerElemIds);
        ExpectNoneExist(innerElemIds);
        InsertElement(outerElemIds);
        ExpectAllExist(outerElemIds);
    txns.EndDynamicOperation();
    ExpectNoneExist(innerElemIds);
    ExpectNoneExist(outerElemIds);

    // Abandoning changes abandons dynamic changes
    persistentElemIds.clear();
    dynamicElemIds.clear();
    InsertElement(persistentElemIds, false);
    txns.BeginDynamicOperation();
    InsertElement(dynamicElemIds);
    ExpectAllExist(persistentElemIds);
    ExpectAllExist(dynamicElemIds);
    EXPECT_EQ(BE_SQLITE_OK, db.AbandonChanges());
    EXPECT_FALSE(txns.InDynamicTxn());
    ExpectNoneExist(dynamicElemIds);
    ExpectNoneExist(persistentElemIds);

    // A normal txn interrupted by dynamic txns can be resumed transparently
    persistentElemIds.clear();
    EXPECT_EQ(DgnDbStatus::Success, txns.BeginMultiTxnOperation());
    InsertElement(persistentElemIds);
    txns.BeginDynamicOperation();
        InsertElement(dynamicElemIds);
    txns.EndDynamicOperation();
    InsertElement(persistentElemIds);
    EXPECT_EQ(DgnDbStatus::Success, txns.EndMultiTxnOperation());
    ExpectAllExist(persistentElemIds);
    txns.ReverseSingleTxn();
    ExpectNoneExist(persistentElemIds);
    ExpectNoneExist(dynamicElemIds);
    txns.ReinstateTxn();
    ExpectAllExist(persistentElemIds);
    ExpectNoneExist(dynamicElemIds);
    m_db->CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* Currently TxnMonitors receive no callbacks for dynamic txns. We may want to change that
* - but if so, they should be distinct callbacks. We should not be erroneously invoking
* their normal callbacks for dynamic changes.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DynamicTxnsTest, TxnMonitors)
    {
    SetupSeedProject(BeSQLite::Db::OpenMode::ReadWrite, true /*=needBriefcase*/);

    DgnDbR db = *m_db;
    auto& txns = db.Txns();

    TxnMonitorVerifier monitor;

    bvector<DgnElementId> persistentIds, dynamicIds;
    InsertElement(persistentIds);
    EXPECT_TRUE(monitor.m_OnTxnClosedCalled);
    EXPECT_EQ(1, monitor.m_adds.size());

    monitor.Clear();
    EXPECT_TRUE(monitor.IsEmpty());

    txns.BeginDynamicOperation();
        EXPECT_TRUE(monitor.IsEmpty());
        InsertElement(dynamicIds);
        EXPECT_TRUE(monitor.IsEmpty());
    txns.EndDynamicOperation();
    EXPECT_TRUE(monitor.IsEmpty());

    txns.ReverseSingleTxn();
    EXPECT_TRUE(monitor.m_OnTxnAppliedCalled);
    EXPECT_FALSE(monitor.HasInstances());
    EXPECT_FALSE(monitor.m_OnTxnClosedCalled);

    monitor.Clear();
    txns.ReinstateTxn();
    EXPECT_TRUE(monitor.m_OnTxnAppliedCalled);
    EXPECT_FALSE(monitor.HasInstances());
    EXPECT_FALSE(monitor.m_OnTxnClosedCalled);

    ExpectAllExist(persistentIds);
    ExpectNoneExist(dynamicIds);

    persistentIds.clear();
    dynamicIds.clear();

    monitor.Clear();
    InsertElement(persistentIds);
    txns.BeginDynamicOperation();
        InsertElement(dynamicIds);
        InsertElement(dynamicIds);
    txns.EndDynamicOperation();
    InsertElement(persistentIds);

    EXPECT_EQ(2, monitor.m_adds.size());
    EXPECT_EQ(0, monitor.m_deletes.size());
    EXPECT_EQ(0, monitor.m_mods.size());

    monitor.Clear();
    txns.ReverseTxns(2);
    EXPECT_TRUE(monitor.m_OnTxnAppliedCalled);
    EXPECT_FALSE(monitor.HasInstances());

    monitor.Clear();
    txns.ReinstateTxn();
    txns.ReinstateTxn();
    EXPECT_TRUE(monitor.m_OnTxnAppliedCalled);
    EXPECT_FALSE(monitor.HasInstances());
    EXPECT_FALSE(monitor.m_OnTxnClosedCalled);

    ExpectAllExist(persistentIds);
    ExpectNoneExist(dynamicIds);
    }
