/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ConnectAuthenticationHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/ConnectAuthenticationHandler.h>

#include <Bentley/Base64Utilities.h>
#include <WebServices/Connect/Connect.h>
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
IHttpHandlerPtr customHttpHandler
) :
AuthenticationHandler (customHttpHandler),
m_urlBaseToAuth (urlBaseToAuth),
m_tokenProvider (customTokenProvider ? customTokenProvider : std::make_shared<ConnectTokenProvider> ()),
m_thread (WorkerThread::Create ("ConnectAuthenticationHandler"))
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectAuthenticationHandler::~ConnectAuthenticationHandler ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectAuthenticationHandler::_ShouldRetryAuthentication (HttpResponseCR response)
    {
    if (response.GetHttpStatus () == HttpStatus::Unauthorized ||
        response.GetHttpStatus () == HttpStatus::Forbidden)
        {
        return true;
        }
    if (Connect::IsImsLoginRedirect (response))
        {
        return true;
        }
    if (response.GetHttpStatus () == HttpStatus::NotFound &&
        response.GetBody ().AsJson ()["errorId"].asString ().Equals ("DatasourceNotFound"))
        {
        // Token MAY be expired, retry. Workaround for TFS#7930
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AuthenticationHandler::AuthorizationResult> ConnectAuthenticationHandler::_RetrieveAuthorization (AttemptCR previousAttempt)
    {
    return m_thread->ExecuteAsync<AuthenticationHandler::AuthorizationResult > ([=]
        {
        if (ShouldStopSendingToken (previousAttempt))
            {
            return AuthenticationHandler::AuthorizationResult::Error (AsyncError ("Stopping authentication"));
            }

        SamlTokenPtr token = m_tokenProvider->GetToken ();

        if (!IsTokenAuthorization (previousAttempt.GetAuthorization ()) &&
            nullptr != token)
            {
            return AuthenticationHandler::AuthorizationResult::Success (token->ToAuthorizationString ());
            }

        token = m_tokenProvider->UpdateToken ();
        if (nullptr == token)
            {
            return AuthenticationHandler::AuthorizationResult::Error (AsyncError ("Failed to get new token"));
            }

        return AuthenticationHandler::AuthorizationResult::Success (token->ToAuthorizationString ());
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectAuthenticationHandler::ShouldStopSendingToken (AttemptCR previousAttempt) const
    {
    if (0 != previousAttempt.GetRequestUrl ().compare (0, m_urlBaseToAuth.size (), m_urlBaseToAuth))
        {
        return true;
        }

    if (IsTokenAuthorization (previousAttempt.GetAuthorization ()) &&
        previousAttempt.GetAttemptNumber () > 1)
        {
        // Used new token and it did not work
        return true;
        }

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectAuthenticationHandler::IsTokenAuthorization (Utf8StringCR auth) const
    {
    static const Utf8String prefix = "token ";
    if (0 == auth.compare (0, prefix.size (), prefix))
        {
        return true;
        }
    return false;
    }
