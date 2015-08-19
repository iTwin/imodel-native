/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/CrawlDelaySleeper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CrawlDelaySleeper.h"
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
        domainIterator->second.second = 0;
        }
    m_AddDomainToMapMutex.unlock();

    std::mutex& mutex = domainIterator->second.first;
    std::lock_guard<std::mutex> lock(mutex);

    if(difftime(time(NULL), domainIterator->second.second) < domainCrawlDelayInSeconds)
        this_thread::sleep_for(chrono::seconds(domainCrawlDelayInSeconds));

    domainIterator->second.second = time(NULL);
    }

