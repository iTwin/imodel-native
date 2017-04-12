/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/ClientHelper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/BCSClientHelper.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES


BCSClientHelper* BCSClientHelper::s_instance = nullptr;
BeMutex BCSClientHelper::s_mutex{};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Karolis.Dziedzelis              04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BCSClientHelper* BCSClientHelper::Initialize(WebServices::ClientInfoPtr clientInfo, IJsonLocalState * ls)
    {
    BeMutexHolder lock(s_mutex);
    if (nullptr == s_instance)
        {
        s_instance = new BCSClientHelper(clientInfo, ls);
        AsyncTasksManager::RegisterOnCompletedListener([]
            {
            if (nullptr != s_instance)
                {
                delete s_instance;
                s_instance = nullptr;
                }
            UrlProvider::Uninitialize();
            });
        }
    else
        {
        s_instance->m_clientInfo = clientInfo;
        if (nullptr != ls)
            s_instance->m_localState = ls;
        }
    return s_instance;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Karolis.Dziedzelis              04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BCSClientHelper* BCSClientHelper::GetInstance()
    {
    BeMutexHolder lock(s_mutex);
    return s_instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbClientPtr BCSClientHelper::SignIn(Tasks::AsyncError* errorOut, BCSSignInInfo const& signinInfo)
    {
    Tasks::AsyncError ALLOW_NULL_OUTPUT(error, errorOut);

    UrlProvider::Initialize(signinInfo.m_environment, UrlProvider::DefaultTimeout, m_localState);

    m_signinMgr = ConnectSignInManager::Create(m_clientInfo, nullptr, m_localState);
    auto signInResult = m_signinMgr->SignInWithCredentials(signinInfo.m_credentials)->GetResult();
    if (!signInResult.IsSuccess())
        {
        error = AsyncError(signInResult.GetError().GetMessage(), signInResult.GetError().GetDescription());;
        return nullptr;
        }
    Utf8String host = UrlProvider::Urls::BIMCollaborationServices.Get();

    AuthenticationHandlerPtr authHandler = m_signinMgr->GetAuthenticationHandler(host);

    DgnDbClientPtr client = DgnDbClient::Create(m_clientInfo, authHandler);
    client->SetServerURL(host);
    client->SetCredentials(signinInfo.m_credentials);

    return client;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BCSClientHelper::QueryProjectId(WSError* errorOut, Utf8StringCR bcsProjectName, Utf8CP wsgBentleyConnectRepository)
    {
    if (m_signinMgr == nullptr)
        {
        BeAssert(false && "Must sign in first");
        return "";
        }

    AsyncError ALLOW_NULL_OUTPUT(error, errorOut);

    auto authHandler = m_signinMgr->GetAuthenticationHandler(UrlProvider::Urls::ConnectWsgGlobal.Get());
    auto client = WSRepositoryClient::Create(UrlProvider::Urls::ConnectWsgGlobal.Get(), wsgBentleyConnectRepository, m_clientInfo, nullptr, authHandler);

    WSQuery query ("GlobalSchema", "Project");
    query.SetSelect ("$id");
    query.SetFilter (Utf8PrintfString ("Active+eq+true+and+Number+eq+'%s'", bcsProjectName.c_str()));

    auto result = client->SendQueryRequest(query)->GetResult();
    if (!result.IsSuccess())
        {
        error = result.GetError();
        return "";
        }

    JsonValueCR instance = result.GetValue().GetJsonValue()["instances"][0];
    Utf8String projectId = instance["instanceId"].asString();

    if (projectId.empty())
        error = WSError();

    return projectId;
    }
