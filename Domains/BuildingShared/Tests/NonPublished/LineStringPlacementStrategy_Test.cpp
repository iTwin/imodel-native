/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct LineStringPointsPlacementStrategyTests : public BuildingSharedTestFixtureBase
    {};

struct LineStringMetesAndBoundsPlacementStrategyTests : public BuildingSharedTestFixtureBase
    {};

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

#pragma region LineString_Points

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(LineStringPointsPlacementStrategyTests, CreatesPrimitiveLineString)
    {
    LineStringPlacementStrategyPtr sut = LineStringPointsPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ICurvePrimitivePtr expectedCP1 = ICurvePrimitive::CreateLineString({{0,0,0},{2,0,0}});
    ICurvePrimitivePtr expectedCP2 = ICurvePrimitive::CreateLineString({{0,0,0},{2,0,0},{2,2,0}});

    ASSERT_FALSE(sut->FinishPrimitive().IsValid());

    sut->AddKeyPoint({0,0,0});
    ASSERT_FALSE(sut->FinishPrimitive().IsValid());

    sut->AddKeyPoint({2,0,0});
    ICurvePrimitivePtr cp1 = sut->FinishPrimitive();
    ASSERT_TRUE(cp1.IsValid());
    ASSERT_TRUE(cp1->IsSameStructureAndGeometry(*expectedCP1));

    sut->AddKeyPoint({2,2,0});
    ICurvePrimitivePtr cp2 = sut->FinishPrimitive();
    ASSERT_TRUE(cp2.IsValid());
    ASSERT_TRUE(cp2->IsSameStructureAndGeometry(*expectedCP2));
    }

#pragma endregion

#pragma region LineString_MetesAndBounds

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(LineStringMetesAndBoundsPlacementStrategyTests, CreatesPrimitiveLineString)
    {
    LineStringPlacementStrategyPtr sut = LineStringMetesAndBoundsPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_FALSE(sut->FinishPrimitive().IsValid());

    ICurvePrimitivePtr expectedCP = ICurvePrimitive::CreateLineString({{0,0,0},{2,2,0},{0,2,0}});
    bvector<Utf8String> directions(2);
    BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::UnitConverter::DirectionToMeetsAndBoundsString(directions[0], DVec3d::FromStartEnd(DPoint3d::From(0,0,0), DPoint3d::From(2,2,0)));
    BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::UnitConverter::DirectionToMeetsAndBoundsString(directions[1], DVec3d::FromStartEnd(DPoint3d::From(2,2,0), DPoint3d::From(0,2,0)));
    bvector<double> lengths(2);
    lengths[0] = DSegment3d::From({0,0,0}, {2,2,0}).Length();
    lengths[1] = DSegment3d::From({2,2,0}, {0,2,0}).Length();

    sut->SetProperty(LineStringMetesAndBoundsPlacementStrategy::prop_MetesAndBounds(), MetesAndBounds(directions, lengths));
    sut->AddKeyPoint({0,0,0});
    ICurvePrimitivePtr cp = sut->FinishPrimitive();
    ASSERT_TRUE(cp.IsValid());
    ASSERT_TRUE(cp->IsSameStructureAndGeometry(*expectedCP));
    }

#pragma endregion
