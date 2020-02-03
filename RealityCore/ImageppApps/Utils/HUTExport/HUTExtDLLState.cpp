//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTExtDLLState.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////
// File HUTExtDLLState.cpp
////////////////////////////////////////////////////////////////////////////////////////////

HUTExtDLLState::HUTExtDLLState()
    {
    m_hInstOld = AfxGetResourceHandle();
    AfxSetResourceHandle(extensionDLL.hResource);
    }

////////////////////////////////////////////////////////////////////////////////////////////

HUTExtDLLState::~HUTExtDLLState()
    {
    AfxSetResourceHandle(m_hInstOld);
    }
