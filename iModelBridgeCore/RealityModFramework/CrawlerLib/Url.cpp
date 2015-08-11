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

const std::wregex Url::s_UrlRegex = wregex(L"https?://(?:www\\.)?([^/\\s]+\\.[^/\\s]+)/?(/[\\S]+)?");
const std::wregex Url::s_DomainNameRegex = wregex(L"[^\\s/]+.[^\\s/]+");
const std::wregex Url::s_RelativeUrlRegex = wregex(L"^/[\\S]*$");
const std::wregex Url::s_RelativeUrlWithDotRegex = wregex(L"^.(/[\\S]+)$");

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Url::Url(WString const& url, UrlPtr const& parent)
    : RefCountedBase()
    {
    wcmatch match;
    if(regex_match(url.c_str(), match, s_UrlRegex))
        {
        m_Url = url;
        m_DomainName = WPrintfString(L"%ls", match[1].str().c_str());
        }
    else if(regex_match(url.c_str(), s_RelativeUrlRegex))
        {
        m_Url = WPrintfString(L"http://%ls%ls", parent->GetDomainName().c_str(), url.c_str());
        m_DomainName = parent->GetDomainName();
        }
    else if(regex_match(url.c_str(), match, s_RelativeUrlWithDotRegex))
        {
        WString parentUrlString = parent->GetUrlWString();
        size_t indexOfLastSlash = parentUrlString.find_last_of(L"/");
        size_t indexOfLastProtocolSlash = parentUrlString.find(L"/", parentUrlString.find(L"/") + 1);
        if(indexOfLastSlash > indexOfLastProtocolSlash)
            {
            m_Url = parentUrlString.substr(0, indexOfLastSlash).append(match[1].str().c_str());
            }
        else
            {
            m_Url = WPrintfString(L"http://%ls%ls", parent->GetDomainName().c_str(), match[1].str().c_str());
            }
        m_DomainName = parent->GetDomainName();
        }
    else
        {
        throw InvalidUrlException(url);
        }
    RemoveTrailingSlash(m_Url);
    m_Parent = parent;
    m_Depth = parent->GetDepth() + 1;
    m_IsExternalPage = !m_DomainName.Equals(parent->GetDomainName());
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
void Url::RemoveTrailingSlash(WString& urlString) const
    {
    size_t size = urlString.size();
    wchar_t lastChar = urlString.at(size - 1);
    if(lastChar == L'/' || lastChar == L'\\')
        urlString = urlString.substr(0, size - 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool Url::IsSubUrlOf(Url const& other)
    {
    if(m_Url.find(other.m_Url) == 0)
        return true;
    else
        return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Seed::Seed(WString const& url)
    : Url()
    {
    wcmatch match;
    if(!regex_match(url.c_str(), match, s_UrlRegex))
        throw InvalidUrlException(url);

    m_Url = url;
    m_DomainName = WPrintfString(L"%ls", match[1].str().c_str());
    m_Parent = NULL;
    m_Depth = 0;
    m_IsExternalPage = false;
    }
