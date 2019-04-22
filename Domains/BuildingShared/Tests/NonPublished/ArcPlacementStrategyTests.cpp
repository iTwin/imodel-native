/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct ArcPlacementStrategyTestFixture : BuildingSharedTestFixtureBase
    {
    };

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

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcPlacementStrategyTestFixture, SetPlacementMethod_ClearsKeyPoints)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartMidEnd);
    ASSERT_TRUE(sut.IsValid());
    ArcManipulationStrategyCR manipSut = dynamic_cast<ArcManipulationStrategyCR>(sut->GetManipulationStrategy());
    ASSERT_EQ(ArcPlacementMethod::StartMidEnd, sut->GetPlacementMethod());

    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({1,1,0});
    assertKeyPoints(manipSut, true, false, true, false, "Before SetPlacementMethod");

    sut->SetPlacementMethod(ArcPlacementMethod::StartMidEnd);
    ASSERT_EQ(ArcPlacementMethod::StartMidEnd, sut->GetPlacementMethod());
    assertKeyPoints(manipSut, true, false, true, false, "Setting the same PlacementMethod does nothing");

    sut->SetPlacementMethod(ArcPlacementMethod::StartEndMid);
    ASSERT_EQ(ArcPlacementMethod::StartEndMid, sut->GetPlacementMethod());
    assertKeyPoints(manipSut, false, false, false, false, "Setting new PlacementMethod clears KeyPoints");
    }

