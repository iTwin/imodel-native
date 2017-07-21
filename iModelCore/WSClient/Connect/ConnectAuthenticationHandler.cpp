/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ConnectAuthenticationHandler.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/ConnectAuthenticationHandler.h>

#include <Bentley/Base64Utilities.h>
#include <WebServices/Connect/ImsClient.h>
#include <WebServices/Connect/ConnectTokenProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectAuthenticationHandler::ConnectAuthenticationHandler
(
Utf8String urlBaseToAuth,
std::shared_ptr<IConnectTokenProvider> customTokenProvider,
IHttpHandlerPtr customHttpHandler,
bool legacyMode
) :
AuthenticationHandler(customHttpHandler),
m_urlBaseToAuth(urlBaseToAuth),
m_tokenProvider(customTokenProvider ? customTokenProvider : std::make_shared<ConnectTokenProvider>(ImsClient::GetShared())),
m_thread(WorkerThread::Create("ConnectAuthenticationHandler")),
m_legacyMode(legacyMode)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectAuthenticationHandler::~ConnectAuthenticationHandler()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectAuthenticationHandler::_ShouldRetryAuthentication(HttpResponseCR response)
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
        response.GetBody().AsJson()["errorId"].asString().Equals("DatasourceNotFound"))
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

        SamlTokenPtr token = m_tokenProvider->GetToken();
        if (!IsTokenAuthorization(previousAttempt.GetAuthorization()) &&
            nullptr != token)
            {
            finalResult->SetSuccess(token->ToAuthorizationString());
            return;
            }

        m_tokenProvider->UpdateToken()->Then(m_thread, [=]  (SamlTokenPtr token)
            {
            if (nullptr == token)
                {
                finalResult->SetError(AsyncError("Failed to get new token"));
                return;
                }
            finalResult->SetSuccess(token->ToAuthorizationString());
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
    if (0 != previousAttempt.GetRequestUrl().compare(0, m_urlBaseToAuth.size(), m_urlBaseToAuth))
        {
        return true;
        }

    unsigned int expiredTokenRetryCount = m_legacyMode ? 1 : 0;

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
    static Utf8CP prefix = "token ";
    if (0 == auth.compare(0, strlen(prefix), prefix))
        return true;

    return false;
    }
