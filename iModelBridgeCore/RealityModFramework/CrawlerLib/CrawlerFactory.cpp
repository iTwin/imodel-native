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

USING_NAMESPACE_BENTLEY_CRAWLERLIB

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Crawler* CrawlerFactory::GetSingleThreadedCrawler()
    {
    IPageDownloader* downloader = new SingleThreadedDownloader;
    UrlQueue* queue = new UrlQueue;
    IPageParser* parser = new PageParser;

    return new Crawler(downloader, queue, parser);
    } 
