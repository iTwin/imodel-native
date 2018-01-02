/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/ArcPlacementStrategyTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

struct ArcStartEndMidPlacementStrategyTests : public BuildingSharedTestFixtureBase
    {
    };

void assertKeyPoints(ArcManipulationStrategyCR sut, bool startSet, bool endSet, bool midSet, bool centerSet, Utf8CP message = "")
    {
    if (startSet)
        ASSERT_TRUE(sut.IsStartSet()) << message;
    else
        ASSERT_FALSE(sut.IsStartSet()) << message;

    if (endSet)
        ASSERT_TRUE(sut.IsEndSet()) << message;
    else
        ASSERT_FALSE(sut.IsEndSet()) << message;

    if (midSet)
        ASSERT_TRUE(sut.IsMidSet()) << message;
    else
        ASSERT_FALSE(sut.IsMidSet()) << message;

    if (centerSet)
        ASSERT_TRUE(sut.IsCenterSet()) << message;
    else
        ASSERT_FALSE(sut.IsCenterSet()) << message;
    }

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

#pragma region Arc_StartCenter_PlacementStrategy

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, KeyPointManipulation)
    {
    ArcPlacementStrategyPtr sut = ArcStartCenterPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());
    ArcManipulationStrategyCR manipSut = dynamic_cast<ArcManipulationStrategyCR>(sut->GetManipulationStrategy());

    assertKeyPoints(manipSut, false, false, false, false, "Initial state");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "Initial state";

    sut->AddDynamicKeyPoint({2,0,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 dynamic key point";
    assertKeyPoints(manipSut, true, false, false, false, "[AddDynamicKeyPoint] Has 1 dynamic key point");

    sut->AddKeyPoint({2,0,0});
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 1 key point";
    assertKeyPoints(manipSut, true, false, false, false, "[AddKeyPoint] Has 1 key point");
    sut->AddDynamicKeyPoint({0,-3,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 key point, 1 dynamic key point";
    assertKeyPoints(manipSut, true, false, false, true, "[AddDynamicKeyPoint] Has 1 key point, 1 dynamic key point");

    sut->AddKeyPoint({0,-3,0});
    assertKeyPoints(manipSut, true, false, false, true, "[AddKeyPoint] Has 2 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 2 key points";
    sut->AddDynamicKeyPoint({0,3,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddDynamicKeyPoint] Has 2 key points, 1 dynamic key point");
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 2 key points, 1 dynamic key point";

    sut->AddKeyPoint({0,3,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddKeyPoint] Has 3 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 3 key points";

    sut->AddDynamicKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddDynamicKeyPoint] Can't add more than 3 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Can't add more than 3 key points";
    sut->AddKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddKeyPoint] Can't add more than 3 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Can't add more than 3 key points";

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, true, false, false, true, "[PopKeyPoint] Has 2 key point");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 2 key point";
    sut->PopKeyPoint();
    assertKeyPoints(manipSut, true, false, false, false, "[PopKeyPoint] Has 1 key point");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 1 key point";
    sut->PopKeyPoint();
    assertKeyPoints(manipSut, false, false, false, false, "[PopKeyPoint] Has 0 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 0 key points";
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
TEST_F(ArcCenterStartPlacementStrategyTests, KeyPointManipulation)
    {
    ArcPlacementStrategyPtr sut = ArcCenterStartPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());
    ArcManipulationStrategyCR manipSut = dynamic_cast<ArcManipulationStrategyCR>(sut->GetManipulationStrategy());

    assertKeyPoints(manipSut, false, false, false, false, "Initial state");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "Initial state";

    sut->AddDynamicKeyPoint({2,0,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 dynamic key point";
    assertKeyPoints(manipSut, false, false, false, true, "[AddDynamicKeyPoint] Has 1 dynamic key point");

    sut->AddKeyPoint({2,0,0});
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 1 key point";
    assertKeyPoints(manipSut, false, false, false, true, "[AddKeyPoint] Has 1 key point");
    sut->AddDynamicKeyPoint({0,-3,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 key point, 1 dynamic key point";
    assertKeyPoints(manipSut, true, false, false, true, "[AddDynamicKeyPoint] Has 1 key point, 1 dynamic key point");

    sut->AddKeyPoint({0,-3,0});
    assertKeyPoints(manipSut, true, false, false, true, "[AddKeyPoint] Has 2 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 2 key points";
    sut->AddDynamicKeyPoint({0,3,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddDynamicKeyPoint] Has 2 key points, 1 dynamic key point");
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 2 key points, 1 dynamic key point";

    sut->AddKeyPoint({0,3,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddKeyPoint] Has 3 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 3 key points";

    sut->AddDynamicKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddDynamicKeyPoint] Can't add more than 3 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Can't add more than 3 key points";
    sut->AddKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddKeyPoint] Can't add more than 3 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Can't add more than 3 key points";

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, true, false, false, true, "[PopKeyPoint] Has 2 key point");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 2 key point";
    sut->PopKeyPoint();
    assertKeyPoints(manipSut, false, false, false, true, "[PopKeyPoint] Has 1 key point");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 1 key point";
    sut->PopKeyPoint();
    assertKeyPoints(manipSut, false, false, false, false, "[PopKeyPoint] Has 0 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 0 key points";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcCenterStartPlacementStrategyTests, FinishPrimitive_3KeyPointsNeededForPrimitiveToBeValid)
    {
    ArcPlacementStrategyPtr sut = ArcCenterStartPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_FALSE(sut->FinishPrimitive().IsValid()) << "IsValid with 0 key points";
    sut->AddKeyPoint({0,0,0});
    ASSERT_FALSE(sut->FinishPrimitive().IsValid()) << "IsValid with 1 key point";
    sut->AddKeyPoint({2,0,0});
    ASSERT_FALSE(sut->FinishPrimitive().IsValid()) << "IsValid with 2 key points";
    sut->AddKeyPoint({-3,3,0});
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid()) << "Is not valid with 3 key points";

    DEllipse3d expectedArc = DEllipse3d::FromVectors({0,0,0}, DVec3d::From(2, 0, 0), DVec3d::From(0, 2, 0), 0, Angle::DegreesToRadians(135));
    DVec3d expectedNormal = DVec3d::FromCrossProduct(expectedArc.vector0, expectedArc.vector90);
    DEllipse3d actualArc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(actualArc));
    DVec3d actualNormal = DVec3d::FromCrossProduct(actualArc.vector0, actualArc.vector90);

    ASSERT_TRUE(actualArc.center.AlmostEqual(expectedArc.center));
    ASSERT_TRUE(actualArc.IsCircular());
    ASSERT_DOUBLE_EQ(actualArc.vector0.Magnitude(), expectedArc.vector0.Magnitude());
    ASSERT_DOUBLE_EQ(actualArc.sweep, expectedArc.sweep);
    ASSERT_DOUBLE_EQ(actualArc.start, expectedArc.start);
    ASSERT_TRUE(actualNormal.AlmostEqual(expectedNormal));
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
    ArcManipulationStrategyCR manipSut = dynamic_cast<ArcManipulationStrategyCR>(sut->GetManipulationStrategy());

    assertKeyPoints(manipSut, false, false, false, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "Initial state";

    sut->AddDynamicKeyPoint({2,0,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 dynamic key point";
    assertKeyPoints(manipSut, true, false, false, false);

    sut->AddKeyPoint({2,0,0});
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 1 key point";
    assertKeyPoints(manipSut, true, false, false, false);
    sut->AddDynamicKeyPoint({0,-3,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 key point, 1 dynamic key point";
    assertKeyPoints(manipSut, true, false, true, false);

    sut->AddKeyPoint({0,-3,0});
    assertKeyPoints(manipSut, true, false, true, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 2 key points";
    sut->AddDynamicKeyPoint({0,3,0});
    assertKeyPoints(manipSut, true, true, true, false);
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 2 key points, 1 dynamic key points";

    sut->AddKeyPoint({0,3,0});
    assertKeyPoints(manipSut, true, true, true, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 3 key points";

    sut->AddDynamicKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, true, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Can't add more than 3 key points";
    sut->AddKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, true, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Can't add more than 3 key points";

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, true, false, true, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 2 key point";
    sut->PopKeyPoint();
    assertKeyPoints(manipSut, true, false, false, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 1 key point";
    sut->PopKeyPoint();
    assertKeyPoints(manipSut, false, false, false, false);
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
    sut->AddKeyPoint({-2,-2,0});
    ASSERT_FALSE(sut->FinishPrimitive().IsValid()) << "IsValid with 2 key points";
    sut->AddKeyPoint({0,3,0});
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid()) << "Is not valid with 3 key points";

    DEllipse3d expectedArc = DEllipse3d::FromPointsOnArc({2,0,0}, {-2,-2,0}, {0,3,0});
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

#pragma region Arc_StartEndMid_PlacementStrategy

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartEndMidPlacementStrategyTests, KeyPointManipulation)
    {
    ArcPlacementStrategyPtr sut = ArcStartEndMidPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());
    ArcManipulationStrategyCR manipSut = dynamic_cast<ArcManipulationStrategyCR>(sut->GetManipulationStrategy());

    assertKeyPoints(manipSut, false, false, false, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());

    sut->AddDynamicKeyPoint({2,0,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 dynamic key point";
    assertKeyPoints(manipSut, true, false, false, false);

    sut->AddKeyPoint({2,0,0});
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 1 key point";
    assertKeyPoints(manipSut, true, false, false, false);
    sut->AddDynamicKeyPoint({0,-3,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 key point, 1 dynamic key point";
    assertKeyPoints(manipSut, true, true, false, false);

    sut->AddKeyPoint({0,3,0});
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 2 key points";
    assertKeyPoints(manipSut, true, true, false, false);
    sut->AddDynamicKeyPoint({0,3,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 2 key points, 1 dynamic key point";
    assertKeyPoints(manipSut, true, true, true, false);

    sut->AddKeyPoint({0,-3,0});
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 3 key points";
    assertKeyPoints(manipSut, true, true, true, false);

    sut->AddDynamicKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, true, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Can't add more than 3 key points";
    sut->AddKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, true, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Can't add more than 3 key points";

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, true, true, false, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 2 key points";
    sut->PopKeyPoint();
    assertKeyPoints(manipSut, true, false, false, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 1 key point";
    sut->PopKeyPoint();
    assertKeyPoints(manipSut, false, false, false, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 0 key points";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartEndMidPlacementStrategyTests, FinishPrimitive_3KeyPointsNeededForPrimitiveToBeValid)
    {
    ArcPlacementStrategyPtr sut = ArcStartEndMidPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_FALSE(sut->FinishPrimitive().IsValid()) << "IsValid with 0 key points";
    sut->AddKeyPoint({2,0,0});
    ASSERT_FALSE(sut->FinishPrimitive().IsValid()) << "IsValid with 1 key point";
    sut->AddKeyPoint({0,3,0});
    ASSERT_FALSE(sut->FinishPrimitive().IsValid()) << "IsValid with 2 key points";
    sut->AddKeyPoint({0,-3,0});
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid()) << "Is not valid with 3 key points";

    DEllipse3d expectedArc = DEllipse3d::FromPointsOnArc({2,0,0}, {0,-3,0}, {0,3,0});
    DVec3d expectedNormal = DVec3d::FromCrossProduct(expectedArc.vector0, expectedArc.vector90);
    DEllipse3d actualArc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(actualArc));
    DVec3d actualNormal = DVec3d::FromCrossProduct(actualArc.vector0, actualArc.vector90);

    ASSERT_TRUE(actualArc.center.AlmostEqual(expectedArc.center));
    ASSERT_TRUE(actualArc.IsCircular());
    ASSERT_DOUBLE_EQ(actualArc.vector0.Magnitude(), expectedArc.vector0.Magnitude());
    ASSERT_DOUBLE_EQ(actualArc.sweep, expectedArc.sweep);
    ASSERT_DOUBLE_EQ(actualArc.start, expectedArc.start);
    ASSERT_TRUE(actualNormal.AlmostEqual(expectedNormal));
    }

#pragma endregion
