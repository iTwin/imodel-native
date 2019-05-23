/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../CachingTestsHelper.h"

class ExtendedDataAdapterTests : public WSClientBaseTest
    {
    private:
        static SeedFile s_seedECDb;

    public:
        std::shared_ptr<ObservableECDb> GetTestECDb();
    };
