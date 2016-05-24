/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/NonPublished/QuantityTestFixture.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "UnitsTests.h"

BEGIN_UNITS_UNITTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     		Chris.Tartamella 02/2016
+===============+===============+===============+===============+===============+======*/
struct QuantityTestFixture : public ::testing::Test
    {
    protected:
        void QuantityEquality(QuantityCR q1, QuantityCR q2) const;
        void QuantityGreater(QuantityCR q1, QuantityCR q2) const;
        void QuantityGreaterEqual(QuantityCR q1, QuantityCR q2) const;
        void QuantityLess(QuantityCR q1, QuantityCR q2) const;
        void QuantityLessEqual(QuantityCR q1, QuantityCR q2) const;

    public:
        QuantityTestFixture() : ::testing::Test() {}
        virtual ~QuantityTestFixture() {}

        virtual void SetUp() override {}
        virtual void TearDown() override {}
    };

END_UNITS_UNITTESTS_NAMESPACE