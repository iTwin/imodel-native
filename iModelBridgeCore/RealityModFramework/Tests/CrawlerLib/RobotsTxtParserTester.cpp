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
                                               L"#User-agent: toto\n"
                                               L"# Ignore the previous line\n"
                                               L"Disallow: /";

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowAllRobotFile, siteBaseUrl);
    vector<UserAgent> agents;
    content->GetUserAgents(agents);
    ASSERT_EQ(1, agents.size());
    ASSERT_EQ(UserAgent(L"*"), agents[0]);
    }

TEST_F(RobotsTxtParserTester, CommentAreIgnoredWhenParsingAllowsAndDisallows)
    {
    WString allowsAndDisallowsWithComments = L"User-agent: *\n"
                                             L"Allow: /tata.html #comment\n"
                                             L"# Allow: /toto.html\n"
                                             L"Disallow: /toto.html #comment\n"
                                             L"#Disallow: /bar";

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(allowsAndDisallowsWithComments, siteBaseUrl);
    UrlPtr expectedAllowedUrl = new Url(L"/tata.html", siteBaseUrl);
    UrlPtr expectedDisallowedUrl = new Url(L"/toto.html", siteBaseUrl);

    ASSERT_FALSE(content->IsUrlDisallowed(expectedAllowedUrl, UserAgent(L"*")));
    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrl, UserAgent(L"*")));
    }

TEST_F(RobotsTxtParserTester, DisallowFollowByBlankIsIgnored)
    {
    WString rules = L"User-agent: *\n"
                    L"Disallow: \n";
    RobotsTxtContentPtr content = parser.ParseRobotsTxt(rules, siteBaseUrl);

    ASSERT_FALSE(content->IsUrlDisallowed(siteBaseUrl, UserAgent(L"*")));
    }

TEST_F(RobotsTxtParserTester, CanDisallowTheRoot)
    {
    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowAllRobotFile, siteBaseUrl);
    UrlPtrSet disallowedUrlsOfStar;
    ASSERT_TRUE(content->IsUrlDisallowed(siteBaseUrl, UserAgent(L"*")));
    }

TEST_F(RobotsTxtParserTester, CanDisallowARelativeUrl)
    {
    WString disallowRelativeUrl = L"User-agent: *\n"
                                  L"Disallow: /toto.html";
    UrlPtr expectedDisallowedUrl = new Url(L"/toto.html", siteBaseUrl);

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowRelativeUrl, siteBaseUrl);
    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrl, UserAgent(L"*")));
    }

TEST_F(RobotsTxtParserTester, CanDisallowAMultipleRelativeUrls)
    {
    WString disallowRelativeUrl = L"User-agent: *\n"
                                  L"Disallow: /toto.html\n"
                                  L"Disallow: /tata.html\n";
    UrlPtr expectedDisallowedUrl1 = new Url(L"/toto.html", siteBaseUrl);
    UrlPtr expectedDisallowedUrl2 = new Url(L"/tata.html", siteBaseUrl);

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowRelativeUrl, siteBaseUrl);

    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrl1, UserAgent(L"*")));
    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrl2, UserAgent(L"*")));
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

    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrl1, UserAgent(L"awesomebot")));
    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrl2, UserAgent(L"awesomebot")));

    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrl1, UserAgent(L"coolbot")));
    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrl2, UserAgent(L"coolbot")));
    }


TEST_F(RobotsTxtParserTester, CanHaveDifferentRulesForDifferentBots)
    {
    WString disallowRules = L"User-agent: awesomebot\n"
                            L"Disallow: /toto.html\n"
                            L"\n"
                            L"User-agent: coolbot\n"
                            L"Allow: /foo/tata.html\n"
                            L"Disallow: /foo\n";

    UrlPtr expectedDisallowedUrlOfAwesomeBot = new Url(L"/toto.html", siteBaseUrl);
    UrlPtr expectedAllowedUrlOfCoolBot = new Url(L"/foo/tata.html", siteBaseUrl);
    UrlPtr expectedDisallowedUrlOfCoolBot = new Url(L"/foo/toto.html", siteBaseUrl);

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowRules, siteBaseUrl);

    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrlOfAwesomeBot, UserAgent(L"awesomebot")));

    ASSERT_FALSE(content->IsUrlDisallowed(expectedAllowedUrlOfCoolBot, UserAgent(L"coolbot")));
    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrlOfCoolBot, UserAgent(L"coolbot")));
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

    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrl1, UserAgent(L"coolbot")));
    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrl2, UserAgent(L"coolbot")));
    }

