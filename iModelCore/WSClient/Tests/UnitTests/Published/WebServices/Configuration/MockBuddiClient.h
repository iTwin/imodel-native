/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Configuration/MockBuddiClient.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Configuration/BuddiClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

using namespace ::testing;

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                  Julija.Semenenko 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockBuddiClient : public IBuddiClient
    {
    public:
        MOCK_METHOD0(GetRegions, AsyncTaskPtr<BuddiRegionsResult>());
        MOCK_METHOD2(GetUrl, AsyncTaskPtr<BuddiUrlResult>(Utf8StringCR urlName, uint32_t regionId));
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
