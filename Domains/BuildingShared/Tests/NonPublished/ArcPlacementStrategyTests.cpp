/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/ArcPlacementStrategyTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct ArcStartCenterPlacementStrategyTests : public BuildingSharedTestFixtureBase
    {
    };

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, AddKeyPoint_AccepsAMaximumOf3KeyPoints)
    {
    CurvePrimitivePlacementStrategyPtr sut = ArcStartCenterPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    sut->AddKeyPoint({-2,0,0});
    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({0,1,0});
    sut->AddKeyPoint({2,0,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 4);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({-2,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({0,0,0}));
    // sut->GetKeyPoints()[2] is generated.
    ASSERT_TRUE(sut->GetKeyPoints()[3].AlmostEqual({0,1,0}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, AddKeyPoint_2KeyPointsExist_Adding3rdAdds2KeyPoints)
    {
    CurvePrimitivePlacementStrategyPtr sut = ArcStartCenterPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());
    sut->AddKeyPoint({-2,0,0});
    sut->AddKeyPoint({0,0,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);

    sut->AddKeyPoint({0,1,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 4);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, PopKeyPoint_4KeyPointsExist_PopsGeneratedKeyPoint)
    {
    CurvePrimitivePlacementStrategyPtr sut = ArcStartCenterPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    sut->AddKeyPoint({-2,0,0});
    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({0,1,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 4);

    sut->PopKeyPoint();
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, FinishPrimitive_CreatesCircularArc)
    {
    CurvePrimitivePlacementStrategyPtr sut = ArcStartCenterPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AddKeyPoint({2,0,0});
    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({0,4,0});
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());

    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));

    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), arc.vector90.Magnitude());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, FinishPrimitive_SweepDeterminedByThe3rdPoint_CCW)
    {
    CurvePrimitivePlacementStrategyPtr sut = ArcStartCenterPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AddKeyPoint({2,0,0});
    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({-4,4,0});
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());

    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));

    ASSERT_TRUE(arc.vector0.AlmostEqual(DVec3d::From(2, 0, 0)));
    ASSERT_TRUE(arc.vector90.AlmostEqual(DVec3d::From(0, 2, 0)));
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::DegreesToRadians(135));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, FinishPrimitive_SweepDeterminedByThe3rdPoint_CW)
    {
    CurvePrimitivePlacementStrategyPtr sut = ArcStartCenterPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AddKeyPoint({2,0,0});
    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({-4,-4,0});
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());

    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));

    ASSERT_TRUE(arc.vector0.AlmostEqual(DVec3d::From(2, 0, 0)));
    ASSERT_TRUE(arc.vector90.AlmostEqual(DVec3d::From(0, -2, 0)));
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::DegreesToRadians(135));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, FinishPrimitive_CanSweepMoreThanPI)
    {
    CurvePrimitivePlacementStrategyPtr sut = ArcStartCenterPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AddKeyPoint({2,0,0});
    sut->AddKeyPoint({0,0,0});
    sut->AppendDynamicKeyPoint({4,-4,0});
    sut->AppendDynamicKeyPoint({0,-4,0});
    sut->AppendDynamicKeyPoint({-4,-4,0});
    sut->AppendDynamicKeyPoint({-4,0,0});
    sut->AddKeyPoint({-4,4,0});
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());

    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));

    ASSERT_TRUE(arc.vector0.AlmostEqual(DVec3d::From(2, 0, 0)));
    ASSERT_TRUE(arc.vector90.AlmostEqual(DVec3d::From(0, -2, 0)));
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::DegreesToRadians(225));
    }