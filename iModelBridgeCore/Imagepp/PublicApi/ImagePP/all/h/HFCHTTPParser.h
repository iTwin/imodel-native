//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCHTTPParser.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCHTTPParser
//-----------------------------------------------------------------------------
// HFCHTTPParser.h : header file
//-----------------------------------------------------------------------------

#pragma once

#include "HFCVersion.h"

class HFCHTTPParser
    {
public:
    //--------------------------------------
    // Types
    //--------------------------------------

    // HTTP Methods
    enum HTTPMethod
        {
        GET,
        POST,
        HEAD,
        UNKNOWN
        };

    // a map of all the additional headers in the request
    typedef map<WString, WString>
    Headers;


    //--------------------------------------
    // Construction - destruction
    //--------------------------------------

    HFCHTTPParser(const WString& pi_Request, bool pi_UTF8Request = false);
    HFCHTTPParser(const HFCHTTPParser& pi_rObj);
    virtual         ~HFCHTTPParser();

    HFCHTTPParser&  operator=(const HFCHTTPParser& pi_rObj);


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Parse the header given as a string during contruction
    bool           Parse();

    // Return the version of the HTTP request
    const HFCVersion&
    GetHTTPVersion() const;
    uint32_t         GetHTTPMajorVersion() const;
    uint32_t         GetHTTPMinorVersion() const;

    // return the HTTP method as an enum and as a string
    HTTPMethod      GetMethod() const;
    const WString&  GetMethodStr() const;

    // return part of the HTTP request
    const WString&  GetSearchPart() const;
    const WString&  GetSearchPartURLEncoded() const;
    const WString&  GetRequest() const;

    // return the addtional headers from the parser
    const Headers&  GetHeaders() const;
    const WString&  GetHeader(const WString& pi_rHeader) const;

    // Add a header to the request
    void            InsertHeaderInRequest(const WString& pi_rHeader);

    // set the SearchPart in case we want to modify it
    void            SetSearchPartURLEncoded(WString& pi_rSearchPart);

private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // string value for the methods
    static const WString s_GetTag;
    static const WString s_HeadTag;
    static const WString s_PostTag;
    static const WString s_UnknownTag;
    static const WString s_Empty;

    // The whole request given at construct time
    WString         m_Request;
    bool           m_UTF8Request;

    // The HTTP method in the request
    HTTPMethod      m_HTTPMethod;

    // the search part in the request (URL decoded and encoded forms)
    WString         m_SearchPart;
    WString         m_SearchPartURLEncoded;

    // The HTTP version in the request
    HFCVersion      m_Version;

    // a map of all the additional headers in the request
    Headers         m_AdditionalHeaders;
    };

#include "HFCHTTPParser.hpp"

