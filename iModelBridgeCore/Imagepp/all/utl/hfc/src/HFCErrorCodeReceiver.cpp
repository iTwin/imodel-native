//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCErrorCodeReceiver.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCErrorCodeReceiver.h>
#include <Imagepp/all/h/HFCErrorCodeModule.h>

//-----------------------------------------------------------------------------
// Static Member Initialization
//-----------------------------------------------------------------------------

HAutoPtr<HFCErrorCodeReceiver> HFCErrorCodeReceiver::s_pSingleton;

//-----------------------------------------------------------------------------
// Public
// Returns the instance of the receiver
//-----------------------------------------------------------------------------
HFCErrorCodeReceiver& HFCErrorCodeReceiver::GetInstance()
    {
    if (s_pSingleton == 0)
        s_pSingleton = new HFCErrorCodeReceiver;

    return (*s_pSingleton);
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HFCErrorCodeReceiver::~HFCErrorCodeReceiver()
    {
    }


//-----------------------------------------------------------------------------
// Public
// Adds a new module in the receiver
//-----------------------------------------------------------------------------
void HFCErrorCodeReceiver::AddModule(const HFCErrorCodeModule* pi_pModule)
    {
    HPRECONDITION(pi_pModule != 0);
    HPRECONDITION(m_Modules.find(pi_pModule->GetID()) == m_Modules.end());

    // Add the module
    m_Modules.insert(ModuleMap::value_type(pi_pModule->GetID(), pi_pModule));
    }


//-----------------------------------------------------------------------------
// Public
// Finds a module that can handle the given error code object
//-----------------------------------------------------------------------------
const HFCErrorCodeModule*
    HFCErrorCodeReceiver::FindModule(const HFCErrorCode& pi_rCode) const
    {
    ModuleMap::const_iterator Itr = m_Modules.find(pi_rCode.GetModuleID());

    if (Itr != m_Modules.end())
        return (*Itr).second;
    else
        return 0;
    }


//-----------------------------------------------------------------------------
// Public
// Default behavior for handling a given error code object
//-----------------------------------------------------------------------------
HSTATUS HFCErrorCodeReceiver::Handle(const HFCErrorCode& pi_rCode) const
    {
    const HFCErrorCodeModule* pModule = FindModule(pi_rCode);

    // if there is a module, find the handler in it
    const HFCErrorCodeHandler* pHandler = 0;
    if (pModule != 0)
        {
        // If there is a handler, use it otherwise take the default handler
        pHandler = pModule->FindHandler(pi_rCode);
        if (pHandler == 0)
            pHandler = GetDefaultHandler();
        }
    else
        pHandler = GetDefaultHandler();

    HASSERT(pHandler != 0);
    return (pHandler->Handle(pi_rCode));
    }


//-----------------------------------------------------------------------------
// Public
// Changes the default error code handler.  Returns the previous handler.
//-----------------------------------------------------------------------------
const HFCErrorCodeHandler*
    HFCErrorCodeReceiver::SetDefaultHandler(const HFCErrorCodeHandler* pi_pHandler)
    {
    HPRECONDITION(pi_pHandler != 0);

    const HFCErrorCodeHandler* pPrevious = GetDefaultHandler();

    m_pDefaultHandler = pi_pHandler;

    return (pPrevious);
    }


//-----------------------------------------------------------------------------
// Public
// Returns the default error code handler.
//-----------------------------------------------------------------------------
const HFCErrorCodeHandler*
    HFCErrorCodeReceiver::GetDefaultHandler() const
    {
    return (m_pDefaultHandler);
    }



//-----------------------------------------------------------------------------
// private
// Constructor
//-----------------------------------------------------------------------------
HFCErrorCodeReceiver::HFCErrorCodeReceiver()
    {
    m_pDefaultHandler = &m_DefaultHandler;
    }
