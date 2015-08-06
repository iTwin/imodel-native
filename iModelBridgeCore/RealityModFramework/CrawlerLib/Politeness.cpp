/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/Politeness.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <CrawlerLib/Politeness.h>
#include <limits>
#include <chrono>
#include <thread>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Politeness::Politeness(IPageDownloader* downloader, IRobotsTxtParser* parser, ISleeper* sleeper)
    : m_UserAgent(L"*")
    {
    m_pDownloader = downloader;
    m_pParser = parser;
    m_pSleeper = sleeper;
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
    delete m_pParser;
    delete m_pSleeper;
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
    WString buffer;
    UrlPtr robotsTxtUrl = new Url(L"/robots.txt", url);
    UrlPtr baseUrl = new Url(L"/", url);
    StatusInt downloadStatus = m_pDownloader->DownloadPage(buffer, robotsTxtUrl);
    if(downloadStatus == SUCCESS)
        {
        RobotsTxtContentPtr content = m_pParser->ParseRobotsTxt(buffer, baseUrl);
        m_RobotsTxtFilesPerDomain.emplace(robotsTxtUrl->GetDomainName(), content);
        }
    else
        {
        RobotsTxtContentPtr content = new RobotsTxtContent(L"", baseUrl);
        m_RobotsTxtFilesPerDomain.emplace(robotsTxtUrl->GetDomainName(), content);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Politeness::WaitToRespectCrawlDelayOf(UrlPtr const& url)
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
        uint32_t delay = (std::min)(m_MaxCrawlDelay, content->GetCrawlDelay(m_UserAgent));
        m_pSleeper->Sleep(delay);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Sleeper::Sleep(uint32_t seconds) const
    {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    }
