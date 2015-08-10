/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/CrawlerFactory.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <CrawlerLib/CrawlerFactory.h>

#include <CrawlerLib/Url.h>
#include <CrawlerLib/PageDownloader.h>
#include <CrawlerLib/UrlQueue.h>
#include <CrawlerLib/RobotsTxtParser.h>
#include <CrawlerLib/RobotsTxtDownloader.h>
#include <CrawlerLib/Politeness.h>

USING_NAMESPACE_BENTLEY_CRAWLERLIB

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Crawler* CrawlerFactory::CreateCrawler(size_t maxNumberOfSimultaneousDownloads)
    {
    CrawlDelaySleeperPtr sleeper = new CrawlDelaySleeper;
    std::vector<IPageDownloader*> downloaders;
    for(size_t i = 0; i < maxNumberOfSimultaneousDownloads; ++i)
        {
        IPageDownloader* downloader = new PageDownloader(sleeper);
        downloaders.push_back(downloader);
        }

    IRobotsTxtDownloader* robotsTxtDownloader = new RobotsTxtDownloader;

    IPoliteness* politeness = new Politeness(robotsTxtDownloader);
    UrlQueue* queue = new UrlQueue(politeness);

    return new Crawler(queue, downloaders);
    }
