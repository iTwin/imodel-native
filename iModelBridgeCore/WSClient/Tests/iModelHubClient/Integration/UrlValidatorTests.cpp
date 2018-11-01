/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/UrlValidatorTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "IntegrationTestsBase.h"
#include <Bentley/BeTest.h>
#include <Bentley/BeSystemInfo.h>
#include <regex>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

Utf8String whitelistViolationDetails = "If this is caused by a necessary API change, update the whitelist and notify iModelBank of the updates. "
                                        "If the whitelist violation is unintentional, modify your changes to use existing API functionality.";

class UrlValidatorTests : public WSClientBaseTest
    {};

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Daniel.Bednarczyk                08/27
+---------------+---------------+---------------+---------------+---------------+------*/
bool lineIsValid(Utf8String line)
    { 
    // 1) Ensure every line has a scope
    std::regex scopeRegex(R"(Scope)", std::regex::ECMAScript);
    std::cmatch scopeMatches;
    std::regex_search(line.c_str(), scopeMatches, scopeRegex);

    if (scopeMatches.empty())
        return false;

    // 2) Ensure there is exactly one logging category per line
    std::regex catRegex(R"(TRACE|DEBUG|INFO|WARNING|ERROR|FATAL)", std::regex::ECMAScript);
    std::cmatch catMatches;

    int catCount = 0;
    while (regex_search(line.c_str(), catMatches, catRegex))
        {
        if (!catMatches.empty())
            {
            catCount++;
            line = catMatches.suffix().str().c_str();
            }
        }

    if (catCount != 1)
        return false;

    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Daniel.Bednarczyk                08/27
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String extractUrl(Utf8String url)
    {
    // Simultaneous threads writing to the log file can corrupt some lines, so check for malformatted data
    if (!lineIsValid(url))
        return Utf8String("");

    // Match imodelhubapi URLs (dev and qa)
    std::regex regex(R"(> HTTP #(\d+) (\w+) https://(dev|qa)-imodelhubapi.bentley.com/s?v(\d+).(\d+)/Repositories/(iModel|Project)--\w{8}-\w{4}-\w{4}-\w{4}-\w{12}/)", std::regex::ECMAScript);
    std::cmatch matches;
    std::regex_search(url.c_str(), matches, regex);

    // Return portion of URL beyond repository GUID
    if (matches.empty())
        return Utf8String("");
    else
        return url.substr(matches.position() + matches[0].length(), Utf8String::npos);

    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Daniel.Bednarczyk                08/27
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String genericizeUrl(Utf8String url)
    {

    if (url.empty())
        return url;

    // Expressions to replace
    bvector<Utf8String> expressions = {
        R"_(\w{8}-\w{4}-\w{4}-\w{4}-\w{12})_",          // All GUIDs
        R"_(Briefcase/(\d+))_",                         // Briefcase Id path (any numeric value)
        R"_(BriefcaseId\+eq\+(\d+))_",                  // Briefcase Id equality query string param (any numeric value)
        R"_(BriefcaseId\+ne\+(\d+))_",                  // Briefcase Id nonequality query string param (any numeric value)
        R"_(Name\+eq\+'(.*)')_",                        // Name query string param (any characters, any length)
        R"_(Id\+eq\+'\w{40}')_",                        // Id query string param (any 40-character alphanumeric string)
        R"_(LockLevel\+gt\+(\d+))_",                    // LockLevel query string param (any numeric value)
        R"_(ReleasedWithChangeSetIndex\+gt\+(\d+))_",   // ReleasedWithChangeSetIndex query string param (any numeric value)
        R"_(InitializationState\+gt\+(\d+))_",          // InitializationState query string param (any numeric value)
        R"_(Index\+le\+(\d+))_",                        // Index query string param (any numeric value)
        R"_(top=(\d+))_",                               // Any 'top' query string param (any numeric value)
        R"_(id\+in\+%5B'(.+)'%5D)_"                     // Any 'id in' query query param within URL-encoded brackets (any characters between first and last quote)
        };

    // Replacement strings (corresponding to expressions vector)
    bvector<Utf8String> replacements = {
        R"_(********-****-****-****-************)_",
        R"_(Briefcase/*)_",
        R"_(BriefcaseId+eq+*)_",
        R"_(BriefcaseId+ne+*)_",
        R"_(Name+eq+'*')_",
        R"_(Id+eq+'*')_",
        R"_(LockLevel+gt+*)_",
        R"_(ReleasedWithChangeSetIndex+gt+*)_",
        R"_(InitializationState+gt+*)_",
        R"_(Index+le+*)_",
        R"_(top=*)_",
        R"_(id+in+['*'])_",
        };

    // Check string for all regular expressions and replace matches with generic substring
    for (int i = 0; i < expressions.size(); i++)
        {
        std::regex regex(expressions[i].c_str(), std::regex::ECMAScript);
        url = std::regex_replace(url.c_str(), regex, replacements[i].c_str()).c_str();
        }

    return url;
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Daniel.Bednarczyk                08/27
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String parseUrlcallback(Utf8String url)
    {
    url = extractUrl(url);
    url = genericizeUrl(url);
    return url;
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Daniel.Bednarczyk                08/27
+---------------+---------------+---------------+---------------+---------------+------*/
bset<Utf8String> loadUrlFile(Utf8String path, Utf8String(*callback)(Utf8String))
    {
    bset<Utf8String> urls;

    // Read file contents
    BeFile file;
    if (BeFileStatus::Success != file.Open(path, BeFileAccess::Read))
        return urls;

    ByteStream byteStream;
    if (BeFileStatus::Success != file.ReadEntireFile(byteStream))
        return urls;

    Utf8String contents((Utf8CP) byteStream.GetData(), byteStream.GetSize());
    file.Close();

    // Split file contents into lines
    bvector<Utf8String> lines;
    BeStringUtilities::Split(contents.c_str(), "\n", lines);

    // Populate set
    for (int i = 0; i < lines.size(); i++)
        {
        Utf8String line = lines[i];

        if (callback != nullptr)
            line = callback(line);

        if (!line.empty())
            {
            urls.insert(line);
            }
        }

    return urls;

    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Daniel.Bednarczyk                08/27
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(UrlValidator, WhitelistCheck)
    {
    // Configuration
    iModelHubHost& host = iModelHubHost::Instance();
    BeFileName assetsPath = host.GetDgnPlatformAssetsDirectory();
    Utf8String whitelistPath = assetsPath.GetNameUtf8().append("whitelist.txt");
    Utf8String localAppDataPath = getenv("LOCALAPPDATA");
    Utf8String logPath = localAppDataPath.append("\\Bentley\\Logs\\iModelHubIntgerationTests.log");

    // Load whitelist
    bset<Utf8String> whitelistURLs = loadUrlFile(whitelistPath, nullptr);
    ASSERT_GT(whitelistURLs.size(), 0);

    // Load log
    Utf8String(*callback)(Utf8String) = &parseUrlcallback;
    bset<Utf8String> logURLs = loadUrlFile(logPath, callback);
    ASSERT_GT(logURLs.size(), 0);
    
    // Assert that all log URLs are whitelisted
    bset<Utf8String>::iterator url;
    bset<Utf8String>::iterator findResult;
    url = logURLs.begin();
    for (url = logURLs.begin(); url != logURLs.end(); ++url)
        {
        findResult = whitelistURLs.find(url.key());

        bool urlIsWhitelisted = true;
        if (findResult == whitelistURLs.end())
            urlIsWhitelisted = false;

        ASSERT_TRUE(urlIsWhitelisted) << "The URL \"" << url.key() << "\" is not whitelisted (see whitelist.txt)\n" << whitelistViolationDetails;
        }
    }

