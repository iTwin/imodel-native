/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/NonPublished/QuantityTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "QuantityTestFixture.h"

BEGIN_UNITS_UNITTESTS_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, Conversion)
    {
    auto q1 = Quantity::Create(22.7, "MPH");
    auto q2 = q1->ConvertTo("M/SEC");
    auto q3 = q1->ConvertTo("N");

    QuantityEquality(*q1, *q2);
    EXPECT_EQ(q3.get(), nullptr);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, InvalidAddition)
    {
    auto q1 = Quantity::Create(22.7, "MPH");
    auto q2 = Quantity::Create(13.4112, "N");

    auto ans = q1->Add(*q2);
    EXPECT_EQ(nullptr, ans.get());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, InvalidSubtract)
    {
    auto q1 = Quantity::Create(22.7, "MPH");
    auto q2 = Quantity::Create(13.4112, "N");

    auto ans = q1->Subtract(*q2);
    EXPECT_EQ(nullptr, ans.get());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, SimpleAddition)
    {
    auto q1 = Quantity::Create(7.5, "M/SEC");
    auto q2 = Quantity::Create(10.2, "M/SEC");
    auto ans = Quantity::Create(17.7, "M/SEC");

    auto q3 = q1->Add(*q2);
    QuantityEquality(*q3, *ans);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, ComplexAddition)
    {
    auto q1 = Quantity::Create(22.7, "MPH");
    auto q2 = Quantity::Create(13.4112, "M/SEC");
    auto ans1 = Quantity::Create(52.7, "MPH");
    auto ans2 = Quantity::Create(23.559008, "M/SEC");

    // Since our units are inconsistent, do both orders.
    // The answer is given in terms of the first term's units.
    auto q3 = q1->Add(*q2);
    QuantityEquality(*q3, *ans1);

    auto q4 = q2->Add(*q1);
    QuantityEquality(*q4, *ans2);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, SimpleSubtraction)
    {
    auto q1 = Quantity::Create(7.5, "M/SEC");
    auto q2 = Quantity::Create(10.2, "M/SEC");
    auto ans = Quantity::Create(2.7, "M/SEC");

    auto q3 = q2->Subtract(*q1);
    QuantityEquality(*ans, *q3);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, ComplexSubtraction)
    {
    auto q1 = Quantity::Create(22.7, "MPH");
    auto q2 = Quantity::Create(13.4112, "M/SEC");
    auto ans1 = Quantity::Create(7.3, "MPH");
    auto ans2 = Quantity::Create(-3.263392, "M/SEC");

    // Since our units are inconsistent, do both orders.
    // The answer is given in terms of the first term's units.
    auto q3 = q2->Subtract(*q1);
    QuantityEquality(*q3, *ans1);

    auto q4 = q1->Subtract(*q2);
    QuantityEquality(*q4, *ans2);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QuantityTestFixture, SimpleMultiplication)
    {
    auto a = Quantity::Create(2.5, "N");
    auto b = Quantity::Create(2.0, "M");
    auto c = Quantity::Create(5.0, "J");

    auto result = a->Multiply(*b);
    QuantityEquality(*c, *result);
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
    auto a = Quantity::Create(5.0, "J"); 
    auto b = Quantity::Create(2.0, "N");
    auto c = Quantity::Create(2.5, "M");

    auto result = a->Divide(*b);
    QuantityEquality(*c, *result);
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
    auto a = Quantity::Create(1.0, "M/SEC");
    auto delta = 1.0 + 5.0*a->GetTolerance()*std::numeric_limits<double>::epsilon();
    auto aprime = Quantity::Create(delta, "M/SEC");

    QuantityEquality(*a, *a);
    QuantityGreater(*aprime, *a);

    auto bprime = aprime->ConvertTo("MPH");
    QuantityGreater(*bprime, *a);
    }

END_UNITS_UNITTESTS_NAMESPACE