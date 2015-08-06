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
Crawler::Crawler(IPageDownloader* downloader, IPoliteness* politeness, UrlQueue* queue, IPageParser* parser)
    {
    m_pDownloader = downloader;
    m_pPoliteness = politeness;
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
    delete m_pPoliteness;
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

        if(m_pPoliteness->CanDownloadUrl(currentUrl))
            {
            m_pPoliteness->WaitToRespectCrawlDelayOf(currentUrl);
            if(m_pDownloader->DownloadPage(buffer, currentUrl) == SUCCESS)
                {
                PageContentPtr content = m_pParser->ParsePage(buffer, currentUrl);
                if(m_pObserver != NULL)
                    {
                    m_pObserver->OnPageCrawled(content);
                    }

                for (auto link : content->GetLinks())
                    {
                    m_pQueue->AddUrl(link);
                    }
                }
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetUserAgent(WString const& agent)
    {
    m_pDownloader->SetUserAgent(agent);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetRequestTimeoutInSeconds(long timeout)
    {
    m_pDownloader->SetRequestTimeoutInSeconds(timeout);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetFollowAutoRedirects(bool follow)
    {
    m_pDownloader->SetFollowAutoRedirects(follow);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetMaxAutoRedirectCount(long count)
    {
    m_pDownloader->SetMaxAutoRedirectCount(count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetMaxHttpConnectionCount(long count)
    {
    m_pDownloader->SetMaxHttpConnectionCount(count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::ValidateSslCertificates(bool validate)
    {
    m_pDownloader->ValidateSslCertificates(validate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::ValidateContentType(bool validate)
    {
    m_pDownloader->ValidateContentType(validate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetListOfValidContentType(bvector<WString> const& contentTypes)
    {
    m_pDownloader->SetListOfValidContentType(contentTypes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetMaximumCrawlDepth(uint32_t depth)
    {
    m_pQueue->SetMaximumCrawlDepth(depth);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetAcceptExternalLinks(bool accept)
    {
    m_pQueue->SetAcceptExternalLinks(accept);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetAcceptLinksInExternalLinks(bool accept)
    {
    m_pQueue->SetAcceptLinksInExternalLinks(accept);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetRespectRobotTxt(bool respect)
    {
    m_pPoliteness->SetRespectRobotTxt(respect);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetRespectRobotTxtIfDisallowRoot(bool respect)
    {
    m_pPoliteness->SetRespectRobotTxtIfDisallowRoot(respect);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetRobotsTxtUserAgent(WString const& userAgent)
    {
    m_pPoliteness->SetUserAgent(UserAgent(userAgent));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetMaxRobotTxtCrawlDelay(uint32_t delay)
    {
    m_pPoliteness->SetMaxCrawlDelay(delay);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetCrawlLinksWithHtmlTagRelNoFollow(bool crawlLinks)
    {
    m_pParser->SetParseLinksRelNoFollow(crawlLinks);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void Crawler::SetCrawlLinksFromPagesWithNoFollowMetaTag(bool crawlLinks)
    {
    m_pParser->SetParsePagesWithNoFollowMetaTag(crawlLinks);
    }
