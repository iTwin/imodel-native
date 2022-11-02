/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "QuantityTestFixture.h"

BEGIN_UNITS_UNITTESTS_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
// static
void QuantityTestFixture::QuantityEquality(QuantityCR q1, QuantityCR q2)
    {
    EXPECT_TRUE(q1.AlmostEqual(q2));
    EXPECT_FALSE(q1.AlmostGreaterThan(q2));
    EXPECT_FALSE(q1.AlmostLessThan(q2));
    EXPECT_TRUE(q1.AlmostGreaterThanOrEqual(q2));
    EXPECT_TRUE(q1.AlmostLessThanOrEqual(q2));
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
// static
void QuantityTestFixture::QuantityGreater(QuantityCR q1, QuantityCR q2)
    {
    EXPECT_FALSE(q1.AlmostEqual(q2));
    EXPECT_TRUE(q1.AlmostGreaterThan(q2));
    EXPECT_FALSE(q1.AlmostLessThan(q2));
    EXPECT_TRUE(q1.AlmostGreaterThanOrEqual(q2));
    EXPECT_FALSE(q1.AlmostLessThanOrEqual(q2));
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
// static
void QuantityTestFixture::QuantityGreaterEqual(QuantityCR q1, QuantityCR q2)
    {
    EXPECT_FALSE(q1.AlmostLessThan(q2));
    EXPECT_TRUE(q1.AlmostGreaterThanOrEqual(q2));
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
// static
void QuantityTestFixture::QuantityLess(QuantityCR q1, QuantityCR q2)
    {
    EXPECT_FALSE(q1.AlmostEqual(q2));
    EXPECT_FALSE(q1.AlmostGreaterThan(q2));
    EXPECT_TRUE(q1.AlmostLessThan(q2));
    EXPECT_FALSE(q1.AlmostGreaterThanOrEqual(q2));
    EXPECT_TRUE(q1.AlmostLessThanOrEqual(q2));
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
// static
void QuantityTestFixture::QuantityLessEqual(QuantityCR q1, QuantityCR q2)
    {
    EXPECT_FALSE(q1.AlmostGreaterThan(q2));
    EXPECT_TRUE(q1.AlmostLessThanOrEqual(q2));
    }

END_UNITS_UNITTESTS_NAMESPACE