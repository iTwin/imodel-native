/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(TestCurvePrimitiveMS)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               08/2018
//=======================================================================================
struct CurvePrimitiveManipulationStrategyTestFixture : public BuildingSharedTestFixtureBase
    {};

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               08/2018
//=======================================================================================
struct TestCurvePrimitiveMS : CurvePrimitiveManipulationStrategy
    {
    DEFINE_T_SUPER(CurvePrimitiveManipulationStrategy)

    protected:
        TestCurvePrimitiveMS() : T_Super() {}

        virtual ICurvePrimitivePtr _FinishPrimitive() const override { return nullptr; }
        virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override { return nullptr; }
        virtual CurvePrimitiveManipulationStrategyPtr _Clone() const override { return nullptr; }
        virtual bool _IsContinious() const override { return false; }
        virtual bool _IsComplete() const override { return false; }
        virtual bool _CanAcceptMorePoints() const override { return true; }
        virtual bvector<ConstructionGeometry> _FinishConstructionGeometry() const override { return bvector<ConstructionGeometry>(); }
        virtual ICurvePrimitive::CurvePrimitiveType _GetResultCurvePrimitiveType() const override { return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid; }

    public:
        static TestCurvePrimitiveMSPtr Create() { return new TestCurvePrimitiveMS(); }

        //--------------------------------------------------------------------------------------
        // @bsimethod                                    Mindaugas.Butkus                08/2018
        //---------------+---------------+---------------+---------------+---------------+------
        void AppendDynamicKeyPointAndUpdateItImmediately(DPoint3dCR dynamicKeyPoint, DPoint3dCR updatedDynamicKeyPoint)
            {
            AppendDynamicKeyPoint(dynamicKeyPoint);
            _UpdateDynamicKeyPoint(updatedDynamicKeyPoint, _GetKeyPoints().size() - 1);
            }

        //--------------------------------------------------------------------------------------
        // @bsimethod                                    Mindaugas.Butkus                08/2018
        //---------------+---------------+---------------+---------------+---------------+------
        void AppendDynamicKeyPointAndInsertDynamicBefore(DPoint3dCR toAppend, DPoint3dCR toInsert)
            {
            AppendDynamicKeyPoint(toAppend);
            _InsertDynamicKeyPoint(toInsert, _GetKeyPoints().size() - 1);
            }

        //--------------------------------------------------------------------------------------
        // @bsimethod                                    Mindaugas.Butkus                08/2018
        //---------------+---------------+---------------+---------------+---------------+------
        void AppendMultipleDynamicsAndInsertDynamicBeforeTheLast(bvector<DPoint3d> const& toAppend, DPoint3dCR toInsert)
            {
            AppendDynamicKeyPoints(toAppend);
            _InsertDynamicKeyPoint(toInsert, _GetKeyPoints().size() - 1);
            }

        //--------------------------------------------------------------------------------------
        // @bsimethod                                    Mindaugas.Butkus                08/2018
        //---------------+---------------+---------------+---------------+---------------+------
        void AppendDynamicKeyPointAndInsertMultipleDynamicsBefore(DPoint3dCR toAppend, bvector<DPoint3d> const& toInsert)
            {
            AppendDynamicKeyPoint(toAppend);
            _InsertDynamicKeyPoints(toInsert, _GetKeyPoints().size() - 1);
            }

        //--------------------------------------------------------------------------------------
        // @bsimethod                                    Mindaugas.Butkus                08/2018
        //---------------+---------------+---------------+---------------+---------------+------
        void AppendMultipleDynamicsAndInsertMultipleDynamicsBeforeTheLast(bvector<DPoint3d> const& toAppend, bvector<DPoint3d> const& toInsert)
            {
            AppendDynamicKeyPoints(toAppend);
            _InsertDynamicKeyPoints(toInsert, _GetKeyPoints().size() - 1);
            }

        //--------------------------------------------------------------------------------------
        // @bsimethod                                    Mindaugas.Butkus                08/2018
        //---------------+---------------+---------------+---------------+---------------+------
        void AppendDynamicAndUpsertDynamicAfter(DPoint3dCR toAppend, DPoint3d toUpsert)
            {
            AppendDynamicKeyPoint(toAppend);
            _UpsertDynamicKeyPoint(toUpsert, _GetKeyPoints().size());
            }

        //--------------------------------------------------------------------------------------
        // @bsimethod                                    Mindaugas.Butkus                08/2018
        //---------------+---------------+---------------+---------------+---------------+------
        void AppendDynamicAndUpsertMultipleDynamicsAfter(DPoint3dCR toAppend, bvector<DPoint3d> const& toUpsert)
            {
            AppendDynamicKeyPoint(toAppend);
            _UpsertDynamicKeyPoints(toUpsert, _GetKeyPoints().size());
            }

        //--------------------------------------------------------------------------------------
        // @bsimethod                                    Mindaugas.Butkus                08/2018
        //---------------+---------------+---------------+---------------+---------------+------
        void AppendMultipleDynamicsAndUpdateMultipleDynamics(bvector<DPoint3d> const& toAppend, bvector<DPoint3d> const& toUpdate)
            {
            AppendDynamicKeyPoints(toAppend);
            _UpdateDynamicKeyPoints(toUpdate, _GetKeyPoints().size() - toUpdate.size());
            }
    };

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurvePrimitiveManipulationStrategyTestFixture, UpdateDynamicKeyPoint_AppendAndUpdateImmediately)
    {
    TestCurvePrimitiveMSPtr sut = TestCurvePrimitiveMS::Create();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());

    sut->AppendDynamicKeyPointAndUpdateItImmediately({0,0,0}, {1,0,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 1);
    ASSERT_TRUE(sut->IsDynamicKeyPointSet());
    ASSERT_TRUE(sut->GetKeyPoints().front().AlmostEqual({1,0,0}));

    sut->ResetDynamicKeyPoint();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurvePrimitiveManipulationStrategyTestFixture, InsertDynamicKeyPoint_AppendAndInsertBefore)
    {
    TestCurvePrimitiveMSPtr sut = TestCurvePrimitiveMS::Create();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());

    sut->AppendDynamicKeyPointAndInsertDynamicBefore({0,0,0}, {1,0,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);
    ASSERT_TRUE(sut->IsDynamicKeyPointSet());
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({0,0,0}));

    sut->ResetDynamicKeyPoint();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurvePrimitiveManipulationStrategyTestFixture, InsertDynamicKeyPoint_AppendMultipleAndInsertBeforeLast)
    {
    TestCurvePrimitiveMSPtr sut = TestCurvePrimitiveMS::Create();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());

    sut->AppendMultipleDynamicsAndInsertDynamicBeforeTheLast({{0,0,0},{1,0,0}}, {2,0,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 3);
    ASSERT_TRUE(sut->IsDynamicKeyPointSet());
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({0,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({2,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[2].AlmostEqual({1,0,0}));

    sut->ResetDynamicKeyPoint();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurvePrimitiveManipulationStrategyTestFixture, InsertDynamicKeyPoints_AppendAndInsertMultipleBefore)
    {
    TestCurvePrimitiveMSPtr sut = TestCurvePrimitiveMS::Create();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());

    sut->AppendDynamicKeyPointAndInsertMultipleDynamicsBefore({0,0,0}, {{1,0,0},{2,0,0}});
    ASSERT_EQ(sut->GetKeyPoints().size(), 3);
    ASSERT_TRUE(sut->IsDynamicKeyPointSet());
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({1,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({2,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[2].AlmostEqual({0,0,0}));

    sut->ResetDynamicKeyPoint();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurvePrimitiveManipulationStrategyTestFixture, InsertDynamicKeyPoints_AppendMultipleAndInsertMultipleBeforeLast)
    {
    TestCurvePrimitiveMSPtr sut = TestCurvePrimitiveMS::Create();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());

    sut->AppendMultipleDynamicsAndInsertMultipleDynamicsBeforeTheLast({{0,0,0},{1,0,0}}, {{2,0,0}, {3,0,0}});
    ASSERT_EQ(sut->GetKeyPoints().size(), 4);
    ASSERT_TRUE(sut->IsDynamicKeyPointSet());
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({0,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({2,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[2].AlmostEqual({3,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[3].AlmostEqual({1,0,0}));

    sut->ResetDynamicKeyPoint();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurvePrimitiveManipulationStrategyTestFixture, UpsertDynamicKeyPoint_AppendAndUpsertAfter)
    {
    TestCurvePrimitiveMSPtr sut = TestCurvePrimitiveMS::Create();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());

    sut->AppendDynamicAndUpsertDynamicAfter({0,0,0}, {1,0,0});
    ASSERT_EQ(sut->GetKeyPoints().size(), 2);
    ASSERT_TRUE(sut->IsDynamicKeyPointSet());
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({0,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({1,0,0}));

    sut->ResetDynamicKeyPoint();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurvePrimitiveManipulationStrategyTestFixture, UpsertDynamicKeyPoints_AppendAndUpsertMultipleAfter)
    {
    TestCurvePrimitiveMSPtr sut = TestCurvePrimitiveMS::Create();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());

    sut->AppendDynamicAndUpsertMultipleDynamicsAfter({0,0,0}, {{1,0,0},{2,0,0}});
    ASSERT_EQ(sut->GetKeyPoints().size(), 3);
    ASSERT_TRUE(sut->IsDynamicKeyPointSet());
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({0,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({1,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[2].AlmostEqual({2,0,0}));

    sut->ResetDynamicKeyPoint();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(CurvePrimitiveManipulationStrategyTestFixture, UpdateDynamicKeyPoints_AppendMultipleAndUpdateMultiple)
    {
    TestCurvePrimitiveMSPtr sut = TestCurvePrimitiveMS::Create();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());

    sut->AppendMultipleDynamicsAndUpdateMultipleDynamics({{0,0,0},{1,0,0},{2,0,0}}, {{3,0,0},{4,0,0}});
    ASSERT_EQ(sut->GetKeyPoints().size(), 3);
    ASSERT_TRUE(sut->IsDynamicKeyPointSet());
    ASSERT_TRUE(sut->GetKeyPoints()[0].AlmostEqual({0,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[1].AlmostEqual({3,0,0}));
    ASSERT_TRUE(sut->GetKeyPoints()[2].AlmostEqual({4,0,0}));

    sut->ResetDynamicKeyPoint();
    ASSERT_EQ(sut->GetKeyPoints().size(), 0);
    ASSERT_FALSE(sut->IsDynamicKeyPointSet());
    }