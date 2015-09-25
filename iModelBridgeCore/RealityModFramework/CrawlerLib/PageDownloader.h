/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/PageDownloader.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/Url.h>

#include "PageParser.h"
#include "DownloadJob.h"
#include "CrawlDelaySleeper.h"

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/bvector.h>

#include <curl/curl.h>
#include <regex>
#include <atomic>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

//=======================================================================================
//! @bsiclass
// Interface for a page downloader. The page downloader downloads a web page as described
// by a download job. The page downloader contains various setting that control its operation
// such as timeout delay, SSL validation and so on.
//=======================================================================================
class IPageDownloader
    {
    public:
    CRAWLERLIB_EXPORT virtual ~IPageDownloader() { }
    virtual PageContentPtr DownloadPage(DownloadJobPtr const& p_DownloadJob) = 0;

    virtual void SetUserAgent(WString const& agent) = 0;
    virtual void SetRequestTimeoutInSeconds(long timeout) = 0;
    virtual void SetFollowAutoRedirects(bool follow) = 0;
    virtual void SetMaxAutoRedirectCount(long count) = 0;
    virtual void ValidateSslCertificates(bool validate) = 0;
    virtual void ValidateContentType(bool validate) = 0 ;
    virtual void SetListOfValidContentType(bvector<WString> const& types) = 0;
    virtual void SetParseLinksRelNoFollow(bool parse) = 0;
    virtual void SetParsePagesWithNoFollowMetaTag(bool parse) = 0;

    //=======================================================================================
    // The AbortDownload can be called by another thread to Abort a download process.
    // Implementation of the behavior is optional if the implementation used does not permit it.
    //=======================================================================================
    virtual void AbortDownload() = 0;
    };

//=======================================================================================
//! @bsiclass
// Concrete basic implementation of an IPageDownloader. This page downloader based
// on CURL download the page described in the download job provided at DownloadPage().
// Various settings are taken into account. Refer to IPageDownloader for details.
//=======================================================================================
class PageDownloader : public IPageDownloader
    {
    public:
    CRAWLERLIB_EXPORT PageDownloader(CrawlDelaySleeperPtr sleeper);
    CRAWLERLIB_EXPORT virtual ~PageDownloader();

    CRAWLERLIB_EXPORT PageContentPtr DownloadPage(DownloadJobPtr const& p_DownloadJob) override;

    CRAWLERLIB_EXPORT void SetUserAgent(WString const& agent) override;
    CRAWLERLIB_EXPORT void SetRequestTimeoutInSeconds(long timeout) override;
    CRAWLERLIB_EXPORT void SetFollowAutoRedirects(bool follow) override;
    CRAWLERLIB_EXPORT void SetMaxAutoRedirectCount(long count) override;
    CRAWLERLIB_EXPORT void ValidateSslCertificates(bool validate) override;
    CRAWLERLIB_EXPORT void ValidateContentType(bool validate) override;
    CRAWLERLIB_EXPORT void SetListOfValidContentType(bvector<WString> const& types) override;
    CRAWLERLIB_EXPORT void SetParseLinksRelNoFollow(bool parse) override;
    CRAWLERLIB_EXPORT void SetParsePagesWithNoFollowMetaTag(bool parse) override;

    void AbortDownload() override;

    private:
    bool IsValidContentType();
    void SetDefaultSettings();
    void SetDataWritingSettings(WString* buffer, void* writeFunction);
    void SetResourceUrl(WString const& url);
    void DownloadContentType(WString& outputContentType);
    void WaitToRespectCrawlDelay(DownloadJobPtr p_DownloadJob);

    static size_t CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    static size_t CurlDiscardDataCallback(void *contents, size_t size, size_t nmemb, void *userp);
    static int CurlProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);


    CURL* m_CurlHandle;
    bool m_ValidateContentType;
    bvector<std::wregex> m_ListOfValidContentTypes;

    PageParser m_Parser;
    CrawlDelaySleeperPtr m_pSleeper;

    std::atomic<bool> m_AbortDownload;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
