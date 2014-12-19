//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCHTTPParser.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCHTTPParser
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------
inline const HFCVersion& HFCHTTPParser::GetHTTPVersion() const
    {
    return m_Version;
    }


//-----------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------
inline uint32_t HFCHTTPParser::GetHTTPMajorVersion() const
    {
    return m_Version.GetNumber(0);
    }


//-----------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------
inline uint32_t HFCHTTPParser::GetHTTPMinorVersion() const
    {
    return m_Version.GetNumber(1);
    }


//-----------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------
inline HFCHTTPParser::HTTPMethod HFCHTTPParser::GetMethod() const
    {
    return m_HTTPMethod;
    }


//-----------------------------------------------------------------------------
// Public
// Get the method as a string
//-----------------------------------------------------------------------------
inline const WString& HFCHTTPParser::GetMethodStr() const
    {
    switch(m_HTTPMethod)
        {
        case HFCHTTPParser::GET:
            return s_GetTag;
            break;
        case HFCHTTPParser::POST:
            return s_PostTag;
            break;
        case HFCHTTPParser::HEAD:
            return s_HeadTag;
            break;
        default:
            return s_Empty;
            break;
        }
    }


//-----------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------
inline const WString& HFCHTTPParser::GetSearchPart() const
    {
    return m_SearchPart;
    }

//-----------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------
inline const WString& HFCHTTPParser::GetSearchPartURLEncoded() const
    {
    return m_SearchPartURLEncoded;
    }


//-----------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------
inline const WString& HFCHTTPParser::GetRequest() const
    {
    return m_Request;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline const WString& HFCHTTPParser::GetHeader(const WString& pi_rHeader) const
    {
    // the header map contains lower-case header names as keys, so map the search
    // name to lower case
    WString LCHeader(pi_rHeader);

    CaseInsensitiveStringTools().ToLower(LCHeader);

    Headers::const_iterator Itr(m_AdditionalHeaders.find(LCHeader));
    if (Itr != m_AdditionalHeaders.end())
        return Itr->second;
    else
        return s_Empty;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline const HFCHTTPParser::Headers& HFCHTTPParser::GetHeaders() const
    {
    return m_AdditionalHeaders;
    }

