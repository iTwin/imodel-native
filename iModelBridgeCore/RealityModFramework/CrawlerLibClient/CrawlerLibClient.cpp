/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLibClient/CrawlerLibClient.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <CrawlerLib/CrawlerFactory.h>


USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;


class CrawlerLibClient : ICrawlerObserver
    {
    public:
    CrawlerLibClient()
        {
        //Use the CrawlerFactory to easily build a crawler. You are responsible of deleting it.
        crawler = CrawlerFactory::GetSingleThreadedCrawler();
        }
    virtual ~CrawlerLibClient()
        {
        delete crawler;
        }

    //Callback method provided by the ICrawlerObserver interface
    virtual void OnPageCrawled(PageContentPtr page)
        {
        printf("%ls\n", page->GetUrl().GetUrlWString());
        }

    void Run()
        {
        UrlPtr seed = new Seed(L"http://mrdata.usgs.gov/wms.html");

        //Set the crawler options
        crawler->SetMaxNumberOfLinkToCrawl(50);
        crawler->SetAcceptExternalLinks(true);
        crawler->SetAcceptLinksInExternalLinks(false);
        crawler->SetMaximumCrawlDepth(1000);
        crawler->SetRequestTimeoutInSeconds(15);
        crawler->SetUserAgent(L"Mozilla/5.0 (Windows NT 6.3; Trident/7.0; rv:11.0) like Gecko");
        crawler->SetObserver(this);

        crawler->SetFollowAutoRedirects(true);
        crawler->SetMaxAutoRedirectCount(10);
        crawler->SetMaxHttpConnectionCount(5);
        crawler->ValidateSslCertificates(false);

        crawler->ValidateContentType(true);
        bvector<WString> validTypes;
        validTypes.push_back(L"text/html");
        validTypes.push_back(L"text/plain");
        crawler->SetListOfValidContentType(validTypes);

        crawler->SetAcceptExternalLinks(true);
        crawler->SetAcceptLinksInExternalLinks(true);

        crawler->SetRespectRobotTxt(true);
        crawler->SetRespectRobotTxtIfDisallowRoot(false);
        crawler->SetRobotsTxtUserAgent(L"botname");
        crawler->SetMaxRobotTxtCrawlDelay(5);
        crawler->SetCrawlLinksWithHtmlTagRelNoFollow(true);
        crawler->SetCrawlLinksFromPagesWithNoFollowMetaTag(true);

        crawler->Crawl(seed);
        }

    private:
    Crawler* crawler;
    };


int wmain(int pi_Argc, wchar_t *pi_ppArgv[])
{
    CrawlerLibClient c;
    c.Run();
}
