/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeHttp/HttpConfigurationHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/DefaultHttpHandler.h>
#include <BeHttp/HttpRequest.h>
#include <BeHttp/IHttpHandler.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpConfigurationHandler : public IHttpHandler
    {
    private:
        IHttpHandlerPtr m_handler;
        std::function<void(HttpRequest& request)> m_configuration;

    public:
        //! @param configuration - callback that will be called for request configuration
        //! @param customHandler - handler to call after configuration.
        BEHTTP_EXPORT HttpConfigurationHandler(
            std::function<void(HttpRequest& request)> configuration,
            IHttpHandlerPtr customHandler = nullptr
            );
        BEHTTP_EXPORT virtual ~HttpConfigurationHandler();

        //! Configure and Perform HttpRequest 
        BEHTTP_EXPORT virtual Tasks::AsyncTaskPtr<HttpResponse> PerformRequest(HttpRequestCR request) override;
    };

END_BENTLEY_HTTP_NAMESPACE
