/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/Parser/ArgumentParser.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Client/WSError.h>
#include "ArgumentParser.h"
#include <Bentley/Base64Utilities.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int ArgumentParser::Parse
(
int argc,
char** argv,
int& logLevelOut,
BeFileName& tempDirOut,
bvector<TestRepositories>& testDataOut,
std::ostream* err,
std::ostream* out
)
    {
    logLevelOut = 1;
    tempDirOut.clear();
    testDataOut.clear();

    for (int i = 1; i < argc; i++)
        {
        if (0 == strcmp(argv[i], "--help"))
            {
            PrintHelp(out);
            return 0;
            }
        }

    auto status = TryParse(argc, argv, logLevelOut, tempDirOut, testDataOut, err);
    if (status == 0)
        return 0;

    logLevelOut = 1;
    tempDirOut.clear();
    testDataOut.clear();

    return status;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int ArgumentParser::TryParse
(
int argc,
char** argv,
int& logLevelOut,
BeFileName& tempDirOut,
bvector<TestRepositories>& testDataOut,
std::ostream* err
)
    {
    TestRepositories* currentRepos = nullptr;
    TestRepository* currentRepo = nullptr;
    for (int i = 1; i < argc; i++)
        {
        auto arg = argv[i];

        if (0 == Utf8String(arg).find("--gtest"))
            continue;

        if (0 == strcmp(arg, "--createcache"))
            {
            testDataOut.push_back(TestRepositories());
            currentRepos = &testDataOut.back();
            currentRepo = &currentRepos->create;
            continue;
            }
        if (0 == strcmp(arg, "--upgradecache"))
            {
            if (currentRepos == nullptr)
                {
                PrintError(err, "Missing --createcache");
                return -1;
                }
            currentRepo = &currentRepos->upgrade;
            *currentRepo = currentRepos->create;
            continue;
            }

        if (0 == strcmp(arg, "--silent"))
            {
            logLevelOut = 0;
            continue;
            }

        Utf8CP argValue = nullptr;
        if (0 == strcmp(arg, "--workdir"))
            {
            if (!GetArgValue(argc, argv, i, argValue, err))
                return -1;
            tempDirOut = BeFileName(argValue);
            continue;
            }

        if (nullptr == currentRepo)
            {
            PrintError(err, "Missig --createcache");
            return -1;
            }

        if (0 == strcmp(arg, "-url"))
            {
            if (!GetArgValue(argc, argv, i, argValue, err))
                return -1;
            currentRepo->schemasDir.clear();
            currentRepo->serverUrl = argValue;
            }
        else if (0 == strcmp(arg, "-r"))
            {
            if (!GetArgValue(argc, argv, i, argValue, err))
                return -1;
            currentRepo->schemasDir.clear();
            currentRepo->id = argValue;
            }
        else if (0 == Utf8String(arg).find("-auth:"))
            {
            if (!GetArgValue(argc, argv, i, argValue, err))
                return -1;

            currentRepo->schemasDir.clear();
            currentRepo->credentials = Credentials();
            currentRepo->token = nullptr;
            currentRepo->environment = nullptr;

            if (!ParseAuth(arg, argValue, *currentRepo, err))
                {
                PrintError(err, Utf8PrintfString("Invalid format: %s %s", arg, argValue).c_str());
                return -1;
                }
            }
        else if (0 == strcmp(arg, "-schemas"))
            {
            if (!GetArgValue(argc, argv, i, argValue, err))
                return -1;
            BeFileName dir(argValue);
            if (!dir.DoesPathExist() || !dir.IsDirectory())
                {
                PrintError(err, Utf8PrintfString("Not a folder: %s %s", arg, argValue).c_str());
                return -1;
                }

            *currentRepo = TestRepository();
            currentRepo->schemasDir = dir;
            currentRepo->schemasDir.AppendSeparator();
            }
        else
            {
            PrintError(err, Utf8PrintfString("Uknown parameter: %s", arg).c_str());
            return -1;
            }
        }

    for (auto& testRepo : testDataOut)
        {
        if (!testRepo.create.IsValid())
            {
            PrintError(err, "Invalid or missing parameters for --createcache");
            return -1;
            }
        }

    if (testDataOut.empty())
        {
        PrintError(err, "Missing --createcache");
        return -1;
        }

    return 0;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArgumentParser::GetArgValue(int argc, char** argv, int& iInOut, Utf8CP& argValueOut, std::ostream* err)
    {
    Utf8CP arg = argv[iInOut];
    if (iInOut + 1 >= argc)
        {
        PrintError(err, Utf8PrintfString("Missig value for: %s", arg).c_str());
        return false;
        }

    argValueOut = argv[++iInOut];
    if (Utf8String::IsNullOrEmpty(argValueOut) ||
        argValueOut[0] == '-' ||
        argValueOut[0] == '.')
        {
        PrintError(err, Utf8PrintfString("Missig value for: %s", arg).c_str());
        return false;
        }

    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArgumentParser::ParseAuth(Utf8CP auth, Utf8CP value, TestRepository& repo, std::ostream* err)
    {
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(auth, ":", tokens);

    if (tokens.size() == 2 && tokens[1] == "basic")
        {
        repo.credentials = ParseCredentials(value, err);
        return repo.credentials.IsValid();
        }

    if (tokens.size() == 3 && tokens[2] == "ims")
        {
        repo.environment = ParseEnv(tokens[1], err);
        repo.credentials = ParseCredentials(value, err);
        return repo.environment && repo.credentials.IsValid();
        }

    if (tokens.size() == 3 && tokens[2] == "token")
        {
        repo.environment = ParseEnv(tokens[1], err);
        repo.token = ParseTokenPath(value, err);
        return repo.environment && repo.token;
        }

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Credentials ArgumentParser::ParseCredentials(Utf8CP str, std::ostream* err)
    {
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(str, ":", tokens);

    if (tokens.size() != 2)
        {
        PrintError(err, Utf8PrintfString("Invalid credentials format: %s", str).c_str());
        return {};
        }

    return Credentials(tokens[0], tokens[1]);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr ArgumentParser::ParseTokenPath(Utf8CP path, std::ostream* err)
    {
    BeFile file;
    bvector<Byte> contents;
    if (BeFileStatus::Success != file.Open(path, BeFileAccess::Read) ||
        BeFileStatus::Success != file.ReadEntireFile(contents))
        {
        PrintError(err, Utf8PrintfString("Failed to read token file: %s", path).c_str());
        return nullptr;
        }
    Utf8String tokenStr((char*) contents.data(), contents.size());
    auto token = std::make_shared<SamlToken>(tokenStr);
    if (!token->IsSupported())
        {
        PrintError(err, Utf8PrintfString("Not a SAML XML token: %s", path).c_str());
        return nullptr;
        }
    return token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<UrlProvider::Environment> ArgumentParser::ParseEnv(Utf8StringCR str, std::ostream* err)
    {
    if (0 == str.CompareToI("DEV"))
        return std::make_shared<UrlProvider::Environment>(UrlProvider::Environment::Dev);
    if (0 == str.CompareToI("QA"))
        return std::make_shared<UrlProvider::Environment>(UrlProvider::Environment::Qa);
    if (0 == str.CompareToI("PROD"))
        return std::make_shared<UrlProvider::Environment>(UrlProvider::Environment::Release);

    PrintError(err, Utf8PrintfString("Invalid environment: %s", str.c_str()).c_str());
    return nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ArgumentParser::PrintError(std::ostream* err, Utf8CP error)
    {
    if (error != nullptr && err != nullptr)
        *err << "Error: " << error << ". Use --help for more info" << std::endl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ArgumentParser::PrintHelp(std::ostream* out)
    {
    if (out == nullptr)
        return;

    *out << "WSClient compatibility test tool version " << BUILD_VERSION << std::endl;
    *out << std::endl;
    *out << "Usage with servers:" << std::endl;
    *out << "  WSClientCompatibilityTests --createcache   <parameters>" << std::endl;
    *out << "                            [--upgradecache [<parameters>]" << std::endl;
    *out << "                            [--createcache   <parameters> ...]" << std::endl;
    *out << "  <parameters>: -url URL -r REPOID [-auth:XXX VALUE]]" << std::endl;
    *out << "  <parameters>: -schemas PATH" << std::endl;
    *out << "Usage with schemas:" << std::endl;
    *out << "  WSClientCompatibilityTests --createcache -schemas PATH --upgradecache ..." << std::endl;
    *out << "Tests:" << std::endl;
    *out << "  --createcache      Run cache creation test" << std::endl;
    *out << "  --upgradecache     Run cache upgrade test. Upgrade will use --createcache test as base. Parameters for upgrade are optional - they default to base ones." << std::endl;
    *out << "Parameters for server connection:" << std::endl;
    *out << "  -url URL           WSG server URL" << std::endl;
    *out << "  -r REPOID          repository ID" << std::endl;
    *out << "  -auth:basic user:pass" << std::endl;
    *out << "                     Authenticate using credentials." << std::endl;
    *out << "  -auth:dev:ims  user:pass" << std::endl;
    *out << "  -auth:qa:ims   user:pass" << std::endl;
    *out << "  -auth:prod:ims user:pass" << std::endl;
    *out << "                     Authenticate using IMS credentials." << std::endl;
    *out << "  -auth:dev:token  C:\\token.xml" << std::endl;
    *out << "  -auth:qa:token   C:\\token.xml" << std::endl;
    *out << "  -auth:prod:token C:\\token.xml" << std::endl;
    *out << "                     Authenticate using XML IMS SAML token. Token is read from file" << std::endl;
    *out << "Parameters when stubbing server with local schema list:" << std::endl;
    *out << "  -schemas C:\\s\\     Path to folder containing all repository schemas. Can be used to test upgrade from last known good schemas to server" << std::endl;
    *out << "Other:" << std::endl;
    *out << "  --workdir C:\\t\\    Set working directory for tests. Defaults to application folder" << std::endl;
    *out << "  --silent           Disable console logging. This still allows using --gtest_output to get full error messages to output file." << std::endl;
    *out << "  --help             Print this help text" << std::endl;
    *out << "Examples:" << std::endl;
    *out << R"(  WSClientCompatibilityTests --workdir C:\Tests\ --createcache -url https://foo.com -r Repo -auth:basic John:Jonson)" << std::endl;
    *out << std::endl;
    *out << R"(  WSClientCompatibilityTests --workdir C:\LastKnownGoodSchemas\ --createcache -url https://qa-wsg20-eus.cloudapp.net -r BentleyCONNECT.PersonalPublishing--CONNECT.PersonalPublishing -auth:qa:ims bentleyvilnius@gmail.com:Q!w2e3r4t5)" << std::endl;
    *out << R"(  WSClientCompatibilityTests --createcache -schemas C:\LastKnownGoodSchemas\ --upgradecache -url https://qa-wsg20-eus.cloudapp.net -r BentleyCONNECT.PersonalPublishing--CONNECT.PersonalPublishing -auth:qa:ims bentleyvilnius@gmail.com:Q!w2e3r4t5)" << std::endl;
    *out << std::endl;
    *out << std::endl;
    *out << std::endl;
    }
