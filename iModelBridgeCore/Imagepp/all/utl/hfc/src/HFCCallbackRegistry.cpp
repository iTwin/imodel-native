//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCCallbackRegistry.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCCallbackRegistry.h>
#include <Imagepp/all/h/HFCCallback.h>


// Singleton
HFC_IMPLEMENT_SINGLETON(HFCCallbackRegistry)

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HFCCallbackRegistry::HFCCallbackRegistry()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HFCCallbackRegistry::~HFCCallbackRegistry()
    {

    }

//-----------------------------------------------------------------------------
// Public
// AddCallback
//-----------------------------------------------------------------------------
bool HFCCallbackRegistry::AddCallback(const HFCCallback* pi_pCallback)
    {
    HPRECONDITION(pi_pCallback != 0);

    m_Callbacks.insert(CallbacksMultiMap::value_type(pi_pCallback->GetClassID(),
        const_cast<HFCCallback*>(pi_pCallback)));

    return true;
    }

//-----------------------------------------------------------------------------
// Public
// RemoveCallback
//-----------------------------------------------------------------------------
void HFCCallbackRegistry::RemoveCallback(const HFCCallback* pi_pCallback)
    {
    HPRECONDITION(pi_pCallback != 0);
    HPRECONDITION(m_Callbacks.find(pi_pCallback->GetClassID()) != m_Callbacks.end());

    CallbacksMultiMap::iterator CallbackIter = m_Callbacks.find(pi_pCallback->GetClassID());

    while (CallbackIter != m_Callbacks.end())
        {
        if (CallbackIter->second == pi_pCallback)
            {
            break;
            }

        CallbackIter++;
        }

    HASSERT(CallbackIter != m_Callbacks.end());

    m_Callbacks.erase(CallbackIter);
    }

//-----------------------------------------------------------------------------
// Public
// GetCallback
//-----------------------------------------------------------------------------
HFCCallback* HFCCallbackRegistry::GetCallback(HCLASS_ID pi_CallbackID,
    unsigned short pi_CallbackInd) const
    {
    HFCCallback*                      pRetCallback;
    CallbacksMultiMap::const_iterator Itr(m_Callbacks.find(pi_CallbackID));

    for (unsigned short Ind = 0; (Ind < pi_CallbackInd) && (Itr != m_Callbacks.end()); Ind++)
        {
        Itr++;
        }

    if (Itr == m_Callbacks.end())
        {
        pRetCallback = 0;
        }
    else
        {
        pRetCallback = Itr->second;
        }

    return pRetCallback;
    }

//-----------------------------------------------------------------------------
// Public
// GetCallback
//-----------------------------------------------------------------------------
unsigned short HFCCallbackRegistry::GetNbCallbacks(HCLASS_ID pi_CallbackID) const
    {
    return (unsigned short)m_Callbacks.count(pi_CallbackID);
    }

