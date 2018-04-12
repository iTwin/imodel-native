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

struct ExtrusionManipulationStrategyStrategyTests : public BuildingSharedTestFixtureBase
    {};

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ExtrusionManipulationStrategyStrategyTests, AppendDynamicKeyPoints)
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