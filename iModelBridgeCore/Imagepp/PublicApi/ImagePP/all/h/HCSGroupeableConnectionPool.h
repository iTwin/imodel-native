//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSGroupeableConnectionPool.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSGroupeableConnectionPoo
//-----------------------------------------------------------------------------

#pragma once

#include "HCSConnectionPool.h"
class HCSConnectionGroup;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements a specific connection pool that manages connections
    on a group basis.

    ?????
    -----------------------------------------------------------------------------
*/
class HCSGroupeableConnectionPool : public HCSConnectionPool
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HCSGroupeableConnectionPool(HCSConnectionGroup* pi_pConnectionGroup);
    HCSGroupeableConnectionPool(HCSConnectionGroup* pi_pConnectionGroup,
                                time_t pi_ExpirationTime,
                                uint32_t pi_ExpirationInterval);
    virtual         ~HCSGroupeableConnectionPool();


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


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    HAutoPtr<HCSConnectionGroup>
    m_pConnections;
    };

