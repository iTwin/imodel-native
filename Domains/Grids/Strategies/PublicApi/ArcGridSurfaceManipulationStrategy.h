/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/ArcGridSurfaceManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

namespace BBS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               02/2018
//=======================================================================================
struct ArcGridSurfaceManipulationStrategy : public SketchGridSurfaceManipulationStrategy
    {
    DEFINE_T_SUPER(SketchGridSurfaceManipulationStrategy)

    private:
        BBS::ArcManipulationStrategyPtr m_geometryManipulationStrategy;
        BBS::ArcPlacementStrategyPtr m_geometryPlacementStrategy;
        SketchArcGridSurfacePtr m_surface;

    protected:
        ArcGridSurfaceManipulationStrategy(Dgn::DgnDbR db);

        // ElementManipulationStrategy
        virtual Dgn::DgnElementPtr _FinishElement(Dgn::DgnModelR model) override;
        virtual BBS::GeometryManipulationStrategyCR _GetGeometryManipulationStrategy() const override { return *m_geometryManipulationStrategy; }
        virtual BBS::GeometryManipulationStrategyR _GetGeometryManipulationStrategyForEdit() override { return *m_geometryManipulationStrategy; }
        virtual BBS::GeometryPlacementStrategyCPtr _TryGetGeometryPlacementStrategy() const  const override { return BBS::GeometryPlacementStrategyCPtr(m_geometryPlacementStrategy); }
        virtual BBS::GeometryPlacementStrategyPtr _TryGetGeometryPlacementStrategyForEdit() override { return m_geometryPlacementStrategy; }

        // SketchGridSurfaceManipulationStrategy
        virtual BentleyStatus _UpdateGridSurface() override;
        virtual Utf8String _GetMessage() const override { return ""; }
        virtual IPlanGridSurface const* _GetPlanGridSurfaceCP() const override { return m_surface.get(); }
        virtual IPlanGridSurface* _GetPlanGridSurfaceP() const override { return m_surface.get(); }
        virtual BBS::CurvePrimitiveManipulationStrategyCR _GetCurvePrimitiveManipulationStrategy() const override { return *m_geometryManipulationStrategy; }
        virtual BBS::CurvePrimitiveManipulationStrategyR _GetCurvePrimitiveManipulationStrategyForEdit() override { return *m_geometryManipulationStrategy; }

    public:
        GRIDSTRATEGIES_EXPORT static ArcGridSurfaceManipulationStrategyPtr Create(Dgn::DgnDbR db);
        static ArcGridSurfaceManipulationStrategyPtr Create(BBS::ArcPlacementMethod arcPlacementMethod, Dgn::DgnDbR db);
    };

END_GRIDS_NAMESPACE