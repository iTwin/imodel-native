//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBRequest.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBRequest
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "PUBCacheEntry.h"

class HFCInternetConnection;
class HCSConnectionPool;
class HFCHTTPHeader;

class PUBRequest : public HFCShareableObject<PUBRequest>
    {
public:
    //--------------------------------------
    // Types
    //--------------------------------------

    enum State
        {
        RECEIVED,
        WAITING,
        PROCESSING,
        DONE
        };


    //--------------------------------------
    // Construction / Destruction
    //--------------------------------------

    PUBRequest(time_t                 pi_ReceivedTime,
               HFCInternetConnection* pi_pConnection,
               HCSConnectionPool&     pi_rPool,
               HFCPtr<PUBCacheEntry>& pi_rpEntry,
               const string&          pi_rRequest,
               HFCHTTPHeader*         pi_pHeader);
    virtual                 ~PUBRequest();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Returns the arrival time of the request
    time_t                  GetReceivedTime() const;

    // Returns the connection.
    HFCInternetConnection&  GetConnection() const;

    // Returns the connection to the connection pool.  If we need to after usage.
    void                    ReleaseConnectionToPool();

    // Returns a reference to the cache entry.
    PUBCacheEntry&          GetCacheEntry() const;

    // returns the request string
    const string&           GetRequestString() const;

    // Returns the state of the request
    State                   GetState() const;

    // Changes the state of the request
    void                    SetState(State pi_State);

    // returns the HTTP header
    HFCHTTPHeader&          GetHTTPHeader() const;


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    time_t                  m_ReceivedTime;
    HAutoPtr<HFCInternetConnection>
    m_pConnection;
    HCSConnectionPool&      m_rPool;
    HFCPtr<PUBCacheEntry>   m_pEntry;
    const string            m_Request;
    State                   m_State;
    HAutoPtr<HFCHTTPHeader> m_pHeader;
    };

#include "PUBRequest.hpp"