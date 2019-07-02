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

struct PointManipulationStrategyTestFixture : public BuildingSharedTestFixtureBase
    {
    };

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(PointManipulationStrategyTestFixture, AppendKeyPoint_AcceptsOnly1KeyPoint)
    {
    PointManipulationStrategyPtr sut = PointManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    sut->AppendKeyPoint({ 1,1,1 });
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    sut->AppendKeyPoint({ 2,2,2 });
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({ 1,1,1 }));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(PointManipulationStrategyTestFixture, AppendDynamicKeyPoint_LeavesNonDynamicKeyPoint)
    {
    PointManipulationStrategyPtr sut = PointManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    sut->AppendKeyPoint({ 1,1,1 });
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    sut->AppendDynamicKeyPoint({ 2,2,2 });
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({ 1,1,1 }));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(PointManipulationStrategyTestFixture, AppendDynamicKeyPoint_ReplacesPreviousDynamicKeyPoint)
    {
    PointManipulationStrategyPtr sut = PointManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    sut->AppendDynamicKeyPoint({ 1,1,1 });
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    sut->AppendKeyPoint({ 2,2,2 });
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({ 2,2,2 }));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(PointManipulationStrategyTestFixture, AppendKeyPoint_ReplacesPreviousDynamicKeyPoint)
    {
    PointManipulationStrategyPtr sut = PointManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    sut->AppendDynamicKeyPoint({ 1,1,1 });
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    sut->AppendKeyPoint({ 2,2,2 });
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({ 2,2,2 }));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(PointManipulationStrategyTestFixture, Clear_RemovesAllKeyPoints)
    {
    PointManipulationStrategyPtr sut = PointManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    sut->AppendDynamicKeyPoint({1,1,1});
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    sut->Clear();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    sut->AppendKeyPoint({2,2,2});
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    sut->Clear();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    }

//-------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                06/2019
//--------------+---------------+---------------+---------------+---------------+------   
TEST_F(PointManipulationStrategyTestFixture, ResetDynamicState)
    {
    PointManipulationStrategyPtr sut = PointManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    sut->AppendDynamicKeyPoint({0,0,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    ASSERT_TRUE(sut->GetKeyPoints().front().AlmostEqual(DPoint3d::From(0, 0, 0)));

    if (true)
        {
        ScopedDynamicKeyPointResetter resetter(*sut);
        ASSERT_EQ(sut->GetKeyPoints().size(), 0);
        }

    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    ASSERT_TRUE(sut->GetKeyPoints().front().AlmostEqual(DPoint3d::From(0, 0, 0)));

    sut->AppendKeyPoint({1,1,1});
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    ASSERT_TRUE(sut->GetKeyPoints().front().AlmostEqual(DPoint3d::From(1, 1, 1)));

    if (true)
        {
        ScopedDynamicKeyPointResetter resetter(*sut);
        ASSERT_EQ(sut->GetKeyPoints().size(), 1);
        ASSERT_TRUE(sut->GetKeyPoints().front().AlmostEqual(DPoint3d::From(1, 1, 1)));
        }

    // Check again if nothing changed in the resetter's destructor.
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    ASSERT_TRUE(sut->GetKeyPoints().front().AlmostEqual(DPoint3d::From(1, 1, 1)));
    }