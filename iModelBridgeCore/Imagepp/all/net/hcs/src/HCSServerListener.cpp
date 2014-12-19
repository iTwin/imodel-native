//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSServerListener.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSServerListener
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSServerListener.h>
#include <Imagepp/all/h/HCSServerConnection.h>
#include <Imagepp/all/h/HCSConnectionPool.h>
#include <Imagepp/all/h/HFCMonitor.h>

// TEMPORARY
#include <Imagepp/all/h/HCSSocketConnection.h>


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSServerListener::HCSServerListener(const HCSServerConnectionConfig& pi_rConfig,
                                     HCSConnectionPool&               pi_rPool)
    : m_ServerConfigChanged(false),
      m_ChangingConfig(false),
      m_rPool(pi_rPool)
    {
    // Set the configuration and start the thread
    SetConfiguration(pi_rConfig);
    m_ServerConfigChanged.Reset();
    m_ChangingConfig.Reset();

    // Start the thread
    StartThread();
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HCSServerListener::~HCSServerListener()
    {
    // Stop the thread
    StopThread();

    // disconnect the server thread so that the call to accept may return
    if (m_pServer != 0)
        m_pServer->Disconnect();

    // Wait until the thread is actually stopped
    WaitUntilSignaled();
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HCSServerListener::Go()
    {
    // Increment this thread priority
    SetPriority(HFCTHREAD_PRIORITY_HIGHEST);

    // In this thread, we loop (almost) indefinitely on the BLOCKING accept
    // call.  Accept will return when a connection is available or if an
    // error occurs.
    //
    // To stop the thread, we will kill the socket that is block on the
    // accept call, causing the method to throw an exception and then
    // verifying the CanRun() method to see if the thread is to be stopped.
    //
    // The throw behaviour is also used by the SetConfiguration() to
    // change the m_pServer to a new configuration.
    bool Continue = true;
    while (Continue)
        {
        try
            {
            // accept a connection
            HFCInternetConnection* pConnection = m_pServer->Accept();
            if (pConnection != 0)
                {
                m_rPool.AddConnection(pConnection);
                }
            }
        catch(...)
            {
            // if we can continue, wait for the server object to become available again
            if (Continue = (m_ChangingConfig.WaitUntilSignaled(0) && CanRun()))
                m_ServerConfigChanged.WaitUntilSignaled();
            }
        }
    }



//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HCSServerConnectionConfig& HCSServerListener::GetConfiguration() const
    {
    return *m_pServerConfig;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HCSServerListener::SetConfiguration(const HCSServerConnectionConfig& pi_rConfig)
    {
    // indicate to the thread that we are changing the configuration
    m_ChangingConfig.Signal();

    // disconnect the server thread so that the call to accept may return
    if (m_pServer != 0)
        m_pServer->Disconnect();

    // Take the given configuration
    m_pServerConfig = pi_rConfig.Clone();

    // Create the new server object
    m_pServer = m_pServerConfig->Create();
    m_pServer->Connect(1);

    // It is the thread's job to create the actual server connection, so
    // signal it.
    m_ServerConfigChanged.Signal();
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HCSConnectionPool& HCSServerListener::GetConnectionPool() const
    {
    return m_rPool;
    }


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------
HCSServerConnection const* HCSServerListener::GetConnection() const
    {
    return m_pServer;
    }
