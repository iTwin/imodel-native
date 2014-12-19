//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCErrorCodeGenerator.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

class HFCErrorCode;
class HFCErrorCodeModule;
class HFCErrorCodeHandler;
class HFCException;

#include "HFCErrorCodeID.h"

class HFCErrorCodeGenerator
    {
public:
    //--------------------------------------
    // Types
    //--------------------------------------

    typedef map<uint32_t, const HFCErrorCodeModule*>
    ModuleMap;
    typedef map<HCLASS_ID, const HFCErrorCodeModule*>
    ExceptionModuleMap;
    typedef map<uint32_t, const HFCErrorCodeModule*>
    OSExceptionModuleMap;
    typedef map<HSTATUS, const HFCErrorCodeModule*>
    StatusModuleMap;


    //--------------------------------------
    // Singleton and destructor
    //--------------------------------------

    _HDLLu static HFCErrorCodeGenerator&
    GetInstance();

    ~HFCErrorCodeGenerator();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Adds a module into the receiver
    _HDLLu void     AddModule(const HFCErrorCodeModule* pi_pModule);

    // Finds a module based on an a module ID
    const HFCErrorCodeModule*
    FindModule(const HFCErrorCodeID& pi_rID) const;

    // Finds a module based on the type of error
    const HFCErrorCodeModule*
    FindModuleByException  (const HFCException& pi_rException) const;
    const HFCErrorCodeModule*
    FindModuleByOSException(uint32_t           pi_Exception) const;
    const HFCErrorCodeModule*
    FindModuleByStatus     (HSTATUS             pi_Status) const;

    // Returns an error code based on the type of error
    HFCErrorCode*   GetErrorCodeForException  (const HFCException&   pi_rException) const;
    HFCErrorCode*   GetErrorCodeForException  (const HFCException&   pi_rException,
                                               const HFCErrorCodeID& pi_rModuleID) const;
    HFCErrorCode*   GetErrorCodeForOSException(uint32_t             pi_Exception) const;
    HFCErrorCode*   GetErrorCodeForOSException(uint32_t             pi_Exception,
                                               const HFCErrorCodeID& pi_rModuleID) const;
    HFCErrorCode*   GetErrorCodeForStatus     (HSTATUS               pi_Status) const;
    HFCErrorCode*   GetErrorCodeForStatus     (HSTATUS               pi_Status,
                                               const HFCErrorCodeID& pi_rModuleID) const;


    // Returns all the Modules for a particular type of error
    const ModuleMap&
    GetModules() const;
    const ExceptionModuleMap&
    GetModulesForException() const;
    const OSExceptionModuleMap&
    GetModulesForOSException() const;
    const StatusModuleMap&
    GetModulesForStatus() const;


private:
    //--------------------------------------
    // Singleton
    //--------------------------------------

    static HAutoPtr<HFCErrorCodeGenerator>
    s_pSingleton;


    //--------------------------------------
    // Construction
    //--------------------------------------

    HFCErrorCodeGenerator();


    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Map of registered modules
    ModuleMap               m_Modules;
    ExceptionModuleMap      m_ExceptionModules;
    OSExceptionModuleMap    m_OSExceptionModules;
    StatusModuleMap         m_StatusModules;

    };