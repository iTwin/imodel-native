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

Json::Value createJson(Utf8String activityId, Utf8String iModelId, Utf8String time)
{
    Json::Value json(Json::objectValue);
    json[Properties::time] = time;
    json[Properties::iModelId] = iModelId;
    json[Properties::activityId] = activityId;
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

EntitlementStatus ValidateResponse(Utf8StringCR expectedResponse, JsonValueCR responseJson)
{
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

EntitlementStatus checkEntitlement(Utf8StringCR url, JsonValueCR requestJson, Utf8StringCR expectedResponse)
{
    Request request = createRequest(requestJson, url);
    Response response = request.Perform().get();
    HttpStatus status = response.GetHttpStatus();
    if (HttpStatus::OK != status)
    {
        std::cerr << "Entitlement Check request failed with status: " << (int)status << "." << std::endl;
        return EntitlementStatus::RequestFailed;
    }

    Json::Value responseJson = parseResponse(response);
    return ValidateResponse(expectedResponse, responseJson);
}

EntitlementStatus EntitlementChecker::checkEntitlementWithRetries()
{
    if (Utf8String::IsNullOrEmpty(s_url.c_str()))
    {
        return EntitlementStatus::LicensingUrlNotSet;
    }

    if (Utf8String::IsNullOrEmpty(s_iModelId.c_str()))
    {
        return EntitlementStatus::iModelIdNotSet;
    }

    int delay = 200;
    Utf8String activityId = BeSQLite::BeGuid(true).ToString();
    Utf8PrintfString time("%lld", std::chrono::system_clock::now().time_since_epoch().count());

    Json::Value requestJson = createJson(activityId, s_iModelId, time);
    Utf8String hash = calculateHash(activityId, s_iModelId, time);

    EntitlementStatus status = EntitlementStatus::Unknown;
    for (int i = 0; i < 6; ++i)
    {
        status = checkEntitlement(s_url, requestJson, hash);
        if (status == EntitlementStatus::Success)
            break;
        BeDuration duration = BeDuration::FromMilliseconds(delay);
        duration.Sleep();
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
            int delay = 1000;
            BeDuration duration = BeDuration::FromMilliseconds(delay);
            duration.Sleep();
            auto res = EntitlementChecker::checkEntitlementWithRetries();

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
