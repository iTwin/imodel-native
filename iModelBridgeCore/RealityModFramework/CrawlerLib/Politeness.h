/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/Politeness.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/Url.h>

#include "RobotsTxtParser.h"
#include "RobotsTxtDownloader.h"

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>

#include <map>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

//=======================================================================================
//! @bsiclass
// Interface that defines the basic settings of crawl and web politeness.
//
//=======================================================================================
struct IPoliteness
    {
    public:
    CRAWLERLIB_EXPORT virtual ~IPoliteness();

    virtual void SetUserAgent(UserAgentCR agent) = 0;
    virtual void SetMaxCrawlDelay(uint32_t delayInSeconds) = 0;
    virtual void SetRespectRobotTxt(bool respect) = 0;
    virtual void SetRespectRobotTxtIfDisallowRoot(bool respect) = 0;
 
    //=======================================================================================
    // Returns the politeness crawl delay in seconds.
    //=======================================================================================
    virtual uint32_t GetCrawlDelay(UrlCR url) = 0;

    virtual bool CanDownloadUrl(UrlCR url) = 0;
    };

//=======================================================================================
//! @bsiclass
// Basic politeness class that implements the IPoliteness interface.
//=======================================================================================
struct Politeness : public IPoliteness
    {
    public:
    //=======================================================================================
    // The constructor takes ownership of the provided IRobotsTxtDownloader provided.
    // It will be responsible of destroying it. A valid downloader MUST be provided.
    //=======================================================================================
    CRAWLERLIB_EXPORT Politeness(IRobotsTxtDownloader* downloader);
    CRAWLERLIB_EXPORT virtual ~Politeness();

    void SetUserAgent(UserAgentCR agent) override {m_UserAgent = &agent;}
    void SetMaxCrawlDelay(uint32_t delayInSeconds) override {m_MaxCrawlDelayInSeconds = delayInSeconds;}
    void SetRespectRobotTxt(bool respect) override {m_RespectRobotsTxt = respect;}
    void SetRespectRobotTxtIfDisallowRoot(bool respect) override {m_RespectRobotsTxtIfDisallowRoot = respect;}


    CRAWLERLIB_EXPORT uint32_t GetCrawlDelay(UrlCR url) override;
    CRAWLERLIB_EXPORT bool CanDownloadUrl(UrlCR url) override;

    private:
    void DownloadRobotsTxt(UrlCR url);
    bool IsContentDisallowUrl(RobotsTxtContentCR content, UrlCR url) const;

    IRobotsTxtDownloader* m_pDownloader;

    bool m_RespectRobotsTxt;
    bool m_RespectRobotsTxtIfDisallowRoot;
    uint32_t m_MaxCrawlDelayInSeconds;
    UserAgentCPtr m_UserAgent;

    std::map<DomainName, RobotsTxtContentCPtr> m_RobotsTxtFilesPerDomain;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
