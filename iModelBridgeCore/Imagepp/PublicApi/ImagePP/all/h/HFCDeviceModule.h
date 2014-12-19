//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCDeviceModule.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCErrorCodeModule.h"

class HFCDeviceExceptionBuilder;
class HFCDeviceModule : public HFCErrorCodeModule
    {
public:
    //--------------------------------------
    // Constants
    //--------------------------------------

    // This value represents the ID for the device module.
    static const uint32_t s_ModuleID;


    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    _HDLLu                 HFCDeviceModule();
    _HDLLu virtual         ~HFCDeviceModule();


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Holds the builders/handlers
    typedef list<HFCDeviceExceptionBuilder* >
    Builders;
    Builders        m_Builders;
    };
