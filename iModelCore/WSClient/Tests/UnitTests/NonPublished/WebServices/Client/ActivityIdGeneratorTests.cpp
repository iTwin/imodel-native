/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "ActivityIdGeneratorTests.h"

#include "../../../../../Client/ActivityIdGenerator.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityIdGeneratorTests, GenerateNextId_CalledOnce_GeneratesIdWithExpectedFormat)
    {
    ActivityIdGenerator activityIdGenerator;

    Utf8String activityId = activityIdGenerator.GenerateNextId();

    EXPECT_FALSE(activityId.empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ActivityIdGeneratorTests, GenerateNextId_CalledMultipleTimes_GeneratesUniqueId)
    {
    ActivityIdGenerator activityIdGenerator;
    std::set<Utf8String> activityIds;
    size_t callCount = 2;

    for(size_t i = 0; i < callCount; i++)
        activityIds.insert(activityIdGenerator.GenerateNextId());

    EXPECT_EQ(callCount, activityIds.size());
    }