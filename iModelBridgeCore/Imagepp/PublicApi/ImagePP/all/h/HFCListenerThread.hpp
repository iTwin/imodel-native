//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCListenerThread.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCListenerThread
//-----------------------------------------------------------------------------
// HFCListenerThread.cpp : source file
//-----------------------------------------------------------------------------

#include  <ImagePP/h/HAutoPtr.h>
#include <ImagePP/win/h/HFCSocket.h>
#include "HFCListenerThread.h"

BEGIN_IMAGEPP_NAMESPACE

static const int32_t s_SleepTime = 100;   // in ms

//-----------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------
template<class T>
HFCListenerThread<T>::HFCListenerThread(unsigned short pi_Port,
                                        HFCThread*  pi_pStarterThread)
    : HFCThread(false),
      m_PortEvent(true, false),
      m_Port(pi_Port),
      m_pStarterThread(pi_pStarterThread)
    {
    HPRECONDITION(pi_pStarterThread != 0);
    }


//-----------------------------------------------------------------
// Public
// Destructor - Stops the thread
//-----------------------------------------------------------------
template<class T>
HFCListenerThread<T>::~HFCListenerThread()
    {
    StopThread();
    WaitUntilSignaled();
    }


//-----------------------------------------------------------------
// Public
// Thread execution code
//-----------------------------------------------------------------
template<class T>
void HFCListenerThread<T>::Go()
    {
    HFCSocket Listener;

    // Create the socket & set in listen mode
    if ((Listener.Create(m_Port) == 0) &&
        (Listener.Listen() == 0))
        {
        // Setup the port
        m_Port = Listener.GetSockPort();
        m_PortEvent.Signal();

        // While we don't receive a kill signal we should loop
        while( CanRun() )
            {
            // Try to accept a connection
            HAutoPtr<HFCSocket> pSocket(Listener.Accept(s_SleepTime));
            if (pSocket.get() != 0)
                m_pStarterThread->Queue(pSocket.release());
            }
        }

    // set the port to 0 and signal it
    m_Port = 0;
    m_PortEvent.Signal();
    }


//-----------------------------------------------------------------
// Public
// Returns the port when it is ready
//-----------------------------------------------------------------
template<class T>
unsigned short HFCListenerThread<T>::GetPort() const
    {
    m_PortEvent.WaitUntilSignaled();
    return (m_Port);
    }

END_IMAGEPP_NAMESPACE