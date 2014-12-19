//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSServerListener.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSServerListener
//-----------------------------------------------------------------------------

#pragma once

#include "HFCThread.h"
#include "HFCEvent.h"

class HFCConnection;
class HCSServerConnection;
class HCSServerConnectionConfig;
class HCSConnectionPool;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements a server listener. Sucha server listener represents the
    eaar (or one of the ears) of a server. The server listener listens upon a
    specific port for activity. If activity is detected, then a connection is
    established with the caller using provided connection pool.

    ?????
    -----------------------------------------------------------------------------
*/
class HCSServerListener : public HFCThread
    {
public:
    //--------------------------------------
    // Construction - Destruction
    //--------------------------------------

    HCSServerListener(const HCSServerConnectionConfig& pi_rConfig,
                      HCSConnectionPool&               pi_rPool);
    virtual         ~HCSServerListener();


    //--------------------------------------
    // Overloaded from HFCThread
    //--------------------------------------

    virtual void    Go();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // changes the server connections
    virtual const HCSServerConnectionConfig&
    GetConfiguration() const;
    virtual void    SetConfiguration(const HCSServerConnectionConfig& pi_rConfig);

    // Returns the associated connection pool
    virtual const HCSConnectionPool&
    GetConnectionPool() const;

    HCSServerConnection const*
    GetConnection() const;


private:
    //--------------------------------------
    // Not implemented
    //--------------------------------------

    HCSServerListener(const HCSServerListener&);
    HCSServerListener& operator=(const HCSServerListener&);


    //--------------------------------------
    // Attributes
    //--------------------------------------

    // The server connection configuration
    mutable HFCEvent    m_ServerConfigChanged;
    mutable HFCEvent    m_ChangingConfig;
    HAutoPtr<HCSServerConnectionConfig>
    m_pServerConfig;

    // The connection pool.
    HCSConnectionPool&  m_rPool;

    // Server Connection
    HAutoPtr<HCSServerConnection>
    m_pServer;
    };

