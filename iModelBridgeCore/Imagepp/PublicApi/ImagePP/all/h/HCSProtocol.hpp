//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSProtocol.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSProtocol
//-----------------------------------------------------------------------------

#include "HCSResponseGroup.h"


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Context>
HCSProtocol<Context>::HCSProtocol(const HFCVersion& pi_rVersion)
    : m_Version(pi_rVersion)
    {
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Context>
HCSProtocol<Context>::~HCSProtocol()
    {
    // delete the pointers in the list.  The actual internal list object will
    // be destroyed afterwards when the list is destroyed.
    for (RequestHandlerList::const_iterator Itr = m_RequestHandlerList.begin();
         Itr != m_RequestHandlerList.end();
         Itr++)
        delete (*Itr);
    }



//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Context>
inline const HFCVersion& HCSProtocol<Context>::GetProtocolVersion() const
    {
    return (m_Version);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Context>
inline const HCSRequestHandler<Context>*
HCSProtocol<Context>::IdentifyRequestHandler(const HCSRequest& pi_rRequest) const
    {
    const HCSRequestHandler<Context>* pRequest = 0;

    // find the first request handler that can handle the request
    for (RequestHandlerList::const_iterator Itr = m_RequestHandlerList.begin();
         (pRequest == 0) && (Itr != m_RequestHandlerList.end());
         Itr++)
        {
        // verify if the current handler can handle the request
        if ((*Itr)->CanHandle(pi_rRequest))
            pRequest = *Itr;
        }

    return (pRequest);
    }


//-----------------------------------------------------------------------------
// public
// CanHandle - Normally, this member would be a pure virtual method so that it
// must be overidden for a specific protocol.  Instead, it will return true so
// that a one version protocol may be implemented with this class without any
// derivation.
//-----------------------------------------------------------------------------
template<class Context>
inline bool HCSProtocol<Context>::CanHandle(const HCSRequestGroup& pi_rRequests) const
    {
    return (true);
    }


//-----------------------------------------------------------------------------
// public
// Add a request handler to the protocol.  The protocol tells the handler that
// it is now part of it.
//-----------------------------------------------------------------------------
template<class Context>
void HCSProtocol<Context>::AddRequestHandler(HCSRequestHandler<Context>* pi_pHandler)
    {
    HPRECONDITION(pi_pHandler != 0);

    // Add the request handler to the list
    m_RequestHandlerList.push_back(pi_pHandler);

    // tell the request handler we're handling it
    if (pi_pHandler->GetProtocol() != this)
        pi_pHandler->SetProtocol(this);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Context>
const HCSRequestHandler<Context>*
HCSProtocol<Context>::GetRequestHandler(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < CountRequestHandlers());

    return (m_RequestHandlerList[pi_Index]);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Context>
inline uint32_t HCSProtocol<Context>::CountRequestHandlers() const
    {
    return (uint32_t)m_RequestHandlerList.size();
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
template<class Context>
bool HCSProtocol<Context>::ExecuteRequest(const HCSRequest& pi_rRequest,
                                           HCSResponseGroup* pio_pResponseGroup,
                                           Context*          pio_pContext) const
    {
    HPRECONDITION(pio_pResponseGroup);
    HPRECONDITION(pio_pContext);
    bool Result = false;

    // Get the appropriate request handler from the protocol
    const HCSRequestHandler<Context>* pHandler = IdentifyRequestHandler(pi_rRequest);

    //
    // Now, the moment of truth, the handling of the request
    //
    if (pHandler != 0)
        {
        pHandler->Execute(pi_rRequest, pio_pResponseGroup, pio_pContext);
        Result = true;
        }

    return (Result);
    }

