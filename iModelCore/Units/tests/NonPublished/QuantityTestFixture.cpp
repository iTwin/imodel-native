/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/NonPublished/QuantityTestFixture.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QuantityTestFixture.h"

BEGIN_UNITS_UNITTESTS_NAMESPACE

void QuantityTestFixture::QuantityEquality(QuantityCR q1, QuantityCR q2) const
    {
    EXPECT_TRUE(q1.AlmostEqual(q2));
    EXPECT_FALSE(q1.AlmostGreaterThan(q2));
    EXPECT_FALSE(q1.AlmostLessThan(q2));
    EXPECT_TRUE(q1.AlmostGreaterThanOrEqual(q2));
    EXPECT_TRUE(q1.AlmostLessThanOrEqual(q2));
    }

void QuantityTestFixture::QuantityGreater(QuantityCR q1, QuantityCR q2) const
    {
    EXPECT_FALSE(q1.AlmostEqual(q2));
    EXPECT_TRUE(q1.AlmostGreaterThan(q2));
    EXPECT_FALSE(q1.AlmostLessThan(q2));
    EXPECT_TRUE(q1.AlmostGreaterThanOrEqual(q2));
    EXPECT_FALSE(q1.AlmostLessThanOrEqual(q2));
    }

void QuantityTestFixture::QuantityGreaterEqual(QuantityCR q1, QuantityCR q2) const
    {
    EXPECT_FALSE(q1.AlmostLessThan(q2));
    EXPECT_TRUE(q1.AlmostGreaterThanOrEqual(q2));
    }

void QuantityTestFixture::QuantityLess(QuantityCR q1, QuantityCR q2) const
    {
    EXPECT_FALSE(q1.AlmostEqual(q2));
    EXPECT_FALSE(q1.AlmostGreaterThan(q2));
    EXPECT_TRUE(q1.AlmostLessThan(q2));
    EXPECT_FALSE(q1.AlmostGreaterThanOrEqual(q2));
    EXPECT_TRUE(q1.AlmostLessThanOrEqual(q2));
    }

void QuantityTestFixture::QuantityLessEqual(QuantityCR q1, QuantityCR q2) const
    {
    EXPECT_FALSE(q1.AlmostGreaterThan(q2));
    EXPECT_TRUE(q1.AlmostLessThanOrEqual(q2));
    }

END_UNITS_UNITTESTS_NAMESPACE