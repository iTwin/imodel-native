/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CrawlerLib/UrlTester.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <CrawlerLib/Url.h>

USING_NAMESPACE_BENTLEY_CRAWLERLIB

class UrlTester : public ::testing::Test
    {
    public:
    UrlTester()
        {
        aValidUrlString = L"http://the-domain.com/toto.html";
        aValidUrlStringWith_www = L"http://www.the-domain.com/toto.html";
        anInvalidUrl = L"http://the-domain/toto.html";

        aValidRelativeLink = L"/relative_link";
        aValidRelativeLinkWithDot = L"./relative_link";

        parentUrl = new Seed(L"http://seed.com");
        }

    WString aValidUrlString;
    WString aValidUrlStringWith_www;
    WString anInvalidUrl;

    WString aValidRelativeLink;
    WString aValidRelativeLinkWithDot;
    WString anInvalidRelativeLink;

    SeedPtr parentUrl;
    };


TEST_F(UrlTester, TheSeedHasNoParent)
    {
    Seed seed(aValidUrlString);
    ASSERT_TRUE(seed.GetParent().IsNull());
    }

TEST_F(UrlTester, TheSeedHasADepthOfZero)
    {
    Seed seed(aValidUrlString);
    ASSERT_EQ(0, seed.GetDepth());
    }

TEST_F(UrlTester, AnUrlIsOneStepDeeperThanItsParent)
    {
    SeedPtr seed = new Seed(aValidUrlString);

    UrlPtr urlWithDepthOfOne = new Url(aValidUrlString, seed);
    ASSERT_EQ(1, urlWithDepthOfOne->GetDepth());

    UrlPtr urlWithDepthOfTwo = new Url(aValidRelativeLink, urlWithDepthOfOne);
    ASSERT_EQ(2, urlWithDepthOfTwo->GetDepth());
    }

TEST_F(UrlTester, WhenAnUrlHasADifferentDomainThanItsParentItIsMarkedAsExternal)
    {
    SeedPtr seed = new Seed(L"http://some-domain.com");
    UrlPtr linkedInternalUrl = new Url(L"http://some-domain.com/foo.html", seed);
    UrlPtr linkedInternalRelativeUrl = new Url(L"/toto.html", seed);
    UrlPtr linkedExternalUrl = new Url(L"http://some-other-domain.com", seed);

    ASSERT_FALSE(linkedInternalUrl->IsExternalPage());
    ASSERT_FALSE(linkedInternalRelativeUrl->IsExternalPage());
    ASSERT_TRUE(linkedExternalUrl->IsExternalPage());
    }

TEST_F(UrlTester, TheParentIsCorrectlySet)
    {
    Url url(aValidRelativeLink, parentUrl);
    ASSERT_EQ(*parentUrl, *url.GetParent());
    }

TEST_F(UrlTester, CanParseUrlDomain)
    {
    Url url(aValidUrlString, parentUrl);
    ASSERT_STREQ(L"the-domain.com", url.GetDomainName().c_str());
    }

TEST_F(UrlTester, CanParseUrlWithWwwDomain)
    {
    Url url(aValidUrlStringWith_www, parentUrl);
    ASSERT_STREQ(L"the-domain.com", url.GetDomainName().c_str());
    }

TEST_F(UrlTester, CanParseARelativeUrl)
    {
    Url finalUrl(aValidRelativeLink, parentUrl);
    Url expectedUrl = Url(L"http://seed.com/relative_link", parentUrl);
    ASSERT_EQ(expectedUrl, finalUrl);
    }

TEST_F(UrlTester, CanParseARelativeUrlThatRepresentsTheRoot)
    {
    Url finalUrl(L"/", parentUrl);
    Url expectedUrl = Url(L"http://seed.com", parentUrl);
    ASSERT_EQ(expectedUrl, finalUrl);
    }

TEST_F(UrlTester, CanParseARelativeUrlWithDot)
    {
    Url finalUrl(aValidRelativeLinkWithDot, parentUrl);
    Url expectedUrl = Url(L"http://seed.com/relative_link", parentUrl);
    ASSERT_EQ(expectedUrl, finalUrl);
    }

TEST_F(UrlTester, TrailingSlashIsRemoved)
    {
    Url anUrlWithTrailingSlash(L"http://toto.com/hello_world/", parentUrl);
    Url expectedUrl(L"http://toto.com/hello_world", parentUrl);
    ASSERT_EQ(expectedUrl, anUrlWithTrailingSlash);
    }

TEST_F(UrlTester, SubUrlTester)
    {
    Url folder(L"http://www.toto.com/foo", parentUrl);
    ASSERT_TRUE(folder.IsSubUrlOf(folder)); //An url is a sub url of ifself

    Url fileInFolder(L"http://www.toto.com/foo/bar.html", parentUrl);
    ASSERT_TRUE(fileInFolder.IsSubUrlOf(folder));

    Url root(L"http://www.toto.com", parentUrl);
    ASSERT_TRUE(folder.IsSubUrlOf(root));

    Url unrelated(L"http://www.tata.com/foo", parentUrl);
    ASSERT_FALSE(fileInFolder.IsSubUrlOf(unrelated));

    ASSERT_FALSE(root.IsSubUrlOf(fileInFolder));
    ASSERT_FALSE(folder.IsSubUrlOf(fileInFolder));
    }
