/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/ClientFreeImpl.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientFreeImpl.h"
#include "GenerateSID.h"
#include "Logging.h"
#include "UsageDb.h"
#include "FreeApplicationPolicyHelper.h"

#include <Licensing/Utils/LogFileHelper.h>

#include <Bentley/BeSystemInfo.h>
#include <BeHttp/HttpError.h>
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_LICENSING

ClientFreeImpl::ClientFreeImpl(
	Utf8StringCR accessToken,
	BeVersionCR clientVersion,
	BeFileNameCR db_path,
	bool offlineMode,
	Utf8StringCR projectId,
	Utf8StringCR featureString,
	IHttpHandlerPtr httpHandler
)
	{
	m_userInfo = ConnectSignInManager::UserInfo();
	m_accessToken = accessToken;

	Utf8String deviceInfo = BeSystemInfo::GetDeviceId();
	if (deviceInfo.Equals("")) deviceInfo = "DefaultDevice";
	m_clientInfo = std::make_shared<ClientInfo>("FreeApplication",clientVersion,"FreeGUID",deviceInfo,"FreeDescriptor");
	
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

LicenseStatus ClientFreeImpl::StartApplication()
	{
	LOG.trace("ClientFreeImpl::StartApplication");

	if (SUCCESS != m_usageDb->OpenOrCreate(m_dbPath))
		{
		LOG.error("ClientFreeImpl::StartApplication ERROR - Database creation failed.");
		return LicenseStatus::Error;
		}

	// Create dummy policy for free application usage
	m_policy = FreeApplicationPolicyHelper::CreatePolicy();

	// Begin heartbeats
	int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();
	
	UsageHeartbeatRealTime(currentTimeUnixMs);

	return LicenseStatus::Ok;
	}

BentleyStatus ClientFreeImpl::StopApplication()
	{
	LOG.trace("ClientFreeImpl::StopApplication");

	StopUsageHeartbeat();

	m_usageDb->Close();

	return SUCCESS;
	}
/*
folly::Future<Utf8String> ClientFreeImpl::PerformGetUserInfo()
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
