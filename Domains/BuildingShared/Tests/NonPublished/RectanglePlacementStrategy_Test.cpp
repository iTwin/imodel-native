/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RectanglePlacementStrategy_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct RectanglePlacementStrategyTestFixture : public BuildingSharedTestFixtureBase
    {};

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(RectanglePlacementStrategyTestFixture, SecondKeyPointSetsRotation)
    {
    RectanglePlacementStrategyPtr sut = RectanglePlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AddDynamicKeyPoint({1,0,0});
    sut->AddKeyPoint({0,0,0});
    sut->AddDynamicKeyPoint({2,0,0});
    sut->AddKeyPoint({1,1,0});

    RotMatrix expectedRotation;
    expectedRotation.InitRotationFromVectorToVector(DVec3d::From(1, 1, 0), DVec3d::From(0, 1, 0));
    RotMatrix actualRotation;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(RectanglePlacementStrategy::prop_Rotation(), actualRotation));
    ASSERT_TRUE(actualRotation.IsEqual(expectedRotation));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(RectanglePlacementStrategyTestFixture, FinishGeometry)
    {
    RectanglePlacementStrategyPtr sut = RectanglePlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AddDynamicKeyPoint({1,0,0});
    sut->AddKeyPoint({0,0,0});
    sut->AddDynamicKeyPoint({2,0,0});
    sut->AddKeyPoint({1,3,0});
    sut->AddDynamicKeyPoint({3,0,0});
    sut->AddKeyPoint({6.5,-0.5,0});

    IGeometryPtr geometry = sut->FinishGeometry();
    ASSERT_TRUE(geometry.IsValid());
    CurveVectorPtr actualShape = geometry->GetAsCurveVector();
    ASSERT_TRUE(actualShape.IsValid());

    CurveVectorPtr expectedShape = CurveVector::CreateLinear({{0,0,0},{0.5,1.5,0},{6.5,-0.5,0},{6,-2,0},{0,0,0}}, CurveVector::BOUNDARY_TYPE_Outer);
    actualShape->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer); // needed for comparison
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*actualShape, *expectedShape));
    }