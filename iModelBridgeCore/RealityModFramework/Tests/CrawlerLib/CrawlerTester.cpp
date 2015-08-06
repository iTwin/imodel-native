/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CrawlerLib/CrawlerTester.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <CrawlerLib/Crawler.h>
#include "./Mocks.h"

using ::testing::Expectation;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::Pointee;
using ::testing::Eq;
using ::testing::_;
using ::testing::InSequence;

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

class UrlQueueMock : public UrlQueue
    {
    public:
    MOCK_CONST_METHOD0(NumberOfUrls, size_t());
    MOCK_METHOD1(AddUrl, void(UrlPtr const& url));
    MOCK_METHOD0(NextUrl, UrlPtr());
    MOCK_METHOD1(SetMaxNumberOfVisitedUrls, void(size_t n));
    MOCK_METHOD1(SetAcceptExternalLinks, void(bool accept));
    MOCK_METHOD1(SetAcceptLinksInExternalLinks, void(bool accept));
    MOCK_METHOD1(SetMaximumCrawlDepth, void(uint32_t depth));

    //Spy methods (execute the UrlQueue real behavior)
    size_t NumberOfUrlsRealFunction() {return UrlQueue::NumberOfUrls();}
    void AddUrlRealFunction(UrlPtr const& url) {return UrlQueue::AddUrl(url);}
    UrlPtr NextUrlRealFunction() {return UrlQueue::NextUrl();}
    };

class PageParserMock : public IPageParser
    {
    public:
    MOCK_CONST_METHOD2(ParsePage, PageContentPtr(WString const& text, UrlPtr const& url));
    MOCK_METHOD1(SetParseLinksRelNoFollow, void(bool parse));
    MOCK_METHOD1(SetParsePagesWithNoFollowMetaTag, void(bool parse));
    };

class CrawlerObserverMock : public ICrawlerObserver
    {
    public:
    MOCK_METHOD1(OnPageCrawled, void(PageContentPtr page));
    };

class PolitenessMock : public IPoliteness
    {
    public:
    MOCK_METHOD1(SetMaxCrawlDelay, void(uint32_t delay));
    MOCK_METHOD1(SetUserAgent, void(UserAgent const& agent));
    MOCK_METHOD1(SetRespectRobotTxt, void(bool respect));
    MOCK_METHOD1(SetRespectRobotTxtIfDisallowRoot, void(bool respect));
    MOCK_METHOD1(CanDownloadUrl, bool(UrlPtr const& url));
    MOCK_METHOD1(WaitToRespectCrawlDelayOf, void(UrlPtr const& url));
    };

class CrawlerTester : public ::testing::Test
    {
    public:
    CrawlerTester()
        {
        downloader = new DownloaderMock;
        queue = new UrlQueueMock;
        parser = new PageParserMock;
        politeness = new PolitenessMock;
        crawler = new Crawler(downloader, politeness, queue, parser);

        seed = new UrlMock(L"http://seed.com");
        linkInPage = new UrlMock(L"http://link.com");
        pageWithoutLinks = new PageContent(*seed, L"This is a page without links");
        pageWithALink = new PageContent(*seed, L"This is page that contains a link");
        pageWithALink->AddLink(linkInPage);

        //Default mock functions return values and behaviors
        ON_CALL(*queue, NumberOfUrls()).WillByDefault(Invoke(queue, &UrlQueueMock::NumberOfUrlsRealFunction));
        ON_CALL(*queue, NextUrl()).WillByDefault(Invoke(queue, &UrlQueueMock::NextUrlRealFunction));
        ON_CALL(*queue, AddUrl(_)).WillByDefault(Invoke(queue, &UrlQueueMock::AddUrlRealFunction));

        ON_CALL(*downloader, DownloadPage(_, _)).WillByDefault(Return(SUCCESS));

        ON_CALL(*parser, ParsePage(_, _)).WillByDefault(Return(pageWithoutLinks));

        ON_CALL(*politeness, CanDownloadUrl(_)).WillByDefault(Return(true));
        }

    ~CrawlerTester()
        {
        delete crawler;
        }

    DownloaderMock* downloader;
    UrlQueueMock* queue;
    PageParserMock* parser;
    PolitenessMock* politeness;
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
    EXPECT_CALL(*parser, ParsePage(_, seed)).WillOnce(Return(pageWithoutLinks));

    crawler->Crawl(seed);
    }

