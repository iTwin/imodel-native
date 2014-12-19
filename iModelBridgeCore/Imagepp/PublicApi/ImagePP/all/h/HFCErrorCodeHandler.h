//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCErrorCodeHandler.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HFCErrorCode.h"

class HFCErrorCodeHandler
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HFCErrorCodeHandler();
    virtual         ~HFCErrorCodeHandler();


    //--------------------------------------
    // Handling method
    //--------------------------------------

    virtual HSTATUS Handle(const HFCErrorCode& pi_rCode) const = 0;
    };

#include "HFCErrorCodeHandler.hpp"