#pragma region Arc_StartCenter_PlacementStrategy

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, KeyPointManipulation)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartCenter);
    ASSERT_TRUE(sut.IsValid());
    ArcManipulationStrategyCR manipSut = dynamic_cast<ArcManipulationStrategyCR>(sut->GetManipulationStrategy());

    DPoint3d start, mid, center, end;

    assertKeyPoints(manipSut, false, false, false, false, "Initial state");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "Initial state";
    ASSERT_FALSE(sut->IsComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());
    ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));

    sut->AddDynamicKeyPoint({2,0,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 dynamic key point";
    assertKeyPoints(manipSut, true, false, false, false, "[AddDynamicKeyPoint] Has 1 dynamic key point");
    ASSERT_FALSE(sut->IsComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));

    sut->AddKeyPoint({2,0,0});
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 1 key point";
    assertKeyPoints(manipSut, true, false, false, false, "[AddKeyPoint] Has 1 key point");
    ASSERT_FALSE(sut->IsComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));

    sut->AddDynamicKeyPoint({0,-3,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 key point, 1 dynamic key point";
    assertKeyPoints(manipSut, true, false, false, true, "[AddDynamicKeyPoint] Has 1 key point, 1 dynamic key point");
    ASSERT_FALSE(sut->IsComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(center.AlmostEqual({0,-3,0}));

    sut->AddKeyPoint({0,-3,0});
    assertKeyPoints(manipSut, true, false, false, true, "[AddKeyPoint] Has 2 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 2 key points";
    ASSERT_FALSE(sut->IsComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(center.AlmostEqual({0,-3,0}));

    sut->AddDynamicKeyPoint({0,3,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddDynamicKeyPoint] Has 2 key points, 1 dynamic key point");
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 2 key points, 1 dynamic key point";
    ASSERT_FALSE(sut->IsComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(center.AlmostEqual({0,-3,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));

    sut->AddKeyPoint({0,3,0});
    ASSERT_TRUE(sut->IsComplete());
    ASSERT_FALSE(sut->CanAcceptMorePoints());
    assertKeyPoints(manipSut, true, true, false, true, "[AddKeyPoint] Has 3 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 3 key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(center.AlmostEqual({0,-3,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));

    sut->AddDynamicKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddDynamicKeyPoint] Can't add more than 3 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Can't add more than 3 key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(center.AlmostEqual({0,-3,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));

    sut->AddKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddKeyPoint] Can't add more than 3 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Can't add more than 3 key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(center.AlmostEqual({0,-3,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, true, false, false, true, "[PopKeyPoint] Has 2 key point");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 2 key point";
    ASSERT_FALSE(sut->IsComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(center.AlmostEqual({0,-3,0}));

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, true, false, false, false, "[PopKeyPoint] Has 1 key point");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 1 key point";
    ASSERT_FALSE(sut->IsComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, false, false, false, false, "[PopKeyPoint] Has 0 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 0 key points";
    ASSERT_FALSE(sut->IsComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());
    ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, FinishPrimitive_CreatesCircularArc)
    {
    CurvePrimitivePlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartCenter);
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
    CurvePrimitivePlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartCenter);
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
    CurvePrimitivePlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartCenter);
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
    CurvePrimitivePlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartCenter);
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

    ASSERT_TRUE(arc.IsCircular());
    ASSERT_TRUE(arc.FractionToPoint(0).AlmostEqual({2,0,0}));
    ASSERT_TRUE(arc.FractionToPoint(1).AlmostEqual({-sqrt(2), sqrt(2), 0}));
    ASSERT_DOUBLE_EQ(fabs(arc.sweep), Angle::DegreesToRadians(225));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, FinishPrimitive_StartCenterEndInline_EndBetweenStartCenter)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartCenter);
    ASSERT_TRUE(sut.IsValid());

    sut->AddKeyPoint({2,0,0});
    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({1,0,0});
    DVec3d normal;
    ASSERT_NE(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));
    ASSERT_FALSE(sut->FinishPrimitive().IsValid());

    DVec3d expectedNormal = DVec3d::From(0, 0, 1);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));
    ASSERT_TRUE(normal.AlmostEqual(expectedNormal));
    
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());

    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));

    ASSERT_TRUE(arc.IsCircular());
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 2);
    DVec3d actualNormal = DVec3d::FromCrossProduct(arc.vector0, arc.vector90);
    actualNormal.Normalize();
    ASSERT_TRUE(actualNormal.AlmostEqual(expectedNormal));
    ASSERT_TRUE(arc.IsFullEllipse());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, FinishPrimitive_StartCenterEndInline_HalfSweepCCW)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartCenter);
    ASSERT_TRUE(sut.IsValid());

    sut->AddKeyPoint({2,0,0});
    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({-3,0,0});
    DVec3d normal;
    ASSERT_NE(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));
    ASSERT_FALSE(sut->FinishPrimitive().IsValid());

    DVec3d expectedNormal = DVec3d::From(0, 0, 1);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));
    ASSERT_TRUE(normal.AlmostEqual(expectedNormal));

    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());
    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));

    ASSERT_TRUE(arc.IsCircular());
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 2);
    DVec3d actualNormal = DVec3d::FromCrossProduct(arc.vector0, arc.vector90);
    actualNormal.Normalize();
    ASSERT_TRUE(actualNormal.AlmostEqual(expectedNormal));
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::Pi());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, GetNormal_StartCenterEndAlmostInline_NormalIsZeroVector)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartCenter);
    ASSERT_TRUE(sut.IsValid());

    DPoint3d start = DPoint3d::From(115596.02633837331, 44433.203571780570, 0);
    DPoint3d center = DPoint3d::From(105955.35751244408, 54401.581828836148, 0);
    DPoint3d badEnd = DPoint3d::From(115596.02633837330, 44433.203571780556, 0);
    DPoint3d goodEnd = DPoint3d::From(115596.02633837330, 44433.203571780556, 1);
    DVec3d expectedNormal = DVec3d::FromNormalizedCrossProduct(DVec3d::FromStartEnd(center, start), DVec3d::FromStartEnd(center, goodEnd));

    sut->AddKeyPoint(start);
    sut->AddKeyPoint(center);
    sut->AddDynamicKeyPoint(badEnd);
    DVec3d normal;
    ASSERT_NE(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));

    sut->AddDynamicKeyPoint(goodEnd);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));

    ASSERT_TRUE(normal.AlmostEqual(expectedNormal));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, FinishPrimitive_StartCenterEndInline_HalfSweepCW)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartCenter);
    ASSERT_TRUE(sut.IsValid());

    DVec3d normal;
    DVec3d expectedNormal = DVec3d::From(0, 0, 1);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));

    sut->AddKeyPoint({2,0,0});
    sut->AddKeyPoint({0,0,0});
    sut->AddDynamicKeyPoint({2,-2,0});
    sut->AddDynamicKeyPoint({0,-2,0});
    sut->AddDynamicKeyPoint({-2,-2,0});
    sut->AddKeyPoint({-3,0,0});

    ASSERT_TRUE(normal.AlmostEqual(expectedNormal));

    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());
    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));

    ASSERT_TRUE(arc.IsCircular());
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 2);
    DVec3d actualNormal = DVec3d::FromCrossProduct(arc.vector0, arc.vector90);
    actualNormal.Normalize();
    ASSERT_TRUE(actualNormal.AlmostEqual(expectedNormal));
    ASSERT_DOUBLE_EQ(arc.sweep, -Angle::Pi());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, FixedSweep)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartCenter);
    ASSERT_TRUE(sut.IsValid());

    DVec3d normal;
    DVec3d expectedNormal = DVec3d::From(0, 0, 1);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));

    bool useSweep;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_UseSweep(), useSweep));
    ASSERT_FALSE(useSweep);
    sut->SetProperty(ArcPlacementStrategy::prop_UseSweep(), true);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_UseSweep(), useSweep));
    ASSERT_TRUE(useSweep);

    sut->SetProperty(ArcPlacementStrategy::prop_Sweep(), Angle::TwoPi());

    ASSERT_FALSE(sut->IsComplete());
    sut->AddKeyPoint({1,0,0});
    sut->AddKeyPoint({0,0,0});
    ASSERT_TRUE(sut->IsComplete());
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());
    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));

    ASSERT_TRUE(arc.center.AlmostEqual({0,0,0}));
    ASSERT_TRUE(arc.IsCircular());
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 1);
    DVec3d actualNormal = DVec3d::FromCrossProduct(arc.vector0, arc.vector90);
    actualNormal.Normalize();
    ASSERT_TRUE(actualNormal.AlmostEqual(expectedNormal));
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::TwoPi());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, FinishConstructionGeometry)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartCenter);
    ASSERT_TRUE(sut.IsValid());

    DVec3d normal;
    DVec3d expectedNormal = DVec3d::From(0, 0, 1);
    DPlane3d plane = DPlane3d::FromOriginAndNormal(DPoint3d::From(0, 0, 0), expectedNormal);
    sut->SetProperty(ArcPlacementStrategy::prop_WorkingPlane(), plane);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));

    ASSERT_TRUE(sut->FinishConstructionGeometry().empty());
    sut->AddKeyPoint({2,0,0});
    ASSERT_TRUE(sut->FinishConstructionGeometry().empty());
    sut->AddDynamicKeyPoint({0,0,0});

    bvector<ConstructionGeometry> cGeom1 = sut->FinishConstructionGeometry();
    ASSERT_EQ(3, cGeom1.size());
    ASSERT_TRUE(cGeom1[0].IsValid());
    ASSERT_TRUE(cGeom1[1].IsValid());
    ASSERT_TRUE(cGeom1[2].IsValid());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcEdge, cGeom1[0].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenterToStart, cGeom1[1].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenter, cGeom1[2].GetType());
    ICurvePrimitivePtr arcCP1 = cGeom1[0].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr lineCP1 = cGeom1[1].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr centerCP1 = cGeom1[2].GetGeometry()->GetAsICurvePrimitive();
    ASSERT_TRUE(arcCP1.IsValid());
    ASSERT_TRUE(lineCP1.IsValid());
    ASSERT_TRUE(centerCP1.IsValid());
    ICurvePrimitivePtr expectedArcCP1 = ICurvePrimitive::CreateArc(DEllipse3d::FromCenterNormalRadius(DPoint3d::From(0, 0, 0), expectedNormal, 2));
    ASSERT_TRUE(arcCP1->IsSameStructureAndGeometry(*expectedArcCP1));
    ICurvePrimitivePtr expectedLineCP1 = ICurvePrimitive::CreateLine(DSegment3d::From({0,0,0},{2,0,0}));
    ASSERT_TRUE(lineCP1->IsSameStructureAndGeometry(*expectedLineCP1));
    DPoint3d expectedCenter1 = DPoint3d::From(0, 0, 0);
    ICurvePrimitivePtr expectedCenterCP1 = ICurvePrimitive::CreatePointString(&expectedCenter1, 1);
    ASSERT_TRUE(centerCP1->IsSameStructureAndGeometry(*expectedCenterCP1));

    sut->AddKeyPoint({-1,0,0});
    bvector<ConstructionGeometry> cGeom2 = sut->FinishConstructionGeometry();
    ASSERT_EQ(3, cGeom2.size());
    ASSERT_TRUE(cGeom2[0].IsValid());
    ASSERT_TRUE(cGeom2[1].IsValid());
    ASSERT_TRUE(cGeom2[2].IsValid());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcEdge, cGeom2[0].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenterToStart, cGeom2[1].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenter, cGeom2[2].GetType());
    ICurvePrimitivePtr arcCP2 = cGeom2[0].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr lineCP2 = cGeom2[1].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr centerCP2 = cGeom2[2].GetGeometry()->GetAsICurvePrimitive();
    ASSERT_TRUE(arcCP2.IsValid());
    ASSERT_TRUE(lineCP2.IsValid());
    ASSERT_TRUE(centerCP2.IsValid());
    ICurvePrimitivePtr expectedArcCP2 = ICurvePrimitive::CreateArc(DEllipse3d::FromCenterNormalRadius(DPoint3d::From(-1, 0, 0), expectedNormal, 3));
    ASSERT_TRUE(arcCP2->IsSameStructureAndGeometry(*expectedArcCP2));
    ICurvePrimitivePtr expectedLineCP2 = ICurvePrimitive::CreateLine(DSegment3d::From({-1,0,0},{2,0,0}));
    ASSERT_TRUE(lineCP2->IsSameStructureAndGeometry(*expectedLineCP2));
    DPoint3d expectedCenter2 = DPoint3d::From(-1, 0, 0);
    ICurvePrimitivePtr expectedCenterCP2 = ICurvePrimitive::CreatePointString(&expectedCenter2, 1);
    ASSERT_TRUE(centerCP2->IsSameStructureAndGeometry(*expectedCenterCP2));

    sut->AddDynamicKeyPoint({-1,3,0});
    bvector<ConstructionGeometry> cGeom3 = sut->FinishConstructionGeometry();
    ASSERT_EQ(3, cGeom3.size());
    ASSERT_TRUE(cGeom3[0].IsValid());
    ASSERT_TRUE(cGeom3[1].IsValid());
    ASSERT_TRUE(cGeom3[2].IsValid());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenterToEnd, cGeom3[0].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenterToStart, cGeom3[1].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenter, cGeom3[2].GetType());
    ICurvePrimitivePtr centerToEndCP1 = cGeom3[0].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr centerToStartCP1 = cGeom3[1].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr centerCP3 = cGeom3[2].GetGeometry()->GetAsICurvePrimitive();
    ASSERT_TRUE(centerToEndCP1.IsValid());
    ASSERT_TRUE(centerToStartCP1.IsValid());
    ASSERT_TRUE(centerCP3.IsValid());
    ICurvePrimitivePtr expectedCenterToEndCP1 = ICurvePrimitive::CreateLine(DSegment3d::From({-1,0,0}, {-1,3,0}));
    ICurvePrimitivePtr expectedCenterToStartCP1 = ICurvePrimitive::CreateLine(DSegment3d::From({-1,0,0}, {2,0,0}));
    ASSERT_TRUE(centerToEndCP1->IsSameStructureAndGeometry(*expectedCenterToEndCP1));
    ASSERT_TRUE(centerToStartCP1->IsSameStructureAndGeometry(*expectedCenterToStartCP1));
    DPoint3d expectedCenter3 = DPoint3d::From(-1, 0, 0);
    ICurvePrimitivePtr expectedCenterCP3 = ICurvePrimitive::CreatePointString(&expectedCenter3, 1);
    ASSERT_TRUE(centerCP3->IsSameStructureAndGeometry(*expectedCenterCP3));

    sut->AddKeyPoint({-4,0,0});
    bvector<ConstructionGeometry> cGeom4 = sut->FinishConstructionGeometry();
    ASSERT_EQ(3, cGeom4.size());
    ASSERT_TRUE(cGeom4[0].IsValid());
    ASSERT_TRUE(cGeom4[1].IsValid());
    ASSERT_TRUE(cGeom4[2].IsValid());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenterToEnd, cGeom4[0].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenterToStart, cGeom4[1].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenter, cGeom4[2].GetType());
    ICurvePrimitivePtr centerToEndCP2 = cGeom4[0].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr centerToStartCP2 = cGeom4[1].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr centerCP4 = cGeom4[2].GetGeometry()->GetAsICurvePrimitive();
    ASSERT_TRUE(centerToEndCP2.IsValid());
    ASSERT_TRUE(centerToStartCP2.IsValid());
    ASSERT_TRUE(centerCP4.IsValid());
    ICurvePrimitivePtr expectedCenterToEndCP2 = ICurvePrimitive::CreateLine(DSegment3d::From({-1,0,0}, {-4,0,0}));
    ICurvePrimitivePtr expectedCenterToStartCP2 = ICurvePrimitive::CreateLine(DSegment3d::From({-1,0,0}, {2,0,0}));
    ASSERT_TRUE(centerToEndCP2->IsSameStructureAndGeometry(*expectedCenterToEndCP2));
    ASSERT_TRUE(centerToStartCP2->IsSameStructureAndGeometry(*expectedCenterToStartCP2));
    DPoint3d expectedCenter4 = DPoint3d::From(-1, 0, 0);
    ICurvePrimitivePtr expectedCenterCP4 = ICurvePrimitive::CreatePointString(&expectedCenter4, 1);
    ASSERT_TRUE(centerCP4->IsSameStructureAndGeometry(*expectedCenterCP4));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartCenterPlacementStrategyTests, ChangeNormalWhilePickingEndPoint)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartCenter);
    ASSERT_TRUE(sut.IsValid());

    sut->AddKeyPoint({1,0,0});
    sut->AddKeyPoint({0,0,0});
    sut->AddDynamicKeyPoint({0,1,0});

    DVec3d actualNormal;
    DVec3d expectedNormal;
    double expectedRadius = 1;

    expectedNormal = DVec3d::From(0, 0, 1);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), actualNormal));
    ASSERT_TRUE(actualNormal.AlmostEqual(expectedNormal));
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());
    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), expectedRadius);
    // Don't care about the sweep sign here since we expect a 90 degree arc
    // and we are checking start/end points too.
    ASSERT_DOUBLE_EQ(fabs(arc.sweep), Angle::DegreesToRadians(90));
    DPoint3d arcStart, arcEnd;
    arc.EvaluateEndPoints(arcStart, arcEnd);
    ASSERT_TRUE(arcStart.AlmostEqual(DPoint3d::From(1, 0, 0)));
    ASSERT_TRUE(arcEnd.AlmostEqual(DPoint3d::From(0, 1, 0)));
    ASSERT_TRUE(DVec3d::FromNormalizedCrossProduct(arc.vector0, arc.vector90).AlmostEqual(expectedNormal));

    expectedNormal = DVec3d::From(0, -1, 0);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    sut->AddDynamicKeyPoint({0,0,1});
    arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), expectedRadius);
    ASSERT_DOUBLE_EQ(fabs(arc.sweep), Angle::DegreesToRadians(90));
    arc.EvaluateEndPoints(arcStart, arcEnd);
    ASSERT_TRUE(arcStart.AlmostEqual(DPoint3d::From(1, 0, 0)));
    ASSERT_TRUE(arcEnd.AlmostEqual(DPoint3d::From(0, 0, 1)));
    ASSERT_TRUE(DVec3d::FromNormalizedCrossProduct(arc.vector0, arc.vector90).AlmostEqual(expectedNormal));

    expectedNormal = DVec3d::From(1, 1, 0);
    expectedNormal.Normalize();
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    sut->AddDynamicKeyPoint({0,0,1});
    arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), expectedRadius);
    ASSERT_DOUBLE_EQ(fabs(arc.sweep), Angle::DegreesToRadians(90));
    arc.EvaluateEndPoints(arcStart, arcEnd);
    DVec3d expectedCenterToArcStart = DVec3d::From(1, -1, 0);
    expectedCenterToArcStart.Normalize();
    ASSERT_TRUE(arcStart.AlmostEqual(expectedCenterToArcStart));
    ASSERT_TRUE(arcEnd.AlmostEqual(DPoint3d::From(0, 0, 1)));

    expectedNormal = DVec3d::From(-1, 0, 0);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    sut->AddDynamicKeyPoint({0,0,1});

    // should produce invalid arc, because the radius should be 0 (center point matches with the start point)
    ASSERT_FALSE(sut->FinishPrimitive().IsValid());

    // go back to the initial normal to check if the keypoint for arc start was untouched.
    expectedNormal = DVec3d::From(0, 0, 1);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    ASSERT_TRUE(actualNormal.AlmostEqual(expectedNormal));
    sut->AddDynamicKeyPoint({0,1,0});
    arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), expectedRadius);
    ASSERT_DOUBLE_EQ(fabs(arc.sweep), Angle::DegreesToRadians(90));
    arc.EvaluateEndPoints(arcStart, arcEnd);
    ASSERT_TRUE(arcStart.AlmostEqual(DPoint3d::From(1, 0, 0)));
    ASSERT_TRUE(arcEnd.AlmostEqual(DPoint3d::From(0, 1, 0)));
    ASSERT_TRUE(DVec3d::FromNormalizedCrossProduct(arc.vector0, arc.vector90).AlmostEqual(expectedNormal));
    }

