/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/Downloader.h $
| 
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <CrawlerLib/Url.h>
#include <curl/curl.h>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

class IPageDownloader
    {
    public:
    CRAWLERLIB_EXPORT virtual ~IPageDownloader() { }
    virtual StatusInt DownloadPage(WString& buffer, UrlPtr const& p_Url) = 0;
    };

class SingleThreadedDownloader : public IPageDownloader
    {
    public:
    CRAWLERLIB_EXPORT SingleThreadedDownloader();
    CRAWLERLIB_EXPORT virtual ~SingleThreadedDownloader();

    CRAWLERLIB_EXPORT virtual StatusInt DownloadPage(WString& buffer, UrlPtr const& p_Url) override;
    
    private:
    static size_t CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

    CURL* m_CurlHandle;
    };


END_BENTLEY_CRAWLERLIB_NAMESPACE
