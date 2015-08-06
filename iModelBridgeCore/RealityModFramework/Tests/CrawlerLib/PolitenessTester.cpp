/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CrawlerLib/PolitenessTester.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <CrawlerLib/Politeness.h>
#include "./Mocks.h"

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;

using ::testing::Return;
using ::testing::Pointee;
using ::testing::_;


class RobotsTxtParserMock : public IRobotsTxtParser
    {
    public:
    MOCK_CONST_METHOD2(ParseRobotsTxt, RobotsTxtContentPtr(WString const& text, UrlPtr const& baseUrl));
    };

class SleeperMock : public ISleeper
    {
    public:
    MOCK_CONST_METHOD1(Sleep, void(uint32_t seconds));
    };

class PolitenessTester : public ::testing::Test
    {
    public:
    PolitenessTester()
        {
        seed = new Seed(L"http://some-domain.com/index.html");
        downloader = new DownloaderMock;
        parser = new RobotsTxtParserMock;
        sleeper = new SleeperMock;
        politeness = new Politeness(downloader, parser, sleeper);
        emptyRobotsTxt = new RobotsTxtContent(L"", seed);

        ON_CALL(*downloader, DownloadPage(_, _)).WillByDefault(Return(SUCCESS));
        ON_CALL(*parser, ParseRobotsTxt(_, _)).WillByDefault(Return(emptyRobotsTxt));
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
    DownloaderMock* downloader;
    RobotsTxtParserMock* parser;
    SleeperMock* sleeper;

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
    EXPECT_CALL(*downloader, DownloadPage(_, _)).Times(0);
    politeness->CanDownloadUrl(seed);
    }

TEST_F(PolitenessTester, WhenAnUnknownDomainIsQueriedTheRobotsTxtIsDownloadedTheFirstTimeOnly)
    {
    UrlPtr anotherUrlInSameDomain = new Url(L"http://some-domain.com/other_page.html", seed);
    UrlPtr expectedRobotsTxtUrl = new Url(L"http://some-domain.com/robots.txt", seed);
    UrlPtr expectedPageBaseUrl = new Url(L"http://some-domain.com", seed);
    EXPECT_CALL(*downloader, DownloadPage(_, Pointee(*expectedRobotsTxtUrl))).Times(1);
    EXPECT_CALL(*parser, ParseRobotsTxt(_, Pointee(*expectedPageBaseUrl))).Times(1);

    politeness->SetRespectRobotTxt(true);
    politeness->CanDownloadUrl(seed);
    politeness->CanDownloadUrl(anotherUrlInSameDomain);
    }

TEST_F(PolitenessTester, WhenDownloadingRobotsTxtFailsDownloadingIsNotRetriedAndEveryPageAndResultIsNotParsed)
    {
    UrlPtr anotherUrlInSameDomain = new Url(L"http://some-domain.com/other_page.html", seed);
    UrlPtr expectedUrl = new Url(L"http://some-domain.com/robots.txt", seed);
    EXPECT_CALL(*downloader, DownloadPage(_, Pointee(*expectedUrl))).Times(1).WillOnce(Return(ERROR));
    EXPECT_CALL(*parser, ParseRobotsTxt(_, _)).Times(0);

    politeness->SetRespectRobotTxt(true);
    politeness->CanDownloadUrl(seed);
    politeness->CanDownloadUrl(anotherUrlInSameDomain);
    }

TEST_F(PolitenessTester, WhenAnUrlIsInTheRobotsTxtDisallowListAllOfItSubUrlAreDisallowed)
    {
    UrlPtr disallowedUrl = new Url(L"/foo", seed);
    UrlPtr disallowedSubUrl = new Url(L"/foo/toto.html", seed);
    EXPECT_CALL(*parser, ParseRobotsTxt(_, _)).WillOnce(Return(GetRobotsTxtContentThatDisallows(disallowedUrl)));

    politeness->SetRespectRobotTxt(true);
    EXPECT_FALSE(politeness->CanDownloadUrl(disallowedUrl));
    EXPECT_FALSE(politeness->CanDownloadUrl(disallowedSubUrl));
    EXPECT_TRUE(politeness->CanDownloadUrl(seed));
    }

TEST_F(PolitenessTester, WhenRootIsDisallowedPolitenessCanDownloadFromUrlAnyway)
    {
    UrlPtr root = new Url(L"/", seed);
    UrlPtr ignoredDisallowedUrl1 = new Url(L"/foo", seed);
    UrlPtr ignoredDisallowedUrl2 = new Url(L"/foo/toto.html", seed);
    EXPECT_CALL(*parser, ParseRobotsTxt(_, _)).WillOnce(Return(GetRobotsTxtContentThatDisallows(root)));

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
    UrlPtr expectedRobotsUrl = new Url(L"http://unknown-domain.com/robots.txt", seed);
    politeness->SetRespectRobotTxt(true);
    EXPECT_CALL(*downloader, DownloadPage(_, Pointee(*expectedRobotsUrl))).Times(1);

    politeness->WaitToRespectCrawlDelayOf(unknowDomain);
    }

TEST_F(PolitenessTester, WaitingToRespectCrawlDelayCallsTheSleeperWithTheRightTimeToSleep)
    {
    politeness->SetRespectRobotTxt(true);
    EXPECT_CALL(*parser, ParseRobotsTxt(_, _)).WillOnce(Return(GetRobotsTxtContentThatHasCrawlDelay(10)));
    EXPECT_CALL(*sleeper, Sleep(10));

    UrlPtr toDownload = new Url(L"http://toto.com", seed);
    politeness->WaitToRespectCrawlDelayOf(toDownload);
    }

TEST_F(PolitenessTester, WhenTheCrawlDelayIsHigherThanTheMaximumDelayItIsOverridden)
    {
    uint32_t expectedDelay = 5;
    uint32_t domainCrawlDelay = 100;

    politeness->SetRespectRobotTxt(true);
    politeness->SetMaxCrawlDelay(expectedDelay);
    EXPECT_CALL(*parser, ParseRobotsTxt(_, _)).WillOnce(Return(GetRobotsTxtContentThatHasCrawlDelay(domainCrawlDelay)));
    EXPECT_CALL(*sleeper, Sleep(expectedDelay));

    UrlPtr toDownload = new Url(L"http://toto.com", seed);
    politeness->WaitToRespectCrawlDelayOf(toDownload);
    }

TEST_F(PolitenessTester, WhenTheRobotsTxtIsNotRespectedWaitingForCrawlDelayDoesNothing)
    {
    politeness->SetRespectRobotTxt(false);
    UrlPtr unknowDomain = new Url(L"http://unknown-domain.com", seed);
    EXPECT_CALL(*sleeper, Sleep(_)).Times(0);
    EXPECT_CALL(*downloader, DownloadPage(_, _)).Times(0);
    politeness->WaitToRespectCrawlDelayOf(unknowDomain);
    }
