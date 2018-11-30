/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/DownloadJob.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <CrawlerLib/Url.h>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

//=======================================================================================
//! @bsiclass
// Simple object to represent a download job definition.
// A job contans an Url and a crawl delay.
//=======================================================================================
struct DownloadJob : public RefCountedBase
    {
    public:
    CRAWLERLIB_EXPORT static DownloadJobPtr Create(uint32_t crawlDelay, UrlCR url);

    CRAWLERLIB_EXPORT uint32_t GetCrawlDelay() const;
    CRAWLERLIB_EXPORT UrlCPtr GetUrlToDownload() const;
    CRAWLERLIB_EXPORT inline bool operator==(DownloadJobCR other) const;

    private:
    DownloadJob(uint32_t crawlDelay, UrlCR url);

    uint32_t m_CrawlDelay;
    UrlCPtr m_pUrlToDownload;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
