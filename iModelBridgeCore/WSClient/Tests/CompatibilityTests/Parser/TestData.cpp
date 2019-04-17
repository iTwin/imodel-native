/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "TestData.h"

void PrintTo(const TestRepository& value, ::std::ostream* os)
    {
    if (!value.label.empty())
        *os << "-l " + value.label + " ";

    if (value.schemasDir.empty())
        *os << Utf8PrintfString("-url %s -r %s ", value.serverUrl.c_str(), value.id.c_str());
    else
        *os << Utf8PrintfString("-schemas %s ", Utf8String(value.schemasDir).c_str());

    if (!value.serviceVersion.IsEmpty())
        *os << "-sv " << value.serviceVersion.ToMajorMinorString() << " ";

    if (!value.tokenPath.empty())
        {
        if (!value.environment)
            *os << "ERROR";
        else if (*value.environment == UrlProvider::Environment::Dev)
            *os << "-auth:dev";
        else if (*value.environment == UrlProvider::Environment::Qa)
            *os << "-auth:qa";
        else if (*value.environment == UrlProvider::Environment::Release)
            *os << "-auth:prod";

        *os << ":token " << Utf8String(value.tokenPath) << " ";
        }

    if (!value.credentials.IsEmpty())
        {
        if (!value.environment)
            *os << "-auth:basic ";
        else if (*value.environment == UrlProvider::Environment::Dev)
            *os << "-auth:dev:ims ";
        else if (*value.environment == UrlProvider::Environment::Qa)
            *os << "-auth:qa:ims ";
        else if (*value.environment == UrlProvider::Environment::Release)
            *os << "-auth:prod:ims ";

        *os << value.credentials.GetUsername() << ":" << value.credentials.GetPassword() << " ";
        }

    if (!value.validateCertificate)
        *os << "-validateCertificate false ";
    }

void PrintTo(const TestRepositories& value, ::std::ostream* os)
    {
    if (value.downloadSchemas.IsValid())
        {
        *os << "--downloadSchemas ";
        PrintTo(value.downloadSchemas, os);
        return;
        }

    *os << "--createcache ";
    PrintTo(value.create, os);

    if (!value.upgrade.IsValid())
        return;

    *os << "--upgradecache ";
    PrintTo(value.upgrade, os);
    }
