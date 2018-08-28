/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Helpers/TestsProgressCallback.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "../Common.h"
#include <BeHttp/HttpRequest.h>

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
struct TestsProgressCallback
    {
    protected:
        double m_lastProgressBytesTransfered = 0.0;
        double m_lastProgressBytesTotal = 0.0;
        int m_progressRetryCount = 0;

    public:
        virtual Http::Request::ProgressCallback Get();
        void Verify(bool completed = true);
    };
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
