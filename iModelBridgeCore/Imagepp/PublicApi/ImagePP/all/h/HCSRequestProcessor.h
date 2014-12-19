//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSRequestProcessor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSRequestProcessor
//-----------------------------------------------------------------------------

#pragma once

#include "HFCThread.h"
#include <Imagepp/all/h/HFCExclusiveKey.h>
#include "HFCMutex.h"
#include "HFCBuffer.h"


class HFCInternetConnection;
class HCSConnectionPool;
class HFCBinStream;


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This abstract class defines the common interface for all requests processors

    ?????
    -----------------------------------------------------------------------------
*/
class HCSRequestProcessor : public HFCThread
    {
public:
    //--------------------------------------
    // Construction / Destruction
    //--------------------------------------

    HCSRequestProcessor(HCSConnectionPool& pi_rPool);
    virtual                 ~HCSRequestProcessor();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // returns the connection pool associated with the processor
    HCSConnectionPool&      GetConnectionPool() const;

    // changes the connection pool associated withe the process
    void                    SetConnectionPool(HCSConnectionPool& pi_rPool);

    // Thread implementation.
    virtual void            Go() = 0;


protected:
    //--------------------------------------
    // Methods
    //--------------------------------------

    // Gets the next active connection from the pool
    HFCInternetConnection*  GetActiveConnectionFromPool(uint32_t pi_TimeOut);

    // Gets the next connection from the pool, active or not
    HFCInternetConnection*  GetConnectionFromPool(uint32_t pi_TimeOut);

    // Releases or destroys the current active connection.  Thread-safe.
    void                    DestroyConnection(bool pi_OnlyDisconnect = false);
    void                    ReleaseConnection();

    // Extracts a request from the current connection.  The time out is in milli-seconds.
    string                  GetRequestFromConnection(const string& pi_rSeparator,
                                                     uint32_t      pi_TimeOut);

    // Extracts a request from the current connection.  The time out is in milli-seconds.
    uint32_t                ReadBytesFromConnection(uint32_t      pi_BytesToRead,
                                                    uint32_t      pi_TimeOut,
                                                    HFCBinStream* po_pOutputStream);


protected:
    //--------------------------------------
    // Member
    //--------------------------------------

    // The associated connection pool
    mutable HFCExclusiveKey m_PoolKey;
    HCSConnectionPool*      m_pPool;

    // The current connection and helpers to control the thread usage of the connection
    HAutoPtr<HFCInternetConnection>
    m_pConnection;
//        typedef HFCMutex
    typedef HFCExclusiveKey
    ConnectionKeyType;
    mutable ConnectionKeyType
    m_ConnectionKey;

private:

    // Check for the separator string at the end of the buffer
    bool                   BufferEndsWithSeparator(const HFCBuffer& pi_rBuffer,
                                                    const string&    pi_rSeparator) const;
    };

#include "HCSRequestProcessor.hpp"

