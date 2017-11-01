/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/iModelBridgeHelpers.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridge.h>
#include <DgnPlatform/DgnPlatformLib.h>

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
* Helper class to ensure that bridge open/close functions are called
* @bsiclass                                                     Sam.Wilson      10/17
+===============+===============+===============+===============+===============+======*/
struct iModelBridgeCallOpenCloseFunctions
    {
    BentleyStatus m_bstatus;
    BentleyStatus m_sstatus = BSIERROR;
    iModelBridge& m_bridge;
    BentleyStatus m_status = BSIERROR;

    iModelBridgeCallOpenCloseFunctions(iModelBridge& bridge, DgnDbR db) : m_bridge(bridge)
        {
        CallOpenFunctions(db);
        }

    ~iModelBridgeCallOpenCloseFunctions()
        {
        CallCloseFunctions();
        }

    void CallOpenFunctions(DgnDbR db)
        {
        m_bstatus = m_bridge._OnOpenBim(db);
        if (BSISUCCESS == m_bstatus)
            m_sstatus = m_bridge._OpenSource();
        }

    void CallCloseFunctions()
        {
        if (BSISUCCESS != m_bstatus)    // If I never opened the BIM, then don't make any callbacks
            return;

        if (BSISUCCESS == m_sstatus)    //  If I opened the source
            m_bridge._CloseSource(m_status);    // close it
        
        m_bridge._OnCloseBim(m_status); // close the bim
        }

    bool IsReady() const {return (BSISUCCESS==m_bstatus) && (BSISUCCESS==m_sstatus);}
    };


/*=================================================================================**//**
* Helper class to ensure that bridge _Terminate function is called
* @bsiclass                                                     Sam.Wilson      10/17
+===============+===============+===============+===============+===============+======*/
struct iModelBridgeCallTerminate
    {
    iModelBridge& m_bridge;
    BentleyStatus m_status = BSIERROR;
    iModelBridgeCallTerminate(iModelBridge& bridge) : m_bridge(bridge) {}
    ~iModelBridgeCallTerminate() {m_bridge._Terminate(m_status);}
    };

/*=================================================================================**//**
* Put an instance of this class on the stack to flag down calls to DgnDb::SaveChanges or undo/redo.
* @bsiclass                                                     Sam.Wilson      10/17
+===============+===============+===============+===============+===============+======*/
struct iModelBridgeLockOutTxnMonitor : TxnMonitor
    {
    DgnDbR m_dgndb;

    iModelBridgeLockOutTxnMonitor(DgnDbR db) : m_dgndb(db)
        {
        DgnPlatformLib::GetHost().GetTxnAdmin().AddTxnMonitor(*this);
        m_dgndb.Domains().DisableSchemaImport();
        }
    ~iModelBridgeLockOutTxnMonitor()
        {
        DgnPlatformLib::GetHost().GetTxnAdmin().DropTxnMonitor(*this);
        m_dgndb.Domains().EnableSchemaImport();
        }
    void _OnCommit(TxnManager& txnMgr) override
        {
        BeAssert(false);
        TxnManager::ValidationError err(TxnManager::ValidationError::Severity::Fatal, "SaveChanges not permitted");
        txnMgr.ReportError(err);
        }
    void _OnAppliedChanges(TxnManager& txnMgr) override
        {
        BeAssert(false);
        TxnManager::ValidationError err(TxnManager::ValidationError::Severity::Fatal, "Undo/redo and change-merging not permitted");
        txnMgr.ReportError(err);
        }

    };

END_BENTLEY_DGN_NAMESPACE
