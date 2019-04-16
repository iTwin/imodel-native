/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "Helpers.h"
#include <WebServices/iModelHub/Common.h>
#include <WebServices/Client/WSRepositoryClient.h>

namespace BackdoorAPISchema
    {
    namespace Class
        {
        static Utf8CP LRPJob = "BackdoorAPILRPJob";
        }
    namespace Property
        {
        static Utf8CP LRPJobId = "JobId";
        static Utf8CP LRPiModelId = "iModelId";
        static Utf8CP LRPJobState = "LRPJobState";
        }
    }

enum LRPJobState
    {
    Scheduled = 0,
    InProgress = 1,
    Completed = 2,
    Failed = 3
    };
    
USING_NAMESPACE_BENTLEY_WEBSERVICES

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
namespace LRPJobBackdoorAPI
    {
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                             Robertas.Maleckas      11/17
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String ScheduleLRPJob (IWSRepositoryClientPtr projectConnection, Utf8StringCR jobId, Utf8StringCR iModelId);

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                             Robertas.Maleckas      11/17
    +---------------+---------------+---------------+---------------+---------------+------*/
    int QueryLRPJobState (IWSRepositoryClientPtr projectConnection, Utf8StringCR lrpJobRecordId);

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                             Robertas.Maleckas      11/17
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool WaitForLRPJobToFinish (IWSRepositoryClientPtr projectConnection, Utf8StringCR lrpJobRecordId);
    }
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
