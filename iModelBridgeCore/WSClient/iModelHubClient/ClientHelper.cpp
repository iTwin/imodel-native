/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/ClientHelper.h>
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include "Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES

ClientHelper* ClientHelper::s_instance = nullptr;
BeMutex ClientHelper::s_mutex{};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ClientHelper::GetUrl()
    {
    if (Utf8String::IsNullOrEmpty(m_url.c_str()))
        m_url = UrlProvider::Urls::iModelHubApi.Get();
    return m_url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              04/17
+---------------+---------------+---------------+---------------+---------------+------*/
ClientHelper* ClientHelper::Initialize(ClientInfoPtr clientInfo, IJsonLocalState * ls, IHttpHandlerPtr customHandler)
    {
    BeMutexHolder lock(s_mutex);
    if (nullptr == s_instance)
        {
        s_instance = new ClientHelper(clientInfo, ls, customHandler);
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
        s_instance->m_localState = ls;
        s_instance->m_customHandler = customHandler;
        }
    return s_instance;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              04/17
+---------------+---------------+---------------+---------------+---------------+------*/
ClientHelper* ClientHelper::GetInstance()
    {
    BeMutexHolder lock(s_mutex);
    return s_instance;
    }


/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              06/18
+---------------+---------------+---------------+---------------+---------------+------*/
struct StaticAuthenticationHandler : public Http::AuthenticationHandler
{
private:
    Utf8String m_authorizationHeader;
protected:
    virtual Tasks::AsyncTaskPtr<Http::AuthenticationHandler::AuthorizationResult> _RetrieveAuthorization(Http::AuthenticationHandler::AttemptCR previousAttempt) override;
public:
    StaticAuthenticationHandler(Utf8StringCR authorizationHeader, IHttpHandlerPtr customHandler = nullptr);
    static Http::AuthenticationHandlerPtr Create(Utf8StringCR authorizationHeader, IHttpHandlerPtr customHandler = nullptr);
};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              06/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tasks::AsyncTaskPtr<AuthenticationHandler::AuthorizationResult> StaticAuthenticationHandler::_RetrieveAuthorization(AuthenticationHandler::AttemptCR previousAttempt)
    {
    return CreateCompletedAsyncTask<AuthenticationHandler::AuthorizationResult>(AuthenticationHandler::AuthorizationResult::Success(m_authorizationHeader));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              06/18
+---------------+---------------+---------------+---------------+---------------+------*/
StaticAuthenticationHandler::StaticAuthenticationHandler(Utf8StringCR authorizationHeader, IHttpHandlerPtr customHandler)
    : m_authorizationHeader(authorizationHeader), Http::AuthenticationHandler(customHandler)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              06/18
+---------------+---------------+---------------+---------------+---------------+------*/
Http::AuthenticationHandlerPtr StaticAuthenticationHandler::Create(Utf8StringCR authorizationHeader, IHttpHandlerPtr customHandler)
    {
    return std::make_shared<StaticAuthenticationHandler>(authorizationHeader, customHandler);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              06/18
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr ClientHelper::SignInWithStaticHeader(Utf8StringCR authorizationHeader)
    {
    return Client::Create(m_clientInfo, StaticAuthenticationHandler::Create(authorizationHeader, m_customHandler), GetUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr ClientHelper::SignInWithCredentials(AsyncError* errorOut, Credentials credentials)
    {
    Tasks::AsyncError ALLOW_NULL_OUTPUT(error, errorOut);

    auto signInManager = ConnectSignInManager::Create(m_clientInfo, m_customHandler, m_localState);
    m_signinMgr = signInManager;
    auto signInResult = ExecuteAsync(signInManager->SignInWithCredentials(credentials));
    if (!signInResult->IsSuccess())
        {
        error = AsyncError(signInResult->GetError().GetMessage(), signInResult->GetError().GetDescription());
        return nullptr;
        }

    return SignInWithManager(m_signinMgr);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Andrius.Zonys                   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr ClientHelper::SignInWithToken(AsyncError* errorOut, SamlTokenPtr token)
    {
    Tasks::AsyncError ALLOW_NULL_OUTPUT(error, errorOut);

    auto signInManager = ConnectSignInManager::Create(m_clientInfo, m_customHandler, m_localState);
    m_signinMgr = signInManager;
    auto signInResult = ExecuteAsync(signInManager->SignInWithToken(token));
    if (!signInResult->IsSuccess())
        {
        error = AsyncError(signInResult->GetError().GetMessage(), signInResult->GetError().GetDescription());
        return nullptr;
        }

    return SignInWithManager(m_signinMgr);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas            03/17
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr ClientHelper::SignInWithManager(IConnectSignInManagerPtr managerPtr)
    {
    m_signinMgr = managerPtr;

    AuthenticationHandlerPtr authHandler = m_signinMgr->GetAuthenticationHandler(GetUrl(), s_instance->m_customHandler);
    ClientPtr client = Client::Create(m_clientInfo, authHandler, GetUrl().c_str());

    return client;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas            03/17
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr ClientHelper::SignInWithManager(IConnectSignInManagerPtr managerPtr, WebServices::UrlProvider::Environment environment)
    {
    UrlProvider::Initialize(environment, UrlProvider::DefaultTimeout, m_localState);
    return SignInWithManager(managerPtr);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             10/18
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr ClientHelper::CreateClient(IConnectTokenProviderPtr tokenProviderPtr, IConnectAuthenticationProvider::HeaderPrefix prefix)
    {
    auto clientHelper = ClientHelper::GetInstance();
    auto configurationHandler = UrlProvider::GetSecurityConfigurator(clientHelper->m_customHandler);
    auto url = clientHelper->GetUrl();
    auto connectHandler = ConnectAuthenticationHandler::Create
        (
        url,
        tokenProviderPtr,
        configurationHandler,
        IConnectAuthenticationProvider::HeaderPrefix::Bearer == prefix ? TOKENPREFIX_BEARER : TOKENPREFIX_Token
        );
    connectHandler->EnableExpiredTokenRefresh();
    return Client::Create(clientHelper->m_clientInfo, connectHandler, url.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientHelper::QueryProjectId(WSError* errorOut, Utf8StringCR bcsProjectNumber, Utf8CP wsgBentleyConnectRepository)
    {
    if (m_signinMgr == nullptr)
        {
        BeAssert(false && "Must sign in first");
        return "";
        }

    AsyncError ALLOW_NULL_OUTPUT(error, errorOut);

    auto authHandler = m_signinMgr->GetAuthenticationHandler(UrlProvider::Urls::ConnectedContext.Get());
    auto client = WSRepositoryClient::Create(UrlProvider::Urls::ConnectedContext.Get(), wsgBentleyConnectRepository, m_clientInfo, nullptr, authHandler);

    WSQuery query("ConnectedContext", "Project");
    query.SetSelect("$id");
    query.SetFilter(Utf8PrintfString("Status+eq+1+and+Number+eq+'%s'", bcsProjectNumber.c_str()));

    auto result = ExecuteAsync(client->SendQueryRequest(query));
    if (!result->IsSuccess())
        {
        error = result->GetError();
        return "";
        }

    JsonValueCR instance = result->GetValue().GetJsonValue()["instances"][0];
    Utf8String projectId = instance["instanceId"].asString();

    if (projectId.empty())
        error = WSError();

    return projectId;
    }
