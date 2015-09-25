/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/Politeness.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "Politeness.h"
#include <limits>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Politeness::Politeness(IRobotsTxtDownloader* downloader)
    : m_UserAgent(L"*")
    {
    BeAssert(downloader != NULL);

    m_pDownloader = downloader;
    m_RespectRobotsTxt = false;
    m_RespectRobotsTxtIfDisallowRoot = true;
    m_MaxCrawlDelayInSeconds = (numeric_limits<uint32_t>::max)();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Politeness::~Politeness()
    {
    delete m_pDownloader;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool Politeness::CanDownloadUrl(UrlPtr const& url)
    {
    BeAssert(url != NULL);

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
        return !IsContentDisallowUrl(iterator->second, url);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool Politeness::IsContentDisallowUrl(RobotsTxtContentPtr const& content, UrlPtr const& url) const
    {
    BeAssert(content != NULL);
    BeAssert(url != NULL);

    if(content->IsRootDisallowed(m_UserAgent) && !m_RespectRobotsTxtIfDisallowRoot)
        return false;

    return content->IsUrlDisallowed(url, m_UserAgent);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Politeness::DownloadRobotsTxt(UrlPtr const& url)
    {
    BeAssert(url != NULL);

    RobotsTxtContentPtr content = m_pDownloader->DownloadRobotsTxt(url);
    m_RobotsTxtFilesPerDomain.emplace(url->GetDomainName(), content);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t Politeness::GetCrawlDelay(UrlPtr const& url)
    {
    BeAssert(url != NULL);

    if(m_RespectRobotsTxt)
        {
        auto iterator = m_RobotsTxtFilesPerDomain.find(url->GetDomainName());
        if(iterator == m_RobotsTxtFilesPerDomain.end())
            {
            DownloadRobotsTxt(url);
            iterator = m_RobotsTxtFilesPerDomain.find(url->GetDomainName());
            }
        RobotsTxtContentPtr content = iterator->second;
        return (std::min)(m_MaxCrawlDelayInSeconds, content->GetCrawlDelay(m_UserAgent));
        }
    return 0;
    }
