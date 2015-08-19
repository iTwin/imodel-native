/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CrawlerLib/UrlQueueTester.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "./Mocks.h"
#include <Bentley/BeTest.h>
#include <CrawlerLib/Url.h>
#include "../../CrawlerLib/UrlQueue.h"

using ::testing::Return;
using ::testing::Pointee;
using ::testing::_;

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

class UrlQueueTester : public ::testing::Test
    {
    public:
    void SetUp()
        {
        politeness = new PolitenessMock;
        queue = new UrlQueue(politeness);
        ON_CALL(*politeness, CanDownloadUrl(_)).WillByDefault(Return(true));
        }
    void TearDown()
        {
        delete queue;
        }
    PolitenessMock* politeness;
    UrlQueue* queue;
    };

TEST_F(UrlQueueTester, TheQueueIsInitiallyEmpy)
    {
    ASSERT_EQ(0, queue->NumberOfUrls());
    }

TEST_F(UrlQueueTester, CanAddAnUrlToTheQueue)
    {
    UrlPtr anUrl = new UrlMock(L"http://toto.com");

    queue->AddUrl(anUrl);

    ASSERT_EQ(1, queue->NumberOfUrls());
    ASSERT_EQ(*anUrl, *queue->NextDownloadJob()->GetUrlToDownload());
    }

TEST_F(UrlQueueTester, CanAddMultipleToQueueAndPopThemInOrder)
    {
    UrlPtr firstAddedUrl = new UrlMock(L"http://toto.com");
    UrlPtr secondAddedUrl = new UrlMock(L"http://tata.com");

    queue->AddUrl(firstAddedUrl);
    queue->AddUrl(secondAddedUrl);

    ASSERT_EQ(2, queue->NumberOfUrls());
    ASSERT_EQ(*firstAddedUrl, *queue->NextDownloadJob()->GetUrlToDownload());
    ASSERT_EQ(*secondAddedUrl, *queue->NextDownloadJob()->GetUrlToDownload());
    ASSERT_EQ(0, queue->NumberOfUrls());
    }

TEST_F(UrlQueueTester, WhenAnUrlIsAddedMultipleTimesItIsIgnoredAfterTheFirstTime)
    {
    UrlPtr urlA = new UrlMock(L"http://toto.com");
    UrlPtr urlB = new UrlMock(L"http://tata.com");
    UrlPtr urlEqualsToA = new UrlMock(L"http://toto.com");

    queue->AddUrl(urlA);
    queue->AddUrl(urlB);
    ASSERT_EQ(2, queue->NumberOfUrls());

    queue->AddUrl(urlA);
    ASSERT_EQ(2, queue->NumberOfUrls());

    queue->AddUrl(urlEqualsToA);
    ASSERT_EQ(2, queue->NumberOfUrls());
    }

TEST_F(UrlQueueTester, WhenTheMaxNumberOfUrlIsReachedAdditionalUrlsAreIgnored)
    {
    UrlPtr url1 = new UrlMock(L"http://1.com");
    UrlPtr url2 = new UrlMock(L"http://2.com");
    UrlPtr url3 = new UrlMock(L"http://3.com");
    UrlPtr url4 = new UrlMock(L"http://4.com");
    UrlPtr url5 = new UrlMock(L"http://5.com");

    queue->SetMaxNumberOfVisitedUrls(3);
    queue->AddUrl(url1);
    queue->AddUrl(url2);
    queue->AddUrl(url3);
    ASSERT_EQ(3, queue->NumberOfUrls());

    queue->AddUrl(url4);
    ASSERT_EQ(3, queue->NumberOfUrls());

    queue->NextDownloadJob()->GetUrlToDownload();
    ASSERT_EQ(2, queue->NumberOfUrls());

    queue->AddUrl(url5);
    ASSERT_EQ(2, queue->NumberOfUrls());
    }

