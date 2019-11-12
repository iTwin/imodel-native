/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"
#include "TestUtils.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct CurveVectorPlacementStrategyTests : public BuildingSharedTestFixtureBase
    {};

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorPlacementStrategyTests, IsComplete_Finish_WithArcs)
    {
    CurveVectorPlacementStrategyPtr sut = CurveVectorPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    CurveVectorPtr expectedCV1 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, {ICurvePrimitive::CreateArc(DEllipse3d::FromPointsOnArc({0,0,0},{2,2,0},{4,0,0}))});
    CurveVectorPtr expectedCV2 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, {ICurvePrimitive::CreateArc(DEllipse3d::FromPointsOnArc({0,0,0},{2,2,0},{4,0,0})),
                                                                                       ICurvePrimitive::CreateArc(DEllipse3d::FromPointsOnArc({4,0,0},{6,-2,0},{8,0,0}))});

    sut->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Arc);

    ASSERT_FALSE(sut->IsComplete()) << "Empty";
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({0,0,0});
    ASSERT_FALSE(sut->IsComplete()) << "Primitive not complete";
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({2,2,0});
    ASSERT_FALSE(sut->IsComplete()) << "Primitive not complete";
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({4,0,0});
    ASSERT_TRUE(sut->IsComplete()) << "First arc is complete";
    CurveVectorPtr actualCV1 = sut->Finish();
    ASSERT_TRUE(actualCV1.IsValid());
    ASSERT_TRUE(actualCV1->IsSameStructureAndGeometry(*expectedCV1));

    sut->AddKeyPoint({6,-2,0});
    ASSERT_FALSE(sut->IsComplete()) << "Second arc is not complete";
    CurveVectorPtr actualCV1_2 = sut->Finish();
    ASSERT_TRUE(actualCV1_2.IsValid());
    ASSERT_TRUE(actualCV1_2->IsSameStructureAndGeometry(*expectedCV1));

    sut->AddKeyPoint({8,0,0});
    ASSERT_TRUE(sut->IsComplete()) << "Both arcs complete";
    CurveVectorPtr actualCV2 = sut->Finish();
    ASSERT_TRUE(actualCV2.IsValid());
    ASSERT_TRUE(actualCV2->IsSameStructureAndGeometry(*expectedCV2));

    sut->PopKeyPoint();
    ASSERT_FALSE(sut->IsComplete());
    CurveVectorPtr actualCV1_3 = sut->Finish();
    ASSERT_TRUE(actualCV1_3.IsValid());
    ASSERT_TRUE(actualCV1_3->IsSameStructureAndGeometry(*expectedCV1));

    sut->PopKeyPoint();
    ASSERT_TRUE(sut->IsComplete());
    CurveVectorPtr actualCV1_4 = sut->Finish();
    ASSERT_TRUE(actualCV1_4.IsValid());
    ASSERT_TRUE(actualCV1_4->IsSameStructureAndGeometry(*expectedCV1));

    sut->PopKeyPoint();
    ASSERT_FALSE(sut->IsComplete());
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->PopKeyPoint();
    ASSERT_FALSE(sut->IsComplete());
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->PopKeyPoint();
    ASSERT_FALSE(sut->IsComplete());
    ASSERT_FALSE(sut->Finish().IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorPlacementStrategyTests, CreateLines)
    {
    CurveVectorPlacementStrategyPtr sut = CurveVectorPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    CurveVectorPtr expectedCV1 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, {ICurvePrimitive::CreateLine({0,0,0}, {2,0,0})});
    CurveVectorPtr expectedCV2 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, {ICurvePrimitive::CreateLine({0,0,0}, {2,0,0}),
                                                     ICurvePrimitive::CreateLine({2,0,0},{2,2,0})});

    sut->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Line);

    ASSERT_FALSE(sut->Finish().IsValid());
    
    sut->AddKeyPoint({0,0,0});
    ASSERT_FALSE(sut->Finish().IsValid());
    
    sut->AddKeyPoint({2,0,0});
    CurveVectorPtr cv1 = sut->Finish();
    ASSERT_TRUE(cv1.IsValid());
    ASSERT_TRUE(cv1->IsSameStructureAndGeometry(*expectedCV1));

    sut->AddKeyPoint({2,2,0});
    CurveVectorPtr cv2 = sut->Finish();
    ASSERT_TRUE(cv2.IsValid());
    ASSERT_TRUE(cv2->IsSameStructureAndGeometry(*expectedCV2));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorPlacementStrategyTests, CreateSplines)
    {
    CurveVectorPlacementStrategyPtr strategy = CurveVectorPlacementStrategy::Create();
    ASSERT_TRUE(strategy.IsValid()) << "Strategy creation should not fail";

    CurveVectorPtr expectedCV1 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, { TestUtils::CreateSpline({{0, 0, 0}, {1, 2, 0}, {2, 4, 0}}, 3) });
    CurveVectorPtr expectedCV2 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, { TestUtils::CreateSpline({{0, 0, 0}, {1, 2, 0}, {2, 4, 0}}, 3),
                                                                                        TestUtils::CreateSpline({{2, 4, 0}, {7, 8, 0}, {4, 2, 0}, {0, 4, 0}}, 4) });
    strategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Spline);
    ASSERT_TRUE(strategy->Finish().IsNull()) << "Should not be created with less than 2 points";

    strategy->AddKeyPoint({ 0, 0, 0 });
    ASSERT_TRUE(strategy->Finish().IsNull()) << "Should not be created with less than 2 points";

    strategy->AddKeyPoint({ 1, 2, 3 });
    strategy->AddKeyPoint({ 2, 4, 5 });
    CurveVectorPtr cv1 = strategy->Finish();
    ASSERT_TRUE(cv1.IsValid()) << "Failed to create curve vector";
    ASSERT_TRUE(cv1->IsSameStructureAndGeometry(*expectedCV1)) << "Created curve vector is incorrect";

    strategy->FinishContiniousPrimitive();

    SplineControlPointsPlacementStrategyPtr currentStrategy = dynamic_cast<SplineControlPointsPlacementStrategy *>(strategy->GetCurrentCurvePrimitivePlacementStrategy().get());
    ASSERT_TRUE(currentStrategy.IsValid()) << "Current strategy should be spline strategy";

    currentStrategy->SetProperty(SplineControlPointsPlacementStrategy::prop_Order(), 4);
    strategy->AddKeyPoint({ 7, 8, 2 });
    strategy->AddKeyPoint({ 4, 2, 3 });
    strategy->AddKeyPoint({ 0, 4, 5 });
    CurveVectorPtr cv2 = strategy->Finish();
    ASSERT_TRUE(cv2.IsValid()) << "Failed to create curve vector";
    ASSERT_TRUE(cv2->IsSameStructureAndGeometry(*expectedCV2)) << "Created curve vector is incorrect";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorPlacementStrategyTests, CreateInterpolationCurves)
    {
    CurveVectorPlacementStrategyPtr strategy = CurveVectorPlacementStrategy::Create();
    ASSERT_TRUE(strategy.IsValid()) << "Strategy creation should not fail";

    CurveVectorPtr expectedCV1 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, { TestUtils::CreateInterpolationCurve({ { 0, 0, 0 },{ 1, 2, 0 },{ 2, 4, 0 } }) });
    CurveVectorPtr expectedCV2 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, { TestUtils::CreateInterpolationCurve({ { 0, 0, 0 },{ 1, 2, 0 },{ 2, 4, 0 } }),
                                                                                        TestUtils::CreateInterpolationCurve({ { 2, 4, 0 },{ 7, 8, 0 },{ 4, 2, 0 },{ 0, 4, 0 } }, DVec3d::From(1, 2, 3), DVec3d::From(5, 4, 2)) });
    strategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::InterpolationCurve);
    ASSERT_TRUE(strategy->Finish().IsNull()) << "Should not be created with less than 2 points";

    strategy->AddKeyPoint({ 0, 0, 0 });
    ASSERT_TRUE(strategy->Finish().IsNull()) << "Should not be created with less than 2 points";

    strategy->AddKeyPoint({ 1, 2, 3 });
    strategy->AddKeyPoint({ 2, 4, 5 });
    CurveVectorPtr cv1 = strategy->Finish();
    ASSERT_TRUE(cv1.IsValid()) << "Failed to create curve vector";
    ASSERT_TRUE(cv1->IsSameStructureAndGeometry(*expectedCV1)) << "Created curve vector is incorrect";

    strategy->FinishContiniousPrimitive();

    SplineThroughPointsPlacementStrategyPtr currentStrategy = dynamic_cast<SplineThroughPointsPlacementStrategy *>(strategy->GetCurrentCurvePrimitivePlacementStrategy().get());
    ASSERT_TRUE(currentStrategy.IsValid()) << "Current strategy should be interpolation curve strategy";

    currentStrategy->SetStartTangent(DVec3d::From(1, 2, 3));
    currentStrategy->SetEndTangent(DVec3d::From(5, 4, 2));
    strategy->AddKeyPoint({ 7, 8, 2 });
    strategy->AddKeyPoint({ 4, 2, 3 });
    strategy->AddKeyPoint({ 0, 4, 5 });
    CurveVectorPtr cv2 = strategy->Finish();
    ASSERT_TRUE(cv2.IsValid()) << "Failed to create curve vector";
    ASSERT_TRUE(cv2->IsSameStructureAndGeometry(*expectedCV2)) << "Created curve vector is incorrect";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorPlacementStrategyTests, CreateLineSplineArc)
    {
    CurveVectorPlacementStrategyPtr strategy = CurveVectorPlacementStrategy::Create();
    ASSERT_TRUE(strategy.IsValid()) << "Strategy creation should not fail";

    CurveVectorPtr expectedCV1 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, { ICurvePrimitive::CreateLine({ 0, 0, 0 },{ 1, 2, 0 }) });
    CurveVectorPtr expectedCV2 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, { ICurvePrimitive::CreateLine({ 0, 0, 0 },{ 1, 2, 0 }),
                                                                                        TestUtils::CreateSpline({ { 1, 2, 0 },{ 2, 4, 0 },{ 7, 8, 0 },{ 4, 2, 0 }}, 4) });
    CurveVectorPtr expectedCV3 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, { ICurvePrimitive::CreateLine({ 0, 0, 0 },{ 1, 2, 0 }),
                                                                                        TestUtils::CreateSpline({ { 1, 2, 0 },{ 2, 4, 0 },{ 7, 8, 0 },{ 4, 2, 0 }}, 4),
                                                                                        ICurvePrimitive::CreateArc(DEllipse3d::FromPointsOnArc({ 4, 2, 0 },{ 3,1,0 },{ 1,1,0 })) });

    strategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Line);
    ASSERT_TRUE(strategy->Finish().IsNull()) << "Should not be created with less than 2 points";

    strategy->AddKeyPoint({ 0, 0, 0 });
    ASSERT_TRUE(strategy->Finish().IsNull()) << "Should not be created with less than 2 points";

    strategy->AddKeyPoint({ 1, 2, 3 });
    CurveVectorPtr cv1 = strategy->Finish();
    ASSERT_TRUE(cv1.IsValid()) << "Failed to create curve vector";
    ASSERT_TRUE(cv1->IsSameStructureAndGeometry(*expectedCV1)) << "Curve vector is incorrect";

    strategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Spline);
    SplineControlPointsPlacementStrategyPtr currentStrategy = dynamic_cast<SplineControlPointsPlacementStrategy *>(strategy->GetCurrentCurvePrimitivePlacementStrategy().get());
    ASSERT_TRUE(currentStrategy.IsValid()) << "Current strategy should be spline strategy";

    currentStrategy->SetProperty(SplineControlPointsPlacementStrategy::prop_Order(), 4);

    strategy->AddKeyPoint({ 2, 4, 5 });
    strategy->AddKeyPoint({ 7, 8, 2 });
    strategy->AddKeyPoint({ 4, 2, 3 });
    CurveVectorPtr cv2 = strategy->Finish();
    ASSERT_TRUE(cv2.IsValid()) << "Failed to create curve vector";
    ASSERT_TRUE(cv2->IsSameStructureAndGeometry(*expectedCV2)) << "Curve vector is incorrect";

    strategy->FinishContiniousPrimitive();
    strategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Arc);

    strategy->AddKeyPoint({ 3, 1, 0 });
    strategy->AddKeyPoint({ 1, 1, 0 });
    CurveVectorPtr cv3 = strategy->Finish();
    ASSERT_TRUE(cv3.IsValid()) << "Failed to create curve vector";
    ASSERT_TRUE(cv3->IsSameStructureAndGeometry(*expectedCV3)) << "Curve vector is incorrect";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorPlacementStrategyTests, CreateLineInterpolationCurveArc)
    {
    CurveVectorPlacementStrategyPtr strategy = CurveVectorPlacementStrategy::Create();
    ASSERT_TRUE(strategy.IsValid()) << "Strategy creation should not fail";

    CurveVectorPtr expectedCV1 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, { ICurvePrimitive::CreateLine({ 0, 0, 0 },{ 1, 2, 0 }) });
    CurveVectorPtr expectedCV2 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, { ICurvePrimitive::CreateLine({ 0, 0, 0 },{ 1, 2, 0 }),
                                                                                        TestUtils::CreateInterpolationCurve({ { 1, 2, 0 },{ 2, 4, 0 },{ 7, 8, 0 },{ 4, 2, 0 }}, DVec3d::From(5, 2, 1), DVec3d::From(1, 2, 5)) });
    CurveVectorPtr expectedCV3 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, { ICurvePrimitive::CreateLine({ 0, 0, 0 },{ 1, 2, 0 }),
                                                                                        TestUtils::CreateInterpolationCurve({ { 1, 2, 0 },{ 2, 4, 0 },{ 7, 8, 0 },{ 4, 2, 0 }}, DVec3d::From(5, 2, 1), DVec3d::From(1, 2, 5)),
                                                                                        ICurvePrimitive::CreateArc(DEllipse3d::FromPointsOnArc({ 4, 2, 0 },{ 3,1,0 },{ 1,1,0 })) });

    strategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Line);
    ASSERT_TRUE(strategy->Finish().IsNull()) << "Should not be created with less than 2 points";

    strategy->AddKeyPoint({ 0, 0, 0 });
    ASSERT_TRUE(strategy->Finish().IsNull()) << "Should not be created with less than 2 points";

    strategy->AddKeyPoint({ 1, 2, 3 });
    CurveVectorPtr cv1 = strategy->Finish();
    ASSERT_TRUE(cv1.IsValid()) << "Failed to create curve vector";
    ASSERT_TRUE(cv1->IsSameStructureAndGeometry(*expectedCV1)) << "Curve vector is incorrect";

    strategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::InterpolationCurve);
    SplineThroughPointsPlacementStrategyPtr currentStrategy = dynamic_cast<SplineThroughPointsPlacementStrategy *>(strategy->GetCurrentCurvePrimitivePlacementStrategy().get());
    ASSERT_TRUE(currentStrategy.IsValid()) << "Current strategy should be spline strategy";

    currentStrategy->SetStartTangent(DVec3d::From(5, 2, 1));
    currentStrategy->SetEndTangent(DVec3d::From(1, 2, 5));

    strategy->AddKeyPoint({ 2, 4, 5 });
    strategy->AddKeyPoint({ 7, 8, 2 });
    strategy->AddKeyPoint({ 4, 2, 3 });
    CurveVectorPtr cv2 = strategy->Finish();
    ASSERT_TRUE(cv2.IsValid()) << "Failed to create curve vector";
    ASSERT_TRUE(cv2->IsSameStructureAndGeometry(*expectedCV2)) << "Curve vector is incorrect";

    strategy->FinishContiniousPrimitive();
    strategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Arc);

    strategy->AddKeyPoint({ 3, 1, 0 });
    strategy->AddKeyPoint({ 1, 1, 0 });
    CurveVectorPtr cv3 = strategy->Finish();
    ASSERT_TRUE(cv3.IsValid()) << "Failed to create curve vector";
    ASSERT_TRUE(cv3->IsSameStructureAndGeometry(*expectedCV3)) << "Curve vector is incorrect";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorPlacementStrategyTests, CreateLineAndArc)
    {
    CurveVectorPlacementStrategyPtr sut = CurveVectorPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    CurveVectorPtr expectedCV1 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, {ICurvePrimitive::CreateLine({0,0,0}, {2,0,0})});
    CurveVectorPtr expectedCV2 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, {ICurvePrimitive::CreateLine({0,0,0}, {2,0,0}),
                                                     ICurvePrimitive::CreateArc(DEllipse3d::FromPointsOnArc({2,0,0},{3,1,0},{1,1,0}))});

    sut->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Line);

    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({0,0,0});
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({2,0,0});
    CurveVectorPtr cv1 = sut->Finish();
    ASSERT_TRUE(cv1.IsValid());
    ASSERT_TRUE(cv1->IsSameStructureAndGeometry(*expectedCV1));
    
    sut->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Arc);

    sut->AddKeyPoint({3,1,0});
    CurveVectorPtr cv1_1 = sut->Finish();
    ASSERT_TRUE(cv1_1.IsValid());
    ASSERT_TRUE(cv1_1->IsSameStructureAndGeometry(*expectedCV1));

    sut->AddKeyPoint({1,1,0});
    CurveVectorPtr cv2 = sut->Finish();
    ASSERT_TRUE(cv2.IsValid());
    ASSERT_TRUE(cv2->IsSameStructureAndGeometry(*expectedCV2));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorPlacementStrategyTests, CreateArcAndLine)
    {
    CurveVectorPlacementStrategyPtr sut = CurveVectorPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    CurveVectorPtr expectedCV1 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, {ICurvePrimitive::CreateArc(DEllipse3d::FromPointsOnArc({0,0,0}, {2,2,0}, {4,0,0}))});
    CurveVectorPtr expectedCV2 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, {ICurvePrimitive::CreateArc(DEllipse3d::FromPointsOnArc({0,0,0}, {2,2,0}, {4,0,0})),
                                                     ICurvePrimitive::CreateLine({4,0,0},{4,-2,0})});

    sut->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Arc);

    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({0,0,0});
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({2,2,0});
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({4,0,0});
    CurveVectorPtr cv1 = sut->Finish();
    ASSERT_TRUE(cv1.IsValid());
    ASSERT_TRUE(cv1->IsSameStructureAndGeometry(*expectedCV1));

    sut->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Line);

    sut->AddKeyPoint({4,-2,0});
    CurveVectorPtr cv2 = sut->Finish();
    ASSERT_TRUE(cv2.IsValid());
    ASSERT_TRUE(cv2->IsSameStructureAndGeometry(*expectedCV2));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorPlacementStrategyTests, ChangeFromLineToArc)
    {
    CurveVectorPlacementStrategyPtr sut = CurveVectorPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    CurveVectorPtr expectedCV = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, {ICurvePrimitive::CreateArc(DEllipse3d::FromPointsOnArc({0,0,0},{2,2,0},{4,0,0}))});

    sut->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Line);

    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({0,0,0});
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Arc);
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({2,2,0});
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({4,0,0});
    CurveVectorPtr cv = sut->Finish();
    ASSERT_TRUE(cv.IsValid());
    ASSERT_TRUE(cv->IsSameStructureAndGeometry(*expectedCV));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorPlacementStrategyTests, ChangeFromArcToSplineToInterpolationCurveToLine)
    {
    CurveVectorPlacementStrategyPtr strategy = CurveVectorPlacementStrategy::Create();
    ASSERT_TRUE(strategy.IsValid()) << "Failed to create strategy";

    CurveVectorPtr expectedCV0 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, { TestUtils::CreateSpline({{1, 2, 0}, {5, 6, 0}}, 3)});
    CurveVectorPtr expectedCV1 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, { TestUtils::CreateInterpolationCurve({{1, 2, 0}, {5, 6, 0}}, DVec3d::From(0, 0, 0), DVec3d::From(5, 2, 7)) });
    CurveVectorPtr expectedCV2 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, { TestUtils::CreateInterpolationCurve({{1, 2, 0}, {5, 6, 0}}, DVec3d::From(0, 0, 0), DVec3d::From(5, 2, 7)),
                                                                                        ICurvePrimitive::CreateLine({5, 6, 0}, {4, 2, 0})});

    strategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Arc);
    ASSERT_TRUE(strategy->Finish().IsNull()) << "Arc should not be created with less than 3 points";

    strategy->AddKeyPoint({ 1, 2, 3 });
    strategy->AddKeyPoint({ 5, 6, 7 });
    ASSERT_TRUE(strategy->Finish().IsNull()) << "Arc should not be created with less than 3 points";

    strategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Spline);
    CurveVectorPtr cv0 = strategy->Finish();
    ASSERT_TRUE(cv0.IsValid()) << "Failed to create curve vector";
    ASSERT_TRUE(cv0->IsSameStructureAndGeometry(*expectedCV0)) << "Curve vector is incorrect";

    strategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::InterpolationCurve);
    SplineThroughPointsPlacementStrategyPtr currentStrategy = dynamic_cast<SplineThroughPointsPlacementStrategy *>(strategy->GetCurrentCurvePrimitivePlacementStrategy().get());
    ASSERT_TRUE(currentStrategy.IsValid()) << "Current strategy should be spline strategy";
    currentStrategy->SetEndTangent(DVec3d::From(5, 2, 7));
    CurveVectorPtr cv1 = strategy->Finish();
    ASSERT_TRUE(cv1.IsValid()) << "Failed to create curve vector";
    ASSERT_TRUE(cv1->IsSameStructureAndGeometry(*expectedCV1)) << "Curve vector is incorrect";

    strategy->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Line);
    CurveVectorPtr cv1_2 = strategy->Finish();
    ASSERT_TRUE(cv1_2.IsValid());
    ASSERT_TRUE(cv1_2->IsSameStructureAndGeometry(*expectedCV1));
    
    strategy->AddKeyPoint({ 4, 2, 1 });
    CurveVectorPtr cv2 = strategy->Finish();
    ASSERT_TRUE(cv2.IsValid()) << "Failed to create curve vector";
    ASSERT_TRUE(cv2->IsSameStructureAndGeometry(*expectedCV2)) << "Curve vector is incorrect";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorPlacementStrategyTests, ChangeFromArcToLine_StartMidKeyPoints)
    {
    CurveVectorPlacementStrategyPtr sut = CurveVectorPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    CurveVectorPtr expectedCV = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, {ICurvePrimitive::CreateLine({0,0,0},{2,0,0})});

    sut->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Arc);

    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({0,0,0});
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({2,2,0});
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Line);
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({2,0,0});
    CurveVectorPtr cv = sut->Finish();
    ASSERT_TRUE(cv.IsValid());
    ASSERT_TRUE(cv->IsSameStructureAndGeometry(*expectedCV));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorPlacementStrategyTests, CreateLineString_Points)
    {
    CurveVectorPlacementStrategyPtr sut = CurveVectorPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    CurveVectorPtr expectedCV1 = CurveVector::CreateLinear({{0,0,0},{0,2,0}}, CurveVector::BOUNDARY_TYPE_Open);
    CurveVectorPtr expectedCV2 = CurveVector::CreateLinear({{0,0,0},{0,2,0},{2,2,0}}, CurveVector::BOUNDARY_TYPE_Open);

    sut->ChangeDefaultNewGeometryType(DefaultNewGeometryType::LineString);
    sut->ChangeDefaultPlacementStrategy(LineStringPlacementStrategyType::Points);

    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({0,0,0});
    ASSERT_FALSE(sut->Finish().IsValid());

    sut->AddKeyPoint({0,2,0});
    CurveVectorPtr cv1 = sut->Finish();
    ASSERT_TRUE(cv1.IsValid());
    ASSERT_TRUE(cv1->IsSameStructureAndGeometry(*expectedCV1));

    sut->AddKeyPoint({2,2,0});
    CurveVectorPtr cv2 = sut->Finish();
    ASSERT_TRUE(cv2.IsValid());
    ASSERT_TRUE(cv2->IsSameStructureAndGeometry(*expectedCV2));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorPlacementStrategyTests, CreateLineString_MetesAndBounds)
    {
    CurveVectorPlacementStrategyPtr sut = CurveVectorPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    CurveVectorPtr expectedCV1 = CurveVector::CreateLinear({{-1,0,0},{1,2,0},{-1,2,0}}, CurveVector::BOUNDARY_TYPE_Open);
    CurveVectorPtr expectedCV2 = CurveVector::CreateLinear({{-1,-1,0},{1,1,0},{-1,1,0}}, CurveVector::BOUNDARY_TYPE_Open);
    CurveVectorPtr expectedCvFinal = CurveVector::CreateLinear({{0,0,0}, {2,2,0}, {0,2,0}}, CurveVector::BOUNDARY_TYPE_Open);

    bvector<Utf8String> directions(2);
    BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::UnitConverter::DirectionToMeetsAndBoundsString(directions[0], DVec3d::FromStartEnd(DPoint3d::From(0, 0, 0), DPoint3d::From(2, 2, 0)));
    BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::UnitConverter::DirectionToMeetsAndBoundsString(directions[1], DVec3d::FromStartEnd(DPoint3d::From(2, 2, 0), DPoint3d::From(0, 2, 0)));
    bvector<double> lengths(2);
    lengths[0] = DSegment3d::From({0,0,0}, {2,2,0}).Length();
    lengths[1] = DSegment3d::From({2,2,0}, {0,2,0}).Length();

    sut->ChangeDefaultNewGeometryType(DefaultNewGeometryType::LineString);
    sut->ChangeDefaultPlacementStrategy(LineStringPlacementStrategyType::MetesAndBounds);

    ASSERT_FALSE(sut->Finish().IsValid());
    sut->AddDynamicKeyPoint({-1,0,0});
    sut->SetProperty(LineStringMetesAndBoundsPlacementStrategy::prop_MetesAndBounds(), MetesAndBounds(directions, lengths));
    CurveVectorPtr cv1 = sut->Finish();
    ASSERT_TRUE(cv1.IsValid());
    ASSERT_TRUE(cv1->IsSameStructureAndGeometry(*expectedCV1));

    sut->AddDynamicKeyPoint({-1,-1,0});
    sut->SetProperty(LineStringMetesAndBoundsPlacementStrategy::prop_MetesAndBounds(), MetesAndBounds(directions, lengths));
    CurveVectorPtr cv2 = sut->Finish();
    ASSERT_TRUE(cv2.IsValid());
    ASSERT_TRUE(cv2->IsSameStructureAndGeometry(*expectedCV2));

    sut->AddKeyPoint({0,0,0});
    sut->SetProperty(LineStringMetesAndBoundsPlacementStrategy::prop_MetesAndBounds(), MetesAndBounds(directions, lengths));
    CurveVectorPtr cvFinal = sut->Finish();
    ASSERT_TRUE(cvFinal.IsValid());
    ASSERT_TRUE(cvFinal->IsSameStructureAndGeometry(*expectedCvFinal));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorPlacementStrategyTests, ChangeDefaultPlacementStrategy)
    {
    CurveVectorPlacementStrategyPtr sut = CurveVectorPlacementStrategy::Create();
    ASSERT_TRUE(sut.IsValid());

    sut->ChangeDefaultNewGeometryType(DefaultNewGeometryType::Arc);
    sut->SetProperty("JUST_TO_TRIGGER_PRIMITIVE_STRATEGY_INIT", false);

    sut->ChangeDefaultPlacementStrategy(ArcPlacementMethod::StartMidEnd);
    if (true)
        {
        sut->AddKeyPoint({1,0,0});
        sut->AddKeyPoint({0,1,0});
        sut->AddDynamicKeyPoint({-1,0,0});

        CurveVectorPtr expected = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, {
            ICurvePrimitive::CreateArc(DEllipse3d::FromPointsOnArc({1,0,0},{0,1,0},{-1,0,0})),
            ICurvePrimitive::CreateLine(DSegment3d::From({-1,0,0},{1,0,0}))
        });
        CurveVectorPtr actual = sut->Finish(true);
        ASSERT_TRUE(actual.IsValid());
        actual->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
        ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*actual, *expected));
        }

    sut->ChangeDefaultPlacementStrategy(ArcPlacementMethod::StartEndMid); // keeps the start point
    if (true)
        {
        sut->AddKeyPoint({-3,0,0});
        sut->AddDynamicKeyPoint({-1,2,0});

        CurveVectorPtr expected = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, {
            ICurvePrimitive::CreateArc(DEllipse3d::FromPointsOnArc({1,0,0}, {-1,2,0}, {-3,0,0})),
            ICurvePrimitive::CreateLine(DSegment3d::From({-3,0,0}, {1,0,0}))
        });
        CurveVectorPtr actual = sut->Finish(true);
        ASSERT_TRUE(actual.IsValid());
        actual->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
        ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*actual, *expected));
        }
    }