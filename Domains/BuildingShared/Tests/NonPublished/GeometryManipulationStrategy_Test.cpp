/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/GeometryManipulationStrategy_Test.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct GeometryManipulationStrategyTests : public BuildingSharedTestFixtureBase
    {
    struct SUT : public CurvePrimitiveManipulationStrategy
        {
        protected:
            virtual bool _IsComplete() const override { return false; }
            virtual bool _CanAcceptMorePoints() const override { return true; }
            virtual bool _IsContinious() const override { return false; }
            virtual ICurvePrimitivePtr _FinishPrimitive() const override { return nullptr; }
            virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override { return nullptr; }
            virtual CurvePrimitiveManipulationStrategyPtr _Clone() const override { return nullptr; }
            virtual bvector<ConstructionGeometry> _FinishConstructionGeometry() const override { return bvector<ConstructionGeometry>(); }
            virtual ICurvePrimitive::CurvePrimitiveType _GetResultCurvePrimitiveType() const override { return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid; }
        };
    };

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyTests, SetDynamicKeyPoint_Insert_Appended)
    {
    GeometryManipulationStrategyPtr sut = new SUT();

    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    sut->InsertDynamicKeyPoint({1,2,3}, 0);
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    ASSERT_TRUE(sut->IsDynamicKeyPointSet());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyTests, SetDynamicKeyPoint_Insert)
    {
    GeometryManipulationStrategyPtr sut = new SUT();
    sut->AppendKeyPoint({1,2,3});
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    
    sut->InsertDynamicKeyPoint({4,5,6}, 0);
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({4,5,6}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({1,2,3}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyTests, SetDynamicKeyPoint_Update)
    {
    GeometryManipulationStrategyPtr sut = new SUT();
    sut->AppendKeyPoint({1,1,1});
    sut->AppendKeyPoint({2,2,2});
    sut->AppendKeyPoint({3,3,3});
    ASSERT_EQ(sut->GetKeyPoints().size(), 3);

    sut->UpdateDynamicKeyPoint({4,4,4}, 1);
    ASSERT_EQ(sut->GetKeyPoints().size(), 3);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,1,1}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({4,4,4}));
    ASSERT_TRUE(sut->GetKeyPoints()[2].AlmostEqual({3,3,3}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyTests, ResetDynamicKeyPoint)
    {
    GeometryManipulationStrategyPtr sut = new SUT();
    sut->AppendKeyPoint({1,1,1});
    sut->AppendKeyPoint({2,2,2});
    sut->AppendKeyPoint({3,3,3});
    ASSERT_EQ(sut->GetKeyPoints().size(), 3);

    ASSERT_FALSE(sut->IsDynamicKeyPointSet());
    sut->UpdateDynamicKeyPoint({4,4,4}, 1);
    ASSERT_TRUE(sut->IsDynamicKeyPointSet());
    sut->ResetDynamicKeyPoint();
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,1,1}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({2,2,2}));
    ASSERT_TRUE(sut->GetKeyPoints()[2].AlmostEqual({3,3,3}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyTests, AppendKeyPoint)
    {
    GeometryManipulationStrategyPtr sut = new SUT();

    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    sut->AppendKeyPoint({1,2,3});
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,2,3}));

    sut->AppendKeyPoint({4,5,6});
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,2,3}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({4,5,6}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyTests, AppendKeyPoint_ResetsInsertDynamicKeyPoint)
    {
    GeometryManipulationStrategyPtr sut = new SUT();

    sut->AppendKeyPoint({1,1,1});
    sut->InsertDynamicKeyPoint({2,2,2}, 1);
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,1,1}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({2,2,2}));
    sut->AppendKeyPoint({3,3,3});
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,1,1}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({3,3,3}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyTests, AppendKeyPoint_ResetsUpdateDynamicKeyPoint)
    {
    GeometryManipulationStrategyPtr sut = new SUT();

    sut->AppendKeyPoint({1,1,1});
    sut->UpdateDynamicKeyPoint({2,2,2}, 0);
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({2,2,2}));
    sut->AppendKeyPoint({3,3,3});
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,1,1}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({3,3,3}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyTests, InsertKeyPoint)
    {
    GeometryManipulationStrategyPtr sut = new SUT();
    sut->AppendKeyPoint({1,1,1});
    sut->AppendKeyPoint({2,2,2});
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);

    sut->InsertKeyPoint({3,3,3}, 1);
    ASSERT_EQ(sut->GetKeyPoints().size(), 3);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,1,1}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({3,3,3}));
    ASSERT_TRUE(sut->GetKeyPoints()[2].AlmostEqual({2,2,2}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyTests, InsertKeyPoint_Append)
    {
    GeometryManipulationStrategyPtr sut = new SUT();
    sut->AppendKeyPoint({1,1,1});
    sut->AppendKeyPoint({2,2,2});
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);

    sut->InsertKeyPoint({3,3,3}, 2);
    ASSERT_EQ(sut->GetKeyPoints().size(), 3);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,1,1}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({2,2,2}));
    ASSERT_TRUE(sut->GetKeyPoints()[2].AlmostEqual({3,3,3}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyTests, ReplaceKeyPoint)
    {
    GeometryManipulationStrategyPtr sut = new SUT();
    sut->AppendKeyPoint({1,1,1});
    sut->AppendKeyPoint({2,2,2});
    sut->AppendKeyPoint({3,3,3});
    ASSERT_EQ(sut->GetKeyPoints().size(), 3);

    sut->ReplaceKeyPoint({4,4,4}, 1);
    ASSERT_EQ(sut->GetKeyPoints().size(), 3);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,1,1}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({4,4,4}));
    ASSERT_TRUE(sut->GetKeyPoints()[2].AlmostEqual({3,3,3}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyTests, PopKeyPoint)
    {
    GeometryManipulationStrategyPtr sut = new SUT();
    sut->AppendKeyPoint({1,1,1});
    sut->AppendKeyPoint({2,2,2});
    sut->AppendKeyPoint({3,3,3});
    ASSERT_EQ(sut->GetKeyPoints().size(), 3);

    sut->PopKeyPoint();
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,1,1}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({2,2,2}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryManipulationStrategyTests, RemoveKeyPoint)
    {
    GeometryManipulationStrategyPtr sut = new SUT();
    sut->AppendKeyPoint({1,1,1});
    sut->AppendKeyPoint({2,2,2});
    sut->AppendKeyPoint({3,3,3});
    ASSERT_EQ(sut->GetKeyPoints().size(), 3);

    sut->RemoveKeyPoint(1);
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,1,1}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({3,3,3}));
    }