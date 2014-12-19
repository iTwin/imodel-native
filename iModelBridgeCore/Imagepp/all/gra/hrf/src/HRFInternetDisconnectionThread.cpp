//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetDisconnectionThread.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetDisconnectionThread
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetDisconnectionThread.h>
#include <Imagepp/all/h/HFCInternetConnection.h>
#include <Imagepp/all/h/HFCMonitor.h>


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HRFInternetDisconnectionThread::HRFInternetDisconnectionThread()
    : HFCThread(false),
      m_ListEvent(true, false)
    {
    StartThread();
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HRFInternetDisconnectionThread::~HRFInternetDisconnectionThread()
    {
    // Reset the run event so that the thread may end
    StopThread();

    // Wait until it actually ends
    WaitUntilSignaled();
    }


//-----------------------------------------------------------------------------
// Public
// Add a connection to be closed
//-----------------------------------------------------------------------------
void HRFInternetDisconnectionThread::AddConnection(HFCInternetConnection* pi_pConnection)
    {
    if (pi_pConnection != 0)
        {
        HFCMonitor Monitor(m_ListMonitor);

        m_List.push_back(pi_pConnection);
        m_ListEvent.Signal();
        }
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetDisconnectionThread::Go()
    {
    // prepare for multiple wait
    HFCSynchroContainer SynchroList;
    SynchroList.AddSynchro(&m_ListEvent);
    SynchroList.AddSynchro(&m_StopEvent);

    // While there is connections to close and the event wants the
    // thread to run, loop
    while (HFCSynchro::WaitForMultipleObjects(SynchroList, false) == 0)
        {
        // Take the first connection in the list
        HFCInternetConnection* pConnection = 0;
            {
            HFCMonitor Monitor(m_ListMonitor);

            // Take the first element and remove it from the list
            pConnection = m_List.front();
            m_List.pop_front();

            // if the list is empty, reset the list event
            if (m_List.empty())
                m_ListEvent.Reset();
            }

        // close the connection
        delete pConnection;
        }

    // delete any remaining connection
    HFCMonitor Monitor(m_ListMonitor);
    while (m_List.size() > 0)
        {
        delete m_List.front();
        m_List.pop_front();
        }
    }
