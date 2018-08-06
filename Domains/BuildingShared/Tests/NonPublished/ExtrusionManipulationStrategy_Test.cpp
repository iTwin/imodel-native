/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/ExtrusionManipulationStrategy_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct ExtrusionManipulationStrategyTestFixture : public BuildingSharedTestFixtureBase
    {};

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ExtrusionManipulationStrategyTestFixture, AppendDynamicKeyPoints)
    {
    CurveVectorManipulationStrategyPtr baseManip = CurveVectorManipulationStrategy::Create();
    ASSERT_TRUE(baseManip.IsValid());
    baseManip->AppendKeyPoint({0,0,0});
    baseManip->AppendKeyPoint({10,0,0});
    baseManip->AppendKeyPoint({10,10,0});
    baseManip->AppendKeyPoint({0,10,0});

    ExtrusionManipulationStrategyPtr sut = ExtrusionManipulationStrategy::Create(*baseManip);
    ASSERT_TRUE(sut.IsValid());

    ASSERT_FALSE(sut->IsComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());

    ASSERT_FALSE(sut->IsBaseComplete());
    sut->SetProperty(ExtrusionManipulationStrategy::prop_BaseComplete(), true);
    ASSERT_TRUE(sut->IsBaseComplete());

    sut->AppendDynamicKeyPoints({{0,0,10},{5,10,10}});
    bool isHeightSet = false;
    bool isDynamicHeightSet = false;
    bool isSweepDirectionSet = false;
    bool isDynamicSweepDirectionSet = false;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionManipulationStrategy::prop_IsHeightSet(), isHeightSet));
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionManipulationStrategy::prop_IsSweepDirectionSet(), isSweepDirectionSet));
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionManipulationStrategy::prop_IsDynamicHeightSet(), isDynamicHeightSet));
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionManipulationStrategy::prop_IsDynamicSweepDirectionSet(), isDynamicSweepDirectionSet));
    ASSERT_FALSE(isHeightSet);
    ASSERT_FALSE(isSweepDirectionSet);
    ASSERT_TRUE(isDynamicHeightSet);
    ASSERT_TRUE(isDynamicSweepDirectionSet);

    ISolidPrimitivePtr primitive = sut->FinishExtrusion(true, true);
    ASSERT_TRUE(primitive.IsValid());
    DgnExtrusionDetail extrusion;
    ASSERT_TRUE(primitive->TryGetDgnExtrusionDetail(extrusion));

    DVec3d expectedExtrusionVector = DVec3d::FromStartEnd(DPoint3d::From(0, 10, 0), DPoint3d::From(5, 10, 10));
    ASSERT_TRUE(extrusion.m_extrusionVector.AlmostEqual(expectedExtrusionVector));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ExtrusionManipulationStrategyTestFixture, FixedHeightAndSweep)
    {
    CurveVectorManipulationStrategyPtr baseManip = CurveVectorManipulationStrategy::Create();
    ASSERT_TRUE(baseManip.IsValid());
    baseManip->ChangeDefaultNewGeometryType(DefaultNewGeometryType::LineString);
    baseManip->ChangeDefaultPlacementStrategy(LineStringPlacementStrategyType::Points);
    ExtrusionManipulationStrategyPtr sut = ExtrusionManipulationStrategy::Create(*baseManip);
    ASSERT_TRUE(sut.IsValid());

    bool useFixedHeight = false;
    bool useFixedSweepDirection = false;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionManipulationStrategy::prop_UseFixedHeight(), useFixedHeight));
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionManipulationStrategy::prop_UseFixedSweepDirection(), useFixedSweepDirection));
    ASSERT_FALSE(useFixedHeight);
    ASSERT_FALSE(useFixedSweepDirection);

    sut->SetProperty(ExtrusionManipulationStrategy::prop_UseFixedHeight(), true);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionManipulationStrategy::prop_UseFixedHeight(), useFixedHeight));
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionManipulationStrategy::prop_UseFixedSweepDirection(), useFixedSweepDirection));
    ASSERT_TRUE(useFixedHeight);
    ASSERT_FALSE(useFixedSweepDirection);

    sut->SetProperty(ExtrusionManipulationStrategy::prop_UseFixedSweepDirection(), true);
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionManipulationStrategy::prop_UseFixedHeight(), useFixedHeight));
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionManipulationStrategy::prop_UseFixedSweepDirection(), useFixedSweepDirection));
    ASSERT_TRUE(useFixedHeight);
    ASSERT_TRUE(useFixedSweepDirection);

    sut->SetProperty(ExtrusionManipulationStrategy::prop_FixedHeight(), 5);
    sut->SetProperty(ExtrusionManipulationStrategy::prop_FixedSweepDirection(), DVec3d::From(1, 1, 1));

    double fixedHeight = 0;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionManipulationStrategy::prop_FixedHeight(), fixedHeight));
    ASSERT_DOUBLE_EQ(fixedHeight, 5);
    double height = 0;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionManipulationStrategy::prop_Height(), height));
    ASSERT_DOUBLE_EQ(height, 5);

    DVec3d fixedSweepDirection;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionManipulationStrategy::prop_FixedSweepDirection(), fixedSweepDirection));
    fixedSweepDirection.Normalize();
    DVec3d expectedSweepDirection = DVec3d::From(1, 1, 1);
    expectedSweepDirection.Normalize();
    ASSERT_TRUE(fixedSweepDirection.AlmostEqual(expectedSweepDirection));
    DVec3d sweepDirection;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(ExtrusionManipulationStrategy::prop_SweepDirection(), sweepDirection));
    sweepDirection.Normalize();
    ASSERT_TRUE(sweepDirection.AlmostEqual(expectedSweepDirection));

    sut->AppendDynamicKeyPoint({2,2,0});
    sut->AppendKeyPoint({0,0,0});
    sut->AppendDynamicKeyPoint({2,2,0});
    sut->AppendKeyPoint({1,0,0});
    sut->AppendDynamicKeyPoint({2,2,0});
    sut->AppendKeyPoint({3,1,0});
    sut->PopKeyPoint();
    sut->AppendDynamicKeyPoint({2,2,0});
    sut->AppendKeyPoint({1,1,0});
    sut->AppendDynamicKeyPoint({2,2,0});
    sut->AppendDynamicKeyPoint({0,0,0});
    ASSERT_TRUE(sut->IsComplete());

    ISolidPrimitivePtr solid = sut->FinishExtrusion(true, true);
    ASSERT_TRUE(solid.IsValid());
    CurveVectorPtr expectedBase = CurveVector::CreateLinear({{0,0,0},{1,0,0},{1,1,0},{0,0,0}}, CurveVector::BOUNDARY_TYPE_Outer);
    DVec3d expectedSweep = DVec3d::From(5, 5, 5);
    DgnExtrusionDetail extrusion;
    ASSERT_TRUE(solid->TryGetDgnExtrusionDetail(extrusion));
    ASSERT_TRUE(extrusion.m_baseCurve.IsValid());
    extrusion.m_baseCurve->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*extrusion.m_baseCurve, *expectedBase));
    ASSERT_TRUE(extrusion.m_extrusionVector.AlmostEqual(expectedSweep));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ExtrusionManipulationStrategyTestFixture, FixedSweep)
    {
    CurveVectorManipulationStrategyPtr baseManip = CurveVectorManipulationStrategy::Create();
    ASSERT_TRUE(baseManip.IsValid());
    baseManip->ChangeDefaultNewGeometryType(DefaultNewGeometryType::LineString);
    baseManip->ChangeDefaultPlacementStrategy(LineStringPlacementStrategyType::Points);
    ExtrusionManipulationStrategyPtr sut = ExtrusionManipulationStrategy::Create(*baseManip);
    ASSERT_TRUE(sut.IsValid());

    sut->SetProperty(ExtrusionManipulationStrategy::prop_UseFixedSweepDirection(), true);
    sut->SetProperty(ExtrusionManipulationStrategy::prop_FixedSweepDirection(), DVec3d::From(0, 0, 1));

    sut->AppendKeyPoint({0,0,0});
    sut->AppendKeyPoint({1,0,0});
    sut->AppendKeyPoint({1,1,0});
    sut->AppendKeyPoint({0,0,0});
    ASSERT_FALSE(sut->IsComplete());
    sut->SetProperty(ExtrusionManipulationStrategy::prop_BaseComplete(), true);
    ASSERT_FALSE(sut->IsComplete());
    sut->AppendKeyPoint({0,0,5});
    ASSERT_TRUE(sut->IsComplete());

    ISolidPrimitivePtr solid = sut->FinishExtrusion(true, true);
    ASSERT_TRUE(solid.IsValid());
    CurveVectorPtr expectedBase = CurveVector::CreateLinear({{0,0,0}, {1,0,0}, {1,1,0}, {0,0,0}}, CurveVector::BOUNDARY_TYPE_Outer);
    DVec3d expectedSweep = DVec3d::From(0, 0, 5);
    DgnExtrusionDetail extrusion;
    ASSERT_TRUE(solid->TryGetDgnExtrusionDetail(extrusion));
    ASSERT_TRUE(extrusion.m_baseCurve.IsValid());
    extrusion.m_baseCurve->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*extrusion.m_baseCurve, *expectedBase));
    ASSERT_TRUE(extrusion.m_extrusionVector.AlmostEqual(expectedSweep));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Vytautas.Kaniusonis             05/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ExtrusionManipulationStrategyTestFixture, CanAcceptMorePoints)
    {
    ExtrusionManipulationStrategyPtr sut = ExtrusionManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());
    //set base
    sut->AppendKeyPoint({ 0,0,0 });
    sut->AppendKeyPoint({ 10,0,0 });
    sut->AppendKeyPoint({ 10,10,0 });
    sut->AppendKeyPoint({ 0,10,0 });

    ASSERT_FALSE(sut->IsComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());

    ASSERT_FALSE(sut->IsBaseComplete());
    sut->SetProperty(ExtrusionManipulationStrategy::prop_BaseComplete(), true);
    ASSERT_TRUE(sut->IsBaseComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());

    //set height
    sut->AppendKeyPoint({ 0,10,10 });
    ASSERT_TRUE(sut->CanAcceptMorePoints());

    //set sweep direction
    sut->AppendKeyPoint({ 0,15,10 });
    ASSERT_FALSE(sut->CanAcceptMorePoints());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Vytautas.Kaniusonis             05/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ExtrusionManipulationStrategyTestFixture, CanAcceptMorePointsFixedHeight)
    {
    ExtrusionManipulationStrategyPtr sut = ExtrusionManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->SetProperty(ExtrusionManipulationStrategy::prop_UseFixedHeight(), true);
    //set base
    sut->AppendKeyPoint({ 0,0,0 });
    sut->AppendKeyPoint({ 10,0,0 });
    sut->AppendKeyPoint({ 10,10,0 });
    sut->AppendKeyPoint({ 0,10,0 });

    ASSERT_FALSE(sut->IsComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());

    ASSERT_FALSE(sut->IsBaseComplete());
    sut->SetProperty(ExtrusionManipulationStrategy::prop_BaseComplete(), true);
    ASSERT_TRUE(sut->IsBaseComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());

    //set sweep direction
    sut->AppendKeyPoint({ 0,15,10 });
    ASSERT_FALSE(sut->CanAcceptMorePoints());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Vytautas.Kaniusonis             05/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ExtrusionManipulationStrategyTestFixture, CanAcceptMorePointsFixedSweep)
    {
    ExtrusionManipulationStrategyPtr sut = ExtrusionManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->SetProperty(ExtrusionManipulationStrategy::prop_UseFixedSweepDirection(), true);
    //set base
    sut->AppendKeyPoint({ 0,0,0 });
    sut->AppendKeyPoint({ 10,0,0 });
    sut->AppendKeyPoint({ 10,10,0 });
    sut->AppendKeyPoint({ 0,10,0 });

    ASSERT_FALSE(sut->IsComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());

    ASSERT_FALSE(sut->IsBaseComplete());
    sut->SetProperty(ExtrusionManipulationStrategy::prop_BaseComplete(), true);
    ASSERT_TRUE(sut->IsBaseComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());

    //set height
    sut->AppendKeyPoint({ 0,10,10 });
    ASSERT_FALSE(sut->CanAcceptMorePoints());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Vytautas.Kaniusonis             05/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ExtrusionManipulationStrategyTestFixture, CanAcceptMorePointsFixedHeightAndSweep)
    {
    ExtrusionManipulationStrategyPtr sut = ExtrusionManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->SetProperty(ExtrusionManipulationStrategy::prop_UseFixedSweepDirection(), true);
    sut->SetProperty(ExtrusionManipulationStrategy::prop_UseFixedHeight(), true);
    //set base
    sut->AppendKeyPoint({ 0,0,0 });
    sut->AppendKeyPoint({ 10,0,0 });
    sut->AppendKeyPoint({ 10,10,0 });
    sut->AppendKeyPoint({ 0,10,0 });

    ASSERT_TRUE(sut->IsComplete());
    ASSERT_TRUE(sut->CanAcceptMorePoints());
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());

    ASSERT_FALSE(sut->IsBaseComplete());
    sut->SetProperty(ExtrusionManipulationStrategy::prop_BaseComplete(), true);
    ASSERT_TRUE(sut->IsBaseComplete());
    ASSERT_FALSE(sut->CanAcceptMorePoints());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ExtrusionManipulationStrategyTestFixture, SetProperty_ToBaseSplinePrimitive)
    {
    CurveVectorManipulationStrategyPtr baseStrategy = CurveVectorManipulationStrategy::Create();
    baseStrategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Spline);
    ExtrusionManipulationStrategyPtr sut = ExtrusionManipulationStrategy::Create(*baseStrategy);

    int initialOrder;
    ASSERT_EQ(BentleyStatus::ERROR, baseStrategy->TryGetProperty(SplineControlPointsPlacementStrategy::prop_Order(), initialOrder)); // there are no primitive strategies, to this returns error
    int expectedOrder = 10;
    baseStrategy->SetProperty(SplineControlPointsPlacementStrategy::prop_Order(), expectedOrder);

    int actualOrder = 0;
    ASSERT_EQ(BentleyStatus::SUCCESS, baseStrategy->TryGetProperty(SplineControlPointsPlacementStrategy::prop_Order(), actualOrder));
    ASSERT_EQ(actualOrder, expectedOrder);
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ExtrusionManipulationStrategyTestFixture, SetProperty_BaseWorkingPlane)
    {
    CurveVectorManipulationStrategyPtr baseStrategy = CurveVectorManipulationStrategy::Create();
    baseStrategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::LineString);
    baseStrategy->ChangeDefaultPlacementStrategy(LineStringPlacementStrategyType::Points);
    ExtrusionManipulationStrategyPtr sut = ExtrusionManipulationStrategy::Create(*baseStrategy);

    DPlane3d defaultWorkingPlane;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(CurveVectorManipulationStrategy::prop_WorkingPlane(), defaultWorkingPlane));

    DPlane3d updatedWorkingPlane = defaultWorkingPlane;
    updatedWorkingPlane.origin.z += 5;

    sut->SetProperty(CurveVectorManipulationStrategy::prop_WorkingPlane(), updatedWorkingPlane);
    DPlane3d actualWorkingPlane;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(CurveVectorManipulationStrategy::prop_WorkingPlane(), actualWorkingPlane));
    ASSERT_TRUE(actualWorkingPlane.origin.AlmostEqual(updatedWorkingPlane.origin));
    actualWorkingPlane.normal.Normalize();
    updatedWorkingPlane.normal.Normalize();
    ASSERT_TRUE(actualWorkingPlane.normal.AlmostEqual(updatedWorkingPlane.normal));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ExtrusionManipulationStrategyTestFixture, ChangedWorkingPlane)
    {
    CurveVectorManipulationStrategyPtr baseStrategy = CurveVectorManipulationStrategy::Create();
    baseStrategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::LineString);
    baseStrategy->ChangeDefaultPlacementStrategy(LineStringPlacementStrategyType::Points);
    ExtrusionManipulationStrategyPtr sut = ExtrusionManipulationStrategy::Create(*baseStrategy);

    sut->SetProperty(CurveVectorManipulationStrategy::prop_WorkingPlane(), DPlane3d::FromOriginAndNormal({0,0,5}, DVec3d::From(0, 0, 1)));
    sut->SetProperty(ExtrusionManipulationStrategy::prop_FixedHeight(), 5);
    sut->SetProperty(ExtrusionManipulationStrategy::prop_FixedSweepDirection(), DVec3d::From(0, 0, 1));

    sut->AppendKeyPoint({0,0,0});
    sut->AppendKeyPoint({5,0,0});

    ISolidPrimitivePtr solid = sut->FinishExtrusion(false, false);
    ASSERT_TRUE(solid.IsValid());
    DgnExtrusionDetail extrusion;
    ASSERT_TRUE(solid->TryGetDgnExtrusionDetail(extrusion));
    ASSERT_TRUE(extrusion.m_baseCurve.IsValid());

    CurveVectorPtr expectedBaseCurve = CurveVector::CreateLinear({{0,0,5},{5,0,5}}, CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_TRUE(extrusion.m_baseCurve->IsSameStructureAndGeometry(*expectedBaseCurve));
    }