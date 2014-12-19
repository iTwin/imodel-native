//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSRequest.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSRequest
//-----------------------------------------------------------------------------

#pragma once

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    The HCSRequest class is used to represent a single request from data obtained
    from a client connection in a server application.  A request is represented
    by a key, which is usually a command for the server, and a value for that key.

    The server receives the requests from the client in the form of a continuous
    byte stream.  Usually, an end marker will indicate the actual end of the
    request string.

    A single request object is constructed from a request string and a key-value
    separator string.  The constructor of the class will parse the request string
    to find the key-value separator within the request.  If the separator is found,
    the string preceding the separator becomes the key and the string following the
    separator, its value.  If the separator string cannot be found in the request,
    the whole string becomes the key and there is no value.


    Some protocols can support multiple requests in one single request string up to
    the end marker.  The requests are separated in the string by yet another marker.
    The request object must know the group to which it belongs because some requests
    may have a different behavior when another specific request is grouped with it.
    This becomes clear with the Internet Imaging Protocol where the "FIF" request is
    grouped with the "OBJ=iip,1.0" request, the resulting response is that of
    the "OBJ=basic-info" request.  When the "FIF" is alone, it produces no response.

    @example
    // Example of the a request constructor
    string RequestString("FIF=/images/96060_25.hmr");
    string Separator("=");
    HCSRequest Request(RequestString, Separator);

    -----------------------------------------------------------------------------
*/
class HCSRequest
    {
public:
    //--------------------------------------
    // Constructor and destructor
    //--------------------------------------

    HCSRequest(const WString& pi_rRequest,
               const WString& pi_rSeparator = WString(L""));
    HCSRequest(const HCSRequest& pi_rObj);
    ~HCSRequest();

    HCSRequest&         operator= (const HCSRequest& pi_rObj);


    //--------------------------------------
    // Access to request data
    //--------------------------------------

    const WString&      GetKey() const;
    const WString&      GetValue() const;


private:

    //--------------------------------------
    // Private Members
    //--------------------------------------

    WString             m_Key;
    WString             m_Value;
    };

#include "HCSRequest.hpp"

