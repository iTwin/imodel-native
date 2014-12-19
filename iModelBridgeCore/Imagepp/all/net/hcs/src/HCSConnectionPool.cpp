//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSConnectionPool.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class :
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSConnectionPool.h>
#include <Imagepp/all/h/HFCConnection.h>
#include <Imagepp/all/h/HFCInternetConnection.h>
#include <Imagepp/all/h/HFCThread.h>


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSConnectionPool::HCSConnectionPool()
    : m_ExpirationEvent(false, false)
    {
    // make the expiration very long, just to initialize.
    m_ExpirationTime     = INT_MAX;
    m_ExpirationInterval = INT_MAX;

    // Do not start the verification thread
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSConnectionPool::HCSConnectionPool(time_t pi_ExpirationTime, uint32_t pi_ExpirationInterval)
    : m_ExpirationEvent(false, false)
    {
    HPRECONDITION(pi_ExpirationTime > 0);
    HPRECONDITION(pi_ExpirationTime < INT_MAX);
    HPRECONDITION(pi_ExpirationInterval > 0);
    HPRECONDITION(pi_ExpirationInterval < INT_MAX);

    // copy the given maximum expiration
    m_ExpirationTime     = pi_ExpirationTime;
    m_ExpirationInterval = pi_ExpirationInterval;

    // Start the expiration verification thread
    StartThread();
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSConnectionPool::~HCSConnectionPool()
    {
    // Stop the expiration thread
    if (GetHandle() != NULL)
        {
        StopThread();
        WaitUntilSignaled();
        }

    // kill the actual connections
    for (Connections::iterator Itr = m_Connections.begin(); Itr != m_Connections.end(); ++Itr)
        delete (*Itr).first;
    }


//-----------------------------------------------------------------------------
// Public
// adds a connection to the pool
//-----------------------------------------------------------------------------
void HCSConnectionPool::AddConnection(HFCInternetConnection* pi_pConnection)
    {
    HFCMonitor Monitor(m_Key);

    m_Connections.insert(Connections::value_type(pi_pConnection, time(NULL)));
    }


//-----------------------------------------------------------------------------
// Public
// removes a connection from the pool
//-----------------------------------------------------------------------------
void HCSConnectionPool::RemoveConnection(HFCInternetConnection* pi_pConnection)
    {
    HFCMonitor Monitor(m_Key);

    // find the connection in the list
    Connections::iterator Itr = m_Connections.find(pi_pConnection);
    if (Itr != m_Connections.end())
        {
        delete (*Itr).first;
        m_Connections.erase(Itr);
        }
    }


//-----------------------------------------------------------------------------
// Public
// Get the first connection in the pool, whether or not the connection is
// active.  The returned connection is given to the caller.  The caller
// has the responsability of replacing it in the pool if needed.
//-----------------------------------------------------------------------------
HFCInternetConnection* HCSConnectionPool::GetConnection(uint32_t pi_TimeOut)
    {
    HFCInternetConnection* pResult = 0;

    HFCMonitor Monitor(m_Key);
    if (m_Connections.size() > 0)
        {
        // get the first connection & remove it from the pool
        Connections::iterator Itr = m_Connections.begin();
        pResult = (*Itr).first;
        Itr = m_Connections.erase(Itr);
        }
    Monitor.ReleaseKey();

    if (pResult == 0)
        HFCThread::Sleep(pi_TimeOut);

    return (pResult);
    }


//-----------------------------------------------------------------------------
// Public
// returns the first connections that has activity.  The activity
// method removes disconnected connections when they are detected.
// The returned connection is given to the caller.  The caller
// has the responsability of replacing it in the pool if needed.
//-----------------------------------------------------------------------------
HFCInternetConnection* HCSConnectionPool::GetActiveConnection(uint32_t pi_TimeOut)
    {
    HFCMonitor Monitor(m_Key);
    HFCInternetConnection* pResult = 0;

    Connections::iterator Itr = m_Connections.begin();
    while ((pResult == 0) && (Itr != m_Connections.end()))
        {
        try
            {
            // verify if there is something on the connection
            if ((*Itr).first->IsConnected())
                {
                if ((*Itr).first->WaitDataAvailable(0))
                    {
                    pResult = (*Itr).first;
                    Itr = m_Connections.erase(Itr);
                    }

                // proceed to the next iteration
                else
                    ++Itr;
                }

            else
                {
                delete (*Itr).first;
                Itr = m_Connections.erase(Itr);
                }
            }
        catch(...)
            {
            // Disconnected, remove the connection from the pool
            HASSERT(!(*Itr).first->IsConnected());
            delete (*Itr).first;
            Itr = m_Connections.erase(Itr);
            }
        }
    Monitor.ReleaseKey();

    if (pResult == 0)
        HFCThread::Sleep(pi_TimeOut);

    return (pResult);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
time_t HCSConnectionPool::GetExpirationTime() const
    {
    HFCMonitor Monitor(m_Key);
    return m_ExpirationTime;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HCSConnectionPool::SetExpirationTime(time_t pi_Time)
    {
    HPRECONDITION(pi_Time > 0);
    HPRECONDITION(pi_Time < INT_MAX);

    HFCMonitor Monitor(m_Key);
    m_ExpirationTime = pi_Time;

    // if both the expiration interval and time are not INT_MAX, start the thread
    if ((GetHandle() == NULL) &&
        (m_ExpirationTime != INT_MAX) &&
        (m_ExpirationInterval != INT_MAX))
        StartThread();
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
uint32_t HCSConnectionPool::GetExpirationInterval() const
    {
    HFCMonitor Monitor(m_Key);
    return m_ExpirationInterval;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HCSConnectionPool::SetExpirationInterval(uint32_t pi_Interval)
    {
    HPRECONDITION(pi_Interval > 0);
    HPRECONDITION(pi_Interval < INT_MAX);

    HFCMonitor Monitor(m_Key);
    m_ExpirationTime = pi_Interval;

    // if both the expiration interval and time are not INT_MAX, start the thread
    if ((GetHandle() == NULL) &&
        (m_ExpirationTime != INT_MAX) &&
        (m_ExpirationInterval != INT_MAX))
        StartThread();
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HCSConnectionPool::DoExpiration()
    {
    m_ExpirationEvent.Signal();
    }


// Get information about the connections in the pool
//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
size_t HCSConnectionPool::GetConnectionCount() const
    {
    HFCMonitor Monitor(m_Key);
    size_t Result = m_Connections.size();
    Monitor.ReleaseKey();

    return (Result);
    }



//-----------------------------------------------------------------------------
// Public
// Expiration thread
//-----------------------------------------------------------------------------
void HCSConnectionPool::Go()
    {
    // prepare for a multiple object event.
    HFCSynchroContainer Events;
    Events.AddSynchro(&m_StopEvent);
    Events.AddSynchro(&m_ExpirationEvent);

    uint32_t Interval;
        {
        HFCMonitor Monitor(m_Key);
        Interval = m_ExpirationInterval * 1000;
        }

    while (HFCSynchro::WaitForMultipleObjects(Events, Interval, false) != 0)
        {
        HFCMonitor Monitor(m_Key);

        // compute the current expiration time
        time_t Now = time(NULL);

        // parse all the entries in the ppol
        Connections::iterator Itr = m_Connections.begin();
        while (CanRun() && (Itr != m_Connections.end()))
            {
            if ((*Itr).second + m_ExpirationTime < Now)
                {
                // Take the current iterator and move to the next element
                Connections::iterator CurrentItr = Itr;
                ++Itr;

                // delete the connections in the current iterator
                RemoveConnection((*CurrentItr).first);
                }
            else
                ++Itr;
            }

        // recompute the interval before sleeping
        Interval = m_ExpirationInterval * 1000;
        }
    }
