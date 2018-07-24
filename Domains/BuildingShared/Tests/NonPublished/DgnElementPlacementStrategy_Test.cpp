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
#include "TestUtils.h"

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

        bvector<IGeometryPtr> m_constructionGeometry;

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

        virtual bvector<IGeometryPtr> _FinishConstructionGeometry() const override { return m_constructionGeometry; }

    public:
        static TestElementManipulationStrategyPtr Create(Dgn::DgnDbR db) { return new TestElementManipulationStrategy(db); }

        void SetConstructionGeometryForTest(bvector<IGeometryPtr> const& constructionGeometry) { m_constructionGeometry = constructionGeometry; }
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
            : T_Super(db)
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

        void SetConstructionGeometryForTest(bvector<IGeometryPtr> const& constructionGeometry) { m_manipStrategy->SetConstructionGeometryForTest(constructionGeometry); }
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

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnElementPlacementStrategyTestFixture, AddWorldOverlay)
    {
    TestElementPlacementStrategyPtr sut = TestElementPlacementStrategy::Create(GetDgnDb());
    ASSERT_TRUE(sut.IsValid());

    if (true)
        {
        CurveVectorPtr expectedCV = CurveVector::CreateLinear({{0,0,0},{1,0,0},{1,1,0}}, CurveVector::BOUNDARY_TYPE_Open);
        sut->SetConstructionGeometryForTest({IGeometry::Create(expectedCV)});
        FakeGraphicBuilderPtr builder = FakeGraphicBuilder::Create(GraphicBuilder::CreateParams::Scene(GetDgnDb()));
        sut->AddWorldOverlay(*builder);

        ASSERT_EQ(1, builder->m_geometry.size());
        IGeometryPtr actualGeometry = builder->m_geometry.front().m_geometry;
        ASSERT_TRUE(actualGeometry.IsValid());
        ASSERT_TRUE(actualGeometry->GetAsCurveVector().IsValid());
        ASSERT_TRUE(actualGeometry->GetAsCurveVector()->IsSameStructureAndGeometry(*expectedCV));
        ASSERT_EQ(1, builder->m_geometry.front().m_graphicParams.GetWidth());
        }

    if (true)
        {
        ICurvePrimitivePtr expectedLineString = ICurvePrimitive::CreateLineString({{1,0,0},{2,2,0},{3,3,0}});
        sut->SetConstructionGeometryForTest({IGeometry::Create(expectedLineString)});
        FakeGraphicBuilderPtr builder = FakeGraphicBuilder::Create(GraphicBuilder::CreateParams::Scene(GetDgnDb()));
        sut->AddWorldOverlay(*builder);

        ASSERT_EQ(1, builder->m_geometry.size());
        IGeometryPtr actualGeometry = builder->m_geometry.front().m_geometry;
        ASSERT_TRUE(actualGeometry.IsValid());
        ASSERT_TRUE(actualGeometry->GetAsICurvePrimitive().IsValid());
        ASSERT_TRUE(actualGeometry->GetAsICurvePrimitive()->IsSameStructureAndGeometry(*expectedLineString));
        ASSERT_EQ(1, builder->m_geometry.front().m_graphicParams.GetWidth());
        }

    if (true)
        {
        bvector<DPoint3d> points {{2,2,0},{2,3,0},{1,3,0}};
        ICurvePrimitivePtr expectedPointString = ICurvePrimitive::CreatePointString(points);
        sut->SetConstructionGeometryForTest({IGeometry::Create(expectedPointString)});
        FakeGraphicBuilderPtr builder = FakeGraphicBuilder::Create(GraphicBuilder::CreateParams::Scene(GetDgnDb()));
        sut->AddWorldOverlay(*builder);

        ASSERT_EQ(1, builder->m_geometry.size());
        IGeometryPtr actualGeometry = builder->m_geometry.front().m_geometry;
        ASSERT_TRUE(actualGeometry.IsValid());
        ASSERT_TRUE(actualGeometry->GetAsICurvePrimitive().IsValid());
        ASSERT_TRUE(actualGeometry->GetAsICurvePrimitive()->IsSameStructureAndGeometry(*expectedPointString));
        ASSERT_EQ(6, builder->m_geometry.front().m_graphicParams.GetWidth());
        }

    if (true)
        {
        ICurvePrimitivePtr expectedArc = ICurvePrimitive::CreateArc(DEllipse3d::FromCenterNormalRadius({0,0,0}, DVec3d::From(0, 0, 1), 5));
        sut->SetConstructionGeometryForTest({IGeometry::Create(expectedArc)});
        FakeGraphicBuilderPtr builder = FakeGraphicBuilder::Create(GraphicBuilder::CreateParams::Scene(GetDgnDb()));
        sut->AddWorldOverlay(*builder);

        ASSERT_EQ(1, builder->m_geometry.size());
        IGeometryPtr actualGeometry = builder->m_geometry.front().m_geometry;
        ASSERT_TRUE(actualGeometry.IsValid());
        ASSERT_TRUE(actualGeometry->GetAsICurvePrimitive().IsValid());
        ASSERT_TRUE(actualGeometry->GetAsICurvePrimitive()->IsSameStructureAndGeometry(*expectedArc));
        ASSERT_EQ(1, builder->m_geometry.front().m_graphicParams.GetWidth());
        }
    }