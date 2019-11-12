/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    ICurvePrimitivePtr linePrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(linePrimitive.IsValid());
    DSegment3d segment;
    ASSERT_TRUE(linePrimitive->TryGetLine(segment));
    DPoint3d start, end;
    segment.GetEndPoints(start, end);
    ASSERT_TRUE(start.AlmostEqual({1,1,1}));
    ASSERT_TRUE(end.AlmostEqual({2,2,2}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(LineManipulationStrategyTests, Create_FromExistingSegment)
    {
    DPoint3d expectedStart = DPoint3d::From(0, 0, 0);
    DPoint3d expectedEnd = DPoint3d::From(1, 0, 0);
    DSegment3d expectedLine = DSegment3d::From(expectedStart, expectedEnd);

    LineManipulationStrategyPtr sut = LineManipulationStrategy::Create(expectedLine);
    ASSERT_TRUE(sut.IsValid());
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());
    ASSERT_TRUE(sut->IsComplete());

    bvector<DPoint3d> keyPoints = sut->GetKeyPoints();
    ASSERT_EQ(2, keyPoints.size());

    ICurvePrimitivePtr linePrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(linePrimitive.IsValid());
    DSegment3d actualLine;
    ASSERT_TRUE(linePrimitive->TryGetLine(actualLine));
    DPoint3d actualStart, actualEnd;
    actualLine.GetEndPoints(actualStart, actualEnd);
    ASSERT_TRUE(actualStart.AlmostEqual(expectedStart));
    ASSERT_TRUE(actualEnd.AlmostEqual(expectedEnd));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(LineManipulationStrategyTests, Create_FromExistingStartEnd)
    {
    DPoint3d expectedStart = DPoint3d::From(0, 0, 0);
    DPoint3d expectedEnd = DPoint3d::From(1, 0, 0);
    DSegment3d expectedLine = DSegment3d::From(expectedStart, expectedEnd);

    LineManipulationStrategyPtr sut = LineManipulationStrategy::Create(expectedStart, expectedEnd);
    ASSERT_TRUE(sut.IsValid());
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());
    ASSERT_TRUE(sut->IsComplete());

    bvector<DPoint3d> keyPoints = sut->GetKeyPoints();
    ASSERT_EQ(2, keyPoints.size());

    ICurvePrimitivePtr linePrimitive = sut->FinishPrimitive();
    ASSERT_TRUE(linePrimitive.IsValid());
    DSegment3d actualLine;
    ASSERT_TRUE(linePrimitive->TryGetLine(actualLine));
    DPoint3d actualStart, actualEnd;
    actualLine.GetEndPoints(actualStart, actualEnd);
    ASSERT_TRUE(actualStart.AlmostEqual(expectedStart));
    ASSERT_TRUE(actualEnd.AlmostEqual(expectedEnd));
    }