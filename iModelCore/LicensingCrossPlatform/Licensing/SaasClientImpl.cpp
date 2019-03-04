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
#include "Utils/FeatureEvent.h"

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Luke.Lindsey             3/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> SaasClientImpl::MarkFeature(Utf8StringCR accessToken, Utf8StringCR featureId, BeVersionCR version, Utf8StringCR projectId)
    {
    LOG.tracev("MarkFeature - Called with featureId: %s, version: %s, projectId: %s", featureId.c_str(), version.ToString().c_str(), projectId.c_str());

    // TODO put this in a feature posting sender
    auto url = m_buddiProvider->UlasRealtimeFeatureUrl();

    HttpClient client(nullptr, m_httpHandler);
    auto uploadRequest = client.CreateRequest(url, "POST");
    uploadRequest.GetHeaders().SetValue("authorization", "Bearer " + accessToken);
    uploadRequest.GetHeaders().SetValue("content-type", "application/json; charset=utf-8");

    auto jsonBody = FeatureEvent::ToJson
    (
        featureId,
        m_productId,
        m_featureString,
        version,
        m_deviceId,
        projectId
    );

    uploadRequest.SetRequestBody(HttpStringBody::Create(jsonBody));

    return uploadRequest.Perform().then(
        [=] (Response response)
        {
        if (!response.IsSuccess())
            {
            LOG.errorv("SaasClientImpl::MarkFeature ERROR: Unable to post %s - %s", jsonBody.c_str(), response.GetBody().AsString().c_str());
            return BentleyStatus::ERROR;
            }
        LOG.tracev("MarkFeature - Successfully marked featureId: %s, version: %s, projectId: %s", featureId.c_str(), version.ToString().c_str(), projectId.c_str());
        return BentleyStatus::SUCCESS;
        });
    }
