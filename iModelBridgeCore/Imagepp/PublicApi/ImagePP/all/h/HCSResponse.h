//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSResponse.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSResponse
//-----------------------------------------------------------------------------

#pragma once

#include "HFCBuffer.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif


    The HCSResponse class holds the data to a single response to a request.
    An object of this class is created by a request handler and is added
    to a response group.  A request that produces more than one response will
    add an object of this type for each single response and insert them in a
    response group.

    At the top level, the server application will receive a response group
    after processing the current requests.  The application will parse each
    responses of the response group, format each response depending on the
    protocol, if it has not already been done by the request handler and
    send the formatted responses through the communication channel.
    -----------------------------------------------------------------------------
*/
class HCSResponse
    {
public:
    //--------------------------------------
    // Basic
    //--------------------------------------

    // Constructor and destructor
    HCSResponse();
    HCSResponse(const Byte* pi_pData,
                size_t       pi_DataSize);
    HCSResponse(const WString& pi_rResponse);
    HCSResponse(const HCSResponse& pi_rResponse);
    ~HCSResponse();

    // Copy operator
    HCSResponse&    operator=(const HCSResponse& pi_rResponse);


    //--------------------------------------
    // Access to data
    //--------------------------------------

    size_t          GetDataSize() const;
    const Byte*    GetData() const;


    //--------------------------------------
    // Data Appending
    //--------------------------------------

    void            AppendResponse(const Byte* pi_pData, size_t pi_DataSize);
    void            AppendResponse(const WString& pi_rResponse);
    void            AppendResponse(const HCSResponse& pi_rResponse);


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Buffer that holds the response data.
    HFCBuffer           m_Buffer;
    };

#include "HCSResponse.hpp"

