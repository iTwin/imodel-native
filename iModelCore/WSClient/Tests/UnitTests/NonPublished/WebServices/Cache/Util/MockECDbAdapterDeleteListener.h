/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/Util/ECDbAdapter.h>

#ifdef USE_GTEST
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockECDbAdapterDeleteListener : public IECDbAdapter::DeleteListener
    {
    public:
        MOCK_METHOD3(OnBeforeDelete, BentleyStatus(ECClassCR ecClass, ECInstanceId ecInstanceId, bset<ECInstanceKey>& additionalToDeleteOut));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif
