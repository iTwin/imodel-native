/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

    RotMatrix actualRotation;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(RectanglePlacementStrategy::prop_Rotation(), actualRotation));
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

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(RectanglePlacementStrategyTestFixture, PopKeyPoint)
    {
    RectanglePlacementStrategyPtr sut = RectanglePlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_TRUE(sut->GetKeyPoints().empty());
    sut->AddKeyPoint({0,0,0});
    ASSERT_FALSE(sut->GetKeyPoints().empty());
    sut->PopKeyPoint();
    ASSERT_TRUE(sut->GetKeyPoints().empty());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(RectanglePlacementStrategyTestFixture, TryGetProperty_Rotation_0degree)
    {
    RectanglePlacementStrategyPtr sut = RectanglePlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({1,0,0});

    RotMatrix expectedRotation = RotMatrix::FromIdentity();
    RotMatrix actualRotation;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(RectanglePlacementStrategy::prop_Rotation(), actualRotation));
    ASSERT_TRUE(actualRotation.IsEqual(expectedRotation));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(RectanglePlacementStrategyTestFixture, TryGetProperty_Rotation_RotationPointAt45DegreeFromXAxis)
    {
    RectanglePlacementStrategyPtr sut = RectanglePlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AddKeyPoint({0,0,0});
    sut->AddKeyPoint({1,1,0});

    double angleDegrees = -45;
    double angleRadians = Angle::DegreesToRadians(angleDegrees);
    RotMatrix expectedRotation = RotMatrix::FromRowValues(
        cos(angleRadians), -sin(angleRadians), 0,
        sin(angleRadians), cos(angleRadians), 0,
        0, 0, 1
    );

    RotMatrix actualRotation;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(RectanglePlacementStrategy::prop_Rotation(), actualRotation));
    ASSERT_TRUE(actualRotation.IsEqual(expectedRotation));
    }