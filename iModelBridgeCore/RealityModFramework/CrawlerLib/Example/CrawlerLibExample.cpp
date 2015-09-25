/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/Example/CrawlerLibExample.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <CrawlerLib/Crawler.h>


USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;

#define __DEMO_PAUSE_STOP_FROM_OTHER_THREAD__ 0

class CrawlerLibClient : ICrawlerObserver
    {
    public:
    CrawlerLibClient()
        {
        //Use the Create() static method to create a crawler.
        crawler = Crawler::Create(25/*8 simultaneous downloads maximum by default*/);
        }

    virtual ~CrawlerLibClient()
        {
        }

    //Callback method provided by the ICrawlerObserver interface
    virtual void OnPageCrawled(PageContentCR page)
        {
        printf("%ls\n", page.GetUrl().GetUrlWString());
        }

    void Run()
        {
        UrlPtr seed = new Seed(L"http://usgs.gov");

        //Set the crawler options
        crawler->SetMaxNumberOfLinkToCrawl(2000);
        crawler->SetAcceptExternalLinks(true);
        crawler->SetAcceptLinksInExternalLinks(false);
        crawler->SetMaximumCrawlDepth(10000);
        crawler->SetRequestTimeoutInSeconds(15);
        crawler->SetUserAgent(L"Mozilla/5.0 (Windows NT 6.3; Trident/7.0; rv:11.0) like Gecko");
        crawler->SetObserver(this);

        crawler->SetFollowAutoRedirects(true);
        crawler->SetMaxAutoRedirectCount(10);
        crawler->ValidateSslCertificates(false);

        crawler->ValidateContentType(true);
        bvector<WString> validTypes;
        validTypes.push_back(L"text/html");
        validTypes.push_back(L"text/plain");
        crawler->SetListOfValidContentType(validTypes);

        crawler->SetAcceptExternalLinks(true);
        crawler->SetAcceptLinksInExternalLinks(true);

        crawler->SetRespectRobotTxt(false);
        crawler->SetRespectRobotTxtIfDisallowRoot(false);
        crawler->SetRobotsTxtUserAgent(L"botname");
        crawler->SetMaxRobotTxtCrawlDelay(5);
        crawler->SetCrawlLinksWithHtmlTagRelNoFollow(true);
        crawler->SetCrawlLinksFromPagesWithNoFollowMetaTag(true);

#if __DEMO_PAUSE_STOP_FROM_OTHER_THREAD__
        //It is possible to pause and stop the crawling from another thread.
        //To demo this feature, Crawler::Crawl is launched in an separate thread
        //and the current thread calls pause, unpause and stop

        future<StatusInt> asyncCrawl = async(launch::async, &Crawler::Crawl, crawler, seed);
        this_thread::sleep_for(chrono::seconds(5));
        printf("pause\n");
        crawler->Pause();
        this_thread::sleep_for(chrono::seconds(5));
        printf("unpause\n");
        crawler->Unpause();
        this_thread::sleep_for(chrono::seconds(10));
        printf("stop\n");
        crawler->Stop();

        asyncCrawl.get();
#else
        crawler->Crawl(seed);
#endif
        }

    private:
    CrawlerPtr crawler;
    };


int wmain(int pi_Argc, wchar_t *pi_ppArgv[])
{
    CrawlerLibClient c;
    c.Run();
}