#pragma endregion

#pragma region Arc_CenterStart_PlacementStrategy

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcCenterStartPlacementStrategyTests, KeyPointManipulation)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::CenterStart);
    ASSERT_TRUE(sut.IsValid());
    ArcManipulationStrategyCR manipSut = dynamic_cast<ArcManipulationStrategyCR>(sut->GetManipulationStrategy());

    DPoint3d start, mid, center, end;

    assertKeyPoints(manipSut, false, false, false, false, "Initial state");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "Initial state";
    ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));

    sut->AddDynamicKeyPoint({2,0,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 dynamic key point";
    assertKeyPoints(manipSut, false, false, false, true, "[AddDynamicKeyPoint] Has 1 dynamic key point");
    ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(center.AlmostEqual({2,0,0}));

    sut->AddKeyPoint({2,0,0});
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 1 key point";
    assertKeyPoints(manipSut, false, false, false, true, "[AddKeyPoint] Has 1 key point");
    ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(center.AlmostEqual({2,0,0}));

    sut->AddDynamicKeyPoint({0,-3,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 key point, 1 dynamic key point";
    assertKeyPoints(manipSut, true, false, false, true, "[AddDynamicKeyPoint] Has 1 key point, 1 dynamic key point");
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(center.AlmostEqual({2,0,0}));
    ASSERT_TRUE(start.AlmostEqual({0,-3,0}));

    sut->AddKeyPoint({0,-3,0});
    assertKeyPoints(manipSut, true, false, false, true, "[AddKeyPoint] Has 2 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 2 key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(center.AlmostEqual({2,0,0}));
    ASSERT_TRUE(start.AlmostEqual({0,-3,0}));

    sut->AddDynamicKeyPoint({0,3,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddDynamicKeyPoint] Has 2 key points, 1 dynamic key point");
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 2 key points, 1 dynamic key point";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(center.AlmostEqual({2,0,0}));
    ASSERT_TRUE(start.AlmostEqual({0,-3,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));

    sut->AddKeyPoint({0,3,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddKeyPoint] Has 3 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 3 key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(center.AlmostEqual({2,0,0}));
    ASSERT_TRUE(start.AlmostEqual({0,-3,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));

    sut->AddDynamicKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddDynamicKeyPoint] Can't add more than 3 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Can't add more than 3 key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(center.AlmostEqual({2,0,0}));
    ASSERT_TRUE(start.AlmostEqual({0,-3,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));

    sut->AddKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, false, true, "[AddKeyPoint] Can't add more than 3 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Can't add more than 3 key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(center.AlmostEqual({2,0,0}));
    ASSERT_TRUE(start.AlmostEqual({0,-3,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, true, false, false, true, "[PopKeyPoint] Has 2 key point");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 2 key point";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(center.AlmostEqual({2,0,0}));
    ASSERT_TRUE(start.AlmostEqual({0,-3,0}));

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, false, false, false, true, "[PopKeyPoint] Has 1 key point");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 1 key point";
    ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(center.AlmostEqual({2,0,0}));

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, false, false, false, false, "[PopKeyPoint] Has 0 key points");
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 0 key points";
    ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcCenterStartPlacementStrategyTests, FinishPrimitive_3KeyPointsNeededForPrimitiveToBeValid)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::CenterStart);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcCenterStartPlacementStrategyTests, FixedSweep)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::CenterStart);
    ASSERT_TRUE(sut.IsValid());

    DVec3d normal;
    DVec3d expectedNormal = DVec3d::From(0, 0, 1);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));

    bool useSweep;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_UseSweep(), useSweep));
    ASSERT_FALSE(useSweep);
    sut->SetProperty(ArcPlacementStrategy::prop_UseSweep(), true);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_UseSweep(), useSweep));
    ASSERT_TRUE(useSweep);

    sut->SetProperty(ArcPlacementStrategy::prop_Sweep(), Angle::TwoPi());

    ASSERT_FALSE(sut->IsComplete());
    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({1,0,0});
    ASSERT_TRUE(sut->IsComplete());
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());
    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));

    ASSERT_TRUE(arc.center.AlmostEqual({0,0,0}));
    ASSERT_TRUE(arc.IsCircular());
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 1);
    DVec3d actualNormal = DVec3d::FromCrossProduct(arc.vector0, arc.vector90);
    actualNormal.Normalize();
    ASSERT_TRUE(actualNormal.AlmostEqual(expectedNormal));
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::TwoPi());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcCenterStartPlacementStrategyTests, AddDynamicKeyPoints_FixedSweep)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::CenterStart);
    ASSERT_TRUE(sut.IsValid());

    DVec3d normal;
    DVec3d expectedNormal = DVec3d::From(0, 0, 1);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));

    sut->SetProperty(ArcPlacementStrategy::prop_UseSweep(), true);
    sut->SetProperty(ArcPlacementStrategy::prop_Sweep(), Angle::TwoPi());

    ASSERT_FALSE(sut->IsComplete());
    sut->AddDynamicKeyPoints({{0,0,0},{1,0,0}});
    ASSERT_TRUE(sut->IsComplete());
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());
    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));

    ASSERT_TRUE(arc.center.AlmostEqual({0,0,0}));
    ASSERT_TRUE(arc.IsCircular());
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 1);
    DVec3d actualNormal = DVec3d::FromCrossProduct(arc.vector0, arc.vector90);
    actualNormal.Normalize();
    ASSERT_TRUE(actualNormal.AlmostEqual(expectedNormal));
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::TwoPi());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcCenterStartPlacementStrategyTests, FinishGeometry_IsNullWhen0Radius)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::CenterStart);
    ASSERT_TRUE(sut.IsValid());

    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), DVec3d::From(0, 0, 1));
    sut->SetProperty(ArcPlacementStrategy::prop_Sweep(), Angle::TwoPi());
    sut->SetProperty(ArcPlacementStrategy::prop_UseSweep(), true);

    sut->AddKeyPoint({0,0,0});
    sut->AddDynamicKeyPoint({0,0,0});
    ASSERT_TRUE(sut->FinishGeometry().IsNull());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcCenterStartPlacementStrategyTests, FinishPrimitive_SweepTest)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::CenterStart);
    ASSERT_TRUE(sut.IsValid());

    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), DVec3d::From(0, 0, 1));

    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({1,0,0});

    if (true)
        {
        sut->AddDynamicKeyPoint({1,0,0});
        ICurvePrimitivePtr arcCV = sut->FinishPrimitive();
        ASSERT_TRUE(arcCV.IsValid());
        DEllipse3d arc;
        ASSERT_TRUE(arcCV->TryGetArc(arc));
        ASSERT_DOUBLE_EQ(arc.sweep, Angle::TwoPi());
        }

    if (true)
        {
        sut->AddDynamicKeyPoint({1,1,0});
        ICurvePrimitivePtr arcCV = sut->FinishPrimitive();
        ASSERT_TRUE(arcCV.IsValid());
        DEllipse3d arc;
        ASSERT_TRUE(arcCV->TryGetArc(arc));
        ASSERT_DOUBLE_EQ(arc.sweep, Angle::DegreesToRadians(45));
        }

    if (true)
        {
        sut->AddDynamicKeyPoint({1,-1,0});
        ICurvePrimitivePtr arcCV = sut->FinishPrimitive();
        ASSERT_TRUE(arcCV.IsValid());
        DEllipse3d arc;
        ASSERT_TRUE(arcCV->TryGetArc(arc));
        ASSERT_DOUBLE_EQ(arc.sweep, Angle::DegreesToRadians(-45));
        }

    if (true)
        {
        sut->AddDynamicKeyPoint({-1,0,0});
        ICurvePrimitivePtr arcCV = sut->FinishPrimitive();
        ASSERT_TRUE(arcCV.IsValid());
        DEllipse3d arc;
        ASSERT_TRUE(arcCV->TryGetArc(arc));
        ASSERT_DOUBLE_EQ(arc.sweep, -Angle::Pi());
        }

    if (true)
        {
        sut->AddDynamicKeyPoint({0,1,0});
        ICurvePrimitivePtr arcCV = sut->FinishPrimitive();
        ASSERT_TRUE(arcCV.IsValid());
        DEllipse3d arc;
        ASSERT_TRUE(arcCV->TryGetArc(arc));
        ASSERT_DOUBLE_EQ(arc.sweep, Angle::DegreesToRadians(-270));
        }

    if (true)
        {
        sut->AddDynamicKeyPoint({1,-1,0});
        ICurvePrimitivePtr arcCV = sut->FinishPrimitive();
        ASSERT_TRUE(arcCV.IsValid());
        DEllipse3d arc;
        ASSERT_TRUE(arcCV->TryGetArc(arc));
        ASSERT_DOUBLE_EQ(arc.sweep, Angle::DegreesToRadians(-45));
        }

    if (true)
        {
        sut->AddDynamicKeyPoint({0,1,0});
        ICurvePrimitivePtr arcCV = sut->FinishPrimitive();
        ASSERT_TRUE(arcCV.IsValid());
        DEllipse3d arc;
        ASSERT_TRUE(arcCV->TryGetArc(arc));
        ASSERT_DOUBLE_EQ(arc.sweep, Angle::DegreesToRadians(90));
        }

    if (true)
        {
        sut->AddDynamicKeyPoint({-1,-1,0});
        ICurvePrimitivePtr arcCV = sut->FinishPrimitive();
        ASSERT_TRUE(arcCV.IsValid());
        DEllipse3d arc;
        ASSERT_TRUE(arcCV->TryGetArc(arc));
        ASSERT_DOUBLE_EQ(arc.sweep, Angle::DegreesToRadians(225));
        }
    }

