//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCHTTPHeader.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCHTTPHeader
//-----------------------------------------------------------------------------

//###############################
// INCLUDE FILES
//###############################

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCHTTPHeader.h>


//###############################
// CONSTANTS
//###############################

// HTTP messages
static const WString HTTP_STATUS_MSG[] = {L"100 CONTINUE\r\n",
                                          L"101 SWITCHING PROTOCOLS\r\n",
                                          L"200 OK\r\n",
                                          L"201 CREATED\r\n",
                                          L"202 ACCEPTED\r\n",
                                          L"203 NON-AUTHORITATIVE INFORMATION\r\n",
                                          L"204 NO CONTENT\r\n",
                                          L"205 RESET CONTENT\r\n",
                                          L"206 PARTIAL CONTENT\r\n",
                                          L"300 MULTIPLE CHOICES\r\n",
                                          L"301 MOVED PERMANENTLY\r\n",
                                          L"302 FOUND\r\n",
                                          L"303 SEE OTHER\r\n",
                                          L"304 NOT MODIFIED\r\n",
                                          L"305 USE PROXY\r\n",
                                          L"307 TEMPORARY REDIRECT\r\n",
                                          L"400 BAD REQUEST\r\n",
                                          L"401 UNAUTORIZED\r\n",
                                          L"402 PAYMENT REQUIRED\r\n",
                                          L"403 FORBIDDEN\r\n",
                                          L"404 NOT FOUND\r\n",
                                          L"405 METHOD NOT ALLOWED\r\n",
                                          L"406 NOT ACCEPTABLE\r\n",
                                          L"407 PROXY AUTHENTICATION REQUIRED\r\n",
                                          L"408 REQUEST TIMEOUT\r\n",
                                          L"409 CONFLICT\r\n",
                                          L"410 GONE\r\n",
                                          L"411 LENGTH REQUIRED\r\n",
                                          L"412 PRECONDITION FAILED\r\n",
                                          L"413 REQUEST ENTITY TOO LARGE\r\n",
                                          L"414 REQUEST-URI TOO LONG\r\n",
                                          L"415 UNSUPPORTED MEDIA TYPE\r\n",
                                          L"416 REQUEST RANGE NOT SATISFIABLE\r\n",
                                          L"417 EXPECTATION FAILED\r\n",
                                          L"426 UPGRADE REQUIRED\r\n",
                                          L"500 INTERNAL SERVER ERROR\r\n",
                                          L"501 NOT IMPLEMENTED\r\n",
                                          L"502 BAD GATEWAY\r\n",
                                          L"503 SERVICE UNAVAILABLE\r\n",
                                          L"504 GATEWAY TIMEOUT\r\n",
                                          L"505 VERSION NOT SUPPORTED\r\n"
                                         };



//-----------------------------------------------------------------
// HFCHTTPHeader
//
//
//-----------------------------------------------------------------
HFCHTTPHeader::HFCHTTPHeader(double pi_Version)
    {
    m_Status  = _MAX_STATUS_CODE;
    m_Version = pi_Version;
    m_HeaderComplete = true;
    }

HFCHTTPHeader::HFCHTTPHeader(const HFCHTTPHeader& pi_rHeader)
    {
    m_Header = pi_rHeader.m_Header;
    m_Status = pi_rHeader.m_Status;
    m_Version = pi_rHeader.m_Version;
    m_HeaderComplete = pi_rHeader.m_HeaderComplete;
    }

//-----------------------------------------------------------------
// ~HFCHTTPHeader
//
//
//-----------------------------------------------------------------
HFCHTTPHeader::~HFCHTTPHeader()
    {
    }

//-----------------------------------------------------------------
// bool SetStatusCode
//
// HHTTPStatus pi_StatusCode:
//
//-----------------------------------------------------------------
bool HFCHTTPHeader::SetStatusCode(HHTTPStatus pi_StatusCode)
    {
    bool Status = false;

    if( pi_StatusCode < _MAX_STATUS_CODE )
        {
        m_Status = pi_StatusCode;
        Status   = true;
        }

    return Status;
    }

HFCHTTPHeader::HHTTPStatus HFCHTTPHeader::GetStatusCode() const
    {
    return m_Status;
    }

//-----------------------------------------------------------------
// void AddToHeader
//
// const WString& pi_rHeader:
//
//-----------------------------------------------------------------
void HFCHTTPHeader::AddToHeader(const WString& pi_rHeader)
    {
    m_Header += pi_rHeader;

    // A header line must end by a \r\n
    // Check to be sure that there is one. If no one
    // is found add it.
    if( pi_rHeader.find(L"\r\n") == WString::npos )
        m_Header += L"\r\n";
    }

//-----------------------------------------------------------------
// WString GetHeader
//
//
//-----------------------------------------------------------------
WString HFCHTTPHeader::GetHeader() const
    {
    WString Header;

    if( m_Status < _MAX_STATUS_CODE )
        {
        WChar Version[255];
        BeStringUtilities::Snwprintf(Version, L"%.1f ", m_Version);

        Header = L"HTTP/";
        Header += Version;
        Header += HTTP_STATUS_MSG[m_Status];
        Header += m_Header;
        if (m_HeaderComplete)
            Header += L"\r\n";
        }

    return Header;
    }

//-----------------------------------------------------------------
// string GetUTF8Header
//
//
//-----------------------------------------------------------------
string HFCHTTPHeader::GetUTF8Header() const
    {
    string Header;

    if( m_Status < _MAX_STATUS_CODE )
        {
        char Version[255];
        sprintf(Version, "%.1f ", m_Version);

        Header = "HTTP/";
        Header += Version;
        Utf8String utf8Str;
        BeStringUtilities::WCharToUtf8(utf8Str,HTTP_STATUS_MSG[m_Status].c_str());

        Header += string(utf8Str.c_str());
        BeStringUtilities::WCharToUtf8(utf8Str,m_Header.c_str());

        Header += string(utf8Str.c_str());

        if (m_HeaderComplete)
            Header += "\r\n";
        }

    return Header;
    }

//-----------------------------------------------------------------
// void ClearHeader
//
//
//-----------------------------------------------------------------
void HFCHTTPHeader::ClearHeader()
    {
    m_Header = L"";
    m_Status = _MAX_STATUS_CODE;
    m_HeaderComplete = true;
    }

//-----------------------------------------------------------------
// void HeaderIsComplete
//
//
//-----------------------------------------------------------------
void HFCHTTPHeader::HeaderIsComplete(bool pi_Complete)
    {
    m_HeaderComplete = pi_Complete;
    }


//-----------------------------------------------------------------
// void GetStatusCodeString
//
//
//-----------------------------------------------------------------
WString HFCHTTPHeader::GetStatusCodeString() const
    {
    // Constructed locally to make an append so that the static
    // strings are not copy-constructed.
    WString Result;

    if( m_Status < _MAX_STATUS_CODE )
        Result += HTTP_STATUS_MSG[m_Status];

    return Result;
    }
