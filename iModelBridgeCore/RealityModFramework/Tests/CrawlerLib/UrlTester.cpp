#include <Bentley/BeTest.h>
#include <CrawlerLib/Url.h>

USING_NAMESPACE_BENTLEY_CRAWLERLIB

WString aValidUrlString = L"http://the-domain.com/toto.html";
WString aValidUrlStringWith_www = L"http://www.the-domain.com/toto.html";
WString anInvalidUrl = L"http://the-domain/toto.html";

WString aValidRelativeLink = L"/relative_link";
WString aValidRelativeLinkWithDot = L"./relative_link";
WString anInvalidRelativeLink = L"not_relative_link";


TEST(UrlTester, CanParseValidUrlDomain)
    {
    Url url(aValidUrlString);
    ASSERT_STREQ(L"the-domain.com", url.GetDomainName().c_str());
    }

TEST(UrlTester, CanValidateAbsoluteLinks)
    {
    ASSERT_TRUE(Url::IsValidAbsoluteLink(aValidUrlString));
    ASSERT_TRUE(Url::IsValidAbsoluteLink(aValidUrlStringWith_www));
    ASSERT_FALSE(Url::IsValidAbsoluteLink(anInvalidUrl));
    }

TEST(UrlTester, CanValidateRelativeLinks)
    {
    ASSERT_TRUE(Url::IsValidRelativeLink(aValidRelativeLink));
    ASSERT_TRUE(Url::IsValidRelativeLink(aValidRelativeLinkWithDot));
    ASSERT_FALSE(Url::IsValidRelativeLink(anInvalidRelativeLink));
    }

TEST(UrlTester, CanParseARelativeUrl)
    {
    Url baseUrl(L"http://toto.com/");

    Url finalUrl(baseUrl, aValidRelativeLink);

    ASSERT_EQ(Url(L"http://toto.com/relative_link"), finalUrl);
    }

TEST(UrlTester, CanParseARelativeUrlWithDot)
    {
    Url baseUrl(L"http://toto.com/foo/bar.html");

    Url finalUrl(baseUrl, aValidRelativeLinkWithDot);

    ASSERT_EQ(Url(L"http://toto.com/foo/relative_link"), finalUrl);
    }