TEST_F(UrlQueueTester, WhenExternalLinksAreEnabledAddigAnExternalLinkAddsTheLinkToTheQueue)
    {
    queue->SetAcceptExternalLinks(true);

    UrlMock internalUrl(L"http://internal.com");
    internalUrl.SetIsExternalPage(false);
    UrlPtr internalUrlPtr = new UrlMock(internalUrl);

    UrlMock externalUrl(L"http://external.com");
    externalUrl.SetIsExternalPage(true);
    UrlPtr externalUrlPtr = new UrlMock(externalUrl);

    queue->AddUrl(internalUrlPtr);
    ASSERT_EQ(1, queue->NumberOfUrls());

    queue->AddUrl(externalUrlPtr);
    ASSERT_EQ(2, queue->NumberOfUrls());
    }

TEST_F(UrlQueueTester, WhenExternalLinksAreDisabledAddigAnExternalLinkIsIgnored)
    {
    queue->SetAcceptExternalLinks(false);

    UrlMock internalUrl(L"http://internal.com");
    internalUrl.SetIsExternalPage(false);
    UrlPtr internalUrlPtr = new UrlMock(internalUrl);

    UrlMock externalUrl(L"http://external.com");
    externalUrl.SetIsExternalPage(true);
    UrlPtr externalUrlPtr = new UrlMock(externalUrl);

    queue->AddUrl(internalUrlPtr);
    ASSERT_EQ(1, queue->NumberOfUrls());

    queue->AddUrl(externalUrlPtr);
    ASSERT_EQ(1, queue->NumberOfUrls());
    }

TEST_F(UrlQueueTester, WhenCrawlingLinksInExternalLinksIsDisabledThoseLinksAreIgnored)
    {
    queue->SetAcceptExternalLinks(true);
    queue->SetAcceptLinksInExternalLinks(false);

    UrlMock internalUrl(L"http://internal.com");
    internalUrl.SetIsExternalPage(false);
    UrlPtr internalUrlPtr = new UrlMock(internalUrl);

    UrlMock externalUrl(L"http://external.com");
    externalUrl.SetIsExternalPage(true);
    UrlPtr externalUrlPtr = new UrlMock(externalUrl);

    UrlMock linkInExternalUrl(L"http://link-with-external-parent.com");
    linkInExternalUrl.SetIsExternalPage(true);
    linkInExternalUrl.SetParent(externalUrlPtr);
    UrlPtr linkInExternalUrlPtr = new UrlMock(linkInExternalUrl);

    queue->AddUrl(internalUrlPtr);
    queue->AddUrl(externalUrlPtr);
    ASSERT_EQ(2, queue->NumberOfUrls());

    queue->AddUrl(linkInExternalUrlPtr);
    ASSERT_EQ(2, queue->NumberOfUrls());
    }

TEST_F(UrlQueueTester, WhenCrawlingLinksInExternalLinksIsEnabledThoseLinksAreAddedToTheQueue)
    {
    queue->SetAcceptExternalLinks(true);
    queue->SetAcceptLinksInExternalLinks(true);

    UrlMock internalUrl(L"http://internal.com");
    internalUrl.SetIsExternalPage(false);
    UrlPtr internalUrlPtr = new UrlMock(internalUrl);

    UrlMock externalUrl(L"http://external.com");
    externalUrl.SetIsExternalPage(true);
    UrlPtr externalUrlPtr = new UrlMock(externalUrl);

    UrlMock linkInExternalUrl(L"http://link-with-external-parent.com");
    linkInExternalUrl.SetIsExternalPage(true);
    linkInExternalUrl.SetParent(externalUrlPtr);
    UrlPtr linkInExternalUrlPtr = new UrlMock(linkInExternalUrl);

    queue->AddUrl(internalUrlPtr);
    queue->AddUrl(externalUrlPtr);
    ASSERT_EQ(2, queue->NumberOfUrls());

    queue->AddUrl(linkInExternalUrlPtr);
    ASSERT_EQ(3, queue->NumberOfUrls());
    }

