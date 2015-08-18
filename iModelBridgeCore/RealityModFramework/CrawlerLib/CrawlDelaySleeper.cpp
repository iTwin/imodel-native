/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/CrawlDelaySleeper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <CrawlerLib/CrawlDelaySleeper.h>
#include <chrono>
#include <thread>
#include <utility>
#include <tuple>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
USING_NAMESPACE_BENTLEY
using namespace std;

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void CrawlDelaySleeper::Sleep(uint32_t domainCrawlDelayInSeconds, DomainName const& domain)
    {
    if(domainCrawlDelayInSeconds <= 0)
        return;

    m_AddDomainToMapMutex.lock();
    auto domainIterator = m_MutexPerDomain.find(domain);
    if(domainIterator == m_MutexPerDomain.end())
        {
        // No iterators or references are invalidated by emplace.
        domainIterator = m_MutexPerDomain.emplace(piecewise_construct, forward_as_tuple(domain), forward_as_tuple()).first; //std::mutex is non copyable class,
                                                                                                                            //therefore the picewise_construct
        }
    m_AddDomainToMapMutex.unlock();

    // We do not remember the last download time so we sleep the domain crawl delay every time.  That might add delays
    // but it is simple enough for now. It might be improved by remembering the last domain crawl time and make sure that we waited enough time.
    std::mutex& mutex = domainIterator->second;
    mutex.lock();
    this_thread::sleep_for(chrono::seconds(domainCrawlDelayInSeconds));
    mutex.unlock();
    }

