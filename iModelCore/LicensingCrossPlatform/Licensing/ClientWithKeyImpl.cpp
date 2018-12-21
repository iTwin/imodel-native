/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/ClientWithKeyImpl.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientWithKeyImpl.h"
#include "GenerateSID.h"
#include "Logging.h"
#include "UsageDb.h"
#include "FreeApplicationPolicyHelper.h"
#include "ApplicationInfo.h"

#include <Licensing/Utils/LogFileHelper.h>
#include <Licensing/Utils/UsageJsonHelper.h>
#include <fstream>

#include <BeHttp/HttpError.h>
//#include <WebServices/Configuration/UrlProvider.h>

//USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_LICENSING

ClientWithKeyImpl::ClientWithKeyImpl(
	Utf8StringCR accessKey,
	ClientInfoPtr clientInfo,
	BeFileNameCR db_path,
	bool offlineMode,
	Utf8StringCR projectId,
	Utf8StringCR featureString,
	IHttpHandlerPtr httpHandler
)
	{
	m_userInfo = ConnectSignInManager::UserInfo();

	m_accessKey = accessKey;
	m_applicationInfo = applicationInfo;
	m_dbPath = db_path;
	m_offlineMode = offlineMode;
	m_projectId = projectId;
	m_featureString = featureString;
	m_httpHandler = httpHandler;
	
	m_usageDb = std::make_unique<UsageDb>();
	m_correlationId = BeGuid(true).ToString();
	m_timeRetriever = TimeRetriever::Get();
	m_delayedExecutor = DelayedExecutor::Get();

	if (projectId.empty())
		m_projectId = "00000000-0000-0000-0000-000000000000";

	if (m_offlineMode)
		{
		// Fake use m_offlineMode in order to avoid unused variable warning 
		// that is treated as error on clang compilers for iOS.
		}
	}

LicenseStatus ClientWithKeyImpl::StartApplication()
	{
	if (SUCCESS != m_usageDb->OpenOrCreate(m_dbPath))
		return LicenseStatus::Error;

	// Create dummy policy for free application usage
	m_policy = FreeApplicationPolicyHelper::CreatePolicy();

	//if (ERROR == RecordUsage())
	//	return LicenseStatus::Error;

	// Begin heartbeats
	//int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();
	//UsageHeartbeat(currentTimeUnixMs);
	//LogPostingHeartbeat(currentTimeUnixMs);
	//SendUsageRealtimeWithKey().wait();

	// This is only a logging example
	LOG.trace("StartApplication");

	return LicenseStatus::Ok;
	}

BentleyStatus ClientWithKeyImpl::StopApplication()
	{
	LOG.trace("StopApplication");

	m_lastRunningPolicyheartbeatStartTime = 0;      // This will stop Policy heartbeat
	m_lastRunningUsageheartbeatStartTime = 0;       // This will stop Usage heartbeat
	m_lastRunningLogPostingheartbeatStartTime = 0;  // This will stop log posting heartbeat

	if (m_usageDb->GetUsageRecordCount() > 0)
		PostUsageLogs();

	if (m_usageDb->GetFeatureRecordCount() > 0)
		PostFeatureLogs();

	m_usageDb->Close();

	return SUCCESS;
	}

folly::Future<folly::Unit> ClientWithKeyImpl::SendUsageRealtimeWithKey()
	{
	LOG.trace("ClientWithKeyImpl::SendUsageRealtimeWithKey");

	auto url = UrlProvider::UrlDescriptor("UsageLoggingServices.RealtimeLogging.Url", "", "", "", "", nullptr).Get();

	url += "/" + m_accessKey;

	std::ofstream logfile("D:/performSendUsageRealtimeWithKey.txt");
	logfile << url << std::endl;
	logfile.close();

	HttpClient client(nullptr, m_httpHandler);
	auto uploadRequest = client.CreateRequest(url, "POST");
	//uploadRequest.GetHeaders().SetValue("authorization", "Bearer " + m_accessTokenString);
	uploadRequest.GetHeaders().SetValue("content-type", "application/json; charset=utf-8");

	// create Json body
	auto jsonBody = UsageJsonHelper::CreateJsonRandomGuids(
		m_applicationInfo->GetDeviceId(),
		m_featureString,
		m_applicationInfo->GetApplicationVersion(),
		m_projectId
	);

	uploadRequest.SetRequestBody(HttpStringBody::Create(jsonBody));

	return uploadRequest.Perform().then(
		[=](Response response)
		{
		if (!response.IsSuccess())
			{
			std::ofstream logfile("D:/performSendUsageRealtimeWithKeyResponse.txt");
			logfile << response.GetBody().AsString() << std::endl;
			logfile << (int)response.GetHttpStatus() << std::endl;
			logfile.close();
			throw HttpError(response);
			}
		std::ofstream logfile("D:/performSendUsageRealtimeWithKeyResponse.txt");
		logfile << response.GetBody().AsString() << std::endl;
		logfile << (int)response.GetHttpStatus() << std::endl;
		logfile.close();
		return folly::makeFuture();
		});
	}
