/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BeNumerical_Test.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeNumerical.h>

static void compareToTolerance (double v)
    {
    double vTol = BeNumerical::ComputeComparisonTolerance (v, v);
    ASSERT_TRUE( BeNumerical::Compare (v, v+vTol)       == -1 );
    ASSERT_TRUE( BeNumerical::Compare (v, v+vTol/10)    ==  0 );
    ASSERT_TRUE( BeNumerical::Compare (v, v)            ==  0 );
    ASSERT_TRUE( BeNumerical::Compare (v, v-vTol/10)    ==  0 );
    ASSERT_TRUE( BeNumerical::Compare (v, v-vTol)       ==  1 );
    }

TEST(BeNumerical,Test1)
    {
    ASSERT_TRUE( BeNumerical::Compare ( 1.0,     0.0) ==  1 );
    ASSERT_TRUE( BeNumerical::Compare ( 1.0e-13, 0.0) ==  1 );
    ASSERT_TRUE( BeNumerical::Compare (0.0,      0.0) ==  0 );
    ASSERT_TRUE( BeNumerical::Compare (-1.0e-13, 0.0) == -1 );
    ASSERT_TRUE( BeNumerical::Compare (-1.0,     0.0) == -1 );

    compareToTolerance (0.0);
    compareToTolerance (1.0e-13);
    compareToTolerance (0.5);
    compareToTolerance (0.51);
    compareToTolerance (1.0);
    compareToTolerance (-1.0);
    compareToTolerance (1.0e9);
    compareToTolerance (-1.0e9);
    compareToTolerance (1.0e17);

    ASSERT_TRUE( BeNumerical::ComputeComparisonTolerance(1.0e9,1.0e9) > BeNumerical::ComputeComparisonTolerance(1.0,1.0));
    }