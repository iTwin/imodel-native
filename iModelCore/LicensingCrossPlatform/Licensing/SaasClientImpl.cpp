/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/SaasClientImpl.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "SaasClientImpl.h"
#include "GenerateSID.h"
#include "Logging.h"
#include "UsageDb.h"
#include "FreeApplicationPolicyHelper.h"

#include <Licensing/Utils/LogFileHelper.h>
#include <Licensing/Utils/UsageJsonHelper.h>

#include <Bentley/BeSystemInfo.h>
#include <BeHttp/HttpError.h>
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_LICENSING

SaasClientImpl::SaasClientImpl
    (
    int productId,
    Utf8StringCR featureString,
    IHttpHandlerPtr httpHandler,
    IBuddiProviderPtr buddiProvider
    )
    {
    m_deviceId = BeSystemInfo::GetDeviceId();
    if (m_deviceId.Equals(""))
        m_deviceId = "DefaultDevice";
    m_productId = productId;
    m_featureString = featureString;
    m_httpHandler = httpHandler;
    m_buddiProvider = buddiProvider;
    m_correlationId = BeGuid(true).ToString();
    m_timeRetriever = TimeRetriever::Get();
    m_delayedExecutor = DelayedExecutor::Get();
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Jason.Wichert           2/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> SaasClientImpl::TrackUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId)
    {
    // Send real time usage
    LOG.trace("TrackUsage");

    auto url = m_buddiProvider->UlasRealtimeLoggingBaseUrl();

    HttpClient client(nullptr, m_httpHandler);
    auto uploadRequest = client.CreateRequest(url, "POST");
    uploadRequest.GetHeaders().SetValue("authorization", "Bearer " + accessToken);
    uploadRequest.GetHeaders().SetValue("content-type", "application/json; charset=utf-8");

    // create Json body
    auto jsonBody = UsageJsonHelper::CreateJsonRandomGuids
        (
        m_deviceId,
        m_featureString,
        version,
        projectId,
        m_productId
        );

    uploadRequest.SetRequestBody(HttpStringBody::Create(jsonBody));

    return uploadRequest.Perform().then(
        [=](Response response)
        {
        if (!response.IsSuccess())
            {
            LOG.errorv("SaasClientImpl::TrackUsage ERROR: Unable to post %s - %s", jsonBody.c_str(), response.GetBody().AsString().c_str());
            return BentleyStatus::ERROR;
            }
        return BentleyStatus::SUCCESS;
        });
    }

/*
folly::Future<Utf8String> SaasClientImpl::PerformGetUserInfo()
    {
    LOG.info("PerformGetUserInfo");

    auto requestEndpointUrl = UrlProvider::Urls::IMSOpenID.Get(); 
    requestEndpointUrl += "/.well-known/openid-configuration"; // Change this to match actual endpoint for getting userInfo endpoint

    HttpClient client(nullptr, m_httpHandler);

    // request Json containing URLs to OIDC endpoints
    Json::Value endpointJson = client.CreateGetRequest(requestEndpointUrl).Perform().then(
        [=](Response response)
        {
        std::ofstream logfile("D:/performgetuserinfo.txt");
        if (!response.IsSuccess())
            {
            logfile << (int)response.GetHttpStatus() << std::endl;
            logfile.close();
            throw HttpError(response);
            }
        logfile << response.GetBody().AsString() << std::endl;
        logfile.close();
        return Json::Value::From(response.GetBody().AsString());
        }).get();

    std::ofstream logfile("D:/performgIntrospectionEndpoint.txt");
    // parse Json to get Introspection endpoint
    auto introspectionUrl = endpointJson["introspection_endpoint"].asString();
    logfile << introspectionUrl << std::endl;
    logfile << m_accessTokenString << std::endl;
    logfile.close();
    // submit token to Introspection endpoint and get user info
    return "";
    /*return client.CreatePostRequest(introspectionUrl).Perform().then(
        [=](Response response)
        {
        std::ofstream logfile("D:/performPostIntrospection.txt");
        if (!response.IsSuccess())
            {
            logfile << (int)response.GetHttpStatus() << std::endl;
            logfile.close();
            throw HttpError(response);
            }
        logfile << response.GetBody().AsString() << std::endl;
        logfile.close();
        return response.GetBody().AsString();
        });
    }
*/
