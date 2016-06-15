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
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AuthenticationHandler::AuthenticationHandler (IHttpHandlerPtr customHttpHandler) :
m_defaultHttpHandler (customHttpHandler == nullptr ? DefaultHttpHandler::GetInstance () : customHttpHandler)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AuthenticationHandler::~AuthenticationHandler ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<HttpResponse> AuthenticationHandler::PerformRequest (HttpRequestCR originalRequest)
    {
    auto requestState = std::make_shared<AuthenticationState> (originalRequest);
    auto response = std::make_shared<HttpResponse> ();

    AsyncTaskPtr<void> task;
    if (Utf8String::IsNullOrEmpty (requestState->GetRequest ().GetHeaders ().GetAuthorization ()))
        {
        task = RetrieveAuthorizationAndPerformRequest (requestState, response);
        }
    else
        {
        task = PerformRequest (requestState, response);
        }

    return task->Then<HttpResponse> ([=]
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
std::shared_ptr<HttpResponse> responseOut
)
    {
    return
    _RetrieveAuthorization (authenticationState->GetLastAttempt ())
    ->Then ([=] (AuthorizationResult& result) mutable
        {
        if (result.IsSuccess ())
            {
            authenticationState->GetRequest ().GetHeaders ().SetAuthorization (result.GetValue ());
            }
        else if (authenticationState->GetLastAttempt ().GetAttemptNumber () > 0)
            {
            return;
            }

        PerformRequest (authenticationState, responseOut);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> AuthenticationHandler::PerformRequest
(
std::shared_ptr<AuthenticationState> authenticationState,
std::shared_ptr<HttpResponse> responseOut
)
    {
    authenticationState->RegisterNewAttempt ();
    return
    m_defaultHttpHandler->PerformRequest (authenticationState->GetRequest ())
    ->Then ([=] (HttpResponseCR response)
        {
        *responseOut = response;

        if (_ShouldRetryAuthentication (*responseOut))
            {
            RetrieveAuthorizationAndPerformRequest (authenticationState, responseOut);
            };
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool AuthenticationHandler::_ShouldRetryAuthentication (HttpResponseCR response)
    {
    if (response.GetHttpStatus () == HttpStatus::Unauthorized ||
        response.GetHttpStatus () == HttpStatus::Forbidden)
        {
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AuthenticationHandler::Attempt::Attempt (Utf8String requestUrl, Utf8String authorization, DateTimeCR utcDate, unsigned attemptNumber) :
m_requestUrl (requestUrl),
m_authorization (authorization),
m_utcDate (utcDate),
m_attemptNumber (attemptNumber)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR AuthenticationHandler::Attempt::GetRequestUrl () const
    {
    return m_requestUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR AuthenticationHandler::Attempt::GetAuthorization () const
    {
    return m_authorization;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DateTimeCR AuthenticationHandler::Attempt::GetUtcDate () const
    {
    return m_utcDate;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned AuthenticationHandler::Attempt::GetAttemptNumber () const
    {
    return m_attemptNumber;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AuthenticationHandler::AuthenticationState::AuthenticationState (HttpRequest request) :
m_request (std::move (request)),
m_attempt (request.GetUrl (), "", DateTime (), 0)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequestR AuthenticationHandler::AuthenticationState::GetRequest ()
    {
    return m_request;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void AuthenticationHandler::AuthenticationState::RegisterNewAttempt ()
    {
    m_attempt = Attempt
        (
        m_request.GetUrl (),
        m_request.GetHeaders ().GetAuthorization (),
        DateTime::GetCurrentTimeUtc (),
        m_attempt.GetAttemptNumber () + 1
        );
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AuthenticationHandler::AttemptCR AuthenticationHandler::AuthenticationState::GetLastAttempt () const
    {
    return m_attempt;
    }
