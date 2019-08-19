/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServicesTestsHelper.h>
#include <WebServices/Configuration/UrlProvider.h>

struct UrlProviderTests : public WSClientBaseTest
    {
    void Reset()
        {
        AsyncTasksManager::SetDefaultScheduler(nullptr);
        UrlProvider::Uninitialize();
        }
    void SetUp() { Reset(); }
    void TearDown() { Reset(); }
    };

typedef ::testing::tuple<UrlProvider::Environment, const UrlProvider::UrlDescriptor*> TestingUrlParameter;
struct UrlProviderParamTests : public UrlProviderTests, public ::testing::WithParamInterface<TestingUrlParameter>
    {
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
                        {UrlProvider::Environment::Release, "Release"},
                        {UrlProvider::Environment::Perf, "Perf"}
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

