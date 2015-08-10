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

#include <Bentley/Bentley.h>
#include <Bentley/BeThread.h>

#include <vector>
#include <map>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

struct CrawlDelaySleeper : RefCountedBase
    {
    public:
    void Sleep(uint32_t seconds, WString const& domain);

    private:
    std::map<WString, BentleyApi::BeMutex> m_MutexPerDomain;
    BeMutex m_AddDomainToMapMutex;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
