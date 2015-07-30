#include <Bentley/BeTest.h>
#include <CrawlerLib/Crawler.h>
#include "./UrlMock.h"

using ::testing::Expectation;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::Pointee;
using ::testing::Eq;
using ::testing::_;

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

class DownloaderMock : public IPageDownloader
    {
    public:
    MOCK_METHOD2(DownloadPage, StatusInt(WString& buffer, UrlPtr const& p_Url));
    };

class UrlQueueMock : public UrlQueue
    {
    public:
    MOCK_CONST_METHOD0(NumberOfUrls, size_t());
    MOCK_METHOD1(AddUrl, void(UrlPtr const& url));
    MOCK_METHOD0(NextUrl, UrlPtr());
    MOCK_METHOD1(SetMaxNumberOfVisitedUrls, void(size_t n));

    size_t NumberOfUrlsRealFunction() {return UrlQueue::NumberOfUrls();}
    void AddUrlRealFunction(UrlPtr const& url) {return UrlQueue::AddUrl(url);}
    UrlPtr NextUrlRealFunction() {return UrlQueue::NextUrl();}
    void SetMaxNumberOfVisitedUrlsRealFunction(size_t n) {return UrlQueue::SetMaxNumberOfVisitedUrls(n);}
    };

class PageParserMock : public IPageParser
    {
    public:
    MOCK_CONST_METHOD2(ParsePage, PageContentPtr(UrlPtr const& url, WString const& text));
    };

class CrawlerObserverMock : public ICrawlerObserver
    {
    public:
    MOCK_METHOD1(OnPageCrawled, void(PageContentPtr page));
    };

class CrawlerTester : public ::testing::Test
    {
    public:
    CrawlerTester()
        {
        downloader = new DownloaderMock;
        queue = new UrlQueueMock;
        parser = new PageParserMock;
        crawler = new Crawler(downloader, queue, parser);

        seed = new UrlMock(L"http://seed.com");
        linkInPage = new UrlMock(L"http://link.com");
        pageWithoutLinks = new PageContent(*seed, L"This is a page without links");
        pageWithALink = new PageContent(*seed, L"This is page that contains a link");
        pageWithALink->AddLink(linkInPage);

        //Default mock functions return values and behaviors
        ON_CALL(*queue, NumberOfUrls()).WillByDefault(Invoke(queue, &UrlQueueMock::NumberOfUrlsRealFunction));
        ON_CALL(*queue, NextUrl()).WillByDefault(Invoke(queue, &UrlQueueMock::NextUrlRealFunction));
        ON_CALL(*queue, AddUrl(_)).WillByDefault(Invoke(queue, &UrlQueueMock::AddUrlRealFunction));
        ON_CALL(*queue, SetMaxNumberOfVisitedUrls(_)).WillByDefault(Invoke(queue, &UrlQueueMock::SetMaxNumberOfVisitedUrlsRealFunction));

        ON_CALL(*downloader, DownloadPage(_, _)).WillByDefault(Return(SUCCESS));

        ON_CALL(*parser, ParsePage(_, _)).WillByDefault(Return(pageWithoutLinks));
        }

    ~CrawlerTester()
        {
        delete crawler;
        }

    DownloaderMock* downloader;
    UrlQueueMock* queue;
    PageParserMock* parser;
    Crawler* crawler;

    UrlPtr seed;
    UrlPtr linkInPage;

    PageContentPtr pageWithoutLinks;
    PageContentPtr pageWithALink;
    };


TEST_F(CrawlerTester, WhenCrawlingTheSeedIsAddedToTheQueueDownloadedAndParsed)
    {
    EXPECT_CALL(*queue, AddUrl(Pointee(*seed)));
    EXPECT_CALL(*downloader, DownloadPage(_, seed));
    EXPECT_CALL(*parser, ParsePage(seed, _)).WillOnce(Return(pageWithoutLinks));

    crawler->Crawl(seed);
    }

TEST_F(CrawlerTester, LinksParsedFromAPageAreAddedToTheQueueAndDownloaded)
    {
    //The seed is downloaded and parsed
    EXPECT_CALL(*queue, AddUrl(Pointee(*seed)));
    EXPECT_CALL(*downloader, DownloadPage(_, seed)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*parser, ParsePage(seed, _)).WillOnce(Return(pageWithALink));

    //The link in the seed page is added
    EXPECT_CALL(*queue, AddUrl(Pointee(*linkInPage)));

    //The link in the seed is also downloaded and parsed
    EXPECT_CALL(*downloader, DownloadPage(_, Pointee(*linkInPage))).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*parser, ParsePage(linkInPage, _)).WillOnce(Return(pageWithoutLinks));

    //Execute
    crawler->Crawl(seed);
    }

TEST_F(CrawlerTester, AfterEachParsingTheResultIsReturnedToTheObserver)
    {
    CrawlerObserverMock observer;
    crawler->SetObserver(static_cast<ICrawlerObserver*>(&observer));

    EXPECT_CALL(*parser, ParsePage(seed, _)).WillOnce(Return(pageWithALink));
    EXPECT_CALL(*parser, ParsePage(linkInPage, _)).WillOnce(Return(pageWithoutLinks));
    EXPECT_CALL(observer, OnPageCrawled(pageWithALink)); //After crawling the seed
    EXPECT_CALL(observer, OnPageCrawled(pageWithoutLinks)); //After crawling the page that was linked in the seed

    crawler->Crawl(seed);
    }

TEST_F(CrawlerTester, SettingTheMaxNumberOfLinksToCrawlsUpdatesTheQueueParameter)
    {
    size_t aNumberOfLinks = 1234;
    EXPECT_CALL(*queue, SetMaxNumberOfVisitedUrls(aNumberOfLinks));

    crawler->SetMaxNumberOfLinkToCrawl(aNumberOfLinks);
    }

END_BENTLEY_CRAWLERLIB_NAMESPACE
