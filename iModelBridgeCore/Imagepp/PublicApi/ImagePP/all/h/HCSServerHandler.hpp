//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSServerHandler.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSServerHandler
//-----------------------------------------------------------------------------

#include "HCSRequestGroup.h"

//-----------------------------------------------------------------------------
// Static Members
//-----------------------------------------------------------------------------

// List of registered protocol creators
template<class Context>
typename HCSServerHandler<Context>::CreatorList*
HCSServerHandler<Context>::s_pRegisteredProtocols = 0;

// Destroyer of the list
template<class Context>
typename HCSServerHandler<Context>::CreatorListDestroyer
HCSServerHandler<Context>::s_RegisteredProtocolsDestroyer;



//-----------------------------------------------------------------------------
// public
// Consntructor
//-----------------------------------------------------------------------------
template<class Context>
HCSServerHandler<Context>::HCSServerHandler()
    {
    HASSERT(HCSServerHandler<Context>::s_pRegisteredProtocols != 0);

    // No default protocol for now
    m_pDefaultProtocol = 0;

    // Add the registered protocols
    for (CreatorList::const_iterator Itr = HCSServerHandler<Context>::s_pRegisteredProtocols->begin();
         Itr != HCSServerHandler<Context>::s_pRegisteredProtocols->end();
         Itr++)
        AddProtocolVersion((*Itr)->Create(), (*Itr)->IsDefault());
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
template<class Context>
HCSServerHandler<Context>::~HCSServerHandler()
    {
    // delete the pointers in the list.  The actual internal list object will
    // be destroyed afterwards when the list is destroyed.
    for (ProtocolMap::const_iterator Itr = m_ProtocolMap.begin();
         Itr != m_ProtocolMap.end();
         Itr++)
        delete (*Itr).second;
    }


//-----------------------------------------------------------------------------
// public
// Finds a protocol that can handle the given request group.  Each protocol
// is parsed until one returns true to HCSProtocol::CanHandle.
//-----------------------------------------------------------------------------
template<class Context>
const HCSProtocol<Context>*
HCSServerHandler<Context>::IdentifyProtocolVersion(const HCSRequestGroup& pi_rRequests,
                                                   Context*               pio_pContext) const
    {
    HPRECONDITION(pio_pContext != 0);
    const HCSProtocol<Context>* pResult = 0;

    // Identify the protocol version for the request
    // Parse the map (in order) of protocols and ask them
    // if they can handle the request group.  Use the first
    // protocol version that can.
    ProtocolMap::const_reverse_iterator Itr = m_ProtocolMap.rbegin();
    while ((pResult == 0) &&
           (Itr != m_ProtocolMap.rend()) )
        {
        if ((*Itr).second->CanHandle(pi_rRequests))
            pResult = (*Itr).second;
        Itr++;
        }

    return (pResult);


    }


//-----------------------------------------------------------------------------
// public
// Finds a capable protocol and executes each request in the request group.
//-----------------------------------------------------------------------------
template<class Context>
HCSResponseGroup*
HCSServerHandler<Context>::ExecuteRequests(const HCSRequestGroup& pi_rRequests,
                                           Context*               pio_pContext) const
    {
    HPRECONDITION(pio_pContext);

    // Try to find a protocol that can handle the request group
    const HCSProtocol<Context>* pProtocol = IdentifyProtocolVersion(pi_rRequests, pio_pContext);

    // If no protocol was found, use the default protocol
    if (pProtocol == 0)
        pProtocol = GetDefaultProtocolVersion();

    // At this point, a protocol must have been found
    HASSERT(pProtocol != 0);

    // Create a new response group.
    HCSResponseGroup* pResult = new HCSResponseGroup;
    HASSERT(pResult);

    // Handle each request in the request group
    HCSRequestGroup::IteratorHandle Iterator = pi_rRequests.StartIteration();
    for (uint32_t i = 0; i < pi_rRequests.CountElements(); i++)
        {
        // Get the request
        const HCSRequest* pRequest = pi_rRequests.GetElement(Iterator);
        HASSERT(pRequest != 0);

        // Handle the current request
        pProtocol->ExecuteRequest(*pRequest, pResult, pio_pContext);

        // Proceed to the next request
        pi_rRequests.Iterate(Iterator);
        }
    pi_rRequests.StopIteration(Iterator);

    // return the response group
    return (pResult);
    }


//-----------------------------------------------------------------------------
// public
// Adds a new protocol version to the server handler.  The given protocol becomes
// the property of the server handler.
// There must not be a similar version in the handler.
//-----------------------------------------------------------------------------
template<class Context>
void HCSServerHandler<Context>::AddProtocolVersion(HCSProtocol<Context>* pi_pProtocol,
                                                   bool                 pi_Default)
    {
    HPRECONDITION(pi_pProtocol != 0);
    HPRECONDITION(m_ProtocolMap.find(pi_pProtocol->GetProtocolVersion()) == m_ProtocolMap.end());

    // add the protocol in the map
    m_ProtocolMap.insert(ProtocolMap::value_type(pi_pProtocol->GetProtocolVersion(),
                                                 pi_pProtocol));

    // set the default if wanted or if there is only one protocol in the map
    if (pi_Default || (m_ProtocolMap.size() == 1))
        m_pDefaultProtocol = pi_pProtocol;
    }


//-----------------------------------------------------------------------------
// public
// Returns the protocol that is identified by the given version.  if none is
// found, the method returns 0.
//-----------------------------------------------------------------------------
template<class Context>
const HCSProtocol<Context>*
HCSServerHandler<Context>::GetProtocolByVersion(const HFCVersion& pi_rVersion) const
    {
    const HCSProtocol<Context>* pResult = 0;

    ProtocolMap::const_iterator Itr = m_ProtocolMap.find(pi_rVersion);
    if (Itr != m_ProtocolMap.end())
        pResult = (*Itr).second;

    return (pResult);
    }


//-----------------------------------------------------------------------------
// public
// Returns the default protocol to use if no response was returned by
// IdentifyProtocolVersion.
//-----------------------------------------------------------------------------
template<class Context>
inline const HCSProtocol<Context>*
HCSServerHandler<Context>::GetDefaultProtocolVersion() const
    {
    return (m_pDefaultProtocol);
    }


//-----------------------------------------------------------------------------
// public
// Returns the number of protocols registerd in the server handler.
//-----------------------------------------------------------------------------
template<class Context>
inline uint32_t HCSServerHandler<Context>::CountProtocolVersions() const
    {
    return (m_ProtocolMap.size());
    }


//-----------------------------------------------------------------------------
// public
// Returns the protocol at the given index in the protocol map
//-----------------------------------------------------------------------------
template<class Context>
inline const HCSProtocol<Context>*
HCSServerHandler<Context>::GetProtocolByIndex(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < m_ProtocolMap.size());
    const HCSProtocol<Context>* pResult = 0;

    // Iterate on the map until we find the item at the required index
    uint32_t Index = 0;
    for (ProtocolMap::const_iterator Itr = m_ProtocolMap.begin();
         (pResult == 0) && (Itr != m_ProtocolMap.end());
         Itr++, Index++)
        if (Index == pi_Index)
            pResult = (*Itr).second;

    HPOSTCONDITION(pResult != 0);
    return (pResult);
    }

