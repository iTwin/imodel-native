/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"
#include "TestUtils.h"

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(TestElementPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(TestElementManipulationStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(TestArcKeyPointContainer)

BEGIN_BUILDING_SHARED_NAMESPACE

#define CONSTRUCTION_GEOMTYPE_ValidForTest 999999

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

        bvector<ConstructionGeometry> m_constructionGeometry;

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
        virtual GeometryPlacementStrategyCPtr _TryGetGeometryPlacementStrategy() const override { return m_geomPlaceStrategy.get(); }
        virtual GeometryPlacementStrategyPtr _TryGetGeometryPlacementStrategyForEdit() override { return m_geomPlaceStrategy; }
        virtual bvector<DPoint3d> _GetKeyPoints() const override { return bvector<DPoint3d>(); }

        virtual bool _IsDynamicKeyPointSet() const override { return false; }
        virtual void _ResetDynamicKeyPoint() override {}
        virtual bool _IsComplete() const override { return false; }
        virtual bool _CanAcceptMorePoints() const override { return false; }

        virtual bvector<ConstructionGeometry> _FinishConstructionGeometry() const override { return m_constructionGeometry; }

    public:
        static TestElementManipulationStrategyPtr Create(Dgn::DgnDbR db) { return new TestElementManipulationStrategy(db); }

        void SetConstructionGeometryForTest(bvector<ConstructionGeometry> const& constructionGeometry) { m_constructionGeometry = constructionGeometry; }
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               11/2018
//=======================================================================================
struct TestArcKeyPointContainer : IArcKeyPointContainer
    {
    private:
        bool m_isStartSet;
        bool m_isCenterSet;
        bool m_isEndSet;
        bool m_isMidSet;
        DPoint3d m_start;
        DPoint3d m_center;
        DPoint3d m_end;
        DPoint3d m_mid;

    protected:
        virtual bool _TryGetStartKeyPoint(DPoint3dR p) const override { p.Init(m_start.x, m_start.y, m_start.z); return m_isStartSet; }
        virtual bool _TryGetCenterKeyPoint(DPoint3dR p) const override { p.Init(m_center.x, m_center.y, m_center.z); return m_isCenterSet; }
        virtual bool _TryGetMidKeyPoint(DPoint3dR p) const override { p.Init(m_mid.x, m_mid.y, m_mid.z); return m_isMidSet; }
        virtual bool _TryGetEndKeyPoint(DPoint3dR p) const override { p.Init(m_end.x, m_end.y, m_end.z); return m_isEndSet; }

    public:
        void SetStart(bool set, DPoint3dCP p) { m_isStartSet = set; if (nullptr != p) m_start.Init(p->x, p->y, p->z); }
        void SetCenter(bool set, DPoint3dCP p) { m_isCenterSet = set; if (nullptr != p) m_center.Init(p->x, p->y, p->z); }
        void SetEnd(bool set, DPoint3dCP p) { m_isEndSet = set; if (nullptr != p) m_end.Init(p->x, p->y, p->z); }
        void SetMid(bool set, DPoint3dCP p) { m_isMidSet = set; if (nullptr != p) m_mid.Init(p->x, p->y, p->z); }
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               05/2018
//=======================================================================================
struct TestElementPlacementStrategy 
    : DgnElementPlacementStrategy
    , IArcElementKeyPointContainer
    {
    DEFINE_T_SUPER(DgnElementPlacementStrategy)

    private:
        TestElementManipulationStrategyPtr m_manipStrategy;

        TestArcKeyPointContainerCP m_keyPointContainer;

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

        virtual IArcKeyPointContainer const& _GetIArcKeyPointContainer() const override { BeAssert(nullptr != m_keyPointContainer); return *m_keyPointContainer; }

    public:
        static TestElementPlacementStrategyPtr Create(Dgn::DgnDbR db) { return new TestElementPlacementStrategy(db); }

        void SetConstructionGeometryForTest(bvector<ConstructionGeometry> const& constructionGeometry) { m_manipStrategy->SetConstructionGeometryForTest(constructionGeometry); }
        void SetArcKeyPointContainerForTest(TestArcKeyPointContainerCP container) { m_keyPointContainer = container; }
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

    ASSERT_STREQ("M", lengthFUSProp.GetFUS().GetCompositeMajorUnit()->GetName().c_str());
    ASSERT_STREQ("SQ_M", areaFUSProp.GetFUS().GetCompositeMajorUnit()->GetName().c_str());
    ASSERT_STREQ("CUB_M", volumeFUSProp.GetFUS().GetCompositeMajorUnit()->GetName().c_str());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnElementPlacementStrategyTestFixture, GetFUS_Inches)
    {
    DgnElementPlacementStrategyPtr sut = TestElementPlacementStrategy::Create(GetDgnDb());
    ASSERT_TRUE(sut.IsValid());

    ECN::ECUnitCP unit = GetDgnDb().Schemas().GetUnit("Units", "IN");

    static Utf8String formatName = "DefaultRealU"; // real4u
    static Utf8String formatSchema = "Formats";
    ECN::ECFormatCP format = GetDgnDb().Schemas().GetFormat(formatSchema, formatName);

    // This creates a copy of the original format so that we can make the precision change.
    Formatting::Format formatOverride(*format);
    formatOverride.GetNumericSpecP()->SetPrecision(Formatting::DecimalPrecision::Precision4);

    auto compSpec = formatOverride.GetCompositeSpecP();
    if (nullptr == compSpec)
        {
        Formatting::CompositeValueSpec comp;
        Formatting::CompositeValueSpec::CreateCompositeSpec(comp, bvector<BEU::UnitCP>{unit});
        formatOverride.SetCompositeSpec(comp);
        }

    sut->SetProperty(DgnElementPlacementStrategy::prop_LengthFUS(), FUSProperty(formatOverride));

    FUSProperty lengthFUSProp, areaFUSProp, volumeFUSProp;
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(DgnElementPlacementStrategy::prop_LengthFUS(), lengthFUSProp));
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(DgnElementPlacementStrategy::prop_AreaFUS(), areaFUSProp));
    ASSERT_EQ(BentleyStatus::SUCCESS, sut->TryGetProperty(DgnElementPlacementStrategy::prop_VolumeFUS(), volumeFUSProp));

    ASSERT_STREQ("IN", lengthFUSProp.GetFUS().GetCompositeMajorUnit()->GetName().c_str());
    ASSERT_STREQ("SQ_IN", areaFUSProp.GetFUS().GetCompositeMajorUnit()->GetName().c_str());
    ASSERT_STREQ("CUB_IN", volumeFUSProp.GetFUS().GetCompositeMajorUnit()->GetName().c_str());
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
        sut->SetConstructionGeometryForTest({ConstructionGeometry(*IGeometry::Create(expectedCV), CONSTRUCTION_GEOMTYPE_GenericCurveVector)});
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
        sut->SetConstructionGeometryForTest({ConstructionGeometry(*IGeometry::Create(expectedLineString), CONSTRUCTION_GEOMTYPE_GenericICurvePrimitive)});
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
        sut->SetConstructionGeometryForTest({ConstructionGeometry(*IGeometry::Create(expectedPointString), CONSTRUCTION_GEOMTYPE_GenericICurvePrimitive)});
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
        sut->SetConstructionGeometryForTest({ConstructionGeometry(*IGeometry::Create(expectedArc), CONSTRUCTION_GEOMTYPE_GenericICurvePrimitive)});
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

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnElementPlacementStrategyTestFixture, ArcKeyPointContainer)
    {
    TestElementPlacementStrategyPtr sut = TestElementPlacementStrategy::Create(GetDgnDb());
    ASSERT_TRUE(sut.IsValid());

    TestArcKeyPointContainer container;
    sut->SetArcKeyPointContainerForTest(&container);

    DPoint3d start, mid, center, end;

    if (true)
        {
        container.SetStart(false, nullptr);
        container.SetCenter(false, nullptr);
        container.SetMid(false, nullptr);
        container.SetEnd(false, nullptr);
        ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
        ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
        ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
        ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
        }

    if (true)
        {
        DPoint3d tmpStart {1,0,0};
        container.SetStart(true, &tmpStart);
        container.SetCenter(false, nullptr);
        container.SetMid(false, nullptr);
        container.SetEnd(false, nullptr);
        ASSERT_TRUE(sut->TryGetStartKeyPoint(start));
        ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
        ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
        ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
        ASSERT_TRUE(start.AlmostEqual(tmpStart));
        }

    if (true)
        {
        DPoint3d tmpMid {0,1,0};
        container.SetStart(false, nullptr);
        container.SetCenter(false, nullptr);
        container.SetMid(true, &tmpMid);
        container.SetEnd(false, nullptr);
        ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
        ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
        ASSERT_TRUE(sut->TryGetMidKeyPoint(mid));
        ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
        ASSERT_TRUE(mid.AlmostEqual(tmpMid));
        }

    if (true)
        {
        DPoint3d tmpCenter {0,0,1};
        container.SetStart(false, nullptr);
        container.SetCenter(true, &tmpCenter);
        container.SetMid(false, nullptr);
        container.SetEnd(false, nullptr);
        ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
        ASSERT_TRUE(sut->TryGetCenterKeyPoint(center));
        ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
        ASSERT_FALSE(sut->TryGetEndKeyPoint(end));
        ASSERT_TRUE(center.AlmostEqual(tmpCenter));
        }

    if (true)
        {
        DPoint3d tmpEnd {1,1,1};
        container.SetStart(false, nullptr);
        container.SetCenter(false, nullptr);
        container.SetMid(false, nullptr);
        container.SetEnd(true, &tmpEnd);
        ASSERT_FALSE(sut->TryGetStartKeyPoint(start));
        ASSERT_FALSE(sut->TryGetCenterKeyPoint(center));
        ASSERT_FALSE(sut->TryGetMidKeyPoint(mid));
        ASSERT_TRUE(sut->TryGetEndKeyPoint(end));
        ASSERT_TRUE(end.AlmostEqual(tmpEnd));
        }
    }