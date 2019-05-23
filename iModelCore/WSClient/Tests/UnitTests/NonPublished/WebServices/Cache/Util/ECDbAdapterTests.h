/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../CachingTestsHelper.h"

class ECDbAdapterTests : public WSClientBaseTest
    {
    private:
        static SeedFile s_seedECDb;
        static SeedFile s_seedEmptyECDb;

    protected:
        static std::shared_ptr<ObservableECDb> CreateTestDb(ECSchemaPtr schema);
        static std::shared_ptr<ObservableECDb> GetTestDb();
        static std::shared_ptr<ObservableECDb> GetEmptyTestDb();
    };
