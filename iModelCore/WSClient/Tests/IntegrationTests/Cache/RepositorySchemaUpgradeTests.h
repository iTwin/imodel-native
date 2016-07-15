/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Cache/RepositorySchemaUpgradeTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServicesTestsHelper.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

struct RepositoryDef
    {
    UrlProvider::Environment env;
    const UrlProvider::UrlDescriptor* url;
    Utf8CP id;

    RepositoryDef(UrlProvider::Environment env, const UrlProvider::UrlDescriptor& url, Utf8CP id) :
        env(env), url(&url), id(id)
        {}
    };

struct RepositorySchemaUpgradeTests : public ::testing::TestWithParam<RepositoryDef>
    {
    static void SetUpTestCase();
    void SetUp();

    static Utf8String GetEnvStr(UrlProvider::Environment env);
    static Utf8String GetDateStr(DateTimeCR dateTime);

    BeFileName GetOutputPath();
    BeFileName GetOutputPath(Utf8StringCR testName, Utf8StringCR repositoryId, Utf8StringCR dateStr);
    BeFileName GetNewOutputPath(Utf8StringCR testName, Utf8StringCR repositoryId);
    BeFileName GetPreviousOutputPath(Utf8StringCR testName, Utf8StringCR repositoryId);
    };

void PrintTo(const RepositoryDef& value, ::std::ostream* os)
    {
    *os << Utf8PrintfString("%s (%s)", value.id, RepositorySchemaUpgradeTests::GetEnvStr(value.env).c_str());
    }
