//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCFileModule.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCErrorCodeModule.h"

class HFCFileExceptionBuilder;
class HFCFileModule : public HFCErrorCodeModule
    {
public:
    //--------------------------------------
    // Constants
    //--------------------------------------

    // This value represents the ID for the file module.
    static const uint32_t s_ModuleID;


    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    _HDLLu                 HFCFileModule();
    _HDLLu virtual         ~HFCFileModule();


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Holds the builders/handlers
    typedef list<HFCFileExceptionBuilder* >
    Builders;
    Builders        m_Builders;
    };
