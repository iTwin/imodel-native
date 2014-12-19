//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSProtocol.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSProtocol
//-----------------------------------------------------------------------------

#pragma once

#include "HFCVersion.h"

template<class Context> class HCSRequestHandler;
class HCSRequestGroup;
class HCSRequest;
class HCSResponseGroup;
class HCSResponse;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    The HCSProtocol is an "almost abstract" class that gives a common interface
    to objects that represent a protocol in a client-server application.
    By almost abstract, we mean that the class basically defines an interface
    for protocol object, but the class itself can be instantiated for an
    application that has a single protocol version.  By default, the CanHandle()
    method always returns true.  The programmer of such an application will only
    have to register the request handlers in the generic protocol object.

    When defining a protocol, a request handler must be overridden from the
    abstract HCSRequestHandler class and registered in the protocol using the
    AddRequestHandler method.  Each of the handlers must provide a way to indicate
    if a handler can process a request object (HCSRequestHandler:: CanHandle)
    and a way to process the request (HCSRequestHandler::Execute).

    The template type can be of any type, depending on an actual protocol
    implementation.  All the request handlers that are added to a protocol must
    be templated with the exact same template type.

    The protocol object can override the CanHandle method of HCSProtocol in order to
    have a more specific way to identify if the protocol can handle a group
    of requests.  For example, the Internet Imaging Protocol defines the IIP
    object that is used to indicate which version of the protocol to use.  So when
    the request "obj=iip,1.0" is present in the given response group, the CanHandle
    method can return true to indicate that it is the proper protocol for the job.

    After the protocol is identified as being able to handle a request group, we now
    must handle the processing of the request.  So we now have to iterate on each
    of the requests in the group.  For each request, we ask the protocol
    to find the request handler that can handle the request.  What the protocol
    basically does is ask each of its request handlers, in the order they were
    entered, if it can handle the given request (HCSRequestHandler::CanHandle).
    When we first encounter a matching request handler, the protocol returns a
    constant pointer to that handler.  We can now execute the request by giving it
    the handler along with the response group (that is the result of the processing)
    and the current context for the processing. If a handler cannot be found for a request,
    the method returns false, otherwise it returns true.
    -----------------------------------------------------------------------------
*/
template<class Context> class HCSProtocol
    {
public:
    //--------------------------------------
    // Constructor and destructor
    //--------------------------------------

    HCSProtocol(const HFCVersion& pi_rVersion);
    virtual             ~HCSProtocol();


    //--------------------------------------
    // Identification Methods
    //--------------------------------------

    // Indicates the version of the protocol
    const HFCVersion&   GetProtocolVersion() const;

    // Identifies which of the protocol request handlers can
    // handle the specified single request
    virtual const HCSRequestHandler<Context>*
    IdentifyRequestHandler(const HCSRequest& pi_rRequest) const;

    // Query about the ability of a protocol to handle requests that make
    // a request group.
    virtual bool       CanHandle(const HCSRequestGroup& pi_rRequests) const;


    //--------------------------------------
    // Request Handler Methods
    //--------------------------------------

    // Adds a request handler to the protocol.
    // IMPORTANT: The handler becomes the property of the protocol.
    void                AddRequestHandler(HCSRequestHandler<Context>* pi_pHandler);

    // Get a request handler based on its position in the list.
    const HCSRequestHandler<Context>*
    GetRequestHandler(uint32_t pi_Index) const;

    // Indicates the number of request handler in the protocol
    uint32_t            CountRequestHandlers() const;

    // Utility method that perform the search for the correct handler
    // a then executes that handler
    bool               ExecuteRequest(const HCSRequest& pi_rRequest,
                                       HCSResponseGroup* pio_pResponseGroup,
                                       Context*          pio_pContext) const;


protected:
private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // List of request handlers
    typedef vector<HCSRequestHandler<Context>*>
    RequestHandlerList;
    RequestHandlerList  m_RequestHandlerList;

    // Protocol version
    HFCVersion          m_Version;
    };

#include "HCSProtocol.hpp"

