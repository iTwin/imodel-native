/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/TransactionManager_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_SQLITE
#if defined (NEEDS_WORK_DGNITEM)

/*=================================================================================**//**
* @bsiclass                                                        Moazzum Ali 02/10
+===============+===============+===============+===============+===============+======*/
struct TxnMonitorVerifier : TxnMonitor
    {
    bool m_OnTxnClosedCalled;
    bool m_OnTxnReverseCalled;
    bool m_OnTxnReversedCalled;

    void Clear()
        {
        m_OnTxnClosedCalled = false;
        m_OnTxnReverseCalled = false;
        m_OnTxnReversedCalled = false;
        }

    TxnMonitorVerifier () 
        {
        DgnPlatformLib::GetHost().GetTxnAdmin().AddTxnMonitor(*this);
        Clear ();
        }

    ~TxnMonitorVerifier ()
        {
        DgnPlatformLib::GetHost().GetTxnAdmin().DropTxnMonitor(*this);
        }

    virtual void _OnTxnBoundary (TxnSummaryCR) override {m_OnTxnClosedCalled = true;}
    virtual void _OnTxnReverse (TxnSummaryCR, bool isUndo) override {m_OnTxnReverseCalled = true;}
    virtual void _OnTxnReversed (TxnSummaryCR, bool isUndo) override {m_OnTxnReversedCalled = true;}
    };

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Transaction Manager
* @bsimethod                                    Majd.Uddin                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct TransactionManagerTests : public ::testing::Test
{
public:
    ScopedDgnHost           m_host;
    DgnProjectPtr      m_proj;

/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .dgndb project file after copying it to out
* @bsimethod                                    Majd.Uddin                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SetupProject (WCharCP projFile, WCharCP testFile, BeSQLite::Db::OpenMode mode)
    {
    BeFileName outFileName;
    ASSERT_EQ (SUCCESS, DgnDbTestDgnManager::GetTestDataOut (outFileName, projFile, testFile, __FILE__));
    DgnFileStatus result;
    m_proj = DgnProject::OpenProject (&result, outFileName, DgnProject::OpenParams(mode));
    ASSERT_TRUE (m_proj.IsValid());
    ASSERT_TRUE( result == DGNFILE_STATUS_Success);
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
static ElementId addLineElement (ITxn& txn, DgnModelR model)
    {
    DSegment3d segment;
    segment.Init (0,0,0, 100,100,100);

    EditElementHandle eh;
    ExtendedElementHandler::InitializeElement (eh, NULL, model, model.Is3d());
    EXPECT_EQ (SUCCESS, eh.AddToModel());

    static Utf8CP testxatt="this is a test of xatt";
    EXPECT_EQ (SUCCESS, txn.AddXAttribute (eh.GetElementRef(), XAttributeHandlerId (100,20), 101, testxatt, (UInt32) strlen(testxatt)));

    return  eh.GetElementId();
    }

static DRange3d s_modifyRange = {-100000., -200000., -30000., 300000., 400000., 500000.};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void modifyLine (ElementId lineId, DgnModelR model)
    {
    EditElementHandle old (lineId, model);
    ASSERT_TRUE (old.IsValid());

    DSegment3d segment;
    segment.Init (s_modifyRange.low.x,
                  s_modifyRange.low.y,
                  s_modifyRange.low.z,
                  s_modifyRange.high.x,
                  s_modifyRange.high.y,
                  s_modifyRange.high.z);

    EditElementHandle eh;
    EXPECT_EQ (SUCCESS, LineHandler::CreateLineElement (eh, NULL, segment, model.Is3d(), model));

    ElementRefP elRef = old.GetElementRef();
    old.ReplaceElement (eh.GetElementP());
    old.ReplaceInModel (elRef);
    }

/*---------------------------------------------------------------------------------**//**
* Transaction on add/deleting/modifying Elements
* @bsimethod                                    Majd.Uddin                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TransactionManagerTests, Elements)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"Elements.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    ITxnManagerR txnMgr = m_proj->GetTxnManager();
    ITxn& txn = txnMgr.GetCurrentTxn();
    txnMgr.Activate ();

    TxnMonitorVerifier monitor;

    //adding a line to the file
    DgnElementPool& pool = m_proj->Models().ElementPool();
    DgnModelP model = m_proj->Models().GetAndFillModelById (NULL, m_proj->Models().GetFirstModelId());
    ASSERT_TRUE (model != NULL);
    model->FillSections();

    txnMgr.SetTxnSource (12345);
    txnMgr.SetTxnDescription ("test adding elements");

    int origElemCount = model->GetElementCount(DgnModelSections::GraphicElements);
    ElementId line1 = addLineElement(txn, *model);

    // Element count should be 4 in the above model as there are 3 existing elements
    EXPECT_EQ(origElemCount+1, model->GetElementCount(DgnModelSections::GraphicElements));

    // Close element creation txn.
    txnMgr.CloseCurrentTxn();
    txnMgr.SetTxnSource (6789);
    txnMgr.SetTxnDescription ("second stage");

    EXPECT_TRUE(monitor.m_OnTxnClosedCalled);

    // create an element that isn't committed. This will be undone too, but won't be redone. Instead, it should be canceled.
    ElementId line2 = addLineElement(txn, *model);

    // reverse the transaction, also deletes/cancels line2 since it was never committed
    txnMgr.ReverseTxns (1);

    // purge any garbage elements so we can make sure the cancel of line2 worked.
    pool.Purge(0);

    //The element count should be back to 3 now
    EXPECT_EQ(origElemCount, model->GetElementCount(DgnModelSections::GraphicElements)); 

    PersistentElementRefP testRef = pool.FindElementById(line1); // it should be deleted with a ref count of 1
    EXPECT_TRUE (NULL!=testRef);
    EXPECT_TRUE (testRef==model->FindElementById(line1)); // it should still be in the model too
    EXPECT_TRUE (testRef->IsDeleted());
    EXPECT_EQ (1, testRef->GetRefCount());

    testRef = pool.FindElementById(line2); // this element should not exist anymore
    EXPECT_TRUE (NULL==testRef);
    EXPECT_TRUE (NULL==model->FindElementById(line2));
    
    EXPECT_TRUE(monitor.m_OnTxnReverseCalled);
    EXPECT_TRUE(monitor.m_OnTxnReversedCalled);

    // now reinstate the txn. Line1 should no longer be deleted and should be found in the model
    txnMgr.ReinstateTxn();
    EXPECT_EQ(origElemCount+1, model->GetElementCount(DgnModelSections::GraphicElements)); 
    testRef = model->FindElementById(line1);
    EXPECT_TRUE (NULL!=testRef);
    EXPECT_EQ (1, testRef->GetRefCount());
    EXPECT_FALSE (testRef->IsDeleted());
    
    addLineElement(txn, *model);
    txnMgr.CloseCurrentTxn();

    // save the current range
    DRange3d originalRange = *model->GetRangeIndexP(true)->GetRange();

    modifyLine (line1, *model); // change the element to increase its range
    txnMgr.CloseCurrentTxn();

    // the range tree should reflect the modified element's range
    DRange3d tRange = *model->GetRangeIndexP(true)->GetRange();
    EXPECT_EQ(0, memcmp(&s_modifyRange, &tRange, sizeof(DRange3d))); 

    txnMgr.ReverseTxns (1); // undo the modify and the range tree should be back to the pre-modified state
    EXPECT_EQ(0, memcmp(&originalRange, model->GetRangeIndexP(true)->GetRange(), sizeof(DRange3d)));

    txnMgr.ReinstateTxn(); // redo and the range tree should now be in the post-modify state
    EXPECT_EQ(0, memcmp(&s_modifyRange, model->GetRangeIndexP(true)->GetRange(), sizeof(DRange3d)));
    }
#endif
