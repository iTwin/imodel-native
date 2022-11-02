/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "UnitsTestFixture.h"

BEGIN_UNITS_UNITTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QuantityTestFixture : UnitsTestFixture
    {
    protected:
        static void QuantityEquality(QuantityCR q1, QuantityCR q2);
        static void QuantityGreater(QuantityCR q1, QuantityCR q2);
        static void QuantityGreaterEqual(QuantityCR q1, QuantityCR q2);
        static void QuantityLess(QuantityCR q1, QuantityCR q2);
        static void QuantityLessEqual(QuantityCR q1, QuantityCR q2);
    };

END_UNITS_UNITTESTS_NAMESPACE