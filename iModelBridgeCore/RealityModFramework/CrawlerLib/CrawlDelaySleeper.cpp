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
void CrawlDelaySleeper::Sleep(uint32_t seconds, WString const& domain)
    {
    if(seconds <= 0)
        return;

    m_AddDomainToMapMutex.lock();
    auto domainIterator = m_MutexPerDomain.find(domain);
    if(domainIterator == m_MutexPerDomain.end())
        {
        m_MutexPerDomain.emplace(piecewise_construct, forward_as_tuple(domain), forward_as_tuple()); //BeMutex is non copyable class,
                                                                                                     //therefore the picewise_construct
        domainIterator = m_MutexPerDomain.find(domain);
        }
    m_AddDomainToMapMutex.unlock();

    std::mutex& mutex = domainIterator->second;
    mutex.lock();
    this_thread::sleep_for(chrono::seconds(seconds));
    mutex.unlock();
    }

