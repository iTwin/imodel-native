//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBRequestDispatcher.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBRequestDispatcher
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HFCSemaphore.h"

class PUBRequest;
class PUBEngineThread;

class PUBRequestDispatcher
    {
public:
    //--------------------------------------
    // Construction / Destruction
    //--------------------------------------

    PUBRequestDispatcher ();
    virtual                 ~PUBRequestDispatcher();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // adds a request to the dispatcher
    virtual void            AddRequest(HFCPtr<PUBRequest>& pi_rpRequest);
    virtual void            AddRequest(PUBRequest*         pi_pRequest);

    // Gets the first available request that meets the dispatching algorithm.
    // This method implements the actual dispatching of requests.
    virtual HFCPtr<PUBRequest>
    GetRequest(PUBEngineThread& pi_rEngine, time_t pi_TimeOut);


protected:
    //--------------------------------------
    // types
    //--------------------------------------

    // a map of request string to request object
    typedef multimap<string, HFCPtr<PUBRequest> >
    RequestMap;


    //--------------------------------------
    // Attributes
    //--------------------------------------

    // the map of request
    mutable HFCSemaphore    m_RequestMapSemaphore;
    mutable HFCExclusiveKey m_RequestMapKey;
    RequestMap              m_RequestMap;
    };