/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/RobotsTxtDownloader.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/Url.h>
#include "RobotsTxtParser.h"

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/bvector.h>
#include <curl/curl.h>

#include <regex>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

//=======================================================================================
//! @bsiclass
//=======================================================================================
class IRobotsTxtDownloader
    {
    public:
    CRAWLERLIB_EXPORT virtual ~IRobotsTxtDownloader() { }
    virtual RobotsTxtContentPtr DownloadRobotsTxt(UrlPtr const& pi_Url) = 0;
    };

//=======================================================================================
//! @bsiclass
//=======================================================================================
class RobotsTxtDownloader : public IRobotsTxtDownloader
    {
    public:
    CRAWLERLIB_EXPORT RobotsTxtDownloader();
    CRAWLERLIB_EXPORT virtual ~RobotsTxtDownloader();

    CRAWLERLIB_EXPORT RobotsTxtContentPtr DownloadRobotsTxt(UrlPtr const& pi_Url) override;

    private:
    void SetDataWritingSettings(WString* buffer, void* writeFunction);
    void SetResourceUrl(WString const& url);

    static size_t CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

    CURL* m_CurlHandle;
    RobotsTxtParser m_Parser;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
