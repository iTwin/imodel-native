/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/ConnectAuthenticationHandler.h>

#include <Bentley/Base64Utilities.h>
#include <WebServices/Connect/ImsClient.h>
#include <WebServices/Connect/ConnectTokenProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectAuthenticationHandler::ConnectAuthenticationHandler
(
Utf8String urlBaseToAuth,
std::shared_ptr<IConnectTokenProvider> customTokenProvider,
IHttpHandlerPtr customHttpHandler,
Utf8String tokenPrefix
) :
AuthenticationHandler(customHttpHandler),
m_urlBaseToAuth(urlBaseToAuth),
m_tokenProvider(customTokenProvider ? customTokenProvider : std::make_shared<ConnectTokenProvider>(ImsClient::GetShared())),
m_thread(WorkerThread::Create("ConnectAuthenticationHandler")),
m_tokenPrefix(tokenPrefix),
m_retryExpiredToken(false)
    {}


/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ConnectAuthenticationHandler> ConnectAuthenticationHandler::CreateLegacy
(
Utf8String urlBaseToAuth,
std::shared_ptr<IConnectTokenProvider> customTokenProvider,
IHttpHandlerPtr customHttpHandler,
bool shouldUseSAMLAuthorization
)
    {
    auto manager = std::shared_ptr<ConnectAuthenticationHandler>(new ConnectAuthenticationHandler(urlBaseToAuth, customTokenProvider, customHttpHandler, shouldUseSAMLAuthorization ? TOKENPREFIX_SAML : TOKENPREFIX_Token));
    manager->EnableExpiredTokenRefresh();
    return manager;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ConnectAuthenticationHandler> ConnectAuthenticationHandler::Create
(
Utf8String urlBaseToAuth,
std::shared_ptr<IConnectTokenProvider> customTokenProvider,
IHttpHandlerPtr customHttpHandler,
Utf8String tokenPrefix
)
    {
    return std::shared_ptr<ConnectAuthenticationHandler>(new ConnectAuthenticationHandler(urlBaseToAuth, customTokenProvider, customHttpHandler, tokenPrefix));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectAuthenticationHandler::EnableExpiredTokenRefresh()
    {
    m_retryExpiredToken = true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectAuthenticationHandler::~ConnectAuthenticationHandler()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectAuthenticationHandler::_ShouldRetryAuthentication(Http::ResponseCR response)
    {
    if (response.GetHttpStatus() == HttpStatus::Unauthorized ||
        response.GetHttpStatus() == HttpStatus::Forbidden)
        {
        return true;
        }
    if (ImsClient::IsLoginRedirect(response))
        {
        return true;
        }
    if (response.GetHttpStatus() == HttpStatus::NotFound &&
        Json::Reader::DoParse(response.GetBody().AsString())["errorId"].asString().Equals("DatasourceNotFound"))
        {
        // Token MAY be expired, retry. Workaround for TFS#7930
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AuthenticationHandler::AuthorizationResult> ConnectAuthenticationHandler::_RetrieveAuthorization(AttemptCR previousAttempt)
    {
    auto finalResult = std::make_shared<AuthenticationHandler::AuthorizationResult>();
    return m_thread->ExecuteAsync([=]
        {
        if (ShouldStopSendingToken(previousAttempt))
            {
            finalResult->SetError(AsyncError("Stopping authentication"));
            return;
            }

        auto token = m_tokenProvider->GetToken();

        if (!IsTokenAuthorization(previousAttempt.GetAuthorization()) &&
            nullptr != token)
            {
            finalResult->SetSuccess(GetTokenAuthorization(token));
            return;
            }

        m_tokenProvider->UpdateToken()->Then(m_thread, [=]  (ISecurityTokenPtr token)
            {
            if (nullptr == token)
                {
                finalResult->SetError(AsyncError("Failed to get new token"));
                return;
                }
            finalResult->SetSuccess(GetTokenAuthorization(token));
            });
        })->Then<AuthenticationHandler::AuthorizationResult>([=]
            {
            return *finalResult;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectAuthenticationHandler::ShouldStopSendingToken(AttemptCR previousAttempt) const
    {
    if (!previousAttempt.GetRequestUrl().StartsWithI(m_urlBaseToAuth.c_str()))
        return true;

    unsigned int expiredTokenRetryCount = m_retryExpiredToken ? 1 : 0;

    if (IsTokenAuthorization(previousAttempt.GetAuthorization()) &&
        previousAttempt.GetAttemptNumber() > expiredTokenRetryCount)
        {
        // Used token and it did not work, try updating token
        return true;
        }

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectAuthenticationHandler::IsTokenAuthorization(Utf8StringCR auth) const
    {
    if (0 == auth.compare(0, strlen( m_tokenPrefix.c_str()),  m_tokenPrefix.c_str()))
        return true; 
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ConnectAuthenticationHandler::GetTokenAuthorization(ISecurityTokenPtr token) const
    {
    return m_tokenPrefix + " " + token->ToAuthorizationString();
    }