TEST_F(UrlQueueTester, WhenAnUrlHasADepthLargerThanTheMaximumDepthItIsIgnored)
    {
    uint32_t maximumDepth = 100;
    queue->SetMaximumCrawlDepth(maximumDepth);

    UrlMock urlWithCorrectDepth(L"http://limit-depth.com");
    urlWithCorrectDepth.SetDepth(maximumDepth);
    UrlPtr urlWithCorrectDepthPtr = new UrlMock(urlWithCorrectDepth);

    UrlMock urlWithIncorrectDepth(L"http://depth-too-large.com");
    urlWithIncorrectDepth.SetDepth(maximumDepth + 1);
    UrlPtr urlWithIncorrectDepthPtr = new UrlMock(urlWithIncorrectDepth);

    queue->AddUrl(urlWithCorrectDepthPtr);
    ASSERT_EQ(1, queue->NumberOfUrls());

    queue->AddUrl(urlWithIncorrectDepthPtr);
    ASSERT_EQ(1, queue->NumberOfUrls());
    }

TEST_F(UrlQueueTester, TheQueueAlternatesBetweenDomains)
    {
    UrlPtr seed = new Seed(L"http://a-seed.com");
    UrlPtr url1_domain1 = new Url(L"http://domain1.com/1", seed);
    UrlPtr url2_domain1 = new Url(L"http://domain1.com/2", seed);
    UrlPtr url1_domain2 = new Url(L"http://domain2.com/1", seed);
    UrlPtr url2_domain2 = new Url(L"http://domain2.com/2", seed);
    UrlPtr url1_domain3 = new Url(L"http://domain3.com/1", seed);

    queue->SetAcceptExternalLinks(true);
    queue->AddUrl(url1_domain1);
    queue->AddUrl(url2_domain1);
    queue->AddUrl(url1_domain2);
    queue->AddUrl(url2_domain2);
    queue->AddUrl(url1_domain3);

    ASSERT_EQ(5, queue->NumberOfUrls());
    ASSERT_EQ(url1_domain1, queue->NextDownloadJob()->GetUrlToDownload());
    ASSERT_EQ(url1_domain2, queue->NextDownloadJob()->GetUrlToDownload());
    ASSERT_EQ(url1_domain3, queue->NextDownloadJob()->GetUrlToDownload());
    ASSERT_EQ(url2_domain1, queue->NextDownloadJob()->GetUrlToDownload());
    ASSERT_EQ(url2_domain2, queue->NextDownloadJob()->GetUrlToDownload());
    }

TEST_F(UrlQueueTester, TheQueueOnlyAddsAPageIfItsPolitenessAllowsIt)
    {
    UrlPtr disallowedUrl = new UrlMock(L"http://get-out.com");
    UrlPtr allowedUrl = new UrlMock(L"http://come-here.com");

    EXPECT_CALL(*politeness, CanDownloadUrl(Pointee(*disallowedUrl))).WillOnce(Return(false));
    queue->AddUrl(disallowedUrl);
    ASSERT_EQ(0, queue->NumberOfUrls());

    EXPECT_CALL(*politeness, CanDownloadUrl(Pointee(*allowedUrl))).WillOnce(Return(true));
    queue->AddUrl(allowedUrl);
    ASSERT_EQ(1, queue->NumberOfUrls());

    }

TEST_F(UrlQueueTester, CanRespectRobotsTxt)
    {
    EXPECT_CALL(*politeness, SetRespectRobotTxt(true));
    queue->SetRespectRobotTxt(true);
    }

TEST_F(UrlQueueTester, CanBypassRobotsTxtIfRootDisabled)
    {
    EXPECT_CALL(*politeness, SetRespectRobotTxtIfDisallowRoot(false));
    queue->SetRespectRobotTxtIfDisallowRoot(false);
    }

TEST_F(UrlQueueTester, CanSetUserAgent)
    {
    EXPECT_CALL(*politeness, SetUserAgent(UserAgent(L"botname")));
    queue->SetRobotsTxtUserAgent(L"botname");
    }

TEST_F(UrlQueueTester, CanSetRobotsTxtMaxCrawlDelay)
    {
    EXPECT_CALL(*politeness, SetMaxCrawlDelay(5));
    queue->SetMaxRobotTxtCrawlDelay(5);
    }

//&&AG TODO add a test taht verifies that the queue returns download jobs that has the good crawl delay

END_BENTLEY_CRAWLERLIB_NAMESPACE
