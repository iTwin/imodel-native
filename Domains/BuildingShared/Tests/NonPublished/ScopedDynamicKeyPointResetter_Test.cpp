/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

BEGIN_BUILDING_SHARED_NAMESPACE

struct ScopedDynamicKeyPointResetterTestFixture : public BuildingSharedTestFixtureBase
    {};

struct TestResettableDynamic : IResettableDynamic
    {
    public:
        bool m_value = false;

    protected:
        virtual DynamicStateBaseCPtr _GetDynamicState() const override { return BooleanDynamicState::Create(m_value).get(); }
        virtual void _SetDynamicState(DynamicStateBaseCR state) override 
            {
            BooleanDynamicStateCPtr booleanState = dynamic_cast<BooleanDynamicStateCP>(&state);
            ASSERT_TRUE(booleanState.IsValid());
            m_value = booleanState->GetState();
            }
    };

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ScopedDynamicKeyPointResetterTestFixture, ScopeTest)
    {
    TestResettableDynamic resettable;

    resettable.m_value = false;
    ASSERT_FALSE(resettable.m_value);
    if (true)
        {
        ScopedDynamicKeyPointResetter resetter(resettable);
        ASSERT_FALSE(resettable.m_value);
        }
    ASSERT_FALSE(resettable.m_value);

    resettable.m_value = true;
    ASSERT_TRUE(resettable.m_value);
    if (true)
        {
        ScopedDynamicKeyPointResetter resetter(resettable);
        ASSERT_FALSE(resettable.m_value);
        }
    ASSERT_TRUE(resettable.m_value);
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ScopedDynamicKeyPointResetterTestFixture, PrimitiveStrategyWithNoChildStrategies)
    {
    LineStringManipulationStrategyPtr strategy = LineStringManipulationStrategy::Create();
    ASSERT_TRUE(strategy.IsValid());

    ASSERT_FALSE(strategy->IsDynamicKeyPointSet());
    strategy->AppendDynamicKeyPoint({0,0,0});
    ASSERT_TRUE(strategy->IsDynamicKeyPointSet());

    if (true)
        {
        ScopedDynamicKeyPointResetter resetter(*strategy);
        ASSERT_FALSE(strategy->IsDynamicKeyPointSet());
        }
    ASSERT_TRUE(strategy->IsDynamicKeyPointSet());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ScopedDynamicKeyPointResetterTestFixture, CurveVectorManipulationStrategyWith3Lines)
    {
    CurveVectorManipulationStrategyPtr strategy = CurveVectorManipulationStrategy::Create();
    ASSERT_TRUE(strategy.IsValid());

    strategy->AppendKeyPoint({0,0,0});
    strategy->AppendKeyPoint({1,0,0});
    strategy->AppendKeyPoint({1,1,0});
    strategy->AppendKeyPoint({2,1,0});

    ASSERT_FALSE(strategy->IsDynamicKeyPointSet());
    strategy->UpdateDynamicKeyPoint({0,1,0}, 2);
    ASSERT_TRUE(strategy->IsDynamicKeyPointSet());

    if (true)
        {
        ScopedDynamicKeyPointResetter resetter(*strategy);
        ASSERT_FALSE(strategy->IsDynamicKeyPointSet());
        }
    ASSERT_TRUE(strategy->IsDynamicKeyPointSet());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ScopedDynamicKeyPointResetterTestFixture, ExtrusionManipulationStrategy_DynamicHeight)
    {
    ExtrusionManipulationStrategyPtr strategy = ExtrusionManipulationStrategy::Create();
    ASSERT_TRUE(strategy.IsValid());

    strategy->AppendKeyPoint({0,0,0});
    strategy->AppendKeyPoint({1,0,0});
    strategy->SetProperty(ExtrusionManipulationStrategy::prop_BaseComplete(), true);

    ASSERT_FALSE(strategy->IsDynamicKeyPointSet());
    strategy->AppendDynamicKeyPoint({1,0,2});
    ASSERT_TRUE(strategy->IsDynamicKeyPointSet());

    if (true)
        {
        ScopedDynamicKeyPointResetter resetter(*strategy);
        ASSERT_FALSE(strategy->IsDynamicKeyPointSet());
        }
    ASSERT_TRUE(strategy->IsDynamicKeyPointSet());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ScopedDynamicKeyPointResetterTestFixture, ChildCurveVectorWithLineString)
    {
    CurveVectorPtr parentCurveVector = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_ParityRegion);
    parentCurveVector->Add(CurveVector::CreateLinear({{0,0,0},{1,0,0},{1,1,0}}));

    CurveVectorManipulationStrategyPtr strategy = CurveVectorManipulationStrategy::Create(*parentCurveVector);
    ASSERT_TRUE(strategy.IsValid());

    ASSERT_FALSE(strategy->IsDynamicKeyPointSet());
    strategy->UpdateDynamicKeyPoint({2,0,0}, 1);
    ASSERT_TRUE(strategy->IsDynamicKeyPointSet());

    if (true)
        {
        ScopedDynamicKeyPointResetter resetter(*strategy);
        ASSERT_FALSE(strategy->IsDynamicKeyPointSet());
        bvector<DPoint3d> keyPoints = strategy->GetKeyPoints();
        ASSERT_EQ(3, keyPoints.size());
        ASSERT_TRUE(keyPoints[1].AlmostEqual({1,0,0}));
        }
    ASSERT_TRUE(strategy->IsDynamicKeyPointSet());

    bvector<DPoint3d> keyPoints = strategy->GetKeyPoints();
    ASSERT_EQ(3, keyPoints.size());
    ASSERT_TRUE(keyPoints[1].AlmostEqual({2,0,0}));
    }