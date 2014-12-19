//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSSocketConnectionPool.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSSocketConnectionPool
//-----------------------------------------------------------------------------


#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSSocketConnectionPool.h>
#include <Imagepp/all/h/HCSSocketConnection.h>
#include <Imagepp/all/h/HCSSocketConnectionGroup.h>



//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSSocketConnectionPool::HCSSocketConnectionPool()
    : HCSGroupeableConnectionPool(new HCSSocketConnectionGroup)
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSSocketConnectionPool::HCSSocketConnectionPool(time_t pi_ExpirationTime, uint32_t pi_ExpirationInterval)
    : HCSGroupeableConnectionPool(new HCSSocketConnectionGroup,
                                  pi_ExpirationTime,
                                  pi_ExpirationInterval)
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSSocketConnectionPool::~HCSSocketConnectionPool()
    {
    // Stop the expiration thread before we get in the parent destructor
    if (GetHandle() != NULL)
        {
        StopThread();
        WaitUntilSignaled();
        }
    }
