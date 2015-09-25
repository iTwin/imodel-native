/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CrawlerLib/CrawlerTester.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <CrawlerLib/Crawler.h>
#include <CrawlerLib/PageContent.h>
#include "./Mocks.h"
#include "../../CrawlerLib/UrlQueue.h"
#include "../../CrawlerLib/PageDownloader.h"

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
    UrlQueueMock(IPoliteness* politeness) : UrlQueue(politeness) {}
    MOCK_CONST_METHOD0(NumberOfUrls, size_t());
    MOCK_METHOD1(AddUrl, void(UrlPtr const& url));
    MOCK_METHOD0(NextDownloadJob, DownloadJobPtr());
    MOCK_METHOD1(SetMaxNumberOfVisitedUrls, void(size_t n));
    MOCK_METHOD1(SetAcceptExternalLinks, void(bool accept));
    MOCK_METHOD1(SetAcceptLinksInExternalLinks, void(bool accept));
    MOCK_METHOD1(SetMaximumCrawlDepth, void(uint32_t depth));
    MOCK_METHOD1(SetRespectRobotTxt, void(bool respect));
    MOCK_METHOD1(SetRespectRobotTxtIfDisallowRoot, void(bool respect));
    MOCK_METHOD1(SetRobotsTxtUserAgent, void(WString const& userAgent));
    MOCK_METHOD1(SetMaxRobotTxtCrawlDelay, void(uint32_t delay));

    //Spy methods (execute the UrlQueue real behavior)
    size_t NumberOfUrlsRealFunction() {return UrlQueue::NumberOfUrls();}
    void AddUrlRealFunction(UrlPtr const& url) {return UrlQueue::AddUrl(url);}
    DownloadJobPtr NextDownloadJobRealFunction() {return UrlQueue::NextDownloadJob();}
    };

class CrawlerObserverMock : public ICrawlerObserver
    {
    public:
    MOCK_METHOD1(OnPageCrawled, void(PageContentCR page));
    };

class PageDownloaderMock : public IPageDownloader
    {
    public:
    MOCK_METHOD1(DownloadPage, PageContentPtr(DownloadJobPtr const& p_DownloadJob));
    MOCK_METHOD1(SetUserAgent, void(WString const& agent));
    MOCK_METHOD1(SetRequestTimeoutInSeconds, void(long timeout));
    MOCK_METHOD1(SetFollowAutoRedirects, void(bool follow));
    MOCK_METHOD1(SetMaxAutoRedirectCount, void(long count));
    MOCK_METHOD1(ValidateSslCertificates, void(bool validate));
    MOCK_METHOD1(ValidateContentType, void(bool validate));
    MOCK_METHOD1(SetListOfValidContentType, void(bvector<WString> const& types));
    MOCK_METHOD1(SetParseLinksRelNoFollow, void(bool parse));
    MOCK_METHOD1(SetParsePagesWithNoFollowMetaTag, void(bool parse));
    MOCK_METHOD0(AbortDownload, void());
    };

class SingleDownloaderCrawlerTester : public ::testing::Test
    {
    public:
    void SetUp()
        {
        politeness = new PolitenessMock;
        queue = new UrlQueueMock(politeness);

        downloader = new PageDownloaderMock;
        std::vector<IPageDownloader*> downloaders;
        downloaders.push_back(downloader);

        crawler = Crawler::Create(queue, downloaders);

        seed = new UrlMock(L"http://seed.com");
        linkInPage = new UrlMock(L"http://link.com");
        pageWithoutLinks = new PageContent(*seed, L"This is a page without links");
        pageWithALink = new PageContent(*seed, L"This is page that contains a link");
        pageWithALink->AddLink(linkInPage);

        //Default mock functions return values and behaviors
        ON_CALL(*queue, NumberOfUrls()).WillByDefault(Invoke(queue, &UrlQueueMock::NumberOfUrlsRealFunction));
        ON_CALL(*queue, NextDownloadJob()).WillByDefault(Invoke(queue, &UrlQueueMock::NextDownloadJobRealFunction));
        ON_CALL(*queue, AddUrl(_)).WillByDefault(Invoke(queue, &UrlQueueMock::AddUrlRealFunction));

        ON_CALL(*downloader, DownloadPage(_)).WillByDefault(Return(pageWithoutLinks));

        ON_CALL(*politeness, CanDownloadUrl(_)).WillByDefault(Return(true));
        ON_CALL(*politeness, GetCrawlDelay(_)).WillByDefault(Return(0));
        }

    void TearDown()
        {
        }

    PageDownloaderMock* downloader;
    PolitenessMock* politeness;
    UrlQueueMock* queue;
    CrawlerPtr crawler;

    UrlPtr seed;
    UrlPtr linkInPage;

    PageContentPtr pageWithoutLinks;
    PageContentPtr pageWithALink;
    };


