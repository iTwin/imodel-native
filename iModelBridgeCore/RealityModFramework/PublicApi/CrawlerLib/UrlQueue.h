/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/UrlQueue.h $
| 
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/PageParser.h>
#include <CrawlerLib/Downloader.h>

#include <queue>
#include <set>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

class UrlQueue
    {
    public:
    CRAWLERLIB_EXPORT UrlQueue();
    CRAWLERLIB_EXPORT UrlQueue(UrlQueue const& other);
    CRAWLERLIB_EXPORT virtual ~UrlQueue();

    UrlQueue& operator=(UrlQueue const& other);

    CRAWLERLIB_EXPORT virtual size_t NumberOfUrls() const;

    CRAWLERLIB_EXPORT virtual void SetMaxNumberOfVisitedUrls(size_t n);
    CRAWLERLIB_EXPORT size_t GetMaxNumberOfVisitedUrls();

    CRAWLERLIB_EXPORT virtual void AddUrl(UrlPtr const& url);
    CRAWLERLIB_EXPORT virtual UrlPtr NextUrl();

    private:
    bool HaveAlreadyVisited(UrlPtr const& url) const;

    size_t m_NumberOfUrls;
    size_t m_MaxNumberOfVisitedUrls;

    std::queue<UrlPtr> m_Urls;
    std::set<UrlPtr, UrlPtrCompare> m_VisitedUrls;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
