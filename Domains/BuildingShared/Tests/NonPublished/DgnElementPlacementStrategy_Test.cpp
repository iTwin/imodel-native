/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/DgnElementPlacementStrategy_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(TestElementPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(TestElementManipulationStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               05/2018
//=======================================================================================
struct DgnElementPlacementStrategyTestFixture : public BuildingSharedTestFixtureBase
    {
    static Dgn::DgnDbR GetDgnDb() { return *DgnClientFx::DgnClientApp::App().Project(); }
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               05/2018
//=======================================================================================
struct TestElementManipulationStrategy : DgnElementManipulationStrategy
    {
    DEFINE_T_SUPER(DgnElementManipulationStrategy)

    private:
        NullGeometryManipulationStrategyPtr m_geomManipStrategy;
        NullGeometryPlacementStrategyPtr m_geomPlaceStrategy;

    protected:
        TestElementManipulationStrategy(Dgn::DgnDbR db) 
            : T_Super(db) 
            {
            m_geomManipStrategy = NullGeometryManipulationStrategy::Create();
            m_geomPlaceStrategy = NullGeometryPlacementStrategy::Create(*m_geomManipStrategy);
            }

        Dgn::DgnElementPtr _FinishElement() override { return nullptr; }
        Dgn::DgnElementPtr _FinishElement(Dgn::DgnModelR) override { return nullptr; }

        virtual GeometryManipulationStrategyCR _GetGeometryManipulationStrategy() const override { return *m_geomManipStrategy; }
        virtual GeometryManipulationStrategyR _GetGeometryManipulationStrategyForEdit() override { return *m_geomManipStrategy; }
        virtual GeometryPlacementStrategyCPtr _TryGetGeometryPlacementStrategy() const override { return m_geomPlaceStrategy; }
        virtual GeometryPlacementStrategyPtr _TryGetGeometryPlacementStrategyForEdit() override { return m_geomPlaceStrategy; }
        virtual bvector<DPoint3d> _GetKeyPoints() const override { return bvector<DPoint3d>(); }

        virtual bool _IsDynamicKeyPointSet() const override { return false; }
        virtual void _ResetDynamicKeyPoint() override {}
        virtual bool _IsComplete() const override { return false; }
        virtual bool _CanAcceptMorePoints() const override { return false; }

    public:
        static TestElementManipulationStrategyPtr Create(Dgn::DgnDbR db) { return new TestElementManipulationStrategy(db); }
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               05/2018
//=======================================================================================
struct TestElementPlacementStrategy : DgnElementPlacementStrategy
    {
    DEFINE_T_SUPER(DgnElementPlacementStrategy)

    private:
        TestElementManipulationStrategyPtr m_manipStrategy;

    protected:
        TestElementPlacementStrategy(Dgn::DgnDbR db)
            : T_Super()
            , m_manipStrategy(TestElementManipulationStrategy::Create(db))
            {}

        virtual DgnElementManipulationStrategyCR _GetDgnElementManipulationStrategy() const override { return *m_manipStrategy; }
        virtual DgnElementManipulationStrategyR _GetDgnElementManipulationStrategyForEdit() override { return *m_manipStrategy; }
        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyForEdit() override { return *m_manipStrategy; }
        virtual void _AddViewOverlay(Dgn::Render::GraphicBuilderR builder, DRange3dCR viewRange, TransformCR worldToView, Dgn::ColorDefCR contrastingToBackgroundColor = Dgn::ColorDef::Black()) const override {}
        virtual Utf8String _GetMessage() const override { return ""; }

    public:
        static TestElementPlacementStrategyPtr Create(Dgn::DgnDbR db) { return new TestElementPlacementStrategy(db); }
    };

END_BUILDING_SHARED_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnElementPlacementStrategyTestFixture, GetFUS_Defaults)
    {
    DgnElementPlacementStrategyPtr sut = TestElementPlacementStrategy::Create(GetDgnDb());
    ASSERT_TRUE(sut.IsValid());

    FUSProperty lengthFUSProp, areaFUSProp, volumeFUSProp;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(DgnElementPlacementStrategy::prop_LengthFUS(), lengthFUSProp));
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(DgnElementPlacementStrategy::prop_AreaFUS(), areaFUSProp));
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(DgnElementPlacementStrategy::prop_VolumeFUS(), volumeFUSProp));

    ASSERT_STREQ("M", lengthFUSProp.GetFUS().GetUnitName().c_str());
    ASSERT_STREQ("SQ.M", areaFUSProp.GetFUS().GetUnitName().c_str());
    ASSERT_STREQ("CUB.M", volumeFUSProp.GetFUS().GetUnitName().c_str());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnElementPlacementStrategyTestFixture, GetFUS_Inches)
    {
    DgnElementPlacementStrategyPtr sut = TestElementPlacementStrategy::Create(GetDgnDb());
    ASSERT_TRUE(sut.IsValid());

    sut->SetProperty(DgnElementPlacementStrategy::prop_LengthFUS(), FUSProperty(Formatting::FormatUnitSet("IN(real4u)")));

    FUSProperty lengthFUSProp, areaFUSProp, volumeFUSProp;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(DgnElementPlacementStrategy::prop_LengthFUS(), lengthFUSProp));
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(DgnElementPlacementStrategy::prop_AreaFUS(), areaFUSProp));
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(DgnElementPlacementStrategy::prop_VolumeFUS(), volumeFUSProp));

    ASSERT_STREQ("IN", lengthFUSProp.GetFUS().GetUnitName().c_str());
    ASSERT_STREQ("SQ.IN", areaFUSProp.GetFUS().GetUnitName().c_str());
    ASSERT_STREQ("CUB.IN", volumeFUSProp.GetFUS().GetUnitName().c_str());
    }