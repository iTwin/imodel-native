/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/ClientFreeImpl.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
	Utf8StringCR featureString,
	IHttpHandlerPtr httpHandler
)
	{
	m_userInfo = ConnectSignInManager::UserInfo();

	Utf8String deviceInfo = BeSystemInfo::GetDeviceId();
	if (deviceInfo.Equals("")) deviceInfo = "DefaultDevice";
	m_clientInfo = std::make_shared<ClientInfo>("FreeApplication",BeVersion(1,0),"FreeGUID",deviceInfo,"FreeDescriptor");
	m_featureString = featureString;
	m_httpHandler = httpHandler;
	m_correlationId = BeGuid(true).ToString();
	m_timeRetriever = TimeRetriever::Get();
	m_delayedExecutor = DelayedExecutor::Get();
	}

LicenseStatus ClientFreeImpl::StartApplication()
	{
	LOG.trace("ClientFreeImpl::StartApplication (This is unnecessary to call for ClientFreeImpl)");
	return LicenseStatus::Ok;
	}

BentleyStatus ClientFreeImpl::StopApplication()
	{
	LOG.trace("ClientFreeImpl::StopApplication (This is unnecessary to call for ClientFreeImpl)");
	return SUCCESS;
	}

folly::Future<BentleyStatus> ClientFreeImpl::TrackUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId)
	{
	return SendUsageRealtime(accessToken, version, projectId);
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
