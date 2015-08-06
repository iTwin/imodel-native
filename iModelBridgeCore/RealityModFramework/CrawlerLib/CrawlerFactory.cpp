/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/CrawlerFactory.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <CrawlerLib/CrawlerFactory.h>
#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/Url.h>
#include <CrawlerLib/Downloader.h>
#include <CrawlerLib/PageParser.h>
#include <CrawlerLib/UrlQueue.h>
#include <CrawlerLib/RobotsTxtParser.h>
#include <CrawlerLib/Politeness.h>

USING_NAMESPACE_BENTLEY_CRAWLERLIB

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Crawler* CrawlerFactory::GetSingleThreadedCrawler()
    {
    IPageDownloader* webPageDownloader = new SingleThreadedDownloader;

    IPageDownloader* robotsTxtDownloader = new SingleThreadedDownloader;
    RobotsTxtParser* robotsTxtParser = new RobotsTxtParser;
    ISleeper* sleeper = new Sleeper;
    IPoliteness* politeness = new Politeness(robotsTxtDownloader, robotsTxtParser, sleeper);

    UrlQueue* queue = new UrlQueue;
    IPageParser* pageParser = new PageParser;

    return new Crawler(webPageDownloader, politeness, queue, pageParser);
    } 
