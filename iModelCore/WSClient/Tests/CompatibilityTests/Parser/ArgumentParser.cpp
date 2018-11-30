/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/Parser/ArgumentParser.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Client/WSError.h>
#include "ArgumentParser.h"
#include <Bentley/Base64Utilities.h>
#include <regex>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int ArgumentParser::Parse
(
int argc,
char** argv,
int& logLevelOut,
BeFileName& tempDirOut,
bvector<TestRepositories>& testDataOut,
std::ostream* err,
std::ostream* output
)
    {
    bvector<Utf8String> args;
    for (int i = 1; i < argc; i++)
        args.push_back(argv[i]);

    return Parse(args, logLevelOut, tempDirOut, testDataOut, err, output);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int ArgumentParser::Parse
(
const bvector<Utf8String>& args,
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

    for (Utf8StringCR arg : args)
        {
        if (arg == "--help")
            {
            PrintHelp(out);
            return 0;
            }
        }

    auto status = TryParse(args, logLevelOut, tempDirOut, testDataOut, err);
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
const bvector<Utf8String>& args,
int& logLevelOut,
BeFileName& tempDirOut,
bvector<TestRepositories>& testDataOut,
std::ostream* err
)
    {
    TestRepositories* currentRepos = nullptr;
    TestRepository* currentRepo = nullptr;
    size_t argc = args.size();
    for (size_t i = 0; i < argc; i++)
        {
        auto& arg = args[i];

        if (0 == arg.find("--gtest"))
            continue;

        if (arg == "--downloadschemas")
            {
            testDataOut.push_back(TestRepositories());
            currentRepo = &testDataOut.back().downloadSchemas;
            continue;
            }
        if (arg == "--createcache")
            {
            testDataOut.push_back(TestRepositories());
            currentRepos = &testDataOut.back();
            currentRepo = &currentRepos->create;
            continue;
            }
        if (arg == "--upgradecache")
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

        Utf8CP argValue = nullptr;

        if (arg == "--config")
            {
            currentRepo = nullptr;

            if (!GetArgValue(argc, args, i, argValue, err))
                return -1;

            auto status = ParseConfigFile(argValue, logLevelOut, tempDirOut, testDataOut, err);
            if (0 != status)
                return status;
            continue;
            }

        if (arg == "--silent")
            {
            logLevelOut = 0;
            continue;
            }

        if (arg == "--workdir")
            {
            if (!GetArgValue(argc, args, i, argValue, err))
                return -1;
            tempDirOut = BeFileName(argValue);
            continue;
            }

        if (nullptr == currentRepo)
            {
            PrintError(err, "Missig --createcache");
            return -1;
            }

        if (arg == "-url")
            {
            if (!GetArgValue(argc, args, i, argValue, err))
                return -1;
            currentRepo->schemasDir.clear();
            currentRepo->serverUrl = argValue;
            }
        else if (arg == "-r")
            {
            if (!GetArgValue(argc, args, i, argValue, err))
                return -1;
            currentRepo->schemasDir.clear();
            currentRepo->id = argValue;
            }
        else if (arg == "-sv")
            {
            if (!GetArgValue(argc, args, i, argValue, err))
                return -1;

            std::regex regex(R"(^(\d+\.\d+)$)", std::regex_constants::icase);
            std::cmatch matches;
            std::regex_search(argValue, matches, regex);
            if (matches.size() != 2)
                {
                PrintError(err, Utf8PrintfString("Invalid service version ('X.Y'): '%s'", argValue).c_str());
                return -1;
                }

            currentRepo->schemasDir.clear();
            currentRepo->serviceVersion = BeVersion(argValue);
            }
        else if (0 == arg.find("-auth:"))
            {
            if (!GetArgValue(argc, args, i, argValue, err))
                return -1;

            currentRepo->schemasDir.clear();
            currentRepo->credentials = Credentials();
            currentRepo->token = nullptr;
            currentRepo->environment = nullptr;

            if (!ParseAuth(arg.c_str(), argValue, *currentRepo, err))
                {
                PrintError(err, Utf8PrintfString("Invalid format: %s %s", arg.c_str(), argValue).c_str());
                return -1;
                }
            }
        else if (arg == "-schemas")
            {
            if (!GetArgValue(argc, args, i, argValue, err))
                return -1;
            BeFileName dir(argValue);
            if (!dir.DoesPathExist() || !dir.IsDirectory())
                {
                PrintError(err, Utf8PrintfString("Not a folder: %s %s", arg.c_str(), argValue).c_str());
                return -1;
                }

            *currentRepo = TestRepository();
            currentRepo->schemasDir = dir;
            currentRepo->schemasDir.AppendSeparator();
            }
        else if (arg == "-l")
            {
            if (!GetArgValue(argc, args, i, argValue, err))
                return -1;
            currentRepo->label = argValue;
            }
        else if (arg == "-c")
            {
            if (!GetArgValue(argc, args, i, argValue, err))
                return -1;
            currentRepo->comment = argValue;
            }
        else if (arg == "-validateCertificates")
            {
            if (!GetArgValue(argc, args, i, argValue, err))
                return -1;

            Utf8String str(argValue);
            str.ToLower().Trim();
            bool validateCertificate = false;
            if (str == "true")
                validateCertificate = true;
            if (str == "false")
                validateCertificate = false;

            currentRepo->validateCertificate = validateCertificate;
            }
        else
            {
            PrintError(err, Utf8PrintfString("Uknown parameter: %s at positition %d", arg.c_str(), i).c_str());
            return -1;
            }
        }

    for (auto& testRepo : testDataOut)
        {
        if (testRepo.upgrade.IsValid() && !testRepo.create.IsValid())
            {
            PrintError(err, "Invalid or missing parameters for --createcache");
            return -1;
            }
        if (testRepo.downloadSchemas.IsValid() && testRepo.create.IsValid() ||
            testRepo.downloadSchemas.IsValid() && testRepo.upgrade.IsValid())
            {
            PrintError(err, "Parse error");
            return -1;
            }
        if (!testRepo.downloadSchemas.IsValid() && !testRepo.create.IsValid() && !testRepo.upgrade.IsValid())
            {
            PrintError(err, "Parse error");
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
bool ArgumentParser::GetArgValue(size_t argc, const bvector<Utf8String>& args, size_t& iInOut, Utf8CP& argValueOut, std::ostream* err)
    {
    Utf8CP arg = args[iInOut].c_str();
    if (iInOut + 1 >= argc)
        {
        PrintError(err, Utf8PrintfString("Missig value for: %s", arg).c_str());
        return false;
        }

    argValueOut = args[++iInOut].c_str();
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
        repo.tokenPath = BeFileName(value);
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
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int ArgumentParser::ParseConfigFile
(
Utf8String filePath,
int& logLevelOut,
BeFileName& tempDirOut,
bvector<TestRepositories>& testDataOut,
std::ostream* err
)
    {
    BeFile file;
    bvector<Byte> content;
    if (BeFileStatus::Success != file.Open(filePath, BeFileAccess::Read) ||
        BeFileStatus::Success != file.ReadEntireFile(content) ||
        BeFileStatus::Success != file.Close())
        {
        PrintError(err, Utf8PrintfString("Could not read config file: %s", filePath.c_str()).c_str());
        return -1;
        }
    content.push_back(0);

    bvector<Utf8String> tokens;
    BeStringUtilities::Split((Utf8CP)content.data(), "\n", tokens);

    for (auto& token : tokens)
        token.Trim();

    bvector<Utf8String> args;
    for (auto& token : tokens)
        {
        token.Trim();
        if (token.empty())
            continue;
        if (token[0] == '#')
            continue;

        auto pos = token.find_first_of(' ');
        if (Utf8String::npos == pos)
            pos = token.length();

        args.push_back(token.substr(0, pos));

        Utf8String value = token.substr(pos, token.length());
        value.Trim();

        if (!value.empty())
            args.push_back(value);
        }

    return TryParse(args, logLevelOut, tempDirOut, testDataOut, err);
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
    *out << "  WSClientCompatibilityTests --createcache     <parameters>" << std::endl;
    *out << "                            [--upgradecache   [<parameters>]" << std::endl;
    *out << "                            [--createcache     <parameters> ...]" << std::endl;
    *out << "                            [--downloadschemas <parameters>]" << std::endl;
    *out << "                            [--config <filepath>]" << std::endl;
    *out << "  <parameters>: -url URL -r REPOID [-sv VERSION][-auth:XXX VALUE][-l TESTLABEL][-c FAILURECOMMENT]" << std::endl;
    *out << "  <parameters>: -schemas PATH" << std::endl;
    *out << "Usage with schemas:" << std::endl;
    *out << "  WSClientCompatibilityTests --createcache -schemas PATH --upgradecache ..." << std::endl;
    *out << "Tests:" << std::endl;
    *out << "  --createcache      Run cache creation test." << std::endl;
    *out << "  --upgradecache     Run cache upgrade test. Upgrade will use --createcache test as base. Parameters for upgrade are optional - they default to base ones." << std::endl;
    *out << "  --downloadschemas  Run schema download test. Schemas will be left in work dir and could be inspected or reused. Will run before any create or upgrade tests." << std::endl;
    *out << "  --config           File containing test setup instead of command-line. Each argument and its value should be on new line. Lines starting with # will be ingored." << std::endl;
    *out << "Parameters for server connection:" << std::endl;
    *out << "  -url URL           WSG server URL" << std::endl;
    *out << "  -r REPOID          repository ID" << std::endl;
    *out << "  -sv VERSION        service version X.Y" << std::endl;
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
    *out << "  -l TestLabel       Add descriptive name for test. Will be used in workdir folder structure as well as test output." << std::endl;
    *out << "  -c \"Failure Comment\"" << std::endl;
    *out << "                     Optional comment to be shown when test fails for known issues." << std::endl;
    *out << "  -validateCertificates [true|false]" << std::endl;
    *out << "                     Optionally disable SSL certificate validation. Default is true." << std::endl;
    *out << "Parameters when stubbing server with local schema list:" << std::endl;
    *out << "  -schemas C:\\s\\     Path to folder containing all repository schemas. Can be used to test upgrade from last known good schemas to server" << std::endl;
    *out << "Other:" << std::endl;
    *out << "  --workdir C:\\t\\    Set working directory for tests. Defaults to application folder" << std::endl;
    *out << "  --silent           Disable console logging. This still allows using --gtest_output to get full error messages to output file." << std::endl;
    *out << "  --help             Print this help text" << std::endl;
    *out << "Examples:" << std::endl;
    *out << R"(  WSClientCompatibilityTests --createcache -url https://foo.com -r Repo -auth:basic John:Jonson)" << std::endl;
    *out << std::endl;
    *out << "Working examples, run in sequence:" << std::endl;
    *out << R"(  WSClientCompatibilityTests --workdir C:\Temp\CR\ --createcache -url https://qa-wsg20-eus.cloudapp.net -r BentleyCONNECT.PersonalPublishing--CONNECT.PersonalPublishing -auth:qa:ims bentleyvilnius@gmail.com:Q!w2e3r4t5)" << std::endl;
    *out << R"(  WSClientCompatibilityTests --workdir C:\Temp\DW\ --downloadschemas -url https://qa-wsg20-eus.cloudapp.net -r BentleyCONNECT.PersonalPublishing--CONNECT.PersonalPublishing -auth:qa:ims bentleyvilnius@gmail.com:Q!w2e3r4t5)" << std::endl;
    *out << R"(  WSClientCompatibilityTests --workdir C:\Temp\UP\ --createcache -schemas C:\Temp\DW\ --upgradecache -url https://qa-wsg20-eus.cloudapp.net -r BentleyCONNECT.PersonalPublishing--CONNECT.PersonalPublishing -auth:qa:ims bentleyvilnius@gmail.com:Q!w2e3r4t5)" << std::endl;
    *out << std::endl;
    *out << std::endl;
    *out << std::endl;
    }
