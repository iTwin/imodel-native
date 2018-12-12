/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CrawlerLib/UrlTester.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

        parentUrl = Seed::Create(L"http://seed.com");
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
    SeedPtr seed = Seed::Create(aValidUrlString);
    ASSERT_TRUE(seed->GetParent().IsNull());
    }

TEST_F(UrlTester, TheSeedHasADepthOfZero)
    {
    SeedPtr seed = Seed::Create(aValidUrlString);
    ASSERT_EQ(0, seed->GetDepth());
    }

TEST_F(UrlTester, AnUrlIsOneStepDeeperThanItsParent)
    {
    SeedPtr seed = Seed::Create(aValidUrlString);

    UrlPtr urlWithDepthOfOne = Url::Create(aValidUrlString, *seed);
    ASSERT_EQ(1, urlWithDepthOfOne->GetDepth());

    UrlPtr urlWithDepthOfTwo = Url::Create(aValidRelativeLink, *urlWithDepthOfOne);
    ASSERT_EQ(2, urlWithDepthOfTwo->GetDepth());
    }

TEST_F(UrlTester, WhenAnUrlHasADifferentDomainThanItsParentItIsMarkedAsExternal)
    {
    SeedPtr seed = Seed::Create(L"http://some-domain.com");
    UrlPtr linkedInternalUrl = Url::Create(L"http://some-domain.com/foo.html", *seed);
    UrlPtr linkedInternalRelativeUrl = Url::Create(L"/toto.html", *seed);
    UrlPtr linkedExternalUrl = Url::Create(L"http://some-other-domain.com", *seed);

    ASSERT_FALSE(linkedInternalUrl->IsExternalPage());
    ASSERT_FALSE(linkedInternalRelativeUrl->IsExternalPage());
    ASSERT_TRUE(linkedExternalUrl->IsExternalPage());
    }

TEST_F(UrlTester, TheParentIsCorrectlySet)
    {
    UrlPtr url = Url::Create(aValidRelativeLink, *parentUrl);
    ASSERT_EQ(*parentUrl, *url->GetParent());
    }

TEST_F(UrlTester, CanParseUrlDomain)
    {
    UrlPtr url = Url::Create(aValidUrlString, *parentUrl);
    ASSERT_STREQ(L"the-domain.com", url->GetDomainName().GetWString().c_str());
    }

TEST_F(UrlTester, CanParseUrlWithWwwDomain)
    {
    UrlPtr url = Url::Create(aValidUrlStringWith_www, *parentUrl);
    ASSERT_STREQ(L"the-domain.com", url->GetDomainName().GetWString().c_str());
    }

TEST_F(UrlTester, CanParseARelativeUrl)
    {
    UrlPtr finalUrl = Url::Create(aValidRelativeLink, *parentUrl);
    UrlPtr expectedUrl = Url::Create(L"http://seed.com/relative_link", *parentUrl);
    ASSERT_EQ(*expectedUrl, *finalUrl);
    }

TEST_F(UrlTester, CanParseARelativeUrlThatRepresentsTheRoot)
    {
    UrlPtr finalUrl = Url::Create(L"/", *parentUrl);
    UrlPtr expectedUrl = Url::Create(L"http://seed.com", *parentUrl);
    ASSERT_EQ(*expectedUrl, *finalUrl);
    }

TEST_F(UrlTester, CanParseARelativeUrlWithDot)
    {
    UrlPtr finalUrl = Url::Create(aValidRelativeLinkWithDot, *parentUrl);
    UrlPtr expectedUrl = Url::Create(L"http://seed.com/relative_link", *parentUrl);
    ASSERT_EQ(*expectedUrl, *finalUrl);
    }

TEST_F(UrlTester, TrailingSlashIsRemoved)
    {
    UrlPtr anUrlWithTrailingSlash = Url::Create(L"http://toto.com/hello_world/", *parentUrl);
    UrlPtr expectedUrl = Url::Create(L"http://toto.com/hello_world", *parentUrl);
    ASSERT_EQ(*expectedUrl, *anUrlWithTrailingSlash);
    }

TEST_F(UrlTester, SubUrlTester)
    {
    UrlPtr folder = Url::Create(L"http://www.toto.com/foo", *parentUrl);
    ASSERT_TRUE(folder->IsSubUrlOf(*folder)); //An url is a sub url of ifself

    UrlPtr fileInFolder = Url::Create(L"http://www.toto.com/foo/bar.html", *parentUrl);
    ASSERT_TRUE(fileInFolder->IsSubUrlOf(*folder));

    UrlPtr root = Url::Create(L"http://www.toto.com", *parentUrl);
    ASSERT_TRUE(folder->IsSubUrlOf(*root));

    UrlPtr unrelated = Url::Create(L"http://www.tata.com/foo", *parentUrl);
    ASSERT_FALSE(fileInFolder->IsSubUrlOf(*unrelated));

    ASSERT_FALSE(root->IsSubUrlOf(*fileInFolder));
    ASSERT_FALSE(folder->IsSubUrlOf(*fileInFolder));
    }
