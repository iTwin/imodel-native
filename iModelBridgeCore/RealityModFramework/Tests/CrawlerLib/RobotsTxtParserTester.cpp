/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CrawlerLib/RobotsTxtParserTester.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <CrawlerLib/Url.h>
#include <CrawlerLib/RobotsTxtParser.h>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;

class RobotsTxtParserTester : public ::testing::Test
    {
    public:
    RobotsTxtParserTester()
        {
        siteBaseUrl = new Seed(L"http://some-site.com");
        disallowAllRobotFile = L"User-agent: *\n"
                               L"Disallow: /";
        }

    UrlPtr siteBaseUrl;
    RobotsTxtParser parser;
    WString disallowAllRobotFile;

    bool SetContainsUrl(UrlPtrSet const& set, UrlPtr const& url)
        {
        return set.count(url) != 0;
        }
    };


TEST_F(RobotsTxtParserTester, TheRobotsTxtContentContainsACopyOfTheFileAndBaseUrl)
    {
    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowAllRobotFile, siteBaseUrl);
    ASSERT_TRUE(content->GetRobotsTxtFile().Equals(disallowAllRobotFile));
    ASSERT_EQ(*siteBaseUrl, *(content->GetBaseUrl()));
    }

TEST_F(RobotsTxtParserTester, CanParseUserAgentWildcard)
    {
    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowAllRobotFile, siteBaseUrl);
    vector<UserAgent> agents;
    content->GetUserAgents(agents);
    ASSERT_EQ(1, agents.size());
    ASSERT_EQ(UserAgent(L"*"), agents[0]);
    }

TEST_F(RobotsTxtParserTester, CommentAreIgnoredWhenParsingUserAgents)
    {
    WString disallowAllRobotFileWithComments = L"User-agent: * #This is a comment\n"
                                               L"#User-agent: *\n"
                                               L"# Ignore the previous line\n"
                                               L"Disallow: /";

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowAllRobotFile, siteBaseUrl);
    vector<UserAgent> agents;
    content->GetUserAgents(agents);
    ASSERT_EQ(1, agents.size());
    ASSERT_EQ(UserAgent(L"*"), agents[0]);
    }

TEST_F(RobotsTxtParserTester, DisallowFollowByBlankIsIgnored)
    {
    WString rules = L"User-agent: *\n"
                    L"Disallow: \n";
    RobotsTxtContentPtr content = parser.ParseRobotsTxt(rules, siteBaseUrl);
    UrlPtrSet disallowedUrls;
    content->GetDisallowedUrlsOf(disallowedUrls, UserAgent(L"*"));

    ASSERT_EQ(0, disallowedUrls.size());
    }

TEST_F(RobotsTxtParserTester, CanDisallowTheRoot)
    {
    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowAllRobotFile, siteBaseUrl);
    UrlPtrSet disallowedUrlsOfStar;
    content->GetDisallowedUrlsOf(disallowedUrlsOfStar, UserAgent(L"*"));
    ASSERT_EQ(1, disallowedUrlsOfStar.size());
    ASSERT_TRUE(SetContainsUrl(disallowedUrlsOfStar, siteBaseUrl));
    }

TEST_F(RobotsTxtParserTester, CanDisallowARelativeUrl)
    {
    WString disallowRelativeUrl = L"User-agent: *\n"
                                  L"Disallow: /toto.html";
    UrlPtr expectedDisallowedUrl = new Url(L"/toto.html", siteBaseUrl);

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowRelativeUrl, siteBaseUrl);
    UrlPtrSet disallowedUrlsOfStar;
    content->GetDisallowedUrlsOf(disallowedUrlsOfStar, UserAgent(L"*"));

    ASSERT_EQ(1, disallowedUrlsOfStar.size());
    ASSERT_TRUE(SetContainsUrl(disallowedUrlsOfStar, expectedDisallowedUrl));
    }

TEST_F(RobotsTxtParserTester, CanDisallowAMultipleRelativeUrls)
    {
    WString disallowRelativeUrl = L"User-agent: *\n"
                                  L"Disallow: /toto.html\n"
                                  L"Disallow: /tata.html\n";
    UrlPtr expectedDisallowedUrl1 = new Url(L"/toto.html", siteBaseUrl);
    UrlPtr expectedDisallowedUrl2 = new Url(L"/tata.html", siteBaseUrl);

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowRelativeUrl, siteBaseUrl);
    UrlPtrSet disallowedUrlsOfStar;
    content->GetDisallowedUrlsOf(disallowedUrlsOfStar, UserAgent(L"*"));

    ASSERT_EQ(2, disallowedUrlsOfStar.size());
    ASSERT_TRUE(SetContainsUrl(disallowedUrlsOfStar, expectedDisallowedUrl1));
    ASSERT_TRUE(SetContainsUrl(disallowedUrlsOfStar, expectedDisallowedUrl2));
    }

