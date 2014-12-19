//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSConnectionPool.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class :
//-----------------------------------------------------------------------------

#pragma once

#include "HFCThread.h"
#include <Imagepp/all/h/HFCExclusiveKey.h>

class HFCInternetConnection;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements a pool of connections. A pool of connections executes
    wihin its own thread. A connection pool is a kind of intelligent behavioral
    list of connections. Connections are added or removed from the pool. Connections
    are asked to the pool that will provide one when such a connection is available.
    -----------------------------------------------------------------------------
*/
class HCSConnectionPool : public HFCThread
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HCSConnectionPool();
    HCSConnectionPool(time_t pi_ExpirationTime, uint32_t pi_ExpirationInterval);
    virtual         ~HCSConnectionPool();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // adds a connection to the pool
    virtual void    AddConnection(HFCInternetConnection* pi_pConnection);

    // removes a connection from the pool
    virtual void    RemoveConnection(HFCInternetConnection* pi_pConnection);

    // Get the first connection in the pool, whether or not the connection is
    // active.  The returned connection is given to the caller.  The caller
    // has the responsability of replacing it in the pool if needed.
    virtual HFCInternetConnection*
    GetConnection(uint32_t pi_TimeOut);               // in milliseconds

    // returns the first connections that has activity.  The activity
    // method removes disconnected connections when they are detected.
    // The returned connection is given to the caller.  The caller
    // has the responsability of replacing it in the pool if needed.
    virtual HFCInternetConnection*
    GetActiveConnection(DWORD pi_TimeOut);         // in milliseconds

    // Get/Set the expiration parameters.
    time_t          GetExpirationTime       () const;               // in seconds
    void            SetExpirationTime       (time_t pi_Time);
    DWORD           GetExpirationInterval   () const;
    void            SetExpirationInterval   (uint32_t pi_Interval);   // in seconds
    void            DoExpiration();

    // Get information about the connections in the pool
    size_t          GetConnectionCount() const;

    // The job of the thread is to verify the expiration of connections
    // in the pool.
    virtual void    Go();


protected:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // The list of requests.  The map is used for both a faster search
    // on pointers and to keep the last use of a connection
    typedef map<HFCInternetConnection*, time_t>
    Connections;
    Connections             m_Connections;

    // The expiration time for an idle connection
    uint32_t                m_ExpirationInterval;
    time_t                  m_ExpirationTime;
    mutable HFCEvent        m_ExpirationEvent;

    // The thread-safe key
    mutable HFCExclusiveKey m_Key;
    };

