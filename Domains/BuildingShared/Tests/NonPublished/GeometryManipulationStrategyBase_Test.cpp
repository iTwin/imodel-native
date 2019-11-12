/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct GeometryManipulationStrategyBaseTestFixture : public BuildingSharedTestFixtureBase
    {
    };

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyBaseTestFixture, FinishGeometry_CreatePrimitive)
    {
    LineManipulationStrategyPtr sutManip  = LineManipulationStrategy::Create();
    GeometryManipulationStrategyBasePtr sut = sutManip;
    ASSERT_TRUE(sut.IsValid());

    sutManip->AppendKeyPoint({0,0,0});
    sutManip->AppendKeyPoint({1,0,0});

    IGeometryPtr geometry = sut->FinishGeometry();
    ASSERT_TRUE(geometry.IsValid());
    ASSERT_TRUE(geometry->GetAsICurvePrimitive().IsValid());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyBaseTestFixture, FinishGeometry_CreateCurveVector)
    {
    CurveVectorManipulationStrategyPtr sutManip = CurveVectorManipulationStrategy::Create();
    GeometryManipulationStrategyBasePtr sut = sutManip;
    ASSERT_TRUE(sut.IsValid());

    sutManip->ChangeDefaultNewGeometryType(DefaultNewGeometryType::LineString);
    sutManip->ChangeDefaultPlacementStrategy(LineStringPlacementStrategyType::Points);
    sutManip->AppendKeyPoint({0,0,0});
    sutManip->AppendKeyPoint({1,0,0});

    IGeometryPtr geometry = sut->FinishGeometry();
    ASSERT_TRUE(geometry.IsValid());
    ASSERT_TRUE(geometry->GetAsCurveVector().IsValid());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyBaseTestFixture, FinishGeometry_CreateSolidPrimitive)
    {
    ExtrusionManipulationStrategyPtr sutManip = ExtrusionManipulationStrategy::Create();
    GeometryManipulationStrategyBasePtr sut = sutManip;
    ASSERT_TRUE(sut.IsValid());

    sutManip->SetProperty(ExtrusionManipulationStrategy::prop_BaseShapeStrategy(), BaseShapeStrategyChangeProperty(DefaultNewGeometryType::LineString));
    sutManip->SetProperty(ExtrusionManipulationStrategy::prop_BaseShapeStrategy(), BaseShapeStrategyChangeProperty(LineStringPlacementStrategyType::Points));
    sutManip->AppendKeyPoint({0,0,0});
    sutManip->AppendKeyPoint({1,0,0});
    sutManip->SetProperty(ExtrusionManipulationStrategy::prop_BaseComplete(), true);
    sutManip->AppendKeyPoint({1,0,1});
    sutManip->AppendKeyPoint({1,0,1});

    IGeometryPtr geometry = sut->FinishGeometry();
    ASSERT_TRUE(geometry.IsValid());
    ASSERT_TRUE(geometry->GetAsISolidPrimitive().IsValid());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyBaseTestFixture, FinishGeometry_CreateNullGeometry)
    {
    NullGeometryManipulationStrategyPtr sutManip = NullGeometryManipulationStrategy::Create();
    GeometryManipulationStrategyBasePtr sut = sutManip;
    ASSERT_TRUE(sut.IsValid());

    sutManip->AppendKeyPoint({0,0,0});
    sutManip->AppendKeyPoint({1,0,0});

    ASSERT_TRUE(sut->FinishGeometry().IsNull());
    }