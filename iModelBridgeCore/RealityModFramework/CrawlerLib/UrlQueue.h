/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/UrlQueue.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <Bentley/Bentley.h>
#include <CrawlerLib/CrawlerLib.h>
#include "DownloadJob.h"
#include "Politeness.h"

#include <queue>
#include <map>
#include <set>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

//=======================================================================================
//! @bsiclass
//=======================================================================================
class UrlQueue
    {
    public:
    //=======================================================================================
    // A valid IPoliteness object must be provided. The UrlQueue takes ownership of the 
    // politeness object and is responsible of destroying it.
    //=======================================================================================
    CRAWLERLIB_EXPORT UrlQueue(IPoliteness* politeness);
    CRAWLERLIB_EXPORT virtual ~UrlQueue();

    CRAWLERLIB_EXPORT virtual size_t NumberOfUrls() const;

    CRAWLERLIB_EXPORT size_t GetMaxNumberOfVisitedUrls();
    CRAWLERLIB_EXPORT virtual void SetMaxNumberOfVisitedUrls(size_t n); //Those methods are virtual so they can be mocked/spy in tests
    CRAWLERLIB_EXPORT virtual void SetAcceptExternalLinks(bool accept);
    CRAWLERLIB_EXPORT virtual void SetAcceptLinksInExternalLinks(bool accept);
    CRAWLERLIB_EXPORT virtual void SetMaximumCrawlDepth(uint32_t depth);
    CRAWLERLIB_EXPORT virtual void SetRespectRobotTxt(bool respect);
    CRAWLERLIB_EXPORT virtual void SetRespectRobotTxtIfDisallowRoot(bool respect);
    CRAWLERLIB_EXPORT virtual void SetRobotsTxtUserAgent(WString const& userAgent);
    CRAWLERLIB_EXPORT virtual void SetMaxRobotTxtCrawlDelay(uint32_t delay);

    //=======================================================================================
    // Add as url to the queue. The url cannot be null.
    //=======================================================================================
    CRAWLERLIB_EXPORT virtual void AddUrl(UrlPtr const& url);

    
    //=======================================================================================
    // Returns the next download job based on the next url in the queue to be processed.
    //=======================================================================================
    CRAWLERLIB_EXPORT virtual DownloadJobPtr NextDownloadJob();

    private:
    bool HaveAlreadyVisited(UrlPtr const& url) const;
    bool IsAcceptedUrl(UrlPtr const& url) const;

    size_t m_NumberOfUrls;
    size_t m_MaxNumberOfVisitedUrls;
    bool m_AcceptExternalLinks;
    bool m_AcceptLinksInExternalLinks;
    uint32_t m_MaximumCrawlDepth;

    std::map<DomainName, std::queue<UrlPtr>>           m_QueuesPerDomain;
    std::map<DomainName, std::queue<UrlPtr>>::iterator m_CurrentDomain;

    UrlPtrSet m_VisitedUrls;
    IPoliteness* m_pPoliteness;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
