//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSSocketConnectionPool.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class :
//-----------------------------------------------------------------------------

#pragma once

#include "HCSGroupeableConnectionPool.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements a specific connection pool that manages socket connections
    on a group basis.

    ?????
    -----------------------------------------------------------------------------
*/
class HCSSocketConnectionPool : public HCSGroupeableConnectionPool
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HCSSocketConnectionPool();
    HCSSocketConnectionPool(time_t pi_ExpirationTime, uint32_t pi_ExpirationInterval);
    virtual         ~HCSSocketConnectionPool();
    };

