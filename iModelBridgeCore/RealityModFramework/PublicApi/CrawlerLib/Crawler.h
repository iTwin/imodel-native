/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/Crawler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>

#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/PageParser.h>
#include <CrawlerLib/Downloader.h>
#include <CrawlerLib/UrlQueue.h>
#include <CrawlerLib/PageContent.h>


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
    CRAWLERLIB_EXPORT Crawler(IPageDownloader* downloader, UrlQueue* queue, IPageParser* parser);
    Crawler() = delete;
    CRAWLERLIB_EXPORT virtual ~Crawler();

    CRAWLERLIB_EXPORT StatusInt Crawl(UrlPtr const& seed);
    CRAWLERLIB_EXPORT void SetObserver(ICrawlerObserver* observer);
    CRAWLERLIB_EXPORT void SetMaxNumberOfLinkToCrawl(size_t n);

    private:
    IPageDownloader* m_pDownloader;
    UrlQueue* m_pQueue;
    IPageParser* m_pParser;

    ICrawlerObserver* m_pObserver;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