TEST_F(SingleDownloaderCrawlerTester, WhenCrawlingTheSeedIsAddedToTheQueueDownloaded)
    {
    DownloadJobPtr job = new DownloadJob(0, seed);

    EXPECT_CALL(*queue, AddUrl(Pointee(*seed)));
    EXPECT_CALL(*downloader, DownloadPage(Pointee(*job))).WillOnce(Return(pageWithoutLinks));

    crawler->Crawl(seed);
    }

TEST_F(SingleDownloaderCrawlerTester, LinksParsedFromAPageAreAddedToTheQueueAndDownloaded)
    {
    //The seed is downloaded and parsed
    DownloadJobPtr seedJob = new DownloadJob(0, seed);
    EXPECT_CALL(*queue, AddUrl(Pointee(*seed)));
    EXPECT_CALL(*downloader, DownloadPage(Pointee(*seedJob))).WillOnce(Return(pageWithALink));

    //The link in the seed page is added
    EXPECT_CALL(*queue, AddUrl(Pointee(*linkInPage)));

    //The link in the seed is also downloaded and parsed
    DownloadJobPtr linkInPageJob = new DownloadJob(0, linkInPage);
    EXPECT_CALL(*downloader, DownloadPage(Pointee(*linkInPageJob))).WillOnce(Return(pageWithoutLinks));

    //Execute
    crawler->Crawl(seed);
    }

TEST_F(SingleDownloaderCrawlerTester, AfterEachDownloadTheResultIsReturnedToTheObserver)
    {
    CrawlerObserverMock observer;
    crawler->SetObserver(static_cast<ICrawlerObserver*>(&observer));

    DownloadJobPtr seedJob = new DownloadJob(0, seed);
    DownloadJobPtr linkInPageJob = new DownloadJob(0, linkInPage);
    EXPECT_CALL(*downloader, DownloadPage(Pointee(*seedJob))).WillOnce(Return(pageWithALink));
    EXPECT_CALL(*downloader, DownloadPage(Pointee(*linkInPageJob))).WillOnce(Return(pageWithALink));
    EXPECT_CALL(observer, OnPageCrawled(_)).Times(2);

    crawler->Crawl(seed);
    }

TEST_F(SingleDownloaderCrawlerTester, SettingTheMaxNumberOfLinksToCrawlsUpdatesTheQueue)
    {
    size_t aNumberOfLinks = 1234;
    EXPECT_CALL(*queue, SetMaxNumberOfVisitedUrls(aNumberOfLinks));

    crawler->SetMaxNumberOfLinkToCrawl(aNumberOfLinks);
    }

TEST_F(SingleDownloaderCrawlerTester, SettingTheMaximumDepthUpdatesTheQueue)
    {
    uint32_t maxCrawlDepth = 5;
    EXPECT_CALL(*queue, SetMaximumCrawlDepth(maxCrawlDepth));

    crawler->SetMaximumCrawlDepth(maxCrawlDepth);
    }

TEST_F(SingleDownloaderCrawlerTester, SettingAcceptanceOfExternalLinksUpdatesTheQueue)
    {
    bool acceptExternalLinks = true;
    EXPECT_CALL(*queue, SetAcceptExternalLinks(acceptExternalLinks));

    crawler->SetAcceptExternalLinks(acceptExternalLinks);
    }

TEST_F(SingleDownloaderCrawlerTester, SettingAcceptanceOfLinksInExternalLinksUpdatesTheQueue)
    {
    bool acceptLinksInExternalLinks = true;
    EXPECT_CALL(*queue, SetAcceptLinksInExternalLinks(acceptLinksInExternalLinks));

    crawler->SetAcceptLinksInExternalLinks(acceptLinksInExternalLinks);
    }

TEST_F(SingleDownloaderCrawlerTester, CanSetRobotsTxtMaxCrawlDelay)
    {
    EXPECT_CALL(*queue, SetMaxRobotTxtCrawlDelay(5));
    crawler->SetMaxRobotTxtCrawlDelay(5);
    }

TEST_F(SingleDownloaderCrawlerTester, CanBypassRobotsTxtIfRootDisabled)
    {
    EXPECT_CALL(*queue, SetRespectRobotTxtIfDisallowRoot(false));
    crawler->SetRespectRobotTxtIfDisallowRoot(false);
    }

TEST_F(SingleDownloaderCrawlerTester, CanSetRobotsTxtUserAgent)
    {
    EXPECT_CALL(*queue, SetRobotsTxtUserAgent(WString(L"botname")));
    crawler->SetRobotsTxtUserAgent(L"botname");
    }

