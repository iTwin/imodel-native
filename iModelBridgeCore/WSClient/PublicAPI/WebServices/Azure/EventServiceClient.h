/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Azure/EventServiceClient.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../Client/WebServicesClient.h"
#include <DgnClientFx/Utils/Http/HttpResponse.h>
#include <WebServices/Connect/ImsClient.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <DgnClientFx/Utils/Http/ProxyHttpHandler.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <DgnDbServer/Client/RepositoryInfo.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct EventServiceClient
{
private:
	Utf8String m_nameSpace;
	Utf8String m_repoId;
	Utf8String m_userId;
	Utf8String m_fullAddress;
	Utf8String m_token;
	Utf8String m_bimServerURL;
	Credentials m_credentials;
	ClientInfoPtr m_clientInfo;
	UrlProvider::Environment m_env;
	bool UpdateToken();
	bool MakeEventServiceRequest(Utf8StringR outToken, Utf8StringR outNameSpace);
	HttpResponse MakeReceiveDeleteRequest(bool longPolling);
	bool Receive(Utf8StringR msgOut, int retry, bool longPolling);
	SamlTokenPtr GetIMSToken();
	HttpResponse MakeEventServiceAPIRequest();
	ClientInfoPtr CreateTestClientInfo();
	Utf8String GetRepository();

public:
	WSCLIENT_EXPORT EventServiceClient(Utf8StringCR repoId, Utf8StringCR userId);
	WSCLIENT_EXPORT EventServiceClient(Utf8StringCR repoId, Utf8StringCR userId, Utf8StringCR bimServerURL, Credentials credentials, ClientInfoPtr clientInfo = nullptr, UrlProvider::Environment env = UrlProvider::Environment::Dev);
	WSCLIENT_EXPORT bool Receive(Utf8StringR msgOut, bool longPolling = true);
	WSCLIENT_EXPORT bool GetInfoThroughIMSClient(Utf8StringR sasToken, Utf8StringR ns); //todo: Public for now, should be made private
	WSCLIENT_EXPORT bool GetInfoThroughWSRepositoryClient(Utf8StringR sasToken, Utf8StringR ns); //todo: Public for now, should be made private
};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Arvind.Venkateswaran   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct EventServiceAPILocalState : public ILocalState
{
private:
	Json::Value m_map;
public:
	JsonValueR GetStubMap();
	void _SaveValue(Utf8CP nameSpace, Utf8CP key, JsonValueCR value) override;
	Json::Value _GetValue(Utf8CP nameSpace, Utf8CP key) const override;
};

END_BENTLEY_WEBSERVICES_NAMESPACE