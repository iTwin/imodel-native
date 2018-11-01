/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/AuthenticationHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BeHttp/AuthenticationHandler.h>
#include <Bentley/Base64Utilities.h>
#include "WebLogging.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Response> AuthenticationHandler::_PerformRequest(RequestCR originalRequest)
    {
    auto requestState = std::make_shared<AuthenticationState> (originalRequest);
    auto response = std::make_shared<Response> ();

    AsyncTaskPtr<void> task;
    if (Utf8String::IsNullOrEmpty(requestState->GetRequest().GetHeaders().GetAuthorization()))
        {
        task = RetrieveAuthorizationAndPerformRequest(requestState, response);
        }
    else
        {
        task = PerformRequest(requestState, response);
        }

    return task->Then<Response> ([=]
        {
        return *response;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> AuthenticationHandler::RetrieveAuthorizationAndPerformRequest
(
std::shared_ptr<AuthenticationState> authenticationState,
std::shared_ptr<Response> responseOut
)
    {
    return
    _RetrieveAuthorization(authenticationState->GetLastAttempt())
    ->Then([=] (AuthorizationResult& result) mutable
        {
        if (result.IsSuccess())
            {
            authenticationState->GetRequest().GetHeaders().SetAuthorization(result.GetValue());
            }
        else if (authenticationState->GetLastAttempt().GetAttemptNumber() > 0)
            {
            return;
            }

        PerformRequest(authenticationState, responseOut);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> AuthenticationHandler::PerformRequest
(
std::shared_ptr<AuthenticationState> authenticationState,
std::shared_ptr<Response> responseOut
)
    {
    authenticationState->RegisterNewAttempt();
    return
    m_defaultHttpHandler->_PerformRequest(authenticationState->GetRequest())
    ->Then([=] (ResponseCR response)
        {
        *responseOut = response;

        if (_ShouldRetryAuthentication(*responseOut))
            {
            RetrieveAuthorizationAndPerformRequest(authenticationState, responseOut);
            };
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool AuthenticationHandler::_ShouldRetryAuthentication(ResponseCR response)
    {
    if (response.GetHttpStatus() == HttpStatus::Unauthorized ||
        response.GetHttpStatus() == HttpStatus::Forbidden)
        {
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AuthenticationHandler::Attempt::Attempt(Utf8String requestUrl, Utf8String authorization, DateTimeCR utcDate, unsigned attemptNumber) :
m_requestUrl(requestUrl),
m_authorization(authorization),
m_utcDate(utcDate),
m_attemptNumber(attemptNumber)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AuthenticationHandler::AuthenticationState::AuthenticationState(Request request) :
m_request(std::move(request)),
m_attempt(request.GetUrl(), "", DateTime(), 0)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void AuthenticationHandler::AuthenticationState::RegisterNewAttempt()
    {
    m_attempt = Attempt
        (
        m_request.GetUrl(),
        m_request.GetHeaders().GetAuthorization(),
        DateTime::GetCurrentTimeUtc(),
        m_attempt.GetAttemptNumber() + 1
        );
    }

