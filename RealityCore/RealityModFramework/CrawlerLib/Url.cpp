/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <CrawlerLib/Url.h>
#include <regex>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;

const std::wregex Url::s_UrlRegex = wregex(L"https?://(?:www\\.)?([^/\\s]+\\.[^/\\s]+)/?(/[\\S]+)?");
const std::wregex Url::s_DomainNameRegex = wregex(L"[^\\s/]+.[^\\s/]+");
const std::wregex Url::s_RelativeUrlRegex = wregex(L"^/[\\S]*$");
const std::wregex Url::s_RelativeUrlWithDotRegex = wregex(L"^.(/[\\S]+)$");

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
WString const& DomainName::GetWString() const
    {
    return m_DomainName;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
UrlPtr Url::Create(WString const& url)
    {
    return Seed::Create(url);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
UrlPtr Url::Create(WString const& url, UrlCR parent)
    {
    return new Url(url, parent);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Url::Url(WString const& url, UrlCR parent)
    : RefCountedBase()
    {
    wcmatch match;
    if (regex_match(url.c_str(), match, s_UrlRegex))
        {
        m_Url = url;
        m_DomainName = WPrintfString(L"%ls", match[1].str().c_str());
        }
    else if (regex_match(url.c_str(), s_RelativeUrlRegex))
        {
        m_Url = WPrintfString(L"http://%ls%ls", parent.GetDomainName().m_DomainName.c_str(), url.c_str());
        m_DomainName = parent.GetDomainName();
        }
    else if (regex_match(url.c_str(), match, s_RelativeUrlWithDotRegex))
        {
        WString parentUrlString = parent.GetUrlWString();
        size_t indexOfLastSlash = parentUrlString.find_last_of(L"/");
        size_t indexOfLastProtocolSlash = parentUrlString.find(L"/", parentUrlString.find(L"/") + 1);
        if (indexOfLastSlash > indexOfLastProtocolSlash)
            {
            m_Url = parentUrlString.substr(0, indexOfLastSlash).append(match[1].str().c_str());
            }
        else
            {
            m_Url = WPrintfString(L"http://%ls%ls", parent.GetDomainName().m_DomainName.c_str(), match[1].str().c_str());
            }
        m_DomainName = parent.GetDomainName();
        }
    else
        {
        throw InvalidUrlException(url);
        }
    RemoveTrailingSlash(m_Url);
    m_Parent = &parent;
    m_Depth = parent.GetDepth() + 1;
    m_IsExternalPage = !(m_DomainName == parent.GetDomainName());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
Url::~Url() {}

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
bool Url::IsSubUrlOf(UrlCR other) const
    {
    if(m_Url.find(other.m_Url) == 0)
        return true;
    else
        return false;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
SeedPtr Seed::Create(WString const& url)
    {
    return new Seed(url);
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
