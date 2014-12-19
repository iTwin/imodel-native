//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFSocketConnection.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFSocketConnection
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFSocketConnection.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const string s_Marker("\r\n");


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFSocketConnection::HRFSocketConnection(const HFCURLInternetImagingSocket& pi_rURL,
                                         const WString&                     pi_rUserName,
                                         const WString&                     pi_rPassword)
    : HFCSocketConnection(pi_rURL.GetURL(), pi_rUserName, pi_rPassword),
      m_RequestEnded(false)
    {
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFSocketConnection::~HRFSocketConnection()
    {
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
void HRFSocketConnection::Send(const Byte* pi_pData, size_t pi_DataSize)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_DataSize > 0);


    m_RequestEnded = false;

    // Send the data
    HFCSocketConnection::Send(pi_pData, pi_DataSize);

    // Send the marker
    HFCSocketConnection::Send((const Byte*)s_Marker.data(), s_Marker.size());
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
bool HRFSocketConnection::RequestEnded() const
    {
    return m_RequestEnded;
    }


//-----------------------------------------------------------------------------
// protected section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// protected
// RequestHasEnded
//-----------------------------------------------------------------------------
void HRFSocketConnection::RequestHasEnded(bool pi_Success)
    {
    m_RequestEnded = true;
    }
