//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPSModule.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HPAModule.h"

class HPSModule : public HPAModule
    {
public:
    //--------------------------------------
    // Constants
    //--------------------------------------

    // This value represents the ID for the HPS module.
    static const uint32_t s_ModuleID;


    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    _HDLLg                HPSModule();
    _HDLLg virtual         ~HPSModule();
    };
