//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSGroupeableConnectionPool.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSGroupeableConnectionPool
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSGroupeableConnectionPool.h>
#include <Imagepp/all/h/HCSConnectionGroup.h>
#include <Imagepp/all/h/HFCInternetConnection.h>


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSGroupeableConnectionPool::HCSGroupeableConnectionPool(HCSConnectionGroup* pi_pConnectionGroup)
    {
    HPRECONDITION(pi_pConnectionGroup != 0);

    m_pConnections = pi_pConnectionGroup;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSGroupeableConnectionPool::HCSGroupeableConnectionPool(HCSConnectionGroup* pi_pConnectionGroup,
                                                         time_t              pi_ExpirationTime,
                                                         uint32_t            pi_ExpirationInterval)
    : HCSConnectionPool(pi_ExpirationTime, pi_ExpirationInterval)
    {
    HPRECONDITION(pi_pConnectionGroup != 0);

    m_pConnections = pi_pConnectionGroup;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSGroupeableConnectionPool::~HCSGroupeableConnectionPool()
    {
    // Stop the expiration thread before we get in the parent destructor
    if (GetHandle() != NULL)
        {
        StopThread();
        WaitUntilSignaled();
        }
    }


//-----------------------------------------------------------------------------
// Public
// adds a connection to the pool
//-----------------------------------------------------------------------------
void HCSGroupeableConnectionPool::AddConnection(HFCInternetConnection* pi_pConnection)
    {
    HPRECONDITION(pi_pConnection->IsCompatibleWith(HFCInternetConnection::CLASS_ID));
    HPRECONDITION(pi_pConnection->IsConnected());

    // add the connection to the parent list
        {
        HFCMonitor Monitor(m_Key);
        HCSConnectionPool::AddConnection(pi_pConnection);
        }

    // add a connection to the connection group
    m_pConnections->AddConnection(pi_pConnection);
    }


//-----------------------------------------------------------------------------
// Public
// removes a connection from the pool
//-----------------------------------------------------------------------------
void HCSGroupeableConnectionPool::RemoveConnection(HFCInternetConnection* pi_pConnection)
    {
    HPRECONDITION(pi_pConnection->IsCompatibleWith(HFCInternetConnection::CLASS_ID));

    // Remove the connection from the connection group
    m_pConnections->RemoveConnection(pi_pConnection);

    // Remove the conneciton from the the parent list
        {
        HFCMonitor Monitor(m_Key);
        HCSConnectionPool::RemoveConnection(pi_pConnection);
        }
    }



//-----------------------------------------------------------------------------
// Public
// Get the first connection in the pool, whether or not the connection is
// active.  The returned connection is given to the caller.  The caller
// has the responsability of replacing it in the pool if needed.
//-----------------------------------------------------------------------------
HFCInternetConnection* HCSGroupeableConnectionPool::GetConnection(uint32_t pi_TimeOut)
    {
    HFCInternetConnection* pResult = 0;

    HFCMonitor Monitor(m_Key);
    if (m_Connections.size() > 0)
        {
        // get the first connection & remove it from the pool
        Connections::iterator Itr = m_Connections.begin();
        pResult = (*Itr).first;
        Itr = m_Connections.erase(Itr);

        // also remove from the connection group
        Monitor.ReleaseKey();
        m_pConnections->RemoveConnection(pResult);
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
//-----------------------------------------------------------------------------
HFCInternetConnection* HCSGroupeableConnectionPool::GetActiveConnection(uint32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut >= 0);
    HFCInternetConnection* pResult = 0;

    // get the first available active connection from the connection group
    HFCInternetConnection* pActive = m_pConnections->CheckActivity(pi_TimeOut);
    if (pActive != 0)
        {
        try
            {
            // The connection may have been removed by the expiration thread, so
            // find the connections in the in the map before
            HFCMonitor Monitor(m_Key);
            Connections::iterator Itr = m_Connections.find(pActive);
            if (Itr != m_Connections.end())
                {
                // remove it from the pool before giving it to the caller
                m_Connections.erase(Itr);
                Monitor.ReleaseKey();

                // do a little operation to check the state of the connection
                pActive->WaitDataAvailable(0);

                // Okely dokely
                pResult = pActive;
                }
            }
        catch(...)
            {
            HASSERT(!pActive->IsConnected());
            HCSConnectionPool::RemoveConnection(pActive);
            }
        }

    return (pResult);
    }
