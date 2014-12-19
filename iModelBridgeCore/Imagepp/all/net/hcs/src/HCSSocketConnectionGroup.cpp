//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSSocketConnectionGroup.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSSocketConnectionGroup
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSSocketConnectionGroup.h>
#include <Imagepp/all/h/HCSSocketConnection.h>
#include <Imagepp/all/h/HFCThread.h>


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCSSocketConnectionGroup::HCSSocketConnectionGroup()
    {
#ifdef _WIN32
    // Initialize the socket sets
    memset(&m_SocketSet, 0, sizeof(fd_set));
    FD_ZERO(&m_SocketSet);
#endif
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCSSocketConnectionGroup::~HCSSocketConnectionGroup()
    {
    }


//-----------------------------------------------------------------------------
// public
// Add a socket connection to the group
//-----------------------------------------------------------------------------
void HCSSocketConnectionGroup::AddConnection(HFCInternetConnection* pi_pConnection)
    {
    HPRECONDITION(pi_pConnection->IsCompatibleWith(HCSSocketConnection::CLASS_ID));
    HFCMonitor Monitor(m_Key);

#ifdef _WIN32
    HCSSocketConnection* pConnection = static_cast<HCSSocketConnection*>(pi_pConnection);

    // add the connection to the map of connections
    HASSERT(pConnection->m_hSocket != NULL);
    HPRECONDITION(m_ConnectionMap.find(pConnection->m_hSocket) == m_ConnectionMap.end());
    m_ConnectionMap.insert(ConnectionMap::value_type(pConnection->m_hSocket, pConnection));

    // add the conenction to the socket sets
    if (m_SocketSet.fd_count < FD_SETSIZE)
        {
        FD_SET(pConnection->m_hSocket, &m_SocketSet);
        // signal to other thread that there is something available in the socket set
        m_SocketSetSemaphore.Signal();
        }
    else
        m_WaitingSockets.push_back(pConnection);
#endif
    }


//-----------------------------------------------------------------------------
// public
// Removes a socket connection to the group
//-----------------------------------------------------------------------------
void HCSSocketConnectionGroup::RemoveConnection(HFCInternetConnection* pi_pConnection)
    {
    HPRECONDITION(pi_pConnection->IsCompatibleWith(HCSSocketConnection::CLASS_ID));

    // check in the active connection list
    HFCMonitor SelectMonitor(m_ActiveConnectionKey);
    ConnectionList::iterator ActiveItr(find(m_ActiveConnections.begin(),
                                            m_ActiveConnections.end(),
                                            pi_pConnection));
    if (ActiveItr != m_ActiveConnections.end())
        {
        m_ActiveConnections.erase(ActiveItr);
        }

    HFCMonitor Monitor(m_Key);

#ifdef _WIN32
    HCSSocketConnection* pConnection = static_cast<HCSSocketConnection*>(pi_pConnection);

    // Remove the connection to the map of connections
    HASSERT(pConnection->m_hSocket != NULL);
    ConnectionMap::iterator Itr = m_ConnectionMap.find(pConnection->m_hSocket);
    if (Itr != m_ConnectionMap.end())
        {
        // remove the connection to the socket sets
        if (FD_ISSET(pConnection->m_hSocket, &m_SocketSet))
            FD_CLR(pConnection->m_hSocket, &m_SocketSet);
        else
            m_WaitingSockets.remove(pConnection);

        // remove from the map
        m_ConnectionMap.erase(Itr);

        // if there are entries in the set waiting list, place them in the set
        while ((m_WaitingSockets.size() > 0) && (m_SocketSet.fd_count < FD_SETSIZE))
            {
            HPRECONDITION((*m_WaitingSockets.begin())->m_hSocket != NULL);
            FD_SET((*m_WaitingSockets.begin())->m_hSocket, &m_SocketSet);
            m_WaitingSockets.pop_front();
            // signal to other thread that there is something available in the socket set
            m_SocketSetSemaphore.Signal();
            }
        }
#endif
    }


//-----------------------------------------------------------------------------
// public
// Checks for activity on a group of socket.
//-----------------------------------------------------------------------------
HFCInternetConnection* HCSSocketConnectionGroup::CheckActivity(uint32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut >= 0);
    HFCInternetConnection* pResult = 0;

        {
        // check in the active connection list
        HFCMonitor SelectMonitor(m_ActiveConnectionKey);
        if (m_ActiveConnections.size() > 0)
            {
            pResult = *m_ActiveConnections.begin();
            m_ActiveConnections.pop_front();
            }
        }

    if (pResult == 0)
        {
        // Wait for pi_TimeOut unless something is added in m_SocketSet meanwhile.
        m_SocketSetSemaphore.WaitUntilSignaled(pi_TimeOut);

        // copy the socket set, because select removes sockets that do not
        // have readable activity.
        fd_set ReadableSet;
            {
            HFCMonitor Monitor(m_Key);
            memcpy(&ReadableSet, &m_SocketSet, sizeof(fd_set));
            }

        if (ReadableSet.fd_count > 0)
            {
            HFCMonitor SelectMonitor(m_ActiveConnectionKey);

            // first verify if a previous select has placed active sockets in the list or not.
            // If not, perform the select function on the socket set
            if (m_ActiveConnections.size() == 0)
                {
                // perform the select operation to check activity on the sockets
                struct timeval TimeOut = {pi_TimeOut / 1000, (pi_TimeOut%1000) * 1000};
                if (select(0, &ReadableSet, NULL, NULL, &TimeOut) > 0)
                    {
                    HFCMonitor Monitor(m_Key);

                    // Parse the sockets until a right one is found
                    for (uint32_t Index = 0; Index < ReadableSet.fd_count; ++Index)
                        {
                        // Find the first socket still in the set.  Another thread may have selected
                        // the same socket so we do not want to give it to this thread.
                        ConnectionMap::iterator Itr = m_ConnectionMap.find(ReadableSet.fd_array[Index]);
                        if (Itr != m_ConnectionMap.end())
                            {
                            // Add the connection to the active connections list
                            m_ActiveConnections.push_back((*Itr).second);

                            // remove from the map
                            m_ConnectionMap.erase(Itr);
                            }

                        // remove the conenction to the socket sets
                        FD_CLR(ReadableSet.fd_array[Index], &m_SocketSet);


                        // if there are entries in the set waiting list, place them in the set
                        while ((m_WaitingSockets.size() > 0) && (m_SocketSet.fd_count < FD_SETSIZE))
                            {
                            HASSERT((*m_WaitingSockets.begin())->m_hSocket != NULL);
                            FD_SET((*m_WaitingSockets.begin())->m_hSocket, &m_SocketSet);
                            m_WaitingSockets.pop_front();
                            // signal to other thread that there is something available in the socket set
                            m_SocketSetSemaphore.Signal();
                            }
                        }
                    }

                // if nothing happened in the previous select, we will check if there are
                // connections in the waiting list and do a swap from the waiting list
                // to the socket set.
                else
                    {
                    HFCMonitor Monitor(m_Key);

                    // only swap if there are waiting sockets
                    if (m_WaitingSockets.size() > 0)
                        {
                        // move the current sockets in the waiting list
                        for (uint32_t Index = 0; Index < m_SocketSet.fd_count; ++Index)
                            {
                            // Find the corresponding socket connection object in the map
                            ConnectionMap::const_iterator Itr = m_ConnectionMap.find(ReadableSet.fd_array[Index]);
                            if (Itr != m_ConnectionMap.end())
                                {
                                m_WaitingSockets.push_back((*Itr).second);
                                }
                            }

                        // clear the socket set
                        FD_ZERO(&m_SocketSet);

                        // if there are entries in the set waiting list, place them in the set
                        while ((m_WaitingSockets.size() > 0) && (m_SocketSet.fd_count < FD_SETSIZE))
                            {
                            HASSERT((*m_WaitingSockets.begin())->m_hSocket != NULL);
                            FD_SET((*m_WaitingSockets.begin())->m_hSocket, &m_SocketSet);
                            m_WaitingSockets.pop_front();
                            // signal to other thread that there is something available in the socket set
                            m_SocketSetSemaphore.Signal();
                            }
                        }
                    }
                }

            // now, there may be some sockets in the active connection list
            if (m_ActiveConnections.size() > 0)
                {
                pResult = *m_ActiveConnections.begin();
                m_ActiveConnections.pop_front();
                }
            }
        }

    return pResult;
    }
