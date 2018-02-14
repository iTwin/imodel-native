/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/CurveVectorManipulationStrategy_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"
#include "TestUtils.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct CurveVectorManipulationStrategyTestFixture : public BuildingSharedTestFixtureBase
    {};

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorManipulationStrategyTestFixture, GetKeyPoints_ContainsMultipleNotConnectedLineStrings)
    {
    CurveVectorPtr initialShape = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    initialShape->Add(ICurvePrimitive::CreateLineString({{0,0,0}, {1,0,0}, {1,1,0}}));
    initialShape->Add(ICurvePrimitive::CreateLineString({{3,1,0}, {0,1,0}, {0,2,0}}));

    CurveVectorManipulationStrategyPtr sut = CurveVectorManipulationStrategy::Create(*initialShape);
    ASSERT_TRUE(sut.IsValid());

    bvector<DPoint3d> expectedKeyPoints {{0,0,0},{1,0,0},{1,1,0},{3,1,0},{0,1,0},{0,2,0}};
    bvector<DPoint3d> const& actualKeyPoints = sut->GetKeyPoints();

    ASSERT_EQ(expectedKeyPoints.size(), actualKeyPoints.size());
    for (size_t i = 0; i < actualKeyPoints.size(); ++i)
        {
        ASSERT_TRUE(actualKeyPoints[i].AlmostEqual(expectedKeyPoints[i]));
        }
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorManipulationStrategyTestFixture, GetKeyPoints_ContainsMultipleConnectedLineStrings)
    {
    CurveVectorPtr initialShape = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    initialShape->Add(ICurvePrimitive::CreateLineString({{0,0,0}, {1,0,0}, {1,1,0}}));
    initialShape->Add(ICurvePrimitive::CreateLineString({{1,1,0}, {0,1,0}, {0,2,0}}));

    CurveVectorManipulationStrategyPtr sut = CurveVectorManipulationStrategy::Create(*initialShape);
    ASSERT_TRUE(sut.IsValid());

    bvector<DPoint3d> expectedKeyPoints {{0,0,0}, {1,0,0}, {1,1,0}, {0,1,0}, {0,2,0}};
    bvector<DPoint3d> const& actualKeyPoints = sut->GetKeyPoints();

    ASSERT_EQ(expectedKeyPoints.size(), actualKeyPoints.size());
    for (size_t i = 0; i < actualKeyPoints.size(); ++i)
        {
        ASSERT_TRUE(actualKeyPoints[i].AlmostEqual(expectedKeyPoints[i]));
        }
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorManipulationStrategyTestFixture, UpdateDynamicKeyPoint_ContainsMultipleNotConnectedLineStrings)
    {
    CurveVectorPtr initialShape = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    initialShape->Add(ICurvePrimitive::CreateLineString({{0,0,0},{1,0,0},{1,1,0}}));
    initialShape->Add(ICurvePrimitive::CreateLineString({{3,1,0},{0,1,0},{0,2,0}}));

    CurveVectorManipulationStrategyPtr sut = CurveVectorManipulationStrategy::Create(*initialShape);
    ASSERT_TRUE(sut.IsValid());

    // check if just initialized strategy creates same geometry
    CurveVectorPtr cv0 = sut->Finish();
    ASSERT_TRUE(cv0.IsValid());
    ASSERT_TRUE(cv0->IsSameStructureAndGeometry(*initialShape));

    CurveVectorPtr expectedShape1 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    expectedShape1->Add(ICurvePrimitive::CreateLineString({{0,0,0}, {2,0,0}, {1,1,0}}));
    expectedShape1->Add(ICurvePrimitive::CreateLineString({{3,1,0}, {0,1,0}, {0,2,0}}));
    sut->UpdateDynamicKeyPoint({2,0,0}, 1);
    CurveVectorPtr cv1 = sut->Finish();
    ASSERT_TRUE(cv1.IsValid());
    ASSERT_TRUE(cv1->IsSameStructureAndGeometry(*expectedShape1));

    CurveVectorPtr expectedShape2 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    expectedShape2->Add(ICurvePrimitive::CreateLineString({{0,0,0}, {1,0,0}, {1,1,0}}));
    expectedShape2->Add(ICurvePrimitive::CreateLineString({{3,1,0}, {-1,1,0}, {0,2,0}}));
    sut->UpdateDynamicKeyPoint({-1,1,0}, 4);
    CurveVectorPtr cv2 = sut->Finish();
    ASSERT_TRUE(cv2.IsValid());
    ASSERT_TRUE(cv2->IsSameStructureAndGeometry(*expectedShape2));

    CurveVectorPtr expectedShape3 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    expectedShape3->Add(ICurvePrimitive::CreateLineString({{0,0,0}, {1,0,0}, {2,1,0}}));
    expectedShape3->Add(ICurvePrimitive::CreateLineString({{3,1,0}, {0,1,0}, {0,2,0}}));
    sut->UpdateDynamicKeyPoint({2,1,0}, 2);
    CurveVectorPtr cv3 = sut->Finish();
    ASSERT_TRUE(cv3.IsValid());
    ASSERT_TRUE(cv3->IsSameStructureAndGeometry(*expectedShape3));
    
    CurveVectorPtr expectedShape4 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    expectedShape4->Add(ICurvePrimitive::CreateLineString({{0,0,0}, {1,0,0}, {1,1,0}}));
    expectedShape4->Add(ICurvePrimitive::CreateLineString({{2,1,0}, {0,1,0}, {0,2,0}}));
    sut->UpdateDynamicKeyPoint({2,1,0}, 3);
    CurveVectorPtr cv4 = sut->Finish();
    ASSERT_TRUE(cv4.IsValid());
    ASSERT_TRUE(cv4->IsSameStructureAndGeometry(*expectedShape4));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorManipulationStrategyTestFixture, UpdateDynamicKeyPoint_ContainsMultipleConnectedLineStrings)
    {
    CurveVectorPtr initialShape = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    initialShape->Add(ICurvePrimitive::CreateLineString({{0,0,0},{1,0,0},{1,1,0}}));
    initialShape->Add(ICurvePrimitive::CreateLineString({{1,1,0},{0,1,0},{0,2,0}}));

    CurveVectorManipulationStrategyPtr sut = CurveVectorManipulationStrategy::Create(*initialShape);
    ASSERT_TRUE(sut.IsValid());

    // check if just initialized strategy creates same geometry
    CurveVectorPtr cv0 = sut->Finish();
    ASSERT_TRUE(cv0.IsValid());
    ASSERT_TRUE(cv0->IsSameStructureAndGeometry(*initialShape));

    CurveVectorPtr expectedShape1 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    expectedShape1->Add(ICurvePrimitive::CreateLineString({{0,0,0},{2,0,0},{1,1,0}}));
    expectedShape1->Add(ICurvePrimitive::CreateLineString({{1,1,0},{0,1,0},{0,2,0}}));
    sut->UpdateDynamicKeyPoint({2,0,0}, 1);
    CurveVectorPtr cv1 = sut->Finish();
    ASSERT_TRUE(cv1.IsValid());
    ASSERT_TRUE(cv1->IsSameStructureAndGeometry(*expectedShape1));

    CurveVectorPtr expectedShape2 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    expectedShape2->Add(ICurvePrimitive::CreateLineString({{0,0,0},{1,0,0},{1,1,0}}));
    expectedShape2->Add(ICurvePrimitive::CreateLineString({{1,1,0},{-1,1,0},{0,2,0}}));
    sut->UpdateDynamicKeyPoint({-1,1,0}, 3);
    CurveVectorPtr cv2 = sut->Finish();
    ASSERT_TRUE(cv2.IsValid());
    ASSERT_TRUE(cv2->IsSameStructureAndGeometry(*expectedShape2));

    CurveVectorPtr expectedShape3 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    expectedShape3->Add(ICurvePrimitive::CreateLineString({{0,0,0},{1,0,0},{2,1,0}}));
    expectedShape3->Add(ICurvePrimitive::CreateLineString({{2,1,0},{0,1,0},{0,2,0}}));
    sut->UpdateDynamicKeyPoint({2,1,0}, 2);
    CurveVectorPtr cv3 = sut->Finish();
    ASSERT_TRUE(cv3.IsValid());
    ASSERT_TRUE(cv3->IsSameStructureAndGeometry(*expectedShape3));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurveVectorManipulationStrategyTestFixture, ReplaceKeyPoint_ContainsMultipleConnectedLineStrings)
    {
    CurveVectorPtr initialShape = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    initialShape->Add(ICurvePrimitive::CreateLineString({{0,0,0}, {1,0,0}, {1,1,0}}));
    initialShape->Add(ICurvePrimitive::CreateLineString({{1,1,0}, {0,1,0}, {0,2,0}}));

    CurveVectorManipulationStrategyPtr sut = CurveVectorManipulationStrategy::Create(*initialShape);
    ASSERT_TRUE(sut.IsValid());

    // check if just initialized strategy creates same geometry
    CurveVectorPtr cv0 = sut->Finish();
    ASSERT_TRUE(cv0.IsValid());
    ASSERT_TRUE(cv0->IsSameStructureAndGeometry(*initialShape));

    CurveVectorPtr expectedShape1 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    expectedShape1->Add(ICurvePrimitive::CreateLineString({{0,0,0}, {2,0,0}, {1,1,0}}));
    expectedShape1->Add(ICurvePrimitive::CreateLineString({{1,1,0}, {0,1,0}, {0,2,0}}));
    sut->ReplaceKeyPoint({2,0,0}, 1);
    CurveVectorPtr cv1 = sut->Finish();
    ASSERT_TRUE(cv1.IsValid());
    ASSERT_TRUE(cv1->IsSameStructureAndGeometry(*expectedShape1));

    CurveVectorPtr expectedShape2 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    expectedShape2->Add(ICurvePrimitive::CreateLineString({{0,0,0}, {2,0,0}, {1,1,0}}));
    expectedShape2->Add(ICurvePrimitive::CreateLineString({{1,1,0}, {-1,1,0}, {0,2,0}}));
    sut->ReplaceKeyPoint({-1,1,0}, 3);
    CurveVectorPtr cv2 = sut->Finish();
    ASSERT_TRUE(cv2.IsValid());
    ASSERT_TRUE(cv2->IsSameStructureAndGeometry(*expectedShape2));

    CurveVectorPtr expectedShape3 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    expectedShape3->Add(ICurvePrimitive::CreateLineString({{0,0,0}, {2,0,0}, {2,1,0}}));
    expectedShape3->Add(ICurvePrimitive::CreateLineString({{2,1,0}, {-1,1,0}, {0,2,0}}));
    sut->ReplaceKeyPoint({2,1,0}, 2);
    CurveVectorPtr cv3 = sut->Finish();
    ASSERT_TRUE(cv3.IsValid());
    ASSERT_TRUE(cv3->IsSameStructureAndGeometry(*expectedShape3));
    }