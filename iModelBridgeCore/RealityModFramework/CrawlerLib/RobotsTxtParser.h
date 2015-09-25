/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/RobotsTxtParser.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    CRAWLERLIB_EXPORT UserAgent(WString const& agentName) : m_AgentName(agentName) {}
    CRAWLERLIB_EXPORT virtual ~UserAgent() {}

    CRAWLERLIB_EXPORT inline bool operator==(UserAgent const& other) const;
    CRAWLERLIB_EXPORT inline bool operator<(UserAgent const& other) const;

    private:
    WString m_AgentName;
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
    CRAWLERLIB_EXPORT RobotsTxtContent(WString const& robotsTxtFile, UrlPtr const& robotsTxtUrl);
    RobotsTxtContent() = delete;

    //=======================================================================================
    // Returns the content of the robot.txt file
    //=======================================================================================
    CRAWLERLIB_EXPORT WString const& GetRobotsTxtFile() const;
    CRAWLERLIB_EXPORT UrlPtr const& GetBaseUrl() const;


    CRAWLERLIB_EXPORT bool IsUrlDisallowed(UrlPtr url, UserAgent const& agent) const;

    //=======================================================================================
    // Get 
    //=======================================================================================
    CRAWLERLIB_EXPORT void GetUserAgents(std::vector<UserAgent>& agentVector) const;
    CRAWLERLIB_EXPORT void AddUserAgent(UserAgent const& agent);

    CRAWLERLIB_EXPORT void AddDisallowedUrl(UserAgent const& agent, UrlPtr const& url);
    CRAWLERLIB_EXPORT void AddAllowedUrl(UserAgent const& agent, UrlPtr const& url);

    
    CRAWLERLIB_EXPORT uint32_t GetCrawlDelay(UserAgent const& agent) const;
    CRAWLERLIB_EXPORT void SetCrawlDelay(UserAgent const& agent, uint32_t delay);

    CRAWLERLIB_EXPORT bool IsRootDisallowed(UserAgent const& agent) const;

    private:
    void GetDisallowedUrlsOf(UrlPtrSet& urlSet, UserAgent const& agent) const;
    void GetAllowedUrlsOf(UrlPtrSet& urlSet, UserAgent const& agent) const;
    void AddAllDisallowedUrlsOfAgentToSet(UrlPtrSet& urlSet, UserAgent const& agent) const;
    void AddAllAllowedUrlsOfAgentToSet(UrlPtrSet& urlSet, UserAgent const& agent) const;

    WString m_RobotTxtFile;
    UrlPtr m_BaseUrl;
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
    CRAWLERLIB_EXPORT RobotsTxtContentPtr ParseRobotsTxt(WString const& robotTxtFileContent, UrlPtr const& baseUrl) const;
    CRAWLERLIB_EXPORT RobotsTxtContentPtr GetEmptyRobotTxt(UrlPtr const& baseUrl) const;

    private:
    static const std::wregex s_UserAgentRegex;
    static const std::wregex s_DisallowRegex;
    static const std::wregex s_AllowRegex;
    static const std::wregex s_CrawlDelayRegex;
    static const std::wregex s_DisallowNothingRegex;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
