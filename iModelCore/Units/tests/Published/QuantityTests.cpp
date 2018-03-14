/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/Published/QuantityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/QuantityTestFixture.h"

BEGIN_UNITS_UNITTESTS_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, Conversion)
    {
    UnitCP mphUnit = UnitRegistry::Get().LookupUnit("MPH");
    UnitCP msecUnit = UnitRegistry::Get().LookupUnit("M/SEC");
    UnitCP newtUnit = UnitRegistry::Get().LookupUnit("N");
    Quantity q1 = Quantity(22.7, *mphUnit);
    Quantity q2 = q1.ConvertTo(msecUnit);
    Quantity q3 = q1.ConvertTo(newtUnit);

    QuantityEquality(q1, q2);
    EXPECT_TRUE(q3.IsNullQuantity());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, InvalidAddition)
    {
    UnitCP mphUnit = UnitRegistry::Get().LookupUnit("MPH");
    UnitCP newtUnit = UnitRegistry::Get().LookupUnit("N");
    Quantity q1 = Quantity(22.7, *mphUnit);
    Quantity q2 = Quantity(13.4112, *newtUnit);

    Quantity ans = q1.Add(q2);
    EXPECT_TRUE(ans.IsNullQuantity());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, InvalidSubtract)
    {
    UnitCP mphUnit = UnitRegistry::Get().LookupUnit("MPH");
    UnitCP newtUnit = UnitRegistry::Get().LookupUnit("N");
    Quantity q1 = Quantity(22.7, *mphUnit);
    Quantity q2 = Quantity(13.4112, *newtUnit);

    Quantity ans = q1.Subtract(q2);
    EXPECT_TRUE(ans.IsNullQuantity());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, SimpleAddition)
    {
    UnitCP msecUnit = UnitRegistry::Get().LookupUnit("M/SEC");
    Quantity q1 =  Quantity(7.5, *msecUnit);
    Quantity q2 =  Quantity(10.2, *msecUnit);
    Quantity ans = Quantity(17.7, *msecUnit);

    Quantity q3 = q1.Add(q2);
    QuantityEquality(q3, ans);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, ComplexAddition)
    {
    UnitCP mphUnit = UnitRegistry::Get().LookupUnit("MPH");
    UnitCP msecUnit = UnitRegistry::Get().LookupUnit("M/SEC");
    Quantity q1 = Quantity(22.7, *mphUnit);
    Quantity q2 = Quantity(13.4112, *msecUnit);
    Quantity ans1 = Quantity(52.7, *mphUnit);
    Quantity ans2 = Quantity(23.559008, *msecUnit);

    // Since our units are inconsistent, do both orders.
    // The answer is given in terms of the first term's units.
    Quantity q3 = q1.Add(q2);
    QuantityEquality(q3, ans1);

    Quantity q4 = q2.Add(q1);
    QuantityEquality(q4, ans2);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, SimpleSubtraction)
    {
    UnitCP msecUnit = UnitRegistry::Get().LookupUnit("M/SEC");
    Quantity q1 = Quantity(7.5, *msecUnit);
    Quantity q2 = Quantity(10.2, *msecUnit);
    Quantity ans = Quantity(2.7, *msecUnit);

    Quantity q3 = q2.Subtract(q1);
    QuantityEquality(ans, q3);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, ComplexSubtraction)
    {
    UnitCP mphUnit = UnitRegistry::Get().LookupUnit("MPH");
    UnitCP msecUnit = UnitRegistry::Get().LookupUnit("M/SEC");
    Quantity q1 = Quantity(22.7, *mphUnit);
    Quantity q2 = Quantity(13.4112, *msecUnit);
    Quantity ans1 = Quantity(7.3, *mphUnit);
    Quantity ans2 = Quantity(-3.263392, *msecUnit);

    // Since our units are inconsistent, do both orders.
    // The answer is given in terms of the first term's units.
    Quantity q3 = q2.Subtract(q1);
    QuantityEquality(q3, ans1);

    Quantity q4 = q1.Subtract(q2);
    QuantityEquality(q4, ans2);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, SimpleMultiplication)
    {
    UnitCP metrUnit = UnitRegistry::Get().LookupUnit("M");
    UnitCP newtUnit = UnitRegistry::Get().LookupUnit("N");
    UnitCP joultUnit = UnitRegistry::Get().LookupUnit("J");
    Quantity a = Quantity(2.5, *newtUnit);
    Quantity b = Quantity(2.0, *metrUnit);
    Quantity c = Quantity(5.0, *joultUnit);

    Quantity result = a.Multiply(b);
    QuantityEquality(c, result);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, ComplexMultiplication)
    {
    EXPECT_TRUE(true);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, SimpleDivision)
    {
    UnitCP metrUnit = UnitRegistry::Get().LookupUnit("M");
    UnitCP newtUnit = UnitRegistry::Get().LookupUnit("N");
    UnitCP joultUnit = UnitRegistry::Get().LookupUnit("J");
    Quantity a = Quantity(5.0, *joultUnit);
    Quantity b = Quantity(2.0, *newtUnit);
    Quantity c = Quantity(2.5, *metrUnit);

    Quantity result = a.Divide(b);
    QuantityEquality(c, result);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, ComplexDivision)
    {
    EXPECT_TRUE(true);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, QuantityComparison)
    {
    UnitCP msecUnit = UnitRegistry::Get().LookupUnit("M/SEC");
    UnitCP mphUnit = UnitRegistry::Get().LookupUnit("MPH");
    Quantity a = Quantity(1.0, *msecUnit);
    double delta = 1.0 + 5.0*a.GetTolerance()*std::numeric_limits<double>::epsilon();
    Quantity aprime = Quantity(delta, *msecUnit);

    QuantityEquality(a, a);
    QuantityGreater(aprime, a);

    Quantity bprime = aprime.ConvertTo(mphUnit);
    QuantityGreater(bprime, a);
    }

END_UNITS_UNITTESTS_NAMESPACE
