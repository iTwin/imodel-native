/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/Url.cpp $
| 
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <CrawlerLib/Url.h>
#include <regex>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;

const std::wregex Url::s_UrlRegex = wregex(L"https?://[www\\.]?([^/\\s]+\\.[^/\\s]+)/?(/[\\S]+)?");
const std::wregex Url::s_DomainNameRegex = wregex(L"[^\\s/]+.[^\\s/]+");
const std::wregex Url::s_RelativeUrlRegex = wregex(L"^/[\\S]+$");
const std::wregex Url::s_RelativeUrlWithDotRegex = wregex(L"^.(/[\\S]+)$");

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Url::Url(WString const& url)
    : RefCountedBase()
    {
    wcmatch match;
    if(!regex_match(url.c_str(), match, s_UrlRegex))
        throw InvalidUrlException(url);

    m_Url = url;
    m_DomainName = WPrintfString(L"%ls", match[1].str().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Url::Url(Url const& baseUrl, WString const& relativePath)
    : RefCountedBase()
    {
    wcmatch match;
    if(regex_match(relativePath.c_str(), s_RelativeUrlRegex))
        {
        WPrintfString completeUrl(L"http://%ls%ls", baseUrl.GetDomainName().c_str(), relativePath.c_str());
        m_Url = completeUrl;
        m_DomainName = baseUrl.GetDomainName();
        }
    else if(regex_match(relativePath.c_str(), match, s_RelativeUrlWithDotRegex))
        {
        WString baseUrlString = baseUrl.GetUrlWString();
        size_t index = baseUrlString.find_last_of(L"/");
        m_Url = baseUrlString.substr(0, index).append(match[1].str().c_str());
        m_DomainName = baseUrl.GetDomainName();
        }
    else
        {
        WPrintfString completeUrl(L"http://%ls%ls", baseUrl.GetDomainName().c_str(), relativePath.c_str());
        throw InvalidUrlException(completeUrl);
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Url::Url(Url const& other)
    : RefCountedBase()
    {
    m_Url = other.m_Url;
    m_DomainName = other.m_DomainName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Url& Url::operator=(Url const& other)
    {
    if(this != &other)
        {
        m_Url = other.m_Url;
        m_DomainName = other.m_DomainName;
        }
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool Url::operator==(Url const& other) const
    {
    return m_Url.Equals(other.m_Url);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool Url::operator<(Url const& other) const
    {
    return m_Url.CompareToI(other.m_Url) < 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool Url::IsValidAbsoluteLink(WString const& link)
    {
    return regex_match(link.c_str(), s_UrlRegex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool Url::IsValidRelativeLink(WString const& link)
    {
    return regex_match(link.c_str(), s_RelativeUrlRegex) || regex_match(link.c_str(), s_RelativeUrlWithDotRegex);
    }
