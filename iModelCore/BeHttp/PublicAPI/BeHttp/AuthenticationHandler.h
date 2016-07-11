/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeHttp/AuthenticationHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/Http.h>
#include <Bentley/Tasks/AsyncError.h>
#include <Bentley/Tasks/AsyncResult.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/WorkerThread.h>
#include <BeHttp/DefaultHttpHandler.h>
#include <BeHttp/HttpClient.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct AuthenticationHandler> AuthenticationHandlerPtr;
struct EXPORT_VTABLE_ATTRIBUTE AuthenticationHandler : public IHttpHandler
    {
    public:
        struct Attempt;
        typedef Attempt& AttemptR;
        typedef const Attempt& AttemptCR;

        typedef Tasks::AsyncResult<Utf8String, Tasks::AsyncError> AuthorizationResult;

    private:
        IHttpHandlerPtr m_defaultHttpHandler;
        struct AuthenticationState;

    private:
        Tasks::AsyncTaskPtr<void> RetrieveAuthorizationAndPerformRequest
            (
            std::shared_ptr<AuthenticationState> authenticationState,
            std::shared_ptr<Response> finalResponseInOut
            );

        Tasks::AsyncTaskPtr<void> PerformRequest
            (
            std::shared_ptr<AuthenticationState> authenticationState,
            std::shared_ptr<Response> responseOut
            );

    protected:
        BEHTTP_EXPORT virtual bool _ShouldRetryAuthentication(ResponseCR response);
        virtual Tasks::AsyncTaskPtr<AuthorizationResult> _RetrieveAuthorization(AttemptCR previousAttempt) = 0;

    public:
        BEHTTP_EXPORT AuthenticationHandler(IHttpHandlerPtr customHttpHandler = nullptr);
        virtual ~AuthenticationHandler() {}

        BEHTTP_EXPORT virtual Tasks::AsyncTaskPtr<Response> _PerformRequest(RequestCR request) override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct AuthenticationHandler::Attempt
    {
    private:
        Utf8String m_requestUrl;
        Utf8String m_authorization;
        DateTime m_utcDate;
        unsigned m_attemptNumber;

    public:
        BEHTTP_EXPORT Attempt(Utf8String requestUrl, Utf8String authorization, DateTimeCR utcDate, unsigned attemptNumber);

        BEHTTP_EXPORT Utf8StringCR GetRequestUrl() const;
        BEHTTP_EXPORT Utf8StringCR GetAuthorization() const;
        BEHTTP_EXPORT DateTimeCR GetUtcDate() const;
        BEHTTP_EXPORT unsigned GetAttemptNumber() const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct AuthenticationHandler::AuthenticationState
    {
    private:
        Request m_request;
        Attempt m_attempt;

    public:
        AuthenticationState(Request request);
        RequestR GetRequest() {return m_request;}

        void RegisterNewAttempt();
        AttemptCR GetLastAttempt() const;
    };

END_BENTLEY_HTTP_NAMESPACE
