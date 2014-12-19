//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCTimer.cpp $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCTimer
//----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCTimer.h>

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
HFCTimerListener::~HFCTimerListener()
    {
    // Nothing to do!
    }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
HFCTimer::~HFCTimer()
    {
    // Deleting listeners still in list

    ListenerList::iterator itr = m_Listeners.begin();
    while (itr != m_Listeners.end())
        {
        delete (*itr);
        ++itr;
        }

    // Stopping timer if still active
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    if (m_TimerID != 0)
        Stop();
#endif
    }


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void HFCTimer::Trigger()
    {
    m_Key.ClaimKey();
    ListenerList::iterator itr = m_Listeners.begin();
    while (itr != m_Listeners.end())
        {
        (*itr)->Trigger(this);
        ++itr;
        }
    m_Key.ReleaseKey();
    }


#if defined (ANDROID) || defined (__APPLE__)
  //DM-Android
#elif defined (_WIN32)
//-----------------------------------------------------------------------------
// The Windows Code begin here
// This callback function was called by the windows timer
//-----------------------------------------------------------------------------
void CALLBACK HFCTimer::TimerCallback(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
    {
    ((HFCTimer*)dwUser)->TriggerInternal();
    }
#endif