//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/test/SocketTest/PUBRequestDispatcher.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : PUBRequestDispatcher
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/PUBRequestDispatcher.h>
#include <Imagepp/all/h/PUBRequest.h>


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
PUBRequestDispatcher ::PUBRequestDispatcher ()
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
PUBRequestDispatcher ::~PUBRequestDispatcher()
    {
    }


//-----------------------------------------------------------------------------
// Public
// Adds a connection to the dispatcher.
//-----------------------------------------------------------------------------
void PUBRequestDispatcher ::AddRequest(HFCPtr<PUBRequest>& pi_rpRequest)
    {
    HPRECONDITION(pi_rpRequest != 0);
    HFCMonitor MapMonitor(m_RequestMapKey);

    // add the request in the map
    m_RequestMap.insert(RequestMap::value_type(pi_rpRequest->GetRequestString(), pi_rpRequest));

    // signal to other thread that there are requests available
    m_RequestMapSemaphore.Signal();
    }


//-----------------------------------------------------------------------------
// Public
// Adds a connection to the dispatcher.
//-----------------------------------------------------------------------------
void PUBRequestDispatcher ::AddRequest(PUBRequest* pi_pRequest)
    {
    HPRECONDITION(pi_pRequest != 0);

    AddRequest(HFCPtr<PUBRequest>(pi_pRequest));
    }


//-----------------------------------------------------------------------------
// Public
// Gets the first available request that meets the dispatching algorithm.
// This method implements the actual dispatching of requests.
//-----------------------------------------------------------------------------
HFCPtr<PUBRequest> PUBRequestDispatcher::GetRequest(PUBEngineThread& pi_rEngine,
                                                    time_t           pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut >= 0);
    HFCPtr<PUBRequest> pResult;

    // wait until there is a request available
    if (m_RequestMapSemaphore.WaitUntilSignaled(pi_TimeOut))
        {
        // Get the first item in the map
        HFCMonitor MapMonitor(m_RequestMapKey);
        HASSERT(m_RequestMap.size() > 0);
        RequestMap::iterator Itr = m_RequestMap.begin();
        HASSERT(Itr != m_RequestMap.end());
        pResult = (*Itr).second;

        // remove it from the map
        m_RequestMap.erase(Itr);
        }

    return pResult;
    }



#if 0
HPRECONDITION(pi_pRequest != 0);

// place in an auto_ptr, because the request is given to us.
HAutoPtr<PUBRequest> pRequest(pi_pRequest);

try
    {
    pRequest->GetConnection().Send((const Byte*)s_Message.data(), s_Message.size());
    pRequest->ReleaseConnectionToPool();
    }
catch(...)
    {
    }
    }
#endif