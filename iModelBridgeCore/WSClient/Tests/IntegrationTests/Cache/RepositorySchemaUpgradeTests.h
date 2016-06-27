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
    RepositoryDef(const UrlProvider::UrlDescriptor& url, Utf8CP id) : url(&url), id(id){}
    const UrlProvider::UrlDescriptor* url;
    Utf8CP id;
    };

void PrintTo(const RepositoryDef& value, ::std::ostream* os)
    {
    *os << value.id;
    }

struct RepositorySchemaUpgradeTests : public ::testing::TestWithParam<RepositoryDef>
    {
    static void SetUpTestCase();
    void SetUp();
    };
