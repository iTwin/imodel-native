//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCErrorCodeGenerator.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCErrorCodeGenerator.h>
#include <Imagepp/all/h/HFCErrorCodeBuilder.h>
#include <Imagepp/all/h/HFCErrorCodeModule.h>
#include <Imagepp/all/h/HFCException.h>

//-----------------------------------------------------------------------------
// Static Member Initialization
//-----------------------------------------------------------------------------

HAutoPtr<HFCErrorCodeGenerator> HFCErrorCodeGenerator::s_pSingleton;




//-----------------------------------------------------------------------------
// Public
// Returns the instance of the receiver
//-----------------------------------------------------------------------------
HFCErrorCodeGenerator& HFCErrorCodeGenerator::GetInstance()
    {
    if (s_pSingleton == 0)
        s_pSingleton = new HFCErrorCodeGenerator;

    return (*s_pSingleton);
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HFCErrorCodeGenerator::~HFCErrorCodeGenerator()
    {
    }


//-----------------------------------------------------------------------------
// Public
// Adds a new module in the receiver
//-----------------------------------------------------------------------------
void HFCErrorCodeGenerator::AddModule(const HFCErrorCodeModule* pi_pModule)
    {
    HPRECONDITION(pi_pModule != 0);
    HPRECONDITION(m_Modules.find(pi_pModule->GetID()) == m_Modules.end());

    // Add the module
    m_Modules.insert(ModuleMap::value_type(pi_pModule->GetID(), pi_pModule));

    // the modules for exceptions
    for (HFCErrorCodeModule::ExceptionBuilderMap::const_iterator ExItr = pi_pModule->GetBuildersForException().begin();
        ExItr != pi_pModule->GetBuildersForException().end();
        ++ExItr)
        m_ExceptionModules.insert(ExceptionModuleMap::value_type((*ExItr).first, pi_pModule));

    // the modules for exceptions
    for (HFCErrorCodeModule::OSExceptionBuilderMap::const_iterator OSExItr = pi_pModule->GetBuildersForOSException().begin();
        OSExItr != pi_pModule->GetBuildersForOSException().end();
        ++OSExItr)
        m_OSExceptionModules.insert(OSExceptionModuleMap::value_type((*OSExItr).first, pi_pModule));

    // the modules for exceptions
    for (HFCErrorCodeModule::StatusBuilderMap::const_iterator StatusItr = pi_pModule->GetBuildersForStatus().begin();
        StatusItr != pi_pModule->GetBuildersForStatus().end();
        ++StatusItr)
        m_StatusModules.insert(StatusModuleMap::value_type((*StatusItr).first, pi_pModule));
    }


//-----------------------------------------------------------------------------
// Public
// Finds a module that can handle the given error code object
//-----------------------------------------------------------------------------
const HFCErrorCodeModule*
    HFCErrorCodeGenerator::FindModule(const HFCErrorCodeID& pi_rID) const
    {
    ModuleMap::const_iterator Itr = m_Modules.find(pi_rID);

    if (Itr != m_Modules.end())
        return (*Itr).second;
    else
        return 0;
    }


//-----------------------------------------------------------------------------
// Public
// Finds the first module that can handle the exception
//-----------------------------------------------------------------------------
const HFCErrorCodeModule*
    HFCErrorCodeGenerator::FindModuleByException(const HFCException& pi_rException) const
    {
    ExceptionModuleMap::const_iterator Itr = m_ExceptionModules.find(pi_rException.GetID());

    if (Itr != m_ExceptionModules.end())
        return (*Itr).second;
    else
        return 0;
    }


//-----------------------------------------------------------------------------
// Public
// Finds the first module that can handle the exception
//-----------------------------------------------------------------------------
const HFCErrorCodeModule*
    HFCErrorCodeGenerator::FindModuleByOSException(uint32_t pi_Exception) const
    {
    OSExceptionModuleMap::const_iterator Itr = m_OSExceptionModules.find(pi_Exception);

    if (Itr != m_OSExceptionModules.end())
        return (*Itr).second;
    else
        return 0;
    }


//-----------------------------------------------------------------------------
// Public
// Finds the first module that can handle the status
//-----------------------------------------------------------------------------
const HFCErrorCodeModule*
    HFCErrorCodeGenerator::FindModuleByStatus(HSTATUS pi_Status) const
    {
    StatusModuleMap::const_iterator Itr = m_StatusModules.find(pi_Status);

    if (Itr != m_StatusModules.end())
        return (*Itr).second;
    else
        return 0;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode*
    HFCErrorCodeGenerator::GetErrorCodeForException(const HFCException& pi_rException) const
    {
    HFCErrorCode* pResult = 0;

    // find the first module that can handle the exception
    ExceptionModuleMap::const_iterator Itr = m_ExceptionModules.find(pi_rException.GetID());
    if (Itr != m_ExceptionModules.end())
        {
        const HFCErrorCodeBuilder* pBuilder = (*Itr).second->FindBuilderForException(pi_rException);
        if (pBuilder != 0)
            pResult = pBuilder->BuildFromException(pi_rException);
        }

    // if not error code was found for this exception, parse the map backwards to find the
    // first compatible (ancestor) exception, if any.
    if (pResult == 0)
        {
        for (ModuleMap::const_reverse_iterator RItr = GetModules().rbegin();
            (pResult == 0) && (RItr != GetModules().rend());
            ++RItr)
            {
            // parse the exceptions in the current module
            for (HFCErrorCodeModule::ExceptionBuilderMap::const_reverse_iterator RItr2 = (*RItr).second->GetBuildersForException().rbegin();
                (pResult == 0) && (RItr2 != (*RItr).second->GetBuildersForException().rend());
                ++RItr2)
                {
                // if the exception is compatible with the current exception class ID, use it.
                if (pi_rException.GetID() == (*RItr2).first)
                    pResult = (*RItr2).second->BuildFromException(pi_rException);
                }
            }
        }

    return (pResult);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode*
    HFCErrorCodeGenerator::GetErrorCodeForException(const HFCException&   pi_rException,
    const HFCErrorCodeID& pi_rModuleID) const
    {
    HFCErrorCode* pResult = 0;

    // Get the module
    ModuleMap::const_iterator Itr = m_Modules.find(pi_rModuleID);
    if (Itr != m_Modules.end())
        {
        const HFCErrorCodeBuilder* pBuilder = (*Itr).second->FindBuilderForException(pi_rException);
        if (pBuilder != 0)
            pResult = pBuilder->BuildFromException(pi_rException);
        }

    return (pResult);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode*
    HFCErrorCodeGenerator::GetErrorCodeForOSException(uint32_t pi_Exception) const
    {
    HFCErrorCode* pResult = 0;

    // find the first module that can handle the exception
    OSExceptionModuleMap::const_iterator Itr = m_OSExceptionModules.find(pi_Exception);
    if (Itr != m_OSExceptionModules.end())
        {
        const HFCErrorCodeBuilder* pBuilder = (*Itr).second->FindBuilderForOSException(pi_Exception);
        if (pBuilder != 0)
            pResult = pBuilder->BuildFromOSException(pi_Exception);
        }

    return (pResult);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode*
    HFCErrorCodeGenerator::GetErrorCodeForOSException(uint32_t             pi_Exception,
    const HFCErrorCodeID& pi_rModuleID) const
    {
    HFCErrorCode* pResult = 0;

    // Get the module
    ModuleMap::const_iterator Itr = m_Modules.find(pi_rModuleID);
    if (Itr != m_Modules.end())
        {
        const HFCErrorCodeBuilder* pBuilder = (*Itr).second->FindBuilderForOSException(pi_Exception);
        if (pBuilder != 0)
            pResult = pBuilder->BuildFromOSException(pi_Exception);
        }

    return (pResult);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode*
    HFCErrorCodeGenerator::GetErrorCodeForStatus(HSTATUS pi_Status) const
    {
    HFCErrorCode* pResult = 0;

    // find the first module that can handle the exception
    StatusModuleMap::const_iterator Itr = m_StatusModules.find(pi_Status);
    if (Itr != m_StatusModules.end())
        {
        const HFCErrorCodeBuilder* pBuilder = (*Itr).second->FindBuilderForStatus(pi_Status);
        if (pBuilder != 0)
            pResult = pBuilder->BuildFromStatus(pi_Status);
        }

    return (pResult);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode*
    HFCErrorCodeGenerator::GetErrorCodeForStatus(HSTATUS               pi_Status,
    const HFCErrorCodeID& pi_rModuleID) const
    {
    HFCErrorCode* pResult = 0;

    // Get the module
    ModuleMap::const_iterator Itr = m_Modules.find(pi_rModuleID);
    if (Itr != m_Modules.end())
        {
        const HFCErrorCodeBuilder* pBuilder = (*Itr).second->FindBuilderForStatus(pi_Status);
        if (pBuilder != 0)
            pResult = pBuilder->BuildFromStatus(pi_Status);
        }

    return (pResult);
    }


//-----------------------------------------------------------------------------
// private
// Constructor
//-----------------------------------------------------------------------------
HFCErrorCodeGenerator::HFCErrorCodeGenerator()
    {
    }


//-----------------------------------------------------------------------------
// private
// Constructor
//-----------------------------------------------------------------------------
const HFCErrorCodeGenerator::ModuleMap&
    HFCErrorCodeGenerator::GetModules() const
    {
    return m_Modules;
    }


//-----------------------------------------------------------------------------
// private
// Constructor
//-----------------------------------------------------------------------------
const HFCErrorCodeGenerator::ExceptionModuleMap&
    HFCErrorCodeGenerator::GetModulesForException() const
    {
    return m_ExceptionModules;
    }


//-----------------------------------------------------------------------------
// private
// Constructor
//-----------------------------------------------------------------------------
const HFCErrorCodeGenerator::OSExceptionModuleMap&
    HFCErrorCodeGenerator::GetModulesForOSException() const
    {
    return m_OSExceptionModules;
    }


//-----------------------------------------------------------------------------
// private
// Constructor
//-----------------------------------------------------------------------------
const HFCErrorCodeGenerator::StatusModuleMap&
    HFCErrorCodeGenerator::GetModulesForStatus() const
    {
    return m_StatusModules;
    }
