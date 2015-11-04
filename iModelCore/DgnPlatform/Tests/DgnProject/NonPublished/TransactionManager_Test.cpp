/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/TransactionManager_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECSqlBuilder.h>
#include <DgnPlatform/WebMercator.h>

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

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .dgndb project file after copying it to out
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isElementIdInKeySet(bset<ECInstanceId> const& theSet, DgnElementId element)
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
* Test of StreetMapModel
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
// Wait for some other free map tile server
//TEST_F(TransactionManagerTests, StreetMapModel)
//    {
//    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests_StreetMapModel.idgndb", Db::OpenMode::ReadWrite);
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
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests_CRUD.idgndb", Db::OpenMode::ReadWrite);

    m_db->SaveChanges();
    m_db->Txns().EnableTracking(true);
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
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests.idgndb", Db::OpenMode::ReadWrite);

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
    Utf8String name = db.Models().GetUniqueModelName("testphysical");

    ModelHandlerR handler = dgn_ModelHandler::Physical::GetHandler();
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

    auto& props = model->GetPropertiesR();
    props.SetRoundoffUnit(100.0, 200.0);
    ASSERT_TRUE(100.0 == props.GetRoundoffUnit());
    ASSERT_TRUE(200.0 == props.GetRoundoffRatio());

    model->Update();
    db.SaveChanges("updated model");
    stat = txns.ReverseSingleTxn(); // undo update
    ASSERT_TRUE(&props == &model->GetPropertiesR());
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(100.0 != props.GetRoundoffUnit());
    ASSERT_TRUE(200.0 != props.GetRoundoffRatio());

    stat = txns.ReinstateTxn();  // redo the update
    ASSERT_TRUE(DgnDbStatus::Success == stat);
    ASSERT_TRUE(100.0 == props.GetRoundoffUnit());
    ASSERT_TRUE(200.0 == props.GetRoundoffRatio());
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
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests.idgndb", Db::OpenMode::ReadWrite);
    auto& txns = m_db->Txns();
    txns.EnableTracking(true);

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
* @bsimethod                                    Maha Nasir                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, ModelInsertReverse)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    auto& txns = m_db->Txns();
    txns.EnableTracking(true);

    auto seedModelId = m_defaultModelId;
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId);
    DgnModelPtr model1 = seedModel->Clone(DgnModel::CreateModelCode("model1"));
    model1->Insert();
    m_db->SaveChanges("changeSet1");

    ASSERT_TRUE(model1 != nullptr);
    EXPECT_TRUE(m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());

    //Reverse insertion.Model 1 should'nt be in the Db now.
    auto stat = txns.ReverseTxns(1);
    EXPECT_EQ(DgnDbStatus::Success, stat);
    EXPECT_FALSE(m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());

    //Reinstate Transaction.Model should be back.
    stat = txns.ReinstateTxn();
    EXPECT_EQ(DgnDbStatus::Success, stat);
    m_db->SaveChanges("changeSet2");

    EXPECT_TRUE(m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TransactionManagerTests, ModelDeleteReverse)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    auto& txns = m_db->Txns();
    txns.EnableTracking(true);

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
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    auto& txns = m_db->Txns();
    txns.EnableTracking(true);

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

    EXPECT_FALSE(m_db->Elements().QueryElementKey(e1id).IsValid());
    EXPECT_FALSE(m_db->Elements().QueryElementKey(e2id).IsValid());

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
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    auto& txns = m_db->Txns();
    txns.EnableTracking(true);

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

    EXPECT_FALSE(m_db->Elements().QueryElementKey(e1id).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(e1id) == nullptr);

    //Reverse Transaction. Element should be back in the model now.
    auto stat = txns.ReverseTxns(1);
    EXPECT_EQ(DgnDbStatus::Success, stat);
    EXPECT_TRUE(m_db->Elements().GetElement(e1id) != nullptr);
    EXPECT_NE(nullptr, m_db->Elements().FindElement(e1id));

    //Reinstate transcation. The elements shouldn't be in the model.
    stat = txns.ReinstateTxn();
    EXPECT_EQ(DgnDbStatus::Success, stat);
    m_db->SaveChanges("changeSet4");

    EXPECT_FALSE(m_db->Elements().QueryElementKey(e1id).IsValid());

    //Both the elements and the model should'nt be in the database.
    txns.ReverseAll(true);
    EXPECT_FALSE(m_db->Models().QueryModelId(DgnModel::CreateModelCode("model1")).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TransactionManagerTests, ReverseToPos)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    auto& txns = m_db->Txns();
    txns.EnableTracking(true);
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
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    auto& txns = m_db->Txns();
    txns.EnableTracking(true);

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
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    auto& txns = m_db->Txns();
    txns.EnableTracking(true);

    //Inserts a  model
    DgnModelId seedModelId = m_defaultModelId;
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId);
    DgnModelPtr model1 = seedModel->Clone(DgnModel::CreateModelCode("Model1"));
    model1->Insert("Test Model 1");
    m_db->SaveChanges("changeSet1");

    ASSERT_TRUE (model1 != nullptr);
    EXPECT_TRUE (m_db->Models().QueryModelId(DgnModel::CreateModelCode("Model1")).IsValid());

    txns.BeginMultiTxnOperation();

    //Inserts 2 models..
    DgnModelPtr model2 = seedModel->Clone(DgnModel::CreateModelCode("Model2"));
    model2->Insert("Test Model 2");
    m_db->SaveChanges("changeSet2");

    ASSERT_TRUE (model2 != nullptr);
    EXPECT_TRUE (m_db->Models().QueryModelId(DgnModel::CreateModelCode("Model2")).IsValid());

    model2->FillModel();
    EXPECT_TRUE (model2->IsFilled());
    m_db->SaveChanges("changeSet3");

    DgnModelPtr model3 = seedModel->Clone(DgnModel::CreateModelCode("Model3"));
    model3->Insert("Test Model 3");
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
