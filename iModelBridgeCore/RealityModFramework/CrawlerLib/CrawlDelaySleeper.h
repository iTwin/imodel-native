/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/CrawlDelaySleeper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/Url.h>

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>

#include <vector>
#include <map>
#include <mutex>
#include <ctime>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

//=======================================================================================
//! @bsiclass
// A CrawlDelaySleeper is an object that will sleep some specific amount of seconds.
// The sleeper object is multithread multi-domain. Each domain can have a different
// time period. 
// The intent of the class is to provide a mechanism to satisfy the sleep conditions
// imposed by a web site for each specific domain. The sleeper is provided to downloader
// objects that will, when required call the Sleep method.
// Given the sleeper is multithread safe it can be shared by many downloader objects
// executing on different threads.
// NOTE: The sleeper will not perform cleanup management of domain/mutex pairs. Once
// a sleep is requested for a domain a domain, mutex pair is created in the object map.
// and this entry will remain in the map for the duration of the existence of the 
// sleeper. Note that the domain entry will be reused if a subsequent call is made
// for a previously known domain.
// 
//=======================================================================================
struct CrawlDelaySleeper : RefCountedBase
    {
    public:
    void Sleep(uint32_t seconds, DomainName const& domain);

    private:
    std::map<DomainName, std::pair<std::mutex, std::time_t>> m_MutexPerDomain;
    std::mutex m_AddDomainToMapMutex;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
