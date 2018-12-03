/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/LineGridSurfaceManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

namespace BBS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

//=======================================================================================
// @bsiclass                                     Haroldas.Vitunskas             01/2018
//=======================================================================================
struct LineGridSurfaceManipulationStrategy : public GridPlanarSurfaceManipulationStrategy
    {
    DEFINE_T_SUPER(GridPlanarSurfaceManipulationStrategy)

    private:
        BBS::LineManipulationStrategyPtr m_geometryManipulationStrategy;
        BBS::LinePlacementStrategyPtr m_geometryPlacementStrategy;
        SketchLineGridSurfacePtr m_surface;

    protected:
        LineGridSurfaceManipulationStrategy(Dgn::DgnDbR db);

        // ElementManipulationStrategy
        virtual Dgn::DgnElementPtr _FinishElement(Dgn::DgnModelR model) override;
        virtual BBS::GeometryManipulationStrategyCR _GetGeometryManipulationStrategy() const override { return *m_geometryManipulationStrategy; }
        virtual BBS::GeometryManipulationStrategyR _GetGeometryManipulationStrategyForEdit() override { return *m_geometryManipulationStrategy; }
        virtual BBS::GeometryPlacementStrategyCPtr _TryGetGeometryPlacementStrategy() const  const override { return BBS::GeometryPlacementStrategyCPtr(m_geometryPlacementStrategy); }
        virtual BBS::GeometryPlacementStrategyPtr _TryGetGeometryPlacementStrategyForEdit() override { return m_geometryPlacementStrategy; }

        // GridPlanarSurfaceManipulationStrategy
        virtual PlanGridPlanarSurfaceCP _GetPlanGridPlanarSurfaceCP() const override { return m_surface.get(); }
        virtual PlanGridPlanarSurfaceP _GetPlanGridPlanarSurfaceP() const override { return m_surface.get(); }

        // SketchGridSurfaceManipulationStrategy
        virtual BentleyStatus _UpdateGridSurface() override;
        virtual Utf8String _GetMessage() const override;
        virtual BBS::CurvePrimitiveManipulationStrategyCR _GetCurvePrimitiveManipulationStrategy() const override { return *m_geometryManipulationStrategy; }
        virtual BBS::CurvePrimitiveManipulationStrategyR _GetCurvePrimitiveManipulationStrategyForEdit() override { return *m_geometryManipulationStrategy; }

    public:
        GRIDSTRATEGIES_EXPORT static LineGridSurfaceManipulationStrategyPtr Create(Dgn::DgnDbR db);
        GRIDSTRATEGIES_EXPORT static LineGridSurfaceManipulationStrategyPtr Create(SketchLineGridSurfaceR surface);
        static LineGridSurfaceManipulationStrategyPtr Create(BBS::LinePlacementStrategyType linePlacementStrategyType, Dgn::DgnDbR db);
    };

END_GRIDS_NAMESPACE