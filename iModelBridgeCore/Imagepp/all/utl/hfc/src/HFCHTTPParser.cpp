//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCHTTPParser.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCHTTPParser (string should be in UTF-8)
//-----------------------------------------------------------------------------
// HFCHTTPParser.cpp : source file
//-----------------------------------------------------------------------------

//################################
// INCLUDE FILES
//################################

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCHTTPParser.h>
#include <Imagepp/all/h/HFCStringTokenizer.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCEncodeDecodeASCII.h>


//-----------------------------------------------------------------
// Constants
//-----------------------------------------------------------------

static const WString s_HTTPVersionTag(L"http/");


//-----------------------------------------------------------------
// Static members initialization
//-----------------------------------------------------------------

const WString HFCHTTPParser::s_GetTag(L"get");
const WString HFCHTTPParser::s_PostTag(L"post");
const WString HFCHTTPParser::s_HeadTag(L"head");
const WString HFCHTTPParser::s_Empty(L"");


//-----------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------
HFCHTTPParser::HFCHTTPParser(const WString& pi_Request,
                             bool          pi_UTF8Request)
    : m_Request(pi_Request),
      m_UTF8Request(pi_UTF8Request)
    {
    HPRECONDITION(!m_Request.empty());
    m_HTTPMethod = HFCHTTPParser::UNKNOWN;
    }


//-----------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------
HFCHTTPParser::HFCHTTPParser(const HFCHTTPParser& pi_rObj)
    : m_Request(pi_rObj.m_Request),
      m_HTTPMethod(pi_rObj.m_HTTPMethod),
      m_SearchPart(pi_rObj.m_SearchPart),
      m_SearchPartURLEncoded(pi_rObj.m_SearchPartURLEncoded),
      m_Version(pi_rObj.m_Version),
      m_AdditionalHeaders(pi_rObj.m_AdditionalHeaders)
    {
    }


//-----------------------------------------------------------------
// public
// Destroyer
//-----------------------------------------------------------------
HFCHTTPParser::~HFCHTTPParser()
    {
    }


//-----------------------------------------------------------------
// public
// Copy Operator
//-----------------------------------------------------------------
HFCHTTPParser& HFCHTTPParser::operator=(const HFCHTTPParser& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_Request           = pi_rObj.m_Request;
        m_HTTPMethod        = pi_rObj.m_HTTPMethod;
        m_SearchPart        = pi_rObj.m_SearchPart;
        m_SearchPartURLEncoded = pi_rObj.m_SearchPartURLEncoded;
        m_Version           = pi_rObj.m_Version;
        m_AdditionalHeaders = pi_rObj.m_AdditionalHeaders;
        }

    return (*this);
    }



