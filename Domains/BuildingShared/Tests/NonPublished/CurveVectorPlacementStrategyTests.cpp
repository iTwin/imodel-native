/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/CurveVectorPlacementStrategyTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

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