//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTExtDllState.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

class HUTExtDLLState
    {
public:
    HUTExtDLLState();
    ~HUTExtDLLState();
protected:
    HINSTANCE m_hInstOld;
    };
