//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCGeneralModule.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCErrorCodeModule.h"
#include "HFCGeneralModuleCodes.h"

class HFCGeneralModule : public HFCErrorCodeModule
    {
public:
    //--------------------------------------
    // Constants
    //--------------------------------------

    // This value represents the ID for the general module.
    static const uint32_t s_ModuleID;


    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    _HDLLu                 HFCGeneralModule();
    _HDLLu virtual         ~HFCGeneralModule();


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    HFCExceptionBuilder
    m_ExceptionBuilder;
    HFCObjectNotInFactoryExceptionBuilder
    m_FactoryBuilder;
    };
