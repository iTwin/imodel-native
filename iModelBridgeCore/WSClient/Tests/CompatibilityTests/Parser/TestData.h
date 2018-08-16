/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/Parser/TestData.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Client/WSError.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

#include <prg.h>
static Utf8CP BUILD_VERSION = REL_V "." MAJ_V "." MIN_V "." SUBMIN_V;

USING_NAMESPACE_BENTLEY_WEBSERVICES

struct TestRepository
    {
    Utf8String label;
    Utf8String comment;

    BeFileName schemasDir;

    Utf8String serverUrl;
    Utf8String id;
    BeVersion serviceVersion;

    Credentials credentials;
    SamlTokenPtr token;
    BeFileName tokenPath;

    std::shared_ptr<UrlProvider::Environment> environment;
    bool validateCertificate = true;

    bool IsValid() const
        {
        return 
            !serverUrl.empty() && !id.empty() && schemasDir.empty() ||
            serverUrl.empty() && id.empty() && !schemasDir.empty();
        }
    };

struct TestRepositories
    {
    TestRepository create;
    TestRepository upgrade;
    TestRepository downloadSchemas;
    };

void PrintTo(const TestRepository& value, ::std::ostream* os);
void PrintTo(const TestRepositories& value, ::std::ostream* os);
