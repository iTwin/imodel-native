//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCErrorCodeReceiver.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

class HFCErrorCode;
class HFCErrorCodeModule;
class HFCErrorCodeHandler;

#include "HFCDefaultErrorCodeHandler.h"

class HFCErrorCodeReceiver
    {
public:
    //--------------------------------------
    // Singleton and destructor
    //--------------------------------------

    _HDLLu static HFCErrorCodeReceiver&
    GetInstance();

    ~HFCErrorCodeReceiver();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Adds a module into the receiver
    _HDLLu void AddModule(const HFCErrorCodeModule* pi_pModule);

    // Finds a module based on an error code object.
    const HFCErrorCodeModule*
    FindModule(const HFCErrorCode& pi_rCode) const;

    // default handling behavior.  Can be performed by using modules.
    _HDLLu HSTATUS  Handle(const HFCErrorCode& pi_rCode) const;

    // Changes the default handler
    const HFCErrorCodeHandler*
    SetDefaultHandler(const HFCErrorCodeHandler* pi_pHandler);
    const HFCErrorCodeHandler*
    GetDefaultHandler() const;


private:
    //--------------------------------------
    // Singleton
    //--------------------------------------

    static HAutoPtr<HFCErrorCodeReceiver>
    s_pSingleton;


    //--------------------------------------
    // Construction
    //--------------------------------------

    HFCErrorCodeReceiver();


    //--------------------------------------
    // Types
    //--------------------------------------

    typedef map<uint32_t, const HFCErrorCodeModule*>
    ModuleMap;


    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Map of registered modules
    ModuleMap       m_Modules;

    // Default Handler if not found in the modules
    const HFCErrorCodeHandler* m_pDefaultHandler;
    HFCDefaultErrorCodeHandler m_DefaultHandler;
    };
