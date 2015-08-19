/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/DownloadJob.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <CrawlerLib/Url.h>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct DownloadJob : public RefCountedBase
    {
    public:
    CRAWLERLIB_EXPORT DownloadJob(uint32_t crawlDelay, UrlPtr url) : RefCountedBase()
        {
        m_CrawlDelay = crawlDelay;
        m_pUrlToDownload = url;
        }

    CRAWLERLIB_EXPORT uint32_t GetCrawlDelay() {return m_CrawlDelay;}
    CRAWLERLIB_EXPORT UrlPtr const& GetUrlToDownload() {return m_pUrlToDownload;}
    CRAWLERLIB_EXPORT inline bool operator==(DownloadJob const& other) const
        {
        return m_CrawlDelay == other.m_CrawlDelay && *m_pUrlToDownload == *(other.m_pUrlToDownload);
        }

    private:
    uint32_t m_CrawlDelay;
    UrlPtr m_pUrlToDownload;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
