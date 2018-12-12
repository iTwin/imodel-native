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

#include <Licensing/Utils/LogFileHelper.h>
#include <fstream>

#include <BeHttp/HttpError.h>
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_LICENSING

ClientWithKeyImpl::ClientWithKeyImpl(
	Utf8String accessKey,
	ClientInfoPtr clientInfo,
	BeFileNameCR db_path,
	bool offlineMode,
	Utf8String projectId,
	Utf8String featureString,
	IHttpHandlerPtr httpHandler
)
	{
	m_userInfo = ConnectSignInManager::UserInfo();

	m_accessTokenString = accessKey;
	m_clientInfo = clientInfo;
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
	int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();
	//UsageHeartbeat(currentTimeUnixMs);
	//LogPostingHeartbeat(currentTimeUnixMs);

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
