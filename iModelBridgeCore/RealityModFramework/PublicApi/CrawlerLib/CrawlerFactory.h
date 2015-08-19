/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/CrawlerFactory.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/Crawler.h>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

//=======================================================================================
//! @bsiclass
//=======================================================================================
class CrawlerFactory
    {
    public:
    CRAWLERLIB_EXPORT static Crawler* CreateCrawler(size_t maxNumberOfSimultaneousDownloads = 8);
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
