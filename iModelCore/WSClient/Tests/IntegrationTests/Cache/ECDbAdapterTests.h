/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Cache/ECDbAdapterTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServicesTestsHelper.h>

class ECDbAdapterTests : public WSClientBaseTest
    {
    protected:
        static std::shared_ptr<ObservableECDb> CreateTestDb(ECSchemaPtr schema);
    };
