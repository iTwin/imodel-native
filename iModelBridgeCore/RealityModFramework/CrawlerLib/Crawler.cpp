/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <CrawlerLib/Crawler.h>
#include <CrawlerLib/Url.h>
#include "UrlQueue.h"
#include "PageDownloader.h"

#include <curl/curl.h>

//#include <vector>
#include <chrono>
#include <future>
#include <atomic>
#include <mutex>
#include <condition_variable>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;

const chrono::milliseconds s_AsyncWaitTime = chrono::milliseconds(75);

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
CrawlerPtr Crawler::Create(size_t maxNumberOfSimultaneousDownloads)
    {
    CrawlDelaySleeperPtr sleeper = new CrawlDelaySleeper;
    bvector<IPageDownloader*> downloaders;
    for(size_t i = 0; i < maxNumberOfSimultaneousDownloads; ++i)
        {
        IPageDownloader* downloader = new PageDownloader(sleeper);
        downloaders.push_back(downloader);
        }

    IRobotsTxtDownloader* robotsTxtDownloader = new RobotsTxtDownloader;

    IPoliteness* politeness = new Politeness(robotsTxtDownloader);
    UrlQueuePtr queue = UrlQueue::Create(politeness);

    return new Crawler(queue, downloaders);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
CrawlerPtr Crawler::Create(UrlQueuePtr queue, bvector<IPageDownloader*> const& downloaders)
    { 
      return new Crawler(queue, downloaders);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Crawler::Crawler(UrlQueuePtr queue, bvector<IPageDownloader*> const& downloaders)
    : m_NumberOfDownloaders(downloaders.size()), m_StopFlag(false), m_PauseFlag(false)
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

    if (0 != m_pObserver)
        {
        delete m_pObserver;
        m_pObserver = 0;
        }

    curl_global_cleanup();
    }

//--------------------------------------------------------------------------------------
// @bsiclass                                                 Martin-Yanick.Guille   11/15
//+---------------+---------------+---------------+---------------+---------------+------
struct TestPredicate : IConditionVariablePredicate
{
	bool& m_PauseFlagIn;

	TestPredicate(bool& pauseFlag) : m_PauseFlagIn(pauseFlag) {}
	virtual bool _TestCondition(BeConditionVariable &cv) override { return m_PauseFlagIn == false; 	}
};



//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt Crawler::Crawl(UrlCR seed)
    {
    BeAssert(m_pDownloaders.size() == m_NumberOfDownloaders);

    m_pQueue->AddUrl(seed);

    vector<future<PageContentPtr>> asyncDownloadThreads(m_NumberOfDownloaders);
    size_t i = 0;
    while(!IsStopped() && (m_pQueue->NumberOfUrls() > 0 || !AllDownloadsFinished(asyncDownloadThreads)))
        {
        // If pause, wait for unpaused.
        //std::unique_lock<BeMutex> lock(m_PauseFlagMutex);
        //m_PauseFlagConditionVariable.wait(lock, [this]{return m_PauseFlag == false;});
		BeMutexHolder holder(m_PauseFlagMutex);
		TestPredicate predicate(m_PauseFlag);
		m_PauseFlagConditionVariable.ProtectedWaitOnCondition(holder, &predicate, BeConditionVariable::Infinite );

        future<PageContentPtr>& currentDownloadThread = asyncDownloadThreads[i];
        IPageDownloader* currentDownloader = m_pDownloaders[i];

        // Thread available and something in queue.
        if(CanStartDownload(currentDownloadThread))
            StartNextDownload(currentDownloadThread, currentDownloader);

        if(IsDownloadResultReady(currentDownloadThread))
            {
            PageContentPtr pageContent = currentDownloadThread.get(); // currentDownloadThread.valid() will be false afterward.
            for (auto link : pageContent->GetLinks())
                {
                // This call might take some time if robots.txt needs to be downloaded. That will occurs once per domain.
                m_pQueue->AddUrl(*link);
                }

            // Start something new now, in case the observer takes a long time to process.
            if(CanStartDownload(currentDownloadThread))
                StartNextDownload(currentDownloadThread, currentDownloader);

            if(m_pObserver != NULL)
                m_pObserver->OnPageCrawled(*pageContent);
            }
        i = (i + 1) % m_NumberOfDownloaders; //Cycle through the download
        }

    DiscardRemainingDownloads(asyncDownloadThreads);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool Crawler::CanStartDownload(future<PageContentPtr> const& asyncDownloadThread) const
    {
    return !asyncDownloadThread.valid() && m_pQueue->NumberOfUrls() > 0; //After default construction, std::future is not valid
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool Crawler::IsDownloadResultReady(future<PageContentPtr> const& asyncDownloadThread) const
    {
    return asyncDownloadThread.valid() && asyncDownloadThread.wait_for(s_AsyncWaitTime) == future_status::ready;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::StartNextDownload(future<PageContentPtr>& asyncDownloadThread, IPageDownloader* downloaderToUse)
    {
    DownloadJobPtr job = m_pQueue->NextDownloadJob();
    asyncDownloadThread = async(launch::async, &IPageDownloader::DownloadPage, downloaderToUse, job);
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
void Crawler::DiscardRemainingDownloads(vector<future<PageContentPtr>>& downloadThreads) const
    {
    for(auto downloader : m_pDownloaders)
        downloader->AbortDownload();

    for(auto thread = downloadThreads.begin(); thread != downloadThreads.end(); ++thread)
        {
        if(thread->valid())
            thread->get();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetObserver(ICrawlerObserver* pObserver)
    {
    m_pObserver = pObserver;
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
void Crawler::SetMaxRobotTxtCrawlDelay(uint32_t delayInSeconds)
    {
    m_pQueue->SetMaxRobotTxtCrawlDelay(delayInSeconds);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::Pause()
    {
    std::lock_guard<BeMutex> lock(m_PauseFlagMutex);
    m_PauseFlag = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::Unpause()
    {
        {
        std::lock_guard<BeMutex> lock(m_PauseFlagMutex);
        m_PauseFlag = false;
        }
    m_PauseFlagConditionVariable.notify_one();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::Stop()
    {
    m_StopFlag.store(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool Crawler::IsStopped() const
    {
    return m_StopFlag.load(memory_order_relaxed);
    }
