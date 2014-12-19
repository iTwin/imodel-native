//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSNamedPipeConnectionGroup.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSNamedPipeConnectionGroup
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSNamedPipeConnectionGroup.h>
#include <Imagepp/all/h/HCSNamedPipeConnection.h>
#include <Imagepp/all/h/HFCThread.h>


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCSNamedPipeConnectionGroup::HCSNamedPipeConnectionGroup()
    {
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCSNamedPipeConnectionGroup::~HCSNamedPipeConnectionGroup()
    {
    }


//-----------------------------------------------------------------------------
// public
// Add a socket connection to the group
//-----------------------------------------------------------------------------
void HCSNamedPipeConnectionGroup::AddConnection(HFCInternetConnection* pi_pConnection)
    {
    HPRECONDITION(pi_pConnection->IsCompatibleWith(HCSNamedPipeConnection::CLASS_ID));
    HFCMonitor Monitor(m_Key);

#ifdef _WIN32
    HCSNamedPipeConnection* pConnection = static_cast<HCSNamedPipeConnection*>(pi_pConnection);

    // add the connection to the map of connections
    HPRECONDITION(m_ConnectionMap.find(&pConnection->m_ReadEvent) == m_ConnectionMap.end());
    m_ConnectionMap.insert(ConnectionMap::value_type(&pConnection->m_ReadEvent, pConnection));

    // add the conenction to the socket sets
    if (m_PipeSet.CountObjects() < MAXIMUM_WAIT_OBJECTS)
        m_PipeSet.AddSynchro(&pConnection->m_ReadEvent);
    else
        m_WaitingPipes.push_back(pConnection);
#endif
    }


//-----------------------------------------------------------------------------
// public
// Removes a socket connection to the group
//-----------------------------------------------------------------------------
void HCSNamedPipeConnectionGroup::RemoveConnection(HFCInternetConnection* pi_pConnection)
    {
    HPRECONDITION(pi_pConnection->IsCompatibleWith(HCSNamedPipeConnection::CLASS_ID));
    HFCMonitor Monitor(m_Key);

#ifdef _WIN32
    HCSNamedPipeConnection* pConnection = static_cast<HCSNamedPipeConnection*>(pi_pConnection);

    // Remove the connection to the map of connections
    ConnectionMap::iterator Itr = m_ConnectionMap.find(&pConnection->m_ReadEvent);
    if (Itr != m_ConnectionMap.end())
        {
        // remove the connection to the socket sets
        m_PipeSet.RemoveSynchro(&pConnection->m_ReadEvent);

        // remove from the map
        m_ConnectionMap.erase(Itr);

        // if there are entries in the set waiting list, place them in the set
        while ((m_WaitingPipes.size() > 0) && (m_PipeSet.CountObjects() < MAXIMUM_WAIT_OBJECTS))
            {
            m_PipeSet.AddSynchro(&(*m_WaitingPipes.begin())->m_ReadEvent);
            m_WaitingPipes.pop_front();
            }
        }
#endif
    }


//-----------------------------------------------------------------------------
// public
// Checks for activity on a group of socket.
//-----------------------------------------------------------------------------
HFCInternetConnection* HCSNamedPipeConnectionGroup::CheckActivity(time_t pi_TimeOut)
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
        // copy the socket set, because select removes sockets that do not
        // have readable activity.
        HFCSynchroContainer ReadableSet;
            {
            HFCMonitor Monitor(m_Key);
            ReadableSet = m_PipeSet;
            }

        if (ReadableSet.CountObjects() > 0)
            {
            HFCMonitor SelectMonitor(m_ActiveConnectionKey);

            // first verify if a previous select has placed active sockets in the list or not.
            // If not, perform the select function on the socket set
            if (m_ActiveConnections.size() == 0)
                {
                uint32_t WaitResult;
                if ((WaitResult = HFCSynchro::WaitForMultipleObjects(ReadableSet, false)) != HFC_SYNCHRO_TIMEOUT)
                    {
                    HFCMonitor Monitor(m_Key);

                    uint32_t StartIndex = 0;
                    uint32_t EndIndex   = ReadableSet.CountObjects();
                    if (WaitResult != HFC_SYNCHRO_ALL)
                        {
                        StartIndex = WaitResult;
                        EndIndex = StartIndex + 1;
                        }

                    // Parse the sockets until a right one is found
                    for (uint32_t Index = StartIndex; Index < EndIndex; ++Index)
                        {
                        // Find the first socket still in the set.  Another thread may have selected
                        // the same socket so we do not want to give it to this thread.
                        ConnectionMap::const_iterator Itr = m_ConnectionMap.find(ReadableSet.GetObject(Index));
                        if (Itr != m_ConnectionMap.end())
                            {
                            // Add the connection to the active connections list
                            m_ActiveConnections.push_back((*Itr).second);

                            // remove from the map
                            m_ConnectionMap.erase(Itr);
                            }

                        // remove the conenction to the socket sets
                        m_PipeSet.RemoveSynchro(ReadableSet.GetObject(Index));

                        // if there are entries in the set waiting list, place them in the set
                        while ((m_WaitingPipes.size() > 0) && (m_PipeSet.CountObjects() < MAXIMUM_WAIT_OBJECTS))
                            {
                            m_PipeSet.AddSynchro(&(*m_WaitingPipes.begin())->m_ReadEvent);
                            m_WaitingPipes.pop_front();
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
                    if (m_WaitingPipes.size() > 0)
                        {
                        // move the current sockets in the waiting list
                        for (uint32_t Index = 0; Index < m_PipeSet.CountObjects(); ++Index)
                            {
                            // Find the corresponding socket connection object in the map
                            ConnectionMap::const_iterator Itr = m_ConnectionMap.find(m_PipeSet.GetObject(Index));
                            if (Itr != m_ConnectionMap.end())
                                {
                                m_WaitingPipes.push_back((*Itr).second);
                                }
                            }

                        // clear the socket set
                        m_PipeSet = HFCSynchroContainer();

                        // if there are entries in the set waiting list, place them in the set
                        // if there are entries in the set waiting list, place them in the set
                        while ((m_WaitingPipes.size() > 0) && (m_PipeSet.CountObjects() < MAXIMUM_WAIT_OBJECTS))
                            {
                            m_PipeSet.AddSynchro(&(*m_WaitingPipes.begin())->m_ReadEvent);
                            m_WaitingPipes.pop_front();
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

            // there is no socket in the set, so wait for the time out to expire
            else
                {
                SelectMonitor.ReleaseKey();
                HFCThread::Sleep(pi_TimeOut);
                }
            }

        // there is no socket in the set, so wait for the time out to expire
        else
            HFCThread::Sleep(pi_TimeOut);
        }

    return pResult;
    }
