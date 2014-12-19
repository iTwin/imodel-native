//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSServerConnection.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSServerConnection
//-----------------------------------------------------------------------------

#pragma once

#include "HFCException.h"
#include "HFCConnection.h"
class HFCInternetConnection;


//-----------------------------------------------------------------------------
// HCSServerConnection class
//-----------------------------------------------------------------------------
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements a server connection

    ?????
    -----------------------------------------------------------------------------
*/
class HCSServerConnection : public HFCConnection
    {
public:
    HDECLARE_CLASS_ID(1820, HFCConnection)

    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HCSServerConnection(const WString& pi_rServer);
    virtual         ~HCSServerConnection();


    //--------------------------------------
    // I/O Methods
    //--------------------------------------

    // Connection method with a username and password.  Not needed in a server
    // socket, thus it will be implemented with an HASSERT and a throw.
    virtual bool   Connect(const WString& pi_rUserName,
                            const WString& pi_rPassword,
                            time_t         pi_TimeOut);

    // Connection method
    virtual bool   Connect(time_t pi_TimeOut)         = 0;

    // Disconnects the server socket.
    virtual void    Disconnect()                       = 0;

    // Restablishes the connection if needed.  Not needed in a server
    // socket, thus it will be implemented with an HASSERT and a throw.
    virtual bool   ValidateConnect(uint32_t pi_TimeOut);

    // this function block until a new connection is established through
    // the  server connection.  If a new connection is established,
    // a new connection pointer is returned.  If any other error occurs,
    // an null pointer is returned
    virtual HFCInternetConnection*
    Accept() = 0;

    // this function block until a new connection is established through
    // the  server connection or a tim-out occurs.  If a new connection is
    // established, a new connection pointer is returned.  If any other
    // error occurs, an null pointer is returned
    virtual HFCInternetConnection*
    Accept(uint32_t pi_TimeOut) = 0;
    };


//-----------------------------------------------------------------------------
// HCSServerConnectionConfig
//-----------------------------------------------------------------------------
class HCSServerConnectionConfig
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HCSServerConnectionConfig();
    virtual         ~HCSServerConnectionConfig();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // copies the server configuration
    virtual HCSServerConnectionConfig*
    Clone() const = 0;

    // create the actual server connection from the settings in the object
    virtual HCSServerConnection*
    Create() = 0;
    };

#include "HCSServerConnection.hpp"