TEST_F(RobotsTxtParserTester, NoRulesAppliesToAnUnknownUserAgentIfAWildcardIsNotSupplied)
    {
    WString disallowRules = L"User-agent: coolbot\n"
                            L"Disallow: /tata.html\n";
    UrlPtr expectedAllowedUrl1 = new Url(L"/", siteBaseUrl);
    UrlPtr expectedAllowedUrl2 = new Url(L"/tata.html", siteBaseUrl);

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(disallowRules, siteBaseUrl);

    ASSERT_FALSE(content->IsUrlDisallowed(expectedAllowedUrl1, UserAgent(L"unknowed")));
    ASSERT_FALSE(content->IsUrlDisallowed(expectedAllowedUrl2, UserAgent(L"unknowed")));
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

TEST_F(RobotsTxtParserTester, WildcardCrawlDelayAppliesToEveryUserAgent)
    {
    WString delayRules = L"User-agent: *\n"
                         L"Crawl-delay: 10";
    RobotsTxtContentPtr content = parser.ParseRobotsTxt(delayRules, siteBaseUrl);
    ASSERT_EQ(10, content->GetCrawlDelay(UserAgent(L"awesomebot")));
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

TEST_F(RobotsTxtParserTester, CanAllowSpecificUrls)
    {
    WString rules = L"User-agent: *\n"
                    L"Allow: /toto.html\n"
                    L"allow : /tata.html\n";
    UrlPtr expectedAllowedUrl1 = new Url(L"/toto.html", siteBaseUrl);
    UrlPtr expectedAllowedUrl2 = new Url(L"/tata.html", siteBaseUrl);

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(rules, siteBaseUrl);

    ASSERT_FALSE(content->IsUrlDisallowed(expectedAllowedUrl1, UserAgent(L"*")));
    ASSERT_FALSE(content->IsUrlDisallowed(expectedAllowedUrl2, UserAgent(L"*")));
    }

TEST_F(RobotsTxtParserTester, WhenParsingAnAllowedUrlItIsIgnoredIfItWasDisallowedPreviously) //See https://en.wikipedia.org/wiki/Robots_exclusion_standard#Allow_directive
    {
    WString rules = L"User-agent: *\n"
                    L"Disallow: /foo/\n"
                    L"Allow: /foo/toto.html\n"
                    L"Allow: /tata.html\n";
    UrlPtr expectedAllowedUrl = new Url(L"/tata.html", siteBaseUrl);
    UrlPtr expectedDisallowedUrl = new Url(L"/foo/toto.html", siteBaseUrl);

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(rules, siteBaseUrl);

    ASSERT_FALSE(content->IsUrlDisallowed(expectedAllowedUrl, UserAgent(L"*")));
    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrl, UserAgent(L"*")));
    }

TEST_F(RobotsTxtParserTester, WhenAnUrlIsDisallowListAllOfItSubUrlAreDisallowed)
    {
    WString rules = L"User-agent: *\n"
                    L"Disallow: /foo/\n";
    UrlPtr expectedDisallowedUrl1 = new Url(L"/foo", siteBaseUrl);
    UrlPtr expectedDisallowedUrl2 = new Url(L"/foo/toto.html", siteBaseUrl);
    UrlPtr expectedDisallowedUrl3 = new Url(L"/foo/bar/tata.html", siteBaseUrl);

    RobotsTxtContentPtr content = parser.ParseRobotsTxt(rules, siteBaseUrl);

    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrl1, UserAgent(L"*")));
    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrl2, UserAgent(L"*")));
    ASSERT_TRUE(content->IsUrlDisallowed(expectedDisallowedUrl3, UserAgent(L"*")));
    }

