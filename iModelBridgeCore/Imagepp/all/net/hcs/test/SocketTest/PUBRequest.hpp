//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBRequest.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBRequest
//-----------------------------------------------------------------------------

#include "HCSConnectionPool.h"
#include "HFCInternetConnection.h"
#include "HFCHTTPHeader.h"


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
inline PUBRequest::PUBRequest(time_t                 pi_ReceivedTime,
                              HFCInternetConnection* pi_pConnection,
                              HCSConnectionPool&     pi_rPool,
                              HFCPtr<PUBCacheEntry>& pi_rpEntry,
                              const string&          pi_rRequest,
                              HFCHTTPHeader*         pi_pHeader)
    : m_rPool(pi_rPool),
      m_Request(pi_rRequest)
    {
    HPRECONDITION(pi_ReceivedTime > 0);
    HPRECONDITION(pi_pConnection != 0);
    HPRECONDITION(pi_rpEntry != 0);
    HPRECONDITION(!pi_rRequest.empty());
    HPRECONDITION(pi_pHeader != 0);

    m_ReceivedTime = pi_ReceivedTime;
    m_pConnection = pi_pConnection;
    m_pEntry = pi_rpEntry;
    m_State = RECEIVED;
    m_pHeader = pi_pHeader;
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
inline PUBRequest::~PUBRequest()
    {
    try
        {
        if (m_pConnection != 0)
            m_pConnection->Disconnect();
        m_pConnection = 0;
        }
    catch(...)
        {
        }
    }


//-----------------------------------------------------------------------------
// Public
// Returns the arrival time of the request
//-----------------------------------------------------------------------------
inline time_t PUBRequest::GetReceivedTime() const
    {
    return m_ReceivedTime;
    }



//-----------------------------------------------------------------------------
// Public
// Returns the connection.
//-----------------------------------------------------------------------------
inline HFCInternetConnection& PUBRequest::GetConnection() const
    {
    HPRECONDITION(m_pConnection != 0);

    return (*m_pConnection);
    }


//-----------------------------------------------------------------------------
// Public
// Returns the connection to the connection pool.  If we need to after usage.
//-----------------------------------------------------------------------------
inline void PUBRequest::ReleaseConnectionToPool()
    {
    if (m_pConnection != 0)
        m_rPool.AddConnection(m_pConnection.release());
    }


//-----------------------------------------------------------------------------
// Public
// Returns a reference to the cache entry.
//-----------------------------------------------------------------------------
inline PUBCacheEntry& PUBRequest::GetCacheEntry() const
    {
    return (*m_pEntry);
    }


//-----------------------------------------------------------------------------
// Public
// returns the request string
//-----------------------------------------------------------------------------
inline const string& PUBRequest::GetRequestString() const
    {
    return m_Request;
    }


//-----------------------------------------------------------------------------
// Public
// Returns the state of the request
//-----------------------------------------------------------------------------
inline PUBRequest::State PUBRequest::GetState() const
    {
    return m_State;
    }


//-----------------------------------------------------------------------------
// Public
// Changes the state of the request
//-----------------------------------------------------------------------------
inline void PUBRequest::SetState(State pi_State)
    {
    HPRECONDITION((pi_State >= RECEIVED) && (pi_State <= DONE));

    m_State = pi_State;
    }


//-----------------------------------------------------------------------------
// Public
// returns the HTTP header
//-----------------------------------------------------------------------------
inline HFCHTTPHeader& PUBRequest::GetHTTPHeader() const
    {
    return *m_pHeader;
    }
