//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HFCCallbackRegistry.h>
#include <ImagePP/all/h/HFCCallback.h>


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
    uint16_t pi_CallbackInd) const
    {
    HFCCallback*                      pRetCallback;
    CallbacksMultiMap::const_iterator Itr(m_Callbacks.find(pi_CallbackID));

    for (uint16_t Ind = 0; (Ind < pi_CallbackInd) && (Itr != m_Callbacks.end()); Ind++)
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
uint16_t HFCCallbackRegistry::GetNbCallbacks(HCLASS_ID pi_CallbackID) const
    {
    return (uint16_t)m_Callbacks.count(pi_CallbackID);
    }

