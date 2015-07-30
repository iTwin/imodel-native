/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/CrawlerFactory.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/Crawler.h>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

class CrawlerFactory
    {
    public:
    CRAWLERLIB_EXPORT static Crawler* GetSingleThreadedCrawler();
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