#pragma endregion

#pragma region Arc_StartMidEnd_PlacementStrategy

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartMidEndPlacementStrategyTests, KeyPointManipulation)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartMidEnd);
    ASSERT_TRUE(sut.IsValid());
    ArcManipulationStrategyCR manipSut = dynamic_cast<ArcManipulationStrategyCR>(sut->GetManipulationStrategy());

    DPoint3d start, mid, center, end;

    assertKeyPoints(manipSut, false, false, false, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "Initial state";
    ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));

    sut->AddDynamicKeyPoint({2,0,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 dynamic key point";
    assertKeyPoints(manipSut, true, false, false, false);
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));

    sut->AddKeyPoint({2,0,0});
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 1 key point";
    assertKeyPoints(manipSut, true, false, false, false);
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));

    sut->AddDynamicKeyPoint({0,-3,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 key point, 1 dynamic key point";
    assertKeyPoints(manipSut, true, false, true, false);
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(mid.AlmostEqual({0,-3,0}));

    sut->AddKeyPoint({0,-3,0});
    assertKeyPoints(manipSut, true, false, true, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 2 key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(mid.AlmostEqual({0,-3,0}));

    sut->AddDynamicKeyPoint({0,3,0});
    assertKeyPoints(manipSut, true, true, true, false);
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 2 key points, 1 dynamic key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(mid.AlmostEqual({0,-3,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));

    sut->AddKeyPoint({0,3,0});
    assertKeyPoints(manipSut, true, true, true, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 3 key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(mid.AlmostEqual({0,-3,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));

    sut->AddDynamicKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, true, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Can't add more than 3 key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(mid.AlmostEqual({0,-3,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));

    sut->AddKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, true, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Can't add more than 3 key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(mid.AlmostEqual({0,-3,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, true, false, true, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 2 key point";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(mid.AlmostEqual({0,-3,0}));

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, true, false, false, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 1 key point";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, false, false, false, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 0 key points";
    ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartMidEndPlacementStrategyTests, FinishPrimitive_3KeyPointsNeededForPrimitiveToBeValid)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartMidEnd);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartMidEndPlacementStrategyTests, FixedSweep)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartMidEnd);
    ASSERT_TRUE(sut.IsValid());

    DVec3d normal;
    DVec3d expectedNormal = DVec3d::From(0, 0, 1);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));

    bool useSweep;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_UseSweep(), useSweep));
    ASSERT_FALSE(useSweep);
    sut->SetProperty(ArcPlacementStrategy::prop_UseSweep(), true);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_UseSweep(), useSweep));
    ASSERT_TRUE(useSweep);

    sut->SetProperty(ArcPlacementStrategy::prop_Sweep(), Angle::TwoPi());

    ASSERT_FALSE(sut->IsComplete());
    sut->AddKeyPoint({2,0,0});
    sut->AddKeyPoint({1,1,0});
    sut->AddKeyPoint({0,0,0});
    ASSERT_TRUE(sut->IsComplete());
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());
    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));

    ASSERT_TRUE(arc.center.AlmostEqual({1,0,0}));
    ASSERT_TRUE(arc.IsCircular());
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 1);
    DVec3d actualNormal = DVec3d::FromCrossProduct(arc.vector0, arc.vector90);
    actualNormal.Normalize();
    ASSERT_TRUE(actualNormal.AlmostEqual(expectedNormal));
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::TwoPi());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartMidEndPlacementStrategyTests, FinishPrimitive_3InlinePoints_IsInvalid)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartMidEnd);
    ASSERT_TRUE(sut.IsValid());

    DVec3d normal;
    DVec3d expectedNormal = DVec3d::From(0, 0, 1);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));

    ASSERT_FALSE(sut->IsComplete());
    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({1,0,0});
    sut->AddDynamicKeyPoint({1,0,0});
    ASSERT_TRUE(sut->FinishPrimitive().IsNull());
    ASSERT_TRUE(sut->FinishGeometry().IsNull());

    sut->AddKeyPoint({2,0,0});
    ASSERT_TRUE(sut->FinishPrimitive().IsNull());
    ASSERT_TRUE(sut->FinishGeometry().IsNull());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartMidEndPlacementStrategyTests, FinishPrimitive_FinishConstructionGeometry)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartMidEnd);
    ASSERT_TRUE(sut.IsValid());

    DVec3d normal;
    DVec3d expectedNormal = DVec3d::From(0, 0, 1);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));

    ASSERT_TRUE(sut->FinishConstructionGeometry().empty());
    sut->AddKeyPoint({0,0,0});
    ASSERT_TRUE(sut->FinishConstructionGeometry().empty());
    sut->AddKeyPoint({1,1,0});
    ASSERT_TRUE(sut->FinishConstructionGeometry().empty());
    sut->AddDynamicKeyPoint({2,0,0});

    bvector<ConstructionGeometry> cGeom1 = sut->FinishConstructionGeometry();
    ASSERT_EQ(4, cGeom1.size());
    ASSERT_TRUE(cGeom1[0].IsValid());
    ASSERT_TRUE(cGeom1[1].IsValid());
    ASSERT_TRUE(cGeom1[2].IsValid());
    ASSERT_TRUE(cGeom1[3].IsValid());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenterToEnd, cGeom1[0].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenterToStart, cGeom1[1].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenter, cGeom1[2].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcInfiniteLine, cGeom1[3].GetType());
    ICurvePrimitivePtr centerToEndCP1 = cGeom1[0].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr centerToStartCP1 = cGeom1[1].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr centerCP1 = cGeom1[2].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr infLineCP1 = cGeom1[3].GetGeometry()->GetAsICurvePrimitive();
    ASSERT_TRUE(centerToEndCP1.IsValid());
    ASSERT_TRUE(centerToStartCP1.IsValid());
    ASSERT_TRUE(centerCP1.IsValid());
    ASSERT_TRUE(infLineCP1.IsValid());
    ICurvePrimitivePtr expectedCenterToEndCP1 = ICurvePrimitive::CreateLine(DSegment3d::From({1,0,0}, {2,0,0}));
    ICurvePrimitivePtr expectedCenterToStartCP1 = ICurvePrimitive::CreateLine(DSegment3d::From({1,0,0}, {0,0,0}));
    DPoint3d expectedCenter1 = DPoint3d::From(1, 0, 0);
    ICurvePrimitivePtr expectedCenterCP1 = ICurvePrimitive::CreatePointString(&expectedCenter1, 1);
    ICurvePrimitivePtr expectedInfLineCP1 = ICurvePrimitive::CreateLine(DSegment3d::From({0,0,0}, {1,1,0}));
    ASSERT_TRUE(centerToEndCP1->IsSameStructureAndGeometry(*expectedCenterToEndCP1));
    ASSERT_TRUE(centerToStartCP1->IsSameStructureAndGeometry(*expectedCenterToStartCP1));
    ASSERT_TRUE(centerCP1->IsSameStructureAndGeometry(*expectedCenterCP1));
    ASSERT_TRUE(infLineCP1->IsSameStructureAndGeometry(*expectedInfLineCP1));

    sut->AddKeyPoint({1,-1,0});
    bvector<ConstructionGeometry> cGeom2 = sut->FinishConstructionGeometry();
    ASSERT_EQ(3, cGeom2.size());
    ASSERT_TRUE(cGeom2[0].IsValid());
    ASSERT_TRUE(cGeom2[1].IsValid());
    ASSERT_TRUE(cGeom2[2].IsValid());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenterToEnd, cGeom2[0].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenterToStart, cGeom2[1].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcCenter, cGeom2[2].GetType());
    ICurvePrimitivePtr centerToEndCP2 = cGeom2[0].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr centerToStartCP2 = cGeom2[1].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr centerCP2 = cGeom2[2].GetGeometry()->GetAsICurvePrimitive();
    ASSERT_TRUE(centerToEndCP2.IsValid());
    ASSERT_TRUE(centerToStartCP2.IsValid());
    ASSERT_TRUE(centerCP2.IsValid());
    ICurvePrimitivePtr expectedCenterToEndCP2 = ICurvePrimitive::CreateLine(DSegment3d::From({1,0,0}, {1,-1,0}));
    ICurvePrimitivePtr expectedCenterToStartCP2 = ICurvePrimitive::CreateLine(DSegment3d::From({1,0,0}, {0,0,0}));
    DPoint3d expectedCenter2 = DPoint3d::From(1, 0, 0);
    ICurvePrimitivePtr expectedCenterCP2 = ICurvePrimitive::CreatePointString(&expectedCenter2, 1);
    ASSERT_TRUE(centerToEndCP2->IsSameStructureAndGeometry(*expectedCenterToEndCP2));
    ASSERT_TRUE(centerToStartCP2->IsSameStructureAndGeometry(*expectedCenterToStartCP2));
    ASSERT_TRUE(centerCP2->IsSameStructureAndGeometry(*expectedCenterCP2));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartMidEndPlacementStrategyTests, InlineKeyPoints_FinishConstructionGeometry)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartMidEnd);
    ASSERT_TRUE(sut.IsValid());

    DVec3d normal;
    DVec3d expectedNormal = DVec3d::From(0, 0, 1);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));

    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({1,0,0});
    sut->AddDynamicKeyPoint({2,0,0});

    bvector<ConstructionGeometry> cGeom = sut->FinishConstructionGeometry();
    ASSERT_EQ(2, cGeom.size());
    ASSERT_TRUE(cGeom[0].IsValid());
    ASSERT_TRUE(cGeom[1].IsValid());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcInfiniteLine, cGeom[0].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcInvalidKeyPoint, cGeom[1].GetType());
    ICurvePrimitivePtr infLineCP = cGeom[0].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr invalidKeyPointCP = cGeom[1].GetGeometry()->GetAsICurvePrimitive();
    ASSERT_TRUE(infLineCP.IsValid());
    ASSERT_TRUE(invalidKeyPointCP.IsValid());
    ICurvePrimitivePtr expectedInfLineCP = ICurvePrimitive::CreateLine(DSegment3d::From({0,0,0}, {1,0,0}));
    DPoint3d expectedInvalidKeyPoint = DPoint3d::From(2, 0, 0);
    ICurvePrimitivePtr expectedInvalidKeyPointCP = ICurvePrimitive::CreatePointString(&expectedInvalidKeyPoint, 1);
    ASSERT_TRUE(infLineCP->IsSameStructureAndGeometry(*expectedInfLineCP));
    ASSERT_TRUE(invalidKeyPointCP->IsSameStructureAndGeometry(*expectedInvalidKeyPointCP));
    }

