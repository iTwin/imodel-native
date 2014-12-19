//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetHIPRedirectHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetHIPRedirectHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetHIPRedirectHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>
#include <Imagepp/all/h/HRFHTTPConnection.h>
#include <Imagepp/all/h/HRFSocketConnection.h>
#include <Imagepp/all/h/HFCEvent.h>
#include <Imagepp/all/h/HFCURLHTTP.h>
#include <Imagepp/all/h/HFCURLInternetImagingSocket.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("hip-redirect");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetHIPRedirectHandler::HRFInternetHIPRedirectHandler()
    : HRFInternetASCIIHandler("hip-redirect")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetHIPRedirectHandler::~HRFInternetHIPRedirectHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetHIPRedirectHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                           HFCBuffer&              pio_rBuffer,
                                           HFCInternetConnection&  pi_rConnection)
    {
    //This code is not expected to be called. The whole redirection concept seems to
    //have never been finished.
    HASSERT(0);

    // Scan the connection untilt the end marker
    ReadUntilMarker(pi_rConnection, pio_rBuffer);

    //
    // The format of the hip-redirect response is
    //
    // hip-redirect:address:port
    //

    // Build a string around the URI
    string URI((const char*)pio_rBuffer.GetData() + s_Label.size() + 1,
               pio_rBuffer.GetDataSize() - s_Label.size() - 1);

    // find the colon
    string::size_type Pos = URI.find(' ');
    if (Pos == string::npos)
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::HIP_REDIRECT);

    string Address(URI.substr(0, Pos));
    uint32_t Port = atol(URI.substr(Pos + 1).c_str());

    // Tell the internet image to reconnect to this address
    if (pi_rFile.GetProtocolVersion().GetNumber(2) == HRFInternetImagingFile::s_HIP_UTF8_Protocol)
        {
        WString tempoStr;
        BeStringUtilities::Utf8ToWChar(tempoStr,Address.c_str());

        SetAlternateServer(pi_rFile, tempoStr, (unsigned short)Port);
        }
    else
        {
        WString tempoStr;
        BeStringUtilities::CurrentLocaleCharToWChar( tempoStr,Address.c_str());

        SetAlternateServer(pi_rFile, tempoStr, (unsigned short)Port);
        }



    // Signal that a redirection has occured
    pi_rFile.m_Redirected = true;
    pi_rFile.m_RedirectEvent.Signal();
    }


//-----------------------------------------------------------------------------
// Protected
//
//-----------------------------------------------------------------------------
void HRFInternetHIPRedirectHandler::SetAlternateServer(HRFInternetImagingFile& pi_rFile,
                                                       const WString&          pi_rHost,
                                                       unsigned short         pi_Port)
    {
    //This code is not expected to be called so it hasn`t been modified during
    //the url encoding redesign task.
    HASSERT(0);

    HPRECONDITION(!pi_rHost.empty());
    HPRECONDITION(pi_Port > 0);
    HFCMonitor Monitor(pi_rFile.m_ConnectionKey);
    HFCPtr<HFCURL> pURL;

    // The connection is HTTP
    if (pi_rFile.m_pConnection->GetClassID() == HRFHTTPConnection::CLASS_ID)
        {
        // Build a URL from with this port and the file name
        WChar   Temp[33];// see _ltow() doc for 33
        WString URLString(HFCURLHTTP::s_SchemeName() + L"://");
        URLString += pi_rHost;
        URLString += L":";
        URLString += _ltow(pi_Port, Temp, 10);
        URLString += L"/?fif=";
        URLString += pi_rFile.m_ImageName;
        pURL = HFCURL::Instanciate(URLString);
        if (pURL == 0)
            throw HRFInternetImagingException(HRFII_INVALID_REDIRECTION_URL_EXCEPTION, pi_rFile.GetURL()->GetURL());
        }

    // The connection is IIP
    else
        {
        // Build a URL from with this port and the file name
        WChar Temp[33];// see _ltoa() doc for 33
        WString URLString(HFCURLInternetImagingSocket::s_SchemeName() + L" ://");
        URLString += pi_rHost;
        URLString += L":";
        URLString += _ltow(pi_Port, Temp, 10);
        URLString += L"/";
        URLString += pi_rFile.m_ImageName;
        pURL = HFCURL::Instanciate(URLString);
        if (pURL == 0)
            throw HRFInternetImagingException(HRFII_INVALID_REDIRECTION_URL_EXCEPTION, pi_rFile.GetURL()->GetURL());
        }

    // Reconnect to a new server
    pi_rFile.m_pDisconnectorThread->AddConnection(pi_rFile.m_pConnection .release());
    pi_rFile.ConnectToHost(pURL);

    // Build the connection request
    WString Request(L"fif=");
    Request += pi_rFile.m_ImageName;
    Request += L"&obj=hip,1.0";

    // In HTTP, set the file prefix for future request
    if (pi_rFile.m_pConnection->GetClassID() == HRFHTTPConnection::CLASS_ID)
        {
        // if the file is constructing, send the request to obtain information
        if (pi_rFile.m_IsConstructing == true)
            {
            if (pi_rFile.GetProtocolVersion().GetNumber(2) == HRFInternetImagingFile::s_HIP_UTF8_Protocol)
                {
                Utf8String utf8Data(Request.c_str());
                pi_rFile.m_pConnection->Send((const Byte*)utf8Data.data(), utf8Data.size());
                }
            else
                {
                size_t  destinationBuffSize = Request.GetMaxLocaleCharBytes();
                char*   pData = (char*)_alloca (destinationBuffSize);

                Request.ConvertToLocaleChars(pData, destinationBuffSize);

                pi_rFile.m_pConnection->Send((const Byte*)pData, strlen(pData));
                }            
            }

        // Set the prefix for requests
        //TBD - The search base should be encoded according to the protocol use and escaped.
        //      Since this code is not expected to be called, no modifications were deemed
        //      necessary.
        HASSERT(0);
        //static_cast<HRFHTTPConnection*>(pi_rFile.m_pConnection.get())->SetSearchBase(Request.c_str());
        }

    // in IIP, send the request so that future request may work on that image
    else
        {
        // if we are not constructing a new IIP file, add a "&" so that the next
        // request is appended in one request
        if (pi_rFile.m_IsConstructing == false)
            Request += L"&obj=aspect-ratio";

        if (pi_rFile.GetProtocolVersion().GetNumber(2) == HRFInternetImagingFile::s_HIP_UTF8_Protocol)
            {
            Utf8String utf8Data(Request.c_str());
            pi_rFile.m_pConnection->Send((const Byte*)utf8Data.data(), utf8Data.size());
            }
        else
            {
            size_t  destinationBuffSize = Request.GetMaxLocaleCharBytes();
            char*   pData= (char*)_alloca (destinationBuffSize);

            Request.ConvertToLocaleChars(pData, destinationBuffSize);

            pi_rFile.m_pConnection->Send((const Byte*)pData, strlen(pData));
            }
        }
    }