/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/TransactionManager_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/WebMercator.h>
#include <DgnPlatform/DgnTexture.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_RENDER

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

    bool IsEmpty() const { return !m_OnTxnClosedCalled && !m_OnTxnReversedCalled && !HasInstances(); }
    bool HasInstances() const { return m_adds.size() + m_deletes.size() + m_mods.size() > 0; }
    };

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct TransactionManagerTests : public DgnDbTestFixture
{
    DEFINE_T_SUPER(DgnDbTestFixture);
public:
    TransactionManagerTests() { }
    ~TransactionManagerTests();
    void TwiddleTime(DgnElementCPtr);
    void SetupProject(WCharCP projFile, WCharCP testFile, Db::OpenMode mode);
};

END_UNNAMED_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .bim project file after copying it to out
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TransactionManagerTests::SetupProject(WCharCP projFile, WCharCP testFile, Db::OpenMode mode)
    {
    T_Super::SetupWithPrePublishedFile(projFile,testFile,mode, true, false);
    ASSERT_TRUE(m_db->IsBriefcase());
    ASSERT_TRUE((Db::OpenMode::ReadWrite != mode) || m_db->Txns().IsTracking());
    }
/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .bim project file after copying it to out
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isElementIdInKeySet(bset<BeInt64Id> const& theSet, DgnElementId element)
    {
    return theSet.find(element) != theSet.end();
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TransactionManagerTests::~TransactionManagerTests()
    {
    SaveDb();
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
* Test of StreetMapModel
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
// Wait for some other free map tile server
//TEST_F(TransactionManagerTests, StreetMapModel)
//    {
//    SetupProject(L"3dMetricGeneral.ibim", L"TransactionManagerTests_StreetMapModel.ibim", Db::OpenMode::ReadWrite);
//
//    auto newModelId = dgn_ModelHandler::StreetMap::CreateStreetMapModel(*m_db, dgn_ModelHandler::StreetMap::MapService::OpenStreetMaps, dgn_ModelHandler::StreetMap::MapType::SatelliteImage, true);
//    ASSERT_TRUE( newModelId.IsValid() );
//
//    DgnModelPtr model = m_db->Models().GetModel(newModelId);
//    ASSERT_TRUE(model.IsValid());
//
//    WebMercatorModel* webmercatormodel = dynamic_cast<WebMercatorModel*>(model.get());
//    ASSERT_NE( nullptr , webmercatormodel );
//    }

/*---------------------------------------------------------------------------------**//**
* Test of Element CRUD
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, CRUD)
    {
    SetupSeedProject();

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
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, ElementAssembly)
    {
    SetupSeedProject();

    TestElementPtr e1 = TestElement::Create(*m_db, m_defaultModelId,m_defaultCategoryId);

    DgnElementCPtr el1 = e1->Insert();
    ASSERT_TRUE(el1.IsValid());

    TestElementPtr e2 = TestElement::Create(*m_db, m_defaultModelId,m_defaultCategoryId);
    e2->SetParentId(el1->GetElementId());
    DgnElementCPtr el2 = e2->Insert();
    ASSERT_TRUE(el2.IsValid());
    ASSERT_EQ(el2->GetParentId(), el1->GetElementId());

    e2 = TestElement::Create(*m_db, m_defaultModelId,m_defaultCategoryId);
    e2->SetParentId(el1->GetElementId());
    DgnElementCPtr el3 = e2->Insert(); // insert another element
    ASSERT_TRUE(el3.IsValid());

    int count =(int) el1->QueryChildren().size(); // now make sure there are two children of el1
    ASSERT_EQ(count, 2);

    auto stat = el3->Delete();      // delete one of the children
    ASSERT_EQ(stat, DgnDbStatus::Success);
    ASSERT_TRUE(!el3->IsPersistent()); // make sure it is no longer persistent

    count =(int) el1->QueryChildren().size(); // should be down to 1 child
    ASSERT_EQ(count, 1);

    ASSERT_TRUE(el1->IsPersistent());    // both parent and child are currently persistent
    ASSERT_TRUE(el2->IsPersistent());

    stat = el1->Delete();               // delete the parent, should also delete child.
    ASSERT_EQ(stat, DgnDbStatus::Success);

    ASSERT_TRUE(!el1->IsPersistent());  // neither should now be persistent
    ASSERT_TRUE(!el2->IsPersistent());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void testModelUndoRedo(DgnDbR db)
    {
    Utf8String name = db.Models().GetUniqueModelName("TestSpatial");

    ModelHandlerR handler = dgn_ModelHandler::Spatial::GetHandler();
    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, db.Domains().GetClassId(handler), DgnModel::CreateModelCode(name)));
    auto modelStatus = model->Insert();
    ASSERT_TRUE(DgnDbStatus::Success == modelStatus);

    auto category = DgnCategory::QueryFirstCategoryId(db);

    TestElementPtr templateEl = TestElement::Create(db, model->GetModelId(), category, "");
    DgnElementCPtr el1 = templateEl->Insert();
    ASSERT_TRUE(el1->IsPersistent());

    templateEl = TestElement::Create(db, model->GetModelId(), category, "");
    DgnElementCPtr el2 = templateEl->Insert();
    ASSERT_TRUE(el2->IsPersistent());

    templateEl = TestElement::Create(db, model->GetModelId(), category, "");
    DgnElementCPtr el3 = templateEl->Insert();
    ASSERT_TRUE(el3->IsPersistent());

    db.SaveChanges("added model");

    auto& txns = db.Txns();
    auto stat = txns.ReverseSingleTxn();
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(!el1->IsPersistent());
    ASSERT_TRUE(!el2->IsPersistent());
    ASSERT_TRUE(!el3->IsPersistent());
    ASSERT_TRUE(!model->IsPersistent());

    stat = txns.ReinstateTxn();  // redo the changes
    ASSERT_TRUE(DgnDbStatus::Success == stat);

    el1 = db.Elements().GetElement(el1->GetElementId());
    el2 = db.Elements().GetElement(el2->GetElementId());
    el3 = db.Elements().GetElement(el3->GetElementId());
    model = db.Models().GetModel(model->GetModelId());
    ASSERT_TRUE(el1->IsPersistent());
    ASSERT_TRUE(el2->IsPersistent());
    ASSERT_TRUE(el3->IsPersistent());
    ASSERT_TRUE(model->IsPersistent());

    stat = model->Delete();
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(!el1->IsPersistent());
    ASSERT_TRUE(!el2->IsPersistent());
    ASSERT_TRUE(!el3->IsPersistent());
    ASSERT_TRUE(!model->IsPersistent());
    db.SaveChanges("deleted model");

    stat = txns.ReverseSingleTxn();
    el1 = db.Elements().GetElement(el1->GetElementId());
    el2 = db.Elements().GetElement(el2->GetElementId());
    el3 = db.Elements().GetElement(el3->GetElementId());
    model = db.Models().GetModel(model->GetModelId());
    ASSERT_TRUE(el1->IsPersistent());
    ASSERT_TRUE(el2->IsPersistent());
    ASSERT_TRUE(el3->IsPersistent());
    ASSERT_TRUE(model->IsPersistent());

    stat = txns.ReinstateTxn();  // redo the delete
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(!el1->IsPersistent());
    ASSERT_TRUE(!el2->IsPersistent());
    ASSERT_TRUE(!el3->IsPersistent());
    ASSERT_TRUE(!model->IsPersistent());

    stat = txns.ReverseSingleTxn(); // undo delete
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    model = db.Models().GetModel(model->GetModelId());
    ASSERT_TRUE(model->IsPersistent());

    auto& displayInfo = model->ToGeometricModelP()->GetDisplayInfoR();
    displayInfo.SetRoundoffUnit(100.0, 200.0);
    ASSERT_TRUE(100.0 == displayInfo.GetRoundoffUnit());
    ASSERT_TRUE(200.0 == displayInfo.GetRoundoffRatio());

    model->Update();
    db.SaveChanges("updated model");
    stat = txns.ReverseSingleTxn(); // undo update
    ASSERT_TRUE(&displayInfo == &model->ToGeometricModelP()->GetDisplayInfoR());
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(100.0 != displayInfo.GetRoundoffUnit());
    ASSERT_TRUE(200.0 != displayInfo.GetRoundoffRatio());

    stat = txns.ReinstateTxn();  // redo the update
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(100.0 == displayInfo.GetRoundoffUnit());
    ASSERT_TRUE(200.0 == displayInfo.GetRoundoffRatio());
    }

/*---------------------------------------------------------------------------------**//**
* Test undo/redo of properties and settings. Normal properties behave normally in response to undo/redo.
* But "Settings" are not supposed to be affected by undo/redo except that
* the undo operation *is* supposed to reverse the effect of "SaveSettings". So,
* if we change setting, then call SaveSettings/SaveChanges, and then call "undo", the setting should remain in the
* post-changed state in memory but that change should *not* be saved to disk. If we call SaveSettings again, the change
* will be saved, again.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void testSettings(DgnDbR db)
    {
    auto& txns = db.Txns();
    PropertySpec setting("setting1", "Test", PropertySpec::Mode::Setting);
    PropertySpec setting2("setting2", "Test", PropertySpec::Mode::Setting);
    PropertySpec setting3("setting3", "Test", PropertySpec::Mode::Setting);
    PropertySpec compressedSetting("commpressedSetting", "Test", PropertySpec::Mode::Setting);
    PropertySpec fakeSetting("setting1", "Test", PropertySpec::Mode::Normal);
    PropertySpec nonsetting("nonSetting", "Test", PropertySpec::Mode::Normal);

    // There are really 3 types of properties/settings:
    // - a string
    // - an uncompressed binary value
    // - a compressed binary value.
    // Each can have updates that change the size of the data. We need to test all permutations with undo/redo

    // first, save a setting that has compressed data
    int compressed[500];
    memset(compressed, 0, sizeof(compressed));
    db.SaveProperty(compressedSetting, compressed, sizeof(compressed));

    // save a string setting.
    Utf8String orig("setting original val");
    db.SavePropertyString(setting, orig.c_str(), 2, 6);

    // save a normal property
    Utf8String nonprop1 = "prop 1";
    db.SavePropertyString(nonsetting, nonprop1.c_str(), 3, 56);

    // save an uncompressed binary setting, we will change its size
    int a[5] = {1,2,3,5,100};
    db.SaveProperty(setting2, a, sizeof(a));

    int sizeSame[12] = {1,2,3,4,5,6,7,8,9,10,11,12}; // save an uncompressed binary setting, we will keep its size
    db.SaveProperty(setting3, sizeSame, sizeof(sizeSame));

    db.SaveSettings(); // this copies all settings changes into the permanent table.

    auto rc = db.SaveChanges("settings change 1"); // save all this as a Txn
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    bool exists = db.HasProperty(setting, 2, 6); // make sure we can still see both properties and settings
    ASSERT_TRUE(exists);
    exists = db.HasProperty(nonsetting, 3, 56);
    ASSERT_TRUE(exists);

    auto stat = txns.ReverseSingleTxn(); // undo all the adds
    ASSERT_TRUE(DgnDbStatus::Success == stat);

    exists = db.HasProperty(setting, 2, 6); // both properties and settings should now be non-existent
    ASSERT_TRUE(!exists);
    exists = db.HasProperty(nonsetting, 3, 56);
    ASSERT_TRUE(!exists);

    Utf8String val;
    stat = txns.ReinstateTxn(); // redo the Txn

    exists = db.HasProperty(setting, 2, 6); // they should all be back
    ASSERT_TRUE(exists);
    exists = db.HasProperty(nonsetting, 3, 56);
    ASSERT_TRUE(exists);

    // now test updating existing settings
    Utf8String state2("setting state 2");
    db.SavePropertyString(setting, state2.c_str(), 2, 6); // change the string setting
    db.QueryProperty(val, setting, 2, 6); // make sure its OK
    ASSERT_TRUE(val.Equals(state2));

    int b[10] = {23,2321,43,12,3400,32,3,21,4}; // change the uncompressed binary setting, changing its size
    db.SaveProperty(setting2, b, sizeof(b));

    Utf8String nonprop2 = "prop in state 2";
    db.SavePropertyString(nonsetting, nonprop2, 3, 56); // change a normal property

    compressed[22]=34;
    db.SaveProperty(compressedSetting, compressed, sizeof(compressed)); // change a compressed binary property

    db.SaveSettings(); // settings become part of the Txn
    db.SaveChanges("settings change 2"); // save Txn

    uint32_t propsize;
    sizeSame[1] = 333;
    db.SaveProperty(setting3, sizeSame, sizeof(sizeSame)); // change this again.
    db.SaveSettings(); // we're going to undo below - this SaveSettings will get abandoned.

    db.QueryPropertySize(propsize, setting3); // make sure the sizes are OK
    ASSERT_TRUE(propsize == sizeof(sizeSame));

    db.QueryProperty(val, setting, 2, 6);
    ASSERT_TRUE(val.Equals(state2));

    db.QueryPropertySize(propsize, setting2);
    ASSERT_TRUE(propsize == sizeof(b));

    int c[10];
    db.QueryProperty(c, propsize, setting2);
    ASSERT_TRUE(0 == memcmp(b,c,propsize));

    db.QueryProperty(val, setting, 2, 6);
    ASSERT_TRUE(val.Equals(state2));

    stat = txns.ReverseSingleTxn(); // now undo the Txn. All settings should remain unchanged.
    ASSERT_TRUE(DgnDbStatus::Success == stat);

    int compressed2[500]; // this is the same size as original array
    db.QueryPropertySize(propsize, compressedSetting); // make sure the size of the compressed setting didn't change
    ASSERT_TRUE(propsize == sizeof(compressed2));

    db.QueryProperty(compressed2, propsize, compressedSetting); // get the compressed data
    ASSERT_TRUE(0 == memcmp(compressed2,compressed,propsize)); // make sure it has the 2nd value, not all zeros

    db.QueryPropertySize(propsize, setting3);
    ASSERT_TRUE(propsize == sizeof(sizeSame));

    // verify that the other settings are in their post-changed state
    int sizeSame2[12];
    db.QueryProperty(sizeSame2, propsize, setting3);
    ASSERT_TRUE(0 == memcmp(sizeSame2,sizeSame,propsize));

    db.QueryProperty(val, nonsetting, 3, 56);
    ASSERT_TRUE(val.Equals(nonprop1));

    db.QueryPropertySize(propsize, setting2);
    ASSERT_TRUE(propsize == sizeof(b));

    db.QueryProperty(c, propsize, setting2);
    ASSERT_TRUE(0 == memcmp(b,c,propsize));

    db.QueryProperty(val, setting, 2, 6);
    ASSERT_TRUE(val.Equals(state2));

    db.QueryProperty(val, fakeSetting, 2, 6); // this is used to peek that the persistent state is post-changed
    ASSERT_TRUE(val.Equals(orig));

    db.QueryProperty(val, fakeSetting, 2, 6);
    ASSERT_TRUE(val.Equals(orig));

    stat = txns.ReinstateTxn(); // redo the updates
    ASSERT_TRUE(DgnDbStatus::Success == stat);

    db.QueryProperty(val, nonsetting, 3, 56);
    ASSERT_TRUE(val.Equals(nonprop2));

    db.QueryProperty(val, setting, 2, 6);
    ASSERT_TRUE(val.Equals(state2));

    stat = txns.ReinstateTxn();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, UndoRedo)
    {
    SetupSeedProject();
    auto& txns = m_db->Txns();
    m_db->SaveChanges();

    TestElementPtr templateEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "", 101.0);

    ASSERT_TRUE(!txns.IsUndoPossible()); // with no changes, you can't undo or redo
    ASSERT_TRUE(!txns.IsRedoPossible());

    DgnElementCPtr el1 = templateEl->Insert();
    m_db->SaveChanges("change 1");

    ASSERT_TRUE(txns.IsUndoPossible());  // we have an undoable Txn, but nothing undone.
    ASSERT_TRUE(!txns.IsRedoPossible());

    ASSERT_TRUE(el1->IsPersistent());
    auto stat = txns.ReverseSingleTxn();
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(!txns.IsUndoPossible());     // we can now redo but not undo
    ASSERT_TRUE(txns.IsRedoPossible());

    DgnElementCPtr afterUndo= m_db->Elements().GetElement(el1->GetElementId());
    ASSERT_TRUE(!afterUndo.IsValid()); // it should not be in database.
    ASSERT_TRUE(!el1->IsPersistent());
    stat = txns.ReinstateTxn();  // redo the add, put the added element back
    ASSERT_TRUE(DgnDbStatus::Success == stat);

    ASSERT_TRUE(txns.IsUndoPossible());
    ASSERT_TRUE(!txns.IsRedoPossible());

    DgnElementCPtr afterRedo = m_db->Elements().GetElement(el1->GetElementId());
    ASSERT_TRUE(afterRedo.IsValid());
    ASSERT_TRUE(!el1->IsPersistent());
    ASSERT_TRUE(el1.get() != afterRedo.get());

    // make sure that undo/redo of an update also is reflected in the RangeTree
    templateEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "");
    DgnElementCPtr el2 = templateEl->Insert();
    m_db->SaveChanges("create new");
    AxisAlignedBox3d extents1 = m_db->Units().ComputeProjectExtents();

    templateEl->ChangeElement(201.);
    templateEl->Update();
    AxisAlignedBox3d extents2 = m_db->Units().ComputeProjectExtents();
    ASSERT_TRUE (!extents1.IsEqual(extents2));
    m_db->SaveChanges("update one");

    stat = txns.ReverseSingleTxn();
    AxisAlignedBox3d extents3 = m_db->Units().ComputeProjectExtents();
    ASSERT_TRUE (extents1.IsEqual(extents3));    // after undo, range should be back to where it was before we did the update
    ASSERT_TRUE(DgnDbStatus::Success == stat);

    stat = txns.ReinstateTxn();  // redo the update
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    AxisAlignedBox3d extents4 = m_db->Units().ComputeProjectExtents();
    ASSERT_TRUE (extents4.IsEqual(extents2));    // now it should be back to the same as after we did the original update

    templateEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "");
    DgnElementCPtr el3 = templateEl->Insert();
    ASSERT_TRUE(el3->IsPersistent());

    el1 = m_db->Elements().GetElement(el1->GetElementId()); // reload e11
    ASSERT_TRUE(el1->IsPersistent());
    el1->Delete();
    ASSERT_TRUE(!el1->IsPersistent());

    DgnElementCPtr afterDelete= m_db->Elements().GetElement(el1->GetElementId());
    ASSERT_TRUE(!afterDelete.IsValid());

    m_db->AbandonChanges();

    ASSERT_TRUE(!el3->IsPersistent());
    ASSERT_TRUE(!el1->IsPersistent());
    DgnElementCPtr afterAbandon = m_db->Elements().GetElement(el1->GetElementId());
    ASSERT_TRUE(afterAbandon.IsValid());
    ASSERT_TRUE(afterAbandon->IsPersistent());

    templateEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "");
    el2 = templateEl->Insert();
    ASSERT_TRUE(el2->IsPersistent());
    stat = txns.ReverseSingleTxn(); // reversing a txn with pending uncommitted changes should abandon them.
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(!el2->IsPersistent());
    ASSERT_TRUE(nullptr == m_db->Elements().FindElement(el2->GetElementId()));
    ASSERT_TRUE(!m_db->Elements().GetElement(el2->GetElementId()).IsValid());

    testModelUndoRedo(*m_db);
    testSettings(*m_db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct ModelTxnMonitor : TxnMonitor
{
    typedef bmap<DgnModelId, TxnTable::ChangeType> ChangeMap;

    ChangeMap   m_changes;

    ModelTxnMonitor() { DgnPlatformLib::GetHost().GetTxnAdmin().AddTxnMonitor(*this); }
    ~ModelTxnMonitor() { DgnPlatformLib::GetHost().GetTxnAdmin().DropTxnMonitor(*this); }

    void CollectChanges(TxnManager& mgr)
        {
        for (auto it : mgr.Models().MakeIterator())
            {
            DgnModelId mid = it.GetModelId();
            auto changeType = it.GetChangeType();
            m_changes[mid] = changeType;
            }
        }

    virtual void _OnCommit(TxnManager& mgr) override {CollectChanges(mgr);}
    virtual void _OnReversedChanges(TxnManager& mgr) override {CollectChanges(mgr);}
    virtual void _OnUndoRedo(TxnManager& mgr, TxnAction action) override {CollectChanges(mgr);}

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
* @bsimethod                                    Maha Nasir                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, ModelInsertReverse)
    {
    SetupSeedProject();
    auto& txns = m_db->Txns();

    ModelTxnMonitor monitor;

    auto seedModelId = m_defaultModelId;
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId);
    DgnModelPtr model1 = seedModel->Clone(DgnModel::CreateModelCode("model1"));
    model1->Insert();
    m_db->SaveChanges("changeSet1");
    DgnModelId model1Id = model1->GetModelId();
    EXPECT_TRUE(monitor.WasAdded(model1Id));
    monitor.Clear();

    ASSERT_TRUE(model1 != nullptr);
    EXPECT_TRUE(m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());

    //Reverse insertion.Model 1 should'nt be in the Db now.
    auto stat = txns.ReverseTxns(1);
    EXPECT_EQ(DgnDbStatus::Success, stat);
    EXPECT_FALSE(m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());
    EXPECT_TRUE(monitor.WasDeleted(model1Id));
    monitor.Clear();

    //Reinstate Transaction.Model should be back.
    stat = txns.ReinstateTxn();
    EXPECT_TRUE(monitor.WasAdded(model1Id));
    monitor.Clear();
    EXPECT_EQ(DgnDbStatus::Success, stat);
    m_db->SaveChanges("changeSet2");

    EXPECT_TRUE(m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, ModelDeleteReverse)
    {
    SetupSeedProject();
    auto& txns = m_db->Txns();

    auto seedModelId = m_defaultModelId;
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId);
    DgnModelPtr model1 = seedModel->Clone(DgnModel::CreateModelCode("model1"));
    model1->Insert();
    m_db->SaveChanges("changeSet1");

    ASSERT_TRUE(model1.IsValid());
    ASSERT_TRUE(model1->GetModelId() == m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")));

    DgnDbStatus modelStatus = model1->Delete();
    EXPECT_EQ(DgnDbStatus::Success, modelStatus);
    EXPECT_FALSE(m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());
    m_db->SaveChanges("changeSet2");

    //Reverse deletion.Model 1 should be in the Db now.
    auto stat = txns.ReverseTxns(1);
    EXPECT_EQ(DgnDbStatus::Success, stat);
    EXPECT_TRUE(m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());

    //Reinstate Transaction.Model should'nt be there anymore.
    stat = txns.ReinstateTxn();
    EXPECT_EQ(DgnDbStatus::Success, stat);
    m_db->SaveChanges("changeSet3");

    EXPECT_FALSE(m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, ElementInsertReverse)
    {
    SetupSeedProject();
    auto& txns = m_db->Txns();

    auto seedModelId = m_defaultModelId;
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId);
    DgnModelPtr model1 = seedModel->Clone(DgnModel::CreateModelCode("model1"));
    model1->Insert();
    m_db->SaveChanges("changeSet1");

    ASSERT_TRUE(model1 != nullptr);
    DgnModelId m1id = m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1"));
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

    EXPECT_EQ(nullptr, m_db->Elements().FindElement(e1id));
    EXPECT_EQ(nullptr, m_db->Elements().FindElement(e2id));

    //Reinstate transcation.The elements should be back in the model.
    stat = txns.ReinstateTxn();
    EXPECT_EQ (DgnDbStatus::Success, stat);
    m_db->SaveChanges("changeSet3");

    DgnElementCPtr e1 = m_db->Elements().GetElement(e1id);
    EXPECT_TRUE(e1 != nullptr);
    EXPECT_NE(nullptr, m_db->Elements().FindElement(e1id));

    DgnElementCPtr e2 = m_db->Elements().GetElement(e2id);
    EXPECT_TRUE(e2 != nullptr);
    EXPECT_NE(nullptr, m_db->Elements().FindElement(e2id));

    //Both the elements and the model shouldn't be in the database.
    txns.ReverseAll(true);
    EXPECT_FALSE(m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TransactionManagerTests, ElementDeleteReverse)
    {
    SetupSeedProject();
    auto& txns = m_db->Txns();

    //Creates a model.
    auto seedModelId = m_defaultModelId;
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId);
    DgnModelPtr model1 = seedModel->Clone(DgnModel::CreateModelCode("model1"));
    model1->Insert();
    m_db->SaveChanges("changeSet1");

    ASSERT_TRUE(model1 != nullptr);
    DgnModelId m1id = m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1"));
    EXPECT_TRUE(m1id.IsValid());

    auto keyE1 = InsertElement("E1", m1id);
    m_db->SaveChanges("changeSet2");

    DgnElementId e1id = keyE1->GetElementId();
    EXPECT_TRUE(e1id.IsValid());
    DgnElementCP pE1 = m_db->Elements().FindElement(e1id);
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
    EXPECT_NE(nullptr, m_db->Elements().FindElement(e1id));

    //Reinstate transcation. The elements shouldn't be in the model.
    stat = txns.ReinstateTxn();
    EXPECT_EQ(DgnDbStatus::Success, stat);
    m_db->SaveChanges("changeSet4");

    EXPECT_FALSE(m_db->Elements().GetElement(e1id).IsValid());

    //Both the elements and the model should'nt be in the database.
    txns.ReverseAll(true);
    EXPECT_FALSE(m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TransactionManagerTests, ReverseToPos)
    {
    SetupSeedProject();
    auto& txns = m_db->Txns();
    auto txn_id = txns.GetCurrentTxnId();

    //creates model
    DgnModelId seedModelId = m_defaultModelId;
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId);
    DgnModelPtr model1 = seedModel->Clone(DgnModel::CreateModelCode("model1"));
    model1->Insert();
    m_db->SaveChanges("changeSet1");

    ASSERT_TRUE (model1 != nullptr);
    EXPECT_TRUE (m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());

    //Reverse insertion.Model 1 should'nt be in the Db now.
    DgnDbStatus stat = txns.ReverseTo(txn_id);
    EXPECT_EQ ((DgnDbStatus)SUCCESS, stat);
    EXPECT_FALSE (m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TransactionManagerTests, CancelToPos)
    {
    SetupSeedProject();
    auto& txns = m_db->Txns();

    //creates model
    DgnModelId seedModelId = m_defaultModelId;
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId);
    DgnModelPtr model1 = seedModel->Clone(DgnModel::CreateModelCode("model1"));
    model1->Insert();
    m_db->SaveChanges("changeSet1");
    auto t1 = txns.GetCurrentTxnId();

    ASSERT_TRUE (model1 != nullptr);
    EXPECT_TRUE (m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());

    //Deletes the model.
    DgnDbStatus modelStatus = model1->Delete ();
    EXPECT_EQ (DgnDbStatus::Success, modelStatus);
    EXPECT_FALSE (m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());
    m_db->SaveChanges("changeSet2");

    //Model should be back in the db.
    DgnDbStatus status = txns.CancelTo(t1);
    EXPECT_EQ ((DgnDbStatus)SUCCESS, status);
    EXPECT_TRUE (m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TransactionManagerTests, MultiTxnOperation)
    {
    SetupSeedProject();
    auto& txns = m_db->Txns();

    //Inserts a  model
    DgnModelId seedModelId = m_defaultModelId;
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId);
    DgnModelPtr model1 = seedModel->Clone(DgnModel::CreateModelCode("Model1"));
    model1->SetLabel("Test Model 1");
    model1->Insert();
    m_db->SaveChanges("changeSet1");

    ASSERT_TRUE (model1 != nullptr);
    EXPECT_TRUE (m_db->Models().QueryModelId(DgnModel::CreateModelCode("Model1")).IsValid());

    txns.BeginMultiTxnOperation();

    //Inserts 2 models..
    DgnModelPtr model2 = seedModel->Clone(DgnModel::CreateModelCode("Model2"));
    model2->SetLabel("Test Model 2");
    model2->Insert();
    m_db->SaveChanges("changeSet2");

    ASSERT_TRUE (model2 != nullptr);
    EXPECT_TRUE (m_db->Models().QueryModelId(DgnModel::CreateModelCode("Model2")).IsValid());

    model2->FillModel();
    EXPECT_TRUE (model2->IsFilled());
    m_db->SaveChanges("changeSet3");

    DgnModelPtr model3 = seedModel->Clone(DgnModel::CreateModelCode("Model3"));
    model3->SetLabel("Test Model 3");
    model3->Insert();
    auto t3 = txns.GetCurrentTxnId();
    m_db->SaveChanges("changeSet4");

    txns.EndMultiTxnOperation();

    DgnDbStatus status = txns.ReverseTxns(1);
    EXPECT_EQ ((DgnDbStatus)SUCCESS, status);

    //Model2 and Model3 shouldn't be in the db.
    EXPECT_FALSE (m_db->Models().QueryModelId(DgnModel::CreateModelCode("Model2")).IsValid());
    EXPECT_FALSE (m_db->Models().QueryModelId(DgnModel::CreateModelCode("Model3")).IsValid());

    status = txns.ReinstateTxn();
    EXPECT_EQ ((DgnDbStatus)SUCCESS, status);

    //Model2 and Model3 shoud be back in the db.
    EXPECT_TRUE (m_db->Models().QueryModelId(DgnModel::CreateModelCode("Model2")).IsValid());
    EXPECT_TRUE (m_db->Models().QueryModelId(DgnModel::CreateModelCode("Model3")).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/15
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
* @bsimethod                                                    Paul.Connelly   11/15
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

    // Saving changes is not permitted during dynamics
    txns.BeginDynamicOperation();
    EXPECT_FALSE(BE_SQLITE_OK == db.SaveChanges());
    txns.EndDynamicOperation();
    EXPECT_EQ(BE_SQLITE_OK, db.SaveChanges());

    // Cannot begin or end multi-txn operations during dynamics
    txns.BeginDynamicOperation();
    EXPECT_EQ(DgnDbStatus::InDynamicTransaction, txns.BeginMultiTxnOperation());
    txns.EndDynamicOperation();
    EXPECT_EQ(DgnDbStatus::Success, txns.BeginMultiTxnOperation());
    txns.BeginDynamicOperation();
    EXPECT_EQ(DgnDbStatus::InDynamicTransaction, txns.EndMultiTxnOperation());
    txns.EndDynamicOperation();
    EXPECT_EQ(DgnDbStatus::Success, txns.EndMultiTxnOperation());
    }

/*---------------------------------------------------------------------------------**//**
* Test how temporary changes interact with persistent changes, undo/redo, etc.
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DynamicTxnsTest, DynamicTxns)
    {
    SetupSeedProject();

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
    }

/*---------------------------------------------------------------------------------**//**
* Currently TxnMonitors receive no callbacks for dynamic txns. We may want to change that
* - but if so, they should be distinct callbacks. We should not be erroneously invoking
* their normal callbacks for dynamic changes.
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DynamicTxnsTest, TxnMonitors)
    {
    SetupSeedProject();

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
    EXPECT_TRUE(monitor.m_OnTxnReversedCalled);
    EXPECT_FALSE(monitor.HasInstances());
    EXPECT_FALSE(monitor.m_OnTxnClosedCalled);

    monitor.Clear();
    txns.ReinstateTxn();
    EXPECT_TRUE(monitor.m_OnTxnReversedCalled);
    EXPECT_FALSE(monitor.HasInstances());
    EXPECT_FALSE(monitor.m_OnTxnClosedCalled);

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
    EXPECT_TRUE(monitor.m_OnTxnReversedCalled);
    EXPECT_FALSE(monitor.HasInstances());

    monitor.Clear();
    txns.ReinstateTxn();
    txns.ReinstateTxn();
    EXPECT_TRUE(monitor.m_OnTxnReversedCalled);
    EXPECT_FALSE(monitor.HasInstances());
    EXPECT_FALSE(monitor.m_OnTxnClosedCalled);

    ExpectAllExist(persistentIds);
    ExpectNoneExist(dynamicIds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t getDependentWidth(DgnElementId id, DgnDbR db)
    {
    auto tx = db.Elements().Get<DgnTexture>(id);
    BeAssert(tx.IsValid());
    return tx.IsValid() ? tx->GetWidth() : -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void incrementDependentWidth(DgnElementId id,DgnDbR db)
    {
    auto cpTx = db.Elements().Get<DgnTexture>(id);
    BeAssert(cpTx.IsValid());
    auto pTx = cpTx.IsValid() ? cpTx->MakeCopy<DgnTexture>() : nullptr;
    if (pTx.IsValid())
        {
        pTx->SetImageSource(pTx->GetImageSource(), pTx->GetWidth() + 1, pTx->GetHeight());
        pTx->Update();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void tickleElement(DgnElementId id, DgnDbR db)
    {
    BeThreadUtilities::BeSleep(1);
    auto el = db.Elements().GetElement(id)->CopyForEdit();
    el->Update();
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct RootChangedCallback : TestElementDrivesElementHandler::Callback
{
    size_t m_invocationCount;

    virtual void _OnRootChanged(DgnDbR db, ECInstanceId relationshipId, DgnElementId source, DgnElementId target)
        {
        ++m_invocationCount;
        incrementDependentWidth(target, db);
        }

    virtual void _ProcessDeletedDependency(DgnDbR db, dgn_TxnTable::ElementDep::DepRelData const& relData) {BeAssert(false && "Not expected to be called");}

    RootChangedCallback() : m_invocationCount(0)
        {
        TestElementDrivesElementHandler::SetCallback(this);
        }

    ~RootChangedCallback()
        {
        TestElementDrivesElementHandler::SetCallback(nullptr);
        }

    size_t GetInvokeCount() const { return m_invocationCount; }
    void ResetInvokeCount() { m_invocationCount = 0; }
    bool WasInvoked() const { return 0 != m_invocationCount; }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DynamicChangesProcessor : DynamicTxnProcessor
{
    size_t m_expectedCount;
    uint32_t m_expectedWidth;
    DgnElementId m_depId;
    RootChangedCallback& m_cb;
    DgnDbR m_db;
    size_t m_actualCount;
    uint32_t m_actualWidth;

    DynamicChangesProcessor(RootChangedCallback& cb, DgnElementId depId, DgnDbR db)
        : m_expectedCount(0), m_expectedWidth(0), m_depId(depId), m_cb(cb), m_db(db),
          m_actualCount(0), m_actualWidth(0) { }

    virtual void _ProcessDynamicChanges() override
        {
        m_actualCount = m_cb.GetInvokeCount();
        m_actualWidth = getDependentWidth(m_depId, m_db);
        }

    void Expect(size_t count, uint32_t width)
        {
        Reset();
        m_expectedCount = count;
        m_expectedWidth = width;
        }

    void Reset()
        {
        m_actualCount = 0;
        m_actualWidth = 0;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DynamicTxnsTest, IndirectChanges)
    {
    SetupSeedProject();

    DgnDbR db = *m_db;
    auto& txns = db.Txns();

    // Set up a dependency between two elements, and register a callback
    DgnCategory cat(DgnCategory::CreateParams(db, "Root", DgnCategory::Scope::Any));
    EXPECT_TRUE(cat.Insert(DgnSubCategory::Appearance()).IsValid());
    DgnElementId rootId = cat.GetElementId();

    Byte textureBytes[] = { 1, 2, 3 };
    ImageSource textureData(ImageSource::Format::Jpeg, ByteStream(textureBytes, 3));
    DgnTexture texture(DgnTexture::CreateParams(db, "Dependent", textureData, 1,1));
    EXPECT_TRUE(texture.Insert().IsValid());
    auto depId = texture.GetElementId();
    db.SaveChanges();

    RootChangedCallback cb;
    TestElementDrivesElementHandler::Insert(*m_db, rootId, depId);
    db.SaveChanges();

    // Make sure our callback is working as expected under normal txns
    // Note the initial addition of the relationship invokes the callback.
    EXPECT_EQ(1, cb.GetInvokeCount());
    EXPECT_EQ(1, getDependentWidth(depId, db));

    // Modifying the root will invoke the callback
    cb.ResetInvokeCount();
    tickleElement(rootId, db);
    db.SaveChanges();
    EXPECT_EQ(1, cb.GetInvokeCount());
    EXPECT_EQ(2, getDependentWidth(depId, db));

    // Undo/redo should not invoke the callback, but should revert/reinstate the effects of the callback
    cb.ResetInvokeCount();
    txns.ReverseSingleTxn();
    EXPECT_FALSE(cb.WasInvoked());
    EXPECT_EQ(1, getDependentWidth(depId, db));

    cb.ResetInvokeCount();
    txns.ReinstateTxn();
    EXPECT_FALSE(cb.WasInvoked());
    EXPECT_EQ(2, getDependentWidth(depId, db));

    // Now test in dynamics
    // Changes should be propagated on EndDynamicOperation()
    cb.ResetInvokeCount();
    DynamicChangesProcessor proc(cb, depId, db);
    txns.BeginDynamicOperation();
        tickleElement(rootId, db);
        EXPECT_FALSE(cb.WasInvoked());
        proc.Expect(1, 3);
    txns.EndDynamicOperation(&proc);

    // EndDynamicOperation should have propagated changes and notified our processor
    EXPECT_EQ(1, proc.m_actualCount);
    EXPECT_EQ(3, proc.m_actualWidth);

    // Once dynamic operation ends, changes should be reverted
    EXPECT_EQ(1, cb.GetInvokeCount());
    EXPECT_EQ(2, getDependentWidth(depId, db));

    // Test nested dynamics
    cb.ResetInvokeCount();
    txns.BeginDynamicOperation();
        tickleElement(rootId, db);
        txns.BeginDynamicOperation();
            // make no changes - ensure outer changes not propagated
            proc.Expect(0, 2);
        txns.EndDynamicOperation(&proc);
        EXPECT_EQ(0, proc.m_actualCount);
        EXPECT_EQ(2, getDependentWidth(depId, db));

        cb.ResetInvokeCount();
        txns.BeginDynamicOperation();
            tickleElement(rootId, db);
            proc.Expect(1, 3);
        txns.EndDynamicOperation(&proc);
        EXPECT_EQ(1, proc.m_actualCount);
        EXPECT_EQ(3, proc.m_actualWidth);
        EXPECT_EQ(2, getDependentWidth(depId, db));

        cb.ResetInvokeCount();
        proc.Expect(1, 3);
    txns.EndDynamicOperation(&proc);
    EXPECT_EQ(1, proc.m_actualCount);
    EXPECT_EQ(3, proc.m_actualWidth);
    EXPECT_EQ(2, getDependentWidth(depId, db));

    cb.ResetInvokeCount();
    db.SaveChanges();
    EXPECT_EQ(2, getDependentWidth(depId, db));
    EXPECT_FALSE(cb.WasInvoked());
    }

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct TestRelationshipLinkTableTrackingTxnMonitor : TxnMonitor
    {
    struct RelDef
        {
        BeSQLite::EC::ECInstanceId relid;
        ECN::ECClassId relclsid;
        DgnElementId srcid;
        DgnElementId tgtid;
        TxnTable::ChangeType changeType;
        };

    bvector<RelDef> m_changes;
    BeSQLite::CachedStatementPtr m_stmt;
    bvector<ECN::ECClassId> m_classesToTrack;

    TestRelationshipLinkTableTrackingTxnMonitor()
        {
        DgnPlatformLib::GetHost().GetTxnAdmin().AddTxnMonitor(*this);
        Clear();
        }
    ~TestRelationshipLinkTableTrackingTxnMonitor()
        {
        DgnPlatformLib::GetHost().GetTxnAdmin().DropTxnMonitor(*this);
        }
    void Clear()
        {
        m_changes.clear();
        }
    //__PUBLISH_EXTRACT_START__ RelationshipLinkTableTrackingTxnMonitor_OnCommit_.sampleCode
    void _OnCommit(TxnManager& txnMgr) override
        {
        // In this example, I track a particular ECRelationshipClass. A real app could have a variety
        // of criteria in its WHERE clause. It's just a SQL SELECT statement. Set it up in whatever way you need.
        // Do not attempt to modify the table!
        if (!m_stmt.IsValid())
            {
            typedef TxnRelationshipLinkTables RLT;
            Utf8String sql = "SELECT ";
            sql.append(RLT::COLNAME_ECInstanceId).append(",");             // 0
            sql.append(RLT::COLNAME_ECClassId).append(",");                // 1
            sql.append(RLT::COLNAME_SourceECInstanceId).append(",");       // 2
            sql.append(RLT::COLNAME_TargetECInstanceId).append(",");       // 3
            sql.append(RLT::COLNAME_ChangeType);                           // 4
            sql.append(" FROM ").append(RLT::TABLE_NAME);
            sql.append(" WHERE ").append(RLT::COLNAME_ECClassId).append(" IN (");
            Utf8CP comma="";
            for (auto clsid : m_classesToTrack)
                {
                sql.append(comma).append(Utf8PrintfString("%" PRIu64, clsid.GetValue()).c_str());
                comma = ",";
                }
            sql.append(")");
            m_stmt = txnMgr.GetDgnDb().GetCachedStatement(sql.c_str());
            }

        while (BE_SQLITE_ROW == m_stmt->Step())
            {
            RelDef def;
            def.relid = m_stmt->GetValueId<BeSQLite::EC::ECInstanceId>      (0);
            def.relclsid = m_stmt->GetValueId<ECN::ECClassId>               (1);
            def.srcid = m_stmt->GetValueId<DgnElementId>                    (2);
            def.tgtid = m_stmt->GetValueId<DgnElementId>                    (3);
            def.changeType = (TxnTable::ChangeType)m_stmt->GetValueInt      (4);
            m_changes.push_back(def);
            }

        m_stmt->Reset();
        m_stmt->ClearBindings();
        }
    //__PUBLISH_EXTRACT_END__

    bool HasChanges() const {return !m_changes.empty(); }

    RelDef const* Find(BeSQLite::EC::ECInstanceId relid)
        {
        for (auto const& def : m_changes)
            {
            if (def.relid == relid)
                return &def;
            }
        return nullptr;
        }

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
static BeSQLite::EC::ECInstanceId insertRelationship(DgnDbR db, ECN::ECClassCR relcls, DgnElementId root, DgnElementId dependent)
    {
    Utf8String ecsql("INSERT INTO ");
    ecsql.append(relcls.GetECSqlName()).append("(SourceECInstanceId,TargetECInstanceId) VALUES(?,?)");

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(ecsql.c_str());
    if (stmt == nullptr)
        return BeSQLite::EC::ECInstanceId();

    if (ECSqlStatus::Success != stmt->BindId(1, root) || 
        ECSqlStatus::Success != stmt->BindId(2, dependent))
        return BeSQLite::EC::ECInstanceId();

    ECInstanceKey rkey;
    if (BE_SQLITE_DONE != stmt->Step(rkey))
        return BeSQLite::EC::ECInstanceId();

    return rkey.GetECInstanceId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
struct ECDbIssueListener : BeSQLite::EC::ECDb::IIssueListener
    {
    mutable BeSQLite::EC::ECDbIssueSeverity m_severity;
    mutable Utf8String m_issue;

    void _OnIssueReported(BeSQLite::EC::ECDbIssueSeverity severity, Utf8CP message) const override
        {
        m_severity = severity;
        m_issue = message;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
static BeSQLite::DbResult modifyRelationshipProperty(DgnDbR db, ECN::ECClassCR relcls, BeSQLite::EC::ECInstanceId relid, Utf8CP propName, Utf8CP newValue)
    {
    Utf8String ecsql("UPDATE ");
    ecsql.append(relcls.GetECSqlName());
    ecsql.append(" SET ").append(propName).append("=?");
    ecsql.append(" WHERE ECInstanceId=?");

    ECDbIssueListener issues;
    db.AddIssueListener(issues);
    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(ecsql.c_str());
    db.RemoveIssueListener();

    stmt->BindText(1, newValue, IECSqlBinder::MakeCopy::No);
    stmt->BindId(2, relid);

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
static BeSQLite::DbResult selectRelationshipProperty(Utf8StringR value, DgnDbR db, ECN::ECClassCR relcls, BeSQLite::EC::ECInstanceId relid, Utf8CP propName)
    {
    Utf8String ecsql("SELECT ");
    ecsql.append(propName);
    ecsql.append(" FROM ");
    ecsql.append(relcls.GetECSqlName());
    ecsql.append("WHERE ECInstanceId=?");

    ECDbIssueListener issues;
    db.AddIssueListener(issues);
    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(ecsql.c_str());
    db.RemoveIssueListener();

    stmt->BindId(1, relid);

    auto rc = stmt->Step();
    if (BE_SQLITE_ROW != rc)
        return rc;

    value = stmt->GetValueText(0);
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
static BeSQLite::DbResult deleteRelationship(DgnDbR db, ECN::ECClassCR relcls, BeSQLite::EC::ECInstanceId relid)
    {
    Utf8String ecsql("DELETE FROM ");
    ecsql.append(relcls.GetECSqlName()).append("WHERE ECInstanceId=?");

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(ecsql.c_str());

    stmt->BindId(1, relid);

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, TestRelationshipLinkTableTracking)
    {
    SetupSeedProject();

    // Put a couple of elements in the default model
    DgnElementId eid1, eid2;
        {
        TestElementPtr e1 = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId);
        TestElementPtr e2 = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId);
        auto pe1 = e1->Insert();
        auto pe2 = e2->Insert();
        ASSERT_TRUE(pe1.IsValid() && pe2.IsValid());
        eid1 = pe1->GetElementId();
        eid2 = pe2->GetElementId();
        ASSERT_EQ(BeSQLite::BE_SQLITE_OK, m_db->SaveChanges());
        }

    TestRelationshipLinkTableTrackingTxnMonitor monitor;

    // Modify one of the elements
        {
        auto e1 = m_db->Elements().GetForEdit<TestElement>(eid1);
        e1->SetTestElementProperty("Changed");
        e1->Update();
        ASSERT_EQ(BeSQLite::BE_SQLITE_OK, m_db->SaveChanges());
        }

    // Verify that my relationship monitor is NOT called when I just change elements.
    ASSERT_FALSE(monitor.HasChanges());

    auto relcls = m_db->Schemas().GetECClass("DgnPlatformTest", "TestElementIsRelatedToElement");
    ASSERT_FALSE(nullptr == relcls);

    //  Insert a relationship.
    BeSQLite::EC::ECInstanceId relid = insertRelationship(*m_db, *relcls, eid1, eid2);
    ASSERT_TRUE(relid.IsValid());
    ASSERT_EQ(BeSQLite::BE_SQLITE_OK, m_db->SaveChanges());
    
    // Verify that my relationship monitor is NOT called, since I haven't started tracking this rel class yet.
    ASSERT_FALSE(monitor.HasChanges());

    // Now start tracking this relationship
    m_db->Txns().BeginTrackingRelationship(*relcls);
    monitor.m_classesToTrack.clear();
    monitor.m_stmt = nullptr;
    monitor.m_classesToTrack.push_back(relcls->GetId());

    //  Delete that relationship
    ASSERT_EQ(BeSQLite::BE_SQLITE_DONE, deleteRelationship(*m_db, *relcls, relid));
    ASSERT_EQ(BeSQLite::BE_SQLITE_OK, m_db->SaveChanges());

    // Verify that my relationship monitor was called and that it detected a delete
    ASSERT_TRUE(monitor.HasChanges());
        {
        TestRelationshipLinkTableTrackingTxnMonitor::RelDef def = monitor.m_changes.front();
        ASSERT_TRUE(def.changeType == TxnTable::ChangeType::Delete);
        ASSERT_TRUE(def.relid == relid);
        ASSERT_TRUE(def.relclsid == relcls->GetId());
        ASSERT_TRUE(def.srcid == eid1);
        ASSERT_TRUE(def.tgtid == eid2);
        monitor.Clear();
        }

    //  Insert another instance of the same relationship class
    auto wasrelid = relid;
    relid = insertRelationship(*m_db, *relcls, eid1, eid2);
    ASSERT_TRUE(relid.IsValid());
    ASSERT_TRUE(relid != wasrelid);
    ASSERT_EQ(BeSQLite::BE_SQLITE_OK, m_db->SaveChanges());

    // Verify that my relationship monitor was called and that it detected an insert
    ASSERT_TRUE(monitor.HasChanges());
        {
        ASSERT_EQ(1, monitor.m_changes.size());
        TestRelationshipLinkTableTrackingTxnMonitor::RelDef const* def = monitor.Find(relid);
        ASSERT_TRUE(nullptr != def);
        ASSERT_TRUE(def->changeType == TxnTable::ChangeType::Insert);
        ASSERT_TRUE(def->relid == relid);
        ASSERT_TRUE(def->relclsid == relcls->GetId());
        ASSERT_TRUE(def->srcid == eid1);
        ASSERT_TRUE(def->tgtid == eid2);
        monitor.Clear();
        }

    //  Modify the relationship
    Utf8String propvalue;
    ASSERT_EQ(BeSQLite::BE_SQLITE_ROW, selectRelationshipProperty(propvalue, *m_db, *relcls, relid, "Property1"));
    ASSERT_TRUE(propvalue.empty());
    ASSERT_EQ(BeSQLite::BE_SQLITE_DONE, modifyRelationshipProperty(*m_db, *relcls, relid, "Property1", "Changed"));
    ASSERT_EQ(BeSQLite::BE_SQLITE_OK, m_db->SaveChanges());
    ASSERT_EQ(BeSQLite::BE_SQLITE_ROW, selectRelationshipProperty(propvalue, *m_db, *relcls, relid, "Property1"));
    ASSERT_STREQ("Changed", propvalue.c_str());

    // Verify that my relationship monitor was called and that it detected an update
    ASSERT_TRUE(monitor.HasChanges());
        {
        ASSERT_EQ(1, monitor.m_changes.size());
        TestRelationshipLinkTableTrackingTxnMonitor::RelDef def = monitor.m_changes.front();
        ASSERT_TRUE(def.changeType == TxnTable::ChangeType::Update);
        ASSERT_TRUE(def.relid == relid);
        ASSERT_TRUE(def.relclsid == relcls->GetId());
        ASSERT_TRUE(def.srcid == eid1);
        ASSERT_TRUE(def.tgtid == eid2);
        monitor.Clear();
        }

    //  Start tracking a different relationship class, and insert an instance of that class
    auto relcls2 = m_db->Schemas().GetECClass("DgnPlatformTest", "TestElementIsRelatedToElement2");
    ASSERT_FALSE(nullptr == relcls2);
    m_db->Txns().BeginTrackingRelationship(*relcls2);

    auto relid2 = insertRelationship(*m_db, *relcls2, eid1, eid2);
    ASSERT_TRUE(relid2.IsValid());
    ASSERT_EQ(BeSQLite::BE_SQLITE_OK, m_db->SaveChanges());

    // Verify that my relationship monitor was NOT called, because it is still selecting on the other relationship class
    ASSERT_FALSE(monitor.HasChanges());

    // Now point my monitor at the second class also, delete the instance, and verify that I saw the deletion
    monitor.m_classesToTrack.push_back(relcls2->GetId());
    monitor.m_stmt = nullptr;
    ASSERT_EQ(2, monitor.m_classesToTrack.size());
    ASSERT_EQ(BE_SQLITE_DONE, deleteRelationship(*m_db, *relcls2, relid2));
    ASSERT_EQ(BeSQLite::BE_SQLITE_OK, m_db->SaveChanges());
    ASSERT_TRUE(monitor.HasChanges());
        {
        ASSERT_EQ(1, monitor.m_changes.size());
        TestRelationshipLinkTableTrackingTxnMonitor::RelDef def = monitor.m_changes.front();
        ASSERT_TRUE(def.changeType == TxnTable::ChangeType::Delete);
        ASSERT_TRUE(def.relid == relid2);
        ASSERT_TRUE(def.relclsid == relcls2->GetId());
        ASSERT_TRUE(def.srcid == eid1);
        ASSERT_TRUE(def.tgtid == eid2);
        monitor.Clear();
        }

    //  Now modify the first relationship, and make sure that I am still tracking it
    ASSERT_EQ(BeSQLite::BE_SQLITE_DONE, modifyRelationshipProperty(*m_db, *relcls, relid, "Property1", "Changed again"));
    ASSERT_EQ(BeSQLite::BE_SQLITE_OK, m_db->SaveChanges());
    ASSERT_TRUE(monitor.HasChanges());
        {
        ASSERT_EQ(1, monitor.m_changes.size());
        TestRelationshipLinkTableTrackingTxnMonitor::RelDef def = monitor.m_changes.front();
        ASSERT_TRUE(def.changeType == TxnTable::ChangeType::Update);
        ASSERT_TRUE(def.relid == relid);
        ASSERT_TRUE(def.relclsid == relcls->GetId());
        ASSERT_TRUE(def.srcid == eid1);
        ASSERT_TRUE(def.tgtid == eid2);
        monitor.Clear();
        }

    //  Finally, modify the first and insert an instance of the second, and verify that both were tracked
    ASSERT_EQ(BeSQLite::BE_SQLITE_DONE, modifyRelationshipProperty(*m_db, *relcls, relid, "Property1", "Changed a third time"));
    relid2 = insertRelationship(*m_db, *relcls2, eid1, eid2);
    ASSERT_TRUE(relid2.IsValid());
    ASSERT_EQ(BeSQLite::BE_SQLITE_OK, m_db->SaveChanges());
    ASSERT_TRUE(monitor.HasChanges());
    ASSERT_EQ(2, monitor.m_changes.size());
        {
        TestRelationshipLinkTableTrackingTxnMonitor::RelDef const* def1 = monitor.Find(relid);
        ASSERT_TRUE(nullptr != def1);
        ASSERT_EQ(TxnTable::ChangeType::Update, def1->changeType);
        TestRelationshipLinkTableTrackingTxnMonitor::RelDef const* def2 = monitor.Find(relid2);
        ASSERT_TRUE(nullptr != def2);
        ASSERT_EQ(TxnTable::ChangeType::Insert, def2->changeType);
        }
    }


