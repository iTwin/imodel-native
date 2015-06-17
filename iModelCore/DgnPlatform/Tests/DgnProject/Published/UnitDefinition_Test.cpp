/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/UnitDefinition_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnHandlers/ScopedDgnHost.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

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
    return UnitDefinition (UnitBase::Meter, UnitSystem::Metric, 1.0, 10.0, L"UnitDefinitionSeed1");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition GenerateDegree_1_10 ()
    {
    return UnitDefinition (UnitBase::Degree, UnitSystem::English, 1.0, 10.0, L"UnitDefinitionSeed2");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition GenerateMeter_10_10 ()
    {
    return UnitDefinition (UnitBase::Meter, UnitSystem::Metric, 10.0, 10.0, L"UnitDefinitionSeed3");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition GenerateMeter_1Tenth_10 ()
    {
    return UnitDefinition (UnitBase::Meter, UnitSystem::Metric, 0.10, 10.0, L"UnitDefinitionSeed4");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition GenerateMeter_0_0 ()
    {
    return UnitDefinition (UnitBase::Meter, UnitSystem::Metric, 0.0, 0.0, L"UnitDefinitionSeed5");
    }

};

using namespace UnitDefinitionSeeds;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, DefaultConstructor)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition def;

    ASSERT_FALSE (def.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, IsEqual_True)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition same = GenerateMeter_1_10();

    ASSERT_TRUE (same.IsEqual (s1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, IsEqual_False)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition s2 = GenerateDegree_1_10();

    ASSERT_FALSE (GenerateMeter_1_10().IsEqual (s2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, AreComparable_True)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition s3 = GenerateMeter_10_10();

    ASSERT_TRUE (GenerateMeter_1_10().AreComparable (s3));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, AreComparable_False)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition s2 = GenerateDegree_1_10();

    ASSERT_FALSE (GenerateMeter_1_10().AreComparable (s2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, AreComparable_False2)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition s5 = GenerateMeter_0_0();

    ASSERT_FALSE (GenerateMeter_1_10().AreComparable (s5));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, IsValid_True)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition s2 = GenerateDegree_1_10();
    UnitDefinition s3 = GenerateMeter_10_10();

    ASSERT_TRUE (s1.IsValid() && s2.IsValid() && s3.IsValid() );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, IsValid_False)
    {
    ScopedDgnHost autoDgnHost;
    ASSERT_FALSE (GenerateMeter_0_0().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, CompareByScale_Error)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition s2 = GenerateDegree_1_10();

    ASSERT_TRUE (BentleyApi::ERROR == s1.CompareByScale (s2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, CompareByScale_Neg1)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition s3 = GenerateMeter_10_10();

    ASSERT_EQ (-1, s1.CompareByScale (s3));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, CompareByScale_0)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition s  = GenerateMeter_1_10();

    ASSERT_EQ (0, s1.CompareByScale (s));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, CompareByScale_1)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition s4 = GenerateMeter_1Tenth_10();

    ASSERT_EQ (1, s1.CompareByScale (s4));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, GetConversionFactorFrom)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition s3 = GenerateMeter_10_10();
    double result = s3.GetConversionFactorFrom(s1);

    // s3 is 10 times bigger than s1
    EXPECT_NEAR (10.0, result, 0.0000001);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, ConvertDistanceFrom)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition s1 = GenerateMeter_1_10();
    UnitDefinition s3 = GenerateMeter_10_10();

    double result = s3.ConvertDistanceFrom(5.0, s1);
    // s3 conversion distance should be 50
    EXPECT_NEAR (50.0, result, 0.0000001);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitDefinition, AssignLabel)
    {
    ScopedDgnHost autoDgnHost;
    const WCharCP label = L"TestLabel";
    UnitDefinition def;
    def.SetLabel(label);

    ASSERT_TRUE (def.GetLabel() == label);
    }
