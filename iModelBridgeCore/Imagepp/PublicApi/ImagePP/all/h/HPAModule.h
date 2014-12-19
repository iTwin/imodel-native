//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPAModule.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCErrorCodeModule.h"
#include "HPAModuleCodes.h"

class HPAModule : public HFCErrorCodeModule
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    _HDLLu                 HPAModule(const HFCErrorCodeID& pi_rID);
    _HDLLu virtual         ~HPAModule() = 0;   // so that it must be implemented by a dependent library


protected:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Holds the builders/handlers
    HPAExceptionBuilder m_Builder;
    };
