#include <Bentley/BeTest.h>
#include <CrawlerLib/PageParser.h>
#include "./UrlMock.h"

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

        linkA = new UrlMock(L"http://a.com");
        linkB = new UrlMock(L"http://b.ca");
        linkC = new UrlMock(L"http://c.gov.qc.ca");

        pageURL = new UrlMock(L"http://www.page-url.com");
        }

    WString htmlPage;
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
    PageContentPtr content = parser.ParsePage(pageURL, htmlPage);

    ASSERT_EQ(*pageURL, content->GetUrl());
    }

TEST_F(PageParserTester, ThePageContentReturnedHasTheRightText)
    {
    PageContentPtr content = parser.ParsePage(pageURL, htmlPage);

    ASSERT_STREQ(htmlPage.c_str(), content->GetText().c_str());
    }

TEST_F(PageParserTester, ThePageContentReturnedContainsAllLinks)
    {
    PageContentPtr content = parser.ParsePage(pageURL, htmlPage);
    size_t numberOfLinks = content->GetLinks().size();

    ASSERT_EQ(3, numberOfLinks);
    ASSERT_TRUE(bvectorContains(content->GetLinks(), linkA));
    ASSERT_TRUE(bvectorContains(content->GetLinks(), linkB));
    ASSERT_TRUE(bvectorContains(content->GetLinks(), linkC));
    }
