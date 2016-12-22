/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/Cache/TestData.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

#include <prg.h>
static Utf8CP BUILD_VERSION = REL_V "." MAJ_V "." MIN_V "." SUBMIN_V;

USING_NAMESPACE_BENTLEY_WEBSERVICES

struct TestRepository
    {
    Utf8String serverUrl;
    Utf8String id;

    Credentials credentials;
    SamlTokenPtr token;
    std::shared_ptr<UrlProvider::Environment> environment;

    bool IsValid() const
        {
        return !serverUrl.empty() && !id.empty();
        }
    };

struct TestRepositories
    {
    TestRepository create;
    TestRepository upgrade;
    };

void PrintTo(const TestRepository& value, ::std::ostream* os);
void PrintTo(const TestRepositories& value, ::std::ostream* os);