/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CrawlerLib/PolitenessTester.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "../../CrawlerLib/Politeness.h"
#include "./Mocks.h"

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;

using ::testing::Return;
using ::testing::Pointee;
using ::testing::_;


class RobotsTxtDownloaderMock : public IRobotsTxtDownloader
    {
    public:
    MOCK_METHOD1(DownloadRobotsTxt, RobotsTxtContentPtr(UrlPtr const& url));
    };

class PolitenessTester : public ::testing::Test
    {
    public:
    PolitenessTester()
        {
        seed = new Seed(L"http://some-domain.com/index.html");
        downloader = new RobotsTxtDownloaderMock;
        politeness = new Politeness(downloader);
        emptyRobotsTxt = new RobotsTxtContent(L"", seed);

        ON_CALL(*downloader, DownloadRobotsTxt(_)).WillByDefault(Return(emptyRobotsTxt));
        }

    ~PolitenessTester()
        {
        delete politeness;
        }

    RobotsTxtContentPtr GetRobotsTxtContentThatDisallows(UrlPtr const& url)
        {
        RobotsTxtContentPtr content = new RobotsTxtContent(L"", new Url(L"http://some-domain.com", seed));
        content->AddUserAgent(UserAgent(L"*"));
        content->AddDisallowedUrl(UserAgent(L"*"), url);
        return content;
        }

    RobotsTxtContentPtr GetRobotsTxtContentThatHasCrawlDelay(uint32_t delay)
        {
        RobotsTxtContentPtr content = new RobotsTxtContent(L"", new Url(L"http://some-domain.com", seed));
        content->AddUserAgent(UserAgent(L"*"));
        content->SetCrawlDelay(UserAgent(L"*"), delay);
        return content;
        }

    Politeness* politeness;
    RobotsTxtDownloaderMock* downloader;

    RobotsTxtContentPtr emptyRobotsTxt;
    WString robotsTxtFileText;

    UrlPtr seed;
    };

TEST_F(PolitenessTester, ByDefaultYouCanDownloadAllPages)
    {
    ASSERT_TRUE(politeness->CanDownloadUrl(seed));
    }

TEST_F(PolitenessTester, ByDefaultRobotsTxtAreIgnoredAndNotDownloaded)
    {
    EXPECT_CALL(*downloader, DownloadRobotsTxt(_)).Times(0);
    politeness->CanDownloadUrl(seed);
    }

TEST_F(PolitenessTester, WhenAnUnknownDomainIsQueriedTheRobotsTxtIsDownloadedTheFirstTimeOnly)
    {
    UrlPtr anotherUrlInSameDomain = new Url(L"http://some-domain.com/other_page.html", seed);
    EXPECT_CALL(*downloader, DownloadRobotsTxt(Pointee(*seed))).Times(1);

    politeness->SetRespectRobotTxt(true);
    politeness->CanDownloadUrl(seed);
    politeness->CanDownloadUrl(anotherUrlInSameDomain);
    }

TEST_F(PolitenessTester, WhenRootIsDisallowedPolitenessCanDownloadFromUrlAnyway)
    {
    UrlPtr root = new Url(L"/", seed);
    UrlPtr ignoredDisallowedUrl1 = new Url(L"/foo", seed);
    UrlPtr ignoredDisallowedUrl2 = new Url(L"/foo/toto.html", seed);
    EXPECT_CALL(*downloader, DownloadRobotsTxt(_)).WillOnce(Return(GetRobotsTxtContentThatDisallows(root)));

    politeness->SetRespectRobotTxt(true);

    politeness->SetRespectRobotTxtIfDisallowRoot(true);
    EXPECT_FALSE(politeness->CanDownloadUrl(root));

    politeness->SetRespectRobotTxtIfDisallowRoot(false);
    EXPECT_TRUE(politeness->CanDownloadUrl(root));
    EXPECT_TRUE(politeness->CanDownloadUrl(ignoredDisallowedUrl1));
    EXPECT_TRUE(politeness->CanDownloadUrl(ignoredDisallowedUrl2));
    }

TEST_F(PolitenessTester, GivenAnUnknowDomainNameWaitingToRespectCrawlDelayDownloadsTheDomainRobotsTxt)
    {
    UrlPtr unknowDomain = new Url(L"http://unknown-domain.com", seed);
    politeness->SetRespectRobotTxt(true);
    EXPECT_CALL(*downloader, DownloadRobotsTxt(Pointee(*unknowDomain))).Times(1);

    politeness->GetCrawlDelay(unknowDomain);
    }

TEST_F(PolitenessTester, PolitenessReturnsTheRightCrawlDelay)
    {
    politeness->SetRespectRobotTxt(true);
    EXPECT_CALL(*downloader, DownloadRobotsTxt(_)).WillOnce(Return(GetRobotsTxtContentThatHasCrawlDelay(10)));

    UrlPtr toDownload = new Url(L"http://toto.com", seed);
    ASSERT_EQ(10, politeness->GetCrawlDelay(toDownload));
    }

TEST_F(PolitenessTester, WhenTheCrawlDelayIsHigherThanTheMaximumDelayItIsOverridden)
    {
    uint32_t expectedDelay = 5;
    uint32_t domainCrawlDelay = 100;

    politeness->SetRespectRobotTxt(true);
    politeness->SetMaxCrawlDelay(expectedDelay);
    EXPECT_CALL(*downloader, DownloadRobotsTxt(_)).WillOnce(Return(GetRobotsTxtContentThatHasCrawlDelay(domainCrawlDelay)));

    UrlPtr toDownload = new Url(L"http://toto.com", seed);
    ASSERT_EQ(expectedDelay, politeness->GetCrawlDelay(toDownload));
    }

TEST_F(PolitenessTester, WhenTheRobotsTxtIsNotRespectedTheCrawlDelayIsZeroAndTheRobotsTxtIsNotDowloaded)
    {
    politeness->SetRespectRobotTxt(false);
    UrlPtr unknowDomain = new Url(L"http://unknown-domain.com", seed);
    EXPECT_CALL(*downloader, DownloadRobotsTxt(_)).Times(0);
    ASSERT_EQ(0, politeness->GetCrawlDelay(unknowDomain));
    }
