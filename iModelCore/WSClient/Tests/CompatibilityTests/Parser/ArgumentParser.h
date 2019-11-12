/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include "TestData.h"
#include <Bentley/BeTest.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

struct ArgumentParser
    {
    private:
        static int TryParse
            (
            const bvector<Utf8String>& args,
            int& logLevelOut,
            BeFileName& tempDirOut,
            bvector<TestRepositories>& testDataOut,
            std::ostream* err
            );
        
        static int ParseConfigFile
            (
            Utf8String filePath,
            int& logLevelOut,
            BeFileName& tempDirOut,
            bvector<TestRepositories>& testDataOut,
            std::ostream* err
            );

        static bool GetArgValue(size_t argc, const bvector<Utf8String>&, size_t& iInOut, Utf8CP& argValueOut, std::ostream* err);
        static bool ParseAuth(Utf8CP auth, Utf8CP value, TestRepository& repo, std::ostream* err);
        static Credentials ParseCredentials(Utf8CP str, std::ostream* err);
        static SamlTokenPtr ParseTokenPath(Utf8CP path, std::ostream* err);
        static std::shared_ptr<UrlProvider::Environment> ParseEnv(Utf8StringCR str, std::ostream* err);

        static void PrintError(std::ostream* err, Utf8CP error);
        static void PrintHelp(std::ostream* output);

    private:
        static int Parse
            (
            const bvector<Utf8String>& args,
            int& logLevelOut,
            BeFileName& tempDirOut,
            bvector<TestRepositories>& testDataOut,
            std::ostream* err = nullptr,
            std::ostream* output = nullptr
            );

    public:
        static int Parse
            (
            int argc,
            char** argv,
            int& logLevelOut,
            BeFileName& tempDirOut,
            bvector<TestRepositories>& testDataOut,
            std::ostream* err = nullptr,
            std::ostream* output = nullptr
            );
    };
