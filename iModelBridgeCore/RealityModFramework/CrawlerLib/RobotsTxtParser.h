/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/RobotsTxtParser.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/Url.h>

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/RefCounted.h>

#include <map>
#include <set>
#include <vector>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

//=======================================================================================
//! @bsiclass
// Abstract the string of an agent into a UserAgent concept.
//=======================================================================================
struct UserAgent : public RefCountedBase
    {
    public:
    CRAWLERLIB_EXPORT static UserAgentPtr Create(WString const& agentName);
    CRAWLERLIB_EXPORT UserAgent(WString const& agentName);
    CRAWLERLIB_EXPORT virtual ~UserAgent();

    CRAWLERLIB_EXPORT WString GetName() const;

    CRAWLERLIB_EXPORT bool operator==(UserAgentCR other) const;
    CRAWLERLIB_EXPORT bool operator<(UserAgentCR other) const;

    private:
    WString m_agentName;
    };

//=======================================================================================
//! @bsiclass
// This class represents the content of a robot.txt file as downloaded from a web domain.
// The robot content describes a list of rules to be followed by a non-human
// operated process when accessing a web domain.
// The robot text content provides access to basic politeness rules such as crawl delay
// allowed/disallowed urls etc.
//=======================================================================================
struct RobotsTxtContent : public RefCountedBase
    {
    public:

    //=======================================================================================
    // Create a robot text content object. The content of the robot.txt file raw is provided
    // along with the URL from which the robot.txt file was downloaded.
    //=======================================================================================
    CRAWLERLIB_EXPORT static RobotsTxtContentPtr Create(WString const& robotsTxtFile, UrlCR robotsTxtUrl);
    RobotsTxtContent() = delete;

    //=======================================================================================
    // Returns the content of the robot.txt file
    //=======================================================================================
    CRAWLERLIB_EXPORT WString const& GetRobotsTxtFile() const;
    CRAWLERLIB_EXPORT UrlCPtr GetBaseUrl() const;


    CRAWLERLIB_EXPORT bool IsUrlDisallowed(UrlCR url, UserAgentCR agent) const;

    //=======================================================================================
    // Get 
    //=======================================================================================
    CRAWLERLIB_EXPORT void GetUserAgents(std::vector<UserAgent>& agentVector) const;
    CRAWLERLIB_EXPORT void AddUserAgent(UserAgentCR agent);

    CRAWLERLIB_EXPORT void AddDisallowedUrl(UserAgentCR agent, UrlPtr url);
    CRAWLERLIB_EXPORT void AddAllowedUrl(UserAgentCR agent, UrlPtr url);

    
    CRAWLERLIB_EXPORT uint32_t GetCrawlDelay(UserAgentCR agent) const;
    CRAWLERLIB_EXPORT void SetCrawlDelay(UserAgentCR agent, uint32_t delay);

    CRAWLERLIB_EXPORT bool IsRootDisallowed(UserAgentCR agent) const;

    private:
    RobotsTxtContent(WString const& robotsTxtFile, UrlCR robotsTxtUrl);

    void GetDisallowedUrlsOf(UrlPtrSet& urlSet, UserAgentCR agent) const;
    void GetAllowedUrlsOf(UrlPtrSet& urlSet, UserAgentCR agent) const;
    void AddAllDisallowedUrlsOfAgentToSet(UrlPtrSet& urlSet, UserAgentCR agent) const;
    void AddAllAllowedUrlsOfAgentToSet(UrlPtrSet& urlSet, UserAgentCR agent) const;

    WString m_RobotTxtFile;
    UrlCPtr m_BaseUrl;
    std::map<UserAgent, UrlPtrSet> m_DisallowedUrls;
    std::map<UserAgent, UrlPtrSet> m_AllowedUrls;
    std::map<UserAgent, uint32_t> m_CrawlDelays;
    };

//=======================================================================================
//! @bsiclass
//=======================================================================================
class RobotsTxtParser
    {
    public:
    CRAWLERLIB_EXPORT RobotsTxtContentPtr ParseRobotsTxt(WString const& robotTxtFileContent, UrlCR baseUrl) const;
    CRAWLERLIB_EXPORT RobotsTxtContentPtr GetEmptyRobotTxt(UrlCR baseUrl) const;

    private:
    static const std::wregex s_UserAgentRegex;
    static const std::wregex s_DisallowRegex;
    static const std::wregex s_AllowRegex;
    static const std::wregex s_CrawlDelayRegex;
    static const std::wregex s_DisallowNothingRegex;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
