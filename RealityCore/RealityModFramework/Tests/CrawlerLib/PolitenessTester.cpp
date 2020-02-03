/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    MOCK_METHOD1(DownloadRobotsTxt, RobotsTxtContentPtr(UrlCR url));
    };

class PolitenessTester : public ::testing::Test
    {
    public:
    PolitenessTester()
        {
        seed = Seed::Create(L"http://some-domain.com/index.html");
        downloader = new RobotsTxtDownloaderMock;
        politeness = new Politeness(downloader);
        emptyRobotsTxt = RobotsTxtContent::Create(L"", *seed);

        ON_CALL(*downloader, DownloadRobotsTxt(_)).WillByDefault(Return(emptyRobotsTxt));
        }

    ~PolitenessTester()
        {
        delete politeness;
        }

    RobotsTxtContentPtr GetRobotsTxtContentThatDisallows(UrlPtr url)
        {
        RobotsTxtContentPtr content = RobotsTxtContent::Create(L"", *Url::Create(L"http://some-domain.com", *seed));
        content->AddUserAgent(UserAgent(L"*"));
        content->AddDisallowedUrl(*UserAgent::Create(L"*"), url);
        return content;
        }

    RobotsTxtContentPtr GetRobotsTxtContentThatHasCrawlDelay(uint32_t delay)
        {
        RobotsTxtContentPtr content = RobotsTxtContent::Create(L"", *Url::Create(L"http://some-domain.com", *seed));
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
    ASSERT_TRUE(politeness->CanDownloadUrl(*seed));
    }

TEST_F(PolitenessTester, ByDefaultRobotsTxtAreIgnoredAndNotDownloaded)
    {
    EXPECT_CALL(*downloader, DownloadRobotsTxt(_)).Times(0);
    politeness->CanDownloadUrl(*seed);
    }

TEST_F(PolitenessTester, WhenAnUnknownDomainIsQueriedTheRobotsTxtIsDownloadedTheFirstTimeOnly)
    {
    UrlPtr anotherUrlInSameDomain = Url::Create(L"http://some-domain.com/other_page.html", *seed);
    EXPECT_CALL(*downloader, DownloadRobotsTxt(*seed)).Times(1);

    politeness->SetRespectRobotTxt(true);
    politeness->CanDownloadUrl(*seed);
    politeness->CanDownloadUrl(*anotherUrlInSameDomain);
    }

TEST_F(PolitenessTester, WhenRootIsDisallowedPolitenessCanDownloadFromUrlAnyway)
    {
    UrlPtr root = Url::Create(L"/", *seed);
    UrlPtr ignoredDisallowedUrl1 = Url::Create(L"/foo", *seed);
    UrlPtr ignoredDisallowedUrl2 = Url::Create(L"/foo/toto.html", *seed);
    EXPECT_CALL(*downloader, DownloadRobotsTxt(_)).WillOnce(Return(GetRobotsTxtContentThatDisallows(root)));

    politeness->SetRespectRobotTxt(true);

    politeness->SetRespectRobotTxtIfDisallowRoot(true);
    EXPECT_FALSE(politeness->CanDownloadUrl(*root));

    politeness->SetRespectRobotTxtIfDisallowRoot(false);
    EXPECT_TRUE(politeness->CanDownloadUrl(*root));
    EXPECT_TRUE(politeness->CanDownloadUrl(*ignoredDisallowedUrl1));
    EXPECT_TRUE(politeness->CanDownloadUrl(*ignoredDisallowedUrl2));
    }

TEST_F(PolitenessTester, GivenAnUnknowDomainNameWaitingToRespectCrawlDelayDownloadsTheDomainRobotsTxt)
    {
    UrlPtr unknowDomain = Url::Create(L"http://unknown-domain.com", *seed);
    politeness->SetRespectRobotTxt(true);
    EXPECT_CALL(*downloader, DownloadRobotsTxt(*unknowDomain)).Times(1);

    politeness->GetCrawlDelay(*unknowDomain);
    }

TEST_F(PolitenessTester, PolitenessReturnsTheRightCrawlDelay)
    {
    politeness->SetRespectRobotTxt(true);
    EXPECT_CALL(*downloader, DownloadRobotsTxt(_)).WillOnce(Return(GetRobotsTxtContentThatHasCrawlDelay(10)));

    UrlPtr toDownload = Url::Create(L"http://toto.com", *seed);
    ASSERT_EQ(10, politeness->GetCrawlDelay(*toDownload));
    }

TEST_F(PolitenessTester, WhenTheCrawlDelayIsHigherThanTheMaximumDelayItIsOverridden)
    {
    uint32_t expectedDelay = 5;
    uint32_t domainCrawlDelay = 100;

    politeness->SetRespectRobotTxt(true);
    politeness->SetMaxCrawlDelay(expectedDelay);
    EXPECT_CALL(*downloader, DownloadRobotsTxt(_)).WillOnce(Return(GetRobotsTxtContentThatHasCrawlDelay(domainCrawlDelay)));

    UrlPtr toDownload = Url::Create(L"http://toto.com", *seed);
    ASSERT_EQ(expectedDelay, politeness->GetCrawlDelay(*toDownload));
    }

TEST_F(PolitenessTester, WhenTheRobotsTxtIsNotRespectedTheCrawlDelayIsZeroAndTheRobotsTxtIsNotDowloaded)
    {
    politeness->SetRespectRobotTxt(false);
    UrlPtr unknowDomain = Url::Create(L"http://unknown-domain.com", *seed);
    EXPECT_CALL(*downloader, DownloadRobotsTxt(_)).Times(0);
    ASSERT_EQ(0, politeness->GetCrawlDelay(*unknowDomain));
    }
