//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSBufferedConnectionPool.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSBufferedConnectionPool
//-----------------------------------------------------------------------------

#pragma once

#include "HCSConnectionPool.h"

class HCSBufferedConnection;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements a buffered connection pool

    ????
    -----------------------------------------------------------------------------
*/
class HCSBufferedConnectionPool : public HCSConnectionPool
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HCSBufferedConnectionPool(HCSConnectionPool& pi_rPool);
    HCSBufferedConnectionPool(HCSConnectionPool& pi_rPool,
                              time_t             pi_ExpirationTime,
                              uint32_t           pi_ExpirationInterval);
    virtual         ~HCSBufferedConnectionPool();


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
    GetActiveConnection(uint32_t pi_TimeOut);         // in milliseconds


protected:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // A reference to the connection pool to buffer
    HCSConnectionPool&
    m_rPool;

    // on top the the buffered connection pool, we will keep a
    // list of connections that are returned and still have data to be
    // read in there internal buffers.  When the connection is returned
    // to the pool and there is still data in the buffer, we will place
    // the connection in the list so that a future Get___Connection
    // uses there connections
    typedef list<HCSBufferedConnection*>
    BufferedConnectionList;
    BufferedConnectionList
    m_BufferedConnections;
    };

