/*--------------------------------------------------------------------------------------+
|
|     $Source: BimFromDgnDb/Tests/BimUpgraderTestFixture.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include "Tests.h"

#define ASSERT_SUCCESS(val) ASSERT_TRUE (SUCCESS == val)
#define EXPECT_SUCCESS(val) EXPECT_TRUE (SUCCESS == val)

BEGIN_BIM_UPGRADER_TEST_NAMESPACE

struct BimUpgraderTestFixture : public testing::Test
    {
    public:
        BentleyApi::BeFileName GetOutputDir();
        BentleyApi::BeFileName GetOutputFileName(BentleyApi::WCharCP filename);
        BentleyApi::BeFileName GetOutputFileName(BentleyApi::Utf8CP filename);

        DgnDbPtr OpenExistingDgnDb(BentleyApi::BeFileNameCR projectName, DgnDb::OpenMode mode = DgnDb::OpenMode::ReadWrite);

    };

END_BIM_UPGRADER_TEST_NAMESPACE