//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCStandardModules.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This class, when built, will register the standard modules.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCGeneralModule.h"
#include "HFCFileModule.h"
#include "HFCMemoryModule.h"
#include "HFCDeviceModule.h"


class HFCStandardModules
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HFCStandardModules();
    ~HFCStandardModules();


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    HFCGeneralModule    m_GeneralModule;
    HFCFileModule       m_FileModule;
    HFCMemoryModule     m_MemoryModule;
    HFCDeviceModule     m_DeviceModule;
    };

#include "HFCStandardModules.hpp"

