//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSRequestHandler.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSRequestHandler
//-----------------------------------------------------------------------------

#pragma once


class HCSRequest;
class HCSResponseGroup;
template<class Context> class HCSProtocol;


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    The HCSRequestHandler is an abstract class that gives a common interface
    to objects that are specialized to handle requests in a client-server
    application.  A new class derived from this class must be made for each
    type of request in a particular protocol.

    Most users of the HCS classes will not see the processing past the protocol
    level, but a more complicated protocol can override the handling of a
    request for a specific need.  What a protocol does basically is try to
    find the first of its request handlers that can handle the request and
    then executes the request handler with the request and a context.

    When searching for handler capable of handling a request, the CanHandle
    method of the handler class will indicate if the current handler in the list
    is capable of handling the request.  This method is pure virtual so that
    a derived class must override it for a particular request.

    When a request handler is found for a particular request, the Execute method of
    the handler class will be called to perform the processing for the request.
    The Execute method takes as parameter: the request, a response group and a
    context.  The response group is used to return all the resulting responses
    to a particular request or request group (groups are handled at the server
    handler level).  The context is used to pass implementation specific values
    indicating information important to the defined protocol.  The context is a
    template type so that no restrictions affect the implementation.

    -----------------------------------------------------------------------------
*/
template<class Context> class HCSRequestHandler
    {
    friend class HCSProtocol<Context>;
public:
    ////////////////////////////////////////
    // Constructor and destructor
    ////////////////////////////////////////

    HCSRequestHandler(const HCSProtocol<Context>* pi_pProtocol = 0);
    virtual             ~HCSRequestHandler();


    ////////////////////////////////////////
    // Handling Methods
    ////////////////////////////////////////

    virtual bool       CanHandle(const HCSRequest& pi_rRequest) const = 0;
    virtual void        Execute  (const HCSRequest& pi_rRequest,
                                  HCSResponseGroup* pio_pResponseGroup,
                                  Context*          pio_pContext) const = 0;


    ////////////////////////////////////////
    // Protocol Methods
    ////////////////////////////////////////

    const HCSProtocol<Context>*
    GetProtocol() const;


protected:
    ////////////////////////////////////////
    // Protocol Methods
    ////////////////////////////////////////

    void                    SetProtocol(const HCSProtocol<Context>* pi_pProtocol);


private:

    // Const pointer to the protocol that holds us
    const HCSProtocol<Context>*
    m_pProtocol;
    };

#include "HCSRequestHandler.hpp"

