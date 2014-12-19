//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSRequestHandler.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSRequestHandler
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
template<class Context>
inline HCSRequestHandler<Context>::HCSRequestHandler(const HCSProtocol<Context>* pi_pProtocol)
    {
    m_pProtocol = pi_pProtocol;
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
template<class Context>
inline HCSRequestHandler<Context>::~HCSRequestHandler()
    {
    }


//-----------------------------------------------------------------------------
// Protected
// Sets the protocol that defines the handler.  Usually called by the protocol
// when a handler is added to it.  At this point, if SetProtocol is invoked,
// the given pointer must exist.
//-----------------------------------------------------------------------------
template<class Context>
inline void HCSRequestHandler<Context>::SetProtocol(const HCSProtocol<Context>* pi_pProtocol)
    {
    HPRECONDITION(pi_pProtocol != 0);

    m_pProtocol = pi_pProtocol;
    }


//-----------------------------------------------------------------------------
// public
// Returns the protocol that defines this request handler
//-----------------------------------------------------------------------------
template<class Context>
inline const HCSProtocol<Context>* HCSRequestHandler<Context>::GetProtocol() const
    {
    return (m_pProtocol);
    }