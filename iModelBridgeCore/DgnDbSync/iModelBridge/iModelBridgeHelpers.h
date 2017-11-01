/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/iModelBridgeHelpers.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridge.h>

BEGIN_BENTLEY_DGN_NAMESPACE

struct iModelBridgeHelpers
{

// Helper class to ensure that bridge book mark functions are called
struct CallOpenCloseFunctions
    {
    BentleyStatus m_bstatus;
    BentleyStatus m_sstatus = BSIERROR;
    iModelBridge& m_bridge;
    BentleyStatus m_status = BSIERROR;

    CallOpenCloseFunctions(iModelBridge& bridge, DgnDbR db) : m_bridge(bridge)
        {
        CallOpenFunctions(db);
        }

    ~CallOpenCloseFunctions()
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


// Helper class to ensure that bridge _Terminate function is called
struct CallTerminate
    {
    iModelBridge& m_bridge;
    BentleyStatus m_status = BSIERROR;
    CallTerminate(iModelBridge& bridge) : m_bridge(bridge) {}
    ~CallTerminate() {m_bridge._Terminate(m_status);}
    };

};

END_BENTLEY_DGN_NAMESPACE
