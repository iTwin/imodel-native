/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "EntitlementChecker.h"
#include "../common/ConversionUtils.h"
#include "../common/EntitlementCheckerUtils.h"
#include "../common/macros.h"
#include <Bentley/BeSystemInfo.h>
#include <BeHttp/HttpClient.h>
#include <Bentley/Bentley.h>
#include <thread>
#include <chrono>
#include <iostream>

using namespace IModelBank;

USING_NAMESPACE_BENTLEY_HTTP

Utf8String EntitlementChecker::s_url = "";
Utf8String EntitlementChecker::s_iModelId = "";


namespace Properties
{
    Utf8CP iModelId = "iModelId";
    Utf8CP time = "time";
    Utf8CP activityId = "activityId";
    Utf8CP responseValue = "responseValue";
}

Utf8String fromEntitlementStatus(EntitlementStatus status)
{
    switch (status)
    {
    case EntitlementStatus::LicensingUrlNotSet:
        return "Licensing URL is not set.";
    case EntitlementStatus::iModelIdNotSet:
        return "iModel id not set.";
    case EntitlementStatus::InvalidResponse:
        return "Received an invalid response from Licensing service.";
    case EntitlementStatus::RequestFailed:
        return "Request to Licensing service has failed.";
    default:
        return "Unknown error.";
    }
}

static void handleEntitlementError(EntitlementStatus status)
{
    std::cerr << "Entitlement check has failed." << fromEntitlementStatus(status) << std::endl << std::flush;
    exit(1);
}

Json::Value createJson(Utf8StringCR iModelId)
{
    Json::Value json(Json::objectValue);
    BeSQLite::BeGuid checkId(true);
    int64_t time = std::chrono::system_clock::now().time_since_epoch().count();
    json.SetOrRemoveInt64(Properties::time, time, 0);
    json[Properties::iModelId] = iModelId;
    json[Properties::activityId] = checkId.ToString();
    return json;
}

Request createRequest(JsonValueCR value, Utf8StringCR url)
{
    HttpStringBodyPtr body = HttpStringBody::Create(Json::FastWriter().write(value));
    HttpClient client;
    Request request = client.CreatePostRequest(url);
    request.GetHeaders().AddValue("Content-Type", "application/json");
    request.SetRequestBody(body);
    return request;
}

Utf8String calculateHashFromJson(JsonValueCR value)
{
    return calculateHash(value[Properties::activityId].asString(),
        value[Properties::iModelId].asString(),
        Utf8PrintfString("%lld", value[Properties::time].asInt64()));
}

Json::Value parseResponse(ResponseCR response)
{
    Utf8String responseString = response.GetBody().AsString();
    Json::Value responseJson = Json::Reader::DoParse(responseString);
    return responseJson;
}

EntitlementStatus ValidateResponse(JsonValueCR requestJson, JsonValueCR responseJson)
{
    Utf8String expectedResponse = calculateHashFromJson(requestJson);
    Utf8String actualResponse = responseJson["responseValue"].asString();
    if (!actualResponse.Equals(expectedResponse))
        return EntitlementStatus::InvalidResponse;
    return EntitlementStatus::Success;
}

void EntitlementChecker::initialize(const Napi::CallbackInfo& info)
{
    REQUIRE_ARGUMENT_STRING(0, licensingUrl);
    REQUIRE_ARGUMENT_STRING(1, iModelId);
    s_url = licensingUrl;
    s_iModelId = iModelId;
}

EntitlementStatus EntitlementChecker::checkEntitlement()
{
    if (Utf8String::IsNullOrEmpty(s_url.c_str()))
    {
        return EntitlementStatus::LicensingUrlNotSet;
    }

    if (Utf8String::IsNullOrEmpty(s_iModelId.c_str()))
    {
        return EntitlementStatus::iModelIdNotSet;
    }

    Json::Value requestJson = createJson(s_iModelId);
    Request request = createRequest(requestJson, s_url);
    Response response = request.Perform().get();
    HttpStatus status = response.GetHttpStatus();
    if (HttpStatus::OK != status)
    {
        std::cerr << "Entitlement Check request failed with status: " << (int)status << "." << std::endl;
        return EntitlementStatus::RequestFailed;
    }

    Json::Value responseJson = parseResponse(response);
    return ValidateResponse(requestJson, responseJson);
}

static EntitlementStatus checkEntitlementWithRetries()
    {
    int delay = 200;
    EntitlementStatus status = EntitlementStatus::Unknown;
    for (int i = 0; i < 6; ++i)
        {
        BeDuration duration = BeDuration::FromMilliseconds(delay);
        duration.Sleep();
        status = EntitlementChecker::checkEntitlement();
        if (status == EntitlementStatus::Success)
            break;
        delay *= 2;
        }
    return status;
    }


static void runEntitlementChecks(BeDuration checkInterval)
{
    try
    {
        while (true)
        {
            auto res = checkEntitlementWithRetries();

            if (res != EntitlementStatus::Success)
                handleEntitlementError(res);
            else
                ConversionUtils::GetNativeLogger().info("Entitlement check suceeded.");

            checkInterval.Sleep();
        }
    }
    catch (...)
    {
        handleEntitlementError(EntitlementStatus::Unknown);
    }
}

void EntitlementChecker::Run(Napi::Env env)
{
    BeDuration checkInterval = BeDuration::Hours(1);

    std::thread([checkInterval] { runEntitlementChecks(checkInterval); }).detach();
}