TEST_F(SingleDownloaderCrawlerTester, RespectingRobotsTxtUpdatesAllDownloaders)
    {
    EXPECT_CALL(*queue, SetRespectRobotTxt(true));
    crawler->SetRespectRobotTxt(true);
    }

class MultiDownloaderCrawlerTester : public ::testing::Test
    {
    public:
    void SetUp()
        {
        PolitenessMock* politeness = new PolitenessMock;
        UrlQueueMock* queue = new UrlQueueMock(politeness);

        downloader1 = new PageDownloaderMock;
        downloader2 = new PageDownloaderMock;
        std::vector<IPageDownloader*> downloaders;
        downloaders.push_back(downloader1);
        downloaders.push_back(downloader2);

        crawler = Crawler::Create(queue, downloaders);
        }

    void TearDown()
        {
        }

    PageDownloaderMock* downloader1;
    PageDownloaderMock* downloader2;

    CrawlerPtr crawler;
    };

TEST_F(MultiDownloaderCrawlerTester, SettingTheUserAgentUpdatesAllDownloaders)
    {
    WString userAgent = L"Mozilla/5.0 (Windows NT 6.3; Trident/7.0; rv:11.0) like Gecko";
    EXPECT_CALL(*downloader1, SetUserAgent(userAgent));
    EXPECT_CALL(*downloader2, SetUserAgent(userAgent));

    crawler->SetUserAgent(userAgent);
    }

TEST_F(MultiDownloaderCrawlerTester, SettingTheRequestTimeoutUpdatesAllDownloaders)
    {
    long aTimeoutValue = 15;
    EXPECT_CALL(*downloader1, SetRequestTimeoutInSeconds(aTimeoutValue));
    EXPECT_CALL(*downloader2, SetRequestTimeoutInSeconds(aTimeoutValue));

    crawler->SetRequestTimeoutInSeconds(aTimeoutValue);
    }

TEST_F(MultiDownloaderCrawlerTester, SettingFollowAutoRedirectsUpdatesAllDownloaders)
    {
    bool followRedirect = true;
    EXPECT_CALL(*downloader1, SetFollowAutoRedirects(followRedirect));
    EXPECT_CALL(*downloader2, SetFollowAutoRedirects(followRedirect));

    crawler->SetFollowAutoRedirects(followRedirect);
    }

TEST_F(MultiDownloaderCrawlerTester, SettingMaxAutoRedirectCountUpdatesAllDownloaders)
    {
    long maxRedirect = 15;
    EXPECT_CALL(*downloader1, SetMaxAutoRedirectCount(maxRedirect));
    EXPECT_CALL(*downloader2, SetMaxAutoRedirectCount(maxRedirect));

    crawler->SetMaxAutoRedirectCount(maxRedirect);
    }

TEST_F(MultiDownloaderCrawlerTester, SettingSslValidationUpdatesAllDownloaders)
    {
    bool validate = true;
    EXPECT_CALL(*downloader1, ValidateSslCertificates(validate));
    EXPECT_CALL(*downloader2, ValidateSslCertificates(validate));

    crawler->ValidateSslCertificates(validate);
    }

TEST_F(MultiDownloaderCrawlerTester, SettingValidteContentTypeUpdatesAllDownloaders)
    {
    bool validate = true;
    EXPECT_CALL(*downloader1, ValidateContentType(validate));
    EXPECT_CALL(*downloader2, ValidateContentType(validate));

    crawler->ValidateContentType(validate);
    }

TEST_F(MultiDownloaderCrawlerTester, SettingTheListOfValidContentTypesUpdatesAllDownloaders)
    {
    bvector<WString> validTypes;
    validTypes.push_back(L"text/html");
    EXPECT_CALL(*downloader1, SetListOfValidContentType(validTypes));
    EXPECT_CALL(*downloader2, SetListOfValidContentType(validTypes));

    crawler->SetListOfValidContentType(validTypes);
    }

TEST_F(MultiDownloaderCrawlerTester, IgnoringLinksInHtmlWithRelNoFollowUpdatesAllDownloaders)
    {
    EXPECT_CALL(*downloader1, SetParseLinksRelNoFollow(false));
    EXPECT_CALL(*downloader2, SetParseLinksRelNoFollow(false));
    crawler->SetCrawlLinksWithHtmlTagRelNoFollow(false);
    }

TEST_F(MultiDownloaderCrawlerTester, IgnoringPagesLinksFromPagesWithNoFollowMetaTagUpdatesAllDownloaders)
    {
    EXPECT_CALL(*downloader1, SetParsePagesWithNoFollowMetaTag(false));
    EXPECT_CALL(*downloader2, SetParsePagesWithNoFollowMetaTag(false));
    crawler->SetCrawlLinksFromPagesWithNoFollowMetaTag(false);
    }

END_BENTLEY_CRAWLERLIB_NAMESPACE
