/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/EllipseManipulationStrategyTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct EllipseManipulationStrategyTests : public BuildingSharedTestFixtureBase
    {};

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(EllipseManipulationStrategyTests, AppendKeyPoint_AccepsAMaximumOf4KeyPoints)
    {
    GeometryManipulationStrategyPtr sut = EllipseManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    sut->AppendKeyPoint({-2,0,0});
    sut->AppendKeyPoint({0,0,0});
    sut->AppendKeyPoint({0,1,0});
    sut->AppendKeyPoint({2,0,0});
    sut->AppendKeyPoint({0,-1,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 4);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({-2,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({0,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[2].AlmostEqual({0,1,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[3].AlmostEqual({2,0,0}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(EllipseManipulationStrategyTests, FinishPrimitive_RequiresAtLeast2KeyPoints)
    {
    CurvePrimitiveManipulationStrategyPtr sut = EllipseManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_TRUE(sut->FinishPrimitive().IsNull());
    sut->AppendKeyPoint({-2,0,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    ASSERT_TRUE(sut->FinishPrimitive().IsNull());
    sut->AppendKeyPoint({0,0,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);
    ASSERT_TRUE(sut->FinishPrimitive().IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(EllipseManipulationStrategyTests, FinishPrimitive_2KeyPoints_CreatesFullCircularArc)
    {
    CurvePrimitiveManipulationStrategyPtr sut = EllipseManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    double radius = 2;
    sut->AppendKeyPoint({-radius,0,0});
    sut->AppendKeyPoint({0,0,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());

    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), arc.vector90.Magnitude());
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), radius);
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::TwoPi());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(EllipseManipulationStrategyTests, FinishPrimitive_3KeyPoints_CreatesFullEllipticArc)
    {
    CurvePrimitiveManipulationStrategyPtr sut = EllipseManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AppendKeyPoint({2,0,0});
    sut->AppendKeyPoint({0,0,0});
    sut->AppendKeyPoint({3,1,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 3);
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());
    
    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 2);
    ASSERT_DOUBLE_EQ(arc.vector90.Magnitude(), 1);
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::TwoPi());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(EllipseManipulationStrategyTests, FinishPrimitive_4KeyPoints_CreatesEllipticArc)
    {
    CurvePrimitiveManipulationStrategyPtr sut = EllipseManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AppendKeyPoint({2,0,0});
    sut->AppendKeyPoint({0,0,0});
    sut->AppendKeyPoint({4,1,0});
    sut->AppendKeyPoint({4,4,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 4);
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());

    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 2);
    ASSERT_DOUBLE_EQ(arc.vector90.Magnitude(), 1);
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::DegreesToRadians(45));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(EllipseManipulationStrategyTests, FinishPrimitive_CreatesArcMoreThanHalfSweep)
    {
    CurvePrimitiveManipulationStrategyPtr sut = EllipseManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AppendKeyPoint({2,0,0});
    sut->AppendKeyPoint({0,0,0});
    sut->AppendKeyPoint({4,-1,0});
    sut->AppendDynamicKeyPoint({4,-4,0});
    sut->AppendDynamicKeyPoint({0,-4,0});
    sut->AppendDynamicKeyPoint({-4,-4,0});
    sut->AppendDynamicKeyPoint({-4,0,0});
    sut->AppendDynamicKeyPoint({-4,4,0});
    sut->AppendKeyPoint({0,4,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 4);
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());

    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 2);
    ASSERT_DOUBLE_EQ(arc.vector90.Magnitude(), 1);
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::DegreesToRadians(270));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(EllipseManipulationStrategyTests, FinishPrimitive_SweepDirectionChangedOnLastAppend)
    {
    CurvePrimitiveManipulationStrategyPtr sut = EllipseManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AppendKeyPoint({2,0,0});
    sut->AppendKeyPoint({0,0,0});
    sut->AppendKeyPoint({4,-1,0});
    sut->AppendDynamicKeyPoint({4,-4,0});
    sut->AppendDynamicKeyPoint({0,-4,0});
    sut->AppendKeyPoint({0,4,0});

    ASSERT_EQ(sut->GetKeyPoints().size(), 4);
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());

    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 2);
    ASSERT_DOUBLE_EQ(arc.vector90.Magnitude(), 1);
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::DegreesToRadians(-90));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(EllipseManipulationStrategyTests, FinishPrimitive_DoFullSweepPlusSomeMoreSweepInSameDirection_SweepDirectionDoesNotChange)
    {
    CurvePrimitiveManipulationStrategyPtr sut = EllipseManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AppendKeyPoint({2,0,0});
    sut->AppendKeyPoint({0,0,0});
    sut->AppendKeyPoint({4,-1,0});
    sut->AppendDynamicKeyPoint({4,-4,0});
    sut->AppendDynamicKeyPoint({0,-4,0});
    sut->AppendDynamicKeyPoint({-4,-4,0});
    sut->AppendDynamicKeyPoint({-4,0,0});
    sut->AppendDynamicKeyPoint({-4,4,0});
    sut->AppendDynamicKeyPoint({0,4,0});
    sut->AppendDynamicKeyPoint({4,4,0});
    sut->AppendDynamicKeyPoint({4,2,0});
    sut->AppendDynamicKeyPoint({4,-4,0});
    sut->AppendKeyPoint({0,-4,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 4);
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());

    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 2);
    ASSERT_DOUBLE_EQ(arc.vector90.Magnitude(), 1);
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::DegreesToRadians(90));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(EllipseManipulationStrategyTests, FinishPrimitive_CreatesEllipse)
    {
    CurvePrimitiveManipulationStrategyPtr sut = EllipseManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AppendKeyPoint({-2,0,0});
    sut->AppendKeyPoint({0,0,0});
    sut->AppendKeyPoint({0,1,0});
    sut->AppendKeyPoint({2,0,0});
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());

    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));
    }