TEST_F(CrawlerTester, LinksParsedFromAPageAreAddedToTheQueueAndDownloaded)
    {
    //The seed is downloaded and parsed
    EXPECT_CALL(*queue, AddUrl(Pointee(*seed)));
    EXPECT_CALL(*downloader, DownloadPage(_, seed)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*parser, ParsePage(_, seed)).WillOnce(Return(pageWithALink));

    //The link in the seed page is added
    EXPECT_CALL(*queue, AddUrl(Pointee(*linkInPage)));

    //The link in the seed is also downloaded and parsed
    EXPECT_CALL(*downloader, DownloadPage(_, Pointee(*linkInPage))).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*parser, ParsePage(_, linkInPage)).WillOnce(Return(pageWithoutLinks));

    //Execute
    crawler->Crawl(seed);
    }

TEST_F(CrawlerTester, AfterEachParsingTheResultIsReturnedToTheObserver)
    {
    CrawlerObserverMock observer;
    crawler->SetObserver(static_cast<ICrawlerObserver*>(&observer));

    EXPECT_CALL(*parser, ParsePage(_, seed)).WillOnce(Return(pageWithALink));
    EXPECT_CALL(*parser, ParsePage(_, linkInPage)).WillOnce(Return(pageWithoutLinks));
    EXPECT_CALL(observer, OnPageCrawled(pageWithALink)); //After crawling the seed
    EXPECT_CALL(observer, OnPageCrawled(pageWithoutLinks)); //After crawling the page that was linked in the seed

    crawler->Crawl(seed);
    }

TEST_F(CrawlerTester, SettingTheMaxNumberOfLinksToCrawlsUpdatesTheQueue)
    {
    size_t aNumberOfLinks = 1234;
    EXPECT_CALL(*queue, SetMaxNumberOfVisitedUrls(aNumberOfLinks));

    crawler->SetMaxNumberOfLinkToCrawl(aNumberOfLinks);
    }

TEST_F(CrawlerTester, SettingTheMaximumDepthUpdatesTheQueue)
    {
    uint32_t maxCrawlDepth = 5;
    EXPECT_CALL(*queue, SetMaximumCrawlDepth(maxCrawlDepth));

    crawler->SetMaximumCrawlDepth(maxCrawlDepth);
    }

TEST_F(CrawlerTester, SettingAcceptanceOfExternalLinksUpdatesTheQueue)
    {
    bool acceptExternalLinks = true;
    EXPECT_CALL(*queue, SetAcceptExternalLinks(acceptExternalLinks));

    crawler->SetAcceptExternalLinks(acceptExternalLinks);
    }

TEST_F(CrawlerTester, SettingAcceptanceOfLinksInExternalLinksUpdatesTheQueue)
    {
    bool acceptLinksInExternalLinks = true;
    EXPECT_CALL(*queue, SetAcceptLinksInExternalLinks(acceptLinksInExternalLinks));

    crawler->SetAcceptLinksInExternalLinks(acceptLinksInExternalLinks);
    }

TEST_F(CrawlerTester, SettingTheUserAgentUpdatesTheDownloader)
    {
    WString userAgent = L"Mozilla/5.0 (Windows NT 6.3; Trident/7.0; rv:11.0) like Gecko";
    EXPECT_CALL(*downloader, SetUserAgent(userAgent));

    crawler->SetUserAgent(userAgent);
    }

TEST_F(CrawlerTester, SettingTheRequestTimeoutUpdatesTheDownloader)
    {
    long aTimeoutValue = 15;
    EXPECT_CALL(*downloader, SetRequestTimeoutInSeconds(aTimeoutValue));

    crawler->SetRequestTimeoutInSeconds(aTimeoutValue);
    }

TEST_F(CrawlerTester, SettingFollowAutoRedirectsUpdatesTheDownloader)
    {
    bool followRedirect = true;
    EXPECT_CALL(*downloader, SetFollowAutoRedirects(followRedirect));

    crawler->SetFollowAutoRedirects(followRedirect);
    }

TEST_F(CrawlerTester, SettingMaxAutoRedirectCountUpdatesTheDownloader)
    {
    long maxRedirect = 15;
    EXPECT_CALL(*downloader, SetMaxAutoRedirectCount(maxRedirect));

    crawler->SetMaxAutoRedirectCount(maxRedirect);
    }

TEST_F(CrawlerTester, SettingMaxHttpConnectionCountUpdatesTheDownloader)
    {
    long maxConnectionCount = 15;
    EXPECT_CALL(*downloader, SetMaxHttpConnectionCount(maxConnectionCount));

    crawler->SetMaxHttpConnectionCount(maxConnectionCount);
    }

TEST_F(CrawlerTester, SettingSslValidationUpdatesTheDownloader)
    {
    bool validate = true;
    EXPECT_CALL(*downloader, ValidateSslCertificates(validate));

    crawler->ValidateSslCertificates(validate);
    }

