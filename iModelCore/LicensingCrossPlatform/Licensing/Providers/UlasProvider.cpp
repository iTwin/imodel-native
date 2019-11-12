/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "UlasProvider.h"

#include "../Logging.h"
#include "../GenerateSID.h"
#include <Licensing/Utils/LogFileHelper.h>
#include <Licensing/Utils/UsageJsonHelper.h>

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_WEBSERVICES

UlasProvider::UlasProvider
    (
    IBuddiProviderPtr buddiProvider,
    IHttpHandlerPtr httpHandler
    ) :
    m_buddiProvider(buddiProvider),
    m_httpHandler(httpHandler)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UlasProvider::PostUsageLogs(ApplicationInfoPtr applicationInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy)
    {
    LOG.debug("UlasProvider::PostUsageLogs");

    Utf8String fileName;
    bvector<WString> logFiles;
    BeFileName logPath(dbPath.GetDirectoryName());

    fileName.Sprintf("LicUsageLog.%s.csv", BeSQLite::BeGuid(true).ToString().c_str());

    logPath.AppendToPath(BeFileName(fileName));

    if (SUCCESS != licensingDb.WriteUsageToCSVFile(logPath))
        {
        LOG.error("UlasProvider::PostLogs - ERROR: Unable to write usage records to usage log.");
        return ERROR;
        }

    licensingDb.CleanUpUsages();

    Utf8String ultimateId;
    ultimateId.Sprintf("%ld", policy->GetUltimateSAPId());

    LogFileHelper lfh;
    logFiles = lfh.GetLogFiles(Utf8String(logPath.GetDirectoryName()));

    bvector<Utf8String> failedLogs; // keep track of logfiles that fail to post
    if (!logFiles.empty())
        {
        for (auto const& logFile : logFiles)
            {
            try
                {
                SendUsageLogs(applicationInfo, BeFileName(logFile), ultimateId).get(); // .get() waits and then return the value or throws the exception
                }
            catch (...)
                {
                failedLogs.push_back(Utf8String(logFile));
                }
            }
        }

    if (failedLogs.empty())
        {
        return BentleyStatus::SUCCESS;
        }

    // TODO: log logFile names that failed?
    LOG.errorv("UlasProvider::PostFeatureLogs ERROR: %d logs failed to post.", failedLogs.size());
    return BentleyStatus::ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> UlasProvider::SendUsageLogs(ApplicationInfoPtr applicationInfo, BeFileNameCR usageCSV, Utf8StringCR ultId)
    {
    LOG.debug("UlasProvider::SendUsageLogs");

    auto url = m_buddiProvider->UlasLocationBaseUrl();
    url += Utf8PrintfString("/usageLog?ultId=%s&prdId=%s&lng=%s", ultId.c_str(), applicationInfo->GetProductId().c_str(),
        applicationInfo->GetLanguage().c_str());

    LOG.debugv("UlasProvider::SendUsageLogs - UsageLoggingServiceLocation: %s", url.c_str());

    HttpClient client(nullptr, m_httpHandler);

    return client.CreateGetRequest(url).Perform().then(
        [=](Response response)
        {
        if (!response.IsSuccess())
            {
            LOG.errorv("UlasProvider::SendUsageLogs ERROR: Unable to obtain location url. Message: %s. %s", HttpError(response).GetMessage().c_str(), HttpError(response).GetDescription().c_str());
            LOG.errorv("UlasProvider::SendUsageLogs ERROR: Response connection status : ", response.GetConnectionStatus());
            LOG.errorv("UlasProvider::SendUsageLogs ERROR: Response Body (Should contain requestID) :", response.GetBody().AsString().c_str());
            throw HttpError(response);
            }

        Json::Value jsonBody = Json::Value::From(response.GetBody().AsString());
        auto status = jsonBody["status"].asString();
        auto epUri = jsonBody["epUri"].asString();
        auto sharedAccessSignature = jsonBody["epInfo"]["SharedAccessSignature"].asString();
        auto requestId = jsonBody["reqID"].asCString();

        LOG.infov("SendUsageLogs LocationService request ID: %s", requestId);

        HttpClient client(nullptr, m_httpHandler);
        auto uploadRequest = client.CreateRequest(epUri + sharedAccessSignature, "PUT");
        uploadRequest.GetHeaders().SetValue("x-ms-blob-type", "BlockBlob");
        uploadRequest.SetRequestBody(HttpFileBody::Create(usageCSV));
        return uploadRequest.Perform().then([=](Response response)
            {
            if (!response.IsSuccess())
                {
                LOG.errorv("UlasProvider::SendUsageLogs ERROR: Unable to post %s - %s", usageCSV.c_str(), HttpError(response).GetMessage().c_str());
                LOG.errorv("UlasProvider::SendUsageLogs ERROR: Response connection status : ", response.GetConnectionStatus());
                LOG.errorv("UlasProvider::SendUsageLogs ERROR: Response Body (Should contain requestID) :", response.GetBody().AsString().c_str());
                throw HttpError(response);
                }

            BeFileName(usageCSV).BeDeleteFile();

            return folly::makeFuture();
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UlasProvider::PostFeatureLogs(ApplicationInfoPtr applicationInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy)
    {
    LOG.debug("UlasProvider::PostFeatureLogs");

    Utf8String fileName;
    bvector<WString> logFiles;
    BeFileName featureLogPath(dbPath.GetDirectoryName());

    fileName.Sprintf("LicFeatureLog.%s.csv", BeSQLite::BeGuid(true).ToString().c_str());

    featureLogPath.AppendToPath(BeFileName(fileName));

    if (SUCCESS != licensingDb.WriteFeatureToCSVFile(featureLogPath))
        {
        LOG.error("UlasProvider::PostFeatureLogs ERROR: Unable to write feature usage records to features log.");
        return ERROR;
        }

    licensingDb.CleanUpFeatures();

    Utf8String ultimateId;
    ultimateId.Sprintf("%ld", policy->GetUltimateSAPId());

    LogFileHelper lfh;
    logFiles = lfh.GetLogFiles(Utf8String(featureLogPath.GetDirectoryName()));

    bvector<Utf8String> failedLogs; // keep track of logfiles that fail to post
    if (!logFiles.empty())
        {
        for (auto const& logFile : logFiles)
            {
            try
                {
                SendFeatureLogs(applicationInfo, BeFileName(logFile), ultimateId).get(); // .get() waits and then return the value or throws the exception
                }
            catch (...)
                {
                failedLogs.push_back(Utf8String(logFile));
                }
            }
        }

    if (failedLogs.empty())
        {
        return BentleyStatus::SUCCESS;
        }

    // TODO: log logFile names that failed?
    LOG.errorv("UlasProvider::PostFeatureLogs ERROR: %d logs failed to post.", failedLogs.size());
    return BentleyStatus::ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> UlasProvider::SendFeatureLogs(ApplicationInfoPtr applicationInfo, BeFileNameCR featureCSV, Utf8StringCR ultId)
    {
    LOG.debug("UlasProvider::SendFeatureLogs");

    auto url = m_buddiProvider->UlasLocationBaseUrl();
    url += Utf8PrintfString("/featureLog?ultId=%s&prdId=%s&lng=%s", ultId.c_str(), applicationInfo->GetProductId().c_str(),
        applicationInfo->GetLanguage().c_str());

    LOG.debugv("UlasProvider::SendFeatureLogs - UsageLoggingServiceLocation: %s", url.c_str());

    HttpClient client(nullptr, m_httpHandler);

    return client.CreateGetRequest(url).Perform().then(
        [=](Response response)
        {
        if (!response.IsSuccess())
            {
            LOG.errorv("UlasProvider::SendFeatureLogs ERROR: Unable to obtain location url. Message: %s. %s", HttpError(response).GetMessage().c_str(), HttpError(response).GetDescription().c_str());
            LOG.errorv("UlasProvider::SendFeatureLogs ERROR: Response connection status : ", response.GetConnectionStatus());
            LOG.errorv("UlasProvider::SendFeatureLogs ERROR: Response Body (Should contain requestID) :", response.GetBody().AsString().c_str());
            throw HttpError(response);
            }

        Json::Value jsonBody = Json::Value::From(response.GetBody().AsString());
        auto status = jsonBody["status"].asString();
        auto epUri = jsonBody["epUri"].asString();
        auto sharedAccessSignature = jsonBody["epInfo"]["SharedAccessSignature"].asString();
        auto requestId = jsonBody["reqID"].asCString();

        LOG.infov("SendFeatureLogs LocationService request ID: %s", requestId);

        HttpClient client(nullptr, m_httpHandler);
        auto uploadRequest = client.CreateRequest(epUri + sharedAccessSignature, "PUT");
        uploadRequest.GetHeaders().SetValue("x-ms-blob-type", "BlockBlob");
        uploadRequest.SetRequestBody(HttpFileBody::Create(featureCSV));
        return uploadRequest.Perform().then([=](Response response)
            {
            if (!response.IsSuccess())
                {
                LOG.errorv("UlasProvider::SendFeatureLogs ERROR: Unable to post %s - %s", featureCSV.c_str(), HttpError(response).GetMessage().c_str());
                LOG.errorv("UlasProvider::SendFeatureLogs ERROR: Response connection status : ", response.GetConnectionStatus());
                LOG.errorv("UlasProvider::SendFeatureLogs ERROR: Response Body (Should contain requestID) :", response.GetBody().AsString().c_str());
                throw HttpError(response);
                }

            BeFileName(featureCSV).BeDeleteFile();

            return folly::makeFuture();
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> UlasProvider::RealtimeTrackUsage(Utf8StringCR accessToken, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, BeVersionCR version, Utf8StringCR projectId, UsageType usageType, Utf8StringCR correlationId, AuthType authType)
    {
    LOG.debug("UlasProvider::RealtimeTrackUsage");
    // Send real time usage
    LOG.trace("TrackUsage");

    auto url = m_buddiProvider->UlasRealtimeLoggingBaseUrl();

    HttpClient client(nullptr, m_httpHandler);
    auto uploadRequest = client.CreateRequest(url, "POST");
    switch (authType)
        {
        case AuthType::SAML:
            uploadRequest.GetHeaders().SetValue("authorization", "SAML " + accessToken);
            break;
        case AuthType::OIDC:
        default:
            uploadRequest.GetHeaders().SetValue("authorization", "Bearer " + accessToken);
        }
    uploadRequest.GetHeaders().SetValue("content-type", "application/json; charset=utf-8");

    // create Json body
    auto jsonBody = UsageJsonHelper::CreateJsonRandomGuids
        (
        deviceId,
        featureString,
        version,
        projectId,
        productId,
        usageType,
        correlationId
        );

    uploadRequest.SetRequestBody(HttpStringBody::Create(jsonBody));

    return uploadRequest.Perform().then(
        [=](Response response)
        {
        if (!response.IsSuccess())
            {
            LOG.errorv("SaasClientImpl::TrackUsage ERROR: Unable to post %s - %s", jsonBody.c_str(), response.GetBody().AsString().c_str());
            LOG.errorv("SaasClientImpl::TrackUsage ERROR: Response connection status : ", response.GetConnectionStatus());
            LOG.errorv("SaasClientImpl::TrackUsage ERROR: Response Body (Should contain requestID) :", response.GetBody().AsString().c_str());
            return BentleyStatus::ERROR;
            }
        return BentleyStatus::SUCCESS;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> UlasProvider::RealtimeMarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, UsageType usageType, Utf8StringCR correlationId, AuthType authType)
    {
    LOG.debug("UlasProvider::RealtimeMarkFeature");
    //LOG.tracev("MarkFeature - Called with featureId: %s, version: %s, projectId: %s", featureEvent.m_featureId.c_str(), featureEvent.m_version.ToString().c_str(), featureEvent.m_projectId.c_str());

    auto url = m_buddiProvider->UlasRealtimeFeatureUrl();

    HttpClient client(nullptr, m_httpHandler);
    auto uploadRequest = client.CreateRequest(url, "POST");
    switch (authType)
        {
        case AuthType::SAML:
            uploadRequest.GetHeaders().SetValue("authorization", "SAML " + accessToken);
            break;
        case AuthType::OIDC:
        default:
            uploadRequest.GetHeaders().SetValue("authorization", "Bearer " + accessToken);
        }
    
    uploadRequest.GetHeaders().SetValue("content-type", "application/json; charset=utf-8");

    auto jsonBody = featureEvent.ToJson
    (
        productId,
        featureString,
        deviceId,
        usageType,
        correlationId
    );

    uploadRequest.SetRequestBody(HttpStringBody::Create(jsonBody));

    return uploadRequest.Perform().then(
        [=](Response response)
        {
        if (!response.IsSuccess())
            {
            LOG.errorv("UlasProvider::RealtimeMarkFeature ERROR: Unable to post %s - %s", jsonBody.c_str(), response.GetBody().AsString().c_str());
            LOG.errorv("UlasProvider::RealtimeMarkFeature ERROR: Response connection status : ", response.GetConnectionStatus());
            LOG.errorv("UlasProvider::RealtimeMarkFeature ERROR: Response Body (Should contain requestID) :", response.GetBody().AsString().c_str());
            return BentleyStatus::ERROR;
            }
        //LOG.tracev("MarkFeature - Successfully marked featureId: %s, version: %s, projectId: %s", featureEvent.m_featureId.c_str(), featureEvent.m_version.ToString().c_str(), featureEvent.m_projectId.c_str());
        return BentleyStatus::SUCCESS;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<Json::Value> UlasProvider::GetAccessKeyInfo(ApplicationInfoPtr applicationInfo, Utf8StringCR accessKey, Utf8StringCR ultimateId)
    {
    LOG.debug("UlasProvider::GetAccessKeyInfo");

    auto url = m_buddiProvider->UlasAccessKeyBaseUrl();
    url += "/info"; // does not require auth

    HttpClient client(nullptr, m_httpHandler);
    auto uploadRequest = client.CreateRequest(url, "POST");
    uploadRequest.GetHeaders().SetValue("content-type", "application/json; charset=utf-8");

    Json::Value requestJson(Json::objectValue);
    GenerateSID gsid;

    requestJson["accesskey"] = accessKey;

    // pass ultimate ID as MachineSID field if using a machine agnostic AccessKey
    requestJson["cSID"] = ultimateId == "" ? gsid.GetMachineSID(applicationInfo->GetDeviceId()) : ultimateId;

    Utf8String jsonBody = Json::FastWriter().write(requestJson);

    uploadRequest.SetRequestBody(HttpStringBody::Create(jsonBody));

    return uploadRequest.Perform().then(
        [=](Response response)
        {
        if (!response.IsSuccess())
            {
            // call failed
            LOG.errorv("UlasProvider::GetAccessKeyInfo ERROR: - %s", HttpError(response).GetMessage().c_str());
            LOG.errorv("UlasProvider::GetAccessKeyInfo ERROR: Response connection status : ", response.GetConnectionStatus());
            LOG.errorv("UlasProvider::GetAccessKeyInfo ERROR: Response Body (Should contain requestID) :", response.GetBody().AsString().c_str());
            return Json::Value::GetNull();
            }

        auto responseBody = response.GetBody().AsString();
        return Json::Value::From(response.GetBody().AsString());
        });
    }
