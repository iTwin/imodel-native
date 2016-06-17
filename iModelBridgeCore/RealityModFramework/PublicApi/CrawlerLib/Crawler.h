/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/Crawler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/PageContent.h>

#include <Bentley/Bentley.h>
#include <Bentley/bvector.h>
#include <Bentley/BeThread.h>

#ifndef _M_CEE
	#include <future>
#include <vector>
#endif /* _M_CEE */

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

struct IPageDownloader;
struct UrlQueue;

//=======================================================================================
//! @bsiclass
// Defines the crawler observer interface as a abstract class.
// The crawler observer that must inherit from the present one must implement
// a behavior for the OnPageCrawled() method.
//=======================================================================================
struct ICrawlerObserver
    {
    public:
    virtual ~ICrawlerObserver() {}

    //=======================================================================================
    // OnPageCrawled is called during the call process whenever a page is discovered by the 
    // crawler. The page content object given defines the page discovered. It is up to
    // the effective crawler observer to discard, classify, copy or do whatever process
    // is required on the page.
    //=======================================================================================
    virtual void OnPageCrawled(PageContentCR page) = 0;
    };

//=======================================================================================
//! @bsiclass
// The crawler is the main concept in the crawler library. The crawler is responsible of
// orchestrating the crawling process. 
// In a normal context a crawler is created, then configured and the the crawling
// process is started. During the crawl process a crawl observer will be called regularly
// when a page has been discovered.
//=======================================================================================
struct Crawler : public RefCountedBase
    {
    public:
    virtual ~Crawler();

    //---------------------------------------------------------------------------------------
    // Static Create
    // These methods create a crawler.
    // Either the simpler creater is called that creates a list of default downloaders 
    // the the limit of simultanous download indicated or these downloaders are
    // provided along with a queue.
    // The default downloaders are PageDownloader objects and the queue is a UrlQueue object.
    // The constructor that receives downloader and queue claim exclusive ownership 
    // to these and will delete every of those objects upon destruction.
    //
    // @bsimethod                                                 Alain.Robert   09/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT static CrawlerPtr Create(size_t maxNumberOfSimultaneousDownloads = 8);
    CRAWLERLIB_EXPORT static CrawlerPtr Create(UrlQueuePtr queue, bvector<IPageDownloader*> const& downloaders);

    //---------------------------------------------------------------------------------------
    // Starts the crawling process according to the crawler settings set using the various
    // configuration methods provided. At a minimum a crawler observer (ICrawlerObserver)
    // must be set.
    // This call blocks until the crawling is complete. The results are returned via the
    // ICrawlerObserver interface.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT StatusInt Crawl(UrlCR seed);

    //---------------------------------------------------------------------------------------
    // This method can be called from another thread than the one that has called the Crawl
    // method. No more results will be returned until the Unpause() method has been called. Though,
    // the web page downloads that where started before calling Pause() will continue until
    // completion, but the return of their content to the ICrawlerObserver is delayed.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void Pause();

    //---------------------------------------------------------------------------------------
    // This method can be called from another thread than the one that has called the Crawl
    // method. It restarts the crawling and the return of the results to the ICrawlerObserver.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void Unpause();

    //---------------------------------------------------------------------------------------
    // This method can be called from another thread than the one that has called the Crawl
    // method. It cancels the current downloads and stops the crawling.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void Stop();



    //***************************************************************************************
    //***************************************************************************************
    //***************************************************************************************
    // Crawling options
    //***************************************************************************************
    //***************************************************************************************
    //***************************************************************************************

    //---------------------------------------------------------------------------------------
    // Sets the ICrawlerObserver to be called after each web page download. Only one observer
    // can be set. Typically, the user of the crawler would implement the ICrawlerObserver
    // interface and send "this" as the argument to this method.
    // See https://en.wikipedia.org/wiki/Observer_pattern
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetObserver(ICrawlerObserver* pObserver);

    //---------------------------------------------------------------------------------------
    // Sets the maximum number of links to crawl. After this limit has been reached the crawling
    //  process will abort. This setting is part of the queue object either given or
    // created by the Create() process.
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetMaxNumberOfLinkToCrawl(size_t n);

    //---------------------------------------------------------------------------------------
    // This method sets the user agent parameter of the HTTP request used to download web pages.
    // This is different from the user agent name found in the robots.txt.
    // See https://en.wikipedia.org/wiki/List_of_HTTP_header_fields#Request_fields
    // The user agent specified will, of course, apply to every downloader objects managed
    // by the crawler.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetUserAgent(WStringCR agent);

    //---------------------------------------------------------------------------------------
    // The request timeout is the time allowed for a complete request to finish. This includes
    // the connection and download phases. By default, there is no timeout, each request
    // has an unlimited time to complete (or fail). This setting will apply to every downloader
    // managed by the crawler.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetRequestTimeoutInSeconds(long timeout);

    //---------------------------------------------------------------------------------------
    // Wether or not a download request should follow HTTP redirects (3XX HTTP codes).
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetFollowAutoRedirects(bool follow);

    //---------------------------------------------------------------------------------------
    // When SetFollowAutoRedirects is set to true, this sets how many consecutive redirects
    // the downloader is allowed to follow.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetMaxAutoRedirectCount(long count);

    //---------------------------------------------------------------------------------------
    // See http://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYPEER.html
    // False by default.
    // Set to true if SSL certificates must be validated.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void ValidateSslCertificates(bool validate);

    //---------------------------------------------------------------------------------------
    // Before donwloading a resource, a HEAD HTTP request can be sent to retrieve the
    // content type of the resource. This process is performed by default.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void ValidateContentType(bool validate);

    //---------------------------------------------------------------------------------------
    // This sets the list of accepted content types. ValidateContentType needs to be set to true
    // for this method to be effective. By default, only text/htlm and text/plain are accepted.
    // Keep in mind that the web page parser is only designed to parse <a></a> html tags. This
    // means that other content types can be downloaded and returned to the user, but the crawler
    // won't be able to find links to crawl in theses resources.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetListOfValidContentType(bvector<WString> const& contentTypes);

    //---------------------------------------------------------------------------------------
    // The seed provided to the crawler have a depth of 0, links in the seed have a depth of 1,
    // links in thoses links have a depth of 2, etc. By default, there is no depth limit.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetMaximumCrawlDepth(uint32_t depth);

    //---------------------------------------------------------------------------------------
    // An external link is a link that is in a different domain from the seed. For example,
    // if the seed is http://toto.ca, http://foo.com is an external link
    // while http://toto.ca/2.html is not.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetAcceptExternalLinks(bool accept);

    //---------------------------------------------------------------------------------------
    // SetAcceptExternalLinks need to be set to true for this method to take effect.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetAcceptLinksInExternalLinks(bool accept);



    //***************************************************************************************
    //***************************************************************************************
    //***************************************************************************************
    // Politeness options
    //***************************************************************************************
    //***************************************************************************************
    //***************************************************************************************

    //---------------------------------------------------------------------------------------
    // Whether or not to respect the robots.txt files of the websites. This method overrides
    // all of the following politeness options. When set to true,
    // every time a new domain is encountered, the crawler tries to download its robots.txt file
    // if there is one at the standard location (http://domain.com/robots.txt) and respects
    // its rules. False by default.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetRespectRobotTxt(bool respect);

    //---------------------------------------------------------------------------------------
    // Use this method to disobey the robots.txt when it disallows crawling the root, for example
    //      User-agent: *
    //      Disallow: /
    // False by default.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetRespectRobotTxtIfDisallowRoot(bool respect);

    //---------------------------------------------------------------------------------------
    // This sets the user agent of the crawler to indicate which rules to follow in the
    // robots.txt. When set, the crawler follows the rules of both the set user agent and
    // the wildcard "*". By default, the crawler obeys only the rules that apply to the
    // wildcard "*".
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetRobotsTxtUserAgent(WStringCR userAgent);

    //---------------------------------------------------------------------------------------
    // The crawler uses to the Crawl-delay fields of robots.txt. By default, there is
    // no maximum crawl delay.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetMaxRobotTxtCrawlDelay(uint32_t delayInSeconds);

    //---------------------------------------------------------------------------------------
    // Appart from the robots.txt, pages can use the parameter rel="nofollow" in the html
    // <a><\a> tags to exclude some links. Thoses links are crawled by default.
    // See https://support.google.com/webmasters/answer/96569?hl=en
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetCrawlLinksWithHtmlTagRelNoFollow(bool crawlLinks);

    //---------------------------------------------------------------------------------------
    // Appart from the robots.txt, pages can use the tag <meta name="robots" content="nofollow" />
    // in the html <head>. By default, this tag is ignored and thoses pages are crawled.
    // See https://support.google.com/webmasters/answer/96569?hl=en
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT void SetCrawlLinksFromPagesWithNoFollowMetaTag(bool crawlLinks);

    private:

    //---------------------------------------------------------------------------------------
    // Do not call this constructor directly. Use the Crawler static Create() methods instead.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    Crawler(UrlQueuePtr queue, bvector<IPageDownloader*> const& downloaders);
    Crawler() = delete;
    

#ifndef _M_CEE
    bool CanStartDownload(std::future<PageContentPtr> const& asyncDownloadThread) const;
    bool IsDownloadResultReady(std::future<PageContentPtr> const& asyncDownloadThread) const;
    void StartNextDownload(std::future<PageContentPtr>& asyncDownloadThread, IPageDownloader* downloaderToUse);
    bool AllDownloadsFinished(std::vector<std::future<PageContentPtr>> const& downloads) const;
    void DiscardRemainingDownloads(std::vector<std::future<PageContentPtr>>& downloads) const;
#endif /* _M_CEE */

    bool IsStopped() const;

    UrlQueuePtr m_pQueue;         // List of urls to explore.

    bvector<IPageDownloader*> m_pDownloaders;

    ICrawlerObserver* m_pObserver;

    const size_t m_NumberOfDownloaders;

    BeAtomic<bool>       m_StopFlag;
    bool                    m_PauseFlag;
    BeMutex              m_PauseFlagMutex;
	BeConditionVariable m_PauseFlagConditionVariable;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
