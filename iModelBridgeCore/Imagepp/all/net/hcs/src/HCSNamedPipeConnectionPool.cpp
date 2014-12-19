//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSNamedPipeConnectionPool.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSNamedPipeConnectionPool
//-----------------------------------------------------------------------------


#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSNamedPipeConnectionPool.h>
#include <Imagepp/all/h/HCSNamedPipeConnection.h>
#include <Imagepp/all/h/HCSNamedPipeConnectionGroup.h>



//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSNamedPipeConnectionPool::HCSNamedPipeConnectionPool()
    : HCSGroupeableConnectionPool(new HCSNamedPipeConnectionGroup)
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSNamedPipeConnectionPool::HCSNamedPipeConnectionPool(time_t pi_ExpirationTime, time_t pi_ExpirationInterval)
    : HCSGroupeableConnectionPool(new HCSNamedPipeConnectionGroup,
                                  pi_ExpirationTime,
                                  pi_ExpirationInterval)
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSNamedPipeConnectionPool::~HCSNamedPipeConnectionPool()
    {
    // Stop the expiration thread before we get in the parent destructor
    if (GetHandle() != NULL)
        {
        StopThread();
        WaitUntilSignaled();
        }
    }
