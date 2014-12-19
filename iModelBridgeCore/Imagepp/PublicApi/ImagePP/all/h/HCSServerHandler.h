//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSServerHandler.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSServerHandler
//-----------------------------------------------------------------------------

#pragma once

#include "HCSProtocol.h"
#include "HCSProtocolCreator.h"

class HCSRequestGroup;
class HCSResponseGroup;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class represents the heart of a server request processor. Whenever a
    request group is received from a client, the server application will
    call the server handler. It is the responsibility of the server handler
    to indentify the protocol to use, based on the request group or the
    context, then call the appropriate protocol for handling of the request group.

    A server handler is thus some kind of intelligent collection of protocols.

    The HCSServerHandler is a class that defines an object that handles
    requests from a client in a client-server application. A server handler
    contains one or more versions of protocols and many different protocols,
    which each in turn contain the request handlers for each of the
    possible requests in the protocol.

    In order to have a working server handler, versions of a protocol must
    be registered in the server handler.  If there is more that one protocol
    versions, each version must provide a way to identify one another based
    on a request group (see HCSProtocol::CanHandle).  When registering
    protocols, one can be declared as the default protocol.  The default
    protocol is used when a request group fails to map to a specific
    protocol using the standard procedure.

    Now that the server handler object is constructed and has one or several
    protocols registered, it can now respond to requests from the client.  Once
    a request string has been decomposed in a request group (see HCSRequestGroup and
    HCSRequest for more information), we can try to identify which protocol will
    handle the requests.  If the IdentifyProtocolVersion method fails to return
    a valid protocol, we use the GetDefaultProtocolVersion to obtain
    the default protocol.

    When a protocol is found, we can process each request of the group
    by finding the request handler and executing it.  The latter can be done
    with the HCSProtocol::ExecuteRequest method.  All the server handler
    processing job, from the identification of the protocol to the execution
    of the request handler in the protocol can be done by using the
    HCSServerHandler::ExecuteRequest method.

    -----------------------------------------------------------------------------
*/
template<class Context> class HCSServerHandler
    {
public:
    //--------------------------------------
    // Friends
    //--------------------------------------

    friend class HCSProtocolCreator<Context>;


    //--------------------------------------
    // Constructor and destructor
    //--------------------------------------

    HCSServerHandler();
    virtual         ~HCSServerHandler();


    //--------------------------------------
    // Request Methods
    //--------------------------------------

    // Identifies which protocol can handle the requests in a request group.
    virtual const HCSProtocol<Context>*
    IdentifyProtocolVersion(const HCSRequestGroup& pi_rRequests,
                            Context*               pio_pContext) const;

    // Processes the content of a request group.  Does the job of finding
    // the protocol and executing each request in the group.
    virtual HCSResponseGroup*
    ExecuteRequests(const HCSRequestGroup& pi_rRequests,
                    Context*               pio_pContext) const;


    //--------------------------------------
    // Protocol Methods
    //--------------------------------------

    // Registers a new protocol version in the server handler.
    // NOTE: The protocol becomes the property of the handler.
    void            AddProtocolVersion(HCSProtocol<Context>* pi_pProtocol,
                                       bool                 pi_Default = false);

    // Gets a protocol version based on a version object.
    const HCSProtocol<Context>*
    GetProtocolByVersion(const HFCVersion& pi_rVersion) const;

    // Gets the default protocol version if IdentifyProtocolVersion was unsuccessful.
    const HCSProtocol<Context>*
    GetDefaultProtocolVersion() const;

    // Returns the number of protocol versions registered in the server handler.
    uint32_t        CountProtocolVersions() const;

    // Returns the protocol at the given index
    const HCSProtocol<Context>*
    GetProtocolByIndex(uint32_t pi_Index) const;


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Protocol
    typedef map<HFCVersion, HCSProtocol<Context>*>
    ProtocolMap;
    ProtocolMap     m_ProtocolMap;

    // Default protocol to use when all else fails.
    const HCSProtocol<Context>*
    m_pDefaultProtocol;


    //--------------------------------------
    // Registered Protocols List
    //--------------------------------------

    // static list of implementation objects
    typedef list<const HCSProtocolCreator<Context>*>
    CreatorList;
    class CreatorListDestroyer
        {
    public:
        ~CreatorListDestroyer()
            {
            delete HCSServerHandler<Context>::s_pRegisteredProtocols;
            HCSServerHandler<Context>::s_pRegisteredProtocols = 0;
            }
        };

    // List of registered protocols
    static CreatorList*
    s_pRegisteredProtocols;

    // Destroyer to avoid a memory leak when exiting
    static CreatorListDestroyer
    s_RegisteredProtocolsDestroyer;
    };

#include "HCSServerHandler.hpp"

