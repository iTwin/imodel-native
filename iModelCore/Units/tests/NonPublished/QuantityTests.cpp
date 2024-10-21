/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/QuantityTestFixture.h"
#include <cfloat>
#include <iostream>

BEGIN_UNITS_UNITTESTS_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, Conversion)
    {
    UnitCP mphUnit = s_unitsContext->LookupUnit("MPH");
    UnitCP msecUnit = s_unitsContext->LookupUnit("M/SEC");
    UnitCP newtUnit = s_unitsContext->LookupUnit("N");
    Quantity q1 = Quantity(22.7, *mphUnit);
    Quantity q2 = q1.ConvertTo(msecUnit);
    Quantity q3 = q1.ConvertTo(newtUnit);

    QuantityEquality(q1, q2);
    EXPECT_TRUE(q3.IsNullQuantity());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, InvalidAddition)
    {
    UnitCP mphUnit = s_unitsContext->LookupUnit("MPH");
    UnitCP newtUnit = s_unitsContext->LookupUnit("N");
    Quantity q1 = Quantity(22.7, *mphUnit);
    Quantity q2 = Quantity(13.4112, *newtUnit);

    Quantity ans = q1.Add(q2);
    EXPECT_TRUE(ans.IsNullQuantity());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, InvalidSubtract)
    {
    UnitCP mphUnit = s_unitsContext->LookupUnit("MPH");
    UnitCP newtUnit = s_unitsContext->LookupUnit("N");
    Quantity q1 = Quantity(22.7, *mphUnit);
    Quantity q2 = Quantity(13.4112, *newtUnit);

    Quantity ans = q1.Subtract(q2);
    EXPECT_TRUE(ans.IsNullQuantity());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, SimpleAddition)
    {
    UnitCP msecUnit = s_unitsContext->LookupUnit("M/SEC");
    Quantity q1 =  Quantity(7.5, *msecUnit);
    Quantity q2 =  Quantity(10.2, *msecUnit);
    Quantity ans = Quantity(17.7, *msecUnit);

    Quantity q3 = q1.Add(q2);
    QuantityEquality(q3, ans);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, ComplexAddition)
    {
    UnitCP mphUnit = s_unitsContext->LookupUnit("MPH");
    UnitCP msecUnit = s_unitsContext->LookupUnit("M/SEC");
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, SimpleSubtraction)
    {
    UnitCP msecUnit = s_unitsContext->LookupUnit("M/SEC");
    Quantity q1 = Quantity(7.5, *msecUnit);
    Quantity q2 = Quantity(10.2, *msecUnit);
    Quantity ans = Quantity(2.7, *msecUnit);

    Quantity q3 = q2.Subtract(q1);
    QuantityEquality(ans, q3);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, ComplexSubtraction)
    {
    UnitCP mphUnit = s_unitsContext->LookupUnit("MPH");
    UnitCP msecUnit = s_unitsContext->LookupUnit("M/SEC");
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, SimpleMultiplication)
    {
    UnitCP metrUnit = s_unitsContext->LookupUnit("M");
    UnitCP newtUnit = s_unitsContext->LookupUnit("N");
    UnitCP joultUnit = s_unitsContext->LookupUnit("J");
    Quantity a = Quantity(2.5, *newtUnit);
    Quantity b = Quantity(2.0, *metrUnit);
    Quantity c = Quantity(5.0, *joultUnit);

    Quantity result = a.Multiply(b);
    QuantityEquality(c, result);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, SimpleDivision)
    {
    UnitCP metrUnit = s_unitsContext->LookupUnit("M");
    UnitCP newtUnit = s_unitsContext->LookupUnit("N");
    UnitCP joultUnit = s_unitsContext->LookupUnit("J");
    Quantity a = Quantity(5.0, *joultUnit);
    Quantity b = Quantity(2.0, *newtUnit);
    Quantity c = Quantity(2.5, *metrUnit);

    Quantity result = a.Divide(b);
    QuantityEquality(c, result);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, QuantityComparison)
    {
    UnitCP msecUnit = s_unitsContext->LookupUnit("M/SEC");
    UnitCP mphUnit = s_unitsContext->LookupUnit("MPH");
    Quantity a = Quantity(1.0, *msecUnit);
    double delta = 1.0 + 5.0*a.GetTolerance()*std::numeric_limits<double>::epsilon();
    Quantity aprime = Quantity(delta, *msecUnit);

    QuantityEquality(a, a);
    QuantityGreater(aprime, a);

    Quantity bprime = aprime.ConvertTo(mphUnit);
    QuantityGreater(bprime, a);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, InvertedUnitCornerCases)
    {
    UnitCP vph = s_unitsContext->LookupUnit("VERTICAL_PER_HORIZONTAL");
    UnitCP hpv = s_unitsContext->LookupUnit("HORIZONTAL_PER_VERTICAL");

    { // 1 in both directions
    Quantity a = Quantity(1.0, *vph);

    Quantity b = a.ConvertTo(hpv);
    ASSERT_TRUE(b.IsValid());
    ASSERT_EQ(1.0, b.GetMagnitude());

    Quantity c = b.ConvertTo(vph);
    ASSERT_TRUE(c.IsValid());
    ASSERT_EQ(1.0, c.GetMagnitude());
    }

    { // invalid unit in both directions
    UnitCP meters = s_unitsContext->LookupUnit("M");
    Quantity a = Quantity(1.0, *vph);
    Quantity b = a.ConvertTo(meters);
    ASSERT_FALSE(b.IsValid());
    ASSERT_EQ(UnitsProblemCode::UncomparableUnits, b.GetProblemCode());
    
    Quantity c = Quantity(1.0, *hpv);
    Quantity d = c.ConvertTo(meters);
    ASSERT_FALSE(d.IsValid());
    ASSERT_EQ(UnitsProblemCode::UncomparableUnits, d.GetProblemCode());

    Quantity e = Quantity(1.0, *meters);
    Quantity f = e.ConvertTo(vph);
    ASSERT_FALSE(f.IsValid());
    ASSERT_EQ(UnitsProblemCode::UncomparableUnits, f.GetProblemCode());
    Quantity g = e.ConvertTo(hpv);
    ASSERT_FALSE(g.IsValid());
    ASSERT_EQ(UnitsProblemCode::UncomparableUnits, g.GetProblemCode());
    }

    { // zero value in both directions
    Quantity a = Quantity(0.0, *vph);
    Quantity b = a.ConvertTo(hpv);
    ASSERT_FALSE(b.IsValid());
    ASSERT_EQ(UnitsProblemCode::InvertingZero, b.GetProblemCode());

    Quantity c = Quantity(0.0, *hpv);
    Quantity d = c.ConvertTo(vph);
    ASSERT_FALSE(d.IsValid());
    ASSERT_EQ(UnitsProblemCode::InvertingZero, d.GetProblemCode());
    }

    { // NaN value in both directions
    Quantity a = Quantity(std::numeric_limits<double>::quiet_NaN(), *vph);
    Quantity b = a.ConvertTo(hpv);
    ASSERT_FALSE(b.IsValid());
    ASSERT_EQ(UnitsProblemCode::NaN, b.GetProblemCode());

    Quantity c = Quantity(std::numeric_limits<double>::quiet_NaN(), *hpv);
    Quantity d = c.ConvertTo(vph);
    ASSERT_FALSE(d.IsValid());
    ASSERT_EQ(UnitsProblemCode::NaN, d.GetProblemCode());
    }

    { // Infinity value in both directions
    Quantity a = Quantity(std::numeric_limits<double>::infinity(), *vph);
    Quantity b = a.ConvertTo(hpv);
    ASSERT_TRUE(b.IsValid());
    ASSERT_EQ(0.0, b.GetMagnitude());

    Quantity c = Quantity(std::numeric_limits<double>::infinity(), *hpv);
    Quantity d = c.ConvertTo(vph);
    ASSERT_TRUE(d.IsValid());
    ASSERT_EQ(0.0, d.GetMagnitude());
    }
    }

END_UNITS_UNITTESTS_NAMESPACE
