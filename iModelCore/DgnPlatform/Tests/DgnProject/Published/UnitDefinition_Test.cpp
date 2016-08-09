/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/UnitDefinition_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>

USING_NAMESPACE_BENTLEY_DGN

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Seeds are defaults - You can add more seeds for testing - never change seeds!
// It should be noted that these should be placed in the test functions to make it obvious the comparisons.
// However, copy and paste is bad.  This is an example of ObjectMother. Builder Pattern should be considered if your
// class is larger.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace UnitDefinitionSeeds
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition GenerateMeter_1_10 ()
    {
    return UnitDefinition (UnitBase::Meter, UnitSystem::Metric, 1.0, 10.0, "UnitDefinitionSeed1");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition GenerateDegree_1_10 ()
    {
    return UnitDefinition (UnitBase::Degree, UnitSystem::English, 1.0, 10.0, "UnitDefinitionSeed2");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition GenerateMeter_10_10 ()
    {
    return UnitDefinition (UnitBase::Meter, UnitSystem::Metric, 10.0, 10.0, "UnitDefinitionSeed3");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition GenerateMeter_1Tenth_10 ()
    {
    return UnitDefinition (UnitBase::Meter, UnitSystem::Metric, 0.10, 10.0, "UnitDefinitionSeed4");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition GenerateMeter_0_0 ()
    {
    return UnitDefinition (UnitBase::Meter, UnitSystem::Metric, 0.0, 0.0, "UnitDefinitionSeed5");
    }

};

using namespace UnitDefinitionSeeds;

//=======================================================================================
// @bsiclass                                                    Umar.Hayat     11/2015
//=======================================================================================
struct UnitDefinitionTest : public ::testing::Test
{
    ScopedDgnHost autoDgnHost;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, DefaultConstructor)
    {
    UnitDefinition def;

    ASSERT_FALSE (def.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, IsEqual_True)
    {
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition same = GenerateMeter_1_10();

    ASSERT_TRUE (same.IsEqual (s1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, IsEqual_False)
    {
    UnitDefinition s2 = GenerateDegree_1_10();

    ASSERT_FALSE (GenerateMeter_1_10().IsEqual (s2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, AreComparable_True)
    {
    UnitDefinition s3 = GenerateMeter_10_10();

    ASSERT_TRUE (GenerateMeter_1_10().AreComparable (s3));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, AreComparable_False)
    {
    UnitDefinition s2 = GenerateDegree_1_10();

    ASSERT_FALSE (GenerateMeter_1_10().AreComparable (s2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, AreComparable_False2)
    {
    UnitDefinition s5 = GenerateMeter_0_0();

    ASSERT_FALSE (GenerateMeter_1_10().AreComparable (s5));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, IsValid_True)
    {
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition s2 = GenerateDegree_1_10();
    UnitDefinition s3 = GenerateMeter_10_10();

    ASSERT_TRUE (s1.IsValid() && s2.IsValid() && s3.IsValid() );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, IsValid_False)
    {
    ASSERT_FALSE (GenerateMeter_0_0().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, CompareByScale_Error)
    {
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition s2 = GenerateDegree_1_10();

    ASSERT_TRUE (BentleyApi::ERROR == s1.CompareByScale (s2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, CompareByScale_Neg1)
    {
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition s3 = GenerateMeter_10_10();

    ASSERT_EQ (-1, s1.CompareByScale (s3));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, CompareByScale_0)
    {
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition s  = GenerateMeter_1_10();

    ASSERT_EQ (0, s1.CompareByScale (s));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, CompareByScale_1)
    {
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition s4 = GenerateMeter_1Tenth_10();

    ASSERT_EQ (1, s1.CompareByScale (s4));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, GetConversionFactorFrom)
    {
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition s3 = GenerateMeter_10_10();
    double result = s3.GetConversionFactorFrom(s1);

    // s3 is 10 times bigger than s1
    EXPECT_NEAR (10.0, result, 0.0000001);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, ConvertDistanceFrom)
    {
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition s3 = GenerateMeter_10_10();

    double result = s3.ConvertDistanceFrom(5.0, s1);
    // s3 conversion distance should be 50
    EXPECT_NEAR (50.0, result, 0.0000001);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitDefinitionTest, AssignLabel)
    {
    Utf8CP label = "TestLabel";
    UnitDefinition def;
    def.SetLabel(label);

    ASSERT_TRUE (def.GetLabel() == label);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik  08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitDefinitionTest, operator_equal)
    {
    UnitDefinition s3 = GenerateMeter_1_10();
    UnitDefinition s4(s3);
    ASSERT_TRUE(s4.operator == (s3) );
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik  08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitDefinitionTest, operator_notequal)
    {
    UnitDefinition s2 = GenerateDegree_1_10();
    ASSERT_TRUE(GenerateMeter_1_10().operator != (s2));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik  08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitDefinitionTest,BaseFromInt)
    {
    ASSERT_TRUE(UnitBase::Degree == UnitDefinition::BaseFromInt(2));
    ASSERT_TRUE(UnitBase::None == UnitDefinition::BaseFromInt(5));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ridha.Malik  08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitDefinitionTest, SystemFromInt)
    {
    ASSERT_TRUE(UnitSystem::Metric == UnitDefinition::SystemFromInt(2));
    ASSERT_TRUE(UnitSystem::Undefined == UnitDefinition::SystemFromInt(5));
    }