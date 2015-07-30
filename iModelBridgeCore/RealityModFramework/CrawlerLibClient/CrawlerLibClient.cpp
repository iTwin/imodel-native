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
        crawler = CrawlerFactory::GetSingleThreadedCrawler();
        }
    virtual ~CrawlerLibClient()
        {
        delete crawler;
        }

    virtual void OnPageCrawled(PageContentPtr page)
        {
        printf("%ls\n", page->GetUrl().GetUrlWString());
        }

    void Run()
        {
        UrlPtr seed = new Url(L"https://en.wikipedia.org/wiki/Main_Page");
        crawler->SetMaxNumberOfLinkToCrawl(100);
        crawler->SetObserver(this);
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
