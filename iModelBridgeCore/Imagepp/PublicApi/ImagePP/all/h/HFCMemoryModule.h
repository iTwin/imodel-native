//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMemoryModule.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCErrorCodeModule.h"

class HFCMemoryExceptionBuilder;
class HFCMemoryModule : public HFCErrorCodeModule
    {
public:
    //--------------------------------------
    // Constants
    //--------------------------------------

    // This value represents the ID for the memory module.
    static const uint32_t s_ModuleID;


    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    _HDLLu                HFCMemoryModule();
    _HDLLu virtual         ~HFCMemoryModule();


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Holds the builders/handlers
    typedef list<HFCMemoryExceptionBuilder* >
    Builders;
    Builders        m_Builders;
    };
