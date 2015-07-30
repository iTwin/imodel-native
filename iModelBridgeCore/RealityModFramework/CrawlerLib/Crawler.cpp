/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/Crawler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <CrawlerLib/Crawler.h>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Crawler::Crawler(IPageDownloader* downloader, UrlQueue* queue, IPageParser* parser)
    {
    m_pDownloader = downloader; 
    m_pQueue = queue; 
    m_pParser = parser;
    m_pObserver = NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
Crawler::~Crawler()
    {
    delete m_pDownloader;
    delete m_pQueue;
    delete m_pParser;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt Crawler::Crawl(UrlPtr const& seed)
    {
    WString buffer;
    m_pQueue->AddUrl(seed);

    while(m_pQueue->NumberOfUrls() > 0)
        {
        buffer = L"";
        UrlPtr currentUrl = m_pQueue->NextUrl();
        m_pDownloader->DownloadPage(buffer, currentUrl);
        PageContentPtr content = m_pParser->ParsePage(currentUrl, buffer);

        if(m_pObserver != NULL)
            {
            m_pObserver->OnPageCrawled(content);
            }

        for (auto link : content->GetLinks()) 
            {
            m_pQueue->AddUrl(link);
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetObserver(ICrawlerObserver* observer)
    {
    m_pObserver = observer;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetMaxNumberOfLinkToCrawl(size_t n)
    {
    m_pQueue->SetMaxNumberOfVisitedUrls(n);
    }