#pragma endregion

#pragma region Arc_StartEndMid_PlacementStrategy

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartEndMidPlacementStrategyTests, KeyPointManipulation)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartEndMid);
    ASSERT_TRUE(sut.IsValid());
    ArcManipulationStrategyCR manipSut = dynamic_cast<ArcManipulationStrategyCR>(sut->GetManipulationStrategy());

    DPoint3d start, mid, center, end;

    assertKeyPoints(manipSut, false, false, false, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());
    ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));

    sut->AddDynamicKeyPoint({2,0,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 dynamic key point";
    assertKeyPoints(manipSut, true, false, false, false);
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));

    sut->AddKeyPoint({2,0,0});
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 1 key point";
    assertKeyPoints(manipSut, true, false, false, false);
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));

    sut->AddDynamicKeyPoint({0,-3,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 1 key point, 1 dynamic key point";
    assertKeyPoints(manipSut, true, true, false, false);
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(end.AlmostEqual({0,-3,0}));

    sut->AddKeyPoint({0,3,0});
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 2 key points";
    assertKeyPoints(manipSut, true, true, false, false);
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));

    sut->AddDynamicKeyPoint({0,3,0});
    ASSERT_TRUE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Has 2 key points, 1 dynamic key point";
    assertKeyPoints(manipSut, true, true, true, false);
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));
    ASSERT_TRUE(mid.AlmostEqual({0,3,0}));

    sut->AddKeyPoint({0,-3,0});
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Has 3 key points";
    assertKeyPoints(manipSut, true, true, true, false);
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));
    ASSERT_TRUE(mid.AlmostEqual({0,-3,0}));

    sut->AddDynamicKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, true, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddDynamicKeyPoint] Can't add more than 3 key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));
    ASSERT_TRUE(mid.AlmostEqual({0,-3,0}));

    sut->AddKeyPoint({2,0,0});
    assertKeyPoints(manipSut, true, true, true, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[AddKeyPoint] Can't add more than 3 key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_TRUE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));
    ASSERT_TRUE(mid.AlmostEqual({0,-3,0}));

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, true, true, false, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 2 key points";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));
    ASSERT_TRUE(end.AlmostEqual({0,3,0}));

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, true, false, false, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 1 key point";
    ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    ASSERT_TRUE(start.AlmostEqual({2,0,0}));

    sut->PopKeyPoint();
    assertKeyPoints(manipSut, false, false, false, false);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet()) << "[PopKeyPoint] Has 0 key points";
    ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
    ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
    ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
    ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartEndMidPlacementStrategyTests, FinishPrimitive_3KeyPointsNeededForPrimitiveToBeValid)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartEndMid);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartEndMidPlacementStrategyTests, FixedSweep)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartEndMid);
    ASSERT_TRUE(sut.IsValid());

    DVec3d normal;
    DVec3d expectedNormal = DVec3d::From(0, 0, 1);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));

    bool useSweep;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_UseSweep(), useSweep));
    ASSERT_FALSE(useSweep);
    sut->SetProperty(ArcPlacementStrategy::prop_UseSweep(), true);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_UseSweep(), useSweep));
    ASSERT_TRUE(useSweep);

    sut->SetProperty(ArcPlacementStrategy::prop_Sweep(), Angle::TwoPi());

    ASSERT_FALSE(sut->IsComplete());
    sut->AddKeyPoint({2,0,0});
    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({1,1,0});
    ASSERT_TRUE(sut->IsComplete());
    ICurvePrimitivePtr arcPrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(arcPrimitive.IsValid());
    DEllipse3d arc;
    ASSERT_TRUE(arcPrimitive->TryGetArc(arc));

    ASSERT_TRUE(arc.center.AlmostEqual({1,0,0}));
    ASSERT_TRUE(arc.IsCircular());
    ASSERT_DOUBLE_EQ(arc.vector0.Magnitude(), 1);
    DVec3d actualNormal = DVec3d::FromCrossProduct(arc.vector0, arc.vector90);
    actualNormal.Normalize();
    ASSERT_TRUE(actualNormal.AlmostEqual(expectedNormal));
    ASSERT_DOUBLE_EQ(arc.sweep, Angle::TwoPi());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ArcStartEndMidPlacementStrategyTests, InlineKeyPoints_FinishConstructionGeometry)
    {
    ArcPlacementStrategyPtr sut = ArcPlacementStrategy::Create(ArcPlacementMethod::StartEndMid);
    ASSERT_TRUE(sut.IsValid());

    DVec3d normal;
    DVec3d expectedNormal = DVec3d::From(0, 0, 1);
    sut->SetProperty(ArcPlacementStrategy::prop_Normal(), expectedNormal);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ArcPlacementStrategy::prop_Normal(), normal));

    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({1,0,0});
    sut->AddDynamicKeyPoint({2,0,0});

    bvector<ConstructionGeometry> cGeom = sut->FinishConstructionGeometry();
    ASSERT_EQ(2, cGeom.size());
    ASSERT_TRUE(cGeom[0].IsValid());
    ASSERT_TRUE(cGeom[1].IsValid());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcInfiniteLine, cGeom[0].GetType());
    ASSERT_EQ(CONSTRUCTION_GEOMTYPE_ArcInvalidKeyPoint, cGeom[1].GetType());
    ICurvePrimitivePtr infLineCP = cGeom[0].GetGeometry()->GetAsICurvePrimitive();
    ICurvePrimitivePtr invalidKeyPointCP = cGeom[1].GetGeometry()->GetAsICurvePrimitive();
    ASSERT_TRUE(infLineCP.IsValid());
    ASSERT_TRUE(invalidKeyPointCP.IsValid());
    ICurvePrimitivePtr expectedInfLineCP = ICurvePrimitive::CreateLine(DSegment3d::From({0,0,0}, {1,0,0}));
    DPoint3d expectedInvalidKeyPoint = DPoint3d::From(2, 0, 0);
    ICurvePrimitivePtr expectedInvalidKeyPointCP = ICurvePrimitive::CreatePointString(&expectedInvalidKeyPoint, 1);
    ASSERT_TRUE(infLineCP->IsSameStructureAndGeometry(*expectedInfLineCP));
    ASSERT_TRUE(invalidKeyPointCP->IsSameStructureAndGeometry(*expectedInvalidKeyPointCP));
    }

#pragma endregion
