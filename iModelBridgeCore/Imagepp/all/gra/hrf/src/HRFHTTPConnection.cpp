//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFHTTPConnection
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFHTTPConnection.h>
#include <Imagepp/all/h/HFCThread.h>
#include <Imagepp/all/h/HFCExclusiveKey.h>
#include <Imagepp/all/h/HFCEvent.h>
#include <Imagepp/all/h/HRFURLInternetImagingHTTP.h>


//-----------------------------------------------------------------------------
// constants
//-----------------------------------------------------------------------------

static const string s_Marker("\r\n");


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFHTTPConnection::HRFHTTPConnection(const HRFURLInternetImagingHTTP& pi_rURL,
                                     const WString&                   pi_rUserName,
                                     const WString&                   pi_rPassword)
    : HFCHTTPConnection(pi_rURL.GetURL(), pi_rUserName, pi_rPassword),
      m_RequestEnded(false)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFHTTPConnection::~HRFHTTPConnection()
    {
    }

void HRFHTTPConnection::Send(const Byte* pi_pData, size_t pi_DataSize)
    {
    m_RequestEnded = false;

    HASSERT_X64(pi_DataSize < ULONG_MAX);
    HFCHTTPConnection::Send(pi_pData, pi_DataSize);
    }


//-----------------------------------------------------------------------------
// public
// RequestEnded
//-----------------------------------------------------------------------------
bool HRFHTTPConnection::RequestEnded() const
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
void HRFHTTPConnection::RequestHasEnded(bool pi_Success)
    {
    if (pi_Success)
        {
        HFCMonitor Monitor(m_BufferKey);
        m_Buffer.AddData((const Byte*)s_Marker.data(), s_Marker.size());
        }
    m_RequestEnded = true;
    }
