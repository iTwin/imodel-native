//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSNamedPipeServerConnection.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSNamedPipeServerConnection
//-----------------------------------------------------------------------------

#pragma once

#include "HCSServerConnection.h"
#include "HFCHandle.h"
#include "HFCEvent.h"


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif


    ?????
    -----------------------------------------------------------------------------
*/
class HCSNamedPipeServerConnection : public HCSServerConnection
    {
public:
    HDECLARE_CLASS_ID(1823, HCSServerConnection)

    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HCSNamedPipeServerConnection(const WString& pi_rName);
    virtual         ~HCSNamedPipeServerConnection();


    //--------------------------------------
    // I/O Methods
    //--------------------------------------

    // Connection method
    virtual bool   Connect(time_t pi_TimeOut);
    virtual bool   ConnectExt(DWORD pi_TimeOut,
                               DWORD pi_OutBufferSize,
                               DWORD pi_InBufferSize);


    // Disconnects the server named pipe.
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


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

#ifdef _WIN32
    // The event to control other threads
    HFCEvent        m_PipeEvent;

    // The handle on the named pipe.
    HFCHandle       m_hPipe;
#endif
    };


//-----------------------------------------------------------------------------
// HCSNamedPipeServerConnectionConfig
//-----------------------------------------------------------------------------
class HCSNamedPipeServerConnectionConfig : public HCSServerConnectionConfig
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HCSNamedPipeServerConnectionConfig(const WString& pi_rName);
    virtual         ~HCSNamedPipeServerConnectionConfig();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // copies the server configuration
    virtual HCSServerConnectionConfig*
    Clone() const;

    // create a named pipe server connection
    virtual HCSServerConnection*
    Create();

    // sets the back log parameter for the connect.
    const WString&  GetName() const;
    void            SetName(const WString& pi_rName);


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    WString          m_Name;
    };

#include "HCSNamedPipeServerConnection.hpp"

