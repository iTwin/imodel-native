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
        env(env), url(&url), id(id) {}
    };

void PrintTo(const RepositoryDef& value, ::std::ostream* os)
    {
    *os << value.id;
    }

struct RepositorySchemaUpgradeTests : public ::testing::TestWithParam<RepositoryDef>
    {
    static void SetUpTestCase();
    void SetUp();

    BeFileName GetOutputPath();
    BeFileName GetOutputPath(Utf8StringCR testName, Utf8StringCR repositoryId, Utf8StringCR dateStr);
    BeFileName GetNewOutputPath(Utf8StringCR testName, Utf8StringCR repositoryId);
    BeFileName GetPreviousOutputPath(Utf8StringCR testName, Utf8StringCR repositoryId);
    };
