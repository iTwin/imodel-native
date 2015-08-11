/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/Crawler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <CrawlerLib/Crawler.h>
#include <curl/curl.h>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;

const chrono::milliseconds Crawler::s_AsyncWaitTime = chrono::milliseconds(75);

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Crawler::Crawler(UrlQueue* queue, std::vector<IPageDownloader*> const& downloaders)
    : m_NumberOfDownloaders(downloaders.size())
    {
    curl_global_init(CURL_GLOBAL_ALL);
    m_pDownloaders = downloaders;
    m_pQueue = queue;
    m_pObserver = NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Crawler::~Crawler()
    {
    for (auto downloader : m_pDownloaders)
        delete downloader;
    delete m_pQueue;
    curl_global_cleanup();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt Crawler::Crawl(UrlPtr const& seed)
    {
    m_pQueue->AddUrl(seed);

    vector<future<PageContentPtr>> asyncResults(m_NumberOfDownloaders);
    size_t i = 0;
    while(m_pQueue->NumberOfUrls() > 0 || !AllDownloadsFinished(asyncResults))
        {
        if(!asyncResults[i].valid() && m_pQueue->NumberOfUrls() > 0) //After default construction, std::future is not valid
            {
            DownloadJobPtr job = m_pQueue->NextDownloadJob();
            asyncResults[i] = async(launch::async, &IPageDownloader::DownloadPage, m_pDownloaders[i], job);
            }
        if(asyncResults[i].valid() && asyncResults[i].wait_for(s_AsyncWaitTime) == future_status::ready)
            {
            PageContentPtr content = asyncResults[i].get();
            for (auto link : content->GetLinks())
                {
                m_pQueue->AddUrl(link);
                }
            if(m_pQueue->NumberOfUrls() > 0)
                {
                DownloadJobPtr job = m_pQueue->NextDownloadJob();
                asyncResults[i] = async(launch::async, &IPageDownloader::DownloadPage, m_pDownloaders[i], job);
                }
            if(m_pObserver != NULL)
                m_pObserver->OnPageCrawled(content);
            }
        i = (i + 1) % m_NumberOfDownloaders;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool Crawler::AllDownloadsFinished(vector<future<PageContentPtr>> const& downloads) const
    {
    for(auto download = downloads.begin(); download != downloads.end(); ++download)
        {
        if(download->valid()) //Meaning that it is downloading or holding an unread result
            return false;
        }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetObserver(ICrawlerObserver* observer)
    {
    m_pObserver = observer;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetMaxNumberOfLinkToCrawl(size_t n)
    {
    m_pQueue->SetMaxNumberOfVisitedUrls(n);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetUserAgent(WString const& agent)
    {
    for(auto downloader : m_pDownloaders)
        downloader->SetUserAgent(agent);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetRequestTimeoutInSeconds(long timeout)
    {
    for(auto downloader : m_pDownloaders)
        downloader->SetRequestTimeoutInSeconds(timeout);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetFollowAutoRedirects(bool follow)
    {
    for(auto downloader : m_pDownloaders)
        downloader->SetFollowAutoRedirects(follow);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetMaxAutoRedirectCount(long count)
    {
    for(auto downloader : m_pDownloaders)
        downloader->SetMaxAutoRedirectCount(count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetMaxHttpConnectionCount(long count)
    {
    for(auto downloader : m_pDownloaders)
        downloader->SetMaxHttpConnectionCount(count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::ValidateSslCertificates(bool validate)
    {
    for(auto downloader : m_pDownloaders)
        downloader->ValidateSslCertificates(validate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::ValidateContentType(bool validate)
    {
    for(auto downloader : m_pDownloaders)
        downloader->ValidateContentType(validate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetListOfValidContentType(bvector<WString> const& contentTypes)
    {
    for(auto downloader : m_pDownloaders)
        downloader->SetListOfValidContentType(contentTypes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetMaximumCrawlDepth(uint32_t depth)
    {
    m_pQueue->SetMaximumCrawlDepth(depth);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetAcceptExternalLinks(bool accept)
    {
    m_pQueue->SetAcceptExternalLinks(accept);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetAcceptLinksInExternalLinks(bool accept)
    {
    m_pQueue->SetAcceptLinksInExternalLinks(accept);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetRespectRobotTxt(bool respect)
    {
    m_pQueue->SetRespectRobotTxt(respect);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetRespectRobotTxtIfDisallowRoot(bool respect)
    {
    m_pQueue->SetRespectRobotTxtIfDisallowRoot(respect);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetRobotsTxtUserAgent(WString const& userAgent)
    {
    m_pQueue->SetRobotsTxtUserAgent(userAgent);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetMaxRobotTxtCrawlDelay(uint32_t delay)
    {
    m_pQueue->SetMaxRobotTxtCrawlDelay(delay);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetCrawlLinksWithHtmlTagRelNoFollow(bool crawlLinks)
    {
    for(auto downloader : m_pDownloaders)
        downloader->SetParseLinksRelNoFollow(crawlLinks);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetCrawlLinksFromPagesWithNoFollowMetaTag(bool crawlLinks)
    {
    for(auto downloader : m_pDownloaders)
        downloader->SetParsePagesWithNoFollowMetaTag(crawlLinks);
    }
