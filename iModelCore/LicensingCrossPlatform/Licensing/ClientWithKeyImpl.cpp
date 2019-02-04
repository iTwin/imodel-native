/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/ClientWithKeyImpl.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientWithKeyImpl.h"
#include "GenerateSID.h"
#include "Logging.h"
#include "UsageDb.h"
#include "FreeApplicationPolicyHelper.h"

#include <Licensing/Utils/LogFileHelper.h>
#include <Licensing/Utils/UsageJsonHelper.h>
//#include <fstream>

#include <BeHttp/HttpError.h>
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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
	m_clientInfo = clientInfo;
	m_dbPath = db_path;
	m_offlineMode = offlineMode;
	m_projectId = projectId;
	m_featureString = featureString;
	m_httpHandler = httpHandler;
	
	// set userId to accessKey to make client compatible with existing ClientImpl code
	m_userInfo.userId = m_accessKey;

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

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicenseStatus ClientWithKeyImpl::StartApplication()
{
	LOG.debug("ClientWithKeyImpl::StartApplication");

	if (SUCCESS != m_usageDb->OpenOrCreate(m_dbPath))
	{
		LOG.error("ClientWithKeyImpl::StartApplication ERROR - Database creation failed.");
		return LicenseStatus::Error;
	}
	// Create dummy policy for free application usage
	m_policy = GetPolicyTokenWithKey();

	if (m_policy == nullptr)
	{
		LOG.error("ClientWithKeyImpl::StartApplication ERROR - Policy token object is null.");
		return LicenseStatus::Error;
	}

	// Get product status
	LicenseStatus licStatus = GetProductStatus();

	if ((LicenseStatus::Ok == licStatus) ||
		(LicenseStatus::Offline == licStatus) ||
		(LicenseStatus::Trial == licStatus))
	{
		int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();

		//UsageHeartbeat(currentTimeUnixMs);
		//LogPostingHeartbeat(currentTimeUnixMs);
		//PolicyHeartbeat(currentTimeUnixMs);
	}
	else
	{
		LOG.errorv("ClientWithKeyImpl::StartApplication - LicenseStatus Returned: %d", licStatus);
	}

	//if (ERROR == RecordUsage())
	//	return LicenseStatus::Error;

	// Begin heartbeats
	//int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();
	//UsageHeartbeat(currentTimeUnixMs);
	//LogPostingHeartbeat(currentTimeUnixMs);
	//SendUsageRealtimeWithKey().wait();


	return licStatus;
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientWithKeyImpl::StopApplication()
	{
	LOG.trace("ClientWithKeyImpl::StopApplication");

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

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy> ClientWithKeyImpl::GetPolicyTokenWithKey()
{
	LOG.debug("ClientWithKeyImpl::GetPolicyTokenWithKey");

	auto policy = GetPolicyWithKey().get();
	if (policy != nullptr)
	{
		StorePolicyInUsageDb(policy);
		DeleteAllOtherUserPolicies(policy);
	}

	return policy;
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<std::shared_ptr<Policy>> ClientWithKeyImpl::GetPolicyWithKey()
{
	LOG.debug("ClientWithKeyImpl::GetPolicyWithKey");

	return folly::collectAll(GetCertificate(), PerformGetPolicyRequestWithKey()).then(
		[](const std::tuple<folly::Try<Utf8String>, folly::Try<Utf8String>>& tup)
	{
		Utf8String cert = std::get<0>(tup).value();
		cert.ReplaceAll("\"", "");

		Utf8String policyToken = std::get<1>(tup).value();
		policyToken.ReplaceAll("\"", "");

		return Policy::Create(JWToken::Create(policyToken,cert));
	});
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> ClientWithKeyImpl::SendUsageRealtimeWithKey()
	{
	LOG.trace("ClientWithKeyImpl::SendUsageRealtimeWithKey");

	auto url = UrlProvider::UrlDescriptor("UsageLoggingServices.RealtimeLogging.Url", "", "", "", "", nullptr).Get();

	url += "/" + m_accessKey;

	/*std::ofstream logfile("D:/performSendUsageRealtimeWithKey.txt");
	logfile << url << std::endl;*/

	HttpClient client(nullptr, m_httpHandler);
	auto uploadRequest = client.CreateRequest(url, "POST");
	//uploadRequest.GetHeaders().SetValue("authorization", "Bearer " + m_accessTokenString);
	uploadRequest.GetHeaders().SetValue("content-type", "application/json; charset=utf-8");

	// create Json body
	auto jsonBody = UsageJsonHelper::CreateJsonRandomGuids(
		m_clientInfo->GetDeviceId(),
		m_featureString,
		m_clientInfo->GetApplicationVersion(),
		m_projectId
	);

	/*logfile << Json::FastWriter::ToString(Json::Reader::DoParse(jsonBody)) << std::endl;
	logfile.close();*/

	uploadRequest.SetRequestBody(HttpStringBody::Create(jsonBody));

	return uploadRequest.Perform().then(
		[=](Response response)
		{
		if (!response.IsSuccess())
			{
			/*std::ofstream logfile("D:/performSendUsageRealtimeWithKeyResponse.txt");
			logfile << response.GetBody().AsString() << std::endl;
			logfile << (int)response.GetHttpStatus() << std::endl;
			logfile.close();*/
			LOG.errorv("ClientWithKeyImpl::SendUsageRealtimeWithKey ERROR: Unable to post %s - %s", jsonBody.c_str(), response.GetBody().AsString().c_str());
			throw HttpError(response);
			}
		/*std::ofstream logfile("D:/performSendUsageRealtimeWithKeyResponse.txt");
		logfile << response.GetBody().AsString() << std::endl;
		logfile << (int)response.GetHttpStatus() << std::endl;
		logfile.close();*/
		return folly::makeFuture();
		});
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<Utf8String> ClientWithKeyImpl::PerformGetPolicyRequestWithKey()
{
	LOG.debug("ClientWithKeyImpl::PerformGetPolicyRequestWithKey");

	//auto env = UrlProvider::GetEnvironment();
	//UrlProvider::SetEnvironment(UrlProvider::Environment::Dev);

	auto url = UrlProvider::Urls::EntitlementPolicyService.Get() + "/GetPolicyWithAccessKey";

	//UrlProvider::SetEnvironment(env);

	LOG.debugv("ClientWithKeyImpl::PerformGetPolicyRequestWithKey - EntitlementPolicyService: %s", url.c_str());

	//auto authHandler = m_authProvider->GetAuthenticationHandler(url, m_httpHandler, IConnectAuthenticationProvider::HeaderPrefix::Saml);

	HttpClient client(nullptr, m_httpHandler);
	auto genSID = GenerateSID();

	auto request = client.CreatePostRequest(url);
	Json::Value requestJson(Json::objectValue);
	requestJson["MachineName"] = m_clientInfo->GetDeviceId();
	requestJson["machineSID"] = genSID.GetMachineSID(m_clientInfo->GetDeviceId());
	requestJson["ClientDateTime"] = DateTime::GetCurrentTimeUtc().ToString();
	requestJson["Locale"] = m_clientInfo->GetLanguage();
	requestJson["AppliesTo"] = GETPOLICY_RequestData_AppliesTo_Url;
	requestJson["AccessKey"] = m_accessKey;

	Json::Value requestedSecurable(Json::objectValue);
	requestedSecurable["ProductId"] = m_clientInfo->GetApplicationProductId();
	requestedSecurable["FeatureString"] = m_featureString;
	requestedSecurable["Version"] = m_clientInfo->GetApplicationVersion().ToString();

	Json::Value requestedSecurables(Json::arrayValue);
	requestedSecurables[0] = requestedSecurable;

	requestJson["RequestedSecurables"] = requestedSecurables;

	request.SetRequestBody(HttpStringBody::Create(Json::FastWriter().write(requestJson)));

	request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);

	return request.Perform().then(
		[=](Response response)
	{
		if (!response.IsSuccess())
		{
			LOG.errorv("ClientWithKeyImpl::PerformGetPolicyRequestWithKey ERROR: Unable to perform policy request %d", (int)response.GetHttpStatus());
			LOG.errorv("ClientWithKeyImpl::PerformGetPolicyRequestWithKey ERROR: Unable to perform policy request %s", response.GetContent()->GetBody()->AsString().c_str());
			return Utf8String("");
			//throw HttpError(response);
		}
		Utf8String responseString = response.GetBody().AsString();
		responseString = responseString.DropQuotes();
		return responseString;
	});
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> ClientWithKeyImpl::SendFeatureLogsWithKey(BeFileNameCR featureCSV, Utf8StringCR ultId)
{
	LOG.debug("ClientImpl::SendFeatureLogs");

	auto url = UrlProvider::Urls::UsageLoggingServicesLocation.Get();
	url += Utf8PrintfString("/featureLog?ultId=%s&prdId=%s&lng=%s", ultId.c_str(), m_clientInfo->GetApplicationProductId().c_str(),
		m_clientInfo->GetLanguage().c_str());

	LOG.debugv("ClientImpl::SendFeatureLogs - UsageLoggingServiceLocation: %s", url.c_str());

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
				LOG.errorv("ClientImpl::SendFeatureLogs ERROR: Unable to post %s - %s", featureCSV.c_str(), HttpError(response).GetMessage().c_str());
				throw HttpError(response);
			}

			BeFileName(featureCSV).BeDeleteFile();

			return folly::makeFuture();
		});
	});
}
