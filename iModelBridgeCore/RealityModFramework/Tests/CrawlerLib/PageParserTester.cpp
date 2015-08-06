/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CrawlerLib/PageParserTester.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <CrawlerLib/PageParser.h>
#include "./Mocks.h"

USING_NAMESPACE_BENTLEY_CRAWLERLIB

class PageParserTester : public ::testing::Test
    {
    public:
    virtual void SetUp()
        {
        htmlPage = L"<!DOCTYPE html>\n"
                   L"<html>\n"
                   L"<head>\n"
                   L"<title>Page Title</title>\n"
                   L"</head>\n"
                   L"<body>\n"
                   L"\n"
                   L"<h1>This is a Heading</h1>\n"
                   L"<p>This is a paragraph.</p>\n"
                   L"<a href=\"http://a.com\">Visit site a!</a><a href=\"http://c.gov.qc.ca\">Visit site c!</a>\n"
                   L"<  a   href =    \n"
                   L"\"http://b.ca\"  >Visit site b!</a>\n"
                   L"\n"
                   L"</body>\n"
                   L"</html>\n";

        htmlPageWithRelNoFollow = L"<!DOCTYPE html>\n"
                                  L"<html>\n"
                                  L"<head>\n"
                                  L"<title>Page Title</title>\n"
                                  L"</head>\n"
                                  L"<body>\n"
                                  L"\n"
                                  L"<h1>This is a Heading</h1>\n"
                                  L"<p>This is a paragraph.</p>\n"
                                  L"<a href=\"http://a.com\" rel=\"nofollow\">Visit site a!</a>\n"
                                  L"<  a   href =    \n"
                                  L"\"http://b.ca\"   rel   =  \n"
                                  L"\"nofollow\"  >Visit site b!</a>\n"
                                  L"\n"
                                  L"</body>\n"
                                  L"</html>\n";

        htmlPageWithNoFollowMetaTag = L"<!DOCTYPE html>\n"
                                      L"<html>\n"
                                      L"<head>\n"
                                      L"<title>Page Title</title>\n"
                                      L"<META name=\"robots\" content=\"noindex, nofollow\">\n"
                                      L"</head>\n"
                                      L"<body>\n"
                                      L"\n"
                                      L"<h1>This is a Heading</h1>\n"
                                      L"<p>This is a paragraph.</p>\n"
                                      L"<a href=\"http://a.com\">Visit site a!</a><a href=\"http://c.gov.qc.ca\">Visit site c!</a>\n"
                                      L"<  a   href =    \n"
                                      L"\"http://b.ca\"  >Visit site b!</a>\n"
                                      L"\n"
                                      L"</body>\n"
                                      L"</html>\n";

        linkA = new UrlMock(L"http://a.com");
        linkB = new UrlMock(L"http://b.ca");
        linkC = new UrlMock(L"http://c.gov.qc.ca");

        pageURL = new UrlMock(L"http://www.page-url.com");
        }

    WString htmlPage;
    WString htmlPageWithRelNoFollow;
    WString htmlPageWithNoFollowMetaTag;
    UrlPtr linkA;
    UrlPtr linkB;
    UrlPtr linkC;
    UrlPtr pageURL;

    PageParser parser;
    };


bool bvectorContains(bvector<UrlPtr> vector, UrlPtr element)
    {
    for(auto e : vector)
        {
        if(*e == *element)
            return true;
        }
    return false;
    }

TEST_F(PageParserTester, ThePageContentReturnedHasTheRightURL)
    {
    PageContentPtr content = parser.ParsePage(htmlPage, pageURL);

    ASSERT_EQ(*pageURL, content->GetUrl());
    }

TEST_F(PageParserTester, ThePageContentReturnedHasTheRightText)
    {
    PageContentPtr content = parser.ParsePage(htmlPage, pageURL);

    ASSERT_STREQ(htmlPage.c_str(), content->GetText().c_str());
    }

TEST_F(PageParserTester, ThePageContentReturnedContainsAllLinks)
    {
    PageContentPtr content = parser.ParsePage(htmlPage, pageURL);
    size_t numberOfLinks = content->GetLinks().size();

    ASSERT_EQ(3, numberOfLinks);
    ASSERT_TRUE(bvectorContains(content->GetLinks(), linkA));
    ASSERT_TRUE(bvectorContains(content->GetLinks(), linkB));
    ASSERT_TRUE(bvectorContains(content->GetLinks(), linkC));
    }

TEST_F(PageParserTester, CanIgnoreLinksMarkedNoFollow)
    {
    parser.SetParseLinksRelNoFollow(true);
    PageContentPtr contentWithAllLinks = parser.ParsePage(htmlPageWithRelNoFollow, pageURL);

    size_t numberOfLinks = contentWithAllLinks->GetLinks().size();
    ASSERT_EQ(2, numberOfLinks);
    ASSERT_TRUE(bvectorContains(contentWithAllLinks->GetLinks(), linkA));
    ASSERT_TRUE(bvectorContains(contentWithAllLinks->GetLinks(), linkB));

    parser.SetParseLinksRelNoFollow(false);
    PageContentPtr contentWithoutNoFollowLinks = parser.ParsePage(htmlPageWithRelNoFollow, pageURL);

    numberOfLinks = contentWithoutNoFollowLinks->GetLinks().size();
    ASSERT_EQ(0, numberOfLinks);
    }

TEST_F(PageParserTester, CanIgnoreAllLinksIfPageHasRobotsNoFollowMetaTag)
    {
    parser.SetParsePagesWithNoFollowMetaTag(true);
    PageContentPtr content = parser.ParsePage(htmlPageWithNoFollowMetaTag, pageURL);
    size_t numberOfLinks = content->GetLinks().size();
    ASSERT_EQ(3, numberOfLinks);
    
    parser.SetParsePagesWithNoFollowMetaTag(false);
    content = parser.ParsePage(htmlPageWithNoFollowMetaTag, pageURL);
    numberOfLinks = content->GetLinks().size();
    ASSERT_EQ(0, numberOfLinks);
    }
