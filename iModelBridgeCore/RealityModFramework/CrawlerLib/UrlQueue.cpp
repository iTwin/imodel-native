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
UrlQueue::UrlQueue()
    {
    m_NumberOfUrls = 0;
    m_MaxNumberOfVisitedUrls = (numeric_limits<size_t>::max)(); //The paratheses are there to prevent the max() macro to replace this call
    m_AcceptExternalLinks = false;
    m_AcceptLinksInExternalLinks = false;
    m_MaximumCrawlDepth = (numeric_limits<uint32_t>::max)();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
UrlQueue::UrlQueue(UrlQueue const& other)
    {
    m_NumberOfUrls = other.m_NumberOfUrls;
    m_MaxNumberOfVisitedUrls = other.m_MaxNumberOfVisitedUrls;
    m_Urls = other.m_Urls;
    m_VisitedUrls = other.m_VisitedUrls;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
UrlQueue::~UrlQueue()
    {

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
UrlQueue& UrlQueue::operator=(UrlQueue const& other)
    {
    if(this != &other)
        {
        m_NumberOfUrls = other.m_NumberOfUrls;
        m_Urls = other.m_Urls;
        }
    return *this;
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
UrlPtr UrlQueue::NextUrl()
    {
    m_NumberOfUrls--;
    UrlPtr next = m_Urls.front();
    m_Urls.pop();
    return next;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void UrlQueue::AddUrl(UrlPtr const& url)
    {
    if(IsAcceptedUrl(url) && m_VisitedUrls.size() < m_MaxNumberOfVisitedUrls)
        {
        m_Urls.push(url);
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
        && (m_AcceptLinksInExternalLinks || !(url->GetParent().IsValid() && url->GetParent()->IsExternalPage()));
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

