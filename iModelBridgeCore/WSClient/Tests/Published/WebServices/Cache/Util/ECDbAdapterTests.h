/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Util/ECDbAdapterTests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../CachingTestsHelper.h"

class ECDbAdapterTests : public WSClientBaseTest
    {
    private:
        static std::shared_ptr<ObservableECDb> s_readonlyReusableDb;

    protected:
        std::shared_ptr<ObservableECDb> CreateTestDb(ECSchemaPtr schema = GetTestSchema());
        std::shared_ptr<ObservableECDb> GetTestDbReusableReadonly();
        static ECSchemaPtr GetTestSchema();
    };
