/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Providers/UlasProvider.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#include "../Logging.h"
#include <Licensing/Utils/LogFileHelper.h>
#include "UlasProvider.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_WEBSERVICES

UlasProvider::UlasProvider
    (
    IBuddiProviderPtr buddiProvider,
    ClientInfoPtr clientInfo,
    BeFileNameCR dbPath,
    IHttpHandlerPtr httpHandler
    ) :
    m_buddiProvider(buddiProvider),
    m_clientInfo(clientInfo),
    m_dbPath(dbPath),
    m_httpHandler(httpHandler)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UlasProvider::PostUsageLogs(ILicensingDb& usageDb, std::shared_ptr<Policy> policy)
    {
    LOG.debug("UlasProvider::PostUsageLogs");

    Utf8String fileName;
    bvector<WString> logFiles;
    BeFileName logPath(m_dbPath.GetDirectoryName());

    fileName.Sprintf("LicUsageLog.%s.csv", BeGuid(true).ToString().c_str());

    logPath.AppendToPath(BeFileName(fileName));

    if (SUCCESS != usageDb.WriteUsageToCSVFile(logPath))
        {
        LOG.error("UlasProvider::PostLogs - ERROR: Unable to write usage records to usage log.");
        return ERROR;
        }

    usageDb.CleanUpUsages();

    Utf8String ultimateId;
    ultimateId.Sprintf("%ld", policy->GetUltimateSAPId());

    LogFileHelper lfh;
    logFiles = lfh.GetLogFiles(Utf8String(logPath.GetDirectoryName()));

    if (!logFiles.empty())
        {
        for (auto const& logFile : logFiles)
            {
            SendUsageLogs(BeFileName(logFile), ultimateId).wait();
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> UlasProvider::SendUsageLogs(BeFileNameCR usageCSV, Utf8StringCR ultId)
    {
    LOG.debug("UlasProvider::SendUsageLogs");

    auto url = m_buddiProvider->UlasLocationBaseUrl();
    url += Utf8PrintfString("/usageLog?ultId=%s&prdId=%s&lng=%s", ultId.c_str(), m_clientInfo->GetApplicationProductId().c_str(),
        m_clientInfo->GetLanguage().c_str());

    LOG.debugv("UlasProvider::SendUsageLogs - UsageLoggingServiceLocation: %s", url.c_str());

    HttpClient client(nullptr, m_httpHandler);

    return client.CreateGetRequest(url).Perform().then(
        [=](Response response)
        {
        if (!response.IsSuccess())
            throw HttpError(response);

        Json::Value jsonBody = Json::Value::From(response.GetBody().AsString());
        auto status = jsonBody["status"].asString();
        auto epUri = jsonBody["epUri"].asString();
        auto sharedAccessSignature = jsonBody["epInfo"]["SharedAccessSignature"].asString();

        HttpClient client(nullptr, m_httpHandler);
        auto uploadRequest = client.CreateRequest(epUri + sharedAccessSignature, "PUT");
        uploadRequest.GetHeaders().SetValue("x-ms-blob-type", "BlockBlob");
        uploadRequest.SetRequestBody(HttpFileBody::Create(usageCSV));
        return uploadRequest.Perform().then([=](Response response)
            {
            if (!response.IsSuccess())
                {
                LOG.errorv("UlasProvider::SendUsageLogs ERROR: Unable to post %s - %s", usageCSV.c_str(), HttpError(response).GetMessage().c_str());
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
BentleyStatus UlasProvider::PostFeatureLogs(ILicensingDb& usageDb, std::shared_ptr<Policy> policy)
    {
    LOG.debug("UlasProvider::PostFeatureLogs");

    Utf8String fileName;
    bvector<WString> logFiles;
    BeFileName featureLogPath(m_dbPath.GetDirectoryName());

    fileName.Sprintf("LicFeatureLog.%s.csv", BeGuid(true).ToString().c_str());

    featureLogPath.AppendToPath(BeFileName(fileName));

    if (SUCCESS != usageDb.WriteFeatureToCSVFile(featureLogPath))
        {
        LOG.error("UlasProvider::PostFeatureLogs ERROR: Unable to write feature usage records to features log.");
        return ERROR;
        }

    usageDb.CleanUpFeatures();

    Utf8String ultimateId;
    ultimateId.Sprintf("%ld", policy->GetUltimateSAPId());

    LogFileHelper lfh;
    logFiles = lfh.GetLogFiles(Utf8String(featureLogPath.GetDirectoryName()));

    if (!logFiles.empty())
        {
        for (auto const& logFile : logFiles)
            {
            SendFeatureLogs(BeFileName(logFile), ultimateId).wait();
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> UlasProvider::SendFeatureLogs(BeFileNameCR featureCSV, Utf8StringCR ultId)
    {
    LOG.debug("UlasProvider::SendFeatureLogs");

    auto url = m_buddiProvider->UlasLocationBaseUrl();
    url += Utf8PrintfString("/featureLog?ultId=%s&prdId=%s&lng=%s", ultId.c_str(), m_clientInfo->GetApplicationProductId().c_str(),
        m_clientInfo->GetLanguage().c_str());

    LOG.debugv("UlasProvider::SendFeatureLogs - UsageLoggingServiceLocation: %s", url.c_str());

    HttpClient client(nullptr, m_httpHandler);

    return client.CreateGetRequest(url).Perform().then(
        [=](Response response)
        {
        if (!response.IsSuccess())
            throw HttpError(response);

        Json::Value jsonBody = Json::Value::From(response.GetBody().AsString());
        auto status = jsonBody["status"].asString();
        auto epUri = jsonBody["epUri"].asString();
        auto sharedAccessSignature = jsonBody["epInfo"]["SharedAccessSignature"].asString();

        HttpClient client(nullptr, m_httpHandler);
        auto uploadRequest = client.CreateRequest(epUri + sharedAccessSignature, "PUT");
        uploadRequest.GetHeaders().SetValue("x-ms-blob-type", "BlockBlob");
        uploadRequest.SetRequestBody(HttpFileBody::Create(featureCSV));
        return uploadRequest.Perform().then([=](Response response)
            {
            if (!response.IsSuccess())
                {
                LOG.errorv("UlasProvider::SendFeatureLogs ERROR: Unable to post %s - %s", featureCSV.c_str(), HttpError(response).GetMessage().c_str());
                throw HttpError(response);
                }

            BeFileName(featureCSV).BeDeleteFile();

            return folly::makeFuture();
            });
        });
    }
