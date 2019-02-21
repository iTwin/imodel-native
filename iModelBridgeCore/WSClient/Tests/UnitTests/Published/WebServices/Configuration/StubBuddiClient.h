/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Configuration/StubBuddiClient.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Configuration/BuddiClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Julija.Semenenko    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubBuddiClient : public IBuddiClient
    {
    public:
        bvector<BuddiRegion> regions;
        Utf8String url = "https://test/foo";

    public:
        AsyncTaskPtr<BuddiRegionsResult> GetRegions() override
            {
            return CreateCompletedAsyncTask(BuddiRegionsResult::Success(regions));
            }

        AsyncTaskPtr<BuddiUrlResult> GetUrl(Utf8StringCR urlName, uint32_t regionId = 0) override
            {
            return CreateCompletedAsyncTask(BuddiUrlResult::Success(url));
            }
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
