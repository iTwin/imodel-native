/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/CrawlDelaySleeper.h $
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

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

struct CrawlDelaySleeper : RefCountedBase
    {
    public:
    void Sleep(uint32_t seconds, DomainName const& domain);

    private:
    std::map<DomainName, std::mutex> m_MutexPerDomain;
    std::mutex m_AddDomainToMapMutex;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
