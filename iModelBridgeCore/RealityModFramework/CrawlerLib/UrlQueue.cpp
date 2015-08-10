/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/UrlQueue.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <CrawlerLib/UrlQueue.h>
#include <limits>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
UrlQueue::UrlQueue(IPoliteness* politeness)
    {
    m_NumberOfUrls = 0;
    m_MaxNumberOfVisitedUrls = (numeric_limits<size_t>::max)(); //The paratheses are there to prevent the max() macro to replace this call
    m_AcceptExternalLinks = false;
    m_AcceptLinksInExternalLinks = false;
    m_MaximumCrawlDepth = (numeric_limits<uint32_t>::max)();
    m_CurrentDomain = m_QueuesPerDomain.end();
    m_pPoliteness = politeness;
    }


UrlQueue::~UrlQueue() 
    {
    delete m_pPoliteness;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
size_t UrlQueue::NumberOfUrls() const
    {
    return m_NumberOfUrls;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
DownloadJobPtr UrlQueue::NextDownloadJob()
    {
    if(m_CurrentDomain == m_QueuesPerDomain.end())
        m_CurrentDomain = m_QueuesPerDomain.begin();

    UrlPtr nextUrl = m_CurrentDomain->second.front();
    m_CurrentDomain->second.pop();
    uint32_t crawlDelay = m_pPoliteness->GetCrawlDelay(nextUrl);
    DownloadJobPtr nextDownloadJob = new DownloadJob(crawlDelay, nextUrl);

    if(m_CurrentDomain->second.size() == 0)
        {
        m_CurrentDomain = m_QueuesPerDomain.erase(m_CurrentDomain); //Erase current iterator while incrementing it.
        }
    else
        {
        ++m_CurrentDomain;
        }

    m_NumberOfUrls--;
    return nextDownloadJob;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void UrlQueue::AddUrl(UrlPtr const& url)
    {
    if(IsAcceptedUrl(url) && m_VisitedUrls.size() < m_MaxNumberOfVisitedUrls)
        {
        auto currentQueueIterator = m_QueuesPerDomain.find(url->GetDomainName());
        if(currentQueueIterator == m_QueuesPerDomain.end())
            {
            m_QueuesPerDomain.emplace(url->GetDomainName(), queue<UrlPtr>());
            currentQueueIterator = m_QueuesPerDomain.find(url->GetDomainName());
            }
        currentQueueIterator->second.push(url);
        m_VisitedUrls.insert(url);
        ++m_NumberOfUrls;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool UrlQueue::IsAcceptedUrl(UrlPtr const& url) const
    {
    return !HaveAlreadyVisited(url)
        && (url->GetDepth() <= m_MaximumCrawlDepth)
        && (m_AcceptExternalLinks || !url->IsExternalPage())
        && (m_AcceptLinksInExternalLinks || !(url->GetParent().IsValid() && url->GetParent()->IsExternalPage()))
        && m_pPoliteness->CanDownloadUrl(url);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool UrlQueue::HaveAlreadyVisited(UrlPtr const& url) const
    {
    return m_VisitedUrls.find(url) != m_VisitedUrls.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void UrlQueue::SetMaxNumberOfVisitedUrls(size_t n)
    {
    m_MaxNumberOfVisitedUrls = n;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
size_t UrlQueue::GetMaxNumberOfVisitedUrls()
    {
    return m_MaxNumberOfVisitedUrls;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void UrlQueue::SetAcceptExternalLinks(bool accept)
    {
    m_AcceptExternalLinks = accept;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void UrlQueue::SetAcceptLinksInExternalLinks(bool accept)
    {
    m_AcceptLinksInExternalLinks = accept;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void UrlQueue::SetMaximumCrawlDepth(uint32_t depth)
    {
    m_MaximumCrawlDepth = depth;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void UrlQueue::SetRespectRobotTxt(bool respect)
    {
    m_pPoliteness->SetRespectRobotTxt(respect);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void UrlQueue::SetRespectRobotTxtIfDisallowRoot(bool respect)
    {
    m_pPoliteness->SetRespectRobotTxtIfDisallowRoot(respect);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void UrlQueue::SetRobotsTxtUserAgent(WString const& userAgent)
    {
    m_pPoliteness->SetUserAgent(UserAgent(userAgent));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void UrlQueue::SetMaxRobotTxtCrawlDelay(uint32_t delay)
    {
    m_pPoliteness->SetMaxCrawlDelay(delay);
    }
