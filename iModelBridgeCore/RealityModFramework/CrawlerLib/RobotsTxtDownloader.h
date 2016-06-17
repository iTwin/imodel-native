/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/RobotsTxtDownloader.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
// This interface class defines the methods required for a Robot text file downloader.
// Robot.txt files are files that can be located at the root of a web domain that
// indicates the rules to be followed when this web site is accessed by a robot
// process instead of by human operator.
//=======================================================================================
struct IRobotsTxtDownloader
    {
    public:
    CRAWLERLIB_EXPORT virtual ~IRobotsTxtDownloader();

    //=======================================================================================
    // This method extracts the robot.txt file from the web domain indicated by the URL
    // provided.
    //=======================================================================================
    virtual RobotsTxtContentPtr DownloadRobotsTxt(UrlCR pi_Url) = 0;
    };

//=======================================================================================
//! @bsiclass
// Concrete Robot text downloader that implements the IRobotsTxtDownloader interface.
// This implementation makes use of CURL to download the file.
//=======================================================================================
struct RobotsTxtDownloader : public IRobotsTxtDownloader
    {
    public:
    CRAWLERLIB_EXPORT RobotsTxtDownloader();
    CRAWLERLIB_EXPORT virtual ~RobotsTxtDownloader();

    //=======================================================================================
    // Downloads the content of the robot.txt file.
    // The url provided must only contain the domain part eg. http:\\www.bentley.com
    // The url provided may not be null.
    //=======================================================================================
    CRAWLERLIB_EXPORT RobotsTxtContentPtr DownloadRobotsTxt(UrlCR pi_Url) override;

    private:
    void SetDataWritingSettings(WString* buffer, void* writeFunction);
    void SetResourceUrl(WString const& url);

    static size_t CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

    CURL* m_CurlHandle;
    RobotsTxtParser m_Parser;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
