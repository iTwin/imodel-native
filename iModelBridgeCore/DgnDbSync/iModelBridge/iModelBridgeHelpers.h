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
// Helper class to ensure that bridge _CloseSource function is called
struct CallCloseSource
    {
    iModelBridge& m_bridge;
    BentleyStatus m_status = BSIERROR;
    bool m_closeOnErrorOnly;

    CallCloseSource(iModelBridge& bridge, bool closeOnErrorOnly) : m_bridge(bridge), m_closeOnErrorOnly(closeOnErrorOnly) {}

    ~CallCloseSource()
        {
        if (m_closeOnErrorOnly && (BSISUCCESS == m_status)) // if we should only close in case of error and there is no error
            return;                                         //  don't close
        m_bridge._CloseSource(m_status);
        }
    };

// Helper class to ensure that bridge _Converted function is called
struct CallOnBimClose
    {
    iModelBridge& m_bridge;
    BentleyStatus m_status = BSIERROR;
    bool m_closeOnErrorOnly;

    CallOnBimClose(iModelBridge& bridge, bool closeOnErrorOnly) : m_bridge(bridge), m_closeOnErrorOnly(closeOnErrorOnly) {}

    ~CallOnBimClose() 
        {
        if (m_closeOnErrorOnly && (BSISUCCESS == m_status)) // if we should only close in case of error and there is no error
            return;                                         //  don't close
        m_bridge._OnCloseBim(m_status);
        }
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
