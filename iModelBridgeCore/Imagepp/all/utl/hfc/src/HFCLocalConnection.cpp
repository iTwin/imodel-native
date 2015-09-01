//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCLocalConnection.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCLocalConnection
//-----------------------------------------------------------------------------
// This class describes a local connection.
//-----------------------------------------------------------------------------

// Precompiled header
#include <ImagePPInternal/hstdcpp.h>


// HFC
#include <Imagepp/all/h/HFCLocalConnection.h>
#include <Imagepp/all/h/HFCURLFile.h>


//-----------------------------------------------------------------------------
// HFCLocalConnection
//
// Constructor.
//
// const HFCURLFile& pi_rURL:
// const string& pi_rUserName:
// const string& pi_rPassword:
//
//-----------------------------------------------------------------------------
HFCLocalConnection::HFCLocalConnection(const WString& pi_rServer,
                                       const WString& pi_rUserName,
                                       const WString& pi_rPassword)
    :HFCConnection(pi_rServer),
     m_UserName(pi_rUserName),
     m_Password(pi_rPassword)
    {
    m_pURL = (HFCURLFile*)HFCURL::Instanciate(pi_rServer);
    }

//-----------------------------------------------------------------------------
// ~HFCLocalConnection
//
// Destructor.
//
//-----------------------------------------------------------------------------
HFCLocalConnection::~HFCLocalConnection()
    {
    // Do nothing
    }

//-----------------------------------------------------------------------------
// bool Connect
//
// Connect to a local drive.
//
// const string& pi_rUserName:
// const string& pi_rPassword:
// time_t pi_TimeOut:
//
//-----------------------------------------------------------------------------
bool HFCLocalConnection::Connect(const WString& pi_rUserName,
                                  const WString& pi_rPassword,
                                  time_t         pi_TimeOut)
    {
    bool  Status = false;

    WString Path(m_pURL->GetHost() + L"\\" + m_pURL->GetPath());

    // Check if the host exist
    if (BeFileName::DoesPathExist(Path.c_str()))
        Status = true;

    SetConnected(Status);

    return Status;
    }

//-----------------------------------------------------------------------------
// void Disconnect
//
// Disconnect from a server.
//
//-----------------------------------------------------------------------------
void HFCLocalConnection::Disconnect()
    {
    SetConnected(false);
    }

//-----------------------------------------------------------------------------
// bool ValidateConnect
//
// Ensure that the connection is alive, if not reconnect.
//
// time_t pi_TimeOut:
//
//-----------------------------------------------------------------------------
bool HFCLocalConnection::ValidateConnect(uint32_t pi_TimeOut)
    {
    return Connect(m_UserName,
                   m_Password,
                   pi_TimeOut);
    }

//-----------------------------------------------------------------------------
// const string& GetUserName
//
// Return the user name.
//
//-----------------------------------------------------------------------------
const WString& HFCLocalConnection::GetUserName()
    {
    return m_UserName;
    }

//-----------------------------------------------------------------------------
// const string& GetPassword
//
// Return the password.
//
//-----------------------------------------------------------------------------
const WString& HFCLocalConnection::GetPassword()
    {
    return m_Password;
    }
