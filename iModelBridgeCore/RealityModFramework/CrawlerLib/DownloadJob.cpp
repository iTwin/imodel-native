/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/DownloadJob.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#include "DownloadJob.h"

USING_NAMESPACE_BENTLEY_CRAWLERLIB

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
DownloadJobPtr DownloadJob::Create(uint32_t crawlDelay, UrlCR url)
    {
    return new DownloadJob(crawlDelay, url);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
DownloadJob::DownloadJob(uint32_t crawlDelay, UrlCR url)
    : RefCountedBase()
    {
    m_CrawlDelay = crawlDelay;
    m_pUrlToDownload = &url;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
uint32_t DownloadJob::GetCrawlDelay() const
    {
    return m_CrawlDelay;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
UrlCPtr DownloadJob::GetUrlToDownload() const
    { 
    return m_pUrlToDownload; 
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
bool DownloadJob::operator == (DownloadJobCR other) const
    {
    return m_CrawlDelay == other.m_CrawlDelay && *m_pUrlToDownload == *(other.m_pUrlToDownload);
    }
