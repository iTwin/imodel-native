//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSSocketServerConnection.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSSocketServerConnection
//-----------------------------------------------------------------------------

#pragma once

#include "HCSServerConnection.h"

#ifndef _WIN32
#error Not yet implemented for this OS
#endif

//-----------------------------------------------------------------------------
// HCSSocketServerConnection class
//-----------------------------------------------------------------------------
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements a specific server connection baased on an operating
    system socket.

    ?????
    -----------------------------------------------------------------------------
*/
class HCSSocketServerConnection : public HCSServerConnection
    {
public:
    HDECLARE_CLASS_ID(1821, HCSServerConnection)

    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    // This contructor uses a port returned by winsock on any address.
    HCSSocketServerConnection();

    // This constructor create a new socket on the any address on the given port
    HCSSocketServerConnection(unsigned short pi_Port);

    // This constructor create a new socket on the given address on a port given by winsock
    HCSSocketServerConnection(const string& pi_rAddress);

    // This constructor create a new socket on the given address or port
    HCSSocketServerConnection(const string& pi_rAddress, unsigned short pi_Port);

    virtual         ~HCSSocketServerConnection();


    //--------------------------------------
    // I/O Methods
    //--------------------------------------

    // Connection method
    virtual bool   Connect(time_t pi_TimeOut);

    // Disconnects the server socket.
    virtual void    Disconnect();

    // this function block until a new connection is established through
    // the  server connection.  If a new connection is established,
    // a new connection pointer is returned.  If any other error occurs,
    // an null pointer is returned
    virtual HFCInternetConnection*
    Accept();

    // this function block until a new connection is established through
    // the  server connection or a tim-out occurs.  If a new connection is
    // established, a new connection pointer is returned.  If any other
    // error occurs, an null pointer is returned
    virtual HFCInternetConnection*
    Accept(uint32_t pi_TimeOut);


    //--------------------------------------
    // OtherMethods
    //--------------------------------------

    // sets the back log parameter for the connect.
    uint32_t         GetBacklog() const;
    void            SetBacklog(uint32_t pi_Backlog = UINT_MAX);

    // Allows other server socket to use the same port
    bool           IsConnectionShared() const;
    void            ShareConnection(bool pi_Shared);

    // returns the address
    const  string&   GetAddress() const;

    // Returns the obtained port
    unsigned short GetPort() const;


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // The socket to use
#ifdef _WIN32
    SOCKET          m_hSocket;
#endif

    // The address and port of the listening socket
    string          m_Address;
    unsigned short m_Port;
    uint32_t         m_Backlog;
    bool           m_Shared;
    };


//-----------------------------------------------------------------------------
// HCSSocketServerConnectionConfig
//-----------------------------------------------------------------------------
class HCSSocketServerConnectionConfig : public HCSServerConnectionConfig
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HCSSocketServerConnectionConfig();
    HCSSocketServerConnectionConfig(unsigned short pi_Port);
    HCSSocketServerConnectionConfig(const string& pi_rAddress);
    HCSSocketServerConnectionConfig(const string& pi_rAddress,
                                    unsigned short pi_Port);
    virtual         ~HCSSocketServerConnectionConfig();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // copies the server configuration
    virtual HCSServerConnectionConfig*
    Clone() const;

    // create a socket server connection
    virtual HCSServerConnection*
    Create();

    // sets the back log parameter for the connect.
    uint32_t         GetBacklog() const;
    void            SetBacklog(uint32_t pi_Backlog = UINT_MAX);

    // Allows other server socket to use the same port
    bool           IsConnectionShared() const;
    void            ShareConnection(bool pi_Shared);

    // returns the address
    const string&   GetAddress() const;

    // Returns the port
    unsigned short GetPort() const;


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    string          m_Address;
    unsigned short m_Port;
    bool           m_Shared;
    uint32_t         m_Backlog;
    };

#include "HCSSocketServerConnection.hpp"

