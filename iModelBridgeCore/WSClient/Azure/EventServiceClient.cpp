/*--------------------------------------------------------------------------------------+
|
|     $Source: Azure/EventServiceClient.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "../DgnDbServerClient/DgnDbServerUtils.h"
#include <WebServices/Azure/EventServiceClient.h>
#include <iomanip>
USING_NAMESPACE_BENTLEY_DGNDBSERVER

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
EventServiceClient::EventServiceClient(Utf8StringCR repoId, Utf8StringCR userId)
{
	m_repoId = repoId;
	m_userId = userId;
	UpdateToken();
	Utf8String baseAddress = "https://" + m_nameSpace + "." + "servicebus.windows.net/";
	m_fullAddress = baseAddress + repoId + "/Subscriptions/" + userId + "/messages/head?timeout=";
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Arvind.Venkateswaran   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
EventServiceClient::EventServiceClient
										(
										Utf8StringCR repoId, 
										Utf8StringCR userId, 
										Utf8StringCR bimServerURL, 
										Credentials credentials, 
										ClientInfoPtr clientInfo, 
										UrlProvider::Environment env
										) : EventServiceClient(repoId, userId)
{
	m_bimServerURL = bimServerURL;
	m_credentials = credentials;
	m_clientInfo = (clientInfo == nullptr) ? CreateTestClientInfo() : clientInfo;
	m_env = env;
	UpdateToken();
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool EventServiceClient::UpdateToken()
{
	//return MakeEventServiceRequest(m_token, m_nameSpace); //todo: need to do some sanity check

	bool rtnVal = false;

	if(m_bimServerURL == "")
	{
		m_nameSpace = "testhubjeehwan-ns";
		m_token = "SharedAccessSignature sig=TOk40ce29TwpOYCFG7EWqHL5%2bmi9fIDX%2fYA0Ckv7Urs%3d&se=1463758026&skn=EventReceivePolicy&sr=https%3a%2f%2ftesthubjeehwan-ns.servicebus.windows.net%2ftest";
		rtnVal = true;
	}

	else
	{
		if (!MakeEventServiceRequest(m_token, m_nameSpace) || m_token == "" || m_nameSpace == "")
			throw std::invalid_argument("Could not get SAS Token and namespace");
		m_fullAddress = "https://" + m_nameSpace + "." + "servicebus.windows.net/" + m_repoId + "/Subscriptions/" + m_userId + "/messages/head?timeout=";
		rtnVal = true;
	}

	return rtnVal;
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Arvind.Venkateswaran   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool EventServiceClient::MakeEventServiceRequest(Utf8StringR outToken, Utf8StringR outNameSpace)
{
	if (!GetInfoThroughWSRepositoryClient(outToken, outNameSpace))
		return false;
	return true;
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponse EventServiceClient::MakeReceiveDeleteRequest(bool longPolling)
{
	char numBuffer[10];
	if (longPolling)
		itoa(230, numBuffer, 10); //max timeout for service bus rest api is set to 230 seconds
	else
		itoa(0, numBuffer, 10);
	Utf8String url = m_fullAddress + Utf8String(numBuffer);
	HttpRequest request(url.c_str(), "DELETE", nullptr);
	request.GetHeaders().Clear();
	request.GetHeaders().SetValue("Content-Length", "0");
	request.GetHeaders().SetValue("Content-Type", "application/atom+xml;type=entry;charset=utf-8");
	request.GetHeaders().SetAuthorization(m_token);
	request.SetTransferTimeoutSeconds(230);
	return request.Perform();
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool EventServiceClient::Receive(Utf8StringR msgOut, int retry, bool longPolling)
{
	HttpResponse response = MakeReceiveDeleteRequest(longPolling);
	switch (response.GetHttpStatus())
	{
	case HttpStatus::OK: //received a message
		msgOut = response.GetBody().AsString();
		return true;
	case HttpStatus::NoContent: //no incoming message
		msgOut = NULL;
		return true;
	case HttpStatus::NotFound: //subscription not setup yet
		return false;
	case HttpStatus::Unauthorized: //token may be expired
		if (retry >= 1 || !UpdateToken())
			return false;
		return Receive(msgOut, retry + 1, longPolling);
	default:
		return false;
	}
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool EventServiceClient::Receive(Utf8StringR msgOut, bool longPolling)
{
	return Receive(msgOut, 0, longPolling);
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Arvind.Venkateswaran   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ClientInfoPtr EventServiceClient::CreateTestClientInfo()
{
	Utf8String productId = "1654"; // Navigator Desktop, definitely needed for generating relying party URL
	WebServices::ClientInfoPtr clientInfo(new ClientInfo("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId));
	return clientInfo;
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Arvind.Venkateswaran   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String EventServiceClient::GetRepository()
{
	return ServerSchema::Plugin::Repository + ("--" + m_repoId);
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Arvind.Venkateswaran   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueR EventServiceAPILocalState::GetStubMap()
{
	return m_map;
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Arvind.Venkateswaran   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void EventServiceAPILocalState::_SaveValue(Utf8CP nameSpace, Utf8CP key, JsonValueCR value)
{
	Utf8PrintfString identifier("%s/%s", nameSpace, key);

	if (value.isNull())
	{
		m_map.removeMember(identifier);
	}
	else
	{
		m_map[identifier] = value;
	}
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Arvind.Venkateswaran   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value EventServiceAPILocalState::_GetValue(Utf8CP nameSpace, Utf8CP key) const
{
	Utf8PrintfString identifier("%s/%s", nameSpace, key);
	return m_map[identifier];
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Arvind.Venkateswaran   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SamlTokenPtr EventServiceClient::GetIMSToken()
{
	EventServiceAPILocalState localState;
	UrlProvider::Initialize(m_env, UrlProvider::DefaultTimeout, &localState);

	/*auto rpUri = ImsClient::GetLegacyRelyingPartyUri();
	auto proxy = ProxyHttpHandler::GetProxyIfReachable(rpUri, credentials);
	auto client = ImsClient::Create(clientInfo, proxy);
	SamlTokenResult result = client->RequestToken(credentials, rpUri)->GetResult();
	if (!result.IsSuccess())
		return nullptr;
	return result.GetValue();*/

	auto client = ImsClient::Create(m_clientInfo);
	SamlTokenResult result = client->RequestToken(m_credentials)->GetResult();
	if (!result.IsSuccess())
		return nullptr;
	return result.GetValue();
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Arvind.Venkateswaran   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponse EventServiceClient::MakeEventServiceAPIRequest()
{
	SamlTokenPtr imstokenptr = GetIMSToken();
	if (imstokenptr == nullptr)
	{
		HttpResponse response(HttpResponseContent::Create(HttpStringBody::Create("Invalid credentials")), "", ConnectionStatus::None, HttpStatus::Unauthorized);
		return response;
	}
	
	Utf8String url = m_bimServerURL + "/v2.4/Repositories/" + GetRepository() + "/" + ServerSchema::Schema::Repository + "/" + ServerSchema::Class::EventService;
	HttpRequest request(url.c_str(), "GET", nullptr);
	request.GetHeaders().Clear();
	request.GetHeaders().SetAuthorization(imstokenptr->ToAuthorizationString());
	request.SetTransferTimeoutSeconds(230);
	return request.Perform();
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Arvind.Venkateswaran   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool EventServiceClient::GetInfoThroughIMSClient(Utf8StringR sasToken, Utf8StringR ns)
{
	HttpResponse response = MakeEventServiceAPIRequest();
	switch (response.GetHttpStatus())
	{
	case HttpStatus::OK:
	{
		JsonValueCR jsonInstances = response.GetBody().AsJson();
		if (
			jsonInstances.isValidIndex(0) && jsonInstances.size() == 1 &&
			jsonInstances.isMember(ServerSchema::Instances) &&
			jsonInstances[ServerSchema::Instances].size() == 1 &&
			jsonInstances[ServerSchema::Instances][0].isMember(ServerSchema::Properties) &&
			jsonInstances[ServerSchema::Instances][0][ServerSchema::Properties].isMember(ServerSchema::Property::EventServiceSASToken) &&
			jsonInstances[ServerSchema::Instances][0][ServerSchema::Properties].isMember(ServerSchema::Property::EventServiceNameSpace)
			)
		{
			sasToken = jsonInstances[ServerSchema::Instances][0][ServerSchema::Properties][ServerSchema::Property::EventServiceSASToken].asCString();
			ns = jsonInstances[ServerSchema::Instances][0][ServerSchema::Properties][ServerSchema::Property::EventServiceNameSpace].asCString();
			return true;
		}
		else
		{
			sasToken = NULL;
			ns = NULL;
			return false;
		}
	}
	case HttpStatus::NoContent:
	{
		sasToken = NULL;
		ns = NULL;
		return false;
	}
	case HttpStatus::NotFound:
	{
		sasToken = NULL;
		ns = NULL;
		return false;
	}
	case HttpStatus::Unauthorized:
	{
		sasToken = NULL;
		ns = NULL;
		return false;
	}
	default:
	{
		sasToken = NULL;
		ns = NULL;
		return false;
	}
	}
}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Arvind.Venkateswaran   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool EventServiceClient::GetInfoThroughWSRepositoryClient(Utf8StringR sasToken, Utf8StringR ns)
{
	//Initialize credentials and urls
	EventServiceAPILocalState localState;
	UrlProvider::Initialize(m_env, UrlProvider::DefaultTimeout, &localState);

	//Create ConnectSignInManager and connect
	auto manager = ConnectSignInManager::Create(m_clientInfo, nullptr, &localState);
	bool isSuccess = manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess();
	if (!isSuccess)
		return false;
	auto authHandler = manager->GetAuthenticationHandler(m_bimServerURL);
	if (authHandler == nullptr)
		return false;

	//Create WSRepositoryClient 
	IWSRepositoryClientPtr wsRepositoryClient = WSRepositoryClient::Create(m_bimServerURL, GetRepository(), m_clientInfo, nullptr, authHandler);
	if (wsRepositoryClient == nullptr)
		return false;

	//Query for https://{server}/{version}/Repositories/DgnDbServer--{repoId}/EventService
	ObjectId eventServiceObject(ServerSchema::Schema::Repository, ServerSchema::Class::EventService, "");
	WSObjectsResult response = wsRepositoryClient->SendGetObjectRequest(eventServiceObject)->GetResult();
	if (!response.IsSuccess())
		return false;
	bvector<WSObjectsReader::Instance> jsoninstances;
	for (WSObjectsReader::Instance instance : response.GetValue().GetInstances())
	{
		jsoninstances.push_back(instance);
	}
	if (jsoninstances.size() < 1)
		return false;

	//Get json values
	RapidJsonValueCR instanceProperties = jsoninstances[0].GetProperties();
	if (!instanceProperties.HasMember(ServerSchema::Property::EventServiceSASToken))
		return false;
	sasToken = instanceProperties[ServerSchema::Property::EventServiceSASToken].GetString();
	ns = instanceProperties[ServerSchema::Property::EventServiceNameSpace].GetString();
	return true;
}