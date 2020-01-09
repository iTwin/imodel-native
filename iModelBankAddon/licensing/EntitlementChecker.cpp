/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Logging/bentleylogging.h>
#include <Licensing/AccessKeyClient.h>
#include <Bentley/Desktop/FileSystem.h>
#include <chrono>
#include "EntitlementChecker.h"
#include "../common/EntitlementCheckerUtils.h"
#include "../common/ConversionUtils.h"
#include "../common/macros.h"

using namespace IModelBank;
using namespace std::chrono;

Licensing::AccessKeyClientPtr s_client;

Licensing::LicenseStatus performEntitlementCheck()
{
    static auto s_lastCheck = system_clock::now() - hours(2);
    static auto s_lastStatus = Licensing::LicenseStatus::Ok;

    if (system_clock::now() - hours(1) < s_lastCheck && s_lastStatus == Licensing::LicenseStatus::Ok)
        return s_lastStatus;
    s_lastStatus = s_client->StartApplication();
    if (s_lastStatus == Licensing::LicenseStatus::Ok)
        s_lastCheck = system_clock::now();
    return s_lastStatus;
}

static Napi::Value checkEntitlement(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENT_STRING(0, iModelId);
    REQUIRE_ARGUMENT_STRING(1, activityId);
    REQUIRE_ARGUMENT_STRING(2, dateTime);

    ConversionUtils::LogMessage("imodel-bank-licensing", NativeLogging::SEVERITY::LOG_INFO, Utf8PrintfString("checkEntitlement for %s", iModelId.c_str()).c_str());
    
    Licensing::LicenseStatus status = performEntitlementCheck();
    if (status != Licensing::LicenseStatus::Ok)
    {
        return Napi::String::New(info.Env(), Utf8PrintfString("Licensing failed with status: %d.", (int)status).c_str());
    }

    Utf8String hash = calculateHash(activityId, iModelId, dateTime);
    
    return Napi::String::New(info.Env(), hash.c_str());
}

static Napi::Value setup(const Napi::CallbackInfo& info)
{
    REQUIRE_ARGUMENT_STRING(0, rootDir);
    REQUIRE_ARGUMENT_STRING(1, licensePath);
    REQUIRE_ARGUMENT_STRING(2, deploymentId);


    Licensing::ApplicationInfoPtr applicationInfo =
        std::make_shared<Licensing::ApplicationInfo>(BeVersion(1, 0), deploymentId, "2792");

    Utf8String accessKey;
    BeFileName dbPath(rootDir);
    dbPath = dbPath.AppendToPath(L"License.db");

    s_client = Licensing::AccessKeyClient::Create
    (
        accessKey,
        applicationInfo,
        dbPath,
        true
    );
    int64_t result = s_client->ImportCheckout(BeFileName(licensePath));
    return Napi::Number::New(info.Env(), result);
}


void EntitlementChecker::Init(Napi::Env env, Napi::Object exports)
{
    exports.Set("checkEntitlement", Napi::Function::New(env, checkEntitlement));
    exports.Set("setup", Napi::Function::New(env, setup));
}
