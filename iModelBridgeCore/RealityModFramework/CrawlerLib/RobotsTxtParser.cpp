/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/RobotsTxtParser.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RobotsTxtParser.h"
#include <Bentley/bvector.h>
#include <Bentley/BeStringUtilities.h>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;


const std::wregex RobotsTxtParser::s_UserAgentRegex = wregex(L"^[Uu]ser-[Aa]gent\\s*:\\s*(\\S+)\\s*#?.*");
const std::wregex RobotsTxtParser::s_DisallowRegex = wregex(L"^[Dd]isallow\\s*:\\s*(\\S+)\\s*#?.*");
const std::wregex RobotsTxtParser::s_DisallowNothingRegex = wregex(L"^[Dd]isallow\\s*:\\s*(#.*)?");
const std::wregex RobotsTxtParser::s_AllowRegex = wregex(L"^[Aa]llow\\s*:\\s*(\\S+)\\s*#?.*");
const std::wregex RobotsTxtParser::s_CrawlDelayRegex = wregex(L"^[Cc]rawl-[Dd]elay\\s*:\\s*(\\d+)\\s*#?.*");

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool UserAgent::operator==(UserAgent const& other) const
    {
    return m_AgentName.Equals(other.m_AgentName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool UserAgent::operator<(UserAgent const& other) const
    {
    return m_AgentName.CompareToI(other.m_AgentName) < 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
RobotsTxtContent::RobotsTxtContent(WString const& robotsTxtFile, UrlPtr const& robotsTxtUrl)
    : RefCountedBase(), m_RobotTxtFile(robotsTxtFile)
    {
    m_BaseUrl = new Url(L"/", robotsTxtUrl);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
WString const& RobotsTxtContent::GetRobotsTxtFile() const
    {
    return m_RobotTxtFile;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
UrlPtr const& RobotsTxtContent::GetBaseUrl() const
    {
    return m_BaseUrl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void RobotsTxtContent::GetUserAgents(vector<UserAgent>& agentVector) const
    {
    for(auto iterator : m_DisallowedUrls)
        {
        agentVector.push_back(iterator.first);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void RobotsTxtContent::AddUserAgent(UserAgent const& agent)
    {
    UrlPtrSet emptySet;
    m_DisallowedUrls.emplace(agent, emptySet);
    m_AllowedUrls.emplace(agent, emptySet);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool RobotsTxtContent::IsUrlDisallowed(UrlPtr url, UserAgent const& agent) const
    {
    UrlPtrSet allowedUrls;
    GetAllowedUrlsOf(allowedUrls, agent);
    for(auto allowed : allowedUrls)
        {
        if(url->IsSubUrlOf(*allowed))
            return false;
        }

    UrlPtrSet disallowedUrls;
    GetDisallowedUrlsOf(disallowedUrls, agent);
    for(auto disallowed : disallowedUrls)
        {
        if(url->IsSubUrlOf(*disallowed))
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void RobotsTxtContent::GetDisallowedUrlsOf(UrlPtrSet& urlSet, UserAgent const& agent) const
    {
    AddAllDisallowedUrlsOfAgentToSet(urlSet, agent);
    AddAllDisallowedUrlsOfAgentToSet(urlSet, UserAgent(L"*"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void RobotsTxtContent::AddAllDisallowedUrlsOfAgentToSet(UrlPtrSet& urlSet, UserAgent const& agent) const
    {
    auto iterator = m_DisallowedUrls.find(UserAgent(agent));
    if(iterator != m_DisallowedUrls.end())
        {
        for(auto url : iterator->second)
            urlSet.insert(url);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void RobotsTxtContent::AddDisallowedUrl(UserAgent const& agent, UrlPtr const& url)
    {
    m_DisallowedUrls.at(agent).insert(url);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void RobotsTxtContent::GetAllowedUrlsOf(UrlPtrSet& urlSet, UserAgent const& agent) const
    {
    AddAllAllowedUrlsOfAgentToSet(urlSet, agent);
    AddAllAllowedUrlsOfAgentToSet(urlSet, UserAgent(L"*"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void RobotsTxtContent::AddAllAllowedUrlsOfAgentToSet(UrlPtrSet& urlSet, UserAgent const& agent) const
    {
    auto iterator = m_AllowedUrls.find(UserAgent(agent));
    if(iterator != m_AllowedUrls.end())
        {
        for(auto url : iterator->second)
            urlSet.insert(url);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void RobotsTxtContent::AddAllowedUrl(UserAgent const& agent, UrlPtr const& url)
    {
    if(!IsUrlDisallowed(url, agent))
        m_AllowedUrls.at(agent).insert(url);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t RobotsTxtContent::GetCrawlDelay(UserAgent const& agent) const
    {
    auto iteratorWildcard = m_CrawlDelays.find(UserAgent(L"*"));
    auto iteratorAgent = m_CrawlDelays.find(agent);

    if(iteratorAgent != m_CrawlDelays.end())
        return iteratorAgent->second;
    else if(iteratorWildcard != m_CrawlDelays.end())
        return iteratorWildcard->second;
    else
        return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void RobotsTxtContent::SetCrawlDelay(UserAgent const& agent, uint32_t delay)
    {
    auto iterator = m_CrawlDelays.find(agent);

    if(iterator != m_CrawlDelays.end())
        m_CrawlDelays.erase(iterator);

    m_CrawlDelays.emplace(agent, delay);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool RobotsTxtContent::IsRootDisallowed(UserAgent const& agent) const
    {
    auto iterator = m_DisallowedUrls.find(agent);
    if(iterator == m_DisallowedUrls.end())
        return false;
    else
        {
        for(auto pUrl : iterator->second)
            {
            if(*pUrl == *m_BaseUrl)
                return true;
            }
        return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
RobotsTxtContentPtr RobotsTxtParser::ParseRobotsTxt(WString const& robotTxtFileContent, UrlPtr const& baseUrl) const
    {
    RobotsTxtContentPtr content = new RobotsTxtContent(robotTxtFileContent, baseUrl);

    bvector<WString> lines;
    WCharCP lineDelimiter = L"\n";
    BeStringUtilities::Split(robotTxtFileContent.c_str(), lineDelimiter, lines);

    wcmatch match;
    vector<UserAgent> agentsConcernedByCurrentRule;
    bool lastLineWasUserAgent = false;
    for (auto line : lines)
        {
        if(regex_match(line.c_str(), match, s_UserAgentRegex))
            {
            UserAgent agent(match[1].str().c_str());
            content->AddUserAgent(agent);

            if(!lastLineWasUserAgent)
                agentsConcernedByCurrentRule.clear();
            agentsConcernedByCurrentRule.push_back(agent);
            lastLineWasUserAgent = true;
            }
        else if(regex_match(line.c_str(), match, s_AllowRegex))
            {
            for(auto agent : agentsConcernedByCurrentRule)
                {
                try
                    {
                    UrlPtr allowedUrl = new Url(match[1].str().c_str(), baseUrl);
                    content->AddAllowedUrl(agent, allowedUrl);
                    }
                catch(InvalidUrlException&) {}
                }
            lastLineWasUserAgent = false;
            }
        else if(regex_match(line.c_str(), match, s_DisallowNothingRegex))
            {
            for(auto agent : agentsConcernedByCurrentRule)
                {
                UrlPtr allowedUrl = new Url(L"/", baseUrl);
                content->AddAllowedUrl(agent, allowedUrl);
                }
            lastLineWasUserAgent = false;
            }
        else if(regex_match(line.c_str(), match, s_DisallowRegex))
            {
            for(auto agent : agentsConcernedByCurrentRule)
                {
                try
                    {
                    UrlPtr disallowedUrl = new Url(match[1].str().c_str(), baseUrl);
                    content->AddDisallowedUrl(agent, disallowedUrl);
                    }
                catch(InvalidUrlException&) {}
                }
            lastLineWasUserAgent = false;
            }
        else if(regex_match(line.c_str(), match, s_CrawlDelayRegex))
            {
            for(auto agent : agentsConcernedByCurrentRule)
                {
                uint32_t delay = static_cast<uint32_t>(BeStringUtilities::Wtoi(match[1].str().c_str()));
                content->SetCrawlDelay(agent, delay);
                }
            lastLineWasUserAgent = false;
            }
        }

    return content;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
RobotsTxtContentPtr RobotsTxtParser::GetEmptyRobotTxt(UrlPtr const& baseUrl) const
    {
    RobotsTxtContentPtr emptyContent = new RobotsTxtContent(L"", baseUrl);
    return emptyContent;
    }