TEST_F(CrawlerTester, SettingValidteContentTypeUpdatesTheDownloader)
    {
    bool validate = true;
    EXPECT_CALL(*downloader, ValidateContentType(validate));

    crawler->ValidateContentType(validate);
    }

TEST_F(CrawlerTester, SettingTheListOfValidContentTypesUpdatesTheDownloader)
    {
    bvector<WString> validTypes;
    validTypes.push_back(L"text/html");
    EXPECT_CALL(*downloader, SetListOfValidContentType(validTypes));

    crawler->SetListOfValidContentType(validTypes);
    }

TEST_F(CrawlerTester, TheCrawlerOnlyDownloadsAPageIfItsPolitenessAllowsIt)
    {
    UrlPtr allowedUrl = new UrlMock(L"http://come-here.com");
    UrlPtr disallowedUrl = new UrlMock(L"http://get-out.com");

    EXPECT_CALL(*politeness, CanDownloadUrl(Pointee(*allowedUrl))).WillOnce(Return(true));
    EXPECT_CALL(*downloader, DownloadPage(_, Pointee(*allowedUrl)));
    EXPECT_CALL(*parser, ParsePage(_, allowedUrl)).WillOnce(Return(pageWithoutLinks));
    crawler->Crawl(allowedUrl);

    EXPECT_CALL(*politeness, CanDownloadUrl(Pointee(*disallowedUrl))).WillOnce(Return(false));
    EXPECT_CALL(*downloader, DownloadPage(_, Pointee(*disallowedUrl))).Times(0);
    crawler->Crawl(disallowedUrl);
    }

TEST_F(CrawlerTester, CanRespectRobotsTxt)
    {
    EXPECT_CALL(*politeness, SetRespectRobotTxt(true));
    crawler->SetRespectRobotTxt(true);
    }

TEST_F(CrawlerTester, CanBypassRobotsTxtIfRootDisabled)
    {
    EXPECT_CALL(*politeness, SetRespectRobotTxtIfDisallowRoot(false));
    crawler->SetRespectRobotTxtIfDisallowRoot(false);
    }

TEST_F(CrawlerTester, CanSetRobotsTxtUserAgent)
    {
    UserAgent expectedUserAgent(L"botname");
    EXPECT_CALL(*politeness, SetUserAgent(expectedUserAgent));
    crawler->SetRobotsTxtUserAgent(L"botname");
    }

TEST_F(CrawlerTester, CanSetRobotsTxtMaxCrawlDelay)
    {
    EXPECT_CALL(*politeness, SetMaxCrawlDelay(5));
    crawler->SetMaxRobotTxtCrawlDelay(5);
    }

TEST_F(CrawlerTester, CanIgnoreLinksInHtmlWithRelNoFollow)
    {
    EXPECT_CALL(*parser, SetParseLinksRelNoFollow(false));
    crawler->SetCrawlLinksWithHtmlTagRelNoFollow(false);
    }

TEST_F(CrawlerTester, CanIgnorePagesLinksFromPagesWithNoFollowMetaTag)
    {
    EXPECT_CALL(*parser, SetParsePagesWithNoFollowMetaTag(false));
    crawler->SetCrawlLinksFromPagesWithNoFollowMetaTag(false);
    }


TEST_F(CrawlerTester, TheCrawlerAskIfItCanDownloadUrlAndWaitsACrawlDelayBeforeDownloading)
    {
    UrlPtr toDownload = new UrlMock(L"http://come-here.com");

    InSequence s;
    EXPECT_CALL(*politeness, CanDownloadUrl(Pointee(*toDownload))).WillOnce(Return(true));
    EXPECT_CALL(*politeness, WaitToRespectCrawlDelayOf(Pointee(*toDownload)));
    EXPECT_CALL(*downloader, DownloadPage(_, Pointee(*toDownload)));

    crawler->Crawl(toDownload);
    }

TEST_F(CrawlerTester, WhenTheDownloaderReturnsAnErrorThePageIsNotParseAndTheObserverIsNotCalled)
    {
    CrawlerObserverMock observer;
    crawler->SetObserver(static_cast<ICrawlerObserver*>(&observer));

    EXPECT_CALL(*downloader, DownloadPage(_, _)).WillOnce(Return(ERROR));
    EXPECT_CALL(*parser, ParsePage(_, _)).Times(0);
    EXPECT_CALL(observer, OnPageCrawled(_)).Times(0);

    UrlPtr urlThatCausesError = new UrlMock(L"http://come-here.com");
    crawler->Crawl(urlThatCausesError);
    }

END_BENTLEY_CRAWLERLIB_NAMESPACE
