/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/IHttpHandler.h>
#include <mutex>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct DefaultHttpHandler> DefaultHttpHandlerPtr;
struct DefaultHttpHandler : IHttpHandler
    {
    private:
        BeMutex m_mutex;
        IHttpHandlerPtr m_handler;

    public:
        DefaultHttpHandler();
        BEHTTP_EXPORT virtual ~DefaultHttpHandler();

        //! Get singleton instance. Will initialize on first call - thread safe.
        BEHTTP_EXPORT static DefaultHttpHandlerPtr GetInstance();

        //! Perform HttpRequest and receive HttpResponse
        BEHTTP_EXPORT virtual Tasks::AsyncTaskPtr<Response> _PerformRequest(const Request& request) override;

        //! Internal - use HttpClient API instead. Enable/Disable implementation. 
        void SetEnabled(bool enabled);
        //! Internal. Get implementation handler.
        IHttpHandlerPtr GetInternalHandler() const;
    };

END_BENTLEY_HTTP_NAMESPACE
