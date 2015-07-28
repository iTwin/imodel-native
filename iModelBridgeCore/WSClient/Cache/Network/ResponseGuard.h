/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Network/ResponseGuard.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/WebServicesCache.h>
#include <MobileDgn/Utils/Http/HttpRequest.h>
#include <MobileDgn/Utils/Threading/CancellationToken.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ResponseGuard> ResponseGuardPtr;

struct ResponseGuard : public ICancellationToken, std::enable_shared_from_this<ResponseGuard>
    {
    private:
        bool                            m_tokenEnabled;
        ICancellationTokenPtr           m_token;
        HttpRequest::ProgressCallback   m_onProgress;

    public:
        ResponseGuard(ICancellationTokenPtr tokenToWrap, HttpRequest::ProgressCallbackCR onProgress);
        virtual ~ResponseGuard()
            {};

        static ResponseGuardPtr Create(ICancellationTokenPtr tokenToWrap, HttpRequest::ProgressCallbackCR onProgress);
        HttpRequest::ProgressCallback GetProgressCallback();

        virtual bool IsCanceled() override;
        virtual void Register(std::weak_ptr<ICancellationListener> listener) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
