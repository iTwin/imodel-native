/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/Crawler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <Bentley/Bentley.h>

#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/PageParser.h>
#include <CrawlerLib/Downloader.h>
#include <CrawlerLib/UrlQueue.h>
#include <CrawlerLib/PageContent.h>
#include <CrawlerLib/Politeness.h>


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
    CRAWLERLIB_EXPORT Crawler(IPageDownloader* downloader, IPoliteness* politeness, UrlQueue* queue, IPageParser* parser);
    Crawler() = delete;
    CRAWLERLIB_EXPORT virtual ~Crawler();

    CRAWLERLIB_EXPORT StatusInt Crawl(UrlPtr const& seed);
    CRAWLERLIB_EXPORT void SetObserver(ICrawlerObserver* observer);
    CRAWLERLIB_EXPORT void SetMaxNumberOfLinkToCrawl(size_t n);
    CRAWLERLIB_EXPORT void SetUserAgent(WString const& agent);
    CRAWLERLIB_EXPORT void SetRequestTimeoutInSeconds(long timeout);
    CRAWLERLIB_EXPORT void SetFollowAutoRedirects(bool follow);
    CRAWLERLIB_EXPORT void SetMaxAutoRedirectCount(long count);
    CRAWLERLIB_EXPORT void SetMaxHttpConnectionCount(long count);
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
    IPageDownloader* m_pDownloader;
    IPoliteness* m_pPoliteness;
    UrlQueue* m_pQueue;
    IPageParser* m_pParser;

    ICrawlerObserver* m_pObserver;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
