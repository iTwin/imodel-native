/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerUtils.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnDbServer/DgnDbServerCommon.h>
#include <MobileDgn/Utils/Http/HttpRequest.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
struct CallbackQueue
    {
    private:
        void Notify();
        struct Callback
            {
            MobileDgn::Utils::HttpRequest::ProgressCallback callback;
            CallbackQueue& m_queue;
            double m_bytesTransfered;
            double m_bytesTotal;
            Callback(CallbackQueue& queue);
            };
        friend struct CallbackQueue::Callback;
        bvector<std::shared_ptr<CallbackQueue::Callback>> m_callbacks;
        MobileDgn::Utils::HttpRequest::ProgressCallbackCR m_callback;
    public:
        CallbackQueue(MobileDgn::Utils::HttpRequest::ProgressCallbackCR callback);

        MobileDgn::Utils::HttpRequest::ProgressCallbackCR NewCallback();
    };
END_BENTLEY_DGNDBSERVER_NAMESPACE
