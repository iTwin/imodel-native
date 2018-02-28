/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Configuration/UrlProviderTests.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServicesTestsHelper.h>
#include <WebServices/Configuration/UrlProvider.h>

typedef ::testing::tuple<UrlProvider::Environment, UrlProvider::UrlDescriptor*> testingUrlParameter;
class UrlProviderTests : public WSClientBaseTest, public ::testing::WithParamInterface< testingUrlParameter>
    {
    public:
        struct PrintToStringParamName
            {
            template <class ParamType>
            std::string operator()(const ::testing::TestParamInfo<ParamType>& info) const
                {
                std::string name;
                UrlProvider::Environment enviroment = ::testing::get<0>(info.param);
                std::map<UrlProvider::Environment, Utf8String> enviroments
                    {
                    {UrlProvider::Environment::Dev, "Dev"},
                    {UrlProvider::Environment::Qa, "Qa"},
                    {UrlProvider::Environment::Release, "Release"}
                    };

                auto urlDescriptor = ::testing::get<1>(info.param);

                name.append(enviroments[enviroment].c_str());
                name.append("_");
                name.append(urlDescriptor->GetName().c_str());
                name.erase(std::remove(name.begin(), name.end(), '.'), name.end());
                return name;
                }
            };
    };
