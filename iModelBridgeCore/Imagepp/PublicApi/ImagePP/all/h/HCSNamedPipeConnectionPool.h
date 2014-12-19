//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSNamedPipeConnectionPool.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class :
//-----------------------------------------------------------------------------

#pragma once

#include "HCSGroupeableConnectionPool.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements a specific connection pool that manages named pipe connections
    on a group basis.

    ?????
    -----------------------------------------------------------------------------
*/
class HCSNamedPipeConnectionPool : public HCSGroupeableConnectionPool
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HCSNamedPipeConnectionPool();
    HCSNamedPipeConnectionPool(time_t pi_ExpirationTime, time_t pi_ExpirationInterval);
    virtual         ~HCSNamedPipeConnectionPool();
    };

