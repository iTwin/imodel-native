/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/Politeness.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <CrawlerLib/Politeness.h>
#include <limits>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Politeness::Politeness(IRobotsTxtDownloader* downloader)
    : m_UserAgent(L"*")
    {
    m_pDownloader = downloader;
    m_RespectRobotsTxt = false;
    m_RespectRobotsTxtIfDisallowRoot = true;
    m_MaxCrawlDelay = (numeric_limits<uint32_t>::max)();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Politeness::~Politeness()
    {
    delete m_pDownloader;
    }

bool Politeness::CanDownloadUrl(UrlPtr const& url)
    {
    if(!m_RespectRobotsTxt)
        return true;
    else
        {
        auto iterator = m_RobotsTxtFilesPerDomain.find(url->GetDomainName());
        if(iterator == m_RobotsTxtFilesPerDomain.end())
            {
            DownloadRobotsTxt(url);
            iterator = m_RobotsTxtFilesPerDomain.find(url->GetDomainName());
            }
        return IsContentDisallowUrl(iterator->second, url);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool Politeness::IsContentDisallowUrl(RobotsTxtContentPtr const& content, UrlPtr const& url) const
    {
    if(content->IsRootDisallowed(m_UserAgent) && !m_RespectRobotsTxtIfDisallowRoot)
        return true;

    UrlPtrSet disallowedUrls;
    content->GetDisallowedUrlsOf(disallowedUrls, m_UserAgent);
    for(auto pUrl : disallowedUrls)
        {
        if(url->IsSubUrlOf(*pUrl))
            return false;
        }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Politeness::DownloadRobotsTxt(UrlPtr const& url)
    {
    RobotsTxtContentPtr content = m_pDownloader->DownloadRobotsTxt(url);
    m_RobotsTxtFilesPerDomain.emplace(url->GetDomainName(), content);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t Politeness::GetCrawlDelay(UrlPtr const& url)
    {
    if(m_RespectRobotsTxt)
        {
        auto iterator = m_RobotsTxtFilesPerDomain.find(url->GetDomainName());
        if(iterator == m_RobotsTxtFilesPerDomain.end())
            {
            DownloadRobotsTxt(url);
            iterator = m_RobotsTxtFilesPerDomain.find(url->GetDomainName());
            }
        RobotsTxtContentPtr content = iterator->second;
        return (std::min)(m_MaxCrawlDelay, content->GetCrawlDelay(m_UserAgent));
        }
    return 0;
    }
