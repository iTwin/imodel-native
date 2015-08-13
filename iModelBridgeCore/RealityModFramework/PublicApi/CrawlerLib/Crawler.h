/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/Crawler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once


#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/PageDownloader.h>
#include <CrawlerLib/UrlQueue.h>
#include <CrawlerLib/PageContent.h>
#include <CrawlerLib/Politeness.h>

#include <Bentley/Bentley.h>

#include <vector>
#include <chrono>
#include <future>
#include <atomic>
#include <mutex>
#include <condition_variable>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

class ICrawlerObserver
    {
    public:
    CRAWLERLIB_EXPORT virtual ~ICrawlerObserver() {}
    CRAWLERLIB_EXPORT virtual void OnPageCrawled(PageContentPtr page) = 0;
    };

class Crawler
    {
    public:
    CRAWLERLIB_EXPORT Crawler(UrlQueue* queue, std::vector<IPageDownloader*> const& downloaders);
    Crawler() = delete;
    CRAWLERLIB_EXPORT virtual ~Crawler();

    CRAWLERLIB_EXPORT StatusInt Crawl(UrlPtr const& seed);
    CRAWLERLIB_EXPORT void Pause();
    CRAWLERLIB_EXPORT void Unpause();
    CRAWLERLIB_EXPORT void Stop();

    CRAWLERLIB_EXPORT void SetObserver(ICrawlerObserver* observer);
    CRAWLERLIB_EXPORT void SetMaxNumberOfLinkToCrawl(size_t n);
    CRAWLERLIB_EXPORT void SetUserAgent(WString const& agent);
    CRAWLERLIB_EXPORT void SetRequestTimeoutInSeconds(long timeout);
    CRAWLERLIB_EXPORT void SetFollowAutoRedirects(bool follow);
    CRAWLERLIB_EXPORT void SetMaxAutoRedirectCount(long count);
    CRAWLERLIB_EXPORT void ValidateSslCertificates(bool validate);
    CRAWLERLIB_EXPORT void ValidateContentType(bool validate);
    CRAWLERLIB_EXPORT void SetListOfValidContentType(bvector<WString> const& contentTypes);
    CRAWLERLIB_EXPORT void SetMaximumCrawlDepth(uint32_t depth);
    CRAWLERLIB_EXPORT void SetAcceptExternalLinks(bool accept);
    CRAWLERLIB_EXPORT void SetAcceptLinksInExternalLinks(bool accept);

    CRAWLERLIB_EXPORT void SetRespectRobotTxt(bool respect);
    CRAWLERLIB_EXPORT void SetRespectRobotTxtIfDisallowRoot(bool respect);
    CRAWLERLIB_EXPORT void SetRobotsTxtUserAgent(WString const& userAgent);
    CRAWLERLIB_EXPORT void SetMaxRobotTxtCrawlDelay(uint32_t delay);
    CRAWLERLIB_EXPORT void SetCrawlLinksWithHtmlTagRelNoFollow(bool crawlLinks);
    CRAWLERLIB_EXPORT void SetCrawlLinksFromPagesWithNoFollowMetaTag(bool crawlLinks);


    private:
    bool CanStartDownload(std::future<PageContentPtr> const& asyncDownloadThread) const;
    bool IsDownloadResultReady(std::future<PageContentPtr> const& asyncDownloadThread) const;
    void StartNextDownload(std::future<PageContentPtr>& asyncDownloadThread, IPageDownloader* downloaderToUse);
    bool AllDownloadsFinished(std::vector<std::future<PageContentPtr>> const& downloads) const;
    void DiscardRemainingDownloads(std::vector<std::future<PageContentPtr>>& downloads) const;

    bool IsStopped() const;

    UrlQueue* m_pQueue;
    std::vector<IPageDownloader*> m_pDownloaders;

    ICrawlerObserver* m_pObserver;

    const size_t m_NumberOfDownloaders;
    static const std::chrono::milliseconds s_AsyncWaitTime;

    std::atomic<bool>       m_StopFlag;

    bool                    m_PauseFlag;
    std::mutex              m_PauseFlagMutex;
    std::condition_variable m_PauseFlagConditionVariable;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
