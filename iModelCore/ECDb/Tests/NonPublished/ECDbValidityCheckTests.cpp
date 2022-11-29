/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECDbValidityCheckTests : ECDbTestFixture
    {

    };

TEST_F(ECDbValidityCheckTests, FirstTest) {
    ASSERT_EQ(true, true);
}

END_ECDBUNITTESTS_NAMESPACE