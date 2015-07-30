#include "./UrlMock.h"
#include <Bentley/BeTest.h>
#include <CrawlerLib/UrlQueue.h>
#include <CrawlerLib/Url.h>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

class QueueTester : public ::testing::Test
    {
    public:
    UrlQueue queue;
    };

TEST_F(QueueTester, TheQueueIsInitiallyEmpy)
    {
    ASSERT_EQ(0, queue.NumberOfUrls());
    }

TEST_F(QueueTester, CanAddAnUrlToTheQueue)
    {
    UrlPtr anUrl = new UrlMock(L"http://toto.com");

    queue.AddUrl(anUrl);

    ASSERT_EQ(1, queue.NumberOfUrls());
    ASSERT_EQ(*anUrl, *queue.NextUrl());
    }

TEST_F(QueueTester, CanAddMultipleToQueueAndPopThemInOrder)
    {
    UrlPtr firstAddedUrl = new UrlMock(L"http://toto.com");
    UrlPtr secondAddedUrl = new UrlMock(L"http://tata.com");

    queue.AddUrl(firstAddedUrl);
    queue.AddUrl(secondAddedUrl);

    ASSERT_EQ(2, queue.NumberOfUrls());
    ASSERT_EQ(*firstAddedUrl, *queue.NextUrl());
    ASSERT_EQ(*secondAddedUrl, *queue.NextUrl());
    ASSERT_EQ(0, queue.NumberOfUrls());
    }

TEST_F(QueueTester, WhenAnUrlIsAddedMultipleTimesItIsIgnoredAfterTheFirstTime)
    {
    UrlPtr urlA = new UrlMock(L"http://toto.com");
    UrlPtr urlB = new UrlMock(L"http://tata.com");
    UrlPtr urlEqualsToA = new UrlMock(L"http://toto.com");

    queue.AddUrl(urlA);
    queue.AddUrl(urlB);
    ASSERT_EQ(2, queue.NumberOfUrls());

    queue.AddUrl(urlA);
    ASSERT_EQ(2, queue.NumberOfUrls());

    queue.AddUrl(urlEqualsToA);
    ASSERT_EQ(2, queue.NumberOfUrls());
    }

TEST_F(QueueTester, WhenTheMaxNumberOfUrlIsReachedAdditionalUrlsAreIgnored)
    {
    UrlPtr url1 = new UrlMock(L"http://1.com");
    UrlPtr url2 = new UrlMock(L"http://2.com");
    UrlPtr url3 = new UrlMock(L"http://3.com");
    UrlPtr url4 = new UrlMock(L"http://4.com");
    UrlPtr url5 = new UrlMock(L"http://5.com");

    queue.SetMaxNumberOfVisitedUrls(3);
    queue.AddUrl(url1);
    queue.AddUrl(url2);
    queue.AddUrl(url3);
    ASSERT_EQ(3, queue.NumberOfUrls());

    queue.AddUrl(url4);
    ASSERT_EQ(3, queue.NumberOfUrls());

    queue.NextUrl();
    ASSERT_EQ(2, queue.NumberOfUrls());

    queue.AddUrl(url5);
    ASSERT_EQ(2, queue.NumberOfUrls());
    }

END_BENTLEY_CRAWLERLIB_NAMESPACE
