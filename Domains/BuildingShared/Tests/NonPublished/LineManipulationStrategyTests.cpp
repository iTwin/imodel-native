/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/LineManipulationStrategyTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct LineManipulationStrategyTests : public BuildingSharedTestFixtureBase
    {
    };

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(LineManipulationStrategyTests, AppendKeyPoint_AccepsOnly2KeyPoints)
    {
    GeometryManipulationStrategyPtr sut = LineManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    sut->AppendKeyPoint({1,1,1});
    sut->AppendKeyPoint({2,2,2});
    sut->AppendKeyPoint({3,3,3});
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,1,1}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({2,2,2}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(LineManipulationStrategyTests, Finish_CreatesLine)
    {
    CurvePrimitiveManipulationStrategyPtr sut = LineManipulationStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->AppendKeyPoint({1,1,1});
    sut->AppendKeyPoint({2,2,2});
    ICurvePrimitivePtr linePrimitive = sut->Finish();
    ASSERT_TRUE(linePrimitive.IsValid());
    DSegment3d segment;
    ASSERT_TRUE(linePrimitive->TryGetLine(segment));
    DPoint3d start, end;
    segment.GetEndPoints(start, end);
    ASSERT_TRUE(start.AlmostEqual({1,1,1}));
    ASSERT_TRUE(end.AlmostEqual({2,2,2}));
    }