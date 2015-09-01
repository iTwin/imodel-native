//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCInternetConnection.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCInternetConnection
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCInternetConnection.h>

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HFCInternetConnection::HFCInternetConnection(const WString& pi_rServer,
                                             const WString& pi_rUserName,
                                             const WString& pi_rPassword)
    : HFCConnection(pi_rServer),
      m_UserName(pi_rUserName),
      m_Password(pi_rPassword),
      m_TimeOut(LONG_MAX)
    {
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFCInternetConnection::~HFCInternetConnection()
    {
    }


//-----------------------------------------------------------------------------
// public
// Partial send.
//-----------------------------------------------------------------------------
void HFCInternetConnection::SendImmediate(const Byte* pi_pData, size_t* pio_pDataSize)
    {
    // By default, do a full send (potentially blocking)
    Send(pi_pData, *pio_pDataSize);
    }


//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
bool HFCInternetConnection::WaitDataWriteable()
    {
    // By default, a connection always accepts new data. In the
    // worse case, the send will block...
    return true;
    }


//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
bool HFCInternetConnection::WaitDataWriteable(uint32_t pi_TimeOut)
    {
    // By default, a connection always accepts new data. In the
    // worse case, the send will block...
    return true;
    }