//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCDefaultErrorCodeHandler.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCErrorCodeHandler.h"

class HFCDefaultErrorCodeHandler : public HFCErrorCodeHandler
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HFCDefaultErrorCodeHandler();
    virtual         ~HFCDefaultErrorCodeHandler();


    //--------------------------------------
    // Handling method
    //--------------------------------------

    _HDLLu virtual HSTATUS Handle(const HFCErrorCode& pi_rCode) const;
    };