TEST_F(RobotsTxtParserTester, CanDisallowMultipleBotsAtTheSameTime)
    {
    WString disallowRules = L"User-agent: awesomebot\n"
                            L"User-agent: coolbot\n"
                            L"Disallow: /toto.html\n"
                            L"Disallow: /tata.html\n";

    UrlPtr expectedDisallowedUrl1 = new Url(L"/toto.html", siteBaseUrl);
    UrlPtr expectedDisallowedUrl2 = new Url(L"/tata.html", siteBaseUrl);

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowRules, siteBaseUrl);
    UrlPtrSet disallowedUrlsOfAwesomebot;
    content->GetDisallowedUrlsOf(disallowedUrlsOfAwesomebot, UserAgent(L"awesomebot"));

    UrlPtrSet disallowedUrlsOfCoolbot;
    content->GetDisallowedUrlsOf(disallowedUrlsOfCoolbot, UserAgent(L"coolbot"));

    ASSERT_EQ(2, disallowedUrlsOfAwesomebot.size());
    ASSERT_TRUE(SetContainsUrl(disallowedUrlsOfAwesomebot, expectedDisallowedUrl1));
    ASSERT_TRUE(SetContainsUrl(disallowedUrlsOfAwesomebot, expectedDisallowedUrl2));

    ASSERT_EQ(2, disallowedUrlsOfCoolbot.size());
    ASSERT_TRUE(SetContainsUrl(disallowedUrlsOfCoolbot, expectedDisallowedUrl1));
    ASSERT_TRUE(SetContainsUrl(disallowedUrlsOfCoolbot, expectedDisallowedUrl2));
    }


TEST_F(RobotsTxtParserTester, CanHaveDifferentRulesForDifferentBots)
    {
    WString disallowRules = L"User-agent: awesomebot\n"
                            L"Disallow: /toto.html\n"
                            L"\n"
                            L"User-agent: coolbot\n"
                            L"Disallow: /tata.html\n";

    UrlPtr expectedDisallowedUrlOfAwesomeBot = new Url(L"/toto.html", siteBaseUrl);
    UrlPtr expectedDisallowedUrlOfCoolBot = new Url(L"/tata.html", siteBaseUrl);

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowRules, siteBaseUrl);

    UrlPtrSet disallowedUrlsOfAwesomebot;
    content->GetDisallowedUrlsOf(disallowedUrlsOfAwesomebot, UserAgent(L"awesomebot"));

    UrlPtrSet disallowedUrlsOfCoolbot;
    content->GetDisallowedUrlsOf(disallowedUrlsOfCoolbot, UserAgent(L"coolbot"));

    ASSERT_EQ(1, disallowedUrlsOfAwesomebot.size());
    ASSERT_TRUE(SetContainsUrl(disallowedUrlsOfAwesomebot, expectedDisallowedUrlOfAwesomeBot));

    ASSERT_EQ(1, disallowedUrlsOfCoolbot.size());
    ASSERT_TRUE(SetContainsUrl(disallowedUrlsOfCoolbot, expectedDisallowedUrlOfCoolBot));
    }

TEST_F(RobotsTxtParserTester, WildcardDisallowsAppliesToEveryUserAgent)
    {
    WString disallowRules = L"User-agent: *\n"
                            L"Disallow: /toto.html\n"
                            L"\n"
                            L"User-agent: coolbot\n"
                            L"Disallow: /tata.html\n";

    UrlPtr expectedDisallowedUrl1 = new Url(L"/toto.html", siteBaseUrl);
    UrlPtr expectedDisallowedUrl2 = new Url(L"/tata.html", siteBaseUrl);

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowRules, siteBaseUrl);
    UrlPtrSet disallowedUrls;
    content->GetDisallowedUrlsOf(disallowedUrls, UserAgent(L"coolbot"));

    ASSERT_EQ(2, disallowedUrls.size());
    ASSERT_TRUE(SetContainsUrl(disallowedUrls, expectedDisallowedUrl1));
    ASSERT_TRUE(SetContainsUrl(disallowedUrls, expectedDisallowedUrl2));
    }

TEST_F(RobotsTxtParserTester, NoRulesAppliesToAnUnknownUserAgentIfAWildcardIsNotSupplied)
    {
    WString disallowRules = L"User-agent: coolbot\n"
                            L"Disallow: /tata.html\n";

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowRules, siteBaseUrl);
    UrlPtrSet disallowedUrlsOfUnknowedAgent;
    content->GetDisallowedUrlsOf(disallowedUrlsOfUnknowedAgent, UserAgent(L"unknowed"));

    ASSERT_EQ(0, disallowedUrlsOfUnknowedAgent.size());
    }

TEST_F(RobotsTxtParserTester, ByDefaultThereIsNoCrawlDelay)
    {
    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowAllRobotFile, siteBaseUrl);
    ASSERT_EQ(0, content->GetCrawlDelay(UserAgent(L"*")));
    ASSERT_EQ(0, content->GetCrawlDelay(UserAgent(L"some-bot-never-mentioned")));
    }

TEST_F(RobotsTxtParserTester, CanParseCrawlDelay)
    {
    WString delayRules = L"User-agent: awesomebot\n"
                         L"Crawl-delay: 10\n"
                         L"User-agent: coolbot\n"
                         L"Crawl-delay :   21 \n";
    RobotsTxtContentPtr content = parser.ParseRobotsTxt(delayRules, siteBaseUrl);
    ASSERT_EQ(10, content->GetCrawlDelay(UserAgent(L"awesomebot")));
    ASSERT_EQ(21, content->GetCrawlDelay(UserAgent(L"coolbot")));
    }

TEST_F(RobotsTxtParserTester, CanTellIfRootIsDisallowed)
    {
    WString rules = L"User-agent: awesomebot\n"
                    L"Disallow: /\n"
                    L"User-agent: coolbot\n"
                    L"Disallow: /toto.html\n";
    RobotsTxtContentPtr content = parser.ParseRobotsTxt(rules, siteBaseUrl);

    ASSERT_TRUE(content->IsRootDisallowed(UserAgent(L"awesomebot")));
    ASSERT_FALSE(content->IsRootDisallowed(UserAgent(L"coolbot")));
    }