//-----------------------------------------------------------------
// Public
// Parse the information in the m_Request attribute
//
// Format of a HTTP request:
//
// METHOD search HTTP/version\r\n
// header1 : value1\r\n
// header2 : value1\r\n
// ...
// headerN : valueN\r\n
// \r\n
//-----------------------------------------------------------------
bool HFCHTTPParser::Parse()
    {
    HPRECONDITION(!m_Request.empty());
    bool Result = false;

    try
        {
        // If the request does not end with "\r\n\r\n", then it is not a valid
        // HTTP request.
        if(m_Request.size() < 4 || 0 != m_Request.substr(m_Request.length()-4).compare(L"\r\n\r\n"))
            throw HFCException(HFC_INVALID_HTTP_REQUEST_STR_EXCEPTION);

        // parse the lines in the request
        HFCStringTokenizer LineTokenizer(m_Request, L"\r\n", true);
        WString Line;
        bool FirstLine = true;
        while (LineTokenizer.Tokenize(Line))
            {
            // the tokenizer gets the tokens in betwee the \r and the \n so we ignore empty lines.
            if (!Line.empty())
                {
                if (FirstLine)
                    {
                    // make a lower case copy of the line to work on. (Will be used only to
                    // search for information
                    WString LCLine(Line);
                    CaseInsensitiveStringTools().ToLower(LCLine);

                    // for the first line, extract the Method, object, search and version
                    WString::size_type SpacePos = LCLine.find(' ');
                    if (SpacePos == WString::npos)
                        throw HFCException(HFC_INVALID_HTTP_REQUEST_STR_EXCEPTION);
                    WString Method(LCLine, 0, SpacePos);
                    if (Method.compare(s_GetTag) == 0)
                        m_HTTPMethod = HFCHTTPParser::GET;
                    else if(Method.compare(s_HeadTag) == 0)
                        m_HTTPMethod = HFCHTTPParser::HEAD;
                    else if(Method.compare(s_PostTag) == 0)
                        m_HTTPMethod = HFCHTTPParser::POST;
                    else
                        throw HFCException(HFC_INVALID_HTTP_REQUEST_STR_EXCEPTION);

                    // find the version in the line
                    WString::size_type VersionPos = LCLine.find(s_HTTPVersionTag);
                    if (VersionPos != WString::npos)
                        {
                        wistringstream VersionStream(LCLine.substr(VersionPos + s_HTTPVersionTag.size()).c_str());
                        double VersionValue;
                        VersionStream >> VersionValue;
                        double Major, Minor;
                        Minor = modf(VersionValue, &Major) * 10.0;
                        m_Version = HFCVersion(L"", L"", 2, (int32_t)Major, (int32_t)Minor);

                        // the search part is everything that lies between the method and the version
                        m_SearchPartURLEncoded = Line.substr(SpacePos, VersionPos - SpacePos);
                        }
                    else
                        {
                        m_Version = HFCVersion(L"", L"", 2, 1, 0);

                        m_SearchPartURLEncoded = Line.substr(SpacePos);
                        }

                    // the search part is everything that lies between the method and the version
                    HFCStringTokenizer::Trim(m_SearchPartURLEncoded);

                    // decode the search part and save the decoded string
                    m_SearchPart = m_SearchPartURLEncoded;
                    HFCEncodeDecodeASCII::EscapeToASCII(m_SearchPart, m_UTF8Request);

                    // the first line is parse, now we will handle headers
                    FirstLine = false;
                    }
                else
                    {
                    // find the colon in the string (it separated the header from its value
                    WString::size_type ColonPos = Line.find(':');

                    // extract the 2 parts fromthe current header
                    WString HeaderName(Line, 0, ColonPos != WString::npos ? ColonPos : Line.size());
                    WString HeaderValue;
                    if (ColonPos != WString::npos)
                        HeaderValue = Line.substr(ColonPos + 1);
                    else
                        HeaderValue = L"";
                    HFCStringTokenizer::Trim(HeaderName);
                    HFCStringTokenizer::Trim(HeaderValue);

                    // force header name (which is a map key) to lower case.
                    WString LCHeaderName(HeaderName);
                    CaseInsensitiveStringTools().ToLower(LCHeaderName);

                    // add the current header to the header map
                    m_AdditionalHeaders.insert(Headers::value_type(LCHeaderName, HeaderValue));
                    }
                }
            }

        Result = true;
        }
    catch(...)
        {
        Result = false;
        }

    return Result;
    }

//-----------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------
void HFCHTTPParser::SetSearchPartURLEncoded(WString& pi_rSearchPart)
    {
    WString::size_type first = m_Request.find(m_SearchPartURLEncoded);
    if (first != WString::npos)
        {
        WString::size_type len = m_SearchPartURLEncoded.size();
        m_SearchPartURLEncoded = pi_rSearchPart;
        m_Request.replace(first, len, m_SearchPartURLEncoded);
        Parse();
        }
    }


//-----------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------
void HFCHTTPParser::InsertHeaderInRequest(const WString& pi_rHeader)
    {
    WString::size_type EndRequest = m_Request.find (L"\r\n\r\n");

    HASSERT (WString::npos != EndRequest);

    if (WString::npos != EndRequest)
        {
        m_Request.insert (EndRequest, WString(L"\r\n") + pi_rHeader);
        Parse();
        }
    }
