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

struct ArcCenterStartPlacementStrategyTests : public BuildingSharedTestFixtureBase
    {
    };

struct ArcStartMidEndPlacementStrategyTests : public BuildingSharedTestFixtureBase
    {
    };

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

#pragma region Arc_StartCenter_PlacementStrategy

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
    sut->AddDynamicKeyPoint({4,-4,0});
    sut->AddDynamicKeyPoint({0,-4,0});
    sut->AddDynamicKeyPoint({-4,-4,0});
    sut->AddDynamicKeyPoint({-4,0,0});
    sut->AddKeyPoint({-4,4,0});
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());

    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));

    ASSERT_TRUE(arc.vector0.AlmostEqual(DVec3d::From(2, 0, 0)));
    ASSERT_TRUE(arc.vector90.AlmostEqual(DVec3d::From(0, -2, 0)));
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::DegreesToRadians(225));
    }

#pragma endregion

#pragma region Arc_CenterStart_PlacementStrategy

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcCenterStartPlacementStrategyTests, FinishPrimitive_1KeyPoint_ResultIsInvalid)
    {
    CurvePrimitivePlacementStrategyPtr sut = ArcCenterStartPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AddKeyPoint({0,0,0});
    ASSERT_FALSE(sut->FinishPrimitive().IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcCenterStartPlacementStrategyTests, IsDynamicKeyPointSet_FirstKeyPointAdded)
    {
    CurvePrimitivePlacementStrategyPtr sut = ArcCenterStartPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AddKeyPoint({0,0,0});
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcCenterStartPlacementStrategyTests, 1KP1DKP_KeyPointIsCenter)
    {
    CurvePrimitivePlacementStrategyPtr sut = ArcCenterStartPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AddKeyPoint({0,0,0});
    sut->AddDynamicKeyPoint({2,0,0});
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());

    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));
    ASSERT_TRUE(arc.center.AlmostEqual({0,0,0}));
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), arc.vector90.Magnitude());
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 2);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcCenterStartPlacementStrategyTests, 1KP1DKP_KeyPointIsCenter_AfterDynamicReset)
    {
    CurvePrimitivePlacementStrategyPtr sut = ArcCenterStartPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AddKeyPoint({0,0,0});
    sut->ResetDynamicKeyPoint();
    sut->AddDynamicKeyPoint({2,0,0});
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());

    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));
    ASSERT_TRUE(arc.center.AlmostEqual({0,0,0}));
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), arc.vector90.Magnitude());
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 2);
    }

#pragma endregion

#pragma region Arc_StartMidEnd_PlacementStrategy

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartMidEndPlacementStrategyTests, KeyPointManipulation)
    {
    ArcPlacementStrategyPtr sut = ArcStartMidEndPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_EQ(sut->GetKeyPoints().size(), 0) << "Initial state";
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "Initial state";
    sut->AddDynamicKeyPoint({2,0,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 dynamic key point";
    ASSERT_EQ(sut->GetKeyPoints().size(), 1) << "[AddDynamicKeyPoint] Has 1 dynamic key point";

    sut->AddKeyPoint({2,0,0});
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 1 key point, 0 dynamic key points";
    ASSERT_EQ(sut->GetKeyPoints().size(), 1) << "[AddKeyPoint] Has 1 key point, 0 dynamic key points";
    sut->AddDynamicKeyPoint({0,-3,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 key point, 1 generated point, 1 dynamic key point";
    ASSERT_EQ(sut->GetKeyPoints().size(), 3) << "[AddDynamicKeyPoint] Has 1 key point, 1 generated point, 1 dynamic key point";

    sut->AddKeyPoint({0,-3,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 3) << "[AddKeyPoint] Has 2 key point, 1 generated point, 0 dynamic key points";
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 2 key point, 1 generated point, 0 dynamic key points";
    sut->AddDynamicKeyPoint({0,3,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 4) << "[AddDynamicKeyPoint] Has 2 key point, 1 generated point, 1 dynamic key points";
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 2 key point, 1 generated point, 1 dynamic key points";

    sut->AddKeyPoint({0,3,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 4) << "[AddKeyPoint] Has 3 key points, 1 generated point";
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 3 key points, 1 generated point";

    sut->PopKeyPoint();
    ASSERT_EQ(sut->GetKeyPoints().size(), 3) << "[PopKeyPoint] Has 2 key point, 1 generated point, 0 dynamic key points";
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 2 key point, 1 generated point, 0 dynamic key points";
    sut->PopKeyPoint();
    ASSERT_EQ(sut->GetKeyPoints().size(), 1) << "[PopKeyPoint] Has 1 key point, 0 dynamic key points";
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 1 key point, 0 dynamic key points";
    sut->PopKeyPoint();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0) << "[PopKeyPoint] Has 0 key points";
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 0 key points";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartMidEndPlacementStrategyTests, FinishPrimitive_3KeyPointsNeededForPrimitiveToBeValid)
    {
    ArcPlacementStrategyPtr sut = ArcStartMidEndPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_FALSE(sut->FinishPrimitive().IsValid()) << "IsValid with 0 key points";
    sut->AddKeyPoint({2,0,0});
    ASSERT_FALSE(sut->FinishPrimitive().IsValid()) << "IsValid with 1 key point";
    sut->AddKeyPoint({0,-3,0});
    ASSERT_FALSE(sut->FinishPrimitive().IsValid()) << "IsValid with 2 key points";
    sut->AddKeyPoint({0,3,0});
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid()) << "Is not valid with 3 key points";

    DEllipse3d expectedArc = DEllipse3d::FromPointsOnArc({2,0,0}, {0,-3,0}, {0,3,0});
    DVec3d expectedNormal = DVec3d::FromCrossProduct(expectedArc.vector0, expectedArc.vector90);
    DEllipse3d actualArc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(actualArc));
    DVec3d actualNormal = DVec3d::FromCrossProduct(actualArc.vector0, actualArc.vector90);
    
    ASSERT_TRUE(actualArc.center.AlmostEqual(expectedArc.center));
    ASSERT_DOUBLE_EQ(actualArc.vector0.Magnitude(), actualArc.vector90.Magnitude()) << "Not circular arc";
    ASSERT_DOUBLE_EQ(actualArc.vector0.Magnitude(), expectedArc.vector0.Magnitude());
    ASSERT_DOUBLE_EQ(actualArc.sweep, expectedArc.sweep);
    ASSERT_DOUBLE_EQ(actualArc.start, expectedArc.start);
    ASSERT_TRUE(actualNormal.AlmostEqual(expectedNormal));
    }

#pragma endregion
