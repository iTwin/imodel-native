/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Network/ResponseGuard.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/WebServicesCache.h>
#include <BeHttp/HttpRequest.h>
#include <Bentley/CancellationToken.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ResponseGuard> ResponseGuardPtr;

struct ResponseGuard : public ICancellationToken, std::enable_shared_from_this<ResponseGuard>
    {
    private:
        bool                            m_tokenEnabled;
        ICancellationTokenPtr           m_token;
        Http::Request::ProgressCallback m_onProgress;

    public:
        ResponseGuard(ICancellationTokenPtr tokenToWrap, Http::Request::ProgressCallbackCR onProgress);
        virtual ~ResponseGuard() {};

        static ResponseGuardPtr Create(ICancellationTokenPtr tokenToWrap, Http::Request::ProgressCallbackCR onProgress);
        Http::Request::ProgressCallback GetProgressCallback();

        virtual bool IsCanceled() override;
        virtual void Register(std::weak_ptr<ICancellationListener> listener) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
