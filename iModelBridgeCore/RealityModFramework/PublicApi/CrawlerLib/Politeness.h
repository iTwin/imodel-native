//[>--------------------------------------------------------------------------------------+
//|
//|     $Source: PublicApi/CrawlerLib/Politeness.h $
//|
//|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//|
//+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/Url.h>
#include <CrawlerLib/RobotsTxtParser.h>
#include <CrawlerLib/RobotsTxtDownloader.h>

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>

#include <map>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE
class IPoliteness
    {
    public:
    CRAWLERLIB_EXPORT virtual ~IPoliteness() {}

    virtual void SetUserAgent(UserAgent const& agent) = 0;
    virtual void SetMaxCrawlDelay(uint32_t delay) = 0;
    virtual void SetRespectRobotTxt(bool respect) = 0;
    virtual void SetRespectRobotTxtIfDisallowRoot(bool respect) = 0;
    virtual uint32_t GetCrawlDelay(UrlPtr const& url) = 0;

    virtual bool CanDownloadUrl(UrlPtr const& url) = 0;
    };

class Politeness : public IPoliteness
    {
    public:
    CRAWLERLIB_EXPORT Politeness(IRobotsTxtDownloader* downloader);
    CRAWLERLIB_EXPORT virtual ~Politeness();

    CRAWLERLIB_EXPORT void SetUserAgent(UserAgent const& agent) override {m_UserAgent = agent;}
    CRAWLERLIB_EXPORT void SetMaxCrawlDelay(uint32_t delay) override {m_MaxCrawlDelay = delay;}
    CRAWLERLIB_EXPORT void SetRespectRobotTxt(bool respect) override {m_RespectRobotsTxt = respect;}
    CRAWLERLIB_EXPORT void SetRespectRobotTxtIfDisallowRoot(bool respect) override {m_RespectRobotsTxtIfDisallowRoot = respect;}


    CRAWLERLIB_EXPORT uint32_t GetCrawlDelay(UrlPtr const& url) override;
    CRAWLERLIB_EXPORT bool CanDownloadUrl(UrlPtr const& url) override;

    private:
    void DownloadRobotsTxt(UrlPtr const& url);
    bool IsContentDisallowUrl(RobotsTxtContentPtr const& content, UrlPtr const& url) const;

    IRobotsTxtDownloader* m_pDownloader;

    bool m_RespectRobotsTxt;
    bool m_RespectRobotsTxtIfDisallowRoot;
    uint32_t m_MaxCrawlDelay;
    UserAgent m_UserAgent;

    std::map<DomainName, RobotsTxtContentPtr> m_RobotsTxtFilesPerDomain;
    };
END_BENTLEY_CRAWLERLIB_NAMESPACE
