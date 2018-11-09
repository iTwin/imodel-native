/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/iModelBridgeHelpers.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridge.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTimeUtilities.h>
#include <iModelBridge/iModelBridgeFwk.h>

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
        StopWatch stopWatch;
        m_bstatus = m_bridge._OnOpenBim(db);
        if (BSISUCCESS == m_bstatus)
            m_sstatus = m_bridge._OpenSource();

        iModelBridgeFwk::LogPerformance(stopWatch, "Time required to open source file and BIM");
        }

    void CallCloseFunctions()
        {
        StopWatch stopWatch;
        if (BSISUCCESS != m_bstatus)    // If I never opened the BIM, then don't make any callbacks
            return;

        if (BSISUCCESS == m_sstatus)    //  If I opened the source
            m_bridge._CloseSource(m_status);    // close it
        
        m_bridge._OnCloseBim(m_status); // close the bim

        iModelBridgeFwk::LogPerformance(stopWatch, "Time required to close source file and BIM");
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


END_BENTLEY_DGN_NAMESPACE
